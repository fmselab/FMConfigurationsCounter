/*
 * Util.cpp
 *
 *  Created on: 28 mar 2023
 */

#include "Util.hpp"
#include <chrono>

bool Util::IGNORE_HIDDEN = false;
bool Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
bool Util::SHUFFLE_CONSTRAINTS = false;
bool Util::PRINT_MDD = true;
bool Util::REORDER_VARIABLES = true;
double Util::N_MAX_NODES = 0;
double Util::N_MAX_EDGES = 0;

/**
 * Given the file name, it returns the count of the products
 *
 * It uses by default a constraint reduction factor of 0 and
 * does not ignore hidden features
 *
 * @param fileName the name of the file
 * @return the number of valid products
 */
string Util::getProductCountFromFile(string fileName) {
	return getProductCountFromFile(fileName, 0);
}

/**
 * Given the file name, it returns the count of the products
 *
 * It uses by default a constraint reduction factor of 0
 *
 * @param fileName the name of the file
 * @param ignore a boolean parameter setting whether the hidden features have to be ignored or not
 * @return the number of valid products
 */
string Util::getProductCountFromFile(string fileName, bool ignore) {
	Util::IGNORE_HIDDEN = ignore;
	return getProductCountFromFile(fileName, 0);
}

/**
 * Given the file name, it returns the count of the products
 *
 * @param fileName the name of the file
 * @param ignore a boolean parameter setting whether the hidden features have to be ignored or not
 * @param reduction_factor_ctc the constraint reduction factor (i.e., the size of the groups merging
 * 		together the constraints before being applied to the MDD)
 * @return the number of valid products
 */
string Util::getProductCountFromFile(string fileName, bool ignore,
		int reduction_factor_ctc) {
	Util::IGNORE_HIDDEN = ignore;
	return getProductCountFromFile(fileName, reduction_factor_ctc);
}

/**
 * Given the file name, it returns the count of the products
 *
 * @param fileName the name of the file
 * @param reduction_factor_ctc the constraint reduction factor (i.e., the size of the groups merging
 * 		together the constraints before being applied to the MDD)
 * @return the number of valid products
 */
string Util::getProductCountFromFile(string fileName, int reduction_factor_ctc) {
	// Open and read the file, then visit it
	std::string *fileToString = Util::parseXML(fileName);
	// Parse the file
	xml_document<> doc;
	doc.parse<0>(const_cast<char*>(fileToString->c_str()));
	xml_node<> *structNode = doc.first_node()->first_node("struct");

	FeatureVisitor v(IGNORE_HIDDEN);
	v.visit(structNode->first_node());
	v.printDefinedVariables();

	// Init MEDDLY
	initialize();

	// Create a domain
	domain *d = domain::create();
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
	policies pmdd(false);
	pmdd.setFullyReduced();
	pmdd.setSinkDown();
	pmdd.setPessimistic();
	// Create a forest in the above domain
	forest *mdd = forest::create(d, false, 	 // this is not a relation
			range_type::BOOLEAN, 			 // terminals are either true or false
			edge_labeling::MULTI_TERMINAL, 	 // disables edge-labeling
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

#ifdef __GMP_H__
	mpz_t card;
	mpz_init(card);
#else
	double card;
#endif

	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG) << "Initial cardinality: " << card << endl;

	// Add the mandatory constraint for the root
	dd_edge c = addMandatory(emptyNode, N, v, mdd);
	// Intersect this edge with the starting node
	startingNode *= c;

	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG)
			<< "Cardinality after mandatory constraints [usually for root]: "
			<< card << endl;

	// Add the mandatory constraint for the other features
	addMandatoryNonLeaf(N, emptyNode, v, c, mdd, startingNode);
	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG) << "Cardinality after mandatory for other features: "
			<< card << endl;

	// Add the OR constraints
	addOrGroupConstraints(v, emptyNode, N, startingNode, mdd);
	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG) << "Cardinality after OR groups: "
			<< card << endl;

	// Add the constraints for alternatives converted as boolean
	addAltGroupConstraints(v, emptyNode, N, startingNode, mdd);
	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG) << "Cardinality after special ALT groups: "
			<< card << endl;

	// Add single implication constraints for each feature: a feature can be
	// included only if the parent is included
	addSingleImplications(N, emptyNode, v, c, mdd, startingNode);
	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_DEBUG)
			<< "Final cardinality after dependencies between features: "
			<< card << endl;

	// Add Cross Tree Constraints
	xml_node<> *constraintNode = structNode->parent()->first_node(
			"constraints");
	// Add Cross Tree Constraints
	if (constraintNode != NULL) {
		addCrossTreeConstraints(v, emptyNode, startingNode, constraintNode, mdd,
				reduction_factor_ctc);
	}
	// Cardinality
	apply(CARDINALITY,startingNode, card);
	logcout(LOG_INFO) << "Number of valid products: "
			<< card << endl;

	if (PRINT_MDD) {
		dot_maker mdd_dot(mdd, "MDD");
		mdd_dot.addRootEdge(startingNode);
		mdd_dot.doneGraph();
	}

	mdd->removeAllComputeTableEntries();
	mdd->removeStaleComputeTableEntries();

	delete fileToString;
	delete bounds;

