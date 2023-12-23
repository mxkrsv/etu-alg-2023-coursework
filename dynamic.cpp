#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <unistd.h>

//#define DEBUG

#ifdef DEBUG
#define DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...) ;
#endif

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define USAGE "Usage: %s [-i <input_filename>] [-o <output_filename>]"

class VitterTreeNode {
	public:
	bool is_NYT() {
		return this->not_yet_transferred;
	};

	void unset_NYT() {
		assert(this->is_leaf());

		this->not_yet_transferred = false;
	}

	bool is_leaf() {
		return !this->left && !this->right;
	}

	bool is_root() {
		return !this->parent;
	}

	bool of_the_same_kind(VitterTreeNode *other) {
		return this->is_leaf() == other->is_leaf();
	}

	bool is_symb() {
		return this->is_symbol;
	}

	char get_symbol() {
		assert(this->is_symb());

		return this->symbol;
	}

	size_t get_weight() {
		return this->weight;
	}

	void inc_weight() {
		DPRINTF("inc_weight: %hhd = %zu\n", this->symbol,
			this->weight + 1);
		this->weight++;
	}

	uint8_t get_number() {
		return this->number;
	}

	VitterTreeNode *get_parent() {
		return this->parent;
	}

	void set_left_child(VitterTreeNode *child) {
		assert(!this->is_NYT());

		this->left = child;
		child->parent = this;
		child->number = this->number - 2;
	}

	void set_right_child(VitterTreeNode *child) {
		assert(!this->is_NYT());

		this->right = child;
		child->parent = this;
		child->number = this->number - 1;
	}

	bool is_child_of(VitterTreeNode *parent) {
		assert(parent);

		return this->parent == parent;
	}

	void swap_with(VitterTreeNode *other);

	VitterTreeNode *
	walk_extended(std::function<bool(VitterTreeNode *)> matches,
		      std::function<void(VitterTreeNode *)> before_left_child,
		      std::function<void(VitterTreeNode *)> after_left_child,
		      std::function<void(VitterTreeNode *)> before_right_child,
		      std::function<void(VitterTreeNode *)> after_right_child);

	VitterTreeNode *walk(std::function<bool(VitterTreeNode *)> matches) {
		return this->walk_extended(
			matches, [](auto) {}, [](auto) {}, [](auto) {},
			[](auto) {});
	};

	VitterTreeNode(bool NYT)
	    : left(nullptr), right(nullptr), parent(nullptr), symbol(0),
	      weight(0), number(UINT8_MAX), not_yet_transferred(NYT),
	      is_symbol(false) {
	}

	VitterTreeNode(char symbol)
	    : left(nullptr), right(nullptr), parent(nullptr), symbol(symbol),
	      weight(0), number(0), not_yet_transferred(false),
	      is_symbol(true) {
	}

	~VitterTreeNode() {
		if (this->left) {
			delete this->left;
		}
		if (this->right) {
			delete this->right;
		}
	}

	private:
	VitterTreeNode *left;
	VitterTreeNode *right;
	VitterTreeNode *parent;

	char symbol;
	// Number of occurrences
	size_t weight;
	// Implicit node number
	uint8_t number = UINT8_MAX;

	bool not_yet_transferred;
	bool is_symbol;
};

void VitterTreeNode::swap_with(VitterTreeNode *other) {
	if (this->parent) {
		if (this->parent->left == this) {
			this->parent->left = other;
		} else {
			this->parent->right = other;
		}
	}

	if (other->parent) {
		if (other->parent->left == other) {
			other->parent->left = this;
		} else {
			other->parent->right = this;
		}
	}

	VitterTreeNode *tmp = other->parent;
	other->parent = this->parent;
	this->parent = tmp;

	tmp = other->right;
	other->right = this->right;
	this->right = tmp;

	tmp = other->left;
	other->left = this->left;
	this->left = tmp;

	uint8_t tmp_number = other->number;
	other->number = this->number;
	this->number = tmp_number;
}

