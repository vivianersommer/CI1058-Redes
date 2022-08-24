	PROG   = main

	    CC = gcc 
        OBJS = main.o servidor.o cliente.o conexao.o

.PHONY: all debug clean limpa purge faxina
	
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@ -g

$(PROG) : % : $(OBJS) %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS) -g
	@rm -f *~ *.o


clean limpa:
	@echo "Limpando ...."
	@rm -f *~ *.bak *.tmp

purge faxina:   clean
	@echo "Faxina ...."
	@rm -f  $(PROG) $(PROG_AUX) *.o $(OBJS) core a.out
	@rm -f *.png marker.out *.log