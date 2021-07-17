#include <iostream>
using std::string;

class Message {
	string message;

public:
	Message(string initialMessage, bool messageType) {
		message = initialMessage;
	}
	string getMessage() {
		return message;
	}
};