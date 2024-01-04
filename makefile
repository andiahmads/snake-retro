build:
	clang++ main.cpp -o snake -std=c++11 -I/opt/local/include -L/opt/local/lib -lraylib
	./snake


clean:
	rm -f snake
