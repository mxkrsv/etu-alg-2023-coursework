build:
	g++ -Wall -Wextra -pedantic main.cpp -o huffman

clean:
	rm -f huffman

.PHONY: clean
