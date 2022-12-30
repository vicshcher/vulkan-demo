CC=clang++
CXXFLAGS=-g -std=c++20 -Wall -Wextra -Wpedantic -Wconversion
LDFLAGS=-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

all: vk

vk:
	$(CC) $(CXXFLAGS) src/main.cpp -o build/main $(LDFLAGS)

clean:
	rm main
