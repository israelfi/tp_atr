#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#include "Messages.h"
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS

using namespace Messages;
using std::chrono::system_clock;


string BaseMessage::getMessage() {
	return message;
}

string BaseMessage::createRandomString(int size)
{
	std::string tmp_s;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	tmp_s.reserve(size);

	for (int i = 0; i < size; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}

const string BaseMessage::currentTime(bool MILISECOND)
{
	/*
	Returns current time in format HH:MM:SS.MSS or HH:MM:SS
	*/

	using namespace std::chrono;

	auto now = system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	auto timer = system_clock::to_time_t(now);

	std::tm bt = *std::localtime(&timer);
	std::ostringstream oss;

	oss << std::put_time(&bt, "%H:%M:%S");
	std::cout << oss.str() << std::endl;
	if (MILISECOND) {
		oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
	}

	return oss.str();
}

int BaseMessage::get_rand_int(int low, int high)
{
	/*
	Returns a random intenger between low and high (closed interval).
	*/
	int number;
	number = rand() % high + low;
	return number;
}

void BaseMessage::getCharMessage(char* outStr)
{
	int message_size = message.length();
	char arr[52];
	for (int i = 0; i < message_size; i++) {
		outStr[i] = message[i];
	}
	return;
}

string SDCDMessage::parserMessage()
{
	/*
	This will possibily be removed
	Message format:
	| NNNNNN | N | AAAAAAAAAA | NNNNNN.NN | A | HH:MM:SS:MSS
	   NSEQ   Type    TAG        VALUE     MODE   TIMESTAMP
	*/

	nseq = stoi(message.substr(0, 5));
	type = stoi(message.substr(6, 6));
	tag = message.substr(7, 16);
	value = stof(message.substr(17, 24));
	ue = message.substr(25, 32);
	mode = message.substr(33, 33);
	time_stamp = message.substr(34, 45);

	return string();

}

string SDCDMessage::createMessage()
{
	/*
	This function is responsible to create the message of SDCD. The message format is:
	| NNNNNN | N | AAAAAAAAAA | NNNNNN.NN | AAAAAAAA | A | HH:MM:SS.MSS
	   NSEQ   Type    TAG        VALUE         UE     MODE  TIMESTAMP
	*/
	int precisionVal = 2;
	int n, aux_random;
	int digits = 0;
	string aux;

	srand(static_cast <unsigned> (time(0)));

	// Getting NSEQ value and completing with zeros unitl it has 6 digits
	n = nseq;
	do {
		n /= 10;
		++digits;
	} while (n != 0);
	string nseq_string = string(6 - digits, '0').append(std::to_string(nseq));

	// TYPE is always 1
	type = 1;

	// Setting TAG value
	tag = createRandomString(10);

	// Generating random float numbers between 10000 and 99999
	value = BaseMessage::get_rand_int(1000000, 9999900) / 100.0;
	string value_string = std::to_string(value).substr(0, std::to_string(value).find(".") + precisionVal + 1);
	
	// Setting UE value
	aux_random = BaseMessage::get_rand_int(0,2);
	if (aux_random == 0) {
		ue = string("K       ");
	}
	else if (aux_random == 1) {
		ue = string("kgf/m2  ");
	}
	else {
		ue = string("kg/m3   ");
	}

	// Choosing MODE randomly: rand() % 2 returns random values, 0 or 1.
	if (rand() % 2) {
		mode = 'A';
	}
	else {
		mode = 'M';
	}

	// Getting the TIMESTAMP from the current time
	time_stamp = SDCDMessage::currentTime();

	aux = nseq_string + "|" + std::to_string(type) + "|" + tag + "|" + value_string + "|" + ue + "|"  + mode + "|" + time_stamp;
	return aux;
}

string SDCDMessage::getMessage()
{
	/*
	Calling getMessage function from parent class
	*/
	return BaseMessage::getMessage();
}

const string SDCDMessage::currentTime()
{	
	/*
	Returns current time in format HH:MM:SS.MSS in string type
	*/

	return BaseMessage::currentTime(true);
}

string SDCDMessage::createRandomString(int size)
{
	return BaseMessage::createRandomString(size);
}

SDCDMessage::SDCDMessage()
{
	/*
	Constructor function. It already atributes nseq values in a incremeting way as the new messages from 
	the same class are created. For example:
	 _______________________
	|	SDCDMessage m1, m2;	|
	|	-> m1.nseq is 0		|
	|	-> m2.nseq is 1		|
	 -----------------------

	This function also calls the createMessage method
	*/
	nseq = (counter++) % 1000000;
	message = SDCDMessage::createMessage();
}

void SDCDMessage::getCharMessage(char* outStr)
{
	BaseMessage::getCharMessage(outStr);
	return;
}

int SDCDMessage::counter = 0;

string PIMSMessage::createMessage() {
	/*
	This function is responsible to create the message of PIMS. The message format is:
	|  NNNNNN  |  N  |  NNNN  |  NN  |  NNNNN  |  HH:MM:SS
	    NSEQ    Type  Alarm ID DEGREE   PREV     TIMESTAMP
	*/

	int n, aux_random;
	int digits = 0;
	string aux;

	// Getting NSEQ value and completing with zeros unitl it has 6 digits
	n = nseq;
	do {
		n /= 10;
		++digits;
	} while (n != 0);
	string nseq_string = string(6 - digits, '0').append(std::to_string(nseq));

	// Type is set in constructor method

	// Alarm ID
	alarm_id = BaseMessage::get_rand_int(1, 9999);
	digits = 0;
	n = alarm_id;
	do {
		n /= 10;
		++digits;
	} while (n != 0);
	string alarm_id_string = string(4 - digits, '0').append(std::to_string(alarm_id));

	// Degree
	degree = BaseMessage::get_rand_int(1, 99);
	digits = 0;
	n = degree;
	do {
		n /= 10;
		++digits;
	} while (n != 0);
	string degree_string = string(2 - digits, '0').append(std::to_string(degree));

	// Prev
	prev = BaseMessage::get_rand_int(1, 14440);
	digits = 0;
	n = prev;
	do {
		n /= 10;
		++digits;
	} while (n != 0);
	string prev_string = string(5 - digits, '0').append(std::to_string(prev));

	// Timestamp
	time_stamp = currentTime();

	aux = nseq_string + "|" + std::to_string(type) + "|" + alarm_id_string + "|" + degree_string + "|" + prev_string + "|" + time_stamp;

	return aux;
}

string PIMSMessage::getMessage()
{
	return BaseMessage::getMessage();
}

const string PIMSMessage::currentTime()
{
	/*
	Returns current time in format HH:MM:SS in string type
	*/

	return BaseMessage::currentTime(false);
}

PIMSMessage::PIMSMessage(int alarm_type)
{
	/*
	Constructor function. It already atributes nseq values in a incremeting way as the new messages from
	the same class are created. For example:
	 _______________________
	|	PIMSMessage p1, p2;	|
	|	-> p1.nseq is 0		|
	|	-> p2.nseq is 1		|
	 -----------------------

	This function also calls the createMessage method
	*/
	nseq = (counter++) % 1000000;
	type = alarm_type;
	message = PIMSMessage::createMessage();
}

void PIMSMessage::getCharMessage(char* outStr)
{
	BaseMessage::getCharMessage(outStr);
	return;
}
int PIMSMessage::counter = 0;
