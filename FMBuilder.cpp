#include <iostream>
#include <fstream>
#include <meddly.h>
#include <meddly_expert.h>
#include <NodeFeatureVisitor.h>
#include "rapidxml.hpp"
#include <vector>
#include "logger.hpp"
#include "Util.hpp"
#include "ConstraintVisitor.h"

using namespace MEDDLY;
using namespace std;
using namespace rapidxml;

/*
 * 				    ROOT
 * 					 |
 * 					 |
 * 			   CONNECTIVITY (OR - AT LEAST ONE)
 * 			    /	 |    \
 * 			   /	 |	   \
 * 			  /		 |	    \
 * 		BLUETOOTH   USB	   WIFI
 *
 *
 * 		ROOT = TRUE
 * 		BLUETOOTH = TRUE -> CONNECTIVITY = TRUE
 * 		USB = TRUE -> CONNECTIVITY = TRUE
 * 		WIFI = TRUE -> CONNECTIVITY = TRUE
 * 		CONNECTIVITY = TRUE -> (BLUETOOTH = TRUE OR USB = TRUE OR WIFI = TRUE)
 */
void fmConnectivity() {
	// Init MEDDLY
	initialize();
	// Create a domain
	domain *d = createDomain();
	assert(d != 0);
	// We have 5 variables, all booleans
	const int N = 5;
	int bounds[N] = { 2, 2, 2, 2, 2 };
	// Create variable in the above domain
	d->createVariablesBottomUp(bounds, N);
	cout << "Created domain with " << d->getNumVariables() << " variables\n";
	// Do not reduce the forest
	forest::policies pmdd(false);
	pmdd.setQuasiReduced();
	pmdd.setNeverDelete();
	pmdd.setFullStorage();
	pmdd.setPessimistic();
	// Create a forest in the above domain
	forest *mdd = d->createForest(false, // this is not a relation
			forest::BOOLEAN, // terminals are either true or false
			forest::MULTI_TERMINAL, // disables edge-labeling
			pmdd);
	assert(mdd != 0);
	// Display forest properties
	cout << "Created forest in this domain with:" << "\n  Relation:\tfalse"
			<< "\n  Range Type:\tBOOLEAN" << "\n  Edge Label:\tMULTI_TERMINAL"
			<< "\n";
	// Create an edge represeting the terminal node TRUE
	dd_edge emptyNode(mdd);
	dd_edge startingNode(mdd);
	mdd->createEdge(true, startingNode);
	mdd->createEdge(true, emptyNode);
	// Cardinality
	cout << "Initial cardinality: " << startingNode.getCardinality() << endl;
	// Add to the MDD the constraint ROOT = TRUE
	vector<int> v(N, -1);
	v[0] = 1;
	dd_edge c = Util::getMDDFromTuple(v, mdd);
	// Intersect this edge with the starting node
	startingNode *= c;
	// Cardinality
	cout << "Cardinality after ROOT = TRUE: " << startingNode.getCardinality()
			<< endl;
	// Add to the MDD the constraint BLUETOOTH = TRUE -> CONNECTIVITY = TRUE
	dd_edge c1 = emptyNode;
	v = vector<int>(N, -1);
	v[2] = 1; // BLUETOOTH = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 -= c; // NOT (BLUETOOTH = TRUE)
	v = vector<int>(N, -1);
	v[1] = 1; // CONNECTIVITY = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 += c; // NOT (BLUETOOTH = TRUE) OR CONNECTIVITY = TRUE
	// Intersect this edge with the starting node
	startingNode *= c1;
	// Cardinality
	cout << "Cardinality after BLUETOOTH = TRUE -> CONNECTIVITY = TRUE: "
			<< startingNode.getCardinality() << endl;
	// Add to the MDD the constraint USB = TRUE -> CONNECTIVITY = TRUE
	c1 = emptyNode;
	v = vector<int>(N, -1);
	v[3] = 1; // USB = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 -= c; // NOT (USB = TRUE)
	v = vector<int>(N, -1);
	v[1] = 1; // CONNECTIVITY = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 += c; // NOT (USB = TRUE) OR CONNECTIVITY = TRUE
	// Intersect this edge with the starting node
	startingNode *= c1;
	// Cardinality
	cout << "Cardinality after USB = TRUE -> CONNECTIVITY = TRUE: "
			<< startingNode.getCardinality() << endl;
	// Add to the MDD the constraint WIFI = TRUE -> CONNECTIVITY = TRUE
	c1 = emptyNode;
	v = vector<int>(N, -1);
	v[4] = 1; // WIFI = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 -= c; // NOT (WIFI = TRUE)
	v = vector<int>(N, -1);
	v[1] = 1; // CONNECTIVITY = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 += c; // NOT (USB = TRUE) OR CONNECTIVITY = TRUE
	// Intersect this edge with the starting node
	startingNode *= c1;
	// Cardinality
	cout << "Cardinality after WIFI = TRUE -> CONNECTIVITY = TRUE: "
			<< startingNode.getCardinality() << endl;
	// Add to the MDD the constraint CONNECTIVITY = TRUE -> (BLUETOOTH = TRUE OR USB = TRUE OR WIFI = TRUE)
	c1 = emptyNode;
	v = vector<int>(N, -1);
	v[1] = 1; // CONNECTIVITY = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 -= c; // NOT (CONNECTIVITY = TRUE)
	v = vector<int>(N, -1);
	v[2] = 1; // BLUETOOTH = TRUE
	dd_edge c3 = Util::getMDDFromTuple(v, mdd);
	v = vector<int>(N, -1);
	v[3] = 1; // USB = TRUE
	dd_edge c4 = Util::getMDDFromTuple(v, mdd);
	v = vector<int>(N, -1);
	v[4] = 1; // WIFI = TRUE
	dd_edge c5 = Util::getMDDFromTuple(v, mdd);
	c3 += c4 + c5; // BLUETOOTH = TRUE OR USB = TRUE OR WIFI = TRUE
	c1 += c3; // NOT (CONNECTIVITY = TRUE) OR (BLUETOOTH = TRUE OR USB = TRUE OR WIFI = TRUE)
	// Intersect this edge with the starting node
	startingNode *= c1;
	// Cardinality
	cout
			<< "Cardinality after CONNECTIVITY = TRUE -> (BLUETOOTH = TRUE OR USB = TRUE OR WIFI = TRUE): "
			<< startingNode.getCardinality() << endl;
	// Export the structure in DOT
	startingNode.writePicture("FM", "pdf");
	// Cardinality
	cout << "Final cardinality: " << startingNode.getCardinality() << endl;
	cleanup();
}

