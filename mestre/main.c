#include "../utils.c"
//um char tem 1 byte = 8 bits

void trataCD(char *msg, short seqMsg, short tamMsg, int soquete){
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
	while(true){
		//TODO fazer timeout como no T2
		read(soquete, msgRec, MSG_SIZE);
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		//TODO responde com NACK caso paridade nao bata
		
		if(seqRec == seqMsg){
			//aguarda OK
			if(tipo == OK){
				printf("OK: Servidor mudou de diretorio com sucesso.\n");
				return;
			}
			//se NACK, reenvia msg
			else if(tipo == NACK){
				write(soquete, msg, (tamMsg+OVERLOAD_SIZE));
				//TODO Atualiza timeout	
			}
			//se ERRO, printa erro
			else if(tipo == ERRO){
				printf("%s\n", dataRec);
				fflush(stdout);
				if(strcmp(dataRec, NAO_EXISTE) == 0){
					printf("ERRO NO SERVIDOR: Diretorio nao existe.\n");
				}
				else if(strcmp(dataRec, NAO_PERMITIDO) == 0){
					printf("ERRO NO SERVIDOR: Permissao negada.\n");
				}
				return;
			}
		}
	}
}

void trataPUT(char *msg, short seqMsg, short tamMsg, int soquete, short *seq, char *arquivo){
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

	//VARIAVEIS A RESPEITO DE MENSAGENS ENVIADAS
	//tamanho da mensagem(sem overload) a ser enviada	
	short tamEnv;
	//mensagem de tamanho do aquivo fica salva aqui
	unsigned char arqTam[DATA_SIZE];

	while(true){
		//TODO fazer timeout do put como no T2
		read(soquete, msgRec, MSG_SIZE);
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		
		if(seqRec == seqMsg){
			//aguarda OK
			if(tipo == OK){
				//TODO Atualiza timeout	
				printf("OK: Servidor aceitou o comando de PUT, iniciando troca de arquivos...\n");
				//envia tamanho do arquivo em bytes a ser enviado
				long long int tam_arquivo = tamArquivo(arquivo);
				tamEnv = sprintf(arqTam, "%lld", tam_arquivo);
				if(tam_arquivo != -1){
					if(tamMsg <= DATA_SIZE){
						//mensagem empacotada de tamanho certo
						char msgEmpacotada[tamEnv+OVERLOAD_SIZE];
						empacotaMsg(arqTam, msgEmpacotada, TAM, *seq, tamEnv);
						printf("Enviando tamanho %s\n", arqTam);	
						fflush(stdout);
						write(soquete, msgEmpacotada, (tamEnv+OVERLOAD_SIZE));	
						*seq = aumentaSeq(*seq);
						while(true){
							//TODO fazer timeout do TAM como no T2
							read(soquete, msgRec, MSG_SIZE);
							int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);		
							//aguarda OK
							if(tipo == OK){
								//TODO Atualiza timeout	
								printf("OK para escrever, memoria suficiente.\n");
								enviaArquivo(arquivo, soquete, tam_arquivo);
								return;
							}
							//se NACK, reenvia msg
							else if(tipo == NACK){
								write(soquete, msg, (tamMsg+OVERLOAD_SIZE));
								//TODO Atualiza timeout	
							}
							//se ERRO, printa erro
							else if(tipo == ERRO){
								//unico erro que a mensagem de tam pode gerar eh de espaco
								if(strcmp(dataRec, NAO_ESPACO)){
									printf("ERRO NO SERVIDOR: Espaço insuficiente.\n");
								}
								return;
							}
						}
					}
				}
				else{
					printf("ERRO: problema ao ler tamanho do arquivo.\n");
				}
				return;
			}
			//se NACK, reenvia msg
			else if(tipo == NACK){
				write(soquete, msg, (tamMsg+OVERLOAD_SIZE));
				//TODO Atualiza timeout	
			}
			//se ERRO, printa erro
			else if(tipo == ERRO){
				//unico erro que a mensagem de put inicial pode gerar eh de permissao
				if(strcmp(dataRec, NAO_PERMITIDO)){
					printf("ERRO NO SERVIDOR: Permissao negada.\n");
				}
				return;
			}
		}
	}		
}

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
	int soquete = ConexaoRawSocket("eno1");
	//int soquete = ConexaoRawSocket("enp3s0");
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
	//TODO alterar para subs ter tamanho qualquer
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
	char path[MAX_INPUT] = "./";

	printf("--------------------x--------------------\n");
	printf("Minishell - Como usar\n");
	printf("lls 'opcoes': ls local\n");
	printf("lcd 'caminho': cd local\n");
	printf("rls 'opcoes': ls remoto\n");
	printf("rcd 'caminho': cd remoto\n");
	printf("get 'arquivo': pega arquivo do dir corrente\n do escravo e coloca no dir corrente do mestre\n");
	printf("put 'arquivo': pega arquivo do dir corrente\n do mestre e coloca no dir corrente do escravo\n");
	printf("--------------------x--------------------\n\n");
	
	while(true){
		//lendo o comando
		printf("%s >> ", path);
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
		else if(strcmp(subs[1], "") == 0 && strcmp(subs[0], "lls") != 0 && strcmp(subs[0], "lcd") != 0){
			printf("ERRO: comando invalido, lcd, rcd, put e get necessitam de mais argumentos.\n");
		}
		else if(strcmp(subs[0],"lcd") == 0){		
			char caminho[MAX_INPUT];
			strcpy(caminho, "");
			if(subs[1] != NULL){
				strcat(caminho, subs[1]);
			}
			if(mudaDir(caminho) == 0 && (caminho != "")){
				apagaRelativos(caminho);
				//se nao houve erro, guardar no diretorio corrente
				strcpy(path, caminho);
			}

		}
		else if(strcmp(subs[0],"lls") == 0){
			strcpy(localCommand, "ls");
			//ls pode ter opcoes, por isso analisar o subs ate o final
			for(int i = 1; i < subsc; i++){	
				strcat(localCommand, " ");
				strcat(localCommand, subs[i]);
			}
			strcat(localCommand, "\n");
			//testa se deu erro, printar erro
			if(system(localCommand) == -1){
				printf("ERRO: erro ao executar o comando %s\n", localCommand);	
			}
			strcpy(localCommand, "");
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
				//TODO checa se arquivo existe antes de enviar
			}
			tamMsg = strlen(comando_salvo);
			if(tamMsg <= DATA_SIZE){
				for(i = 0; i < tamMsg; i++){
					comando[i] = comando_salvo[i];
				}
				
				//mensagem empacotada de tamanho certo
				char msgResto[tamMsg+OVERLOAD_SIZE];
				empacotaMsg(comando, msgResto, tipo, sequencia, tamMsg);
				write(soquete, msgResto, (tamMsg+OVERLOAD_SIZE));	
				sequencia = aumentaSeq(sequencia);
				
				switch(tipo){
					//ls
					case 7:
					//TODO, se for um ls, o mestre deve aguardar pelos pacotes e printar o pacote na tela
						break;
					//cd
					case 6:
						trataCD(msgResto, sequencia-1, tamMsg, soquete);		
						break;
					//put
					case 9:
						trataPUT(msgResto, sequencia-1, tamMsg, soquete, &sequencia, subs[1]);
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
