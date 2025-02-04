#include "QE.h"
#include "LineToken.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <stack>
#include <iostream>
#include <algorithm>
#include <regex>
#include <sstream>

using namespace std;

/* Query Evaluator
1. Each method is called by the Query Parser depending on the command
2. The field obtained is the select string, field 1 and field 2
3. According to the command, the respective method is used
4. The method will access the PKB for the information
5. There will be a filter to access the type of the string to further evaluate the answers
6. The query answers will be placed in vector<string> ans1 for all commands and ans2 for pattern command
7. If there are only 1 command, the respective vector will be printed else AND the two vectors before printing*/

QE::QE(PKB * p)
{
	pkb = p;
}

void QE::evaluateSingleQuery(string queryString) {
	queryStringForQP = queryString;
	QP queryParser = QP();
	queryParser.processSingleQuery(queryStringForQP);
	vector<string> ansST;
	vector<string> ansP;
	Query q = queryParser.getQuery();
	vector<string> fields = q.getQueryFields();
	int size = fields.size();
	if (size == 0) {
		answerForSingleQuery = list<string>();
	}
	else if (size == 4) {
		//cout << "Currently in size = 4" << endl;
		string select = fields[0];
		string command = fields[1];
		string one = fields[2];
		string two = fields[3];
		//cout << select << endl;
		//cout << command << endl;
		//cout << one << endl;
		//cout << two << endl;
		if (checkAnswerSize(selectField(select, command, one, two, q))) {
			answerForSingleQuery = convertVectorToList(selectField(select, command, one, two, q));
		}
		else {
			answerForSingleQuery = list<string>();
			//answerForSingleQuery.push_back("none");
		}
	}
	else if (size == 8) {
		string select = fields[0];
		string command = fields[1];
		string one = fields[2];
		string two = fields[3];
		string select2 = fields[4];
		string command2 = fields[5];
		string one2 = fields[6];
		string two2 = fields[7];
		int relate1;
		int relate2;
		bool firstIsPattern = false;
		pair<int, int> dependency = pair<int, int>{ 0, 0 };
		if (command.substr(0, 7).compare("pattern") == 0) {
			relate1 = relation(select, command.substr(7, command.size() - 7), "");
			relate2 = relation(select2, one2, two2);
			dependency = dependent(select, one2, two2, command.substr(7, command.size() - 7), one);
		}
		else {
			relate1 = relation(select, one, two);
			relate2 = relation(select2, command2.substr(7, command2.size() - 7), "");
			dependency = dependent(select, one, two, command2.substr(7, command2.size() - 7), one2);
		}
		if (dependency.first != 0) {
			if (dependency.first == 1) {
				if (firstIsPattern) {
					string newSelect;
					if (dependency.second == 1) {
						newSelect = one2;
					}
					else {
						newSelect = two2;
					}
					vector<string> condAns = selectField(newSelect, command, one, two, q);
					pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
					// do 2nd cond with dfilter
					vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
					answerForSingleQuery = convertVectorToList(finalAns);
				}
				else {
					string newSelect;
					if (dependency.second == 1) {
						newSelect = command2.substr(7, command2.size() - 7);
					}
					else {
						newSelect = one2;
					}
					vector<string> condAns = selectField(newSelect, command, one, two, q);
					pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
					// do 2nd cond with dfilter
					vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
					answerForSingleQuery = convertVectorToList(finalAns);
				}
			}
			else if (dependency.first == 2) {
				if (firstIsPattern) {
					string newSelect;
					if (dependency.second == 1) {
						newSelect = command.substr(7, command.size() - 7);
					}
					else {
						newSelect = one;
					}
					vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
					pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
					// do 2nd cond with dfilter
					vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
					answerForSingleQuery = convertVectorToList(finalAns);
				}
				else {
					string newSelect;
					if (dependency.second == 1) {
						newSelect = one;
					}
					else {
						newSelect = two;
					}
					vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
					pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
					// do 2nd cond with dfilter
					vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
					answerForSingleQuery = convertVectorToList(finalAns);
				}
			}
			else if (dependency.first == 3) {
				string newSelect;
				if (firstIsPattern) {
					if (dependency.second == 1) {
						newSelect = command.substr(7, command.size() - 7);
					}
					else {
						newSelect = one;
					}
				}
				else {
					if (dependency.second == 1) {
						newSelect = one;
					}
					else {
						newSelect = two;
					}
				}
				vector<string> boolAns = findCommonAnswer(selectField(newSelect, command, one, two, q), 
					selectField(newSelect, command2, one2, two2, q));
				if(checkAnswerSize(boolAns) == 0){
					answerForSingleQuery = list<string>();
				}
				else {
					answerForSingleQuery = convertVectorToList(selectField(select, command, one, two, q));
				}
			}
			// invalid query, because of 2 common synonyms
			else if (dependency.first == 4) {
				answerForSingleQuery = list<string>();
			}
		}
		else if (relate1 == 0 && relate2 != 0) {
			if (checkAnswerSize(selectField(select, command, one, two, q))) {
				if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
					answerForSingleQuery = convertVectorToList(selectField(select2, command2, one2, two2, q));
				}
				else {
					answerForSingleQuery = list<string>();
					//answerForSingleQuery.push_back("none");
				}
			}
			else {
				answerForSingleQuery = list<string>();
				//answerForSingleQuery.push_back("none");
			}
		}
		else if (relate1 != 0 && relate2 == 0) {
			if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
				if (checkAnswerSize(selectField(select, command, one, two, q))) {
					answerForSingleQuery = convertVectorToList(selectField(select, command, one, two, q));
				}
				else {
					answerForSingleQuery = list<string>();
					//answerForSingleQuery.push_back("none");
				}
			}
			else {
				answerForSingleQuery = list<string>();
				//answerForSingleQuery.push_back("none");
			}
		}
		else {
			if (checkAnswerSize(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q)))) {
				answerForSingleQuery = convertVectorToList(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q)));
			}
			else {
				answerForSingleQuery = list<string>();
				//answerForSingleQuery.push_back("none");
			}
		}
	}
}

QE::QE(string fileName, PKB * p)
{
	filePath = fileName;
	QP qp(fileName);
	qp.process();
	queryVector = qp.getVectorQuery();
	pkb = p;
	vector<Query> query = qp.getVectorQuery();
	vector<string> ansST;
	vector<string> ansP;
	for (int i = 0; i < query.size(); i++) { //No query
		Query q = query[i];
		vector<string> fields = q.getQueryFields();
		int size = fields.size();
		if (size == 0) {
			answers.push_back("none");
		}
		else if (size == 4) { // 1 field
			string select = fields[0];
			string command = fields[1];
			string one = fields[2];
			string two = fields[3];
			if (checkAnswerSize(selectField(select, command, one, two, q))) {
				answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
			}
			else {
				answers.push_back("none");
			}
		}
		else if (size == 8) { // 2 fields
			string select = fields[0];
			string command = fields[1];
			string one = fields[2];
			string two = fields[3];
			string select2 = fields[4];
			string command2 = fields[5];
			string one2 = fields[6];
			string two2 = fields[7];
			int relate1;
			int relate2;
			bool firstIsPattern = false;
			pair<int, int> dependency = pair<int, int>{ 0, 0 };
			if (command.substr(0, 7).compare("pattern") == 0) {
				relate1 = relation(select, command.substr(7, command.size() - 7), one);
				relate2 = relation(select2, one2, two2);
				dependency = dependent(select, one2, two2, command.substr(7, command.size() - 7), one);
			}
			else {
				relate1 = relation(select, one, two);
				relate2 = relation(select2, command2.substr(7, command2.size() - 7), one2);
				dependency = dependent(select, one, two, command2.substr(7, command2.size() - 7), one2);
			}
			if (dependency.first != 0) {
				if (dependency.first == 1) {
					if (firstIsPattern) {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = one2;
						}
						else {
							newSelect = two2;
						}
						vector<string> condAns = selectField(newSelect, command, one, two, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>> { dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
					else {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = command2.substr(7, command2.size() - 7);
						}
						else {
							newSelect = one2;
						}
						vector<string> condAns = selectField(newSelect, command, one, two, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>> { dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
				}
				else if (dependency.first == 2) {
					if (firstIsPattern) {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = command.substr(7, command.size() - 7);
						}
						else {
							newSelect = one;
						}
						vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>> { dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
					else {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = one;
						}
						else {
							newSelect = two;
						}
						vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>> { dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
				}
				else if (dependency.first == 3) {
					string newSelect;
					if (firstIsPattern) {
						if (dependency.second == 1) {
							newSelect = command.substr(7, command.size() - 7);
						}
						else {
							newSelect = one;
						}
					}
					else {
						if (dependency.second == 1) {
							newSelect = one;
						}
						else {
							newSelect = two;
						}
					}
					vector<string> boolAns = findCommonAnswer(selectField(newSelect, command, one, two, q),
						selectField(newSelect, command2, one2, two2, q));
					if (checkAnswerSize(boolAns) == 0) {
						answers.push_back("none");
					}
					else {
						answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
					}
				}
				// invalid query, because of 2 common synonyms
				else if (dependency.first == 4) {
					answers.push_back("none");
				}
			}
			else if (relate1 == 0 && relate2 != 0) {
				if (checkAnswerSize(selectField(select, command, one, two, q))) {
					if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
						answers.push_back(vectorSToString(selectField(select2, command2, one2, two2, q)));
					}
					else {
						answers.push_back("none");
					}
				}
				else {
					answers.push_back("none");
				}
			}
			else if (relate1 != 0 && relate2 == 0) {
				if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
					if (checkAnswerSize(selectField(select, command, one, two, q))) {
						answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
					}
					else {
						answers.push_back("none");
					}
				}
				else {
					answers.push_back("none");
				}
			}
			else {
				if (checkAnswerSize(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q)))) {
					answers.push_back(vectorSToString(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q))));
				}
				else {
					answers.push_back("none");
				}
			}
		}
	}
}

