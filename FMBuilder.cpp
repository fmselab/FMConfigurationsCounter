#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Not Working Models:
	//				   LinuxModel
	//				   automotive01

	Util::SORT_CONSTRAINTS = true;
	Util::getProductCountFromFile("examples/gplAheadModel.xml", IGNORE_HIDDEN_MAIN, 0);
	return 0;
}
