CC=gcc
CFLAGS=-D FRAMESTORE=$(framesize) -D VARMEMSIZE=$(varmemsize)
C_FILES=shell.c interpreter.c varstore.c scheduler.c pagetbl.c codestore.c pcb.c readyqueue.c accessrecord.c
O_FILES=shell.o interpreter.o varstore.o scheduler.o pagetbl.o codestore.o pcb.o readyqueue.o accessrecord.o

.PHONY: files clean

mysh: $(C_FILES)
	$(CC) $(CFLAGS) -c $^
	$(CC) $(CFLAGS) -o mysh $(O_FILES)
	
debug: $(C_FILES)
	$(CC) $(CFLAGS) -c -g3 -O0 $^
	$(CC) $(CFLAGS) -o mysh $(O_FILES)

clean: 
	rm mysh; rm *.o
