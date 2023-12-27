#include <assert.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

#define DEBUG

#ifdef DEBUG
#define DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...) ;
#endif

class HuffmanDict {
	public:
	char **huffmanCodes = new char *[256];
	HuffmanDict() {
		for (int i = 0; i < 256; ++i) {
			huffmanCodes[i] = nullptr;
		}
	}
	~HuffmanDict() {
		for (int i = 0; i < 256; ++i) {
			delete[] huffmanCodes[i];
		}
		delete[] huffmanCodes;
	}
};

typedef size_t FrequencyDict[256];

class TreeNode {
	public:
	TreeNode *left;
	TreeNode *right;

	// Payload

	// Character to be encoded
	uint8_t ch;
	// Frequency for a tree
	int freq;

	// Constructor
	TreeNode(char c, int f)
	    : left(nullptr), right(nullptr), ch(c), freq(f) {
	}

	// Methods
	bool isLeaf();
	void encode(char *, int, HuffmanDict &);
};

class ListNode {
	public:
	ListNode *next; // Для связанного списка

	// Payload
	TreeNode data;

	// Constructor
	ListNode(char c, int f) : next(nullptr), data(c, f) {
	}

	// Methods
	ListNode *popMin(ListNode **);
	ListNode *insertSorted(ListNode *);
};

// class List {
//	public:
//
//	// Methods
//	ListNode *popMin();
//	void insertSorted(ListNode *);
//
//	private:
//
//	// Payload
//	ListNode head;
// }

bool TreeNode::isLeaf() {
	assert(this);

	return (this->left == nullptr && this->right == nullptr);
}

void TreeNode::encode(char *currentCode, int curLen, HuffmanDict &dict) {
	assert(this);

	if (this->isLeaf()) {
		dict.huffmanCodes[this->ch] = new char[curLen + 1];
		strncpy(dict.huffmanCodes[this->ch], currentCode, curLen);
		dict.huffmanCodes[this->ch][curLen] = '\0';

		std::cout << "Character: " << this->ch
			  << ", Code: " << dict.huffmanCodes[this->ch]
			  << std::endl;
	}

	assert(curLen < 255);

	if (this->left) {
		currentCode[curLen] = '0';
		this->left->encode(currentCode, curLen + 1, dict);
	}

	if (this->right) {
		currentCode[curLen] = '1';
		this->right->encode(currentCode, curLen + 1, dict);
	}
}

ListNode *ListNode::popMin(ListNode **result) {
	assert(result);

	*result = this;

	if (*result) {
		// Invalidate the node
		ListNode *next = this->next;
		this->next = nullptr;

		return next;
	} else {
		return nullptr;
	}
}

ListNode *ListNode::insertSorted(ListNode *newNode) {
	assert(newNode);

	if (!this || newNode->data.freq <= this->data.freq) {
		newNode->next = this;
		return newNode;
	} else {
		ListNode *current = this;
		while (current->next != nullptr &&
		       current->next->data.freq < newNode->data.freq) {
			assert(current != current->next);
			current = current->next;
		}
		newNode->next = current->next;
		current->next = newNode;

		return this;
	}
}

void writeBit(FILE *file, uint8_t bit, uint8_t &buffer, int &bitsWritten) {
	assert(file);

	buffer <<= 1;
	buffer |= (bit - '0');
	bitsWritten++;

	if (bitsWritten == 8) {
		fwrite(&buffer, 1, 1, file);
		buffer = 0;
		bitsWritten = 0;
	}
}

void flushBits(FILE *file, uint8_t &buffer, int &bitsWritten) {
	assert(file);

	if (bitsWritten > 0) {
		buffer <<= (8 - bitsWritten);
		fwrite(&buffer, 1, 1, file);
		buffer = 0;
		bitsWritten = 0;
	}
}

void writeHeader(FILE *file, FrequencyDict c) {
	int uniqueCount = 0;
	for (int i = 0; i < 256; ++i) {
		if (c[i] > 0) {
			uniqueCount++;
		}
	}
	// Записываем количество уникальных символов как short
	fwrite(&uniqueCount, sizeof(short), 1, file);

	// Записываем пары "символ-частота"
	for (int i = 0; i < 256; ++i) {
		if (c[i] > 0) {
			// Записываем символ (1 байт)
			unsigned char symbol = (unsigned char)i;
			fwrite(&symbol, sizeof(unsigned char), 1, file);

			// Записываем частоту (4 байта)
			fwrite(&c[i], sizeof(c[0]), 1, file);
		}
	}
}

