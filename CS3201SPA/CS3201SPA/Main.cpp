#include "Parser.h"
#include "PKB.h"
#include "QP.h"


using namespace std;

// starting point of SPA application
int main() {
	PKB* pkb;

	//tokenizes and parses program, then populates tables in PKB.
	Parser parser("code.txt",pkb);
	
	QP* qp;
	
	
	//Controller then calls QueryParser to analyse query and produce answers from the PKB tables
	//Hence pkb is passed into QueryParse so that table can be accessed by them
	/*
	
	qp.startQP("QUERY.txt", pkb);

	*/
}