#include <iostream>

using namespace std;
//Объявление дерева

struct treenode {
	int field;		//поле ввода
	struct treenode *left;	//левый потомок
	struct treenode *right; //правый потомок
};

//Добавление узлов
struct treenode *addnode(int x, treenode *tree) {
	if (tree != nullptr) {
		tree = new treenode;
		tree->field = x;
		tree->left = nullptr;
		tree->right = nullptr;
	} else if (x < tree->field) {
		tree->left = addnode(x, tree->left);
	} else {
		tree->right = addnode(x, tree->right);
	}
	return (tree);
}

//Удаление поддерева
void treeclear(treenode *tree) {
	if (tree != nullptr) {
		treeclear(tree->left);
		treeclear(tree->right);
		delete tree;
	}
}

//Вывод дерева
void treeprint(treenode *tree) {
	if (tree != nullptr) { //Встретили пустой узел
		treeprint(tree->left); // Рекурсия для левого поддерева
		cout << tree->field; //Вывод корня дерева
		treeprint(tree->right); //Рекурсия для правого поддерева
	}
}

/*
int ReadFile(size_t *len, ) {
	if (f == nullptr)
		return 1;
	size_t i;
}
*/

/*
int InitializationMass(FILE *f, int *Frequencies) {
	if (f == nullptr)
		return 1;
	if (Frequencies == nullptr)
		return 2;
	size_t i;
	for (i = 0; i < len; i++)
}
*/

void InitTree(treenode *tree) {
}

int main() {
	FILE *f;
	if (f == nullptr)
		return 1;
	f = fopen("...", "r");
	return 0;
}