#ifdef __GMP_H__
	return mpz_get_str(NULL, 10, card);
#else
	return to_string(card);
#endif
}

/**
 * Prints the valid elements, by extracting them from an MDD
 *
 * @param strm the output stream to be used
 * @param e the base edge of the MDD
 */
void Util::printElements(std::ostream &strm, dd_edge &e) {
	int nLevels = ((e.getForest())->getDomain())->getNumVariables();
	for (enumerator iter(e); iter; ++iter) {
		const int *minterm = iter.getAssignments();
		for (int i = nLevels; i > 0; i--) {
			strm << minterm[i] << ";";
		}
		strm << "\n";
	}
}

/**
 * It translates a tuple (i.e., a specific assignment) to the corresponfing MDD
 *
 * @param tupla the tuple
 * @param mdd the forest containing the destination MDD
 * @return the starting edge of the new MDD representing the tuple of interest
 */
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

/**
 * It parses an XML file and returns its string representation
 *
 * @param fileName the file name
 * @return a string pointer to the string content of the file
 */
string* Util::parseXML(const string &fileName) {
	// Open and read the file
	std::ifstream t(fileName);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string *buffer = new string(size, ' ');
	t.seekg(0);
	t.read(&(*buffer)[0], size);
	return buffer;
}

/**
 * Utility method to print an integer vector on the desired output stream
 *
 * @param v the vector to be prented
 * @param out the output to be used
 */
void Util::printVector(vector<int> v, std::ostream &out) {
	out << "\t";
	for (int i : v) {
		out << i << " ";
	}
	out << endl;
}

dd_edge Util::addMandatory(const dd_edge &emptyNode, const int N,
		FeatureVisitor &v, forest *mdd) {
	// Add the mandatory constraint for the root
	dd_edge c;
	vector<int> constraint = vector<int>(N, -1);
	vector<int> indxs = v.getMandatoryIndex();
	for (unsigned int i = 0; i < indxs.size(); i++) {
		int noneIndex = v.getIndexOfNoneForVariable(indxs[i]);
		logcout(LOG_DEBUG) << "Variable with index " << indxs[i]
				<< " set as MANDATORY" << endl;
		constraint[N - indxs[i] - 1] = noneIndex;
	}
	c = Util::getMDDFromTuple(constraint, mdd);
	c = emptyNode - c;
	return c;
}

