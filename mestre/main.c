#include "../utils.c"
//um char tem 1 byte = 8 bits

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
	int soquete = ConexaoRawSocket("eno1");
	//comando(ou pedaco de comando) a ser enviado, empacotado
	unsigned char comando[DATA_SIZE];
	//mensagem ja empacotada de tamanho maximo, pronta a ser enviada
	unsigned char mensagem[MSG_SIZE];
	//comando sem ser empacotado
	char comando_usuario[MAX_INPUT];
	//var auxiliar que salva o valor de comando usuario
	char comando_salvo[MAX_INPUT];
	//para guardar o input do usuario
	char *input;
	char *subs[3];

	//tipo de mensagem a ser enviada
	short tipo = 0;
	//numero de sequencia da mensagem
	short sequencia = 0;
	//numero de pedacos de mensagem a serem enviados
	int numMensagens = 0;
	//tamanho do pedaco mensagem a ser enviada
	short tam = 0;
	//tamanho da mensagem inteira
	short tamMsg = 0;
	//apenas para guardar comandos locais
	char localCommand[DATA_SIZE];
	//guarda o PATH do diretorio corrente
	char path[MAX_INPUT] = ".";

	printf("--------------------x--------------------\n");
	printf("Minishell - Como usar\n");
	printf("lls 'opcoes': ls local\n");
	printf("lcd 'caminho': cd local\n");
	printf("rls 'opcoes': ls remoto\n");
	printf("rcd 'caminho': cd remoto\n");
	printf("get 'arquivo': pega arquivo do dir corrente\n\t do escravo e coloca no dir corrente do mestre\n");
	printf("put 'arquivo': pega arquivo do dir corrente\n\t do mestre e coloca no dir corrente do escravo\n");
	printf("--------------------x--------------------\n\n");
	
	while(true){
		//lendo o comando
		printf(">> ");
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
		input = strtok(comando_usuario, " \n");
		/*walking trough the other substrings*/
		int i = 0;	
		while(input != NULL){
			subs[i] = input;
			input = strtok(NULL, " \n");
			i = i+1;
		}
		
		//subsc guarda a quantidade de argumentos em subs
		int subsc = i;
		
		//se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		if(strcmp(subs[0],"lcd") != 0 && strcmp(subs[0], "lls") != 0 && strcmp(subs[0],"rcd") != 0 && strcmp(subs[0],"rls") != 0 && strcmp(subs[0],"get") != 0 && strcmp(subs[0],"put") != 0){
			printf("ERRO: comando invalido\n");
		}
		else if(strcmp(subs[0],"lcd") == 0){		
			strcpy(localCommand, path);
			strcat(localCommand, "/");
			if(subs[1] != NULL){
				//TODO testar se o path novo eh muito grande e se tem "..", 
				//se sim apagar o que vem antes dos .. a nao ser que seja o './' inicial
				char *caminho = apagaRelativos(subs[1]);
				strcat(localCommand, caminho);
			}
			strcat(localCommand, "\n");
			printf("o comando local foi %s\n", localCommand);	
			if(mudaDir(localCommand) == 0){
				//se nao houve erro, guardar no diretorio corrente
				strcat(path, "/");
				strcat(path, caminho);
			}
		}
		else if(strcmp(subs[0],"lls") == 0){
			strcpy(localCommand, "ls ");
			//TODO ls pode ter opcoes, por isso analisar o subs ate o final
			for(int i = 1; i < subsc-1; i++){
				strcat(localCommand, subs[i]);
				strcat(localCommand, " ");
			}
			//se tiver opcoes, o ultimo subs deve ser o path
			strcat(localCommand, path);
			strcat(localCommand, "/");
			if(subs[subsc] != NULL){
				strcat(localCommand, subs[subsc]);
			}
			strcat(localCommand, "\n");
			//TODO testar se deu erro, printar erro
			system(localCommand);
		}
		else{	
			//empacotar mensagem no formato correto
			if(strcmp(subs[0], "rcd") == 0){
				tipo = 6; 
			}
			else if(strcmp(subs[0], "rls") == 0){
				tipo = 7;
			}
			else if(strcmp(subs[0], "get") == 0){
				tipo = 8;
			}
			else if(strcmp(subs[0], "put") == 0){
				tipo = 9;
			}

			tamMsg = strlen(comando_salvo);
			if(tamMsg <= DATA_SIZE){
				for(i = 0; i < tamMsg; i++){
					comando[i] = comando_salvo[i];
				}
				
				//mensagem empacotada de tamanho certo
				char msgResto[tamMsg+OVERLOAD_SIZE];
				empacotaMsg(comando, msgResto, tipo, sequencia, tamMsg);
				printf("%s\n", msgResto);
				fflush(stdout);
				write(soquete, msgResto, (tamMsg+OVERLOAD_SIZE));	
				sequencia = aumentaSeq(sequencia);
				
				switch(tipo){
				//TODO esperar resposta de acordo com o comando previamente enviado
					//ls
					case 7:
					//TODO, se for um ls, o mestre deve aguardar pelos pacotes e printar o pacote na tela
						break;
					//cd
					case 6:
					//basicamente nao faz nada
						break;
					case 9:
					//put
					//TODO se for um put, o mestre deve receber um ack e depois enviar os pacotes de dados
						break;
					case 8:
					//get
					//TODO se for um get, o mestre deve recever corretamente os pacotes enviados pelo escravo
						break;
					default:
						printf("ERRO: Tipo de mensagem nao esperado");
						break;
				}
			}
			else {
				printf("ERRO: O comando eh muito grande e nao cabe em uma mensagem.\n");
			}
		}
	}
}

/*
//descobre o numero de mensagens que precisarao ser enviadas para isso
			tamMsg = strlen(comando_salvo);
			numMensagens = ceil((tamMsg/DATA_SIZE));
			//printf("tipo %d, nummsg %d, tammsg %d\n", tipo, numMensagens, tamMsg);
			//empacota e envia cada pedaco

			int resto = tamMsg%DATA_SIZE;
			for(i = 0; i < (tamMsg - resto); i++){
				//copiando o maximo que da em comando
				if(((i%DATA_SIZE) == 0) && (i != 0)){
					strncpy(comando, aux, DATA_SIZE);
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
					empacotaMsg(comando, mensagem, tipo, sequencia, DATA_SIZE);
					printf("%s\n", mensagem);
					fflush(stdout);	
					write(soquete, comando, MSG_SIZE);
					sequencia = aumentaSeq(sequencia);
			}
			//empacota sobra
			if(resto != 0){
				for(i = resto-1; i >= 0; i--){
					aux[i] = comando_salvo[tamMsg-resto+i];
				}
				strncpy(comando, aux, resto);
				char msgResto[resto];
				empacotaMsg(comando, msgResto, tipo, sequencia, resto);
				printf("%s\n", msgResto);
				fflush(stdout);
				write(soquete, msgResto, MSG_SIZE);	
				sequencia = aumentaSeq(sequencia);
			}

			//TODO esperar resposta de acordo com o comando previamente enviado
			//ls
				//TODO, se for um ls, o mestre deve aguardar pelos pacotes e printar o pacote na tela
			//cd
				//basicamente nao faz nada
			//put
				//TODO se for um put, o mestre deve receber um ack e depois enviar os pacotes de dados
			//get
				//TODO se for um get, o mestre deve recever corretamente os pacotes enviados pelo escravo
			//ERRO
				//TODO printar erro
		}
*/