VitterTreeNode *VitterTreeNode::walk_extended(
	std::function<bool(VitterTreeNode *)> matches,
	std::function<void(VitterTreeNode *)> before_left_child,
	std::function<void(VitterTreeNode *)> after_left_child,
	std::function<void(VitterTreeNode *)> before_right_child,
	std::function<void(VitterTreeNode *)> after_right_child) {

	VitterTreeNode *ret;

	if (this->left) {
		before_left_child(this);
		ret = this->left->walk_extended(
			matches, before_left_child, after_left_child,
			before_right_child, after_right_child);
		if (ret) {
			return ret;
		}
		after_left_child(this);
	}

	if (matches(this)) {
		return this;
	};

	if (this->right) {
		before_right_child(this);
		ret = this->right->walk_extended(
			matches, before_left_child, after_left_child,
			before_right_child, after_right_child);
		if (ret) {
			return ret;
		}
		after_right_child(this);
	}

	return nullptr;
}

class BitList {
	public:
	size_t get_length() {
		return this->length;
	}

	void push_back(bool bit) {
		if (length == capacity) {
			this->list = (bool *)reallocarray(
				this->list, this->capacity * 2, sizeof(bool));
			this->capacity *= 2;
		}

		this->length++;
		this->list[this->length - 1] = bit;
	}

	void pop_back() {
		this->length--;
	};

	bool operator[](size_t idx) {
		assert(idx < length);

		return this->list[idx];
	}

	BitList &operator=(const BitList &other) {
		assert(this->list);
		free(this->list);

		this->list = (bool *)malloc(sizeof(bool) * other.capacity);
		assert(this->list);

		memcpy(this->list, other.list, other.length * sizeof(bool));
		this->length = other.length;
		this->capacity = other.capacity;

		return *this;
	}

	char *get_string() {
		char *ret = (char *)malloc(sizeof(char) * (this->length + 1));

		for (size_t i = 0; i < length; i++) {
			ret[i] = list[i] ? '1' : '0';
		}
		ret[this->length] = '\0';

		return ret;
	}

	BitList() : length(0), capacity(8) {
		this->list = (bool *)malloc(sizeof(bool) * 8);
		assert(this->list);
	}

	BitList(const BitList &other) {
		this->list = (bool *)malloc(sizeof(bool) * other.capacity);
		assert(this->list);

		memcpy(this->list, other.list, other.length * sizeof(bool));
		this->length = other.length;
		this->capacity = other.capacity;
	}

	BitList(uint8_t symbol) : BitList() {
		for (size_t i = 0; i < 8; i++) {
			this->push_back(symbol & (1U << (8 - i - 1)));
		}

		assert(this->length == 8);
	}

	~BitList() {
		free(list);
	}

	private:
	bool *list;
	size_t length;
	size_t capacity;
};

class VitterTree {
	public:
	VitterTreeNode *search_symbol(char symbol);
	VitterTreeNode *search_number(uint8_t number);

	bool insert(char);

	BitList
	get_huffman_code(std::function<bool(VitterTreeNode *)> match_fn);
	BitList get_huffman_code_for_char(char symbol);

	BitList get_prev_NYT_code() {
		return this->prev_NYT_code;
	};

	BitList get_prev_symbol_code() {
		return this->prev_symbol_code;
	};

	VitterTree() : prev_NYT_code(), prev_symbol_code() {
		root = new VitterTreeNode(true);
		NYT = root;
	}

	~VitterTree() {
		delete root;
	}

	private:
	VitterTreeNode *root;
	VitterTreeNode *NYT;
	BitList prev_NYT_code;
	BitList prev_symbol_code;

	VitterTreeNode *find_block_leader(VitterTreeNode *block_entry);
	BitList get_huffman_code_for_NYT();
};

VitterTreeNode *VitterTree::search_symbol(char symbol) {
	return this->root->walk([symbol](VitterTreeNode *node) -> bool {
		DPRINTF("search_symbol(%d, %d) = %d\n",
			node->is_symb() ? node->get_symbol() : -1, symbol,
			node->is_symb() && node->get_symbol() == symbol);
		return node->is_symb() && node->get_symbol() == symbol;
	});
}

VitterTreeNode *VitterTree::search_number(uint8_t number) {
	return this->root->walk([number](VitterTreeNode *node) -> bool {
		return node->get_number() == number;
	});
}