void Util::addOrGroupConstraints(FeatureVisitor v, const dd_edge emptyNode,
		const int N, dd_edge &startingNode, forest *mdd) {
	dd_edge c(mdd);
	dd_edge cTemp(mdd);
	vector<int> constraint;
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
		vector<int> *idx = orIndxs[i].second;
		cout << "\t" << idx->size() << endl;
		for (unsigned int j = 0; j < idx->size(); j++) {
			constraint = vector<int>(N, -1);
			constraint[N - idx->data()[j] - 1] = 1;
			Util::printVector(constraint, logcout(LOG_DEBUG));
			if (j == 0)
				cTemp = Util::getMDDFromTuple(constraint, mdd);
			else
				cTemp += Util::getMDDFromTuple(constraint, mdd);
		}
		// NOT PARENT AVAILABLE -> CHILDREN
		c = c + cTemp;

		// Intersect this edge with the starting node
		startingNode *= c;
	}

	// ORs non leaf
	vector<pair<pair<int, int>, vector<pair<int, int>>*>> orIndxNonLeaf =
			v.getOrIndexsNonLeaf();
	for (unsigned int i = 0; i < orIndxNonLeaf.size(); i++) {
		logcout(LOG_DEBUG)
				<< "Adding constraint for OR-Group elements with their root [Index: "
				<< orIndxNonLeaf[i].first.first << ", NoneValue: "
				<< v.getValueForVar(orIndxNonLeaf[i].first.first,
						orIndxNonLeaf[i].first.second) << "]\n";
		constraint = vector<int>(N, -1);
		// PARENT AVAILABLE -> OR BETWEEN CHILDREN
		constraint[N - orIndxNonLeaf[i].first.first - 1] =
				orIndxNonLeaf[i].first.second;
		c = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		cTemp = emptyNode;
		vector<pair<int, int>> *idx = orIndxNonLeaf[i].second;
		for (unsigned int j = 0; j < idx->size(); j++) {
			constraint = vector<int>(N, -1);
			constraint[N - idx->data()[j].first - 1] = idx->data()[j].second;
			Util::printVector(constraint, logcout(LOG_DEBUG));
			dd_edge thisConstraint = Util::getMDDFromTuple(constraint, mdd);
			thisConstraint = emptyNode - thisConstraint;
			if (j == 0)
				cTemp *= thisConstraint;
			else
				cTemp += thisConstraint;
		}
		// NOT PARENT AVAILABLE -> CHILDREN
		c = c + cTemp;

		// Intersect this edge with the starting node
		startingNode *= c;
	}
}

void Util::addAltGroupConstraints(FeatureVisitor v, const dd_edge emptyNode,
		const int N, dd_edge &startingNode, forest *mdd) {
	// ALT-groups non leaf
	vector<pair<pair<int, int>, vector<pair<int, int>>*>> altIndexesExclusion =
			v.getAltIndexesExclusion();
	for (pair<pair<int, int>, vector<pair<int, int>>*> vAlt : altIndexesExclusion) {
		logcout(LOG_DEBUG)
				<< "Adding constraint for ALT-Group elements with their root [Index: "
				<< vAlt.first.first << ", None Value: "
				<< v.getValueForVar(vAlt.first.first, vAlt.first.second)
				<< "]\n";
		dd_edge c(mdd);
		dd_edge cTemp(mdd);
		dd_edge cTemp2(mdd);
		vector<int> constraint;
		// Every element is a single ALT Group.
		// We need to add the constraint ONE SELECTED => ALL THE OTHER UNSELECTED
		// It is computed as NOT (ONE SELECTED) OR (ALL THE OTHER UNSELECTED)
		for (unsigned int i = 0; i < vAlt.second->size(); i++) {
			// not i-th element selected (i.e., the i-th element as NONE)
			constraint = vector<int>(N, -1);
			constraint[N - vAlt.second->data()[i].first - 1] =
					vAlt.second->data()[i].second;
			c = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			cTemp = emptyNode;
			for (unsigned int j = 0; j < vAlt.second->size(); j++) {
				if (i != j) {
					// j-th element unselected
					constraint = vector<int>(N, -1);
					constraint[N - vAlt.second->data()[j].first - 1] =
							vAlt.second->data()[j].second;
					// compute the AND with the other siblings
					cTemp *= Util::getMDDFromTuple(constraint, mdd);
				}
			}
			// OR between c and cTemp
			c = c + cTemp;

			// Intersect this edge with the starting node
			startingNode *= c;
		}

		// Now, we need to add the constraint PARENT => AT LEAST ONE SELECTED
		// (i.e. [not parent] or [or between children])
		// not parent
		constraint = vector<int>(N, -1);
		constraint[N - vAlt.first.first - 1] = vAlt.first.second;
		c = Util::getMDDFromTuple(constraint, mdd);
		// or between children
		for (unsigned int i = 0; i < vAlt.second->size(); i++) {
			// i-th element selected (i.e., the i-th element != NONE)
			constraint = vector<int>(N, -1);
			constraint[N - vAlt.second->data()[i].first - 1] =
					vAlt.second->data()[i].second;
			cTemp = emptyNode - Util::getMDDFromTuple(constraint, mdd);
			if (i == 0)
				cTemp2 = cTemp;
			else
				cTemp2 = cTemp2 + cTemp;
		}
		// [not parent] or [or between children]
		c = c + cTemp2;
		// Intersect this edge with the starting node
		startingNode *= c;
	}
}

