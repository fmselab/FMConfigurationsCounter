#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	// Not Working Models:
	//				   	LinuxModel
	//				   	automotive01
	// 					automotive02v4
	//					berkeleyDBModel

	Util::SORT_CONSTRAINTS = false;
	Util::getProductCountFromFile("examples/berkeleyDBModel.xml", IGNORE_HIDDEN_MAIN, 0);
	return 0;
}
