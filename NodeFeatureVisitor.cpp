/* * NodeVisitor.cpp
 *
 *  Created on: Mar 24, 2023
 *      Author: bombarda
 */
#include <NodeFeatureVisitor.h>
#include <iostream>
#include <string.h>
#include <math.h>

using namespace std;
using namespace rapidxml;

int FeatureVisitor::index = 0;
bool FeatureVisitor::COMPRESS_AND_VARS = true;
int FeatureVisitor::COMPRESS_AND_THRESHOLD = 10;

FeatureVisitor::FeatureVisitor() {
	this->ignoreHidden = false;
}

FeatureVisitor::FeatureVisitor(bool ignoreHidden) {
	this->ignoreHidden = ignoreHidden;
}

bool FeatureVisitor::isVisitable(xml_node<> *node) {
	return strcmp(node->name(), "description") != 0;
}

void FeatureVisitor::visit(xml_node<> *node) {
	if (!isVisitable(node))
		return;

	logcout(LOG_DEBUG) << "Visiting node "
			<< node->first_attribute("name")->value() << endl;

	if (ignoreHidden && node->first_attribute("hidden")) {
		logcout(LOG_DEBUG) << "\tIgnoring node "
				<< node->first_attribute("name")->value() << " as it is hidden"
				<< endl;
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
	// Visit all the child features
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling())
		if (!isLeaf(n))
			return false;

	return true;
}

bool FeatureVisitor::isLeaf(xml_node<> *node) {
	if (strcmp(node->name(), "feature") == 0)
		return true;
	return false;
}

int FeatureVisitor::getNumChildren(xml_node<> *node) {
	return getNumChildren(node, false);
}

int FeatureVisitor::getNumChildren(xml_node<> *node,
		bool ignoreHiddenFeatures) {
	int i = 0;
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		if (!ignoreHiddenFeatures)
			i++;
		else if (!n->first_attribute("hidden")) {
			i++;
		}
	}
	return i;
}

int countOccurrences(xml_node<> *node, string word) {
	int nO = 0;

	// If the node is a VAR, check its name, otherwise fetch all the children
	if (strcmp(node->name(), "var") == 0) {
		if (strcmp(node->value(), word.c_str()) == 0) {
			return 1;
		}
		return 0;
	} else {
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			nO += countOccurrences(n, word);
		}
	}

	return nO;
}

bool comparePairs(pair<string, int> p1, pair<string, int> p2) {
	return p1.second < p2.second;
}

void FeatureVisitor::reorderVariables(xml_node<> *node) {
	// For each variable, it count how many occurrences are there in the file.
	// Then, all the lists and index are sorted in order to have first (i.e., in the bottom of the MDD)
	// the mostly occurring variables. In this way, possible additional edges, are focused in the bottom
	// of the MDD and, thus, the MDD size is kept under control
	vector<pair<string, int>> occurrences;
	for (std::map<string, vector<string>*>::iterator it = variables.begin();
			it != variables.end(); ++it) {
		string varName = it->first;
		xml_node<> *n1 = node;

		int nOccurrences = countOccurrences(n1, varName);
		occurrences.push_back(make_pair(varName, nOccurrences));
	}

	// Sort the occurrences vector based on the occurrence number
	std::sort(occurrences.begin(), occurrences.end(), comparePairs);

	// Reorder all the lists and maps
	int newIndex = 0;
	vector<int> mandatoryIndexNew;
	// Old index, new index
	map<int, int> indexMapping;

	for (pair<string, int> a : occurrences) {
		int oldIndex = variableIndex[a.first];
		indexMapping[oldIndex] = newIndex;
		// ---mandatoryIndex
		for (int indx : mandatoryIndex) {
			if (indx == oldIndex)
				mandatoryIndexNew.push_back(newIndex);
		}
		// ---variableIndex
		variableIndex[a.first] = newIndex;
		// ---indexVariable
		indexVariable[newIndex] = a.first;

		newIndex++;
	}

	// ---altIndexesExclusion
	for (pair<pair<int, int>, vector<pair<int, int>>*> &item : altIndexesExclusion) {
		item.first.first = indexMapping[item.first.first];
		for (pair<int, int> &itemVector : *item.second) {
			itemVector.first = indexMapping[itemVector.first];
		}
	}
	// ---orIndexsNonLeaf
	for (pair<pair<int, int>, vector<pair<int, int>>*> &item : orIndexsNonLeaf) {
		item.first.first = indexMapping[item.first.first];
		for (pair<int, int> &itemVector : *item.second) {
			itemVector.first = indexMapping[itemVector.first];
		}
	}
	// ---mandatoryImplications
	for (pair<pair<int, int>, pair<int, int>> &item : mandatoryImplications) {
		item.first.first = indexMapping[item.first.first];
		item.second.first = indexMapping[item.second.first];
	}
	// ---singleImplications
	for (pair<pair<int, int>, pair<int, int>> &item : singleImplications) {
		item.first.first = indexMapping[item.first.first];
		item.second.first = indexMapping[item.second.first];
	}
	// ---singleImplicationsNonLeaf
	for (pair<pair<int, int>, pair<int, int>> &item : singleImplicationsNonLeaf) {
		item.first.first = indexMapping[item.first.first];
		item.second.first = indexMapping[item.second.first];
	}
	// ---orIndexs
	for (pair<pair<int, int>, vector<int>*> &item : orIndexs) {
		item.first.first = indexMapping[item.first.first];
		for (int &itemVector : *item.second) {
			itemVector = indexMapping[itemVector];
		}
	}

	// Copy new data
	mandatoryIndex = mandatoryIndexNew;
}

