# REDESTrab1

Este repositorio eh para guardar o primeiro trabalho da disciplina de REDES 1: 

## *Implementar um FTP simplificado entre duas máquinas usando RawSockets*

Com muita gentileza, um colega me deu o link do trabalho dele para servir de inspiracao:
https://gitlab.c3sl.ufpr.br/lfo14/t1_redes1

### *Descricao do trabalho desenvolvida em sala de aula:*

Devem haver: 1 mestre e 1 escravo.

*Mestre:* O mestre é onde um usuário vai digitar os comandos.
*Escravo:* O escravo corresponde ao servidor, que atende os comandos dados pelo Mestre.

As mensagens transmitidas deverão ter o seguinte formato:

| Inicio | Tamanho | Sequencia | Tipo | Dados | Paridade |
| ------------- | ------------- | ------------- | ------------- | -------------------------- | ------------- |

