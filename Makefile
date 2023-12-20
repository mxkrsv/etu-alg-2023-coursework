all: dynamic static

dynamic: dynamic.cpp
	g++ -g -Wall -Wextra -pedantic dynamic.cpp -o dynamic_huffman

static: static.cpp
	g++ -g -Wall -Wextra -pedantic static.cpp -o static_huffman

clean:
	rm -f static_huffman dynamic_huffman

.PHONY: clean
