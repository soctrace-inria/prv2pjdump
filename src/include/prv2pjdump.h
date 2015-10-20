#ifndef INCLUDE_PRV2PJDUMP_H_
#define INCLUDE_PRV2PJDUMP_H_
#include <iostream>
#include <getopt.h>
#include "paraverParser.h"

class Prv2Pjdump {
private:
	string inputFile = "";
	string confFile = "";
	string outputFile = "";
	string resourceFile = "";

public:
	void launch(int argc, char **argv);
	int handleFilenames();
};

void printHelp();
bool exist(string filename);

#endif /* INCLUDE_PRV2PJDUMP_H_ */