void Util::addMandatoryNonLeaf(const int N, const dd_edge &emptyNode,
		FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode) {
	// Add the mandatory constraint for the other features
	vector<int> constraint;
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
		if (mandatoryImplications[i].first.second >= v.getBoundForVar(mandatoryImplications[i].first.first)) {
			constraint[N - mandatoryImplications[i].first.first - 1] =
					mandatoryImplications[i].first.second - v.getBoundForVar(mandatoryImplications[i].first.first);
			tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			tempC = emptyNode - tempC;
		} else {
			constraint[N - mandatoryImplications[i].first.first - 1] =
							mandatoryImplications[i].first.second;
			tempC = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		}

		// B
		constraint = vector<int>(N, -1);
		if (mandatoryImplications[i].second.second >= v.getBoundForVar(mandatoryImplications[i].second.first)) {
			constraint[N - mandatoryImplications[i].second.first - 1] =
					mandatoryImplications[i].second.second - v.getBoundForVar(mandatoryImplications[i].second.first);
			tempC1 = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
			tempC1 = emptyNode - tempC1;
		} else {
			constraint[N - mandatoryImplications[i].second.first - 1] =
							mandatoryImplications[i].second.second;
			tempC1 = Util::getMDDFromTuple(constraint, mdd) * emptyNode;
		}

		// C = A <=> B
		apply(EQUAL, tempC, tempC1, c);
#ifdef __GMP_H__
		mpz_t card;
		mpz_init(card);
#else
		double card;
#endif
		apply(CARDINALITY,c, card);
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << card
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		apply(CARDINALITY,startingNode, card);
		logcout(LOG_DEBUG) << "\tNew cardinality: "
				<< card << endl;
	}
}

void Util::addSingleImplications(const int N, const dd_edge &emptyNode,
		FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode) {
	vector<int> constraint;
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
#ifdef __GMP_H__
		mpz_t card;
		mpz_init(card);
#else
		double card;
#endif
		apply(CARDINALITY,startingNode, card);
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << card
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		apply(CARDINALITY,startingNode, card);
		logcout(LOG_DEBUG) << "\tNew cardinality: "
				<< card << endl;
	}

	// Add the mandatory constraint for the other features non leaf
	singleImplications = v.getSingleImplicationsNonLeaf();
	for (unsigned int i = 0; i < singleImplications.size(); i++) {
		logcout(LOG_DEBUG) << "Adding constraint for dependency not[Index: "
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
		c = tempC + tempC1;
		double card;
		apply(CARDINALITY,startingNode, card);
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << card
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		apply(CARDINALITY,startingNode, card);
		logcout(LOG_DEBUG) << "\tNew cardinality: "
				<< card << endl;
	}
}

