#include<iostream>
#include<string>
#include "Messages.h"

using namespace Messages;


string BaseMessage::getMessage() {
	return message;
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
	time = message.substr(34, 45);

	return string();

}

string SDCDMessage::createMessage()
{
	/*
	This function is responsible to create the message of SDCD. The message format is:
	| NNNNNN | N | AAAAAAAAAA | NNNNNN.NN | A | HH:MM:SS:MSS
	   NSEQ   Type    TAG        VALUE     MODE   TIMESTAMP

	TODO: 
	- create the parameters randomly
	- find a way of casting float and int to string in order to add them in the final message
	*/

	tag = string("012-34-567");
	value = 765.21;
	ue = string("a");
	mode = 'a';
	time = string("HH:MM:SS.MSS");
	string aux;
	aux = tag + "|" + ue + "|" + time;
	return aux;
}

string SDCDMessage::getMessage()
{
	/*
	Calling getMessage function from parent class
	*/
	return BaseMessage::getMessage();
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
	nseq = counter++;
	message = createMessage();
}

int SDCDMessage::counter = 0;

