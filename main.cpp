/**
 * @file main.cpp
 * 
 * @brief Entrega Parcial AP1
 * 
 * Dirceu Morais da Costa Júnior (2669455)
 * Lucas Henrique Motta			 (2669730)
 */

#include <iostream>
#include "Lexico.h"

using namespace std;

int main(int argc, char* argv[]){
	Lexico lex(argv[1]);
	
	Token* t = lex.proximoToken();
	// imprime no terminal e grava no arquivo saida.txt
	ofstream saida("saida.txt");
	while(t != nullptr){
		saida << t->toString() << endl;
		cout << t->toString() << endl;
		delete t;
		t = lex.proximoToken();
	}

	saida.close();
	cout << "\n\n abrir arquivo Fonte\n\n" << endl;

	return 0;
}