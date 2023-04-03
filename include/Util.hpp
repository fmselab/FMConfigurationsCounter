/*
 * Util.hpp
 *
 *  Created on: 28 mar 2023
 *      Author: parallels
 */

#ifndef INCLUDE_UTIL_HPP_
#define INCLUDE_UTIL_HPP_

#include <meddly.h>
#include <vector>
#include <iostream>
#include "ConstraintVisitor.h"
#include "NodeFeatureVisitor.h"
#include <fstream>
#include "rapidxml.hpp"
#include <gmp.h>

using namespace rapidxml;
using namespace MEDDLY;
using namespace std;

class Util {
private:
	static dd_edge addMandatory(const dd_edge &emptyNode, const int N,
			FeatureVisitor &v, forest *mdd);
	static void addOrGroupConstraints(FeatureVisitor v, const dd_edge emptyNode,
			const int N, dd_edge &startingNode, forest *mdd);
	static void addMandatoryNonLeaf(const int N, const dd_edge &emptyNode,
			FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode);
	static void addSingleImplications(const int N, const dd_edge &emptyNode,
			FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode);
	static void addCrossTreeConstraints(const FeatureVisitor v,
			const dd_edge emptyNode, dd_edge &startingNode,
			xml_node<> *constraintNode, forest *mdd, int reduction_factor);
	static void addAltGroupConstraints(FeatureVisitor v, const dd_edge emptyNode,
			const int N, dd_edge &startingNode, forest *mdd);

public:
	static void printElements(std::ostream &strm, dd_edge &e);
	static dd_edge getMDDFromTuple(vector<int> tupla, forest *mdd);
	static string* parseXML(const string &fileName);
	static void printVector(vector<int> v, ostream &out);
	static double getProductCountFromFile(string fileName);
	static double getProductCountFromFile(string fileName, bool ignore);
	static double getProductCountFromFile(string fileName, bool ignore, int reduction_factor_ctc);
	static double getProductCountFromFile(string fileName, int reduction_factor_ctc);

	static bool IGNORE_HIDDEN;
	static bool SORT_CONSTRAINTS;
};

#endif /* INCLUDE_UTIL_HPP_ */
