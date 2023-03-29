/* * NodeVisitor.cpp
 *
 *  Created on: Mar 24, 2023
 *      Author: bombarda
 */
#include <NodeFeatureVisitor.h>
#include <iostream>
#include <string.h>

using namespace std;
using namespace rapidxml;

int FeatureVisitor::index = 0;

FeatureVisitor::FeatureVisitor() {
	this->ignoreHidden = false;
}

FeatureVisitor::FeatureVisitor(bool ignoreHidden) {
	this->ignoreHidden = ignoreHidden;
}

void FeatureVisitor::visit(xml_node<> *node) {
	logcout(LOG_DEBUG) << "Visiting node "
			<< node->first_attribute("name")->value() << endl;

	if (ignoreHidden && node->first_attribute("hidden")) {
		logcout(LOG_DEBUG) << "\tIgnoring node "
					<< node->first_attribute("name")->value() << " as it is hidden" << endl;
		return;
	}

	// Dispatcher, depending on the node type
	if (strcmp(node->name(), "alt") == 0)
		visitAlt(node);
	else if (strcmp(node->name(), "and") == 0)
		visitAnd(node);
	else if (strcmp(node->name(), "or") == 0)
		visitOr(node);
	else if (strcmp(node->name(), "feature") == 0)
		visitFeature(node);
	else
		throw std::invalid_argument("Invalid node type");
}

bool FeatureVisitor::areChildrenAllLeaf(xml_node<> *node) {
	bool areAllLeaf = true;

	// Visit all the child features
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling())
		if (strcmp(n->name(), "feature") != 0) {
			areAllLeaf = false;
		}
	return areAllLeaf;
}

void FeatureVisitor::visitAlt(xml_node<> *node) {
	bool areAllLeaf = true;

	// Check whether children are all leafs
	areAllLeaf = areChildrenAllLeaf(node);

	if (areAllLeaf) {
		// All the children are leaf -> The alternative variable is an enumerative one
		string varName = node->first_attribute("name")->value();
		int indexOfNone = -1;
		vector<string> *values = new vector<string>;

		// Get the possible values
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling())
			values->push_back(n->first_attribute("name")->value());

		// Add to the possible values also the unselected one
		indexOfNone = values->size();
		values->push_back("NONE");

		// Create the variable
		variables[varName] = values;
		variableIndex[varName] = index;
		indexVariable[index] = varName;

		// VAL = NONE <=> PARENT = NONE
		// Handle the mandatory part of the value. It is mandatory only if
		// the parent has been selected
		setMandatory(node, indexOfNone, index);
		setDependency(node);

		// Increase the index
		index++;
	} else {
		// The alternative variable has to be managed as a boolean one (i.e., as an and)
		visitAnd(node);
	}
}

void FeatureVisitor::visitOr(xml_node<> *node) {
	bool areAllLeaf;

	// Check whether children are all leafs
	areAllLeaf = areChildrenAllLeaf(node);

	if (areAllLeaf) {
		// If all the n children are leafs, it is enough to create n variables (one for each child)
		// and add a constraint stating that one of them must be selected, plus an additional
		// boolean variable for the parent feature
		visitFeature(node);
		vector<int> *orIndex = new vector<int>;

		// Visit all the children features
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			orIndex->push_back(index);
			visit(n);
		}

		int indexOfNone = getIndexOfNoneForVariable(
				node->first_attribute("name")->value());

		// Add the orIndex list to the list stored in the FeatureVisitor
		orIndexs.push_back(
				make_pair(
						make_pair(
								variableIndex[node->first_attribute("name")->value()],
								indexOfNone), orIndex));
	} else {
		// The feature is converted in a boolean variable
		visitFeature(node);
		vector<pair<int, int>> *orIndex = new vector<pair<int, int>>;
		int indexOfNone = -1;
		int varIndex = -1;

		// Then all the children are visited and their index is stored
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			varIndex = index;
			visit(n);
			indexOfNone = getIndexOfNoneForVariable(indexVariable[varIndex]);
			orIndex->push_back(make_pair(varIndex, indexOfNone));
		}

		indexOfNone = getIndexOfNoneForVariable(
				node->first_attribute("name")->value());
		varIndex = variableIndex[node->first_attribute("name")->value()];

		// Add the orIndex list to the list stored in the FeatureVisitor
		orIndexsNonLeaf.push_back(
				make_pair(make_pair(varIndex, indexOfNone), orIndex));
	}
}

void FeatureVisitor::defineSingleVariable(xml_node<> *node) {
	// Define the current variable, which is a boolean variable
	vector<string> *values = new vector<string>;
	values->push_back("false");
	values->push_back("true");
	variables[node->first_attribute("name")->value()] = values;
	variableIndex[node->first_attribute("name")->value()] = index;
	indexVariable[index] = node->first_attribute("name")->value();
}

void FeatureVisitor::setMandatoryNoParent(xml_node<> *node, int varIndex) {
	// Is the variable mandatory?
	if (node->first_attribute("mandatory"))
		if (strcmp(node->first_attribute("mandatory")->value(), "true") == 0)
			mandatoryIndex.push_back(varIndex);
}

