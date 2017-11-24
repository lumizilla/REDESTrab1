#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/ioctl.h> 
#include <sys/stat.h>
#include <sys/statvfs.h>
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
#include <math.h>
#include <errno.h>

//esse eh o tamanho maximo de uma mensagem em bytes, 
//considerando que tenham 31 bytes de dados
//e mais 4 bytes pra outras coisas. 
#define MSG_SIZE 35

//tamanho maximo do campo de dados
#define DATA_SIZE 31

//tamanho do overload das mensagens
#define OVERLOAD_SIZE 4 

//tamanho maximo do numero de sequencia
#define SEQ_MAX 200

//tamanho maximo do comando de input do mestre
#define MAX_INPUT 200

//tamanho maximo do path em termos do numero de diretorios andados
#define MAX_PATH 30

//tamanho maximo do nome de um diretorio com a /
#define MAX_DIR 31

//tamanho maximo para nome de um arquivo a ser enviado
#define FILE_NAME 50

//tipos das mensagens
#define ACK 0
#define TAM 2
#define OK 3
#define CD 6
#define LS 7
#define GET 8
#define PUT 9
#define FIM 10
#define MOSTRA 12
#define DADO 13
#define ERRO 14
#define NACK 15

//erros
#define NAO_EXISTE "1"
#define NAO_PERMITIDO "2"
#define NAO_ESPACO "3"

typedef struct cont {
	unsigned short tam : 5,
		seq : 6,
		tipo : 5;
}controle;
