#include "include/paraverParser.h"

const string STATE_CFG = "STATES";
const string EVENT_CFG = "EVENT_TYPE";
const string VALUE_CFG = "VALUES";
const string TYPE_CFG = "TYPE";

const string RESOURCE_LEVEL = "LEVEL";

const char PRV_SEPARATOR = ':';
const char PJDUMP_SEPARATOR = ',';

const int STATE_CATEGORY = 1;
const int EVENT_CATEGORY = 2;
const int LINK_CATEGORY = 3;

const string UNKNOWN_STATE_TYPE = "UNKNOWN_TYPE";

const map<int, string> ParaverParser::defaultStateValues = ParaverParser::create_map();

ParaverParser::ParaverParser() {
	keyWords.push_back(STATE_CFG);
	keyWords.push_back(EVENT_CFG);
	keyWords.push_back(VALUE_CFG);
	keyWords.push_back(TYPE_CFG);
}

void ParaverParser::parse(string traceFile, string confFile,
		string resourceFile, string outputFile) {

	this->outputFile = outputFile;

	if (!confFile.empty())
		parseConf(confFile);

	if (!resourceFile.empty())
		parseResource(resourceFile);

	parseTrace(traceFile);
}

/**
 * Parsing the configuration file (.pcf) in order to get the name of the events and of the states
 */
void ParaverParser::parseConf(string confFile) {
	ifstream file;
	file.open(confFile.c_str());
	string line;

	while (getline(file, line)) {
		if (line.empty())
			continue;

		// If the line describes a state
		if (line.compare(STATE_CFG) == 0) {
			while (getline(file, line) && !contains(keyWords, line)
					&& !line.empty()) {

				string id = line.substr(0, line.find(" "));
				int stateId = stoi(id, nullptr);
				line.erase(0, line.find(" ") + 1);

				string stateName = trim(line);

				stateNames.insert(make_pair(stateId, stateName));
			}
		}

		// If the line describes an event
		if (line.compare(EVENT_CFG) == 0) {
			parseEventConf(&file, line);
		}
	}
	file.close();
}

void ParaverParser::parseEventConf(ifstream *file, string line) {
	bool valuePresent = false;

	while (getline(*file, line)
			&& (line.compare(EVENT_CFG) == 0 || line.compare(VALUE_CFG) == 0
					|| (!contains(keyWords, line) && !line.empty()))) {
		if (line.compare(VALUE_CFG) == 0) {
			valuePresent = true;
			break;
		}
		if (line.compare(EVENT_CFG) == 0) {
			continue;
		}

		// Remove color gradient ID
		line.erase(0, line.find(" "));
		line = trim(line);

		// Get id
		string id = line.substr(0, line.find(" "));
		int eventId = stoi(id, nullptr);
		line.erase(0, line.find(" ") + 1);

		// Get name
		string eventName = trim(line);

		eventNames.insert(make_pair(eventId, eventName));
	}

	// handle VALUES
	if (valuePresent) {
		while (getline(*file, line) && !contains(keyWords, line)
				&& !line.empty()) {

			string id = line.substr(0, line.find(" "));
			int eventId = stoi(id, nullptr);
			line.erase(0, line.find(" ") + 1);

			string eventType = trim(line);

			eventTypes.insert(make_pair(eventId, eventType));
		}
	}

	// If we left VALUE for another EVENT_TYPE
	if (line.compare(EVENT_CFG) == 0)
		parseEventConf(file, line);
}

