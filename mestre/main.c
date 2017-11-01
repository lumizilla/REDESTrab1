#include "../utils.c"
//um char tem 1 byte = 8 bits

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
	int soquete = ConexaoRawSocket("eno1");
	//comando(ou pedaco de comando) a ser enviado, empacotado
	unsigned char comando[DATA_SIZE];
	//mensagem ja empacotada, pronta a ser enviada
	unsigned char mensagem[MSG_SIZE];
	//comando sem ser empacotado
	char comando_usuario[MAX_INPUT];
	//var auxiliar que salva o valor de comando usuario
	char comando_salvo[MAX_INPUT];
	//para guardar o input do usuario
	char *input;
	char *subs[3];

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

	char aux[DATA_SIZE];
	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando_usuario, MAX_INPUT, stdin);
		//salvando comando_usuario em outra variavel
		strcpy(comando_salvo, comando_usuario);
		//separando o comando em substrings
		//e limpando possivel lixos
		tipo = 0;
		subs[0] = "";
		subs[1] = "";
		subs[2] = "";

		/*getting the first substring*/
		input = strtok(comando_usuario, " ");
		/*walking trough the other substrings*/
		int i = 0;		
		while(input != NULL){
			subs[i] = input;
			input = strtok(NULL, " ");
			i = i+1;
		}
		printf("%s\n", subs[0]);
		//se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		if(strcmp(subs[0],"cd") != 0 && strcmp(subs[0],"ls") != 0 && strcmp(subs[0],"get") != 0 && strcmp(subs[0],"put") != 0){
			printf("ERRO: comando invalido\n");
		}
		else{	
			//empacotar mensagem no formato correto
			if(strcmp(subs[0], "cd") == 0){
				tipo = 6; 
			}
			else if(strcmp(subs[0], "ls") == 0){
				tipo = 7;
			}
			else if(strcmp(subs[0], "get") == 0){
				tipo = 8;
			}
			else if(strcmp(subs[0], "put") == 0){
				tipo = 9;
			}
			//descobre o numero de mensagens que precisarao ser enviadas para isso
			printf("%s\n", comando_salvo);
			tamMsg = strlen(comando_salvo);
			numMensagens = ceil((tamMsg/DATA_SIZE));
			printf("tipo %d, nummsg %d, tammsg %d\n", tipo, numMensagens, tamMsg);
			//empacota e envia cada pedaco

			int resto = tamMsg%DATA_SIZE;
			for(i = 0; i < (tamMsg - resto); i++){
				//copiando o maximo que da em comando
				if(((i%DATA_SIZE) == 0) && (i != 0)){
					strncpy(comando, aux, DATA_SIZE);
					printf("comando: %s\n", comando);
					empacotaMsg(comando, mensagem, tipo, sequencia, DATA_SIZE);
					printf("%s\n", mensagem);
					fflush(stdout);	
					write(soquete, comando, MSG_SIZE);
					sequencia = aumentaSeq(sequencia);
				}
				aux[i%DATA_SIZE] = comando_salvo[i];
			}
			//empacotando o ultimo pacote
			if(((i%DATA_SIZE) == 0) && (i != 0)){
					strncpy(comando, aux, DATA_SIZE);
					printf("comando: %s\n", comando);
					empacotaMsg(comando, mensagem, tipo, sequencia, DATA_SIZE);
					printf("%s\n", mensagem);
					fflush(stdout);	
					write(soquete, comando, MSG_SIZE);
					sequencia = aumentaSeq(sequencia);
			}
			//TODO empacota sobra
			if(resto != 0){
				printf("%d\n", strlen(comando_salvo));
				for(i = resto-1; i >= 0; i--){
					printf("tem resto, %d\n", tamMsg-resto+i);
					aux[i] = comando_salvo[tamMsg-resto+i];
					printf("aux = %s\n", aux);
					fflush(stdout);
				}	
				sequencia = aumentaSeq(sequencia);
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
