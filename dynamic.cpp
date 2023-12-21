#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

/*
class FixedBitList {
	public:
	size_t get_length() {
		return this->length;
	}

	void push_back(bool bit) {
		assert(this->length < this->capacity);

		if (bit) {
			// Set bit
			this->list |= (1U << (8 - length - 1));
		} else {
			// Unset bit
			this->list &= ~(1U << (8 - length - 1));
		}

		this->length++;
	}

	void pop_back() {
		this->length--;
	}

	bool operator[](size_t idx) {
		assert(idx < length);
		return ((this->list << idx) >> (this->capacity - 1));
	}

	char *get_string() {
		char *ret;

		asprintf(&ret, "%.*b", (int)this->length,
			 this->list >> (this->capacity - this->length));

		return ret;
	}

	// XXX: only one-byte cap is supported
	FixedBitList(size_t capacity) : list(0), length(0), capacity(capacity) {
		assert(capacity == 8);
	}

	private:
	uint8_t list;
	size_t length;
	size_t capacity;
};
*/

class BitList {
	public:
	size_t get_length() {
		return this->length;
	}

	void push_back(bool bit) {
		if (length == capacity) {
			this->list = (bool *)reallocarray(
				this->list, this->capacity * 2, sizeof(bool));
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

	void insert(char);

	BitList get_huffman_code(char symbol);

	VitterTree() {
		root = new VitterTreeNode(true);
		NYT = root;
	}

	~VitterTree() {
		delete root;
	}

	private:
	VitterTreeNode *root;

	VitterTreeNode *NYT;

	VitterTreeNode *find_block_leader(VitterTreeNode *block_entry);
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
BitList VitterTree::get_huffman_code(char symbol) {
	BitList huffman_code;

	this->root->walk_extended(
		[symbol](VitterTreeNode *node) -> bool {
			if (node->is_symb()) {
				DPRINTF("get_huffman_code: matches(%d, %d) = "
					"%d\n",
					node->get_symbol(), symbol,
					node->get_symbol() == symbol);
				return node->get_symbol() == symbol;
			}

			return false;
		},
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
void VitterTree::insert(char symbol) {
	VitterTreeNode *search_result = this->search_symbol(symbol);

	VitterTreeNode *current = nullptr;
	if (!search_result) {
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
}

// TODO: implement building the code itself
static int dynamic_huffman_filter(FILE *input_file, FILE *output_file) {
	VitterTree vitter_tree;

	int symbol;
	while ((symbol = fgetc(input_file)) != EOF) {
		vitter_tree.insert((char)symbol);

		BitList huffman_code = vitter_tree.get_huffman_code(symbol);

		char *huffman_code_str = huffman_code.get_string();
		printf("%hhd: %s (length %zu)\n", (char)symbol,
		       huffman_code_str, huffman_code.get_length());
		free(huffman_code_str);
	}

	if (!feof(input_file)) {
		perror("fgetc");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	const char *input_filename = nullptr;
	const char *output_filename = nullptr;

	int opt;
	while ((opt = getopt(argc, argv, "i:o:")) != -1) {
		switch (opt) {
			case 'i':
				assert(optarg);
				input_filename = optarg;
				break;
			case 'o':
				assert(optarg);
				output_filename = optarg;
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

	return dynamic_huffman_filter(input_file, output_file);
}
