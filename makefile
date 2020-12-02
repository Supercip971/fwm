
CFILES := $(shell find src -type f -name \*.cpp)
CCFILES := $(shell find src -type f -name \*.c)
HFILES := $(shell find src -type f -name \*.h)

OBJFILES := $(patsubst %.cpp, %.o, $(CFILES)) $(patsubst %.c, %.o, $(CCFILES))
LIBX11_PATH := /usr/lib/libX11.so
OUTPUT := "./build/fwm"
%.o: %.cpp $(CFILES) $(HFILES)
	@echo "[ C++] building $< "
	@g++ -c -pthread -fpermissive -fsanitize=address -Wall -Werror -fsanitize=undefined -std=c++20 -m64 -Wall -g -march=x86-64 -I src/ -O3 -msse -mavx -o $@ $<
%.o: %.c $(CCFILES) $(HFILES)
	@echo "[ C ] building $<"
	@gcc -c -pthread -Wall -Werror -m64 -Wall -g -march=x86-64 -I src/ -O3 -msse -mavx -o $@ $<


$(OUTPUT): $(OBJFILES)
	@echo "all"
	@g++ $(OBJFILES) $(LIBX11_PATH) -fpermissive -fsanitize=address -fsanitize=undefined -pthread -o build/fwm



XEPHYR :=$(shell whereis -b Xephyr | cut -f2 -d' ')
run:
	xinit ./.xinit_test -- $(XEPHYR) :100 -ac -screen 1080x720 -host-cursor
run_test:
	./build/fwm $(TST_FILES)
	cat output

test:
	@echo "not implemented for the moment"

format: 
	clang-format -i --style=file $(CFILES) $(HFILES)


clean:
	rm $(OBJFILES)
	rm ./build/fwm


all: $(OUTPUT) 
	@make run




