#ifndef PARAVER_PARSER_H
#define PARAVER_PARSER_H
#include <string>
#include <iostream>
#include <map>
#include <regex>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

/**
 * Note: works only for one application
 */
class ParaverParser {

private:
	map<int, string> stateNames;
	map<int, string> eventNames;
	map<int, string> eventTypes;
	long traceDuration;
	int numberOfTasks;
	int numberofApplications;
	int numberOfNodes;

	string outputFile;

	map<string, vector<string> > resourceNames;

	vector<string> keyWords;
	vector<int> nbProcPerNode;

	vector<int> taskThread;
	vector<int> taskNode;

	void parseConf(string confFile);
	void parseTrace(string traceFile);
	void parseResource(string resourceFile);
	string trim(string aString);
	bool contains(vector<string> v, string element);
	void parseEventConf(ifstream * file, string line);
	void parseHeader(string headerLine);
	string parseEvent(string line);
	string parseState(string line);
	string parseLink(string line);
	string parseRecord(string line);

	string buildProducers();

public:
	ParaverParser(string traceFile, string confFile, string resourceFile, string outputFile);
};

#endif
