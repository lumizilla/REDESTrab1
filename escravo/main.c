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
	while(true){
		//recebe mensagem
		int r = read(soquete, msgRec, MSG_SIZE);
		//desempacota mensagem
		int status = desempacotaMsg(msgRec, dataRec, &seqRec, &tamRec, &tipo);
		//se retornou 0, nao houve erro, se retornou -1 inicio nao confere, 
		//-2 paridade não confere.
		//se inicio nao confere soh ignora a mensagem
		//confere se paridade incorreta responder com NACK
		if(status == -2){
			//empacota NACK, SEQ = NUMERO DE SEQ DA MENSAGEM RECEBIDA
			printf("Mensagem recebida com paridade incoreta \n");
			unsigned char msgEnviar[MSG_SIZE];
			//itoa(seqRec, msg, 10);
			empacotaMsg("", msgEnviar, NACK, seqRec, 0);
			printf("%s\n", msgEnviar);
			fflush(stdout);
			write(soquete, msgEnviar, OVERLOAD_SIZE); 
			sequencia = aumentaSeq(sequencia);
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
					int i = 0;	
					while(input != NULL){
						subs[i] = input;
						input = strtok(NULL, " \n");
						i = i+1;
					}
					//TODO para o ls, guardar o dir corrente
					int error = mudaDir(subs[1]);
					//responde com ACK
					if(error == 0){
						unsigned char msgEnviar[MSG_SIZE];
						empacotaMsg("", msgEnviar, OK, seqRec, 0);
						printf("%s\n", msgEnviar);
						fflush(stdout);
						write(soquete, msgEnviar, OVERLOAD_SIZE); 
					}
					//Se ERRO responde com o cod do erro
					else{
						unsigned char msgEnviar[MSG_SIZE];
						if(error == EACCES){			
							empacotaMsg(NAO_PERMITIDO, msgEnviar, ERRO, seqRec, sizeof(NAO_PERMITIDO));
							printf("nao permitido %s\n", msgEnviar);
							fflush(stdout);
							write(soquete, msgEnviar, sizeof(NAO_PERMITIDO)+OVERLOAD_SIZE); 
						}
						else{
							empacotaMsg(NAO_EXISTE, msgEnviar, ERRO, seqRec, sizeof(NAO_EXISTE));
							printf("nao existe %s\n", msgEnviar);
							fflush(stdout);
							write(soquete, msgEnviar, sizeof(NAO_EXISTE)+OVERLOAD_SIZE); 
						}
					}
					break;
				case 7: //ls
					//TODO Realiza o comando ls, responde com ACK, Envia dados vindos do ls
					break;
				case 8: //get
					//TODO Responde com ACK/ERRO, se foi um ACK enviar o TAM do arquivo e os dados e o OK
					break;
				case 9: //put
					printf("Recebi um put: %s\n", msgRec);
					//TODO Responde com ACK/ERRO(se nao tem permissao de escrita)
						//se foi um ACK, recebe o tamanho do arquivo 
							//se tem memoria suficiente, responde com ACK
								//recebe os dados e os salva
							//se nao tem memoria suficiente, responde com ERRO
					break;
				default:
					printf("ERRO: o tipo da mensagem não confere\n");
			}
		}	
	}	
}