void FeatureVisitor::visitAlt(xml_node<> *node) {
	if (getNumChildren(node) > 1) {
		// The alternative variable is an enumerative one
		string varName = node->first_attribute("name")->value();
		int indexOfNone = -1;
		int currentIndex = index;
		vector<string> *values = new vector<string>;

		// Get the possible values
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			if (isVisitable(n)) {
				values->push_back(n->first_attribute("name")->value());
			}
		}

		// Add to the possible values also the unselected one
		indexOfNone = values->size();
		values->push_back("NONE");

		// Create the variable
		variables[varName] = values;
		variableIndex[varName] = currentIndex;
		indexVariable[currentIndex] = varName;

		// VAL = NONE <=> PARENT = NONE
		// Handle the mandatory part of the value. It is mandatory only if
		// the parent has been selected
		setMandatory(node, indexOfNone, currentIndex);
		setDependency(node);

		// Increase the index
		index++;

		// Check if one of the children is not a leaf. In that case, visit it
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			if (isVisitable(n)) {
				if (!isLeaf(n)) {
					if (strcmp(n->name(), "alt") == 0) {
						// N is an alternative. It must be visited itself
						int nIndex = index;
						visit(n);
						// Add the constraint that if n is selected, then n's parent should have value equals to n
						mandatoryImplications.push_back(
								make_pair(
										make_pair(nIndex,
												getIndexOfNoneForVariable(
														indexVariable[nIndex])),
										make_pair(currentIndex,
												getIndexOfValue(
														indexVariable[nIndex]).second
														+ variables[indexVariable[currentIndex]]->size())));
					} else {
						// N is not an alternative. We should consider n's children
						for (xml_node<> *n1 = n->first_node(); n1;
								n1 = n1->next_sibling())
							visit(n1);
					}
				}
			}
		}
	} else {
		// The alternative variable is not a real alternative
		vector<pair<int, int>> *childrenIndex = new vector<pair<int, int>>;
		int parentIndex = index;
		int indexOfNoneParent = -1;

		// Define the current variable, which is a boolean variable
		defineSingleVariable(node);
		indexOfNoneParent = getIndexOfNoneForVariable(
				indexVariable[parentIndex]);

		// Set dependencies between the feature and its parent
		setDependency(node);

		// Is the variable mandatory. If it is not the root we should
		// set the additional constraint VAL = NONE <=> PARENT = NONE
		setMandatory(node, 0, index);

		// Increase the index
		index++;

		// Visit all the child features
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			int thisIndex = index;
			visit(n);
			int noneIndex = getIndexOfNoneForVariable(indexVariable[thisIndex]);
			childrenIndex->push_back(make_pair(thisIndex, noneIndex));
		}

		altIndexesExclusion.push_back(
				make_pair(make_pair(parentIndex, indexOfNoneParent),
						childrenIndex));
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
			if (isVisitable(n)) {
				varIndex = index;
				visit(n);
				indexOfNone = getIndexOfNoneForVariable(
						indexVariable[varIndex]);
				orIndex->push_back(make_pair(varIndex, indexOfNone));
			}
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
	if (variables.count(variableName) > 0)
		for (unsigned int i = 0; i < variables[variableName]->size(); i++) {
			if (variables[variableName]->data()[i] == "false")
				indexOfNone = i;

			if (variables[variableName]->data()[i] == "NONE")
				indexOfNone = i;
		}
	else
		indexOfNone = -1;

	return indexOfNone;
}

