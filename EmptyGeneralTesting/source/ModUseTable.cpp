#include "ModUseTable.h"


using namespace std;


ModUseTable::ModUseTable()
{
	modUseTable = unordered_map<int, pair<vector<string>, vector<string>>>();
}

ModUseTable::ModUseTable(unordered_map<int, pair<vector<string>, vector<string>>> mUTable)
{
	modUseTable = mUTable;
}

ModUseTable::~ModUseTable()
{
}

unordered_map<int, pair<vector<string>, vector<string>>> ModUseTable::getTable() {
	return modUseTable;
}

void ModUseTable::setModUseTable(unordered_map<int, pair<vector<string>, vector<string>>> mUTable)
{
	modUseTable = mUTable;
}