/**
 * Comparator used for comparing edges in an MDD
 *
 * @param e1 the first edge
 * @param e2 the second edge
 * @return TRUE if the number of edges of the MDD starting with e1 is lower than that of e2,
 *         FALSE otherwise
 */
bool compareEdges(dd_edge e1, dd_edge e2) {
	return (e1.getNodeCount() < e2.getNodeCount());
}

/**
 * Adds to the MDD the Crosstree Constraints.
 *
 * First, it visit the Constraint part of the XML file. Then, if needed, the constraints are
 * sorted from the one having the lowest complexity to the one with the highest.
 * Finally, the constraints are applied to the MDD by computing the intersection with the
 * current initial node of the MDD.
 *
 * @param v the FeatureVisitor, used for accessing to variables information
 * @param emptyNode the empty node
 * @param startingNode the initial node of the MDD being built
 * @param constraintNode the XML node from which the constraints start
 * @param mdd the forest
 * @param reduction_factor the reduction factor to be used for compressing constraints
 *   before applying them to the MDD
 */
void Util::addCrossTreeConstraints(const FeatureVisitor v,
		const dd_edge emptyNode, dd_edge &startingNode,
		xml_node<> *constraintNode, forest *mdd, int reduction_factor) {
	ConstraintVisitor cVisitor(v, emptyNode, mdd);
	int i = 0;
	// Visit the sub-tree for constraints and create a set of edges for each of them
	cVisitor.visit(constraintNode, reduction_factor);
	// Now, compute the intersection between startingNode and each of the constraint
	vector<dd_edge> constraintList = cVisitor.getConstraintMddList();
	// Order the vector from the lowest cardinality to the highest
	if (SORT_CONSTRAINTS_WHEN_APPLYING) {
		sort(constraintList.begin(), constraintList.end(), compareEdges);
	}
	// Apply the constraints
	i = 0;
#ifdef __GMP_H__
	mpz_t card;
	mpz_init(card);
#else
	double card;
#endif

	int oldNodes = 0;

	for (dd_edge& e : constraintList) {
		try {
			startingNode *= e;

			unsigned long nodes = startingNode.getNodeCount();
			if (Util::REORDER_VARIABLES && i != 0 && (long unsigned int)i != constraintList.size() - 1) {
				if ((nodes > 1.5 * oldNodes && nodes < 1000000 && nodes > 100000) || (nodes > 1.1 * oldNodes && nodes > 1000000)) {
					logcout(LOG_DEBUG) << "\t\tStart reordering" << endl;
					mdd->removeAllComputeTableEntries();
					int numVariables = mdd->getNumVariables();
					mdd->dynamicReorderVariables(numVariables,1);
					logcout(LOG_DEBUG) << "\t\tEnd reordering" << endl;
				}
			}

			apply(CARDINALITY,startingNode, card);
			logcout(LOG_DEBUG) << "\tNew cardinality after constraint " << (++i)
					<< ": " << card << " - Edges: "
					<< startingNode.getEdgeCount() << " - Nodes: "
					<< nodes << endl;
			e.detach();

			if (startingNode.getNodeCount() > N_MAX_NODES)
				N_MAX_NODES = startingNode.getNodeCount();

			if (startingNode.getEdgeCount()> N_MAX_EDGES)
				N_MAX_EDGES = startingNode.getEdgeCount();

			oldNodes = nodes;

		} catch(MEDDLY::error& e) {
			cerr   << "\nCaught meddly error '" << e.getName()
				<< "'\n thrown in " << e.getFile()
				<< " line " << e.getLine() << "\n";
		}
	}
}
