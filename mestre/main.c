#include "../utils.c"
//um char tem 1 byte = 8 bits

int empacotaMsg(char *msg, char **mensagens, char *tipo){
	//Formato: INICIO | TAMANHO | SEQUENCIA | TIPO | DADOS | PARIDADE
	tamMsg = sizeof(*msg);
	for(i = 0; i < ceil(tamMsg/DATA_SIZE); i++){
		//EMPACOTAR AS MSGS
		//Inicio tem 8 bits
		mensagens[i][0] = 0x7E;
		//Tamanho tem 5 bits
		//Sequencia tem 6 bits e vai ser referente ao i
		//Tipo tem 5 bits, portanto somando tam+seq+tipo temos 16 bits, 2 bytes
		//TODO mensagem.h e mensagem.c do trabalho da marcela
	}
	return ceil(tamMsg/DATA_SIZE);
}

int main(){
        printf("\niniciando programa mestre(cliente)!\n");        
        int soquete = ConexaoRawSocket("eno1");
	unsigned char comando[DATA_SIZE];
	char *comando_usuario;
	char *subs[3];
	unsigned char mensagens[MAX_MSG][DATA_SIZE];
	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando_usuario, MAX_INPUT, stdin);
		//separando o comando em substrings
		//e limpando possivel lixo
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
			numMensagens = empacotaMsg(comando_usuario, mensagens);
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
