/*
 * FMBuilderTest.cpp
 *
 *  Created on: 29 mar 2023
 *      Author: Andrea Bombarda
 */
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <gmp.h>
#include "Util.hpp"

/**
 * This file contains a bunch of unit test cases aimed at checking the functionality
 * of the FMProductCounter and assuring non-regression.
 *
 * Each model is tested both ignoring hidden features and considering them.
 *
 * Each test case reports, when the number is too high, as a comment the number of valid
 * configuration expected and expressed in scientific notation.
 */
TEST_CASE("gplTinyModel without ignore", "[gplTinyModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplTinyModel.xml", false)
					== 6);
	FeatureVisitor::index = 0;
}

TEST_CASE("gplTinyModel with ignore", "[gplTinyModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplTinyModel.xml", true)
					== 6);
	FeatureVisitor::index = 0;
}

TEST_CASE("carModel with ignore", "[carModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(Util::getProductCountFromFile("examples/carModel.xml", true) == 7);
}

TEST_CASE("carModel without ignore", "[carModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(Util::getProductCountFromFile("examples/carModel.xml", false) == 7);
}

TEST_CASE("ppuModel with ignore", "[ppuModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/ppuModel.xml", true)
					== 96000);
}

TEST_CASE("ppuModel without ignore", "[ppuModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/ppuModel.xml", false)
					== 96000);
}

TEST_CASE("aplModel with ignore", "[aplModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/aplModel.xml", true)
					== 159120);
}

TEST_CASE("aplModel without ignore", "[aplModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/aplModel.xml", false)
					== 159184);
}

TEST_CASE("gplModel without ignore", "[gplModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplModel.xml", false)
					== 960);
}

TEST_CASE("gplModel with ignore", "[gplModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplModel.xml", true)
					== 186);
}

TEST_CASE("eshopModel with ignore", "[eshopModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 2.2602e+49
	double nConfig = 22602043046884907138622062009629005242624813039616.0;
	double computedCfgs = Util::getProductCountFromFile(
			"examples/eshopModel.xml", true);
	REQUIRE(computedCfgs == nConfig);
}

TEST_CASE("eshopModel without ignore", "[eshopModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 2.2602e+49
	double nConfig = 22602043046884907138622062009629005242624813039616.0;
	double computedCfgs = Util::getProductCountFromFile(
			"examples/eshopModel.xml", false);
	REQUIRE(computedCfgs == nConfig);
}

TEST_CASE("violetModel without ignore", "[violetModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 5.93843e+18
	double nConfig = 5938433009513149440.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/violetModel.xml", false)
					== nConfig);
}

TEST_CASE("violetModel with ignore", "[violetModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 3.89226e+18
	double nConfig = 3892263109263360000.0;
	double computedConfig = Util::getProductCountFromFile(
			"examples/violetModel.xml", true);
	REQUIRE(computedConfig == nConfig);
}

TEST_CASE("gplAheadModel with ignore", "[gplAheadModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	double nConfig = 728;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplAheadModel.xml", true)
					== nConfig);
}

TEST_CASE("gplAheadModel without ignore", "[gplAheadModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	double nConfig = 100266;
	REQUIRE(
			Util::getProductCountFromFile("examples/gplAheadModel.xml", false)
					== nConfig);
}

TEST_CASE("waterlooModel with ignore", "[waterlooModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 6.70835e+101
	double nConfig = 670835392217916052367826961285705439055529026755794918488321216695556598191228298976523484421022547968.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/waterlooModel.xml", true)
					== nConfig);
}

TEST_CASE("waterlooModel without ignore", "[waterlooModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 6.70835e+101
	double nConfig = 670835392217916052367826961285705439055529026755794918488321216695556598191228298976523484421022547968.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/waterlooModel.xml", false)
					== nConfig);
}

TEST_CASE("axTLSModel with ignore", "[axTLSModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	//8.26244e+11
	double nConfig = 826244333568.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/axTLSModel.xml", true)
					== nConfig);
}

TEST_CASE("axTLSModel without ignore", "[axTLSModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	//8.26244e+11
	double nConfig = 826244333568.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/axTLSModel.xml", false)
					== nConfig);
}

TEST_CASE("uClibCModel without ignore", "[uClibCModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 1.66019e+40
	double nConfig = 16601881363009995393029846406265571377152.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/uClibCModel.xml", false)
					== nConfig);
}

TEST_CASE("uClibCModel with ignore", "[uClibCModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 1.66019e+40
	double nConfig = 16601881363009995393029846406265571377152.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/uClibCModel.xml", true)
					== nConfig);
}

TEST_CASE("berkeleyDBModel1 without ignore", "[berkeleyDBModel1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 4.02597e+09
	double nConfig = 4025968128.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/berkeleyDBModel.xml", false)
					== nConfig);
}

TEST_CASE("berkeleyDBModel1 with ignore", "[berkeleyDBModel2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 4.02597e+09
	double nConfig = 4025968128.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/berkeleyDBModel.xml", true)
					== nConfig);
}

TEST_CASE("Embtoolkit without ignore", "[embToolkit1]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 5.13456e+96
	double nConfig = 5134555717728406502218134350650551212671101329005046085996988582713204660115755558759929862946816.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/Embtoolkit.xml", false, 15)
					== nConfig);
}

TEST_CASE("Embtoolkit with ignore", "[embToolkit2]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 5.13456e+96
	double nConfig = 5134555717728406502218134350650551212671101329005046085996988582713204660115755558759929862946816.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/Embtoolkit.xml", true, 15)
					== nConfig);
}

TEST_CASE("FinancialServices01 without ignore", "[FinancialServices011]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 9.74512e+13
	double nConfig = 97451212554676.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/FinancialServices01.xml", false, 0)
					== nConfig);
}

TEST_CASE("FinancialServices01 with ignore", "[FinancialServices012]") {
	Util::SORT_CONSTRAINTS_WHEN_APPLYING = false;
	// 9.74512e+13
	double nConfig = 97451212554676.0;
	REQUIRE(
			Util::getProductCountFromFile("examples/FinancialServices01.xml", true, 0)
					== nConfig);
}