// trim from start
static inline string &ltrim(string &s) {
	s.erase(s.begin(),
			find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
static inline string &rtrim(string &s) {
	s.erase(
			find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(),
			s.end());
	return s;
}

/**
 * Remove white spaces at the beginning and at the end of a string
 */
string ParaverParser::trim(string aString) {
	return ltrim(rtrim(aString));
}

/**
 * Check if an vector contains a string
 */
bool ParaverParser::contains(vector<string> v, string element) {
	return (find(v.begin(), v.end(), element) != v.end());
}

/**
 * Parse the resource file (.row) to get the names of the producers
 */
void ParaverParser::parseResource(string resourceFile) {
	ifstream file;
	file.open(resourceFile.c_str());
	string line;

	while (getline(file, line)) {
		// If the line describes a state
		if (line.compare(0, 5, RESOURCE_LEVEL) == 0) {

			line.erase(0, line.find(" ") + 1);
			line = trim(line);
			string resourceType = line.substr(0, line.find(" "));

			resourceNames.insert(make_pair(resourceType, vector<string>()));

			while (getline(file, line) && !line.empty()) {
				resourceNames.at(resourceType).push_back(line);
			}
		}
	}
}

/**
 * Parse the .prv file
 */
void ParaverParser::parseTrace(string traceFile) {
	ifstream file;
	file.open(traceFile.c_str());
	string line;

	// Get the first non empty line
	while (getline(file, line) && line.empty()) {
	}

	// Handle the header
	parseHeader(line);

	// Open the output file
	pjdumpFile.open(outputFile);

	if (!pjdumpFile.good()) {
		cout << "Error: could not open output file " << outputFile << endl;
		return;
	}

	// process the producers
	pjdumpFile << buildProducers();

	// handle the trace records
	while (getline(file, line)) {
		string pjdumpLine = parseRecord(line);

		if (!pjdumpLine.empty())
			pjdumpFile << pjdumpLine;
	}

	pjdumpFile.close();
}

/**
 * Parse the header of a .prv file
 */
void ParaverParser::parseHeader(string headerLine) {
	// Skip date metadata
	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);
	// Another one because of the ":" in the hour
	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);

	// Get duration
	string durationStr = headerLine.substr(0, headerLine.find(PRV_SEPARATOR));
	if (durationStr.find_first_of("_") == string::npos)
		traceDuration = stol(durationStr);
	else {
		durationStr = durationStr.substr(0, headerLine.find("_"));
		traceDuration = stol(durationStr);
	}

	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);

	// Handle number of nodes and processor per node
	string nodeStr = headerLine.substr(0, headerLine.find(PRV_SEPARATOR));

	numberOfNodes = stoi(nodeStr.substr(0, nodeStr.find("(")));
	nodeStr.erase(0, nodeStr.find("(") + 1);

	while (nodeStr.find(",") != string::npos) {
		nbProcPerNode.push_back(stoi(nodeStr.substr(0, nodeStr.find(","))));
		nodeStr.erase(0, nodeStr.find(",") + 1);
	}
	nbProcPerNode.push_back(stoi(nodeStr.substr(0, nodeStr.find(")"))));
	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);

	// Number of applications
	numberofApplications = stoi(
			headerLine.substr(0, headerLine.find(PRV_SEPARATOR)));
	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);

	//Parse task
	string taskStr = headerLine.substr(0, headerLine.find(")"));
	numberOfTasks = stoi(taskStr.substr(0, taskStr.find("(")));
	taskStr.erase(0, taskStr.find("(") + 1);

	while (taskStr.find(",") != string::npos) {
		//Config for a task
		string taskConfig = taskStr.substr(0, taskStr.find(","));
		// Number of thread in the task
		taskThread.push_back(
				stoi(taskConfig.substr(0, taskConfig.find(PRV_SEPARATOR))));

		// Node executing the task
		taskNode.push_back(
				stoi(
						taskConfig.substr(taskConfig.find(PRV_SEPARATOR) + 1,
								taskConfig.length())));

		taskStr.erase(0, taskStr.find(",") + 1);
	}

	// Handle the last task
	taskThread.push_back(stoi(taskStr.substr(0, taskStr.find(PRV_SEPARATOR))));
	taskNode.push_back(
			stoi(
					taskStr.substr(taskStr.find(PRV_SEPARATOR) + 1,
							taskStr.length())));
}

/**
 * Parse one line of the .prv file
 */
string ParaverParser::parseRecord(string line) {

	string pjdumpLine = "";

	if (line.empty())
		return pjdumpLine;

	string token = line.substr(0, line.find(PRV_SEPARATOR));

	if (!isdigit(token.at(0)))
		return pjdumpLine;

	int category = stoi(token);

	switch (category) {
	case STATE_CATEGORY:
		pjdumpLine = parseState(line);
		break;

	case EVENT_CATEGORY:
		pjdumpLine = parseEvent(line);
		break;

	case LINK_CATEGORY:
		pjdumpLine = parseLink(line);
		break;

	default:
		cerr << "Unsupported type of event: " + line << endl;
	}

	return pjdumpLine;
}

/**
 * Parse a line representing a type state
 *
 * from:
 *	1:cpuID:appID:taskID:threadID:startTime:endTime:type
 *
 * to:
 *	State, container, state type, startTime, endTime, duration, imbricationLevel, value
 */
string ParaverParser::parseState(string line) {

	// remove token
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int cpuID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int appID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int taskID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int threadID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long startTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long endTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	stringstream pjDumpLine;
	pjDumpLine << "State, ";
	pjDumpLine << getContainerName(cpuID, taskID, threadID) << PJDUMP_SEPARATOR;
	pjDumpLine << getStateName(type) << PJDUMP_SEPARATOR;
	pjDumpLine << startTimestamp << PJDUMP_SEPARATOR;
	pjDumpLine << endTimestamp << PJDUMP_SEPARATOR;
	pjDumpLine << (endTimestamp - startTimestamp) << PJDUMP_SEPARATOR;
	pjDumpLine << "0,";
	pjDumpLine << getStateName(type) << endl;

	return pjDumpLine.str();
}

/**
 * Parse a line representing a punctual event
 *
 * from
 * 	2:cpuID:appID:taskID:threadID:startTime:type:value
 *
 * to
 * 	Event, containerName, eventName, timeStamp, eventValue
 */
