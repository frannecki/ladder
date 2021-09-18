CC=g++
PROTO=protoc
CFLAGS=-Wall -std=c++11 -O0 -g
TGT=test.out
PROJECT_PATH=./../ladder

INCLUDE += -I$(PROJECT_PATH)/include/
OBJ += $(patsubst %.cpp,%.o,$(shell find . -maxdepth 1 -name '*.cpp' -type f))
OBJ += $(patsubst %.proto,%.pb.cc,$(shell find ./proto -maxdepth 1 -name '*.proto' -type f))
LIB += -L/usr/local/lib -lpthread -L$(PROJECT_PATH)/build -lladder
LIB += -L/usr/local/lib64 -lgtest_main -lgmock_main -lgtest -lgmock -lprotobuf

ALL: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^ $(LIB)

%pb.cc: %proto
	$(PROTO) --proto_path=./proto  --cpp_out=./proto $^

clean:
	rm -r $(TGT)
	rm $(OBJ)