int writeTextToFile(uint8_t *text, size_t size, HuffmanDict &c,
		    FrequencyDict freqDict) {
	char nameFile[100];
	std::cout << "\nWrite the name for the compressed file\n" << std::endl;
	std::cin >> nameFile;

	FILE *newfile = fopen(nameFile, "wb");
	if (newfile == nullptr) {
		std::cout << "Error opening file for writing!" << std::endl;
		return 1;
	}

	writeHeader(newfile, freqDict);

	uint8_t buffer = 0;
	int bitsWritten = 0;

	for (size_t j = 0; j < size; j++) {
		// Записывем коды Хаффмана в двоичном виде
		for (int i = 0; c.huffmanCodes[text[j]][i] != '\0'; ++i) {
			uint8_t bit = c.huffmanCodes[text[j]][i];
			writeBit(newfile, bit, buffer, bitsWritten);
		}
	}

	// Очистить все оставшиеся биты, чтобы убедиться, что все данные
	// записаны
	flushBits(newfile, buffer, bitsWritten);

	if (fclose(newfile) != 0) {
		std::cout << "Error closing file!" << std::endl;
		return 1;
	}

	std::cout << "File successfully created: " << nameFile << std::endl;
	return 0;
}

uint8_t *readTextFromFile(size_t *size) {
	char filePath[100];

	std::cout << "Enter file path: ";
	std::cin.getline(filePath, sizeof(filePath));

	FILE *file = fopen(filePath, "rb");
	if (file == nullptr) {
		perror("File opening error");
		return nullptr;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t *text = new uint8_t[*size];
	if (text == nullptr) {
		std::cout << "Memory allocation error!" << std::endl;
		fclose(file);
		return nullptr;
	}

	// memset(text, 0, *size);
	fread(text, sizeof(char), *size, file);

	fclose(file);

	return text;
}

void huffman(uint8_t *text, size_t size) {
	assert(text && size);

	HuffmanDict huffmanDict;
	FrequencyDict freqDict = {0};

	// Count the frequencies
	for (size_t i = 0; i < size; i++) {
		freqDict[text[i]]++;
	}

	// Linked list for the sorted nodes
	ListNode *listHead = nullptr;

	for (int i = 0; i < 256; i++) {
		if (freqDict[i] > 0) {
			ListNode *newNode = new ListNode(i, freqDict[i]);
			listHead = listHead->insertSorted(newNode);
		}
	}

	TreeNode *treeRoot = nullptr;
	while (true) {
		ListNode *firstMin;
		ListNode *secondMin;

		listHead = listHead->popMin(&firstMin);
		listHead = listHead->popMin(&secondMin);

		assert(firstMin);

		// Если нашли только один узел, то это корень дерева
		if (!secondMin) {
			treeRoot = &firstMin->data;
			break;
		}

		// Создаем новый узел слиянием двух узлов с минимальной частотой
		ListNode *newNode = new ListNode(
			'\0', firstMin->data.freq + secondMin->data.freq);
		newNode->data.left = &firstMin->data;
		newNode->data.right = &secondMin->data;

		// Вставляем новый узел в отсортированный список
		listHead = listHead->insertSorted(newNode);
	}

	// Codes buffer
	char code[256];

	treeRoot->encode(code, 0, huffmanDict);

	std::cout << "\nThe original string is:\n" << text << std::endl;
	writeTextToFile(text, size, huffmanDict, freqDict);
	std::cout << "\nThe coding string is:\n";
	for (size_t i = 0; i < size; i++) {
		//  Вывод кодов
		std::cout << huffmanDict.huffmanCodes[text[i]] << " ";
	}
	std::cout << std::endl;

	// Освобождение памяти
	delete[] text;
	delete listHead;
}

int main() {
	size_t file_size;
	// TODO: fix data flow
	uint8_t *text = readTextFromFile(&file_size);
	huffman(text, file_size);
	return 0;
}