/*
 * 				    ROOT-------------------------------------------------------------
 * 					 |																|
 * 					 |																|
 * 			   CONNECTIVITY (MANDATORY, ONLY ONE)								 DISPLAY
 * 			    /	 |    \
 * 			   /	 |	   \
 * 			  /		 |	    \
 * 		BLUETOOTH   USB	   WIFI
 *
 *
 * 		ROOT = TRUE
 * 		CONNECTITIVITY = BLUETOOTH -> DISPLAY = TRUE
 */
void fmConnectivityWithMandatoryOnlyOne() {
	// Init MEDDLY
	initialize();
	// Create a domain
	domain *d = createDomain();
	assert(d != 0);
	// We have 3 variables, all booleans
	const int N = 3;
	int bounds[N] = { 2, 3, 2 };
	// Create variable in the above domain
	d->createVariablesBottomUp(bounds, N);
	cout << "Created domain with " << d->getNumVariables() << " variables\n";
	// Do not reduce the forest
	forest::policies pmdd(false);
	pmdd.setQuasiReduced();
	pmdd.setNeverDelete();
	pmdd.setFullStorage();
	pmdd.setPessimistic();
	// Create a forest in the above domain
	forest *mdd = d->createForest(false, // this is not a relation
			forest::BOOLEAN, // terminals are either true or false
			forest::MULTI_TERMINAL, // disables edge-labeling
			pmdd);
	assert(mdd != 0);
	// Display forest properties
	cout << "Created forest in this domain with:" << "\n  Relation:\tfalse"
			<< "\n  Range Type:\tBOOLEAN" << "\n  Edge Label:\tMULTI_TERMINAL"
			<< "\n";
	// Create an edge represeting the terminal node TRUE
	dd_edge emptyNode(mdd);
	dd_edge startingNode(mdd);
	mdd->createEdge(true, startingNode);
	mdd->createEdge(true, emptyNode);
	// Cardinality
	cout << "Initial cardinality: " << startingNode.getCardinality() << endl;
	// Add to the MDD the constraint ROOT = TRUE
	vector<int> v(N, -1);
	v[0] = 1;
	dd_edge c = Util::getMDDFromTuple(v, mdd);
	// Intersect this edge with the starting node
	startingNode *= c;
	// Cardinality
	cout << "Cardinality after ROOT = TRUE: " << startingNode.getCardinality()
			<< endl;
	// Add to the MDD the constraint CONNECTITIVITY = BLUETOOTH -> DISPLAY = TRUE
	dd_edge c1 = emptyNode;
	v = vector<int>(N, -1);
	v[1] = 0; // CONNECTITIVITY = BLUETOOTH
	c = Util::getMDDFromTuple(v, mdd);
	c1 -= c; // NOT (CONNECTITIVITY = BLUETOOTH)
	v = vector<int>(N, -1);
	v[2] = 1; // DISPLAY = TRUE
	c = Util::getMDDFromTuple(v, mdd);
	c1 += c; // NOT (CONNECTITIVITY = BLUETOOTH) OR DISPLAY = TRUE
	// Intersect this edge with the starting node
	startingNode *= c1;
	// Cardinality
	cout << "Cardinality after CONNECTITIVITY = BLUETOOTH -> DISPLAY = TRUE: "
			<< startingNode.getCardinality() << endl;
	// Export the structure in DOT
	startingNode.writePicture("FMConnectivityMultipleValues", "pdf");
	// Cardinality
	cout << "Final cardinality: " << startingNode.getCardinality() << endl;
}