// TODO: very ineffective because it's called every single time
BitList
VitterTree::get_huffman_code(std::function<bool(VitterTreeNode *)> match_fn) {
	BitList huffman_code;

	this->root->walk_extended(
		match_fn,
		[&huffman_code](auto) {
			DPRINTF("get_huffman_code: pushed false\n");
			huffman_code.push_back(false);
		},
		[&huffman_code](auto) {
			DPRINTF("get_huffman_code: popped\n");
			huffman_code.pop_back();
		},
		[&huffman_code](auto) {
			DPRINTF("get_huffman_code: pushed true\n");
			huffman_code.push_back(true);
		},
		[&huffman_code](auto) {
			DPRINTF("get_huffman_code: popped\n");
			huffman_code.pop_back();
		});

	return huffman_code;
}

BitList VitterTree::get_huffman_code_for_char(char symbol) {
	return this->get_huffman_code([symbol](VitterTreeNode *node) -> bool {
		if (node->is_symb()) {
			DPRINTF("get_huffman_code_for_char: matches(%d, %d) = "
				"%d\n",
				node->get_symbol(), symbol,
				node->get_symbol() == symbol);
			return node->get_symbol() == symbol;
		}

		return false;
	});
}

BitList VitterTree::get_huffman_code_for_NYT() {
	return this->get_huffman_code(
		[](VitterTreeNode *node) -> bool { return node->is_NYT(); });
}

// TODO: ineffective
VitterTreeNode *VitterTree::find_block_leader(VitterTreeNode *block_entry) {
	VitterTreeNode *result = block_entry;

	for (uint8_t i = block_entry->get_number(); i != 0; i++) {
		VitterTreeNode *current = this->search_number(i);
		assert(current);

		if (current->of_the_same_kind(block_entry) &&
		    current->get_weight() == block_entry->get_weight()) {
			result = current;
		}
	}

	return result;
}

// TODO: verify (source: Russian Wikipedia)
// true -> inserted new, false -> was already present
bool VitterTree::insert(char symbol) {
	VitterTreeNode *search_result = this->search_symbol(symbol);

	VitterTreeNode *current = nullptr;
	bool was_new = false;
	if (!search_result) {
		this->prev_NYT_code = this->get_huffman_code_for_NYT();
		was_new = true;

		this->NYT->unset_NYT();

		VitterTreeNode *new_NYT = new VitterTreeNode(true);
		VitterTreeNode *symbol_node = new VitterTreeNode(symbol);

		this->NYT->set_left_child(new_NYT);
		this->NYT->set_right_child(symbol_node);

		DPRINTF("insert: the new node was NYT, assigned number: %hhu\n",
			symbol_node->get_number());

		symbol_node->inc_weight();
		// this->NYT->inc_weight();

		this->NYT = new_NYT;

		current = symbol_node->get_parent();
	} else {
		DPRINTF("insert: the new node is already present (%hhu)\n",
			search_result->get_number());

		this->prev_symbol_code =
			this->get_huffman_code_for_char(symbol);

		current = search_result;
	}

	while (true) {
		DPRINTF("insert: rebalancing above\n");
		VitterTreeNode *block_leader = this->find_block_leader(current);
		if (current != block_leader) {
			if (!current->is_child_of(block_leader)) {
				DPRINTF("insert: exchanging with the block "
					"leader (%hhu <-> %hhu)\n",
					current->get_number(),
					block_leader->get_number());
				current->swap_with(block_leader);
				// current = block_leader;
			} else {
				DPRINTF("insert: is the child of the block "
					"leader (%hhu), not exchanging\n",
					block_leader->get_number());
			}
		} else {
			DPRINTF("insert: already the block leader\n");
		}

		current->inc_weight();

		if (current->is_root()) {
			break;
		}

		current = current->get_parent();
	}

	return was_new;
}

class FileBitWriter {
	public:
	void add(BitList bits);
	void flush();

	FileBitWriter(FILE *file) : buffer(0), count(0) {
		assert(file);
		this->file = file;
	};

	private:
	FILE *file;
	uint8_t buffer;
	size_t count;

	void add_bit(bool bit);
};

