CC=g++
CFLAGS=-std=c++11 -O0 -g -fPIC -DDEBUG
TGT_PATH=build
TGT=$(TGT_PATH)/libladder.so

INCLUDE += -Iinclude/
OBJ += $(patsubst %.cpp,%.o,$(find src -name *.cpp -maxdepth 1))
LIB += -L/usr/local/lib -lpthread

ALL: $(TGT)

$(TGT): $(OBJ)
	mkdir -p $(TGT_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $^

clean:
	rm -r $TGT_PATH
	rm src/*.o
