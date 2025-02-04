#ifndef Query_H
#define Query_H
#pragma once
#include <string>
#include "LineToken.h"
#include <vector>

using namespace std;

class Query
{
private:
	vector<string> stmtD;
	vector<string> assignD;
	vector<string> variableD;
	vector<string> constantD;
	vector<string> whileD;
	vector<string> prog_lineD;
	vector<string> queryFields;

public:
	Query();
	string toString();
	void printQuery();
	Query(vector<string> stmtd, vector<string> assignd, vector<string> variabled, vector<string> constantd, vector<string> whiled, vector<string> prog_lined, vector<string> queryfield);
	string checkSynType(string syn);
	vector<string> getQueryFields();
	~Query();
};

#endif