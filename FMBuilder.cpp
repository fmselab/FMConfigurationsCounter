#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN false

int main(int argc, char **argv) {
	// Models missing:
	//				   LinuxModel
	//				   PPUModel
	//				   Waterloo

	Util::SORT_CONSTRAINTS = true;
	Util::getProductCountFromFile("examples/ppuModel.xml", IGNORE_HIDDEN_MAIN);
	return 0;
}
