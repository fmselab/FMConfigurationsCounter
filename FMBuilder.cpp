#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN true

int main(int argc, char **argv) {
	if (Util::getProductCountFromFile("examples/gplModel.xml", IGNORE_HIDDEN_MAIN) != 186)
		cerr << "Error for gplModel" << endl;

	return 0;
}
