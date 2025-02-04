#include "LineToken.h"
#include "LineTokenizer.h"
#include "ParentTable.h"
#include "ModUseTable.h"
#include "VarList.h"
#include "StatementTable.h"
#include "FollowTable.h"
#include "Parser.h"
#include <unordered_map>
#include <stack>
#include <vector>


using namespace std;


Parser::Parser()
{
}

Parser::Parser(string fileName, PKB * p)
{
	pkb = p;
	LineTokenizer tokenizer = LineTokenizer(fileName);
	tokenizer.tokenize();
	tokens = tokenizer.getTokenVector();
	populateAllTables();
}

Parser::~Parser()
{
}

void Parser::populateFollowTable()
{
	vector<LineToken> tokensCopy = tokens;
	vector<pair<int,int>> ft;
	
	bool isWhile = false;
	stack<LineToken> nestedStack;

	int *nesting = new int[tokensCopy.size()];

	int counter = 0;

	for (auto it = tokensCopy.begin(); it != tokensCopy.end(); ++it) {
		nesting[counter] = it->getLevel();
		counter++;
	}

	//create followtable array here

	// pass in a nesting table
	// int nesting[] = { 0,1,2,3,3,2,0 };

	// count the size of the nesting array;
	int countSize = tokensCopy.size();

	// n for outer loop, n for inner loop
	int n, m;
	for (n = 0; n < countSize - 1; n++) {
		// get the current nesting level
		int currentNesting = nesting[n];
		for (m = n + 1; m < countSize; m++) {
			int nextNesting = nesting[m];

			// if next nesting level drops below current level, no need to proceed
			if (nextNesting < currentNesting) {
				break;
			}
			// if same level means follow
			if (nextNesting == currentNesting) {
				//std::cout << (n + 1) << (m + 1) << "\n";
				pair<int, int> entry(n + 1, m + 1);
				ft.push_back(entry);
				break;
			}
		}
	}
	pkb->setFollowTable(new FollowTable(ft));
}

void Parser::populateParentTable()
{
	vector<LineToken> tokensCopy = tokens;
	vector<pair<int, int>> pt;
	
	bool isWhile = false;
	stack<LineToken> nestedStack;

	for (auto it = tokensCopy.begin(); it != tokensCopy.end(); ++it) {

		//populates ParentTable
		if (isWhile) {
			//current statement is still within the current nesting level
			if (nestedStack.top().getLevel() == it->getLevel()-1) {
				int parent = nestedStack.top().getStmtNumber();
				int child = it->getStmtNumber();
				pair<int, int> entry(parent, child);
				pt.push_back(entry);
			}
			else //current statement goes back to same nesting level as "while" or even lower
			{
				//pop out all while loops that has ended
				while (!nestedStack.empty() && nestedStack.top().getLevel() >= it->getLevel()) {
					nestedStack.pop();
				}
				//if stack is still not empty, current statement is a child of the while loop in the stack
				if (!nestedStack.empty()) {
					int parent = nestedStack.top().getStmtNumber();
					int child = it->getStmtNumber();
					pair<int, int> entry(parent, child);
					pt.push_back(entry);
				}
				else //nestedStack is empty
					isWhile = false;
			}
		}

		if (it->getType() == "WHILE") {
			isWhile = true;
			nestedStack.push(*it);
		}
	}
	pkb->setParentTable(new ParentTable(pt));
}

void Parser::populateStatementTable()
{
	vector<LineToken> tokensCopy = tokens;
	vector<LineToken>::iterator it;
	unordered_map<int, LineToken> st;

	for (it = tokensCopy.begin(); it != tokensCopy.end(); ++it) {
		st.insert({ it->getStmtNumber(), *it });
	}
	pkb->setStatementTable(new StatementTable(st));
}

void Parser::appendToParents(unordered_map<int, pair<vector<string>, vector<string>>>* table, vector<int> stack, string mod, vector<string> used) {
	for (int i = 0; i < stack.size(); ++i) {
		appendModVar(table, stack.at(i), mod);
		appendUsedVar(table, stack.at(i), used);
	}
}

