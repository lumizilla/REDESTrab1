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
long long int tamArquivo(char *filename) {
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
int desempacotaMsg(char *msg, char *data, short *seq, short *tam, short *tipo){
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
		data[i-3]= msg[i];
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

int enviaArquivo(char *arquivo, int soquete, long long int tamArq, short *seq){
	//TODO timeout nos dados
	
	//VARIAVEIS A RESPEITO DE MENSAGENS RECEBIDAS
	//mensagem recebida
	unsigned char msgRec[MSG_SIZE];
	//bits de DADOS da msg recebida
	unsigned char dataRec[DATA_SIZE];
	//numero de sequencia da mensagem recebida
	short seqRec = 0;
	//tamanho de DADOS da mensagem recebida
	short tamRec = 0;
	//tipo de mensagem a ser enviada/recebida
	short tipo = 0;

	//mensagem ja empacotada de tamanho maximo, pronta a ser enviada
	unsigned char mensagem[MSG_SIZE];
	
	//abre arquivo	
	FILE *fp;
	fp = fopen(arquivo, "r");
	if (fp==NULL) {fputs ("ERRO ao abrir arquivo para leitura\n",stderr); return -1;}
	
	//envia os dados

	//indica a mensagem mais a esquerda da janela
	int janelaInicio = *seq;
	//indica a mensagem mais a direita da janela, tem que aumentar 2 vezes ja que janela tem tam = 3
	int janelaFim = aumentaSeq(janelaInicio);
	janelaFim = aumentaSeq(janelaFim);

	//envia pedaco1
	char pedaco1[DATA_SIZE];
	int resultado = fread(pedaco1, DATA_SIZE, 1, fp);
	//TODO testar erros nos fread
	empacotaMsg(pedaco1, mensagem, DADO, *seq, DATA_SIZE);
	*seq = aumentaSeq(*seq);
	write(soquete, mensagem, MSG_SIZE); 
	//TODO aguada ACK desta mensagem antes de continuar

	//envia pedaco2
	char pedaco2[DATA_SIZE];
	resultado = fread(pedaco2, DATA_SIZE, 1, fp);
	//TODO testar erros nos fread
	empacotaMsg(pedaco2, mensagem, DADO, *seq, DATA_SIZE);
	*seq = aumentaSeq(*seq);
	write(soquete, mensagem, MSG_SIZE); 
	//envia pedaco3
	char pedaco3[DATA_SIZE];
	resultado = fread(pedaco3, DATA_SIZE, 1, fp);
	//TODO testar erros nos fread
	empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
	*seq = aumentaSeq(*seq);
	write(soquete, mensagem, MSG_SIZE); 
	
	//enquanto nao tiver enviado tudo
	while(!feof(fp)){
		//recebe mensagem
		read(soquete, msgRec, MSG_SIZE);
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		//se nao houve erro de paridade e nem de inicio
		if(status == 0){
			if(tipo == ACK){			
				//se mensagem = ACK pedaco1
				if(seqRec == janelaInicio){
					//anda janela 1 posicao (pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
				}
				//se mensagem = ACK pedaco2
				else if(seqRec == aumentaSeq(janelaInicio)){
					//anda janela 2 posicoes 
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
				}
				//se mensagem = ACK pedaco3
				else if(seqRec == janelaFim){
					//anda janela 3 posicoes
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
				}
			}
			else if(tipo == NACK){
				//se nack de pedaco1, reenviar pedaco, nao andar janela
				if(seqRec == janelaInicio){
					empacotaMsg(pedaco1, mensagem, DADO, *seq, DATA_SIZE);
					write(soquete, mensagem, MSG_SIZE); 
				}
				//se nack de pedaco2,
				else if(seqRec == aumentaSeq(janelaInicio)){
					//reenvia pedaco2
					empacotaMsg(pedaco2, mensagem, DADO, *seq, DATA_SIZE);
					write(soquete, mensagem, MSG_SIZE); 
					//anda janela 1 posicao
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
				}
				//se nack de pedaco3
				else if(seqRec == janelaFim){
					//reenvia pedaco3
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					write(soquete, mensagem, MSG_SIZE); 
					//anda janela 2 posicoes
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
					//(pedaco1 = pedaco2, pedaco2 = pedaco3, pedaco3 = envianovopedaco)
					strcpy(pedaco1, pedaco2);
					strcpy(pedaco2, pedaco3);
					resultado = fread(pedaco3, DATA_SIZE, 1, fp);
					//TODO testar erros nos fread
					empacotaMsg(pedaco3, mensagem, DADO, *seq, DATA_SIZE);
					*seq = aumentaSeq(*seq);
					write(soquete, mensagem, MSG_SIZE); 
					janelaInicio = aumentaSeq(janelaInicio);
					janelaFim = aumentaSeq(janelaFim);
				}
			}
		}
		//TODO testatimeouts
	}
	//fecha file pointer
	fclose (fp);

	//envia fim
	empacotaMsg("", mensagem, FIM, *seq, 0);
	int seqFim = *seq;
	*seq = aumentaSeq(*seq);
	write(soquete, mensagem, OVERLOAD_SIZE); 
	while(true){	
		//TODO testatimeouts
		//recebe mensagem
		read(soquete, msgRec, MSG_SIZE);
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		//se nao houve erro de paridade e nem de inicio
		if(status == 0){
			//se a sequencia corresponde
			if(seqRec == seqFim){	
				//aguarda OK
				if(tipo == OK){
					return 0;
				}else if(tipo == NACK){
					//se NACK, reenvia msg
					write(soquete, mensagem, OVERLOAD_SIZE); 
				}
			}
		}
	}
	return -1;
}

int recebeArquivo(char *arquivo, int soquete, long long int tamArq){
	//TODO com janela deslizante e timeout, recebe os dados e os salva
	
	//VARIAVEIS A RESPEITO DE MENSAGENS RECEBIDAS
	//mensagem recebida
	unsigned char msgRec[MSG_SIZE];
	//bits de DADOS da msg recebida
	unsigned char dataRec[DATA_SIZE];
	//numero de sequencia da mensagem recebida
	short seqRec = 0;
	//tamanho de DADOS da mensagem recebida
	short tamRec = 0;
	//tipo de mensagem a ser enviada/recebida
	short tipo = 0;
	
	//msg a ser enviada de ACK/NACK
	unsigned char msgStatus[OVERLOAD_SIZE];
	
	//marca quais pacotes foram recebidos
	int pedacos[3];
	pedacos[0] = 0;
	pedacos[1] = 0;
	pedacos[2] = 0;
	
	//variaveis a respeito de cada um dos pedacos que foram recebidos
	unsigned char data[3][DATA_SIZE];
	short seq[3];	
	short tam[3];

	//abre novo arquivo com nome = arquivo	
	//abre arquivo	
	FILE *fp;
	fp = fopen(arquivo, "w");
	if (fp==NULL) {fputs ("ERRO ao abrir arquivo para escrita\n",stderr); return -1;}
	//recebe seq do primeiro pedaco recebido
	int janelaInicio;
	//recebe primeiro pedaco
	//recebe mensagem
	read(soquete, msgRec, MSG_SIZE);
	int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
	//se nao houve erro de paridade e nem de inicio
	if(status == 0 && tipo == DADO){
		janelaInicio = seqRec;
		//envia ACK
		empacotaMsg("", msgStatus, ACK, seqRec, 0);
		write(soquete, msgStatus, OVERLOAD_SIZE); 
		//escreve no arquivo		
		fwrite(dataRec, tamRec, 1, fp); 
		janelaInicio = aumentaSeq(janelaInicio);
	}else{return -1;}
	
	int janelaFim = aumentaSeq(janelaInicio);
	janelaFim = aumentaSeq(janelaFim);
	//enquanto nao tiver recebido todos os pedacos
		while(true){
			//recebe pedaco
			read(soquete, msgRec, MSG_SIZE);
			int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
			if(status == -2){
				//TODO
			}
			else if(status == 0){
				//se pedaco == FIM, envia ACK
				if(tipo == FIM){
					//envia ACK
					empacotaMsg("", msgStatus, ACK, seqRec, 0);
					write(soquete, msgStatus, OVERLOAD_SIZE); 
					fclose(fp);	
					return 0;			
				}

				//SE PEDACO ESTA FORA DA JANELA E JA FOI RECEBIDO
				//TODO envia ACK dela e nao da break

				//SE PEDACO ESTA DENTRO DA JANELA						
				//se pedaco 1
				else if(seqRec == janelaInicio){
					//MANEJA PEDACO1
					pedacos[0] = 1;
					//salva dados
					strcpy(data[0], dataRec);
					seq[0] = seqRec;
					tam[0] = tamRec;
					//escreve no arquivo		
					fwrite(dataRec, tamRec, 1, fp); 
					
					//se pedaco 2 ja foi recebido E 3 nao foi
						//nao faz nada
					//se pedaco 2 ja foi recebido E 3 ja foi recebido
					else if(pedacos[1] == 1 && pedacos[2] == 1){
						//escreve no arquivo pedaco2		
						fwrite(data[1], tam[1], 1, fp); 
						//escreve no arquivo pedaco3		
						fwrite(data[2], tam[2], 1, fp); 
						//TODO ACK NO PEDACO 3
						//TODO anda janela
					}
				}			
				//se pedaco 2, e 1 nao foi recebido, aguarda 1
				else if(seqRec == aumentaSeq(janelaInicio) && pedacos[0] == 0){
					//MANEJA PEDACO2
					pedacos[1] = 1;
					//salva dados
					strcpy(data[1], dataRec);
					seq[1] = seqRec;
					tam[1] = tamRec;
				}			
				//se pedaco 2, e 1 foi recebido 
				else if(seqRec == aumentaSeq(janelaInicio) && pedacos[0] == 1){
					//MANEJA PEDACO2
					pedacos[1] = 1;
					//salva dados
					strcpy(data[1], dataRec);
					seq[1] = seqRec;
					tam[1] = tamRec;
					//escreve no arquivo pedaco2		
					fwrite(data[1], tam[1], 1, fp); 
				}	
				//se pedaco 2, e 1 e 3 foram recebidos
				else if(seqRec == aumentaSeq(janelaInicio) && pedacos[0] == 1 && pedacos[2] == 1){
					//MANEJA PEDACO2
					pedacos[1] = 1;
					//salva dados
					strcpy(data[1], dataRec);
					seq[1] = seqRec;
					tam[1] = tamRec;
					//escreve no arquivo pedaco2		
					fwrite(data[1], tam[1], 1, fp); 
					//escreve no arquivo pedaco3		
					fwrite(data[2], tam[2], 1, fp); 
				}	
				//se pedaco 3, e 2 ou 1 nao foram recebidos, aguarda 2 e 1
				else if(seqRec == janelaFim && (pedacos[0] == 0 || pedacos[1] == 0)){
					//MANEJA PEDACO3
					pedacos[2] = 1;
					//salva dados
					strcpy(data[2], dataRec);
					seq[2] = seqRec;
					tam[2] = tamRec;
				}	
				//se pedaco 3, e 2 e 1 foram recebidos 
				else if(seqRec == janelaFim && pedacos[0] == 1 && pedacos[1] == 1){
					//MANEJA PEDACO3
					pedacos[2] = 1;
					//salva dados
					strcpy(data[2], dataRec);
					seq[2] = seqRec;
					tam[2] = tamRec;
					//escreve no arquivo pedaco3		
					fwrite(data[2], tam[2], 1, fp); 
					//TODO ANDA JANELA
				}	
			}
		}
	return 0;
}