int FeatureVisitor::getIndexOfNoneForVariable(const string &variableName) {
	int indexOfNone = -1;
	for (unsigned int i = 0; i < variables[variableName]->size(); i++) {
		if (variables[variableName]->data()[i] == "false")
			indexOfNone = i;

		if (variables[variableName]->data()[i] == "NONE")
			indexOfNone = i;
	}
	return indexOfNone;
}

void FeatureVisitor::setMandatoryImplication(xml_node<> *node, int indexOfNone,
		int varIndex) {
	string parentName;
	if (!node->parent()->first_attribute("name")) {
		string error =
				"setMandatoryImplication: Error in retrieving the parent node from ";
		error += node->name();
		throw std::invalid_argument(error);
	} else {
		parentName = node->parent()->first_attribute("name")->value();
	}
	int indexOfNoneParent = getIndexOfNoneForVariable(parentName);
	mandatoryImplications.push_back(
			make_pair(make_pair(varIndex, indexOfNone),
					make_pair(variableIndex[parentName], indexOfNoneParent)));
}

void FeatureVisitor::setSingleImplication(xml_node<> *node, int indexOfNone) {
	string parentName;
	if (!node->parent()->first_attribute("name")) {
		string nodeName = node->name();
		string error =
				"setSingleImplication: Error in retrieving the parent node from ";
		throw std::invalid_argument(error + nodeName);
	} else {
		parentName = node->parent()->first_attribute("name")->value();
	}
	int indexOfNoneParent = getIndexOfNoneForVariable(parentName);
	singleImplications.push_back(
			make_pair(make_pair(index, indexOfNone),
					make_pair(variableIndex[parentName], indexOfNoneParent)));
}

void FeatureVisitor::setMandatory(xml_node<> *node, int indexOfNone,
		int varIndex) {
	// Is the variable mandatory?
	if (node->first_attribute("mandatory")) {
		if (node->parent() && strcmp(node->parent()->name(), "struct") != 0) {
			setMandatoryImplication(node, indexOfNone, varIndex);
		} else {
			setMandatoryNoParent(node, varIndex);
		}
	}
}

void FeatureVisitor::visitAnd(xml_node<> *node) {
	// Define the current variable, which is a boolean variable
	defineSingleVariable(node);

	// Set dependencies between the feature and its parent
	setDependency(node);

	// Is the variable mandatory. If it is not the root we should
	// set the additional constraint VAL = NONE <=> PARENT = NONE
	setMandatory(node, 0, index);

	// Increase the index
	index++;

	// Visit all the child features
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		visit(n);
	}
}

void FeatureVisitor::visitFeature(xml_node<> *node) {
	// The single feature is Boolean.
	defineSingleVariable(node);

	// The only possible configuration is if it is mandatory or not...
	// If the feature is mandatory, we need to add the constraint
	setMandatory(node, 0, index);

	// Set dependencies between the feature and its parent
	setDependency(node);

	// Increase the index
	index++;
}

void FeatureVisitor::setDependency(xml_node<> *node) {
	int indexOfNone = getIndexOfNoneForVariable(indexVariable[index]);
	if (node->parent() && strcmp(node->parent()->name(), "struct") != 0) {
		setSingleImplication(node, indexOfNone);
	}
}

vector<int> FeatureVisitor::getMandatoryIndex() {
	return mandatoryIndex;
}

vector<pair<pair<int, int>, vector<int>*>> FeatureVisitor::getOrIndexs() {
	return orIndexs;
}

void FeatureVisitor::printVariablesInMap() {
	for (map<string, vector<string>*>::const_iterator it = variables.begin();
			it != variables.end(); ++it) {
		logcout(LOG_DEBUG) << it->first << " - index: " << variableIndex[it->first] << endl;
	}
}

int FeatureVisitor::getNVar() {
	// Number of variables defined in the feature model
	return variables.size();
}

vector<pair<pair<int, int>, pair<int, int>>> FeatureVisitor::getMandatoryImplications() {
	return mandatoryImplications;
}

vector<pair<pair<int, int>, pair<int, int>>> FeatureVisitor::getSingleImplications() {
	return singleImplications;
}

int* FeatureVisitor::getBounds() {
	// Bounds for each variable.
	// It uses the indexVariable map to retrieve the name of the i-th variable and, with it, get the size of the
	// possible values
	const int n = getNVar();
	int *bounds = new int[n];

	for (int i = 0; i < n; i++) {
		bounds[i] = variables[indexVariable[i]]->size();
	}

	return bounds;
}

string FeatureVisitor::getValueForVar(int indexVar, int indexVal) {
	return variables[indexVariable[indexVar]]->data()[indexVal];
}

vector<pair<pair<int, int>, vector<pair<int, int>>*>> FeatureVisitor::getOrIndexsNonLeaf() {
	return orIndexsNonLeaf;
}
