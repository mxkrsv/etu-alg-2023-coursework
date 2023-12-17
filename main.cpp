#include <cstring>
#include <fstream>
#include <iostream>

union Data {
	int ascii[256] = {0};
	char huffmanCode[256][256];
};

class Node {
	public:
	int freq;    // Частота - данные для дерева
	Node *left;  // Левое поддерево
	Node *right; // Правое поддерево
	unsigned char ch; // Символ, который будет закодирован
	Node *next; // Для связанного списка
	Node(char c, int f)
	    : freq(f), left(nullptr), right(nullptr), ch(c), next(nullptr) {
	}
};

bool isLeaf(Node *root) {
	return (root->left == nullptr && root->right == nullptr);
}

void encode(Node *root, char *str, int length, Data &c) {
	if (root == nullptr) {
		return;
	}
	// найден листовой узел
	if (isLeaf(root)) {
		std::copy(str, str + length,
			  c.huffmanCode[static_cast<unsigned char>(root->ch)]);
	}

	str[length] = '0';
	encode(root->left, str, length + 1, c);

	str[length] = '1';
	encode(root->right, str, length + 1, c);
}

Node *popMin(Node *&head) {
	if (head == nullptr) {
		return nullptr;
	}
	Node *minNode = head;
	head = head->next;
	minNode->next = nullptr;
	return minNode;
}

void insertSorted(Node *&head, Node *newNode) {
	if (head == nullptr || newNode->freq <= head->freq) {
		newNode->next = head;
		head = newNode;
	} else {
		Node *current = head;
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

int WriteTextToFile(char *text, Data &c) {
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

	for (char *ptr = text; *ptr != '\0'; ++ptr) {
		// Записывем коды Хаффмана в двоичном виде
		for (int i = 0;
		     c.huffmanCode[static_cast<unsigned char>(*ptr)][i] != '\0';
		     ++i) {
			char bit =
				c.huffmanCode[static_cast<unsigned char>(*ptr)]
					     [i];
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

char *ReadTextfromFile() {
	char filePath[100];
	size_t fileSize;

	std::cout << "Enter file path: ";
	std::cin.getline(filePath, sizeof(filePath));

	FILE *file = fopen(filePath, "rb");
	if (file == nullptr) {
		perror("File opening error");
		return nullptr;
	}

	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *text = new char[fileSize + 1];
	if (text == nullptr) {
		std::cout << "Memory allocation error!" << std::endl;
		fclose(file);
		return nullptr;
	}

	memset(text, 0, fileSize + 1);
	fread(text, sizeof(char), fileSize, file);

	fclose(file);

	return text;
}

void Huffman(char *text) {
	Data c;
	Node *head = nullptr; // Связанный список для отсортированных узлов

	// Пустая строка
	if (text == nullptr) {
		return;
	}

	// Подсчет частот и создание отсортированного списка
	for (char *ptr = text; *ptr != '\0'; ptr++) {
		c.ascii[static_cast<unsigned char>(*ptr)]++;
	}

	for (int i = 0; i < 256; i++) {
		if (c.ascii[i] > 0) {
			Node *newNode =
				new Node(static_cast<char>(i), c.ascii[i]);
			insertSorted(head, newNode);
		}
	}

	Node *root = nullptr;
	while (head != nullptr) {
		Node *firstMin = popMin(head);
		Node *secondMin = popMin(head);

		// Если нашли только один узел, то это корень дерева
		if (secondMin == nullptr) {
			root = firstMin;
			break;
		}

		// Создаем новый узел слиянием двух узлов с минимальной частотой
		Node *newNode =
			new Node('\0', firstMin->freq + secondMin->freq);
		newNode->left = firstMin;
		newNode->right = secondMin;

		// Вставляем новый узел в отсортированный список
		insertSorted(head, newNode);
	}

	// Буффер для хранения кодов
	char str[256];

	encode(root, str, 0, c);

	std::cout << "\nThe original string is:\n" << text << std::endl;
	WriteTextToFile(text, c);
	std::cout << "\nThe coding string is:\n";
	for (char *ptr = text; *ptr != '\0'; ++ptr) {
		// Вывод кодов
		std::cout << c.huffmanCode[static_cast<unsigned char>(*ptr)]
			  << " ";
	}
	std::cout << std::endl;

	// Освобождение памяти
	delete[] text;
}

int main() {
	char *text = ReadTextfromFile();
	Huffman(text);
	return 0;
}

// D:\\Huffman\\MainHuffman\\text.txt
