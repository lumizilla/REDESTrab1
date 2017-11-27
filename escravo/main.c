#include "../utils.c"

int main(){
  printf("\niniciando programa escravo(servidor)!\n");
  int soquete = ConexaoRawSocket("eno1");
	//tipo de mensagem a ser enviada/recebida
	short tipo = 0;
	//numero de sequencia da mensagem a ser enviada
	short sequencia = 0;
	//numero de sequencia da mensagem recebida
	short seqRec = 0;
	//tamanho de DADOS da mensagem recebida
	short tamRec = 0;
	//mensagem recebida por completo
	unsigned char msgRec[MSG_SIZE];
	//bits de DADOS da msg recebida
	unsigned char dataRec[DATA_SIZE];
	//para guardar o input do usuario
	char *input;
	//TODO alterar para subs ter tamanho qualquer
	char *subs[3];
	//guarda o PATH do diretorio corrente
	char path[MAX_INPUT] = "./";
	//apenas para guardar comandos locais
	char localCommand[DATA_SIZE];

	//auxiliares
	int i;

	//nome do arquivo a ser enviadorecebido
	char nomeArq[FILE_NAME];
	
	unsigned char msgEnviar[MSG_SIZE];
	while(true){
		//recebe mensagem
		int r = read(soquete, msgRec, MSG_SIZE);
		//desempacota mensagem
		strcpy(dataRec, "");
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		dataRec[tamRec] = 0x00;
		//se retornou 0, nao houve erro, se retornou -1 inicio nao confere,
		//-2 paridade não confere.
		//se inicio nao confere soh ignora a mensagem
		//confere se paridade incorreta responder com NACK
		if(status == -2){
			//empacota NACK, SEQ = NUMERO DE SEQ DA MENSAGEM RECEBIDA
			printf("Mensagem recebida com paridade incoreta, enviando nack.\n");
			empacotaMsg("", msgEnviar, NACK, seqRec, 0);
			write(soquete, msgEnviar, OVERLOAD_SIZE);
		}
		//confere o tamanho da mensagem
		//confere qual a sequencia da mensagem
		if(status == 0){
			//cria resposta adequada para cada tipo de mensagem
			switch(tipo){
				case 6: //cd
					printf("Recebi um cd: %s\n", dataRec);
					/*getting the first substring*/
					input = strtok(dataRec, " \n");
					/*walking trough the other substrings*/
					i = 0;
					while(input != NULL){
						subs[i] = input;
						input = strtok(NULL, " \n");
						i = i+1;
					}
					int error = mudaDir(subs[1]);
					//responde com ACK
					if(error == 0){
						empacotaMsg("", msgEnviar, OK, seqRec, 0);
						write(soquete, msgEnviar, OVERLOAD_SIZE);
						apagaRelativos(subs[1]);
						//se nao houve erro, guardar no diretorio corrente
						//TODO para o ls, checar se da certo o ls de acordo com o cd
						strcpy(path, subs[1]);
					}
					//Se ERRO responde com o cod do erro
					else{
						if(error == EACCES){
							empacotaMsg(NAO_PERMITIDO, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
							printf("nao permitido\n");
							fflush(stdout);
							write(soquete, msgEnviar, sizeof(NAO_PERMITIDO)+OVERLOAD_SIZE);
						}
						else{
							empacotaMsg(NAO_EXISTE, msgEnviar, ERRO, seqRec, sizeof(NAO_EXISTE));
							printf("nao existe\n");
							fflush(stdout);
							write(soquete, msgEnviar, sizeof(NAO_EXISTE)+OVERLOAD_SIZE);
						}
					}
					break;
				case 7: {//ls 
					
					printf("Recebi um ls: %s\n", dataRec);
					fflush(stdout);
					if(dataRec[sizeof(dataRec)-1] == 0x0A){
						dataRec[sizeof(dataRec)-1] = 0x00;
					}
					/*getting the first substring*/
					input = strtok(dataRec, " \n");
					/*walking trough the other substrings*/
					i = 0;
					while(input != NULL){
						subs[i] = input;
						input = strtok(NULL, " \n");
						i = i+1;
					}
					strcpy(dataRec, "ls ");
					for(int j = 1; j < i; j++){
						strcat(dataRec, subs[j]);
					}
					FILE *fp;
					fp = fopen("ls.txt", "w");
					strcat(dataRec, " > ls.txt\n");
					int result = system(dataRec);
					if(result == -1){
						empacotaMsg(NAO_PERMITIDO, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
						printf("nao permitido: %s\n", msgEnviar);
						fflush(stdout);
						write(soquete, msgEnviar, sizeof(NAO_PERMITIDO)+OVERLOAD_SIZE);					
					}else if(result == 512){
						empacotaMsg(NAO_EXISTE, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
						printf("nao existe: %s\n", msgEnviar);
						fflush(stdout);
						write(soquete, msgEnviar, sizeof(NAO_EXISTE)+OVERLOAD_SIZE);					
					}				
					else if(result == 0){
						printf("codigo retornado: %d\n", result); 
						empacotaMsg("", msgEnviar, OK, seqRec, 0);
						printf("ls excutado com sucesso.\n");
						fflush(stdout);
						write(soquete, msgEnviar, OVERLOAD_SIZE);
						//tamanho da mensagem(sem overload) a ser enviada
						short tamEnv;
						//mensagem de tamanho do aquivo fica salva aqui
						unsigned char arqTam[DATA_SIZE];
						long long int tam_arquivo = tamArquivo("ls.txt");
						tamEnv = sprintf(arqTam, "%lld", tam_arquivo);
						//mensagem empacotada de tamanho certo
						char msgEmpacotada[tamEnv+OVERLOAD_SIZE];
						empacotaMsg(arqTam, msgEmpacotada, TAM, sequencia, tamEnv);
						printf("Enviando tamanho %s\n", arqTam);
						fflush(stdout);
						write(soquete, msgEmpacotada, (tamEnv+OVERLOAD_SIZE));
						sequencia = aumentaSeq(sequencia);
						int aux = 0;
						while(aux == 0){
							//TODO fazer timeout do TAM como no T2
							read(soquete, msgRec, MSG_SIZE);
							int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
							//aguarda OK
							if(tipo == OK){
								//TODO Atualiza timeout
								printf("OK para escrever, memoria suficiente.\n");
								enviaArquivo("ls.txt", soquete, tam_arquivo, &sequencia, MOSTRA);
								aux = -1;
							}
							//se NACK, reenvia msg
							else if(tipo == NACK){
								write(soquete, msgEmpacotada, (tamEnv+OVERLOAD_SIZE));
								//TODO Atualiza timeout
							}
						}
						system("rm ls.txt");
						}
					}
					break;
				case 8: //get
					//TODO Responde com ACK/ERRO, se foi um ACK enviar o TAM do arquivo e os dados e o OK
					printf("Recebi um get: %s\n", dataRec);
					fflush(stdout);
					/*getting the first substring*/
					input = strtok(dataRec, " \n");
					/*walking trough the other substrings*/
					i = 0;
					while(input != NULL){
						subs[i] = input;
						input = strtok(NULL, " \n");
						i = i+1;
					}
					strcpy(nomeArq, subs[1]);
					//checa se pode ler deste diretorio
					if(access("./", R_OK) == 0){
						//Responde com OK
						if(access(subs[1], F_OK) != -1){
							empacotaMsg("", msgEnviar, OK, seqRec, 0);
							printf("pode ler o arquivo deste diretorio.\n");
							fflush(stdout);
							write(soquete, msgEnviar, OVERLOAD_SIZE);
							//se foi um OK, envia o tamanho do arquivo
							long long int tam_arquivo = tamArquivo(nomeArq);
							//mensagem de tamanho do aquivo fica salva aqui
							unsigned char arqTam[DATA_SIZE];
							short tamEnv = sprintf(arqTam, "%lld", tam_arquivo);
							if(tam_arquivo != -1){
								//mensagem empacotada de tamanho certo
								char msgEmpacotada[tamEnv+OVERLOAD_SIZE];
								empacotaMsg(arqTam, msgEmpacotada, TAM, sequencia, tamEnv);
								printf("Enviando tamanho %s\n", arqTam);
								fflush(stdout);
								write(soquete, msgEmpacotada, (tamEnv+OVERLOAD_SIZE));
								sequencia = aumentaSeq(sequencia);
								while(true){
									//TODO fazer timeout do TAM como no T2
									read(soquete, msgRec, MSG_SIZE);
									int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
									//aguarda OK
									if(tipo == OK){
										//TODO Atualiza timeout
										printf("OK para escrever, memoria suficiente.\n");
										enviaArquivo(nomeArq, soquete, tam_arquivo, &sequencia, DADO);
										//TODO deletar arquivo desta maquina
										break;
									}
									//se NACK, reenvia msg
									else if(tipo == NACK){
										write(soquete, msgEmpacotada, (tamEnv+OVERLOAD_SIZE));
										//TODO Atualiza timeout
									}
									//se ERRO, printa erro
									else if(tipo == ERRO){
										//unico erro que a mensagem de tam pode gerar eh de espaco
										if(strcmp(dataRec, NAO_ESPACO)){
											printf("ERRO NO SERVIDOR: Espaço insuficiente.\n");
										}
										break;
									}
								}
							}
						}else{
							empacotaMsg(NAO_EXISTE, msgEnviar, ERRO, seqRec, sizeof(NAO_EXISTE));
							printf("nao existe: %s\n", subs[1]);
							fflush(stdout);
							write(soquete, msgEnviar, sizeof(NAO_EXISTE)+OVERLOAD_SIZE);
						}
					}else{
						empacotaMsg(NAO_PERMITIDO, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
						printf("nao permitido: %s\n", msgEnviar);
						fflush(stdout);
						write(soquete, msgEnviar, sizeof(NAO_PERMITIDO)+OVERLOAD_SIZE);
					}
					break;
				case 9: //put
					printf("Recebi um put: %s\n", dataRec);
					fflush(stdout);
					/*getting the first substring*/
					input = strtok(dataRec, " \n");
					/*walking trough the other substrings*/
					i = 0;
					while(input != NULL){
						subs[i] = input;
						input = strtok(NULL, " \n");
						i = i+1;
					}
					strcpy(nomeArq, subs[1]);
					//checa se pode escrever neste diretorio
					if(access("./", W_OK) == 0){
						//Responde com OK
						empacotaMsg("", msgEnviar, OK, seqRec, 0);
						printf("pode escrever neste diretorio.\n");
						fflush(stdout);
						write(soquete, msgEnviar, OVERLOAD_SIZE);
						//se foi um OK, recebe o tamanho do arquivo
						while(true){
							//recebe mensagem
							int r = read(soquete, msgRec, MSG_SIZE);
							//desempacota mensagem
							strcpy(dataRec, "");
							int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
							//se inicio nao confere soh ignora a mensagem
							//se paridade nao confere envia nack
							if(status == -2){
								//empacota NACK, SEQ = NUMERO DE SEQ DA MENSAGEM RECEBIDA
								printf("Mensagem recebida com paridade incoreta, enviando nack.\n");
								empacotaMsg("", msgEnviar, NACK, seqRec, 0);
								write(soquete, msgEnviar, OVERLOAD_SIZE);
								sequencia = aumentaSeq(sequencia);
							}
							else if(status == 0){
								if(checaMemoria("./", dataRec) == 0){
									//se tem memoria suficiente, responde com ACK
									empacotaMsg("", msgEnviar, OK, seqRec, 0);
									dataRec[tamRec] = 0x00;
									printf("tem espaco para escrever arq de tam: %s\n", dataRec);
									fflush(stdout);
									write(soquete, msgEnviar, OVERLOAD_SIZE);
									recebeArquivo(nomeArq, soquete, atoll(dataRec), DADO);
								}
								else{
									//se nao tem memoria suficiente, responde com ERRO
									empacotaMsg(NAO_ESPACO, msgEnviar, ERRO, seqRec, sizeof(NAO_ESPACO));
									printf("nao tem espaco para escrever arq de tam: %s\n", msgEnviar);
									fflush(stdout);
									write(soquete, msgEnviar, sizeof(NAO_ESPACO)+OVERLOAD_SIZE);
								}
								break;
							}
						}
					}
					//ERRO(se nao tem permissao de escrita)
					else{
						empacotaMsg(NAO_PERMITIDO, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
						printf("nao permitido: %s\n", msgEnviar);
						fflush(stdout);
						write(soquete, msgEnviar, sizeof(NAO_PERMITIDO)+OVERLOAD_SIZE);
					}
					break;
				default:
					printf("ERRO: o tipo de comando não confere\n");
			}
		}
	}
}
