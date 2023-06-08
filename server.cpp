#include <winsock2.h>
#include <string>
#include <fstream>
#include <sstream>
#include "md5.h"

using namespace std;

const char *PREFIX = "[SERVER]";

// Convert bytes to MB
#define MB 1048576
#define WIDTH 7

string ReadFile(string fileName){
	ifstream file(fileName);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
// Counter
int counter = atoi(ReadFile("visit_counter.txt").c_str());

// Store html files in memory
// string HTML_INDEX = ReadFile("public/index.html");
// string HTML_HELLO = ReadFile("public/hello.html");
// string HTML_404 = ReadFile("public/404.html");


// Ignore this, you did not see this.
class Server { public:
	// Checking if the request method matches the function else, respond with 501 Not Implemented
	int get(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("GET", type);
	}
	int post(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("POST", type);
	}
	int head(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("HEAD", type);
	}
	int _delete(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("DELETE", type);
	}
	int put(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("PUT", type);
	}
	int options(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("OPTIONS", type);
	}
	int patch(const char* path, char* desination, char* type) {
		return !strcmp(path, desination) && !strcmp("PATCH", type);
	}
};

string Routing(char* path, char* HTTP_REPONSE, char* method){
	string html, response, content_type;
	Server app;
	
	if (app.get("/memory", path, method)){
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof (statex);
		GlobalMemoryStatusEx (&statex);
		// Display memory to json
		html += "{\"used\": " + to_string(statex.ullAvailPhys / MB) + ", \"total\": " + to_string(statex.ullTotalPhys / MB) + "}";
		content_type = "application/json\r\n";
	}
	else if (app.post("/md5", path, method)){
		string data = HTTP_REPONSE;
		html += md5(data);
		content_type = "text/md5\r\n";
	}
	else if (app.get("/", path, method)){
		counter++;
		html += "<h1>C++ Server default page</h1>";
		html += "<a href='/hello'>Go to /hello</a>";
		html += "<h2>This page was requested " + to_string(counter) + " times</h2>";
		html += "<a href='/info'>Go to memory viewer</a><br>";

		html += "<h2>Generate MD5 hash on the server</h2>";
		html += "<input type='text' id='md5' value='abc'>";
		html += "<button id='button' onclick='exec()'>Calculate</button>";
		html += "<script>const exec = () => { let md5 = document.getElementById('md5'); fetch('/md5', { method: 'POST', body: md5.value }).then(function (r) { return r.text() }).then(function (text) { md5.value = text }) }</script>";
		
		content_type = "text/html\r\n";
		// html += HTML_INDEX;
	}
	else if (app.get("/hello", path, method)){
		html += "<h1>C++ Server /hello page</h1>";
		html += "<p>Hello, world!</p>";
		html += "<a href='/'>Go back</a><br><br>";
		html += "<a href='/info'>Go to memory viewer</a>";
		content_type = "text/html\r\n";

		// html += HTML_HELLO;
	}
	else if (app.get("/info", path, method)){
		// Show current memory usage

		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof (statex);
		GlobalMemoryStatusEx (&statex);

		html += "<h1>C++ Server /info page</h1>";
		html += "<a href='/'>Go back</a>";
		html += "<p id='memory'>Current memory usage:";
		html += "<script>setInterval(() => fetch('/memory').then(function(r){return r.text()}).then(function(text){let json=JSON.parse(text);document.getElementById('memory').innerHTML = `Current memory usage: <b>${json.used}/${json.total}</b> MB`}), 500)</script>";

		content_type = "text/html\r\n";
	}
	else {
		string newpath = path;
		string newmethod = method;
		response =  "HTTP/1.1 501 Not Implemented\r\n";
		response += "Server: C++ Server 1.2\r\n";
		response += "Content-Type: text/html\r\n\r\n";
	
		response += "<pre>Cannot " + newmethod + " " + newpath + "</pre>";

		// html += HTML_404;
		return response;
	}

	response =  "HTTP/1.1 200 OK\r\n";
	response += "Server: C++ Server 1.2\r\n";
	response += "Content-Type: " + content_type;
	response += "Content-Length: " + to_string(strlen(html.c_str())) + "\r\n\r\n";

	response += html;
	return response;
}

DWORD WINAPI NewClient(LPVOID socket){
	SOCKET HTTP_SOCKET = (SOCKET) socket;
	SOCKET s;


	if (HTTP_SOCKET == INVALID_SOCKET) printf("%s Accept failed with error code: %d\n", PREFIX, WSAGetLastError());

	char HTTP_REPONSE[1024];
	int recv_size = recv(HTTP_SOCKET, HTTP_REPONSE, sizeof(HTTP_REPONSE), 0);
	if (recv_size == SOCKET_ERROR) {
		printf("%s Recv failed with error code : %d",PREFIX , WSAGetLastError());
		closesocket(HTTP_SOCKET);
		return 0;
	}

	HTTP_REPONSE[recv_size] = '\0';
	// printf("%s Received %d bytes of data from client.\n", PREFIX, recv_size);
	
	char* DATA = strstr(HTTP_REPONSE, "\r\n\r\n");

	// Parse HTTP_REPONSE
	char *HTTP_METHOD = strtok(HTTP_REPONSE, " ");
	char *HTTP_URL = strtok(NULL, " ");
	char *HTTP_VERSION = strtok(NULL, " ");


	string request = Routing(HTTP_URL, DATA, HTTP_METHOD);

	// Get ip adress from client
	struct sockaddr_in client;
	int client_len = sizeof(client);
	getpeername(HTTP_SOCKET, (struct sockaddr*)&client, &client_len);


	// printf("%s %s %s %s\n", PREFIX, HTTP_METHOD, HTTP_URL, strtok(HTTP_VERSION, "\n"));
	printf("%s %s %s %s %s\n", PREFIX, inet_ntoa(client.sin_addr), HTTP_METHOD, HTTP_URL, strtok(HTTP_VERSION, "\n"));

	send(HTTP_SOCKET , request.c_str(), strlen(request.c_str()) , 0);
	closesocket(HTTP_SOCKET);
	return 1;
}

int main(int argc, char *argv[]){
	system("cls");

	// Save counter on exit
	atexit([] () { 
		ofstream File("visit_counter.txt");
		File << counter;
		File.close();
	});

	WSADATA wsa;
	SOCKET s, HTTP_SOCKET;
	struct sockaddr_in server;

	printf("%s Starting HTTP Server.\n", PREFIX);
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) printf("%s WSAStartup failed with error code: %d\n", PREFIX, WSAGetLastError());

	printf("%s Successfully started HTTP server.\n", PREFIX);

	//Create a socket
	if ((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) printf("%s Could not create socket: %d\n", PREFIX, WSAGetLastError());

	printf("%s Socket created.\n", PREFIX);

	//Setup the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(80);

	//Associate a local address with a socket.
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) printf("%s Bind failed with error code: %d\n", PREFIX, WSAGetLastError());
	printf("%s Socket bind complete.\n", PREFIX);


	//Listen to incoming connections
	listen(s , 3);

	// Start accepting clients in a new thread
	while (1) {
		HTTP_SOCKET = SOCKET_ERROR;
		while (HTTP_SOCKET == SOCKET_ERROR ) HTTP_SOCKET = accept( s, NULL, NULL );

		DWORD threadId;
		CreateThread(NULL, 0, NewClient, (LPVOID) HTTP_SOCKET, 0, &threadId);
	}
	closesocket(s);
	WSACleanup();
	return 0;
}
