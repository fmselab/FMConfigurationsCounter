#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Models missing: GPLAheadModel
	//				   LinuxModel
	//				   PPUModel
	//				   Waterloo

	Util::SORT_CONSTRAINTS = true;
	Util::getProductCountFromFile("examples/gplAheadModel.xml", IGNORE_HIDDEN_MAIN);
	return 0;
}
