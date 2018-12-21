im:
	gcc client.c -o client.o -lpthread -ldl
	gcc server.c -o server.o -lpthread 
clean:
	rm *.o

