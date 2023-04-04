#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Not Working Models:
	//				   	LinuxModel
	//				   	automotive01
	// 					automotive02v4

	Util::SORT_CONSTRAINTS = false;
	Util::getProductCountFromFile("examples/automotive02v4Model.xml", IGNORE_HIDDEN_MAIN, 10);
	return 0;
}
