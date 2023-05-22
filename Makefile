all: st_react react_server

st_react: st_reactor.c
	gcc -c -Wall -Werror -fpic st_reactor.c
	gcc -shared -o st_reactor.so st_reactor.o

react_server: react_server.o st_reactor.so
	gcc -Wall -o $@ $< ./st_reactor.so -pthread

clean:
	rm react_server.o st_reactor.o st_reactor.so react_server
