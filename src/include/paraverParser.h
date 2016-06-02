#ifndef PARAVER_PARSER_H
#define PARAVER_PARSER_H
#include <map>
#include <regex>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include "common.h"
#include "event.h"

using namespace std;

/**
 * Note: works only for one application
 */
class ParaverParser {

private:
	// Match the names of the states with their IDs
	map<int, string> stateNames;
	// Match the names of the events with their IDs
	map<int, string> eventNames;
	// Match the names of the event value with their IDs
	map<int, map<int, string>> eventTypes;

	long long traceDuration = 0;
	int numberOfTasks = 0;
	int numberofApplications = 0;
	int numberOfNodes = 0;

	// Store the created containers
	set<string> createdContainers;

	// Store the last state created
	State * lastState;

	// Path to the output file
	string outputFile;

	// The output file
	ofstream pjdumpFile;

	// Match the name of the resources from the row file
	// The first key is the level (CPU, NODE, THREAD, etc.)
	map<string, vector<string>> resourceNames;

	// Contains the names of the leave producer in the hierarchy (typically the threads)
	map<int, map<int, map<int, string>>> threadProducers;

	// Store keywords of the .cfg configuration file
	set<string> keyWords;
	// Number of processors per node
	vector<int> nbProcPerNode;

	// Number of threads in a task (tasks are stored in the same order as the
	vector<int> taskThread;
	// Node executing the task
	vector<int> taskNode;

	// Specify if we use the paraver events to build the pjdump states
	bool useEventForState = false;

	void parseConf(string confFile);
	void parseTrace(string traceFile);
	void parseResource(string resourceFile);
	string trim(string aString);
	bool contains(set<string> v, string element);
	void parseEventConf(ifstream * file, string line);
	void parseHeader(string headerLine);
	string parseEvent(string line);
	string parseState(string line);
	string parseLink(string line);
	string parseRecord(string line);
	string getStateName(int type);
	string getEventName(int type);
	string getContainerName(int cpuID, unsigned int taskID, int threadID);

	string buildProducers();
	void buildContainer(int cpuID, int taskID, int threadID, string parent);
	void buildContainer(string name, string parentName);

public:
	ParaverParser(bool eventForState);
	void parse(string traceFile, string confFile, string resourceFile, string outputFile);

	// Default values for the states as described in www.bsc.es/media/1370.pdf
	static map<int, string> create_map() {
		map<int, string> m;
		m[0] = "Idle";
		m[1] = "Running";
		m[2] = "Not Created";
		m[3] = "Waiting a message";
		m[4] = "Blocked";
		m[5] = "Thread synchronize";
		m[6] = "Wait / Wait all";
		m[7] = "Schedule and Fork/Join";
		m[8] = "Test/Probe";
		m[9] = "Blocking Send";
		m[10] = "Immediate Send";
		m[11] = "Immediate Receive";
		m[12] = "I/O";
		m[13] = "Global OP";
		m[14] = "Tracing Disable";
		return m;
	}

	//Store the default names for the states
	static const map<int, string> defaultStateValues;

};

#endif
