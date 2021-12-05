CC=g++
PROTO=protoc
CFLAGS=-Wall -std=c++11 -O0 -g
TGT=test.out
PROJECT_PATH=./../../ladder

INCLUDE += -I$(PROJECT_PATH)/include/ -I./../
OBJ += $(patsubst %.cpp,%.o,$(shell find . -maxdepth 2 -name '*.cpp' -type f))
OBJ += $(patsubst %.proto,%.pb.cc,$(shell find ./../proto -maxdepth 1 -name '*.proto' -type f))
LIB += -L/usr/local/lib -lpthread -L$(PROJECT_PATH)/build -lladder
LIB += -L/usr/local/lib64 -lprotobuf -lz -lssl

ALL: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^ $(LIB)

%pb.cc: %proto
	$(PROTO) -I=../proto  --cpp_out=../proto $^

clean:
	rm -rf $(TGT)
	rm -f $(OBJ)
