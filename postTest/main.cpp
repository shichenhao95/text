#include<iostream>
#include "httpConnect.h"
int main(int argc, char** argv) {
	HttpConnect *http = new HttpConnect;
	http->getData("127.0.0.1", "/login", "id=liukang&pw=123");
	http->postData("127.0.0.1", "/login", "id=liukang&pw=123");
	system("Pause");
	return 0;
}