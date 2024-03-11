#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

/**
 * Main function, used during testing.
 *
 * We should write a parameter-based main, in order to chose the configuration
 * parameters and the model to be analyzed from the command line.
 */
int main(int argc, char **argv) {
	// Not Working Models:
	//				   	LinuxModel
	//				   	automotive01
	// 					automotive02v4Model
	//					BusyBox

	Util::SORT_CONSTRAINTS_WHEN_APPLYING = true;
	Util::SHUFFLE_CONSTRAINTS = false;
	FeatureVisitor::COMPRESS_AND_THRESHOLD=20;
	FeatureVisitor::COMPRESS_AND_VARS=true;
	if (argc > 1) {
		if (argc == 2) {
			Util::getProductCountFromFile(argv[1], IGNORE_HIDDEN_MAIN, 1);
		} else {
			Util::getProductCountFromFile(argv[1], IGNORE_HIDDEN_MAIN, atoi(argv[2]));
		}
	}
	return 0;
}