//constructor for unit testing QE, bypasses query tokenizing
QE::QE(vector<Query> qVector, PKB * p) {
	queryVector = qVector;
	pkb = p;
	vector<Query> query = qVector;
	vector<string> ansST;
	vector<string> ansP;
	for (int i = 0; i < query.size(); i++) {
		Query q = query[i];
		vector<string> fields = q.getQueryFields();
		int size = fields.size();
		if (size == 0) {
			answers.push_back("none");
		}
		else if (size == 4) {
			string select = fields[0];
			string command = fields[1];
			string one = fields[2];
			string two = fields[3];
			if (checkAnswerSize(selectField(select, command, one, two, q))) {
				answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
			}
			else {
				answers.push_back("none");
			}
		}
		else if (size == 8) {
			string select = fields[0];
			string command = fields[1];
			string one = fields[2];
			string two = fields[3];
			string select2 = fields[4];
			string command2 = fields[5];
			string one2 = fields[6];
			string two2 = fields[7];
			int relate1;
			int relate2;
			bool firstIsPattern = false;
			pair<int, int> dependency = pair<int, int>{ 0, 0 };
			if (command.substr(0, 7).compare("pattern") == 0) {
				relate1 = relation(select, command.substr(7, command.size() - 7), one);
				relate2 = relation(select2, one2, two2);
				dependency = dependent(select, one2, two2, command.substr(7, command.size() - 7), one);
			}
			else {
				relate1 = relation(select, one, two);
				relate2 = relation(select2, command2.substr(7, command2.size() - 7), one2);
				dependency = dependent(select, one, two, command2.substr(7, command2.size() - 7), one2);
			}
			if (dependency.first != 0) {
				if (dependency.first == 1) {
					if (firstIsPattern) {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = one2;
						}
						else {
							newSelect = two2;
						}
						vector<string> condAns = selectField(newSelect, command, one, two, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
					else {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = command2.substr(7, command2.size() - 7);
						}
						else {
							newSelect = one2;
						}
						vector<string> condAns = selectField(newSelect, command, one, two, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select2, command2, one2, two2, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
				}
				else if (dependency.first == 2) {
					if (firstIsPattern) {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = command.substr(7, command.size() - 7);
						}
						else {
							newSelect = one;
						}
						vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
					else {
						string newSelect;
						if (dependency.second == 1) {
							newSelect = one;
						}
						else {
							newSelect = two;
						}
						vector<string> condAns = selectField(newSelect, command2, one2, two2, q);
						pair<int, vector<string>> dFilter = pair<int, vector<string>>{ dependency.second, condAns };
						// do 2nd cond with dfilter
						vector<string> finalAns = selectField(select, command, one, two, q, dFilter);
						answers.push_back(vectorSToString(finalAns));
					}
				}
				else if (dependency.first == 3) {
					string newSelect;
					if (firstIsPattern) {
						if (dependency.second == 1) {
							newSelect = command.substr(7, command.size() - 7);
						}
						else {
							newSelect = one;
						}
					}
					else {
						if (dependency.second == 1) {
							newSelect = one;
						}
						else {
							newSelect = two;
						}
					}
					vector<string> boolAns = findCommonAnswer(selectField(newSelect, command, one, two, q),
						selectField(newSelect, command2, one2, two2, q));
					if (checkAnswerSize(boolAns) == 0) {
						answers.push_back("none");
					}
					else {
						answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
					}
				}
				// invalid query, because of 2 common synonyms
				else if (dependency.first == 4) {
					answers.push_back("none");
				}
			}
			else if (relate1 == 0 && relate2 != 0) {
				if (checkAnswerSize(selectField(select, command, one, two, q))) {
					if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
						answers.push_back(vectorSToString(selectField(select2, command2, one2, two2, q)));
					}
					else {
						answers.push_back("none");
					}
				}
				else {
					answers.push_back("none");
				}
			}
			else if (relate1 != 0 && relate2 == 0) {
				if (checkAnswerSize(selectField(select2, command2, one2, two2, q))) {
					if (checkAnswerSize(selectField(select, command, one, two, q))) {
						answers.push_back(vectorSToString(selectField(select, command, one, two, q)));
					}
					else {
						answers.push_back("none");
					}
				}
				else {
					answers.push_back("none");
				}
			}
			else {
				if (checkAnswerSize(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q)))) {
					answers.push_back(vectorSToString(findCommonAnswer(selectField(select, command, one, two, q), selectField(select2, command2, one2, two2, q))));
				}
				else {
					answers.push_back("none");
				}
			}
		}
	}
}

vector<string> QE::selectField(string select, string command, string one, string two, Query q, pair<int, vector<string>> dFilter) {
	bool isNum1 = isInt(one);
	bool isNum2 = isInt(two);
	vector<string> ansP;
	vector<string> ansST;
	int a, b;
	if (isNum1) {
		a = atoi(one.c_str());
	}
	if (isNum2) {
		b = atoi(two.c_str());
	}
	if (command.substr(0, 7).compare("pattern") == 0) {
		ansP = pattern(select, command, one, two, q, dFilter);
		return ansP;
	}
	else {
		if (command.compare("Modifies") == 0) {
			if (!isNum1 && !isNum2) {
				ansST = ModifiesS(select, one, two, q, dFilter);
			}
			else {
				ansST = ModifiesS(select, a, two, q);
			}
		}
		else if (command.compare("Uses") == 0) {
			if (isNum1 && !isNum2) {
				ansST = UsesS(select, a, two, q);
			}
			else {
				ansST = UsesS(select, one, two, q, dFilter);
			}
		}
		else if (command.compare("Parent") == 0) {
			if (isNum1 && !isNum2) {
				ansST = Parent(select, a, two, q);
			}
			else if (!isNum1 && isNum2) {
				ansST = Parent(select, one, b, q);
			}
			else if (isNum1 && isNum2) {
				ansST = Parent(select, a, b, q);
			}
			else {
				ansST = Parent(select, one, two, q, dFilter);
			}
		}
		else if (command.compare("Parent*") == 0) {
			if (isNum1 && !isNum2) {
				ansST = ParentT(select, a, two, q);
			}
			else if (!isNum1 && isNum2) {
				ansST = ParentT(select, one, b, q);
			}
			else if (isNum1 && isNum2) {
				ansST = ParentT(select, a, b, q);
			}
			else {
				ansST = ParentT(select, one, two, q, dFilter);
			}
		}
		else if (command.compare("Follows") == 0) {
			//cout << "Follows detected!" << endl;
			if (isNum1 && !isNum2) {
				//cout << "Hello" << endl;
				ansST = Follows(select, a, two, q);
			}
			else if (!isNum1 && isNum2) {
				ansST = Follows(select, one, b, q);
			}
			else if (isNum1 && isNum2) {
				ansST = Follows(select, a, b, q);
			}
			else {
				ansST = Follows(select, one, two, q, dFilter);
			}
		}
		else if (command.compare("Follows*") == 0) {
			if (isNum1 && !isNum2) {
				ansST = FollowsT(select, a, two, q);
			}
			else if (!isNum1 && isNum2) {
				ansST = FollowsT(select, one, b, q);
			}
			else if (isNum1 && isNum2) {
				ansST = FollowsT(select, a, b, q);
			}
			else {
				ansST = FollowsT(select, one, two, q, dFilter);
			}
		}
		return ansST;
	}
}