dd_edge addMandatory(const dd_edge &emptyNode, const int N, FeatureVisitor &v,
		forest *mdd) {
	// Add the mandatory constraint for the root
	dd_edge c = emptyNode;
	vector<int> constraint = vector<int>(N, -1);
	vector<int> indxs = v.getMandatoryIndex();
	for (unsigned int i = 0; i < indxs.size(); i++) {
		logcout(LOG_DEBUG) << "Variable with index " << indxs[i]
				<< " set as MANDATORY" << endl;
		constraint[N - indxs[i] - 1] = 1;
	}
	c = Util::getMDDFromTuple(constraint, mdd);
	return c;
}

void addOrGroupConstraints(FeatureVisitor v, const dd_edge emptyNode,
		const int N, dd_edge &startingNode, forest *mdd) {
	dd_edge c(mdd);
	dd_edge cTemp(mdd);
	vector<int> constraint = vector<int>(N, -1);
	// Add the OR constraints
	vector<pair<pair<int, int>, vector<int>*>> orIndxs = v.getOrIndexs();
	for (unsigned int i = 0; i < orIndxs.size(); i++) {
		logcout(LOG_DEBUG)
				<< "Adding constraint for OR-Group elements with their root [Index: "
				<< orIndxs[i].first.first << ", None Value: "
				<< v.getValueForVar(orIndxs[i].first.first,
						orIndxs[i].first.second) << "]\n";
		constraint = vector<int>(N, -1);
		// PARENT AVAILABLE -> OR BETWEEN CHILDREN
		constraint[N - orIndxs[i].first.first - 1] = orIndxs[i].first.second;
		c = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		cTemp = emptyNode;
		vector<int> *idx = orIndxs[i].second;
		cout << "\t" << idx->size() << endl;
		for (unsigned int j = 0; j < idx->size(); j++) {
			constraint = vector<int>(N, -1);
			constraint[N - idx->data()[j] - 1] = 1;
			Util::printVector(constraint, logcout(LOG_DEBUG));
			if (j == 0)
				cTemp *= Util::getMDDFromTuple(constraint, mdd);
			else
				cTemp += Util::getMDDFromTuple(constraint, mdd);
		}
		// NOT PARENT AVAILABLE -> CHILDREN
		c = c + cTemp;

		// Intersect this edge with the starting node
		startingNode *= c;
	}

	// ORs non leaf
	vector<pair<pair<int, int>, vector<pair<int, int>>*>> orIndxNonLeaf = v.getOrIndexsNonLeaf();
	for (unsigned int i = 0; i < orIndxNonLeaf.size(); i++) {
			logcout(LOG_DEBUG)
					<< "Adding constraint for OR-Group elements with their root [Index: "
					<< orIndxNonLeaf[i].first.first << ", Value: "
					<< v.getValueForVar(orIndxNonLeaf[i].first.first,
							orIndxNonLeaf[i].first.second) << "]\n";
			constraint = vector<int>(N, -1);
			// PARENT AVAILABLE -> OR BETWEEN CHILDREN
			constraint[N - orIndxNonLeaf[i].first.first - 1] = orIndxNonLeaf[i].first.second;
			c = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			cTemp = emptyNode;
			vector<pair<int, int>> *idx = orIndxNonLeaf[i].second;
			for (unsigned int j = 0; j < idx->size(); j++) {
				constraint = vector<int>(N, -1);
				constraint[N - idx->data()[j].first - 1] = idx->data()[j].second;
				if (j == 0)
					cTemp *= Util::getMDDFromTuple(constraint, mdd);
				else
					cTemp += Util::getMDDFromTuple(constraint, mdd);
			}
			// NOT PARENT AVAILABLE -> CHILDREN
			c = c + cTemp;

			// Intersect this edge with the starting node
			startingNode *= c;
		}
}

