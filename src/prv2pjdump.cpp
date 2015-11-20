#include "include/prv2pjdump.h"

using namespace std;

const int MINIMUM_INPUT_SIZE = 2;
const string PARAVER_FILE_EXTENSION = ".prv";
const string PARAVER_CONF_FILE_EXTENSION = ".pcf";
const string PARAVER_RESOURCE_FILE_EXTENSION = ".row";
const string PJDUMP_FILE_EXTENSION = ".pjdump";

/**
 * prv2pjdump is a utility program converting a paraver trace to pjdump format.
 */
int main(int argc, char **argv) {
	Prv2Pjdump prv2pjd = Prv2Pjdump();
	prv2pjd.launch(argc, argv);
}

void Prv2Pjdump::launch(int argc, char **argv) {

	if (argc < MINIMUM_INPUT_SIZE) {
		cout
				<< "Error: not enough arguments provided. You must provide at least one prv file."
				<< endl << endl;
		printHelp();
		return;
	}

	int opt;
	int option_index = 0;
	static struct option long_options[] = { { "output-file", required_argument,
			0, 'o' }, { 0, 0, 0, 0 } };

	// Check the provided options
	while ((opt = getopt_long(argc, argv, "o:", long_options, &option_index))
			!= -1) {
		switch (opt) {
		case 'o':
			outputFile = optarg;
			break;
		default: // '?'
			printHelp();
			return;
		}
	}

	if (optind < argc) {
		inputFile = argv[optind];
	} else {
		cout
				<< "Error: not enough arguments provided. You must provide at least one prv file."
				<< endl << endl;
		printHelp();
		return;
	}

	if (handleFilenames() < 0) {
		printHelp();
		return;
	}

	ParaverParser * parser = new ParaverParser();
	parser->parse(inputFile, confFile, resourceFile, outputFile);

	cout << "End of conversion" << endl;

	return;
}

int Prv2Pjdump::handleFilenames() {

	if (!exist(inputFile)) {
		cout << "Error: cannot open input file: " << inputFile << endl;
		return -1;
	}

	string basename = inputFile.substr(0, inputFile.find(".prv"));

	string potentialConfFile = basename + PARAVER_CONF_FILE_EXTENSION;
	if (exist(potentialConfFile)) {
		confFile = potentialConfFile;
	} else {
		cout
				<< "Warning: no configuration file could be found ("
						+ potentialConfFile + ")." << endl
				<< "Parsing will be done with default values." << endl;
	}

	string potentialResFile = basename + PARAVER_RESOURCE_FILE_EXTENSION;
	if (exist(potentialResFile)) {
		resourceFile = potentialResFile;
	} else {
		cout
				<< "Warning: no resource file could be found ("
						+ potentialResFile + ")." << endl
				<< "Parsing will be done with default values." << endl;
	}

	if (outputFile.empty()) {
		// Generate default name
		outputFile = basename + PJDUMP_FILE_EXTENSION;

		// Check if there is already an existing file with this name
		// and if so find the first name corresponding to a non-existing file
		int i = 1;

		while (exist(outputFile)) {
			stringstream outputss;
			outputss << basename << "_" << i << PJDUMP_FILE_EXTENSION;
			outputFile = outputss.str();
			i++;
		}

		cout << "Converted trace will be written in " << outputFile << endl;
	}

	return 0;
}

void printHelp() {
	cout << "Usage: prv2pjdump [OPTION] FILE" << endl;
	cout << "Convert the paraver FILE into pjdump." << endl << endl;
	cout << "\t -o, --output-file \tSpecify an output file." << endl;
}

/**
 * Check if a file exists
 */
bool exist(string filename) {
	ifstream ifile(filename);
	return ifile.is_open();
}
