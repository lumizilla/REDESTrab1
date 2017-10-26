#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/ioctl.h> 
#include <net/ethernet.h> 
#include <linux/if_packet.h> 
#include <linux/if.h> 
#include <netinet/in.h>
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include "utils.c"

//esse eh o tamanho maximo de uma mensagem em bytes, 
//considerando que tenham 31 bytes de dados
//e mais 4 bytes pra outras coisas. 
#define MSG_SIZE 35

//tamanho maximo do campo de dados
#define DATA_SIZE 31

//tamanho maximo do numero de sequencia
#define SEQ_MAX 200

typedef struct controle {
	unsigned short tam : 5,
		seq : 6,
		tipo : 5;
}controle;
