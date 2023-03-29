/*
 * FMBuilderTest.cpp
 *
 *  Created on: 29 mar 2023
 *      Author: bombarda
 */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "Util.hpp"

TEST_CASE("gplTinyModel without ignore", "gplTinyModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE(Util::getProductCountFromFile("examples/gplTinyModel.xml") == 6);
}

TEST_CASE("gplTinyModel with ignore", "gplTinyModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE(Util::getProductCountFromFile("examples/gplTinyModel.xml") == 6);
}

TEST_CASE("carModel with ignore", "carModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE(Util::getProductCountFromFile("examples/carModel.xml") == 7);
}

TEST_CASE("carModel without ignore", "carModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE(Util::getProductCountFromFile("examples/carModel.xml") == 7);
}

TEST_CASE("aplModel with ignore", "aplModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE(Util::getProductCountFromFile("examples/aplModel.xml") == 159120);
}

TEST_CASE("aplModel without ignore", "aplModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE(Util::getProductCountFromFile("examples/aplModel.xml") == 159184);
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
TEST_CASE("First attempt of constructing an MDD for a FeatureModel", "1Test") {
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
TEST_CASE("Second attempt of constructing an MDD for a FeatureModel", "2Test") {
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

