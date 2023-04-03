/*
 * ConstraintVisitor.h
 *
 *  Created on: 27 mar 2023
 *      Author: parallels
 */

#ifndef CONSTRAINTVISITOR_H_
#define CONSTRAINTVISITOR_H_

#include "rapidxml.hpp"
#include "NodeFeatureVisitor.h"
#include <meddly.h>
#include <vector>
#include <iostream>
#include <string.h>
#include "Util.hpp"
#include <algorithm>
#include "logger.hpp"

using namespace std;
using namespace rapidxml;
using namespace MEDDLY;

class ConstraintVisitor {
private:
	FeatureVisitor visitor;
	dd_edge emptyNode;
	forest* mdd;
	vector<dd_edge> constraintMddList;

	dd_edge visitConstraint(xml_node<> * node);
	dd_edge visitConj(xml_node<> * node);
	dd_edge visitDisj(xml_node<> * node);
	dd_edge visitImplies(xml_node<> * node);
	dd_edge visitVar(xml_node<> * node);
	dd_edge visitNot(xml_node<> * node);
	dd_edge visitEq(xml_node<> * node);

public:
	ConstraintVisitor(FeatureVisitor v, dd_edge emptyNode, forest* mdd);
	virtual ~ConstraintVisitor();
	void visit(xml_node<> * &node, int reduction_factor);
	void visit(xml_node<> * &node);
	vector<dd_edge> getConstraintMddList();
};

#endif /* CONSTRAINTVISITOR_H_ */
