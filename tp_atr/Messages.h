#include<iostream>
using std::string;

namespace Messages
{
	class BaseMessage
	{
	public:
		string message;
		string getMessage();
	};

	class SDCDMessage:BaseMessage
	{
	/*
	Class that handles process messages that comes from the SCDC
	*/
	public:
		static int counter;  // int SDCDMessage::counter = 0;
		int nseq;
		int type;
		string tag;
		float value;
		string ue;
		string mode;
		string time;
		string parserMessage();
		string createMessage();
		string getMessage();
		SDCDMessage();

	};

}
