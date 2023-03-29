#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN false

int main(int argc, char **argv) {
	if (Util::getProductCountFromFile("examples/violetModel.xml", IGNORE_HIDDEN_MAIN) != 186)
		cerr << "Error for violetModel" << endl;

	return 0;
}
