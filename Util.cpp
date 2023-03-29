/*
 * Util.cpp
 *
 *  Created on: 28 mar 2023
 *      Author: parallels
 */

#include "Util.hpp"

void Util::printElements(std::ostream &strm, dd_edge &e) {
	int nLevels = ((e.getForest())->getDomain())->getNumVariables();
	for (enumerator iter(e); iter; ++iter) {
		const int *minterm = iter.getAssignments();
		strm << "[";
		for (int i = nLevels; i > 0; i--) {
			strm << " " << minterm[i];
		}
		switch ((e.getForest())->getRangeType()) {
		case forest::BOOLEAN:
			strm << " --> T]\n";
			break;
		case forest::INTEGER: {
			int val = 0;
			iter.getValue(val);
			strm << " --> " << val << "]\n";
		}
			break;
		case forest::REAL: {
			int val = 0;
			iter.getValue(val);
			strm << " --> " << val << "]\n"; //&&%0.3f
		}
			break;
		default:
			strm << "Error: invalid range_type\n";
		}
	}
}

dd_edge Util::getMDDFromTuple(vector<int> tupla, forest *mdd) {
	const int N = tupla.size();

	// Create an element to insert in the MDD
	// Note that this is of size (N + 1), since [0] is a special level handle
	// dedicated to terminal nodes.
	int *elementList[1];

	elementList[0] = new int[N + 1];

	int i = 1;
	for (vector<int>::reverse_iterator it = tupla.rbegin(), end = tupla.rend();
			it != end; ++it) {
		elementList[0][i++] = *it;// Starting from the last element to the first
	}

	// Create edge for the tuple
	dd_edge element(mdd);
	mdd->createEdge(elementList, 1, element);
	// Clean up the memory
	delete elementList[0];
	return element;
}

xml_node<>* Util::parseXML(const string &fileName) {
	// Open and read the file
	std::ifstream t(fileName);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string buffer(size, ' ');
	t.seekg(0);
	t.read(&buffer[0], size);
	// Parse the file
	xml_document<> doc;
	doc.parse<0>(const_cast<char*>(buffer.c_str()));
	xml_node<>* structNode = doc.first_node()->first_node("struct");
	return structNode;
}

void Util::printVector(vector<int> v, std::ostream& out) {
	out << "\t";
	for (int i: v) {
		out << i << " ";
	}
	out << endl;
}
