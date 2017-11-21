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
			//empacota NACK, NO CAMPO DE DADOS O NUMERO DE SEQ DA MENSAGEM RECEBIDA
			printf("Mensagem recebida com paidade incoreta \n");
			unsigned char msgEnviar[MSG_SIZE];
			unsigned char msg[DATA_SIZE];
			//itoa(seqRec, msg, 10);
			snprintf(msg, sizeof(msg), "%d", seqRec);			
			empacotaMsg(msg, msgEnviar, NACK, sequencia, DATA_SIZE);
			printf("%s\n", msgEnviar);
			fflush(stdout);
			write(soquete, msgEnviar, MSG_SIZE); 
			sequencia = aumentaSeq(sequencia);
		}
		//confere o tamanho da mensagem
		//confere qual a sequencia da mensagem
		if(status == 0){
			//TODO cria resposta adequada para cada tipo de mensagem
			printf("%d\n", r);
			printf("%s\n", dataRec);
			switch(tipo){
				case 6: //cd
					printf("Recebi um cd: %s\n", msgRec);
					//TODO Realiza a troca de diretório e responde com ACK
					//TODO Se ERRO responde com o cod do erro
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

