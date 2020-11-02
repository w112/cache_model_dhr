OBJS=CacheSim.o test.o attack.o
CC=g++
CFLAGS=-c -g
TARGET=test

$(TARGET):$(OBJS)
	$(CC) $^ -o $@

%.o:%.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -rf *.o test
