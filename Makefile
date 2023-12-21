debug: dynamic static

release: dynamic_release static_release

dynamic_release: dynamic.cpp
	g++ -g -O2 dynamic.cpp -o dynamic_huffman

dynamic: dynamic.cpp
	g++ -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -pedantic dynamic.cpp -o dynamic_huffman

static_release: static.cpp
	g++ -g -O2 static.cpp -o static_huffman

static: static.cpp
	g++ -g -Wall -Wextra -pedantic static.cpp -o static_huffman

clean:
	rm -f static_huffman dynamic_huffman

.PHONY: clean