void FileBitWriter::add(BitList bits) {
	for (size_t i = 0; i < bits.get_length(); i++) {
		this->add_bit(bits[i]);
	}
}

void FileBitWriter::add_bit(bool bit) {
	this->buffer <<= 1;
	if (bit) {
		this->buffer |= 1U;
	}
	this->count++;

	if (this->count == 8) {
		this->flush();
	}
}

void FileBitWriter::flush() {
	this->buffer <<= (8 - this->count);
	fwrite(&this->buffer, sizeof(bool), 1, this->file);
	this->buffer = 0;
	this->count = 0;
}

// TODO: investigate bad compression ratio
static int dynamic_huffman_encode_filter(FILE *input_file, FILE *output_file) {
	VitterTree vitter_tree;

	FileBitWriter archive_writer = FileBitWriter(output_file);

	int symbol;
	while ((symbol = fgetc(input_file)) != EOF) {
		bool was_new = vitter_tree.insert((char)symbol);

		if (was_new) {
			BitList NYT_code = vitter_tree.get_prev_NYT_code();

#ifdef DEBUG
			char *NYT_code_str = NYT_code.get_string();
			DPRINTF("new symbol %hhd: NYT code %s (length %zu)\n",
				(char)symbol, NYT_code_str,
				NYT_code.get_length());
			free(NYT_code_str);
#endif

			archive_writer.add(NYT_code);

			archive_writer.add(BitList(char(symbol)));
		} else {
			BitList huffman_code =
				vitter_tree.get_prev_symbol_code();

#ifdef DEBUG
			char *huffman_code_str = huffman_code.get_string();
			DPRINTF("old symbol %hhd: %s (length %zu)\n",
				(char)symbol, huffman_code_str,
				huffman_code.get_length());
			free(huffman_code_str);
#endif

			archive_writer.add(huffman_code);
		}
	}

	if (!feof(input_file)) {
		perror("fgetc");
		return EXIT_FAILURE;
	}

	archive_writer.flush();

	return EXIT_SUCCESS;
}

class FileBitScanner {
	public:
	// 0 or 1 -> success, EOF -> EOF or error
	int next_bit() {
		if (this->count == 0) {
			if (this->read_out() == EOF) {
				return EOF;
			}
		}

		bool ret = buffer & (1U << 7);
		buffer <<= 1;

		this->count--;
		return ret;
	}

	FileBitScanner(FILE *file) : buffer(0), count(0) {
		assert(file);
		this->file = file;
	}

	private:
	FILE *file;
	uint8_t buffer;
	size_t count;

	int read_out() {
		if (fread(&this->buffer, sizeof(this->buffer), 1, this->file) !=
		    1) {
			return EOF;
		};

		this->count = 8;

		return 0;
	}
};

int dynamic_huffman_decode_filter(FILE *input_file, FILE *output_file) {
	FileBitScanner archive_reader(input_file);

	int read_result;
	while ((read_result = archive_reader.next_bit()) != EOF) {
	}

	if (!feof(input_file)) {
		perror("fread");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
};

int main(int argc, char *argv[]) {
	const char *input_filename = nullptr;
	const char *output_filename = nullptr;
	bool decode = false;

	int opt;
	while ((opt = getopt(argc, argv, "i:o:d")) != -1) {
		switch (opt) {
			case 'i':
				assert(optarg);
				input_filename = optarg;
				break;
			case 'o':
				assert(optarg);
				output_filename = optarg;
				break;
			case 'd':
				decode = true;
				break;
			default:
				eprintf(USAGE, argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (optind < argc) {
		eprintf(USAGE, argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!input_filename) {
		input_filename = "/dev/stdin";
	}

	if (!output_filename) {
		output_filename = "/dev/stdout";
	}

	FILE *input_file = fopen(input_filename, "r");
	if (!input_filename) {
		perror("fopen");
	}

	FILE *output_file = fopen(output_filename, "w");
	if (!output_filename) {
		perror("fopen");
	}

	int ret;
	if (decode) {
		ret = dynamic_huffman_decode_filter(input_file, output_file);
	} else {
		ret = dynamic_huffman_encode_filter(input_file, output_file);
	}

	fclose(input_file);
	fclose(output_file);
	return ret;
}
