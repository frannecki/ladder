CC=g++
PROTO=protoc
CFLAGS=-Wall -std=c++11 -O0 -g -DDEBUG -fno-omit-frame-pointer
TGT=test_http_server.out
PROJECT_PATH=./../../ladder

INCLUDE += -I$(PROJECT_PATH)/include/ -I./../
OBJ += $(patsubst %.cpp,%.o,$(shell find . -maxdepth 1 -name '*.cpp' -type f))
LIB += -L/usr/local/lib -lpthread -L$(PROJECT_PATH)/build -lladder

ALL: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^ $(LIB)

clean:
	rm -rf $(TGT)
	rm -f $(OBJ)