void addMandatoryNonLeaf(const int N, const dd_edge &emptyNode,
		FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode) {
	// Add the mandatory constraint for the other features
	vector<int> constraint = vector<int>(N, -1);
	vector<pair<pair<int, int>, pair<int, int> > > mandatoryImplications =
			v.getMandatoryImplications();
	for (unsigned int i = 0; i < mandatoryImplications.size(); i++) {
		logcout(LOG_DEBUG) << "Adding constraint [Index: "
				<< mandatoryImplications[i].first.first << ", Value: "
				<< v.getValueForVar(mandatoryImplications[i].first.first,
						mandatoryImplications[i].first.second)
				<< "] <=> [Index: " << mandatoryImplications[i].second.first
				<< ", Value: "
				<< v.getValueForVar(mandatoryImplications[i].second.first,
						mandatoryImplications[i].second.second) << "]\n";
		c = emptyNode;
		dd_edge tempC(mdd);
		dd_edge tempC1(mdd);
		mdd->createEdge(true, tempC);
		mdd->createEdge(true, tempC1);
		constraint = vector<int>(N, -1);
		// A
		constraint[N - mandatoryImplications[i].first.first - 1] =
				mandatoryImplications[i].first.second;
		tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		// B
		constraint = vector<int>(N, -1);
		constraint[N - mandatoryImplications[i].second.first - 1] =
				mandatoryImplications[i].second.second;
		tempC1 = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		// C = A <=> B
		apply(EQUAL, tempC, tempC1, c);
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << c.getCardinality()
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		logcout(LOG_DEBUG) << "\tNew cardinality: "
						<< startingNode.getCardinality() << endl;
	}
}

void addSingleImplications(const int N, const dd_edge &emptyNode,
		FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode) {
	// Add the mandatory constraint for the other features
	vector<int> constraint = vector<int>(N, -1);
	vector<pair<pair<int, int>, pair<int, int> > > singleImplications =
			v.getSingleImplications();
	for (unsigned int i = 0; i < singleImplications.size(); i++) {
		logcout(LOG_DEBUG) << "Adding constraint [Index: "
				<< singleImplications[i].second.first << ", Value: "
				<< v.getValueForVar(singleImplications[i].second.first,
						singleImplications[i].second.second) << "] => [Index: "
				<< singleImplications[i].first.first << ", Value: "
				<< v.getValueForVar(singleImplications[i].first.first,
						singleImplications[i].first.second) << "]\n";
		c = emptyNode;
		dd_edge tempC(mdd);
		dd_edge tempC1(mdd);
		mdd->createEdge(true, tempC);
		mdd->createEdge(true, tempC1);
		constraint = vector<int>(N, -1);
		// A
		constraint[N - singleImplications[i].second.first - 1] =
				singleImplications[i].second.second;
		tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		// B
		constraint = vector<int>(N, -1);
		constraint[N - singleImplications[i].first.first - 1] =
				singleImplications[i].first.second;
		tempC1 = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		// C = A => B = notA or B
		tempC = emptyNode - tempC;
		c = tempC + tempC1;
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << c.getCardinality()
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		logcout(LOG_DEBUG) << "\tNew cardinality: "
				<< startingNode.getCardinality() << endl;
	}
}

void addCrossTreeConstraints(const FeatureVisitor v, const dd_edge emptyNode,
		dd_edge &startingNode, xml_node<> *constraintNode, forest *mdd) {
	// Add Cross Tree Constraints
	ConstraintVisitor cVisitor(v, emptyNode, mdd);
	// Visit the sub-tree for constraints and create a set of edges for each of them
	cVisitor.visit(constraintNode);
	// Now, compute the intersection between startingNode and each of the constraint
	vector<dd_edge> constraintList = cVisitor.getConstraintMddList();
	for (dd_edge e : constraintList) {
		startingNode *= e;
	}
}

