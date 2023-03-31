#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Models missing:
	//				   LinuxModel
	//				   Waterloo

	Util::SORT_CONSTRAINTS = true;
	Util::getProductCountFromFile("examples/linuxModel.xml", IGNORE_HIDDEN_MAIN, 20);
	return 0;
}
