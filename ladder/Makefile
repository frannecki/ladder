CC=g++
CFLAGS=-Wall -std=c++11 -O0 -g -fPIC -DDEBUG
TGT_PATH=build
TGT=$(TGT_PATH)/libladder.so

INCLUDE += -Iinclude/
OBJ += $(patsubst %.cpp,%.o,$(shell find src -name *.cpp -type f -maxdepth 1))
LIB += -L/usr/local/lib -lpthread

ALL: $(TGT)

$(TGT): $(OBJ)
	mkdir -p $(TGT_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -shared -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -c -o $@ $^

clean:
	rm -r $(TGT_PATH)
	rm $(OBJ)
