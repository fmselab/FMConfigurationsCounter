#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Models missing: GPLAheadModel
	//				   LinuxModel
	//				   PPUModel
	//				   Waterloo

	Util::getProductCountFromFile("examples/linuxModel.xml", IGNORE_HIDDEN_MAIN);
	return 0;
}
