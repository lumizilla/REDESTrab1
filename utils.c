//neste arquivo estao as funcoes de utilidade que podem ser usadas
//tanto pelo cliente quanto pelo servidor

#include "defines.h"

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
int aumentaSeq (int seq){
	if(seq == SEQ_MAX){
		return 0;
	}
	int aux = seq + 1;
	return aux;
}

int empacotaMsg(char *msg, char *msgEnviar, short tipo, short seq, short tam){
	printf("dentro de empacota\n");
	fflush(stdout);
	controle ctrl;
	//um unsigned short ou short then 2 bytes
	ctrl.tam = (unsigned short) tam;
	ctrl.seq = (unsigned short) seq;
	ctrl.tipo = (unsigned short) tipo;
	//EMPACOTA A MSG
        //Formato: INICIO | TAMANHO | SEQUENCIA | TIPO | DADOS | PARIDADE
	//Inicio tem 8 bits
	msgEnviar[0] = 0x7E;

        //Tamanho tem 5 bits
        //Sequencia tem 6 bits e vai ser referente ao i
        //Tipo tem 5 bits, portanto somando tam+seq+tipo temos 16 bits, 2 bytes
	//um char = 1 byte, por isso aqui devem ser atribuidos 2 chars
	char *aux = (char*) (&ctrl);
	msgEnviar[1] = aux[0];
	msgEnviar[2] = aux[1];
	
	// empacotar a msg
	for(int i = 3; i < tam+3; i++){
		msgEnviar[i] = msg[i-3];
	}

	//TODO fazer a paridade

        return 0;
}

int desempacotaMsg(char *msgRecebida){
	//TODO Verificar inicio
	//TODO retirar informacao do tamanho
	//TODO verificar se paridade bate

	//TODO retornar dados extraidos
	//TODO retornar tipo da mensagem
	//TODO retornar o numero de sequencia
	return 0;
}
