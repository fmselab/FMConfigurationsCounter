/*
 * ConstraintVisitor.cpp
 *
 *  Created on: 27 mar 2023
 */

#include "ConstraintVisitor.h"

/**
 * Comparator used for comparing edges in an MDD
 *
 * @param e1 the first edge
 * @param e2 the second edge
 * @return TRUE if the number of edges of the MDD starting with e1 is lower than that of e2,
 *         FALSE otherwise
 */
bool compareConstraint(dd_edge e1, dd_edge e2) {
	return (e1.getEdgeCount() < e2.getEdgeCount());
}

/**
 * Costructor
 *
 * @param v the FeatureVisitor object, used for retreaving information about variables
 * @param emptyNode the empty node, containing the status of the MDD at the beginning
 * @param mdd the forest
 */
ConstraintVisitor::ConstraintVisitor(FeatureVisitor v, const dd_edge &emptyNode,
		forest *mdd) {
	this->visitor = v;
	this->emptyNode = emptyNode;
	this->mdd = mdd;
}

/**
 * Destructor
 *
 * It clears the list of MDDs previously created
 */
ConstraintVisitor::~ConstraintVisitor() {
	for (dd_edge e : constraintMddList)
		e.detach();

	constraintMddList.clear();
}

/**
 * Main access point to the ConstraintVisitor when no reduction is used.
 *
 * @param node the node to be visited
 */
void ConstraintVisitor::visit(xml_node<> *&node) {
	visit(node, 0);
}

/**
 * Main access point to the ConstraintVisitor when a reduction needs to be used.
 *
 * Output information is printed on the logger at LOG_DEBUG level.
 *
 * @param node the node to be visited
 * @param reduction_factor the reduction factor to be used
 */
void ConstraintVisitor::visit(xml_node<> *&node, int reduction_factor) {
	int i = 0;
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		if (strcmp(n->name(), "rule") == 0) {
			// What to do and how to deal with when the constraint contains an hidden feature???
			// Now, I'm using emptynode for every variable which is not in the FeatureVisitor lists
			// but I'm not sure it's the best (and correct) solution
			dd_edge c = visitConstraint(n->first_node());
			double card;
			apply(CARDINALITY,c, card);
			logcout(LOG_DEBUG) << "Constraint " << ++i << " cardinality "
					<< card << endl;
			constraintMddList.push_back(c);
		}
	}

	if (reduction_factor > 0) {
		if (Util::SHUFFLE_CONSTRAINTS) {
			std::shuffle(std::begin(constraintMddList),
					std::end(constraintMddList), std::random_device());
		} else {
			// Alternate sort the constraints: the first element is the maximum, then the minimum,
			// then the second maximum, and so on. In this way, the composition-constraint has
			// a lower cardinality
			std::sort(constraintMddList.begin(), constraintMddList.end(), compareConstraint);
			vector<dd_edge> constraintMddListAlternateSorted;
			int i = 0, j = constraintMddList.size()-1;

			while (i < j) {
				constraintMddListAlternateSorted.push_back(constraintMddList[j--]);
				constraintMddListAlternateSorted.push_back(constraintMddList[i++]);
			}

		    // If the total element in array is odd
		    // then print the last middle element.
		    if (constraintMddList.size() % 2 != 0) {
		    	constraintMddListAlternateSorted.push_back(constraintMddList[i]);
		    }

			constraintMddList = constraintMddListAlternateSorted;
		}

		vector<dd_edge> temp;

		// Compact the constraints
		for (unsigned int i = 0; i < constraintMddList.size(); i +=
				reduction_factor) {
			dd_edge cumulativeNode = constraintMddList[i];
			logcout(LOG_DEBUG) << "\tReducing constraints from " << (i + 1)
					<< endl;
			for (int j = 1;
					j < reduction_factor && i + j < constraintMddList.size();
					j++) {
				cumulativeNode *= constraintMddList[i + j];
			}
			temp.push_back(cumulativeNode);
		}

		logcout(LOG_DEBUG) << "Constraints reduced to " << temp.size() << endl;
		constraintMddList = temp;
	}
}

