CC = g++
CFLAGS = -g -w -fPIC
INCLUDES = -I/usr/local/include
LIBS = -lm -lpthread -ldl -lpfm

OBJ = normal
SRC = main.c example.c

all: 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OBJ) $(SRC) $(LIBS)

.PHONY: clean

clean:
	rm normal
 
