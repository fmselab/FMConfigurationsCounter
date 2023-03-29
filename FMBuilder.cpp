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

#define IGNORE_HIDDEN_MAIN true


int main(int argc, char **argv) {
	Util::IGNORE_HIDDEN = IGNORE_HIDDEN_MAIN;

	if (Util::getProductCountFromFile("examples/gplModel.xml") != 186)
		cerr << "Error for gplModel" << endl;

	return 0;
}