string ParaverParser::parseEvent(string line) {

	// remove token
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int cpuID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int appID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int taskID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int threadID = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long timestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	while (count(line.begin(), line.end(), PRV_SEPARATOR) > 1) {
		int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
		line.erase(0, line.find(PRV_SEPARATOR) + 1);

		long value = stol(line.substr(0, line.find(PRV_SEPARATOR)));
		line.erase(0, line.find(PRV_SEPARATOR) + 1);
	}

	int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long value = stol(line);

	stringstream pjDumpLine;
	pjDumpLine << "Event, ";
	pjDumpLine << getContainerName(cpuID, taskID, threadID) << PJDUMP_SEPARATOR;
	pjDumpLine << getEventName(type) << PJDUMP_SEPARATOR;
	pjDumpLine << timestamp << PJDUMP_SEPARATOR;
	pjDumpLine << getEventName(type) << endl;

	return pjDumpLine.str();
}

/**
 * Parse a line representing a type link
 *
 * from
 * 	3:objectSender:wantedSendTime:actualSendTime:objectReceiver:wantedReceiveTime:actualRcvTime:size:tag
 *
 * objectSender = cpuID:appId:TaskID:threadID
 * objectReceiver = cpuID:appId:TaskID:threadID
 *
 * to
 * 	Link, containerName, linkType, startTime, endTime, duration, linkValue, sendingContainerName, receivingContainerName
 */
string ParaverParser::parseLink(string line) {

	// remove token
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	// Sender fields
	int cpuIDSend = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int appIDSend = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int taskIDSend = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int threadIDSend = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long wantedSendTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long actualSendTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	//receiverFields
	int cpuIDReceive = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int appIDReceive = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int taskIDReceive = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int threadIDReceive = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long wantedRcvTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long actualRcvTimestamp = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	// Other fields
	// Size in byte
	long size = stol(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int tag = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	stringstream pjDumpLine;
	pjDumpLine << "Link, ";
	pjDumpLine << getContainerName(cpuIDSend, taskIDSend, threadIDSend)
			<< PJDUMP_SEPARATOR;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << actualSendTimestamp << PJDUMP_SEPARATOR;
	pjDumpLine << actualRcvTimestamp << PJDUMP_SEPARATOR;
	pjDumpLine << (actualRcvTimestamp - actualSendTimestamp)
			<< PJDUMP_SEPARATOR;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << getContainerName(cpuIDSend, taskIDSend, threadIDSend)
			<< PJDUMP_SEPARATOR;
	pjDumpLine << getContainerName(cpuIDReceive, taskIDReceive, threadIDReceive)  << endl;

	return pjDumpLine.str();
}

/**
 * Build the containers
 *
 * Container, nameOfParent, type, startTime, endTime, duration, name
 */
string ParaverParser::buildProducers() {
	stringstream producers;

	if(resourceNames.find("THREAD") == resourceNames.end())
		return "";

	for (unsigned int i = 0; i < resourceNames.at("THREAD").size(); i++) {
		producers << "Container, 0,";
		producers << resourceNames.at("THREAD").at(i) << PJDUMP_SEPARATOR;
		producers << 0 << PJDUMP_SEPARATOR;
		producers << traceDuration << PJDUMP_SEPARATOR;
		producers << traceDuration << PJDUMP_SEPARATOR;
		producers << resourceNames.at("THREAD").at(i) << endl;
	}

	return producers.str();
}

/**
 * Look if a name for the given type exists, if not, falls back on default values and if not return a default name with the type at the end
 */
string ParaverParser::getStateName(int type) {
	if (stateNames.find(type) != stateNames.end())
		return stateNames.at(type);

	if (defaultStateValues.find(type) != defaultStateValues.end())
		return defaultStateValues.at(type);

	stringstream ss;
	ss << UNKNOWN_STATE_TYPE << "_" << type;

	return ss.str();
}

string ParaverParser::getEventName(int type) {
	if (eventNames.find(type) != eventNames.end())
		return eventNames.at(type);

	stringstream ss;
	ss << type;

	return ss.str();
}


string ParaverParser::getContainerName(int cpuID, int taskID, int threadID) {
	if (resourceNames.find("THREAD") != resourceNames.end())
		if (resourceNames.at("THREAD").size() > taskID)
			return resourceNames.at("THREAD").at(taskID - 1);

	stringstream ss;
	ss << "THREAD " << cpuID << "." << taskID << "." << threadID;

	if (find(createdContainers.begin(), createdContainers.end(), ss.str()) != createdContainers.end())
		return ss.str();

	buildContainer(cpuID, taskID, threadID);

	return ss.str();
}

/**
 * Create a container and add it to the pjdump file
 */
void ParaverParser::buildContainer(int cpuID, int taskID, int threadID) {

	stringstream container;
	stringstream containerName;
	containerName << "THREAD " << cpuID << "." << taskID << "." << threadID;

	container << "Container, 0,";
	container << containerName.str() << PJDUMP_SEPARATOR;
	container << 0 << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << containerName.str() << endl;

	pjdumpFile << container.str();

	createdContainers.push_back(containerName.str());
}
