#include "../utils.c"
//um char tem 1 byte = 8 bits

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
	int soquete = ConexaoRawSocket("eno1");
	//comando(ou pedaco de comando) a ser enviado, empacotado
	unsigned char comando[DATA_SIZE];
	//comando sem ser empacotado
	char comando_usuario[MAX_INPUT];
	//para guardar o input do usuario
	char *subs;
	unsigned char mensagens[SEQ_MAX][DATA_SIZE];
	//tipo de mensagem a ser enviada
	short tipo = 0;
	//numero de sequencia da mensagem
	short sequencia = 0;
	//numero de pedacos de mensagem a serem enviados
	int numMensagens = 0;
	//tamanho do pedaco mensagem a ser enviada
	short tam = 0;
	//tamanho da mensagem inteira
	int tamMsg = 0;

	const char s[2] = " ";
	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando_usuario, MAX_INPUT, stdin);
		printf("%s\n", comando_usuario);
		//separando o comando em substrings
		//e limpando possivel lixos
		tipo = 0;
		//subs[0] = "";
		//subs[1] = "";
		//subs[2] = "";

		/*getting the first substring*/
		subs = strtok(comando_usuario, s);
		/*walking trough the other substrings*/
		while(subs != NULL){
			printf("-%s", subs);
			subs = strtok(NULL, s);
		}
		printf("aqui");
		//se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		if(subs[0] != "cd" && subs[0] != "ls" && subs[0] != "get" && subs[0] != "put"){
			printf("ERRO: comando invalido\n");
		}
		else{	
			//empacotar mensagem no formato correto
			if(subs[0] == "cd"){
				tipo = 6; 
			}
			else if(subs[0] == "ls"){
				tipo = 7;
			}
			else if(subs[0] == "get"){
				tipo = 8;
			}
			else if(subs[0] == "put"){
				tipo = 9;
			}
			//descobre o numero de mensagens que precisarao ser enviadas para isso
			tamMsg = sizeof(comando_usuario);
			numMensagens = ceil((tamMsg/DATA_SIZE));
			//empacota e envia cada pedaco
			char *aux;
			int resto = tamMsg%DATA_SIZE;
			for(int i = 0; i < (tamMsg - resto); i++){
				//copiando o maximo que da em comando
				aux[i%tamMsg] = comando_usuario[i];
				if((i%tamMsg == 0) && (i != 0)){
					strncpy(comando, aux, DATA_SIZE);
					empacotaMsg(comando, comando, tipo, sequencia, tam);	
					write(soquete, comando, MSG_SIZE);
					sequencia = aumentaSeq(sequencia);
				}
			}
			
			//TODO empacota sobra
			if(resto != 0){
				printf("tem resto\n");
			}
			printf("acabou de empacotar\n");
			//TODO esperar resposta de acordo com o comando previamente enviado
			//ls
				//TODO, se for um ls, o mestre deve aguardar pelos pacotes e printar o pacote na tela
			//cd
				//TODO
			//put
				//TODO se for um put, o mestre deve receber um ack e depois enviar os pacotes de dados
			//get
				//TODO se for um get, o mestre deve recever corretamente os pacotes enviados pelo escravo
		}
	}
}
