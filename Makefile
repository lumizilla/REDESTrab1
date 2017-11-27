all: 
	echo "Compilando mestre..."; cd mestre; gcc -g -std=gnu99 main.c -o mestre -lm; cd ..; \
        echo "Compilando escravo..."; cd escravo; gcc -std=gnu99 main.c -o escravo -lm; cd ..	

clean:
	echo "Removendo arquivos inuteis..."; \
	rm mestre/mestre; \
	rm escravo/escravo; echo "Diretorio limpinho!"
