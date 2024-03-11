/* * NodeVisitor.cpp
 *
 *  Created on: Mar 24, 2023
 *      Author: Andrea Bombarda
 */
#include <NodeFeatureVisitor.h>
#include <iostream>
#include <string.h>
#include <math.h>

using namespace std;
using namespace rapidxml;

/**
 * Index used by the FeatureVisitor component to identify the current variable
 */
int FeatureVisitor::index = 0;

/**
 * Configuration parameter to set whether the COMPRESS_AND optimization has to be used.
 * If COMPRESS_AND_VARS is true, AND groups are merged in a single MDD variable that
 * represents which of the child are selected by using a binary representation.
 *
 * For instance,
 *
 * 				--------
 * 				|  F1  |
 * 				--------
 * 				/   |   \
 * 			   /	|    \
 * 			  F2   F3    F4
 *
 * will be translated in a single variable F1={NONE, 0, 1, 2, 3, 4, 5, 6, 7}
 * where NONE indicates that F1 is not selected, 0=000 indicates that F1 is selected but F2, F3 and F4 are not,
 * 1=001 indicates that F2 is selected, but F3 and F4 not, and so on.
 *
 * If a sub-feature is mandatory, the implemented algorithm automatically discards those configurations in which
 * the feature is not selected.
 */
bool FeatureVisitor::COMPRESS_AND_VARS = true;

/**
 * Threshold to set a threshold (i.e., the number of children) for the COMPRESS_AND optimization.
 * Experiments have shown that using the optimization is feasible for a low number of child (~10), while
 * for higher values it may become useless since it saves nodes in depth but creates more node in width.
 */
int FeatureVisitor::COMPRESS_AND_THRESHOLD = 10;

/**
 * Constructor for a FeatureVisitor object.
 *
 * By default, hidden features are not ignored.
 * Use parameterized constructor to set differently.
 */
FeatureVisitor::FeatureVisitor() {
	this->ignoreHidden = false;
}

/**
 * Constructor for a FeatureVisitor object.
 *
 * @param ignoreHidden a boolean stating whether hidden features have to be ignored or not
 */
FeatureVisitor::FeatureVisitor(bool ignoreHidden) {
	this->ignoreHidden = ignoreHidden;
}

/**
 * It determines whether the node given as parameter is visitable or not (i.e., it contains
 * meaningful information for counting products)
 *
 * @param node the node to be checked
 * @return true is the node contains meaningful information, false otherwise
 */
bool FeatureVisitor::isVisitable(xml_node<> *node) {
	return strcmp(node->name(), "description") != 0;
}

/**
 * Dispatcher implementing the visitor pattern, depending on the node type.
 *
 * It works both for nodes that are visitable or not. If the node is not visitable,
 * the method ends with no effect.
 * Moreover, base on the ignoreHidden internal field, it works both for hidden and
 * not hidden features.
 *
 * Output information is printed on the logger at LOG_DEBUG level.
 *
 * @param node the node to be visited
 */
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

	// Dispatch the visit based on the node type
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

/**
 * Given a node, it visits all the children and checks whether they are all leaf
 *
 * @param node the node to be visited
 * @return true if all the children are leaf, false otherwise
 */
bool FeatureVisitor::areChildrenAllLeaf(xml_node<> *node) {
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling())
		if (!isLeaf(n))
			return false;

	return true;
}

/**
 * Given a node, it checks whether the node is a leaf (i.e., a feature)
 *
 * @param node the node to be visited
 * @return true if the node is a leaf, false otherwise
 */
bool FeatureVisitor::isLeaf(xml_node<> *node) {
	return strcmp(node->name(), "feature") == 0;
}

/**
 * Given a node, it returns the number of children.
 * NOTE THAT Hidden features are not ignored
 *
 * @param node the node to be visited
 * @return the number of children for the given node
 */
int FeatureVisitor::getNumChildren(xml_node<> *node) {
	return getNumChildren(node, false);
}

/**
 * Given a node, it returns the number of children, by considering or not considering
 * hidden features.
 *
 * @param node the node to be visited
 * @param ignoreHiddenFeatures the boolean variable indicating if hidden features have to be ignored or not
 * @return the number of children for the given node
 */
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

