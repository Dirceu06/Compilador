#include <iostream>
#include "Lexico.h"


using namespace std;


int main(int argc, char* argv[]){
	Lexico lex(argv[1]);
	
	Token* t = lex.proximoToken();
	while(t != nullptr){
		cout << t->toString() << endl;
		delete t;
		t = lex.proximoToken();
	}

	cout << "\n\n abrir arquivo Fonte\n\n" << endl;

	return 0;
}





