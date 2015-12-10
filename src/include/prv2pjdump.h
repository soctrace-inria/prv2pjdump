#ifndef INCLUDE_PRV2PJDUMP_H_
#define INCLUDE_PRV2PJDUMP_H_
#include <getopt.h>
#include "common.h"
#include "paraverParser.h"

class Prv2Pjdump {
private:
	string inputFile = "";
	string confFile = "";
	string outputFile = "";
	string resourceFile = "";
	bool useEventForState = false;

public:
	void launch(int argc, char **argv);
	int handleFilenames();
};

void printHelp();
bool exist(string filename);

#endif /* INCLUDE_PRV2PJDUMP_H_ */