/**
 * Given a starting node, it recursively counts how many times a given word is used.
 *
 * This method is used for sorting variables based on the number of occurrencies for each.
 *
 * @param node the starting node
 * @param word the word we are looking for
 * @return the number of occurrencies for the given word (which is typically a name of a variable)
 */
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

/**
 * Given two pairs in the form of <variableName, numberOfOccurrencies> each, it is used
 * as a comparator
 *
 * @param p1 the first pair
 * @param p2 the second pair
 * @return TRUE if the number of occurrencies for p1 is lower than that of p2, FALSE otherwise
 */
bool comparePairs(pair<string, int> p1, pair<string, int> p2) {
	return p1.second < p2.second;
}

/**
 * Visitor for an ALT group.
 *
 * There are two possible cases:
 *
 * - The group has more than 1 child. In this case the ALT Group is translated into a single enumerative variable, and
 *   some constraint is added in order to assure that, if the considered sub-feature f is not a leaf, the subtree
 *   having f as root can be selected only if the parent feature of f has value = f.
 *
 * - The group has only one child. In this case the ALT Group is not really a group, but must be treated as a single feature
 *
 * @param node the node to be visited
 */
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
	// Check whether children are all leafs
	if (areChildrenAllLeaf(node)) {
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

/**
 * This method creates a single Boolean variable for the given node
 *
 * @param node the node to be converted into a MDD boolean variable
 */
void FeatureVisitor::defineSingleVariable(xml_node<> *node) {
	// Define the current variable, which is a boolean variable
	vector<string> *values = new vector<string>;
	values->push_back("false");
	values->push_back("true");
	variables[node->first_attribute("name")->value()] = values;
	variableIndex[node->first_attribute("name")->value()] = index;
	indexVariable[index] = node->first_attribute("name")->value();
}

/**
 * This method defines whether the node given as parameter is mandatory or not (without checking its parent -> it should
 * be used only for the root feature).
 *
 * @param node the node to be checked
 * @param varIndex the index of the varible corresponding to the node to be checked
 */
void FeatureVisitor::setMandatoryNoParent(xml_node<> *node, int varIndex) {
	// Is the variable mandatory?
	if (node->first_attribute("mandatory"))
		if (strcmp(node->first_attribute("mandatory")->value(), "true") == 0)
			mandatoryIndex.push_back(varIndex);
}

/**
 * Given the name of the variable, it returns the index of the none element (which is used to set the feature as non selected)
 *
 * @param variableName the name of the variable of interest
 * @return the integer corresponding to the index of the none element
 */
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

/**
 * Given the name of a variable, it returns its index and value. It is used for ALT Groups,
 * where a the children features of the feature model are translated into a single enumerative MDD variable.
 *
 * For this reason, the method receives the name of the variable in the feature model and returns
 * a pair <x,y> where x is the index of the corresponding MDD variable and y is the index of its value
 *
 * @param variableName the name of the vartiable we are looking for
 * @return a pair <x,y> where x is the index of the corresponding MDD variable and y is the index of its value
 */
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

		if (dependencyPair.first != -1 && dependencyPair.second != -1) {
			// If the parent has been merged in an alternative
			singleImplicationsNonLeaf.push_back(
					make_pair(make_pair(index, indexOfNone), dependencyPair));
		} else {
			// If the parent has been merged into an AND
			// TODO
		}

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

/**
 * Visitor for a feature.
 *
 * The method checks whether the feature is a leaf and is mandatory. In that case, the feature is not translated into
 * a real MDD variable, but we only keep track of it, since its name in the constraints will require to be substituted
 * with the name of the parent feature.
 *
 * In all the other cases, a regular boolean variable will be created, together with the information needed to create
 * the mandatory constraints (i.e., if the parent is selected, then also that feature must be selected) and those
 * related to the dependency between features (i.e., if this feature is selected, it is required that the parent is
 * selected as well).
 *
 * @param node the node to be visited
 */
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

/**
 * This method returns the indexes that are mutually exclusive due to their presence in an ALT Group
 *
 * @return a vector<pair<pair<int, int>, vector<pair<int, int>>*>> with each element with the format:
 * 		<<int x, int y>, vector<int z, int w>>, where x and z are the indexes of the variables, and y
 * 		and w are their values
 */
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
