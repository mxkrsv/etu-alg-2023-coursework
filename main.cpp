#include <assert.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>

union Data {
	int ascii[256] = {0};
	char huffmanCode[256][256];
};

class TreeNode {
	public:
	int freq;	 // Частота - данные для дерева
	TreeNode *left;	 // Левое поддерево
	TreeNode *right; // Правое поддерево
	unsigned char ch; // Символ, который будет закодирован
	TreeNode *next; // Для связанного списка

	// Constructor
	TreeNode(char c, int f)
	    : freq(f), left(nullptr), right(nullptr), ch(c), next(nullptr) {
	}

	// Methods
	bool isLeaf();
	void encode(char *, int, Data &);
};

bool TreeNode::isLeaf() {
	assert(this);

	return (this->left == nullptr && this->right == nullptr);
}

void TreeNode::encode(char *str, int length, Data &c) {
	assert(this);

	// найден листовой узел
	if (this->isLeaf()) {
		std::copy(str, str + length, c.huffmanCode[this->ch]);
	}

	str[length] = '0';
	if (this->left) {
		this->left->encode(str, length + 1, c);
	}

	str[length] = '1';
	if (this->right) {
		this->right->encode(str, length + 1, c);
	}
}

TreeNode *popMin(TreeNode *&head) {
	if (head == nullptr) {
		return nullptr;
	}
	TreeNode *minNode = head;
	head = head->next;
	minNode->next = nullptr;
	return minNode;
}

void insertSorted(TreeNode *&head, TreeNode *newNode) {
	if (head == nullptr || newNode->freq <= head->freq) {
		newNode->next = head;
		head = newNode;
	} else {
		TreeNode *current = head;
		while (current->next != nullptr &&
		       current->next->freq < newNode->freq) {
			current = current->next;
		}
		newNode->next = current->next;
		current->next = newNode;
	}
}

void writeBit(FILE *file, char bit, char &buffer, int &bitsWritten) {
	buffer <<= 1;
	buffer |= (bit & 1);
	bitsWritten++;

	if (bitsWritten == 8) {
		fwrite(&buffer, 1, 1, file);
		buffer = 0;
		bitsWritten = 0;
	}
}

void flushBits(FILE *file, char &buffer, int &bitsWritten) {
	if (bitsWritten > 0) {
		buffer <<= (8 - bitsWritten);
		fwrite(&buffer, 1, 1, file);
		buffer = 0;
		bitsWritten = 0;
	}
}

int writeTextToFile(uint8_t *text, size_t size, Data &c) {
	char nameFile[100];
	std::cout << "\nWrite the name for the compressed file\n" << std::endl;
	std::cin >> nameFile;

	FILE *newfile = fopen(nameFile, "wb");
	if (newfile == nullptr) {
		std::cout << "Error opening file for writing!" << std::endl;
		return 1;
	}

	char buffer = 0;
	int bitsWritten = 0;

	for (size_t j = 0; j < size; j++) {
		// Записывем коды Хаффмана в двоичном виде
		for (int i = 0; c.huffmanCode[text[j]][i] != '\0'; ++i) {
			char bit = c.huffmanCode[text[j]][i];
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
	Data c;
	TreeNode *head = nullptr; // Связанный список для отсортированных узлов

	assert(text && size);

	// Count the frequencies
	for (size_t i = 0; i < size; i++) {
		c.ascii[text[i]]++;
	}

	for (int i = 0; i < 256; i++) {
		if (c.ascii[i] > 0) {
			TreeNode *newNode = new TreeNode(i, c.ascii[i]);
			insertSorted(head, newNode);
		}
	}

	TreeNode *root = nullptr;
	while (head != nullptr) {
		TreeNode *firstMin = popMin(head);
		TreeNode *secondMin = popMin(head);

		// Если нашли только один узел, то это корень дерева
		if (secondMin == nullptr) {
			root = firstMin;
			break;
		}

		// Создаем новый узел слиянием двух узлов с минимальной частотой
		TreeNode *newNode =
			new TreeNode('\0', firstMin->freq + secondMin->freq);
		newNode->left = firstMin;
		newNode->right = secondMin;

		// Вставляем новый узел в отсортированный список
		insertSorted(head, newNode);
	}

	// Буффер для хранения кодов
	char str[256];

	root->encode(str, 0, c);

	std::cout << "\nThe original string is:\n" << text << std::endl;
	writeTextToFile(text, size, c);
	std::cout << "\nThe coding string is:\n";
	for (size_t i = 0; i < size; i++) {
		// Вывод кодов
		std::cout << c.huffmanCode[text[i]] << " ";
	}
	std::cout << std::endl;

	// Освобождение памяти
	delete[] text;
}

int main() {
	size_t file_size;
	uint8_t *text = readTextFromFile(&file_size);
	huffman(text, file_size);
	return 0;
}
