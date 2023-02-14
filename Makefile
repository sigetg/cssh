all: cssh 

cssh: cssh.o pid_list.o pid_list.h
	gcc -std=c99 -Wall -g -o cssh cssh.o pid_list.o

cssh.o: cssh.c pid_list.h
	gcc -std=c99 -Wall -g -c cssh.c

pid_list.o: pid_list.c pid_list.h
	gcc -std=c99 -Wall -g -c pid_list.c

clean:
	rm -rf cssh cssh.o pid_list.o
