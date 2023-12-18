#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <unistd.h>

#define DEBUG

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
		this->weight++;
	}

	uint8_t get_number() {
		return this->number;
	}

	VitterTreeNode *get_parent() {
		return this->parent;
	}

	// void set_number(uint8_t number) {
	//	this->number = number;
	// }

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

	void swap_with(VitterTreeNode *other) {
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
	}

	VitterTreeNode *walk(std::function<bool(VitterTreeNode *)> func);

	VitterTreeNode(bool NYT) : not_yet_transferred(NYT) {
	}

	VitterTreeNode(char symbol) : symbol(symbol), is_symbol(true) {
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

VitterTreeNode *
VitterTreeNode::walk(std::function<bool(VitterTreeNode *)> func) {
	VitterTreeNode *ret;

	if (this->left) {
		ret = this->left->walk(func);
		if (ret) {
			return ret;
		}
	}

	if (func(this)) {
		return this;
	};

	if (this->right) {
		ret = this->right->walk(func);
		if (ret) {
			return ret;
		}
	}

	return nullptr;
}

class VitterTree {
	public:
	VitterTreeNode *search(char symbol);
	VitterTreeNode *search(uint8_t number);

	VitterTreeNode *find_block_leader(VitterTreeNode *block_entry);

	void insert(char);

	VitterTree() {
		root = new VitterTreeNode(true);
		NYT = root;
	}

	private:
	VitterTreeNode *root;

	VitterTreeNode *NYT;
};

VitterTreeNode *VitterTree::search(char symbol) {
	return this->root->walk([symbol](VitterTreeNode *node) -> bool {
		return node->is_symb() && node->get_symbol() == symbol;
	});
}

VitterTreeNode *VitterTree::search(uint8_t number) {
	return this->root->walk([number](VitterTreeNode *node) -> bool {
		return node->get_number() == number;
	});
}

// TODO: ineffective
VitterTreeNode *VitterTree::find_block_leader(VitterTreeNode *block_entry) {
	VitterTreeNode *result = block_entry;

	for (uint8_t i = block_entry->get_number(); i != 0; i++) {
		VitterTreeNode *current = this->search(i);
		assert(current);

		if (block_entry->of_the_same_kind(result) &&
		    current->get_weight() == block_entry->get_weight()) {
			result = current;
		}
	}

	return result;
}

// TODO: verify (source: Russian Wikipedia)
void VitterTree::insert(char symbol) {
	VitterTreeNode *search_result = this->search(symbol);

	if (!search_result) {
		this->NYT->unset_NYT();

		VitterTreeNode *new_NYT = new VitterTreeNode(true);
		VitterTreeNode *symbol_node = new VitterTreeNode(symbol);

		this->NYT->set_left_child(new_NYT);
		this->NYT->set_right_child(symbol_node);

		symbol_node->inc_weight();
		this->NYT->inc_weight();

		this->NYT = new_NYT;
	} else {
		VitterTreeNode *current = search_result;

		while (true) {
			VitterTreeNode *block_leader =
				this->find_block_leader(current);
			if (current != block_leader) {
				if (!current->is_child_of(block_leader)) {
					current->swap_with(block_leader);
				}
			}

			current->inc_weight();

			if (current->is_root()) {
				break;
			}

			current = current->get_parent();
		}
	}
}

// TODO: implement building the code itself
static int dynamicHuffmanFilter(FILE *input_file, FILE *output_file) {
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

	return dynamicHuffmanFilter(input_file, output_file);
}
