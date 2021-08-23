CC=g++
CFLAGS=-Wall -std=c++11 -O0 -g
TGT=test.out
PROJECT_PATH=./../ladder

INCLUDE += -I$(PROJECT_PATH)/include/
OBJ += $(patsubst %.cpp,%.o,$(shell find . -maxdepth 1 -name *.cpp -type f))
LIB += -L/usr/local/lib -lpthread -L$(PROJECT_PATH)/build -lladder
LIB += -L/usr/local/lib64 -lgtest_main -lgmock_main -lgtest -lgmock

ALL: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^ $(LIB)

clean:
	rm -r $(TGT)
	rm $(OBJ)

