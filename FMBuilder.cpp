#include <iostream>
#include "Util.hpp"

using namespace std;

#define IGNORE_HIDDEN_MAIN false

int main(int argc, char **argv) {
	Util::getProductCountFromFile("examples/violetModel.xml", IGNORE_HIDDEN_MAIN);
	return 0;
}