bool QE::isInt(string input) {
	regex integer_rgx("^[0-9]+$");
	return regex_match(input, integer_rgx);
}

vector<string> QE::ModifiesS(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //returns statement number that modifies variable two
	unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (dFilter.first == 0) {
		vector<string> ans;
		vector<string> check;
		int relate = relation(select, one, two);
		if (relate == 1) {
			if (two == "_" || q.checkSynType(two) == "VARIABLE") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
					string str = to_string(i->first);
					ans.push_back(str);
				}
				ans = filter(ans, one, q);
				return ans;
			}
			else {
				size_t found = two.find("\"");
				if (found != std::string::npos) {
					two.erase(remove(two.begin(), two.end(), '\"'), two.end());
					for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
						check = i->second.first;
						for (int j = 0; j < check.size(); ++j) {
							if (check[j].compare(two) == 0) {
								string str = to_string(i->first);
								ans.push_back(str);
							}
						}
					}
				}
				ans = filter(ans, one, q);
				return ans;
			}
		}
		else if (relate == 2) {
			vector<string> modify;
			vector<string> variables;
			for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
				string str = to_string(i->first);
				ans.push_back(str);
			}
			ans = filter(ans, one, q);
			for (int j = 0; j < ans.size(); ++j) {
				int value = atoi(ans[j].c_str());
				modify = modUseTable[value].first;
				for (int k = 0; k < modify.size(); ++k) {
					if (find(variables.begin(), variables.end(), modify[k]) != variables.end())
						continue;
					else {
						variables.push_back(modify[k]);
					}
				}
			}
			return variables;
		}
		else {
			bool status;
			if (two == "_" || q.checkSynType(two) == "VARIABLE") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
					string str = to_string(i->first);
					ans.push_back(str);
				}
				ans = filter(ans, one, q);
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
			}
			else {
				size_t found = two.find("\"");
				if (found != std::string::npos) {
					two.erase(remove(two.begin(), two.end(), '\"'), two.end());
					for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
						check = i->second.first;
						for (int j = 0; j < check.size(); ++j) {
							if (check[j].compare(two) == 0) {
								string str = to_string(i->first);
								ans.push_back(str);
							}
						}
					}
					ans = filter(ans, one, q);
				}
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
			}
			string choice = q.checkSynType(select);
			ans = Choices(choice, status);
			return ans;
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Select v such that Modifies(a, v) through a filter of a
		if (dFilter.first == 1) {
			set<string> varBag;
			for (int i = 0; i < dF.size(); ++i) {
				int stmtNum = stoi(dF[i]);
				vector<string> modEntry = modUseTable[stmtNum].first;
				for (int i = 0; i<modEntry.size(); ++i) {
					varBag.insert(modEntry[i]);
				}
			}
			ans = vector<string>(varBag.begin(), varBag.end());
		}
		// Select <synonym> such that Modifies(<synonym>, v), through a filter of v
		else if (dFilter.first == 2) {
			for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
				vector<string> modEntry = it->second.first;
				for (int i = 0; i < dF.size(); ++i) {
					string var = dF[i];
					if (find(modEntry.begin(), modEntry.end(), var) != modEntry.end()) {
						ans.push_back(to_string(it->first));
						break;
					}
				}
			}
			ans = filter(ans, select, q);
		}
		return ans;
	}
}
vector<string> QE::ModifiesS(string select, int one, string two, Query q) { //returns variable that is modifies in statement line one
	unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	vector<string> modify = modUseTable[one].first;
	int relate = relation(select, to_string(one), two);
	if (relate == 2) {
		for (int i = 0; i < modify.size(); ++i) {
			ans.push_back(modify[i]);
		}
		return ans;
	}
	else {
		bool status = false;
		if (two == "_" || q.checkSynType(two) == "VARIABLE") {
			status = true;
		}
		else {
			size_t found = two.find("\"");
			if (found != std::string::npos) {
				two.erase(remove(two.begin(), two.end(), '\"'), two.end());
			}
			for (int i = 0; i < modify.size(); ++i) {
				if (modify[i] == two) {
					status = true;
					break;
				}
			}
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}

vector<string> QE::UsesS(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //returns statment numbers that uses variable two
	unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (dFilter.first == 0) {
		vector<string> ans;
		vector<string> use;
		vector<string> check;
		int relate = relation(select, one, two);
		if (relate == 1) {
			if (two == "_") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
					string str = to_string(i->first);
					ans.push_back(str);
				}
				ans = filter(ans, one, q);
				return ans;
			}
			else if (q.checkSynType(two) == "CONSTANT") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); i++) {
					check = i->second.second;
					for (int j = 0; j < check.size(); ++j) {
						if (isInt(check[j])) {
							string str = to_string(i->first);
							ans.push_back(str);
						}
					}
				}
				ans = filter(ans, one, q);
				return ans;
			}
			else if (q.checkSynType(two) == "VARIABLE") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); i++) {
					check = i->second.second;
					for (int j = 0; j < check.size(); ++j) {
						if (!isInt(check[j])) {
							string str = to_string(i->first);
							ans.push_back(str);
						}
					}
				}
				ans = filter(ans, one, q);
				return ans;
			}
			else {
				size_t found = two.find('\"');
				if (found != std::string::npos) {
					two.erase(remove(two.begin(), two.end(), '\"'), two.end());
					for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
						check = i->second.second;
						for (int j = 0; j < check.size(); ++j) {
							if (check[j].compare(two) == 0) {
								string str = to_string(i->first);
								ans.push_back(str);
							}
						}
					}
				}
				ans = filter(ans, one, q);
				return ans;
			}
		}
		else if (relate == 2) {
			vector<string> variables;
			for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
				string str = to_string(i->first);
				ans.push_back(str);
			}
			ans = filter(ans, one, q);
			for (int j = 0; j < ans.size(); ++j) {
				int value = atoi(ans[j].c_str());
				use = modUseTable[value].second;
				for (int k = 0; k < use.size(); ++k) {
					if (find(variables.begin(), variables.end(), use[k]) != variables.end())
						continue;
					else {
						if (q.checkSynType(two) == "VARIABLE") {
							if (isInt(use[k])) {
								continue;
							}
							else {
								variables.push_back(use[k]);
							}
						}
						else {
							if (isInt(use[k])) {
								variables.push_back(use[k]);
							}
							else {
								continue;
							}
						}
					}
				}
			}
			return variables;
		}
		else {
			bool status;
			if (one == "_" && two == "_") {
				status = true;
			}
			else if (two == "_" || q.checkSynType(two) == "VARIABLE") {
				for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
					string str = to_string(i->first);
					ans.push_back(str);
				}
				ans = filter(ans, one, q);
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
			}
			else {
				size_t found = two.find("\"");
				if (found != std::string::npos) {
					two.erase(remove(two.begin(), two.end(), '\"'), two.end());
					for (auto i = modUseTable.begin(); i != modUseTable.end(); ++i) {
						check = i->second.second;
						for (int j = 0; j < check.size(); ++j) {
							if (check[j].compare(two) == 0) {
								string str = to_string(i->first);
								ans.push_back(str);
							}
						}
					}
					ans = filter(ans, one, q);
				}
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
			}
			string choice = q.checkSynType(select);
			ans = Choices(choice, status);
			return ans;
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Select [v or c] such that Uses(a, [v or c]) through a filter of a
		if (dFilter.first == 1) {
			if (q.checkSynType(select) == "VARIABLE") {
				set<string> varBag;
				for (int i = 0; i < dF.size(); ++i) {
					int stmtNum = stoi(dF[i]);
					vector<string> useEntry = modUseTable[stmtNum].second;
					for (int i = 0; i<useEntry.size(); ++i) {
						if (!isInt(useEntry[i])) {
							varBag.insert(useEntry[i]);
						}
					}
				}
				ans = vector<string>(varBag.begin(), varBag.end());
			}
			else if (q.checkSynType(select) == "CONSTANT") {
				set<string> constBag;
				for (int i = 0; i < dF.size(); ++i) {
					int stmtNum = stoi(dF[i]);
					vector<string> useEntry = modUseTable[stmtNum].second;
					for (int i = 0; i<useEntry.size(); ++i) {
						if (isInt(useEntry[i])) {
							constBag.insert(useEntry[i]);
						}
					}
				}
				ans = vector<string>(constBag.begin(), constBag.end());
			}
		}
		// Select <synonym> such that Uses(<synonym>, v), through a filter of v
		else if (dFilter.first == 2) {
			for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
				vector<string> useEntry = it->second.second;
				for (int i = 0; i < dF.size(); ++i) {
					string var = dF[i];
					if (find(useEntry.begin(), useEntry.end(), var) != useEntry.end()) {
						ans.push_back(to_string(it->first));
						break;
					}
				}
			}
			ans = filter(ans, select, q);
		}
		return ans;
	}
}
vector<string> QE::UsesS(string select, int one, string two, Query q) { //returns variables that are used in statement line one
	unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	vector<string> use = modUseTable[one].second;
	vector<string> varList = pkb->getVarList()->getAllVar();
	int relate = relation(select, to_string(one), two);
	if (relate == 2) {
		for (int i = 0; i < use.size(); ++i) {
			if (q.checkSynType(two) == "VARIABLE") {
				for (int j = 0; j < varList.size(); ++j) {
					if (varList[j] == use[i]) {
						ans.push_back(use[i]);
					}
				}
			}
			else if (q.checkSynType(two) == "CONSTANT") {
				for (int k = 0; k < varList.size(); ++k) {
					if (varList[k] == use[i]) {
						break;
					}
					if (k == varList.size() - 1) {
						ans.push_back(use[i]);
					}
				}
			}
		}
		return ans;
	}
	else {
		bool status = false;
		if (two == "_" || q.checkSynType(two) == "VARIABLE" || q.checkSynType(two) == "CONSTANT") {
			status = true;
		}
		else {
			size_t found = two.find("\"");
			if (found != std::string::npos) {
				two.erase(remove(two.begin(), two.end(), '\"'), two.end());
			}
			for (int i = 0; i < use.size(); ++i) {
				if (use[i] == two) {
					status = true;
					break;
				}
			}
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}

vector<string> QE::Parent(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //return all the while statement base on condition
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (dFilter.first == 0) {
		vector<string> sub;
		vector<string> ans;
		bool status = false;
		int relate = relation(select, one, two);
		if (relate == 1) {
			if (q.checkSynType(one) == "WHILE" || one == "_" || q.checkSynType(one) == "STATEMENT") { //if one is not while, return none
				for (int i = 0; i < parTable.size(); ++i) {	//select all the child statements of one	
					string str = to_string(parTable[i].second);
					sub.push_back(str);
				}
				sub = filter(sub, two, q); // sub contains only the correct declaration of two
				for (int j = 0; j < parTable.size(); ++j) { //comparing sub and child of parTable to obtain parent statement line
					for (int k = 0; k < sub.size(); ++k) {
						int value = atoi(sub[k].c_str()); //int of sub
						if (parTable[j].second == value) {
							string str = to_string(parTable[j].first);
							if (find(ans.begin(), ans.end(), str) != ans.end()) {
								continue; //parent statement line already exist in ans
							}
							else {
								ans.push_back(str); //adding parent statement line that does not exist in ans
							}
						}
					}
				}
				return ans;
			}
			else { //if one is not while or _, return none
				return ans;
			}
		}
		else {
			if (q.checkSynType(one) == "WHILE" || one == "_" || q.checkSynType(one) == "STATEMENT") {
				for (int i = 0; i < parTable.size(); ++i) {
					string str = to_string(parTable[i].second);
					ans.push_back(str);
				}
				ans = filter(ans, two, q);
			}
			else {
				return ans;
			}
		}
		if (relate == 2) {
			return ans;
		}
		else {
			if (ans.size() > 0) {
				status = true;
			}
			else {
				status = false;
			}
			string choice = q.checkSynType(select);
			ans = Choices(choice, status);
			return ans;
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Impossible as the LHS of a Parent query cannot be an assignment
		if (dFilter.first == 1) {

		}
		// Select [w or s] such that Parent([w or s], a) pattern a("x", _)
		else if (dFilter.first == 2) {
			set<string> stmtNumBag;
			for (int i = 0; i < dF.size(); ++i) {	
				for (auto it = parTable.begin(); it != parTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (it->second == stmtNum) {
						stmtNumBag.insert(to_string(it->first));
						break;
					}
				}
			}
			ans = vector<string>(stmtNumBag.begin(), stmtNumBag.end());
		}
		return ans;
	}
}

vector<string> QE::Parent(string select, int one, string two, Query q) { //return all the child of statement line one
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	int relate = relation(select, to_string(one), two);
	if (relate == 2) {
		for (int i = 0; i < parTable.size(); ++i) {
			if (parTable[i].first == one) {
				string str = to_string(parTable[i].second);
				ans.push_back(str);
			}
		}
		ans = filter(ans, two, q);
		return ans;
	}
	else {
		bool status = false;
		if (two == "_") {
			status = true;
		}
		else {
			for (int i = 0; i < parTable.size(); ++i) {
				if (parTable[i].first == one) {
					string str = to_string(parTable[i].second);
					ans.push_back(str);
				}
			}
			ans = filter(ans, two, q);
			if (ans.size() > 0) {
				status = true;
			}
			else {
				status = false;
			}
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}
vector<string> QE::Parent(string select, string one, int two, Query q) { //return the parent statement line of the statement line two
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	bool status = false;
	int relate = relation(select, one, to_string(two));
	for (int i = 0; i < parTable.size(); i++) {
		if (parTable[i].second == two) {
			string str = to_string(parTable[i].first);
			ans.push_back(str);
			break;
		}
	}
	ans = filter(ans, one, q);
	if (relate == 1) {
		return ans;
	}
	else {
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}

vector<string> QE::Parent(string select, int one, int two, Query q) { // select does not have any relation to fields
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	bool status = false;
	for (int i = 0; i < parTable.size(); i++) {
		if (parTable[i].first == one && parTable[i].second == two) {
			status = true;
			break;
		}
	}
	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;
}

vector<string> QE::ParentT(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //return all the while statment base on condition
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (dFilter.first == 0) {
		vector<string> sub;
		vector<string> ans;
		bool status = false;
		int relate = relation(select, one, two);
		if (relate == 1) {
			if (q.checkSynType(one) == "WHILE" || one == "_" || q.checkSynType(one) == "STATEMENT") {
				for (int i = 0; i < parTable.size(); ++i) {	//select all the child statements of one	
					string str = to_string(parTable[i].second);
					sub.push_back(str);
				}
				sub = filter(sub, two, q); // sub contains only the correct declaration of two
				for (int j = 0; j < parTable.size(); ++j) { //comparing sub and child of parTable to obtain parent statement line
					for (int k = 0; k < sub.size(); ++k) {
						int value = atoi(sub[k].c_str()); //int of sub
						if (parTable[j].second == value) {
							string str = to_string(parTable[j].first);
							if (find(ans.begin(), ans.end(), str) != ans.end()) {
								continue; //parent statement line already exist in ans
							}
							else {
								ans.push_back(str); //adding parent statement line that does not exist in ans
							}
						}
					}
				}
				return ans;
			}
			else {
				return ans;
			}
		}
		else {
			if (q.checkSynType(one) == "WHILE" || one == "_" || q.checkSynType(one) == "STATEMENT") {
				for (int i = 0; i < parTable.size(); ++i) {
					string str = to_string(parTable[i].second);
					ans.push_back(str);
				}
				ans = filter(ans, two, q);
			}
			else {
				return ans;
			}
			if (relate == 2) {
				return ans;
			}
			else {
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
				string choice = q.checkSynType(select);
				ans = Choices(choice, status);
				return ans;
			}
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Impossible as the LHS of a Parent query cannot be an assignment
		if (dFilter.first == 1) {

		}
		// Select [w or s] such that Parent([w or s], a) pattern a("x", _)
		else if (dFilter.first == 2) {
			int prevSize = 0;
			set<int> stmtNumBag;
			set<int> temp;
			for (int i = 0; i < dF.size(); ++i) {
				for (auto it = parTable.begin(); it != parTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (it->second == stmtNum) {
						stmtNumBag.insert(it->first);
						break;
					}
				}
			}
			while (true) {
				for (set<int>::iterator it = stmtNumBag.begin(); it != stmtNumBag.end(); ++it) {
					int stmtNum = *it;
					for (auto iter = parTable.begin(); iter != parTable.end(); ++iter) {
						if (iter->second == stmtNum) {
							temp.insert(iter->first);
							break;
						}
					}
				}
				if (temp.size() > prevSize) {
					prevSize = temp.size();
					stmtNumBag.insert(temp.begin(), temp.end());
				}
				else {
					break;
				}
			}
			for (auto it = stmtNumBag.begin(); it != stmtNumBag.end(); ++it) {
				ans.push_back(to_string(*it));
			}
		}
		return ans;
	}
}
vector<string> QE::ParentT(string select, int one, string two, Query q) { //return all the child* of statement line one
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	bool status = false;
	vector<string> ans;
	stack<int> par;
	int relate = relation(select, to_string(one), two);
	int next = one;
	while (true) {
		for (int i = 0; i < parTable.size(); ++i) {
			if (parTable[i].first == next) {
				par.push(parTable[i].second);
				string str = to_string(parTable[i].second);
				ans.push_back(str);
			}
		}
		if (par.empty()) {
			break;
		}
		else {
			next = par.top();
			par.pop();
		}
	}
	ans = filter(ans, two, q);
	if (relate == 2) {
		return ans;
	}
	else {
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}
vector<string> QE::ParentT(string select, string one, int two, Query q) { //return all the parent* of statement line two
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	stack<int> par;
	bool status = false;
	int relate = relation(select, one, to_string(two));
	int previous = two;
	while (true) {
		for (int i = 0; i < parTable.size(); ++i) {
			if (parTable[i].second == previous) {
				par.push(parTable[i].first);
				string str = to_string(parTable[i].first);
				ans.push_back(str);
			}
		}
		if (par.empty()) {
			break;
		}
		else {
			previous = par.top();
			par.pop();
		}
	}
	ans = filter(ans, one, q);
	if (relate == 1) {
		return ans;
	}
	else {
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}
vector<string> QE::ParentT(string select, int one, int two, Query q) { //select has no relation with field
	vector<pair<int, int>> parTable = pkb->getParentTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> ans;
	stack<int> par; 
	bool status = false;
	int next = one;
	while (true) {
		for (int i = 0; i < parTable.size(); ++i) {
			if (parTable[i].first == next) {
				par.push(parTable[i].second);
				string str = to_string(parTable[i].second);
				ans.push_back(str);
			}
		}
		if (par.empty()) {
			break;
		}
		else {
			next = par.top();
			par.pop();
		}
	}
	for (int j = 0; j < ans.size(); ++j) {
		int value = atoi(ans[j].c_str());
		if (value == two) {
			status = true;
			break;
		}
	}
	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;	
}

vector<string> QE::Follows(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //return statement line of follow base on condition
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	if (dFilter.first == 0) {
		vector<string> sub;
		vector<string> sub2;
		vector<string> ans;
		int relate = relation(select, one, two);
		bool status = false;
		if (relate == 1) {
			for (int i = 0; i < folTable.size(); ++i) {
				string str = to_string(folTable[i].first);
				sub.push_back(str);
			}
			sub = filter(sub, one, q); // sub contains only the correct declaration of one
			for (int j = 0; j < folTable.size(); ++j) { //comparing sub and second of parTable to obtain follow statement line
				for (int k = 0; k < sub.size(); ++k) {
					int value = atoi(sub[k].c_str()); //int of sub
					if (folTable[j].first == value) {
						string str = to_string(folTable[j].second);
						sub2.push_back(str);
					}
				}
			}
			sub2 = filter(sub2, two, q);
			for (int a = 0; a < folTable.size(); ++a) { //comparing sub2 and first of parTable to obtain follow statement line
				for (int b = 0; b < sub2.size(); ++b) {
					int value = atoi(sub2[b].c_str()); //int of sub2
					if (folTable[a].second == value) {
						string str = to_string(folTable[a].first);
						ans.push_back(str);
					}
				}
			}
			return ans;
		}
		else {
			for (int i = 0; i < folTable.size(); ++i) {
				string str = to_string(folTable[i].second);
				sub.push_back(str);
			}
			sub = filter(sub, two, q); // sub contains only the correct declaration of second
			for (int j = 0; j < folTable.size(); ++j) { //comparing sub and first of parTable to obtain follow statement line
				for (int k = 0; k < sub.size(); ++k) {
					int value = atoi(sub[k].c_str()); //int of sub
					if (folTable[j].second == value) {
						string str = to_string(folTable[j].first);
						sub2.push_back(str);
					}
				}
			}
			sub2 = filter(sub2, one, q);
			for (int a = 0; a < folTable.size(); ++a) { //comparing sub2 and second of parTable to obtain follow statement line
				for (int b = 0; b < sub2.size(); ++b) {
					int value = atoi(sub2[b].c_str()); //int of sub
					if (folTable[a].first == value) {
						string str = to_string(folTable[a].second);
						ans.push_back(str);
					}
				}
			}
			if (relate == 2) {
				return ans;
			}
			else {
				if (ans.size() > 0) {
					status = true;
				}
				else {
					status = false;
				}
				string choice = q.checkSynType(select);
				ans = Choices(choice, status);
				return ans;
			}
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Select <synonym> such that Follows(a, <synonym>) through a filter of a
		if (dFilter.first == 1) {
			set<string> stmtNumBag;
			for (int i = 0; i < dF.size(); ++i) {
				for (auto it = folTable.begin(); it != folTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (stmtNum == it->first) {
						stmtNumBag.insert(to_string(it->second));
						break;
					}
				}
			}
			ans = vector<string>(stmtNumBag.begin(), stmtNumBag.end());
		}
		// Select <synonym> such that Follows(<synonym>, a) through a filter of a
		else if (dFilter.first == 2) {
			set<string> stmtNumBag;
			for (int i = 0; i < dF.size(); ++i) {
				for (auto it = folTable.begin(); it != folTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (stmtNum == it->second) {
						stmtNumBag.insert(to_string(it->first));
						break;
					}
				}
			}
			ans = vector<string>(stmtNumBag.begin(), stmtNumBag.end());
		}
		ans = filter(ans, select, q);
		return ans;
	}
}
vector<string> QE::Follows(string select, int one, string two, Query q) { //return the statement line that follows statement line one
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;
	int relate = relation(select, to_string(one), two);
	bool status = false;

	for (int i = 0; i < folTable.size(); ++i) {
		if (folTable[i].first == one) {
			string str = to_string(folTable[i].second);
			ans.push_back(str);
			break;
		}
	}
	ans = filter(ans, two, q);
	if(relate == 2){
		return ans;
	}
	else {
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
	}
	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;
}
vector<string> QE::Follows(string select, string one, int two, Query q) { //return the statement line that is before statement line two
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;
	bool status = false;
	int relate = relation(select, one, to_string(two));
	for (int i = 0; i < folTable.size(); i++) {
		if (folTable[i].second == two) {
			string str = to_string(folTable[i].first);
			ans.push_back(str);
			break;
		}
	}
	ans = filter(ans, one, q);
	if (relate == 1) {
		return ans;
	}
	else {
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
	}
	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;
}
vector<string> QE::Follows(string select, int one, int two, Query q) { // select has no relation to field
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;
	bool status = false;

	for (int i = 0; i < folTable.size(); ++i) {
		if (folTable[i].first == one) {
			if (folTable[i].second == two) {
				status = true;
				break;
			}
		}
	}
	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;
}


vector<string> QE::FollowsT(string select, string one, string two, Query q, pair<int, vector<string>> dFilter) { //return all the statement lines base on condition
	int relate = relation(select, one, two);

	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (dFilter.first == 0) {
		set<int> set;
		vector<string> ans;
		string type1 = q.checkSynType(one);
		string type2 = q.checkSynType(two);

		/*
		int current = folTable[0].first;

		for (int i = 0; i < folTable.size(); ++i) {

		//cout << folTable[i].first << "//" << folTable[i].second << "   ";

		if (folTable[i].first == current) {

		// check for a and w on the right
		// ignore for _

		int next = folTable[i].second;


		// we didn't get a match so we move on instead of adding to our answer
		if (next == two) {
		status = true;
		}

		current = folTable[i].second;

		}
		}

		*/

		// e.g Select a Follows(w,a)
		// follow same format as e,g 2,a
		// double for loops
		// outer loop to loop each possible left statement and skip those that are not the same type as type 1
		// inner loop same as e.g 2,a


		if (select.compare(two) == 0) {

			for (int i = 0; i < folTable.size(); ++i) {

				int current = folTable[i].first;


				// only go ahead if same type	
				if (type1.compare(stmtTable[current].getType()) == 0 || type1.compare("STATEMENT") == 0 || one.compare("_") == 0) {

					for (int j = i; j < folTable.size(); ++j) {

						//cout << folTable[j].first << "//" << folTable[j].second << "   ";


						// check for a and w on the right
						// ignore for _

						int next = folTable[j].second;

						// make sure not _
						if (one.compare("_") != 0) {

							// we didn't get a match so we move on instead of adding to our answer
							if ((type2.compare(stmtTable[next].getType()) == 0 && folTable[j].first == current) || type2.compare("STATEMENT") == 0) {

								//cout << "//" << current << "   " << to_string(folTable[j].second) << " ";

								set.insert(folTable[j].second);

								current = folTable[j].second;

							}
							else if (folTable[j].first == current) {
								current = folTable[j].second;
							}

						}
						else {
							set.insert(folTable[j].second);

						}

					}

				}


			}

		}


		// e.g a,6 same as select a such that a,w
		// or _,w
		if (select.compare(one) == 0) {

			for (int i = folTable.size() - 1; i >= 0; --i) {

				int current = folTable[i].second;


				// only go ahead if same type	
				if (type2.compare(stmtTable[current].getType()) == 0 || type2.compare("STATEMENT") == 0 || two.compare("_") == 0) {

					for (int j = i; j >= 0; --j) {

						//cout << folTable[j].first << "//" << folTable[j].second << "   ";


						// check for a and w on the right
						// ignore for _

						int next = folTable[j].first;

						// make sure not _
						if (true) {
							// we didn't get a match so we move on instead of adding to our answer
							if ((type1.compare(stmtTable[next].getType()) == 0 && folTable[j].second == current) || type1.compare("STATEMENT") == 0) {

								//cout << "//" << current << "   " << to_string(folTable[j].second) << " ";

								set.insert(folTable[j].first);

								current = folTable[j].first;

							}

						}
						else {
							set.insert(folTable[j].first);

						}


					}

				}


			}

		}


		cout << one << select << two;

		if ((one.compare("_") == 0 || q.checkSynType(one) == "STATEMENT") && (two.compare("_") == 0 || q.checkSynType(two) == "STATEMENT")) {
			set.insert(1);
		}
		else {
			if (select.compare(one) != 0 && select.compare(two) != 0) {

				for (int i = 0; i < folTable.size(); ++i) {

					int current = folTable[i].first;

					if (one.compare("_") != 0) {

						// only go ahead if same type	
						if (type1.compare(stmtTable[current].getType()) == 0 || type1.compare("STATEMENT") == 0) {

							for (int j = i; j < folTable.size(); ++j) {

								//cout << folTable[j].first << "//" << folTable[j].second << "   ";


								// check for a and w on the right
								// ignore for _

								int next = folTable[j].second;

								// make sure not _
								if (two.compare("_") != 0) {

									// we didn't get a match so we move on instead of adding to our answer
									if ((type2.compare(stmtTable[next].getType()) == 0 && folTable[j].first == current) || type2.compare("STATEMENT") == 0) {

										//cout << "//" << current << "   " << to_string(folTable[j].second) << " ";

										set.insert(folTable[j].second);

										current = folTable[j].second;
									}

								}
								else {
									set.insert(folTable[j].second);
								}

							}

						}


					}
					else {
						set.insert(1);
					}



				}

			}
		}


		while (!set.empty()) {
			string str = to_string(*set.begin());

			ans.push_back(str);
			set.erase(set.begin());
		}
		if (relate != 0) {
			return ans;
		}
		else {
			bool status;
			if (ans.size() > 0) {
				status = true;
			}
			else {
				status = false;
			}
			string choice = q.checkSynType(select);
			ans = Choices(choice, status);
			return ans;
		}
	}
	else {
		vector<string> ans;
		vector<string> dF = dFilter.second;
		// Select <synonym> such that Follows*(a, <synonym>) through a filter of a
		if (dFilter.first == 1) {
			int prevSize = 0;
			set<string> stmtNumBag;
			set<string> temp;
			for (int i = 0; i < dF.size(); ++i) {
				for (auto it = folTable.begin(); it != folTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (it->first == stmtNum) {
						stmtNumBag.insert(to_string(it->second));
						break;
					}
				}
			}
			while (true) {
				for (auto it = stmtNumBag.begin(); it != stmtNumBag.end(); ++it) {
					string stmtNum = *it;
					for (auto iter = folTable.begin(); iter != folTable.end(); ++iter) {
						if (stmtNum == to_string(iter->first)) {
							temp.insert(to_string(iter->second));
							break;
						}
					}
				}
				if (temp.size() > prevSize) {
					prevSize = temp.size();
					stmtNumBag.insert(temp.begin(), temp.end());
				}
				else {
					break;
				}
			}
			ans = vector<string>(stmtNumBag.begin(), stmtNumBag.end());
		}
		// Select <synonym> such that Follows*(<synonym>, a) through a filter of a
		else if (dFilter.first == 2) {
			int prevSize = 0;
			set<string> stmtNumBag;
			set<string> temp;
			for (int i = 0; i < dF.size(); ++i) {
				for (auto it = folTable.begin(); it != folTable.end(); ++it) {
					int stmtNum = stoi(dF[i]);
					if (it->second == stmtNum) {
						stmtNumBag.insert(to_string(it->first));
						break;
					}
				}
			}
			while (true) {
				for (auto it = stmtNumBag.begin(); it != stmtNumBag.end(); ++it) {
					string stmtNum = *it;
					for (auto iter = folTable.begin(); iter != folTable.end(); ++iter) {
						if (stmtNum == to_string(iter->second)) {
							temp.insert(to_string(iter->first));
							break;
						}
					}
				}
				if (temp.size() > prevSize) {
					prevSize = temp.size();
					stmtNumBag.insert(temp.begin(), temp.end());
				}
				else {
					break;
				}
			}
			ans = vector<string>(stmtNumBag.begin(), stmtNumBag.end());
		}
		ans = filter(ans, select, q);
		return ans;
	}
}
vector<string> QE::FollowsT(string select, int one, string two, Query q) { //return all the statement lines that follows* statement line one

	set<int> set;
	// e.g Follows* 2,_ and 3,a and 4,w 
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;
	int relate = relation(select, to_string(one), two);
	int current = one;

	string type = q.checkSynType(two);

	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();

	for (int i = 0; i < folTable.size(); ++i) {

		cout << folTable[i].first << "//" << folTable[i].second << "   ";

		if (folTable[i].first == current) {

			// check for a and w on the right
			// ignore for _

			int next = folTable[i].second;

			// make sure not _
			if (two.compare("_") != 0) {

				// we didn't get a match so we move on instead of adding to our answer
				if (type.compare(stmtTable[next].getType()) == 0 || type.compare("STATEMENT") == 0) {

					//ans.push_back(str);
					set.insert(folTable[i].second);

				}

			}
			else {
				// just add since _
				set.insert(folTable[i].second);

			}

			current = folTable[i].second;

		}
	}

	while (!set.empty()) {
		string str = to_string(*set.begin());

		ans.push_back(str);
		set.erase(set.begin());
	}


	if (relate != 0) {
		return ans;
	}
	else {
		bool status;
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}
vector<string> QE::FollowsT(string select, string one, int two, Query q) { //return all the statements lines that come before statement line two and in the sames nesting level
	int relate = relation(select, one, to_string(two));
	set<int> set;
	// e.g Follows* 2,_ and 3,a and 4,w 
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;

	int current = two;

	string type = q.checkSynType(one);

	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();

	// a,2 _,3 w,4 so we do from the back instead


	for (int i = folTable.size() - 1; i >= 0; --i) {

		if (folTable[i].second == current) {

			int next = folTable[i].first;

			// check for a and w on the right
			// ignore for _

			// make sure not _
			if (one.compare("_") != 0) {

				// we get a match so we move on instead of adding to our answer
				if (type.compare(stmtTable[next].getType()) == 0 || type.compare("STATEMENT") == 0) {

					//ans.push_back(str);
					set.insert(folTable[i].first);
				}

			}
			else {
				// just add since _
				set.insert(folTable[i].first);
			}


			current = folTable[i].first;

		}
	}

	while (!set.empty()) {
		string str = to_string(*set.begin());

		ans.push_back(str);
		set.erase(set.begin());
	}


	if (relate != 0) {
		return ans;
	}
	else {
		bool status;
		if (ans.size() > 0) {
			status = true;
		}
		else {
			status = false;
		}
		string choice = q.checkSynType(select);
		ans = Choices(choice, status);
		return ans;
	}
}

vector<string> QE::FollowsT(string select, int one, int two, Query q) { //return all the statements lines that come before statement line two and in the sames nesting level
	vector<pair<int, int>> folTable = pkb->getFollowTable()->getTable();
	vector<string> ans;

	int current = one;

	bool status = false;

	for (int i = 0; i < folTable.size(); ++i) {

		//cout << folTable[i].first << "//" << folTable[i].second << "   ";

		if (folTable[i].first == current) {

			// check for a and w on the right
			// ignore for _

			int next = folTable[i].second;


			// we didn't get a match so we move on instead of adding to our answer
			if (next == two) {
				status = true;
			}

			current = folTable[i].second;

		}
	}

	string choice = q.checkSynType(select);
	ans = Choices(choice, status);
	return ans;

}


vector<string> QE::pattern(string select, string command, string one, string two, Query q, pair<int, vector<string>> dFilter) { //return the statement lines that has this pattern
	unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
	bool isUnderscore = false;
	bool returnAllFlag = false;
	vector<string> ans;
	//If this query was not dependent on the other conditions
	if (dFilter.first == 0) {
		//check if LHS is '_' or variable synonym v, which have the same meaning
		string type = q.checkSynType(one);
		if (one == "_" || type == "VARIABLE") {
			isUnderscore = true;
		}
		// "one" is a variable
		else {
			for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
				// strip the apostophes from one, assuming var is surrounded by " and "
				string var = one.substr(1, one.size() - 2);
				vector<string> modEntry = it->second.first;
				if (find(modEntry.begin(), modEntry.end(), var) != modEntry.end()) {
					ans.push_back(to_string(it->first));
				}
			}
		}
		//check if both sides are "_"
		if (two == "_" && isUnderscore) {
			returnAllFlag = true;
		}
		// if LHS was "_", and RHS is a sub expression
		else if (isUnderscore) {
			//take out the var in the middle, assuming it is surrounded by _" and "_
			string var = two.substr(2, two.size() - 4);
			for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
				vector<string> useEntry = it->second.second;
				if (find(useEntry.begin(), useEntry.end(), var) != useEntry.end()) {
					ans.push_back(to_string(it->first));
				}
			}
		}
		//if RHS is a subexpression and LHS was not "_"
		else if ((two.find("_", 0) != string::npos) && (two.find("_", 1) != string::npos) && (!isUnderscore)) {
			//take out the var in the middle, assuming it is surrounded by _" and "_
			//cout << "check" << endl;
			string var = two.substr(2, two.size() - 4);
			for (int i = 0; i < ans.size(); ++i) {
				int stmtNum = stoi(ans[i]);
				vector<string> useEntry = modUseTable[stmtNum].second;
				//if the current stmts in ans do not satisfy RHS pattern, erase them
				if (find(useEntry.begin(), useEntry.end(), var) == useEntry.end()) {
					ans.erase(ans.begin() + i);
					--i;
				}
			}
		}
		//if RHS is an expression (1st iteration doesn't have this)
		else {

		}
		//keep only stmts of the assignment type
		ans = findCommonAnswer(ans, getAllType("ASSIGN"));

		//check if synonym to select matches synonyms in condition
		int relate = relation(select, command.substr(7, command.size() - 7), one);

		//special case where "Select v pattern a(v, <anything>)"
		if (relate == 2) {
			set<string> varBag;
			//get all variables from the LHS of all assignment stmts matching the LHS conditions
			if (returnAllFlag) {
				//get all var on LHS of all assign stmts
				ans = getAllType("ASSIGN");
			}
			//get all var on LHS of the marked stmts
			for (int i = 0; i < ans.size(); ++i) {
				int stmtNum = stoi(ans[i]);
				vector<string> modEntry = modUseTable[stmtNum].first;
				for (int j = 0; j < modEntry.size(); ++j) {
					varBag.insert(modEntry[j]);
				}
			}
			ans = vector<string>(varBag.begin(), varBag.end());
		}
		// if not related and conditionals are satisfied, or pattern a(_,_), return all of type 'select'
		else if (relate == 0 && ans.size() != 0 || returnAllFlag) {
			string type = q.checkSynType(select);
			ans = getAllType(type);
		}
	}
	else {
		// Select a such that pattern a(v, <anything>), through a filter of v
		if (dFilter.first == 2) {
			vector<string> allAssign = getAllType("ASSIGN");
			vector<string> dF = dFilter.second;
			if (two == "_") {
				for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
					vector<string> modEntry = it->second.first;
					for (int i = 0; i < dF.size(); ++i) {
						if (find(modEntry.begin(), modEntry.end(), dF[i]) != modEntry.end()) {
							ans.push_back(to_string(it->first));
						}
					}
				}
			}
			else {
				string var = two.substr(2, two.size() - 4);
				for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
					vector<string> modEntry = it->second.first;
					vector<string> useEntry = it->second.second;
					bool matchesLHS = false;
					bool matchesRHS = false;
					if (find(useEntry.begin(), useEntry.end(), var) != useEntry.end()) {
						matchesRHS = true;
					}
					for (int i = 0; i < dF.size(); ++i) {
						if (find(modEntry.begin(), modEntry.end(), dF[i]) != modEntry.end()) {
							matchesLHS = true;
							break;
						}
					}
					if (matchesLHS && matchesRHS) {
						ans.push_back(to_string(it->first));
					}
				}
			}
			ans = findCommonAnswer(ans, allAssign);
		}
		// Select v such that pattern a(v, <anything>), through a filter of a
		else if (dFilter.first == 1) {
			ans = dFilter.second;
			set<string> varBag;
			if (two == "_") {
				for (int i = 0; i < ans.size(); ++i) {
					int stmtNum = stoi(ans[i]);
					vector<string> modEntry = modUseTable[stmtNum].first;
					for (int j = 0; j < modEntry.size(); ++j) {
						varBag.insert(modEntry[j]);
					}
				}
			}
			else {
				string var = two.substr(2, two.size() - 4);
				for (int i = 0; i < ans.size(); ++i) {
					int stmtNum = stoi(ans[i]);
					vector<string> modEntry = modUseTable[stmtNum].first;
					vector<string> useEntry = modUseTable[stmtNum].second;
					if (find(useEntry.begin(), useEntry.end(), var) != useEntry.end()) {
						for (int j = 0; j < modEntry.size(); ++j) {
							varBag.insert(modEntry[j]);
						}
					}
				}
			}
			ans = vector<string>(varBag.begin(), varBag.end());
		}
	}
	return ans;
}

vector<string> QE::getAllType(string type) {
	vector<string> all;
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (type == "STATEMENT" || type == "PROG_LINE") {
		for (auto it = stmtTable.begin(); it != stmtTable.end(); ++it) {
			if (it->first != 0) {
				all.push_back(to_string(it->first));
			}
		}
	}
	else if (type == "ASSIGN" || type == "WHILE") {
		for (auto it = stmtTable.begin(); it != stmtTable.end(); ++it) {
			if (it->second.getType() == type) {
				all.push_back(to_string(it->first));
			}
		}
	}
	else if (type == "VARIABLE") {
		all = pkb->getVarList()->getAllVar();
	}
	else if (type == "CONSTANT") {
		unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
		for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
			vector<string> used = it->second.second;
			for (int i = 0; i < used.size(); ++i) {
				if (isInt(used[i])) {
					all.push_back(used[i]);
				}
			}
		}
	}
	return all;
}

int QE::relation(string select, string one, string two) {
	if (select == one) {
		return 1;
	}
	else if (select == two) {
		return 2;
	}
	else {
		return 0;
	}
}
// return the dependent synonym, for eg, Select v such that Modifies(a, v) pattern a("d", _)
// would return a pair {2, 1}, where the problem lies with the 2nd condition, and it is dependent
// on the 1st synonym in the non-problem condition, "a".
pair<int, int> QE::dependent(string select, string cond1One, string cond1Two, string cond2One, string cond2Two) {
	int cond1Relation = relation(select, cond1One, cond1Two);
	int cond2Relation = relation(select, cond2One, cond2Two);

	//Case where both conditions are not related to select, but are to each other
	if (cond1Relation == 0 && cond2Relation == 0) {
		int var1Relation = relation(cond1One, cond2One, cond2Two);
		int var2Relation = relation(cond1Two, cond2One, cond2Two);
		//Corner case ".. Modifies(1, "y") pattern a("y", _)
		bool is1Ident = cond1One.find("\"", 0) != string::npos;
		bool is2Ident = cond1Two.find("\"", 0) != string::npos;
		if (var1Relation == 0 && var2Relation == 0) {
			return pair<int, int>{0, 0};
		}
		else if (var1Relation == 0 && var2Relation != 0) {
			if (!is2Ident) {
				return pair<int, int>{3, 2};
			}
			else {
				return pair<int, int>{0, 0};
			}

		}
		else if (var1Relation != 0 && var2Relation == 0) {
			if (!is1Ident) {
				return pair<int, int>{3, 1};
			}
			else {
				return pair<int, int>{0, 0};
			}
		}
		// final case where both conditions have 2 common synonym - not allowed in iter1
		else {
			if (is1Ident) {
				return pair<int, int>{3, 2};
			}
			else if (is2Ident) {
				return pair<int, int>{3, 1};
			}
			else {
				return pair<int, int>{4, 0};
			}
		}
	}
	else if (cond1Relation == 0 && cond2Relation != 0) {
		int var1Relation = relation(cond2One, cond1One, cond1Two);
		int var2Relation = relation(cond2Two, cond1One, cond1Two);

		if (var1Relation == 0 && var2Relation == 0) {
			return pair<int, int>{0, 0};
		}
		else if (var1Relation == 0 && var2Relation != 0) {
			return pair<int, int>{1, 2};
		}
		else if (var1Relation != 0 && var2Relation == 0) {
			return pair<int, int>{1, 1};
		}
		// final case where both conditions have 2 common synonym - not allowed in iter1
		else {
			return pair<int, int>{4, 0};
		}
	}
	else if (cond1Relation != 0 && cond2Relation == 0) {
		int var1Relation = relation(cond1One, cond2One, cond2Two);
		int var2Relation = relation(cond1Two, cond2One, cond2Two);

		if (var1Relation == 0 && var2Relation == 0) {
			return pair<int, int>{0, 0};
		}
		else if (var1Relation == 0 && var2Relation != 0) {
			return pair<int, int>{2, 2};
		}
		else if (var1Relation != 0 && var2Relation == 0) {
			return pair<int, int>{2, 1};
		}
		// final case where both conditions have 2 common synonym - not allowed in iter1
		else {
			return pair<int, int>{4, 0};
		}
	}
	else {
		return pair<int, int>{0, 0};
	}
}
vector<string> QE::filter(vector<string> vec, string field, Query q) {
	vector<string> filAns;
	string type = q.checkSynType(field);
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	if (type == "WHILE") {
		for (int i = 0; i < vec.size(); ++i) {
			int value = atoi(vec[i].c_str());
			if (type == stmtTable[value].getType()) {
				filAns.push_back(vec[i]);
			}
		}
	}
	else  if (type == "ASSIGN") {
		for (int i = 0; i < vec.size(); ++i) {
			//obtain token of the statement number and compare with the type in two
			int value = atoi(vec[i].c_str());
			if (type == stmtTable[value].getType()) {
				// add it to final answer
				filAns.push_back(vec[i]);
			}
		}
	}
	else {
		return vec;
	}
	return filAns;
}

vector<string> QE::findCommonAnswer(vector<string> ansST, vector<string> ansP) {
	vector<string> answer;
	for (int a = 0; a < ansST.size(); a++) {
		if (find(ansP.begin(), ansP.end(), ansST.at(a)) != ansP.end()) {
			answer.push_back(ansST.at(a));
		}
	}
	return answer;
}

string QE::vectorSToString(vector<string> vecString) {
	string ans;
	stringstream ss;
	for (int i = 0; i < vecString.size(); ++i) {
		if (i != 0) {
			ss << ", ";
		}
		ss << vecString.at(i);
	}
	ans = ss.str();
	return ans;
}

//getter for the answers for unit testing QE
vector<string> QE::getAnswers() {
	return answers;
}

void QE::displayAllAnswers() {
	cout << answers.size() << endl;
	for (int i = 0; i < answers.size(); ++i) {
		cout << "Statement number is: " << answers.at(i) << endl;
	}
}

vector<string> QE::Choices(string choice, bool status) {
	unordered_map<int, LineToken> stmtTable = pkb->getStatementTable()->getTable();
	vector<string> finAns;
	if (status == true) {
		if (choice == "ASSIGN" || choice == "WHILE") {
			for (auto i = 0; i < stmtTable.size(); ++i){
				if (choice == stmtTable[i].getType()) {
					// add it to final answer
					finAns.push_back(to_string(i));
				}
			}
			return finAns;
		}

		else if (choice == "STATEMENT") {
			for (auto i = 1; i <= stmtTable.size(); ++i) {
				finAns.push_back(to_string(i));
			}
			return finAns;
		}
		else if (choice == "VARIABLE") {
			finAns = pkb->getVarList()->getAllVar();
			return finAns;
		}
		else if (choice == "CONSTANT") {
			unordered_map<int, pair<vector<string>, vector<string>>> modUseTable = pkb->getmodUseTable()->getTable();
			for (auto it = modUseTable.begin(); it != modUseTable.end(); ++it) {
				vector<string> used = it->second.second;
				for (int i = 0; i < used.size(); ++i) {
					if (isInt(used[i])) {
						finAns.push_back(used[i]);
					}
					else {
						continue;
					}
				}
			}
			return finAns;
		}
		else {
			return finAns;
		}
	}
}
bool QE::checkAnswerSize(vector<string> answerVector) {
	if (answerVector.size() == 0) {
		return false;
	}
	else {
		return true;
	}
}

void QE::getQSForQP(string queryString) {
	queryStringForQP = queryString;
}

list<string> QE::convertVectorToList(vector<string> answerVec) {
	list<string> answerList;
	for (int a = 0; a < answerVec.size(); ++a) {
		answerList.push_back(answerVec.at(a));
	}
	return answerList;
}

list<string> QE::getAnswerForSingleQuery() {
	return answerForSingleQuery;
}