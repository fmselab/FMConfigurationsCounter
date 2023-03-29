/*
 * FMBuilderTest.cpp
 *
 *  Created on: 29 mar 2023
 *      Author: bombarda
 */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "Util.hpp"

TEST_CASE("gplTinyModel without ignore", "gplTinyModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE (Util::getProductCountFromFile("examples/gplTinyModel.xml") == 6);
}

TEST_CASE("gplTinyModel with ignore", "gplTinyModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE (Util::getProductCountFromFile("examples/gplTinyModel.xml") == 6);
}

TEST_CASE("carModel with ignore", "carModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE (Util::getProductCountFromFile("examples/carModel.xml") == 7);
}

TEST_CASE("carModel without ignore", "carModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE (Util::getProductCountFromFile("examples/carModel.xml") == 7);
}

TEST_CASE("aplModel with ignore", "aplModel") {
	Util::IGNORE_HIDDEN = true;
	REQUIRE (Util::getProductCountFromFile("examples/aplModel.xml") == 159120);
}

TEST_CASE("aplModel without ignore", "aplModel") {
	Util::IGNORE_HIDDEN = false;
	REQUIRE (Util::getProductCountFromFile("examples/aplModel.xml") == 159184);
}
