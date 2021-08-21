CC=g++
CFLAGS=-std=c++11 -O0 -g
TGT=test
PROJECT_PATH=./../ladder

INCLUDE += -I$(PROJECT_PATH)/include/
OBJ += $(patsubst %.cpp,%.o,$(find . -name *.cpp -maxdepth 1))
LIB += -L/usr/local/lib -lpthread -L$(PROJECT_PATH)/build -lladder

ALL: $(TGT)

$(TGT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^

clean:
	rm -r $(TGT)
	rm src/*.o

