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
#include <fstream>
#include "rapidxml.hpp"

using namespace rapidxml;
using namespace MEDDLY;
using namespace std;

class Util {
public:
	static void printElements(std::ostream &strm, dd_edge &e);
	static dd_edge getMDDFromTuple(vector<int> tupla, forest *mdd);
	static xml_node<>* parseXML(const string &fileName);
	static void printVector(vector<int> v, ostream& out);
};

#endif /* INCLUDE_UTIL_HPP_ */
