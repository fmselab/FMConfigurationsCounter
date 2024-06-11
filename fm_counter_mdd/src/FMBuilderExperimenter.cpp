#define HAVE_CONFIG_H
#include <iostream>
#include <fstream>
#include <filesystem>
#include <time.h>
#include <stdio.h>
#include "Util.hpp"
#include "boost/program_options.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace std;

#define IGNORE_HIDDEN_MAIN true
#define HELP "help"
#define AUTOVALIDATE "autovalidate"
#define SILENT "silent"
#define DONOTGENERATE "donotgenerate"

/**
 * Main function, used during testing.
 *
 * We should write a parameter-based main, in order to chose the configuration
 * parameters and the model to be analyzed from the command line.
 */
int main(int argc, char **argv) {
	po::options_description desc("Allowed options");
	desc.add_options()
					(HELP, "produce help message")
					("m", po::value<string>(), "set model file name")
					("r", po::value<int>(), "number of cross-tree constraints to merge [1]")
					("o", po::value<string>(), "set output file name")
					("dr", "dinamically reorder variables")
					("mergeAnd", "merge and groups")
					("nMergeAnd", po::value<int>(), "threshold for merging and groups [5]")
					;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	string path="";
	string outputPath="";
	string numProducts = "";
	int ctcToMerge = 0;
	ofstream outputFile;

	if (vm.count(HELP)) {
		cout << desc << "\n";
		return 0;
	}
	if (vm.count("m")) {
		path = vm["m"].as<string>();
	} else {
		cerr << "Please specify the model of interest" << endl;
		return -1;
	}
	if (vm.count("r")) {
		ctcToMerge = vm["r"].as<int>();
	} else {
		ctcToMerge = 1;
	}
	if (vm.count("o")) {
		outputPath = vm["o"].as<string>();
	} else {
		cerr << "Please specify the output path" << endl;
		return -1;
	}
	if (vm.count("mergeAnd")) {
		FeatureVisitor::COMPRESS_AND_VARS=true;
		if (vm.count("nMergeAnd")) {
			FeatureVisitor::COMPRESS_AND_THRESHOLD=vm["nMergeAnd"].as<int>();;
		} else {
			FeatureVisitor::COMPRESS_AND_THRESHOLD=5;
		}
	} else {
		FeatureVisitor::COMPRESS_AND_VARS=false;
		FeatureVisitor::COMPRESS_AND_THRESHOLD=0;
	}
	if (vm.count("dr")) {
		Util::REORDER_VARIABLES=true;
	} else {
		Util::REORDER_VARIABLES=false;
	}
	outputFile.open (outputPath, ios::out | ios::app);
	if (outputFile.is_open()) {
		double time1, timedif;
		time1 = (double) clock() / CLOCKS_PER_SEC;
		numProducts = Util::getProductCountFromFile(path, IGNORE_HIDDEN_MAIN, ctcToMerge);
		timedif = ( ((double) clock()) / CLOCKS_PER_SEC) - time1;
		outputFile << path << ";" << numProducts << ";" << timedif << ";" << ctcToMerge << ";" <<
				FeatureVisitor::COMPRESS_AND_VARS << ";" << FeatureVisitor::COMPRESS_AND_THRESHOLD << ";" <<
				Util::REORDER_VARIABLES << ";" << Util::N_MAX_EDGES << ";" << Util::N_MAX_NODES << "\n";
		outputFile.close();
	} else {
		cerr << "Error in locating output file" << endl;
	}
}
