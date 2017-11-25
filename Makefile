CC=			g++
CFLAGS=		-g -gdwarf-2 -Wall -std=c++11
LD=			g++
LDFLAGS=	-L.
TARGETS=	pq

all:		$(TARGETS)

test:
	@echo Testing ...
	@[ `valgrind --leak-check=full ./pq .` ]

pq: pq.o scheduler.o client.o server.o
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -o $@ $^

pq.o: pq.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^

scheduler: scheduler.o
	@$(CC) $(CFLAGS) -o $@ $^

scheduler.o: scheduler.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^

client: client.o
	@$(CC) $(CFLAGS) -o $@ $^

client.o: client.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^

server: server.o
	@$(CC) $(CFLAGS) -o $@ $^

server.o: server.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	@echo "Cleaning..."
	@rm -f $(TARGETS) *.o *.socke*