int readFmFromFile(string fileName) {
	// Open and read the file, then visit it
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

	FeatureVisitor v;
	v.visit(structNode->first_node());
	v.printVariablesInMap();

	// Init MEDDLY
	initialize();
	// Create a domain
	domain *d = createDomain();
	assert(d != 0);
	// We have 3 variables, all booleans
	const int N = v.getNVar();
	int *bounds = v.getBounds();
	// Create variable in the above domain
	d->createVariablesBottomUp(bounds, N);
	logcout(LOG_DEBUG) << "Created domain with " << d->getNumVariables()
			<< " variables\n";
	logcout(LOG_DEBUG) << "Bounds: " << endl;
	for (int i = 0; i < N; i++)
		logcout(LOG_DEBUG) << "\t" << bounds[i] << endl;
	// Do not reduce the forest
	forest::policies pmdd(false);
	pmdd.setQuasiReduced();
	pmdd.setNeverDelete();
	pmdd.setFullStorage();
	pmdd.setPessimistic();
	// Create a forest in the above domain
	forest *mdd = d->createForest(false, // this is not a relation
			forest::BOOLEAN, // terminals are either true or false
			forest::MULTI_TERMINAL, // disables edge-labeling
			pmdd);
	assert(mdd != 0);
	// Display forest properties
	logcout(LOG_DEBUG) << "Created forest in this domain with:"
			<< "\n  Relation:\tfalse" << "\n  Range Type:\tBOOLEAN"
			<< "\n  Edge Label:\tMULTI_TERMINAL" << "\n";
	// Create an edge representing the terminal node TRUE
	dd_edge emptyNode(mdd);
	dd_edge startingNode(mdd);
	mdd->createEdge(true, startingNode);
	mdd->createEdge(true, emptyNode);
	// Cardinality
	logcout(LOG_DEBUG) << "Initial cardinality: "
			<< startingNode.getCardinality() << endl;
	// Add the mandatory constraint for the root
	dd_edge c = addMandatory(emptyNode, N, v, mdd);
	// Intersect this edge with the starting node
	startingNode *= c;

	// Cardinality
	logcout(LOG_DEBUG)
			<< "Cardinality after mandatory constraints [usually for root]: "
			<< startingNode.getCardinality() << endl;

	// Add the mandatory constraint for the other features
	addMandatoryNonLeaf(N, emptyNode, v, c, mdd, startingNode);
	// Cardinality
	logcout(LOG_DEBUG) << "Cardinality after mandatory for other features: "
			<< startingNode.getCardinality() << endl;

	// Add the OR constraints
	addOrGroupConstraints(v, emptyNode, N, startingNode, mdd);
	// Cardinality
	logcout(LOG_DEBUG) << "Cardinality after OR groups: "
			<< startingNode.getCardinality() << endl;

	// Add single implication constraints for each feature: a feature can be
	// included only if the parent is included
	addSingleImplications(N, emptyNode, v, c, mdd, startingNode);
	// Cardinality
	logcout(LOG_DEBUG)
			<< "Final cardinality after dependencies between features: "
			<< startingNode.getCardinality() << endl;

	// Add Cross Tree Constraints
	xml_node<> *constraintNode = structNode->parent()->first_node(
			"constraints");
	addCrossTreeConstraints(v, emptyNode, startingNode, constraintNode, mdd);
	// Cardinality
	logcout(LOG_INFO) << "Number of valid products: "
			<< startingNode.getCardinality() << endl;

	return startingNode.getCardinality();
}

int main(int argc, char **argv) {
//	cout << "####### CONNECTIVITY #######" << endl;
//	fmConnectivity();
//	cout << "####### CONNECTIVITY MULTIPLE VALUES #######" << endl;
//	fmConnectivityWithMandatoryOnlyOne();
//
//
//
//
//	if (readFmFromFile("examples/gplTinyModel.xml") != 6)
//		cerr << "Error for gplTinyModel" << endl;
//	if (readFmFromFile("examples/carModel.xml") != 7)
//		cerr << "Error for carModel" << endl;
//	if (readFmFromFile("examples/aplModel.xml") != 159184)
//			cerr << "Error for aplModel" << endl;
	if (readFmFromFile("examples/gplModel.xml") != 186)
				cerr << "Error for gplModel" << endl;


	return 0;
}