/**
 * Dispatcher implementing the visitor pattern, depending on the node type, for constraints.
 *
 * @param node the node to be visited
 */
dd_edge ConstraintVisitor::visitConstraint(xml_node<> *node) {
	// Dispatcher, depending on the node type
	if (strcmp(node->name(), "not") == 0)
		return visitNot(node);
	else if (strcmp(node->name(), "var") == 0)
		return visitVar(node);
	else if (strcmp(node->name(), "imp") == 0)
		return visitImplies(node);
	else if (strcmp(node->name(), "eq") == 0)
		return visitEq(node);
	else if (strcmp(node->name(), "disj") == 0)
		return visitDisj(node);
	else if (strcmp(node->name(), "conj") == 0)
		return visitConj(node);
	else {
		string nodeName = node->name();
		string error = "Invalid node type: " + nodeName;
		throw std::invalid_argument(error);
	}
}

/**
 * This method visits an AND in constraints.
 *
 * Given an AND-Node, it visit all the children and returns the INTERSECTION of the MDDs
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the AND
 */
dd_edge ConstraintVisitor::visitConj(xml_node<> *node) {
	dd_edge baseNode = this->emptyNode;
	// Visit each child and then compute the AND among all of them
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		baseNode *= visitConstraint(n);
	}
	return baseNode;
}

/**
 * This method visits an OR in constraints.
 *
 * Given an OR-Node, it visit all the children and returns the UNION of the MDDs
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the OR
 */
dd_edge ConstraintVisitor::visitDisj(xml_node<> *node) {
	dd_edge baseNode;
	// Visit each child and then compute the OR among all of them
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		if (n == node->first_node())
			baseNode = visitConstraint(n);
		else
			baseNode += visitConstraint(n);
	}
	return baseNode;
}

/**
 * This method visits an IMPLIES in constraints.
 *
 * Given an IMPLIES-Node, it visit the left and right parts
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the IMPLIES
 */
dd_edge ConstraintVisitor::visitImplies(xml_node<> *node) {
	xml_node<> *left = node->first_node();
	xml_node<> *right = left->next_sibling();
	dd_edge leftEdge = visitConstraint(left);
	dd_edge rightEdge = visitConstraint(right);
	leftEdge = this->emptyNode - leftEdge;
	return leftEdge + rightEdge;
}

/**
 * This method visits an EQUAL in constraints.
 *
 * Given an EQUAL-Node, it visit the left and right parts
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the EQUAL
 */
dd_edge ConstraintVisitor::visitEq(xml_node<> *node) {
	xml_node<> *left = node->first_node();
	xml_node<> *right = left->next_sibling();
	dd_edge leftEdge = visitConstraint(left);
	dd_edge rightEdge = visitConstraint(right);
	apply(EQUAL, leftEdge, rightEdge, leftEdge);
	return leftEdge;
}

/**
 * This method visits a VARIABLE in constraints.
 *
 * Given a VARIABLE-Node, it visit if in the following way:
 * 	- First the variable is searched in the list of variables. If it is found, then an MDD
 * 	  with the correct value is created
 *
 * 	- If the variable is not found in the list of variables, it means that it has been substituted
 * 	  since it is an AND or ALT. For this reason, it needs to be found between the values of another
 * 	  MDD variable. Then, when the correct values are found, an MDD is created
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the VARIABLE
 */
