#include<iostream>
using std::string;

namespace Messages
{
	class BaseMessage
	{
	public:
		string message;
		string getMessage();
		string createRandomString(int size);
		const string currentTime(bool MILISECOND);
		int get_rand_int(int low, int high);
		void getCharMessage(char* outStr);
	};

	class SDCDMessage:BaseMessage
	{
	/*
	Class that handles process messages that comes from the SCDC
	*/
	public:
		static int counter;
		int nseq;
		int type;
		string tag;
		float value;
		string ue;
		string mode;
		string time_stamp;

		string parserMessage();
		string createMessage();
		string getMessage();
		const string currentTime();
		string createRandomString(int size);
		SDCDMessage();
		void getCharMessage(char* outStr);

	};

	class PIMSMessage:BaseMessage
	{
	/*
	Class that handles process messages that comes from the PIMS
	*/
	public:
		static int counter;  // int SDCDMessage::counter = 0;
		int nseq;
		int type;
		int alarm_id;
		int degree;
		int prev;
		string time_stamp;

		string createMessage();
		string getMessage();
		const string currentTime();
		PIMSMessage(int alarm_type);
		void getCharMessage(char* outStr);

	};

}
