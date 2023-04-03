#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "rapidxml.hpp"
#include <map>
#include <vector>
#include <utility>
#include <iostream>
#include <algorithm>
#include "logger.hpp"

using namespace std;
using namespace rapidxml;

class FeatureVisitor {
private:
	// Map of all the variables
	map<string, vector<string>*> variables;
	map<string, int> variableIndex;
	map<int, string> indexVariable;
	vector<int> mandatoryIndex;
	vector<pair<pair<int, int>, vector<pair<int,int>>*>> altIndexesExclusion;
	vector<pair<pair<int, int>, vector<int>*>> orIndexs;
	vector<pair<pair<int, int>, vector<pair<int, int>>*>> orIndexsNonLeaf;
	vector<pair<pair<int, int>, pair<int, int>>> mandatoryImplications;
	vector<pair<pair<int, int>, pair<int, int>>> singleImplications;
	vector<pair<pair<int, int>, pair<int, int>>> singleImplicationsNonLeaf;

	bool ignoreHidden;

	void defineSingleVariable(xml_node<> *node);
	void setMandatory(xml_node<> *node, int indexOfNone, int varIndex);
	bool areChildrenAllLeaf(xml_node<> *node);
	bool isLeaf(xml_node<> *node);
	int getNumChildren(xml_node<> *node);
	void setMandatoryNoParent(rapidxml::xml_node<> *node, int varIndex);
	void setMandatoryImplication(rapidxml::xml_node<> *node, int indexOfNone, int varIndex);
	int getIndexOfNoneForVariable(const std::string &variableName);
	void setDependency(xml_node<> *node);
	void setSingleImplication(rapidxml::xml_node<> *node, int indexOfNone);
	pair<int, int> getIndexOfValue(const string &variableName);

public:
	static int index;

	FeatureVisitor();
	FeatureVisitor(bool ignoreHidden);
	void visit(xml_node<> * node);
	void visitAnd(xml_node<> * node);
	void visitAlt(xml_node<> * node);
	void visitOr(xml_node<> * node);
	void visitFeature(xml_node<> * node);
	void printVariablesInMap();
	int getNVar();
	int* getBounds();
	vector<int> getMandatoryIndex();
	vector<pair<pair<int, int>, vector<int>*>> getOrIndexs();
	vector<pair<pair<int, int>, vector<pair<int, int>>*>> getOrIndexsNonLeaf();
	vector<pair<pair<int, int>, pair<int, int>>> getMandatoryImplications();
	vector<pair<pair<int, int>, pair<int, int>>> getSingleImplications();
	vector<pair<pair<int, int>, pair<int, int>>> getSingleImplicationsNonLeaf();
	vector<pair<pair<int, int>, vector<pair<int,int>>*>> getAltIndexesExclusion();
	string getValueForVar(int indexVar, int indexVal);

	virtual ~FeatureVisitor();

	friend class ConstraintVisitor;
};

#endif
