#include "../utils.c"
//um char tem 1 byte = 8 bits

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
        
	int soquete = ConexaoRawSocket("eno1");
	unsigned char comando[DATA_SIZE];
	char *comando_usuario;
	char *subs[3];
	unsigned char mensagens[MAX_MSG][DATA_SIZE];
	char *tipo = "";
	int sequencia = 0;

	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando_usuario, MAX_INPUT, stdin);
		//separando o comando em substrings
		//e limpando possivel lixos
		tipo = ""
		subs[0] = "";
		subs[1] = "";
		subs[2] = "";
		subs = strtok(comando_usuario, " ");
		//se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		if(subs[0] != "cd" && subs[0] != "ls" && subs[0] != "get" && subs[0] != "put"){
			printf("ERRO: comando invalido\n");
		}
		else{
			//TODO empacotar mensagem no formato correto
			tipo = subs[0];

			numMensagens = empacotaMsg(comando_usuario, mensagens, tipo, &sequencia);
			//enviar mensagem ao servidor
			for(i = 0; i < numMensagens; i++){
				write(soquete, mensagens[i], MSG_SIZE);
			}

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
