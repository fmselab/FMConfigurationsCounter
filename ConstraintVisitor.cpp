/*
 * ConstraintVisitor.cpp
 *
 *  Created on: 27 mar 2023
 *      Author: parallels
 */

#include "ConstraintVisitor.h"

ConstraintVisitor::ConstraintVisitor(FeatureVisitor v, const dd_edge& emptyNode,
		forest *mdd) {
	this->visitor = v;
	this->emptyNode = emptyNode;
	this->mdd = mdd;
}

ConstraintVisitor::~ConstraintVisitor() {
	for (dd_edge e : constraintMddList)
		e.clear();

	constraintMddList.clear();
}

void ConstraintVisitor::visit(xml_node<> *&node) {
	visit(node, 0);
}

void ConstraintVisitor::visit(xml_node<> *&node, int reduction_factor) {
	int i = 0;
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		if (strcmp(n->name(), "rule") == 0) {
			// What to do and how to deal with when the constraint contains an hidden feature???
			// Now, I'm using emptynode for every variable which is not in the FeatureVisitor lists
			// but I'm not sure it's the best (and correct) solution
			dd_edge c = visitConstraint(n->first_node());
			logcout(LOG_DEBUG) << "Constraint " << ++i << " cardinality "
					<< c.getCardinality() << endl;
			constraintMddList.push_back(c);
		}
	}

	if (reduction_factor > 0) {
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

dd_edge ConstraintVisitor::visitConj(xml_node<> *node) {
	dd_edge baseNode = this->emptyNode;
	// Visit each child and then compute the AND among all of them
	for (xml_node<> *n = node->first_node(); n; n = n->next_sibling()) {
		baseNode *= visitConstraint(n);
	}
	return baseNode;
}

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

dd_edge ConstraintVisitor::visitImplies(xml_node<> *node) {
	xml_node<> *left = node->first_node();
	xml_node<> *right = left->next_sibling();
	dd_edge leftEdge = visitConstraint(left);
	dd_edge rightEdge = visitConstraint(right);
	leftEdge = this->emptyNode - leftEdge;
	return leftEdge + rightEdge;
}

dd_edge ConstraintVisitor::visitEq(xml_node<> *node) {
	xml_node<> *left = node->first_node();
	xml_node<> *right = left->next_sibling();
	dd_edge leftEdge = visitConstraint(left);
	dd_edge rightEdge = visitConstraint(right);
	apply(EQUAL, leftEdge, rightEdge, leftEdge);
	return leftEdge;
}

dd_edge ConstraintVisitor::visitVar(xml_node<> *node) {
	// Name of the feature
	string variableName = node->value();
	// Number of variables
	const int N = mdd->getDomain()->getNumVariables();
	// It is possible to find the feature, so we need to get its index
	// In this case, it is boolean feature
	if (visitor.variableIndex.count(variableName) > 0 && visitor.variables.count(variableName) > 0) {
		vector<string> *values = visitor.variables[variableName];
		int variableIndex = visitor.variableIndex[variableName];
		if (values->size() > 2
				|| (std::find(values->begin(), values->end(), "true")
						== values->end()
						&& std::find(values->begin(), values->end(), "false")
								== values->end())) {
			// Enumerative -> Instead of looking for the "true" value, we need to look for the none value
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
	}
	return emptyNode;
}

dd_edge ConstraintVisitor::visitNot(xml_node<> *node) {
	dd_edge baseNode = this->emptyNode;
	// Visit the sub-constraint
	dd_edge subConstraint = visitConstraint(node->first_node());
	// Negate the sub-constraint
	baseNode = baseNode - subConstraint;
	return baseNode;
}

vector<dd_edge> ConstraintVisitor::getConstraintMddList() {
	return constraintMddList;
}

