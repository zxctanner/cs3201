#ifndef QE_H
#define QE_H
#include <vector>
#include <string>
#include <list>
#include "ModUseTable.h"
#include "ParentTable.h"
#include "StatementTable.h"
#include "FollowTable.h"
#include "PKB.h"
#include "LineToken.h"
#include "Query.h"
#include "QP.h"
using namespace std;

// token.getType() == ASSIGN
//token.getType() == WHILE

class QE
{
private:
	string filePath;
	string queryStringForQP;
	PKB* pkb;
	vector<string> answers;
	vector<Query> queryVector;
	list<string> answerForSingleQuery;

public:
	QE(PKB* p);
	QE(string fileName, PKB* p);
	QE(vector<Query> vQ, PKB* p);
	
	void getQSForQP(string queryString);
	void evaluateSingleQuery(string queryString);
	list<string> getAnswerForSingleQuery();
	list<string> convertVectorToList(vector<string> answerVec);
	vector<string> selectField(string select, string command, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0, {} });
	bool isInt(string input);
	vector<string> Choices(string choice, bool status);
	vector<string> ModifiesS(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> ModifiesS(string select, int one, string two, Query q);

	vector<string> UsesS(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> UsesS(string select, int one, string two, Query Q);

	vector<string> Parent(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> Parent(string select, int one, string two, Query q);
	vector<string> Parent(string select, string one, int two, Query q);
	vector<string> Parent(string select, int one, int two, Query q);

	vector<string> ParentT(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> ParentT(string select, int one, string two, Query q);
	vector<string> ParentT(string select, string one, int two, Query q);
	vector<string> ParentT(string select, int one, int two, Query q);

	vector<string> Follows(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> Follows(string select, int one, string two, Query q);
	vector<string> Follows(string select, string one, int two, Query q);
	vector<string> Follows(string select, int one, int two, Query q);

	vector<string> FollowsT(string select, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });
	vector<string> FollowsT(string select, int one, string two, Query q);
	vector<string> FollowsT(string select, string one, int two, Query q);
	vector<string> FollowsT(string select, int one, int two, Query q);

	vector<string> pattern(string select, string command, string one, string two, Query q, pair<int, vector<string>> dFilter = pair<int, vector<string>>{ 0,{} });

	vector<string> getAllType(string type);
	int relation(string select, string one, string two);
	pair<int, int> dependent(string select, string cond1One, string cond1Two, string cond2One, string cond2Two);
	vector<string> filter(vector<string> vec, string field, Query q);
	vector<string> findCommonAnswer(vector<string> ansST, vector<string> ansP);
	string vectorSToString(vector<string> vecString);
	vector<string> getAnswers();
	void displayAllAnswers();
	bool checkAnswerSize(vector<string> answerVector);
};

#endif

