/*
 * event.h
 *
 *  Created on: 4 déc. 2015
 *      Author: youenn
 */

#ifndef INCLUDE_EVENT_H_
#define INCLUDE_EVENT_H_
#include "common.h"

const string EVENT_TAG = "Event";
const string STATE_TAG = "State";
const string LINK_TAG = "Link";

class Event {

protected:
	// Timestamp of the vent
	long long timeStamp = -1;
	// Container that generated the event
	string container = "";
	// Name of the event
	string name = "";
	// Value of the event
	string value = "";
	// Value of the pjdump tag
	string tag = "";

public:
	Event();
	Event(long long timestamp, string container, string name);
	virtual ~Event();
	virtual string toPjdump();

	// Getters and Setters
	const string& getContainer() const {
		return container;
	}

	void setContainer(const string& container) {
		this->container = container;
	}

	const string& getName() const {
		return name;
	}

	void setName(const string& name) {
		this->name = name;
	}

	const string& getTag() const {
		return tag;
	}

	void setTag(const string& tag) {
		this->tag = tag;
	}

	long long getTimeStamp() const {
		return timeStamp;
	}

	void setTimeStamp(long long timeStamp) {
		this->timeStamp = timeStamp;
	}

	const string& getValue() const {
		return value;
	}

	void setValue(const string& value) {
		this->value = value;
	}
};


class State : public Event {

private:
	// State end date
	long long endDate = -1;
	// Imbrication level of the state
	int imbrication = 0;

public:
	State();
	State(long long timestamp, string container, string name, long long endtimeStamp);
	string toPjdump();

	// Getters and Setters
	long long getEndDate() const {
		return endDate;
	}

	void setEndDate(long long endDate) {
		this->endDate = endDate;
	}

	int getImbrication() const {
		return imbrication;
	}

	void setImbrication(int imbrication = 0) {
		this->imbrication = imbrication;
	}
};

class Link: public Event {
private:
	long long receiveTimestamp;
	string receiverContainer;

public:
	Link(long long timestamp, string container, string name,
			long long endtimeStamp, string receiverContainer);
	string toPjdump();
};

#endif /* INCLUDE_EVENT_H_ */