void Parser::appendModVar(unordered_map<int, pair<vector<string>, vector<string>>> *table, int stmtNum, string newVar) {
	vector<string> currentModVar = table->at(stmtNum).first;
	vector<string> second = table->at(stmtNum).second;
	//check if table already contains that variable (if the variable is not "")
	if (newVar != "") {
		if (find(currentModVar.begin(), currentModVar.end(), newVar) == currentModVar.end()) {
			currentModVar.push_back(newVar);
			table->at(stmtNum) = pair<vector<string>, vector<string>>(currentModVar, second);
		}
	}
}

void Parser::appendUsedVar(unordered_map<int, pair<vector<string>, vector<string>>>* table, int stmtNum, vector<string> newVar) {
	vector<string> first = table->at(stmtNum).first;
	vector<string> currentUsedVar = table->at(stmtNum).second;
	//check if table already contains each variable
	for (int i = 0; i < newVar.size(); ++i) {
		string theVar = newVar.at(i);
		if (find(currentUsedVar.begin(), currentUsedVar.end(), theVar) == currentUsedVar.end()) {
			currentUsedVar.push_back(theVar);
		}
	}
	table->at(stmtNum) = pair<vector<string>, vector<string>>(first, currentUsedVar);
}

void Parser::populateModUseTable() {
	vector<LineToken> tokensCopy = tokens;
	vector<int> whileStack;
	unordered_map<int, pair<vector<string>, vector<string>>> table;
	unordered_map<string, int> list;

	int currentLevel = 0;
	for (auto it = tokensCopy.begin(); it != tokensCopy.end(); ++it) {
		int thisLevel = it->getLevel();
		// current level is initialized to 1 so that the following stmts will not be seen as nested in the procedure
		// this will be modified after iteration 1, where procedure level is important.
		if (it->getType() == "PROCEDURE") {
			currentLevel = 1;
		}
		else if (thisLevel >= currentLevel) {
			currentLevel = thisLevel;
		}
		else if (thisLevel < currentLevel) {
			//delete a number of elements from the whileStack
			int toDelete = currentLevel - thisLevel;
			for (int i = 0; i < toDelete; ++i) {
				whileStack.pop_back();
			}
			currentLevel = thisLevel;
		}

		if (it->getType() == "ASSIGN") {
			string modVar = it->getName();
			vector<string> modified(1, modVar);
			vector<string> used = it->getExpr();

			//if this stmt is a child of other stmts, update all parents' modified and uses table
			if (!whileStack.empty()) {
				appendToParents(&table, whileStack, modVar, used);
			}

			//add new variables to VarList, while checking for no constant values
			list.insert(pair<string, int>(modVar, list.size()));
			for (int i = 0; i < used.size(); ++i) {
				try {
					int converted = stoi(used[i]);
				}
				catch (exception e) {
					list.insert(pair<string, int>(used[i], list.size()));
				}
			}

			//add this stmt into the table
			table.insert({ it->getStmtNumber(), pair<vector<string>, vector<string>>(modified, used) });
		}

		if (it->getType() == "WHILE") {
			vector<string> modified;
			vector<string> used(1, it->getName());

			//if this stmt is a child of other stmts, update all parents' modified and uses table
			if (!whileStack.empty()) {
				appendToParents(&table, whileStack, "", used);
			}

			//add this while statement into the table
			table.insert({ it->getStmtNumber(), pair<vector<string>, vector<string>>(modified, used) });
			
			//add new variables to VarList
			list.insert(pair<string, int>(it->getName(), list.size()));

			//add the stmt number into the stack
			whileStack.push_back(it->getStmtNumber());
		}
	}
	pkb->setModUseTable(new ModUseTable(table));
	pkb->setVarList(new VarList(list));
}


void Parser::populateAllTables()
{
	populateFollowTable();
	populateModUseTable();
	populateStatementTable();
	populateParentTable();
}
