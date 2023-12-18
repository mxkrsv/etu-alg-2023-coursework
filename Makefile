build:
	g++ -g -Wall -Wextra -pedantic static.cpp -o static_huffman
	g++ -g -Wall -Wextra -pedantic dynamic.cpp -o dynamic_huffman

clean:
	rm -f static_huffman dynamic_huffman

.PHONY: clean
