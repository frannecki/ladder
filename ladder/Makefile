CC=g++
CFLAGS=-Wall -std=c++11 -O0 -g -fPIC -DDEBUG -fno-omit-frame-pointer
TGT_PATH=build
TGT=$(TGT_PATH)/libladder.so

INCLUDE += -Iinclude/
OBJ += $(patsubst %.cpp,%.o,$(shell find src -maxdepth 2 -name '*.cpp' -type f))
LIB += -L/usr/local/lib -lpthread -lprotobuf -lz -lcrypto -lssl

ALL: $(TGT)

$(TGT): $(OBJ)
	mkdir -p $(TGT_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -shared -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -c -o $@ $^

clean:
	rm -rf $(TGT_PATH)
	rm -f $(OBJ)
