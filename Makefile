all: 
	echo "Compilando mestre..."; cd mestre; gcc main.c -o mestre; cd ..; \
        echo "Compilando escravo..."; cd escravo; gcc main.c -o escravo; cd ..	

clean:
	echo "Removendo arquivos inuteis..."; \
	rm mestre/mestre; \
	rm escravo/escravo; echo "Diretorio limpinho!"
