//neste arquivo estao as funcoes de utilidade que podem ser usadas
//tanto pelo cliente quanto pelo servidor

#include "defines.h"

int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, (size_t) sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

//retona o tam de um arquivo em bytes
off_t tamArquivo(const char *filename) {
    struct stat st; 
    if (stat(filename, &st) == 0)
        return st.st_size;
    return -1; 
}

long long int checaMemoria(char *path, char* tam){
	struct statvfs stat;
	if(statvfs(path,&stat) != 0){
		//ocorreu um erro
		return -1;
	}
	//tamanho do bloco * numero de blocos disponiveis = bytes disponiveis
	if(atol(tam) <= stat.f_bsize * stat.f_bavail){
		return 0;
	}
	else{
		//tamanho eh grande demais
		return -1;
	}
}

void apagaRelativos(char *caminho){
	char *subs[MAX_PATH];
	/*getting the first substring*/
	char *input = strtok(caminho, "/");	
	/*walking trough the other substrings*/
	int i = 0;	
	while(input != NULL){
		subs[i] = input;
		input = strtok(NULL, "/");
		i = i+1;
	}

	//se o primeiro elemento eh um ..
	if(strcmp(subs[1], "..") == 0){
		subs[1]	= "";
	}
	if(strcmp(subs[1], ".") == 0){
		subs[1]	= "";
	}
	for(int j = 2; j < i; j++){
		//se o subs atual eh um .., apagar o path anterior a este que nao eh vazio
		if(strcmp(subs[j], "..") == 0){
			subs[j] = "";
			for(int k = j-1; k > 0; k --){
				if(strcmp(subs[k], "") != 0){
					subs[k] = "";
					break;
				}
			}
		}
		else if(strcmp(subs[j], ".") == 0){
			subs[1]	= "";
		}
	}
	//colocando tudo o que nao eh vazio de volta novamente em uma string
	char retorno[MAX_DIR*MAX_PATH];
	strcpy(retorno, "");
	for(int j = 0; j < i; j++){
		if(strcmp(subs[j], "") != 0){
			strcat(retorno, subs[j]);
			strcat(retorno, "/");
		}
	}
	
	strcpy(caminho, retorno);
	return;
}

int mudaDir(char *caminho){
	if(chdir(caminho) < 0){
		switch(errno){
			case EACCES:
				fprintf(stderr, "ERRO: Permissao negada.\n");
				break;
			case ELOOP:
				fprintf(stderr, "ERRO: Loop no link simbolico.\n");
				break;
			case ENAMETOOLONG:
				fprintf(stderr, "ERRO: %s eh muito grande.\n", caminho);
				break;
			case ENOTDIR:
				fprintf(stderr, "ERRO: %s nao eh um diretorio.\n", caminho);
  				break;
			case ENOENT:
				fprintf(stderr, "ERRO: %s nao existe.\n", caminho);
				break;
			default:
				fprintf(stderr, "ERRO: Algum erro ocorreu com o cd %s\n", caminho);
				break;
		}
		return errno;
	}
	else{
		return 0;	
	}	
}

int aumentaSeq (int seq){
	if(seq == SEQ_MAX){
		return 0;
	}
	int aux = seq + 1;
	return aux;
}

int empacotaMsg(char *msg, char *msgEnviar, short tipo, short seq, short tam){
	controle ctrl;
	//um unsigned short ou short then 2 bytes
	ctrl.tam = (unsigned short) tam;
	ctrl.seq = (unsigned short) seq;
	ctrl.tipo = (unsigned short) tipo;
	//EMPACOTA A MSG
        //Formato: INICIO | TAMANHO | SEQUENCIA | TIPO | DADOS | PARIDADE
	//Inicio tem 8 bits
	msgEnviar[0] = 0x7E;
	
        //Tamanho tem 5 bits
        //Sequencia tem 6 bits e vai ser referente ao i
        //Tipo tem 5 bits, portanto somando tam+seq+tipo temos 16 bits, 2 bytes
	//um char = 1 byte, por isso aqui devem ser atribuidos 2 chars
	char *aux = (char*) (&ctrl);
	msgEnviar[1] = aux[0];
	msgEnviar[2] = aux[1];

	//segfault nesta parte 
	// empacotar a msg
	
	for(int i = 3; i < tam+3; i++){
		msgEnviar[i] = msg[i-3];
	}

	//a paridade faz referencia aos campos: Tamanho, sequencia, tipo e dados. Eh vertical e Tem 8 bits
	char paridade = 0x00;
	char atual;
	char anterior;	
	anterior = msgEnviar[1];
	for(int i = 2; i < tam + 3; i++){
		atual = msgEnviar[i];			
		//Irei fazer um AND dos chars
		anterior = atual & anterior;
	}
	paridade = anterior;
	msgEnviar[tam+3] = paridade;
        return 0;
}

//retorna 0 para nenhum erro
//-1 para erro de inicio
//-2 para erro de paridade
int desempacotaMsg(unsigned char *msg, unsigned char *data, short *seq, short *tam, short *tipo){
	//Verificar inicio
	if(msg[0] != 0x7E){
		//printf("ERRO: O inicio da mensagem %s não confere\n", msg);
		return -1;
	}
	controle ctrl;
	char *aux = (char*) (&ctrl);
	aux[0] = msg[1];
	aux[1] = msg[2];
	//retirar informacao do tamanho
	*tam = ctrl.tam;
	//retornar o numero de sequencia
	*seq = ctrl.seq;
	//retornar tipo da mensagem
	*tipo = ctrl.tipo;
	//retornar dados extraidos
	for(int i = 3; i < *tam+3; i++){
		data[i-3] = msg[i];
	}
	//verificar se paridade bate
	char paridade = msg[*tam+3];
	char atual;
	char anterior;
	anterior = msg[1];
	for(int i = 2; i < *tam + 3; i++){
		atual = msg[i];			
		//Irei fazer um AND dos chars
		anterior = atual & anterior;
	}
	if(paridade != anterior){
		printf("ERRO: A paridade não confere\n");
		return -2;
	}
	return 0;
}

void enviaArquivo(char *arquivo, int soquete){

	return;
}
