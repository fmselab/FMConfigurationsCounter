/*
 * Util.cpp
 *
 *  Created on: 28 mar 2023
 *      Author: parallels
 */

#include "Util.hpp"

bool Util::IGNORE_HIDDEN = false;

int Util::getProductCountFromFile(string fileName) {
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
	xml_node<> *structNode = doc.first_node()->first_node("struct");

	FeatureVisitor v(IGNORE_HIDDEN);
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
	xml_node<> *structNode = doc.first_node()->first_node("struct");
	return structNode;
}

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
		logcout(LOG_DEBUG) << "Variable with index " << indxs[i]
				<< " set as MANDATORY" << endl;
		constraint[N - indxs[i] - 1] = 1;
	}
	c = Util::getMDDFromTuple(constraint, mdd);
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

void Util::addSingleImplications(const int N, const dd_edge &emptyNode,
		FeatureVisitor &v, dd_edge &c, forest *mdd, dd_edge &startingNode) {
	// Add the mandatory constraint for the other features
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
		logcout(LOG_DEBUG) << "\tConstraint cardinality: " << c.getCardinality()
				<< endl;
		// Intersect this edge with the starting node
		startingNode *= c;
		logcout(LOG_DEBUG) << "\tNew cardinality: "
				<< startingNode.getCardinality() << endl;
	}
}

void Util::addCrossTreeConstraints(const FeatureVisitor v,
		const dd_edge emptyNode, dd_edge &startingNode,
		xml_node<> *constraintNode, forest *mdd) {
	// Add Cross Tree Constraints
	ConstraintVisitor cVisitor(v, emptyNode, mdd);
	// Visit the sub-tree for constraints and create a set of edges for each of them
	cVisitor.visit(constraintNode);
	// Now, compute the intersection between startingNode and each of the constraint
	vector<dd_edge> constraintList = cVisitor.getConstraintMddList();
	for (dd_edge e : constraintList) {
		startingNode *= e;
		logcout(LOG_DEBUG) << "\tNew cardinality "
				<< startingNode.getCardinality() << endl;
	}
}
