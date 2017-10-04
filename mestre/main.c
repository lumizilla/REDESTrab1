#include "../utils.c"

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
        int soquete = ConexaoRawSocket("eno1");
	unsigned char comando[DATA_SIZE];

	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando, sizeof(comando), stdin);
		//TODO se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		//TODO empacotar mensagem no formato correto
		//enviar mensagem ao servidor
		write(soquete, comando, MSG_SIZE);
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
