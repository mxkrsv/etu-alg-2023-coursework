build:
	g++ -g -Wall -Wextra -pedantic static.cpp -o static_huffman

clean:
	rm -f static_huffman

.PHONY: clean
