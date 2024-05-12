FILES=./src/vm_priority_queue/vm_priority_queue.c ./src/mm.c 
CC = gcc
CFLAGS = -g -r

all: build_lib ./main.c
	mkdir -p bin
	$(CC) $(CFLAGS) ./main.c -o ./build/main.o
	$(CC) ./build/mm.o ./build/vm_priority_queue.o ./build/main.o -o ./bin/app.out

build_lib: $(FILES)
	mkdir -p build
	$(CC) $(CFLAGS) ./src/mm.c -o ./build/mm.o
	$(CC) $(CFLAGS) ./src/vm_priority_queue/vm_priority_queue.c -o ./build/vm_priority_queue.o

./src/mm.c : ./src/mm.h
./src/vm_priority_queue/vm_priority_queue.c : ./src/vm_priority_queue/vm_priority_queue.h

run : ./bin/app.out
	./bin/app.out


clean:
	rm -rf ./build
	rm -rf ./bin