pair<int, int> FeatureVisitor::getIndexOfValue(const string &variableName) {
	int varInd = -1;
	int valueIndex = -1;
	for (std::map<string, vector<string>*>::iterator it = variables.begin();
			it != variables.end(); ++it) {
		if (std::find(it->second->begin(), it->second->end(), variableName)
				!= it->second->end()) {
			// Get the variable index
			varInd = variableIndex[it->first];

			// Get the index of the needed value
			auto itElement = std::find(it->second->begin(), it->second->end(),
					variableName);
			valueIndex = itElement - it->second->begin();
		}
	}

	return make_pair(varInd, valueIndex);
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
	// If the parent is not an Alternative
	if (indexOfNoneParent != -1) {
		mandatoryImplications.push_back(
				make_pair(make_pair(varIndex, indexOfNone),
						make_pair(variableIndex[parentName],
								indexOfNoneParent)));
	} else {
		pair<int, int> dependencyPair = getIndexOfValue(parentName);

		if (dependencyPair.first != -1 && dependencyPair.second != -1) {
			// If the parent has been merged in an alternative
			mandatoryImplications.push_back(
					make_pair(make_pair(varIndex, indexOfNone),
							make_pair(dependencyPair.first,
									dependencyPair.second
											+ variables[indexVariable[dependencyPair.first]]->size())));
		} else {
			// If the parent has been merged into an AND
			// TODO
		}
	}
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
	// If the parent is not an Alternative
	if (indexOfNoneParent != -1)
		singleImplications.push_back(
				make_pair(make_pair(index, indexOfNone),
						make_pair(variableIndex[parentName],
								indexOfNoneParent)));
	else {
		pair<int, int> dependencyPair = getIndexOfValue(parentName);
		// If the parent has been merged in an alternative
		singleImplicationsNonLeaf.push_back(
				make_pair(make_pair(index, indexOfNone), dependencyPair));

	}
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
//	if (FeatureVisitor::COMPRESS_AND_VARS
//			&& getNumChildren(node, this->ignoreHidden)
//					<= FeatureVisitor::COMPRESS_AND_THRESHOLD) {
//		int nChildren = getNumChildren(node, this->ignoreHidden);
//		vector<string> *values = new vector<string>;
//		vector<int> mandatories;
//		int i = 0;
//
//		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
//			if (strcmp(n->name(), "description") != 0
//					&& (!this->ignoreHidden || !(n->first_attribute("hidden")))) {
//				if (n->first_attribute("mandatory")
//						and strcmp(n->first_attribute("mandatory")->value(),
//								"true") == 0) {
//					// This index is mandatory
//					mandatories.push_back(i);
//				}
//				i++;
//			}
//		}
//
//		values->push_back("NONE");
//
//		// Now define all the possible values
//		for (double iD = 0; iD < pow(2, nChildren); iD++) {
//			bool discard = false;
//
//			// If we have at least one mandatory feature, the 0 value cannot be assumed
//			if (mandatories.size() == 0 || iD > 0) {
//				for (unsigned int j = 0; j < mandatories.size(); j++) {
//					// If the mandatory bits are not set
//					if (!(((int) iD) & (1 << mandatories.at(j)))) {
//						discard = true;
//						break;
//					}
//				}
//
//				if (!discard)
//					values->push_back(to_string(iD));
//			}
//		}
//
//		// Create a variable
//		variables[node->first_attribute("name")->value()] = values;
//		variableIndex[node->first_attribute("name")->value()] = index;
//		indexVariable[index] = node->first_attribute("name")->value();
//
//		// Set dependencies between the feature and its parent
//		setDependency(node);
//
//		// Is the variable mandatory. If it is not the root we should
//		// set the additional constraint VAL = NONE <=> PARENT = NONE
//		setMandatory(node, 0, index);
//
//		// Increase the index
//		index++;
//
//		// Now, for each child, define which values make it true
//		i = 0;
//		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
//			if (strcmp(n->name(), "description") != 0
//					&& (!this->ignoreHidden || !(n->first_attribute("hidden")))) {
//				string childName = n->first_attribute("name")->value();
//				string parentName = node->first_attribute("name")->value();
//
//				vector<string> parTruthValues;
//				for (unsigned int j = 0; j < values->size(); j++) {
//					// Do not consider NONE (since parent is not selected) and 0 (since no child feature
//					// is selected)
//					if (values->at(j) != "NONE" && std::stod(values->at(j)) > 0)
//						// If the corresponding bit is set
//						if ((std::stoi(values->at(j)) & (1 << i)))
//							parTruthValues.push_back(values->at(j));
//				}
//
//				andLeafs[childName] = make_pair(parentName, parTruthValues);
//				i++;
//
//				// If the node is not a feature, then visit its siblings
//				if (strcmp(n->name(), "feature") != 0) {
//					for (xml_node<> *n1 = n->first_node(); n1;
//							n1 = n1->next_sibling())
//						if (strcmp(n1->name(), "description") != 0)
//							visit(n1);
//				}
//			}
//		}
//	}

	// All children are leaf. We can create a variable with multiple values and
	// then substitute accordingly the constraints
	if (areChildrenAllLeaf(node) && FeatureVisitor::COMPRESS_AND_VARS
			&& getNumChildren(node, this->ignoreHidden)
					<= FeatureVisitor::COMPRESS_AND_THRESHOLD) {
		int nChildren = getNumChildren(node, this->ignoreHidden);
		vector<string> *values = new vector<string>;
		vector<int> mandatories;
		int i = 0;

		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			if (isVisitable(n)
					&& (!this->ignoreHidden || !(n->first_attribute("hidden")))) {
				if (n->first_attribute("mandatory")
						and strcmp(n->first_attribute("mandatory")->value(),
								"true") == 0) {
					// This index is mandatory
					mandatories.push_back(i);
				}
				i++;
			}
		}

		values->push_back("NONE");

		// Now define all the possible values
		for (double iD = 0; iD < pow(2, nChildren); iD++) {
			bool discard = false;

			// If we have at least one mandatory feature, the 0 value cannot be assumed
			if (mandatories.size() == 0 || iD > 0) {
				for (unsigned int j = 0; j < mandatories.size(); j++) {
					// If the mandatory bits are not set
					if (!(((int) iD) & (1 << mandatories.at(j)))) {
						discard = true;
						break;
					}
				}

				if (!discard)
					values->push_back(to_string(iD));
			}
		}

		// Create a variable
		variables[node->first_attribute("name")->value()] = values;
		variableIndex[node->first_attribute("name")->value()] = index;
		indexVariable[index] = node->first_attribute("name")->value();

		// Set dependencies between the feature and its parent
		setDependency(node);

		// Is the variable mandatory. If it is not the root we should
		// set the additional constraint VAL = NONE <=> PARENT = NONE
		setMandatory(node, 0, index);

		// Increase the index
		index++;

		// Now, for each child, define which values make it true
		i = 0;
		for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
			if (isVisitable(n)
					&& (!this->ignoreHidden || !(n->first_attribute("hidden")))) {
				string childName = n->first_attribute("name")->value();
				string parentName = node->first_attribute("name")->value();

				vector<string> parTruthValues;
				for (unsigned int j = 0; j < values->size(); j++) {
					// Do not consider NONE (since parent is not selected) and 0 (since no child feature
					// is selected)
					if (values->at(j) != "NONE" && std::stod(values->at(j)) > 0)
						// If the corresponding bit is set
						if ((std::stoi(values->at(j)) & (1 << i)))
							parTruthValues.push_back(values->at(j));
				}

				andLeafs[childName] = make_pair(parentName, parTruthValues);
				i++;
			}
		}

	} else {
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
}

