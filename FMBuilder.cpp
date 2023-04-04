#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Not Working Models:
	//				   	LinuxModel
	//				   	automotive01
	// 					automotive02v4
	//					BusyBox

	Util::SORT_CONSTRAINTS_WHEN_APPLYING = true;
	Util::SHUFFLE_CONSTRAINTS = true;
	Util::getProductCountFromFile("examples/LinuxModel.xml", IGNORE_HIDDEN_MAIN, 10);
	return 0;
}