dd_edge ConstraintVisitor::visitVar(xml_node<> *node) {
	// Name of the feature
	string variableName = node->value();
	// Has the variable name to be substituted? This may happen if the feature is mandatory and it is leaf
	if (visitor.substitutions.count(variableName)) {
		variableName = visitor.substitutions[variableName];
	}

	// Number of variables
	const int N = mdd->getDomain()->getNumVariables();
	// It is possible to find the feature, so we need to get its index
	// In this case, it is boolean feature
	if (visitor.variableIndex.count(variableName) > 0
			&& visitor.variables.count(variableName) > 0) {
		vector<string> *values = visitor.variables[variableName];
		int variableIndex = visitor.variableIndex[variableName];

		// Enumerative (if the size is greater than 2 or true/false are not present)
		if (values->size() > 2
				|| (std::find(values->begin(), values->end(), "true")
						== values->end()
						&& std::find(values->begin(), values->end(), "false")
								== values->end())) {
			// Instead of looking for the "true" value, we need to look for the none value
			// and then complement the result
			int noneIndex = visitor.getIndexOfNoneForVariable(variableName);
			vector<int> constraint(N, -1);
			constraint[N - variableIndex - 1] = noneIndex;
			dd_edge tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			return emptyNode - tempC;
		} else {
			// Boolean
			vector<int> constraint(N, -1);
			constraint[N - variableIndex - 1] = 1;
			dd_edge tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			return tempC;
		}
	} else {
		// In this case, the feature has been translated into an enumerative
		// so, we need to look for the values
		for (std::map<string, vector<string>*>::iterator it =
				visitor.variables.begin(); it != visitor.variables.end();
				++it) {
			if (std::find(it->second->begin(), it->second->end(), variableName)
					!= it->second->end()) {
				int variableIndex = visitor.variableIndex[it->first];
				vector<int> constraint(N, -1);
				// Get the index of the needed value
				auto itElement = std::find(it->second->begin(),
						it->second->end(), variableName);
				int valueIndex = itElement - it->second->begin();
				constraint[N - variableIndex - 1] = valueIndex;
				dd_edge tempC = Util::getMDDFromTuple(constraint, mdd)
						* emptyNode;
				return tempC;
			}
		}

		// Here the variable has not been found by none of the previous attempts
		// since it has been compressed into a single AND variable
		for (std::map<string, pair<string, vector<string>>>::iterator it =
				visitor.andLeafs.begin(); it != visitor.andLeafs.end(); ++it) {
			if (it->first == variableName) {
				int variableIndex = visitor.variableIndex[it->second.first];
				vector<string> *varValues = visitor.variables[it->second.first];
				vector<int> constraint(N, -1);
				dd_edge tempC = emptyNode;

				for (unsigned int i = 0; i < it->second.second.size(); i++) {
					// Get the index of the needed value
					auto itElement = std::find(varValues->begin(),
							varValues->end(), it->second.second.at(i));
					int valueIndex = itElement - varValues->begin();
					constraint[N - variableIndex - 1] = valueIndex;
					if (i == 0)
						tempC = Util::getMDDFromTuple(constraint, mdd);
					else
						tempC += Util::getMDDFromTuple(constraint, mdd);
				}

				return tempC * emptyNode;
			}
		}
	}

	// If none of the previous return has been performed, it means that the variable has not been found
	// (possibily because it is hidden, and ignored). For this reason, we return an empty node.
	return emptyNode;
}

/**
 * This method visits a NOT in constraints.
 *
 * Given a NOT-Node, it visit the positive part and then complements the results by subtracting it to
 * the base node
 *
 * @param node the node to be visited
 * @return an MDD edge, i.e., the root of the MDD corresponding to the NOT
 */
dd_edge ConstraintVisitor::visitNot(xml_node<> *node) {
	dd_edge baseNode = this->emptyNode;
	// Visit the sub-constraint
	dd_edge subConstraint = visitConstraint(node->first_node());
	// Negate the sub-constraint
	baseNode = baseNode - subConstraint;
	return baseNode;
}

/**
 * Returns the list of constraints converted into MDD
 *
 * @return a vector<dd_edge> containing the list of constraints converted into MDDs
 */
vector<dd_edge> ConstraintVisitor::getConstraintMddList() {
	return constraintMddList;
}
