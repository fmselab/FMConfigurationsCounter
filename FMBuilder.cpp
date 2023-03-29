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
	if (Util::getProductCountFromFile("examples/gplModel.xml", IGNORE_HIDDEN_MAIN) != 186)
		cerr << "Error for gplModel" << endl;

	return 0;
}
