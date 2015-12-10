/*
 * Event.cpp
 *
 *  Created on: 4 déc. 2015
 *      Author: youenn
 */

#include "include/event.h"

Event::Event() {

}

Event::Event(long long timestamp, string container, string name) {
	this->timeStamp = timestamp;
	this->container = container;
	this->name = name;

	tag = EVENT_TAG;
}

Event::~Event() {
	// TODO Auto-generated destructor stub
}

/**
 * Convert an event to the corresponding pjdump line
 */
string Event::toPjdump() {
	stringstream pjDumpLine;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << container << PJDUMP_SEPARATOR;
	pjDumpLine << name << PJDUMP_SEPARATOR;
	pjDumpLine << timeStamp << PJDUMP_SEPARATOR;
	pjDumpLine << name << endl;

	return pjDumpLine.str();
}

State::State() {
	tag = STATE_TAG;
}


State::State(long long timestamp, string container, string name, long long endtimeStamp) {
	this->timeStamp = timestamp;
	this->container = container;
	this->name = name;
	this->endDate = endtimeStamp;

	tag = STATE_TAG;
}

string State::toPjdump() {
	stringstream pjDumpLine;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << container << PJDUMP_SEPARATOR;
	pjDumpLine << name << PJDUMP_SEPARATOR;
	pjDumpLine << timeStamp << PJDUMP_SEPARATOR;
	pjDumpLine << endDate << PJDUMP_SEPARATOR;
	pjDumpLine << (endDate - timeStamp) << PJDUMP_SEPARATOR;
	pjDumpLine << imbrication << PJDUMP_SEPARATOR;
	pjDumpLine << name << endl;

	return pjDumpLine.str();
}

Link::Link(long long timestamp, string container, string name,
		long long rcvTimestamp, string rcvContainer) {
	this->timeStamp = timestamp;
	this->container = container;
	this->name = name;
	this->receiveTimestamp = rcvTimestamp;
	this->receiverContainer = rcvContainer;

	tag = LINK_TAG;
}

string Link::toPjdump() {
	stringstream pjDumpLine;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << container << PJDUMP_SEPARATOR;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << timeStamp << PJDUMP_SEPARATOR;
	pjDumpLine << receiveTimestamp << PJDUMP_SEPARATOR;
	pjDumpLine << (receiveTimestamp - timeStamp) << PJDUMP_SEPARATOR;
	pjDumpLine << tag << PJDUMP_SEPARATOR;
	pjDumpLine << container << PJDUMP_SEPARATOR;
	pjDumpLine << receiverContainer << endl;

	return pjDumpLine.str();
}
