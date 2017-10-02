#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>

int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, (size_t) sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

int main(){
        printf("\niniciando programa mestre(cliente)!\n");
        
        int soquete = ConexaoRawSocket("eth0");
	
	//TODO: arrumar tamanho maximo do comando e das outras coisas no codigo
	char comando[100];

	while(true){
		//lendo o comando
		printf("Qual o seu comando?\n");
		fgets(comando, sizeof(comando), stdin);
		
		//TODO: se o comando nao atender aos padroes, nao enviar e avisar o usuario qual o padrao
		
		//TODO colocando no formato correto
		
		//enviando o comando
		write(soquete, comando, 67);
		
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
