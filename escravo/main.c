#include "../utils.c"

int main(){
        printf("\niniciando programa escravo(servidor)!\n");
        int soquete = ConexaoRawSocket("eno1");

	while(true){
		//recebe mensagem
		unsigned char dataRec[MSG_SIZE];
		int r = read(soquete, dataRec, MSG_SIZE);
		printf("%d\n", r);
		printf("%s\n", dataRec);
		//TODO desempacota mensagem
			//confere se mensagem tem inicio 01111110
			//confere o tamanho da mensagem
			//confere qual a sequencia da mensagem
			//confere o tipo da mensagem
			//extrai dados de acordo com tamanho da mensagem
			//atraves da paridade, confere se mensagem eh correta(usar paridade vertical de 8bits)
				//se paridade incorreta responder com nack
		//TODO define qual o tipo de mensagem e opera ela
		//ver detalhes no README
		//TODO cria resposta adequada para cada tipo de mensagem
			//cd
			//ls
			//get
			//put
		//TODO empacota resposta e envia
	}	
}