void FeatureVisitor::visitFeature(xml_node<> *node) {
	if (node->first_attribute("mandatory") && node->parent()) {
		if (getNumChildren(node) == 0
				&& strcmp(node->parent()->name(), "alt") != 0) {
			// In this case the feature is mandatory and a leaf, so we can avoid representing it.
			// The only operation needed is to substitute in every constraint the variable with its
			// parent name
			substitutions[node->first_attribute("name")->value()] =
					node->parent()->first_attribute("name")->value();
			return;
		}
	}

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

/**
 * This method is used to set the dependency between the node passed as parameter
 * and its parent.
 *
 * The basic idea is that the node passed as parameter cannot be selected if its parent is not.
 *
 * @param node the pointer to the node which depends on its parent
 */
void FeatureVisitor::setDependency(xml_node<> *node) {
	int indexOfNone = getIndexOfNoneForVariable(indexVariable[index]);
	if (node->parent() && strcmp(node->parent()->name(), "struct") != 0) {
		setSingleImplication(node, indexOfNone);
	}
}

/**
 * Returns the list of indexes which are mandatory and do not depends on other features.
 *
 * In practice, it should contain only the root
 *
 * @return the vector of mandatory indexes
 */
vector<int> FeatureVisitor::getMandatoryIndex() {
	return mandatoryIndex;
}

vector<pair<pair<int, int>, vector<int>*>> FeatureVisitor::getOrIndexs() {
	return orIndexs;
}

/**
 * Prints on the logger (at LOG_DEBUG level) all the variables defined for the
 * feature model by the FMProductsCounter.
 *
 * The output format is the following:
 *
 * [variable name] - index [index of the variable] - size [number of possible values for the variable]
 */
void FeatureVisitor::printDefinedVariables() {
	for (map<string, vector<string>*>::const_iterator it = variables.begin();
			it != variables.end(); ++it) {
		logcout(LOG_DEBUG) << it->first << " - index: "
				<< variableIndex[it->first] << " - size: " << it->second->size()
				<< endl;
	}
}

/**
 * Number of variables.
 *
 * @return the number of variables defined in the feature model
 */
int FeatureVisitor::getNVar() {
	return variables.size();
}

vector<pair<pair<int, int>, pair<int, int>> > FeatureVisitor::getMandatoryImplications() {
	return mandatoryImplications;
}

vector<pair<pair<int, int>, pair<int, int>> > FeatureVisitor::getSingleImplications() {
	return singleImplications;
}

vector<pair<pair<int, int>, pair<int, int>> > FeatureVisitor::getSingleImplicationsNonLeaf() {
	return singleImplicationsNonLeaf;
}

/**
 * It returns the bounds (i.e., number of elements) for all the variables
 *
 * It uses the indexVariable map to retrieve the name of the i-th variable and,
 * with it, get the size of the possible values
 *
 * @retrurn the bounds (i.e., number of elements) for all the variables
 */
int* FeatureVisitor::getBounds() {
	const int n = getNVar();
	int *bounds = new int[n];

	for (int i = 0; i < n; i++) {
		bounds[i] = getBoundForVar(i);
	}

	return bounds;
}

/**
 * Given the index of a variable and the index of a value for the variable
 * it returns the corresponding string value
 *
 * @param indexVar the index of the variable
 * @param indexVal the index of the value
 * @retrurn the corresponding string value
 */
string FeatureVisitor::getValueForVar(int indexVar, int indexVal) {
	if (indexVal >= getBoundForVar(indexVar))
		return "-"
				+ variables[indexVariable[indexVar]]->data()[indexVal
						- getBoundForVar(indexVar)];
	return variables[indexVariable[indexVar]]->data()[indexVal];
}

vector<pair<pair<int, int>, vector<pair<int, int>>*>> FeatureVisitor::getOrIndexsNonLeaf() {
	return orIndexsNonLeaf;
}

vector<pair<pair<int, int>, vector<pair<int, int>>*>> FeatureVisitor::getAltIndexesExclusion() {
	return altIndexesExclusion;
}

/**
 * Destructor
 */
FeatureVisitor::~FeatureVisitor() {

}

/**
 * Given the index of a variable, it returns the number of possible values
 *
 * @param index the index of the variable
 * @retrurn the number of possible values for the given variable
 */
int FeatureVisitor::getBoundForVar(int index) {
	return variables[indexVariable[index]]->size();
}
