all: 
	echo "Compilando mestre..."; cd mestre; gcc  main.c -o mestre -lm; cd ..; \
        echo "Compilando escravo..."; cd escravo; gcc  main.c -o escravo -lm; cd ..	

clean:
	echo "Removendo arquivos inuteis..."; \
	rm mestre/mestre; \
	rm escravo/escravo; echo "Diretorio limpinho!"
