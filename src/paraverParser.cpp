#include "include/paraverParser.h"

const string STATE_CFG = "STATES";
const string EVENT_CFG = "EVENT_TYPE";
const string VALUE_CFG = "VALUES";
const string TYPE_CFG = "TYPE";

const string RESOURCE_LEVEL = "LEVEL";
const string RESOURCE_LEVEL_CPU = "CPU";
const string RESOURCE_LEVEL_THREAD = "THREAD";
const string RESOURCE_LEVEL_NODE = "NODE";
const string RESOURCE_LEVEL_APPLICATION = "APPL";
const string RESOURCE_LEVEL_SYSTEM = "SYSTEM";
const string RESOURCE_LEVEL_WORKLOAD = "WORKLOAD";

const int STATE_CATEGORY = 1;
const int EVENT_CATEGORY = 2;
const int LINK_CATEGORY = 3;

const string UNKNOWN_STATE_TYPE = "UNKNOWN_TYPE";

const string TASK_CONTAINER_PREFIX = "Task";
const string APPLICATION_CONTAINER_PREFIX = "Application";
const string NODE_CONTAINER_PREFIX = "Node";

const map<int, string> ParaverParser::defaultStateValues = ParaverParser::create_map();

ParaverParser::ParaverParser(bool eventForState) {
	keyWords.insert(STATE_CFG);
	keyWords.insert(EVENT_CFG);
	keyWords.insert(VALUE_CFG);
	keyWords.insert(TYPE_CFG);
	lastState = nullptr;

	this->useEventForState = eventForState;
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
	int eventId = -1;

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
		eventId = stoi(id, nullptr);
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
			int eventValueId = stoi(id, nullptr);
			line.erase(0, line.find(" ") + 1);

			string eventType = trim(line);

			if(eventTypes.find(eventId) == eventTypes.end())
				eventTypes[eventId] = map<int, string>();

			eventTypes[eventId][eventValueId] = eventType;
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
 * Check if a vector contains a string
 */
bool ParaverParser::contains(set<string> s, string element) {
	return (s.find(element) != s.end());
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

	// Open the output file
	pjdumpFile.open(outputFile);

	if (!pjdumpFile.good()) {
		cout << "Error: could not open output file " << outputFile << endl;
		return;
	}

	// Get the first non empty line
	while (getline(file, line) && line.empty()) {
	}

	// Handle the header
	parseHeader(line);

	// process the producers
	//pjdumpFile << buildProducers();

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

	string date = headerLine.substr(0, headerLine.find(PRV_SEPARATOR));
	// Skip date metadata
	headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);
	if (date.find("(") != string::npos && date.find(")") == string::npos) {
		// Another one because of the ":" in the hour
		headerLine.erase(0, headerLine.find(PRV_SEPARATOR) + 1);
	}

	// Get duration
	string durationStr = headerLine.substr(0, headerLine.find(PRV_SEPARATOR));
	if (durationStr.find_first_of("_") == string::npos)
		traceDuration = stoll(durationStr);
	else {
		durationStr = durationStr.substr(0, headerLine.find("_"));
		traceDuration = stoll(durationStr);
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

	// Parse tasks and threads for each application
	for (int appNumber = 0; appNumber < numberofApplications; appNumber++) {
		string appName = APPLICATION_CONTAINER_PREFIX + "_" + to_string(appNumber);
		buildContainer(appName, "0");
		int taskCount = 1;

		//Parse task
		string taskStr = headerLine.substr(0, headerLine.find(")"));
		numberOfTasks = stoi(taskStr.substr(0, taskStr.find("(")));
		taskStr.erase(0, taskStr.find("(") + 1);

		while (taskStr.find(",") != string::npos) {
			// Config for a task
			string taskConfig = taskStr.substr(0, taskStr.find(","));
			// Number of thread in the task
			taskThread.push_back(
					stoi(taskConfig.substr(0, taskConfig.find(PRV_SEPARATOR))));

			unsigned int nodeNumber = stoi(
					taskConfig.substr(taskConfig.find(PRV_SEPARATOR) + 1,
							taskConfig.length()));

			string nodeName = NODE_CONTAINER_PREFIX + "_" + to_string(nodeNumber);
			// Check for a substitute name
			if (resourceNames.at(RESOURCE_LEVEL_NODE).size() >= nodeNumber) {
				nodeName = resourceNames.at(RESOURCE_LEVEL_NODE).at(nodeNumber - 1);
			}

			buildContainer(nodeName, appName);

			// Node executing the task
			taskNode.push_back(nodeNumber);

			string taskName = TASK_CONTAINER_PREFIX + "_" + to_string(taskCount);
			buildContainer(taskName, nodeName);
			taskCount++;

			taskStr.erase(0, taskStr.find(",") + 1);
		}

		unsigned int nodeNumber = stoi(
				taskStr.substr(taskStr.find(PRV_SEPARATOR) + 1,
						taskStr.length()));
		string nodeName = NODE_CONTAINER_PREFIX + "_" + to_string(nodeNumber);
		// Check for a substitute name
		if (resourceNames.at(RESOURCE_LEVEL_NODE).size() >= nodeNumber) {
			nodeName = resourceNames.at(RESOURCE_LEVEL_NODE).at(nodeNumber - 1);
		}

		// Handle the last task
		taskThread.push_back(
				stoi(taskStr.substr(0, taskStr.find(PRV_SEPARATOR))));
		taskNode.push_back(
				stoi(
						taskStr.substr(taskStr.find(PRV_SEPARATOR) + 1,
								taskStr.length())));

		string taskName = TASK_CONTAINER_PREFIX + "_" + to_string(taskCount);
		buildContainer(taskName, nodeName);
	}
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

	long long startTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long long endTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	if(lastState != nullptr)
		free(lastState);

	lastState = new State(startTimestamp,
			getContainerName(appID, taskID, threadID), getStateName(type),
			endTimestamp);

	return lastState->toPjdump();
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

	long long timestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	map<int, long long> params = map<int, long long>();
	int firstType = -1;
	int firstValue = -1;

	while (count(line.begin(), line.end(), PRV_SEPARATOR) > 1) {
		int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
		line.erase(0, line.find(PRV_SEPARATOR) + 1);

		long long value = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
		line.erase(0, line.find(PRV_SEPARATOR) + 1);

		params[type] = value;

		if (firstType == -1)
			firstType = type;
		if (firstValue == -1)
			firstValue = value;
	}

	int type = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long long value = stoll(line);
	params[type] = value;

	if (firstType == -1)
		firstType = type;
	if (firstValue == -1)
		firstValue = value;

	if (useEventForState && lastState != nullptr) {
		if (lastState->getContainer()
				== getContainerName(appID, taskID, threadID)
				&& lastState->getTimeStamp() <= timestamp
				&& lastState->getEndDate() >= timestamp) {

			// Use the first param as it seems to be the most significant
			// Add a prefix in order to avoid having event and state with the same name
			string stateType = getEventName(firstType) + "_state";

			// Is there an alternative name defined in the .pcf file
			if(eventTypes.find(firstType) != eventTypes.end()){
				stateType =  eventTypes.at(firstType).at(firstValue);
			}

			State eventState = State(timestamp, lastState->getContainer(),
					stateType, lastState->getEndDate());
			eventState.setImbrication(1);

			free(lastState);
			lastState = nullptr;

			return eventState.toPjdump();
		}
	}

	Event event = Event(timestamp, getContainerName(appID, taskID, threadID),
			getEventName(type));

	return event.toPjdump();
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

	long long wantedSendTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long long actualSendTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
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

	long long wantedRcvTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	long long actualRcvTimestamp = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	// Other fields
	// Size in byte
	long long size = stoll(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	int tag = stoi(line.substr(0, line.find(PRV_SEPARATOR)));
	line.erase(0, line.find(PRV_SEPARATOR) + 1);

	Link link = Link(actualSendTimestamp,
			getContainerName(appIDSend, taskIDSend, threadIDSend), "",
			actualRcvTimestamp,
			getContainerName(appIDReceive, taskIDReceive, threadIDReceive));

	return link.toPjdump();
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


string ParaverParser::getContainerName(int appID, unsigned int taskID, int threadID) {
	if (threadProducers.find(appID) != threadProducers.end())
		if (threadProducers.at(appID).find(taskID)
				!= threadProducers.at(appID).end())
			if (threadProducers.at(appID).at(taskID).find(threadID)
					!= threadProducers.at(appID).at(taskID).end())
				return threadProducers.at(appID).at(taskID).at(threadID);

	string name = "THREAD " + to_string(appID) + "." + to_string(taskID) + "."
			+ to_string(threadID);

	if(contains(createdContainers, name))
		return name;

	string taskParentID = TASK_CONTAINER_PREFIX + "_" + to_string(taskID);
	if (contains(createdContainers, taskParentID)) {
		buildContainer(appID, taskID, threadID, taskParentID);
	} else {
		buildContainer(appID, taskID, threadID, "0");
	}

	return name;
}

void ParaverParser::buildContainer(int appID, int taskID, int threadID, string parent) {
	stringstream container;
	string name = "THREAD " + to_string(appID) + "." + to_string(taskID) + "."
			+ to_string(threadID);

	// Check if already existing
	if(contains(createdContainers, name))
		return;

	container << "Container" << PJDUMP_SEPARATOR;
	container << parent << PJDUMP_SEPARATOR;
	container << name << PJDUMP_SEPARATOR;
	container << 0 << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << name << endl;

	pjdumpFile << container.str();

	createdContainers.insert(name);

	if (threadProducers.find(appID) == threadProducers.end()) {
		threadProducers[appID] = map<int, map<int, string>>();
	}
	if (threadProducers.at(appID).find(taskID)
			== threadProducers.at(appID).end()) {
		threadProducers[appID][taskID] = map<int, string>();
	}

	threadProducers[appID][taskID][threadID] = name;
}

void ParaverParser::buildContainer(string name, string parentName) {
	// Check if already existing
	if(contains(createdContainers, name))
		return;

	stringstream container;

	container << "Container" << PJDUMP_SEPARATOR;
	container << parentName << PJDUMP_SEPARATOR;
	container << name << PJDUMP_SEPARATOR;
	container << 0 << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << traceDuration << PJDUMP_SEPARATOR;
	container << name << endl;

	pjdumpFile << container.str();

	createdContainers.insert(name);
}
