# REDESTrab1

## *Implementar um FTP simplificado entre duas máquinas usando RawSockets*

Este repositorio eh para guardar o primeiro trabalho da disciplina de REDES 1 \
Com muita gentileza, um colega me deu o link do trabalho dele para servir de inspiracao:
https://gitlab.c3sl.ufpr.br/lfo14/t1_redes1

outro trabalho, do vytor calixto:
https://gitlab.c3sl.ufpr.br/vsbc14/Caco

o melhor e mais comentado, da marcela:
https://gitlab.c3sl.ufpr.br/mro15/ftp-simplificado

### Entrega:

- Entrega na ultima semana de aula em uma terca feira 
- Relatorio impresso com as escolhar feitas na implementacao 
- Codigo por email apos a apresentacao 
- Deve ser feito em C ou C++

## Fazer isso aqui depois de bootar:
```
sudo apt-get update; sudo apt-get install git -y; sudo apt-get install build-essential -y; sudo apt-get install gedit -y; sudo apt-get install tree -y; git clone https://github.com/lumizila/REDESTrab1.git; git config --global user.email "lumiwille@gmail.com"; git config --global user.name "Luiza"; cd REDESTrab1;
```
## RODAR AS COISAS COM SUDO

### Descricao do trabalho desenvolvida em sala de aula:

Devem haver: 1 mestre e 1 escravo. \
*Mestre:* O mestre é onde um usuário vai digitar os comandos. \
*Escravo:* O escravo corresponde ao servidor, que atende os comandos dados pelo Mestre.

As mensagens transmitidas deverão ter o seguinte formato:

| Inicio        | Tamanho       | Sequencia     | Tipo          | Dados                      | Paridade      |
| ------------- | ------------- | ------------- | ------------- | -------------------------- | ------------- |
| 8 bits        | 5 bits        | 6 bits        | 5 bits        | 0⁻31 bytes                 | 8 bits        |

Considerações sobre cada campo de mansagens: \
O campo *inicio* é composto pela sequência: 01111110. O fim das mensagens deve ser presvisto atraves do campo tamanho. \
O campo *tamanho* vai ter um numero entre 0 e 31 bytes que fazem referência somente ao campo de dados. \
O campo *sequencia* começa do 0, quando a sequência chega no máximo ela volta à 0. \
O campo *tipo* deve seguir a seguinte tabela (os codigos com descricao em branco nao foram atribuidos a nada): 

| Tipo de mensagem | Descricao |
| ---------------- | --------- |
| 0 | ACK |
| 1 | |
| 2 | Tamanho do arquivo |
| 3 | Ok, consegui |
| 4 | |
| 5 | |
| 6 | cd |
| 7 | ls |
| 8 | get |
| 9 | put |
| A | FIM |
| B | |
| C | Mostra na tela |
| D | Dados |
| E | Erro |
| F | NACK |

O campo *dados* eh onde estarao os dados realmente uteis. Ver a explicacao do controle de fluxo abaixo. \
O campo *paridade* faz referencia aos campos: Tamanho, sequencia, tipo e dados.

### Especificacao dos comandos cd, ls, get, put e dos codigos de erro.

#### cd 
![cd](imagens/cd.jpg?raw=true "Optional Title") 

#### ls 
![ls](imagens/ls.jpg?raw=true "Optional Title")

#### get 
![get](imagens/get.jpg?raw=true "Optional Title")

#### put 
![put](imagens/put.jpg?raw=true "Optional Title")

#### codigos de erro

- 1: diretorio ou arquivo inexistente 
- 2: acesso(permissao)
- 3: espaço insuficiente

### Outras consideracoes

O LLC(logical link control) é composto por: \
-Enquadramento (Inicio: campo inicio; Fim: campo tamanho) \
-Sequencializacao (campo sequencia) \
-Controle de fluxo (Apenas para dados, usa janela deslizante de tamanho 3 e repeticao seletiva) \
-Deteccao de erros (Usa paridade vertical de 8 bits) 

Protocolo: deve seguir o protocolo baseado no Kermit

##### TIMEOUT EH OBRIGATORIO

##### Cuidar com envio de arquivos binarios !! Testar isso !



