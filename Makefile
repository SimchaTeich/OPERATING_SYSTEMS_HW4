all: react server

react: st_reactor.c
	gcc -c -Wall -Werror -fpic st_reactor.c
	gcc -shared -o libst_reactor.so st_reactor.o

server: react_server.c
	gcc -Wall -o react_server react_server.c -L. -l st_reactor

clean:
	rm react_server st_reactor.o libst_reactor.so 
