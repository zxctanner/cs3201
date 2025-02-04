#ifndef ParentTable_H
#define ParentTable_H
#include <vector>

using namespace std;

class ParentTable {
private:
	vector<pair<int, int>> parentTable;
public:
	ParentTable();
	ParentTable(vector<pair<int, int>> pTable);
	~ParentTable();
	//void addEntry(int parent, int child);
	vector<pair<int, int>> getTable();
	void setTable(vector<pair<int, int>> pTable);
};

#endif