#include <winsock2.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

const char *PREFIX = "[SERVER]";

string ReadFile(string fileName){
	ifstream file(fileName);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
int PATH(char *URL, const char PATH[]) { return std::string(URL).compare(PATH) == 0;}

// Store html files in memory
string HTML_INDEX = ReadFile("public/index.html");
string HTML_HELLO = ReadFile("public/hello.html");
string HTML_404 = ReadFile("public/404.html");


string Request(char* path){
	string html, response;

	if (PATH(path, "/")) {
		html += HTML_INDEX;
	}
	else if (PATH(path, "/hello")) {
		html += HTML_HELLO;
	}
	else {
		html += HTML_404;
	}

	response =  "HTTP/1.1 200 OK\r\n";
	response += "Server: C++ Server 1.1\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + to_string(strlen(html.c_str())) + "\r\n";
	response += "\r\n";
	response += html;
	return response;
}

DWORD WINAPI NewClient (LPVOID socket){
	SOCKET HTTP_SOCKET = (SOCKET) socket;
	SOCKET s;
	int addr_param;

	addr_param = sizeof(struct sockaddr_in);

	if (HTTP_SOCKET == INVALID_SOCKET) printf("%s Accept failed with error code: %d\n", PREFIX, WSAGetLastError());

	char HTTP_REPONSE[2048];
	int recv_size = recv(HTTP_SOCKET, HTTP_REPONSE, sizeof(HTTP_REPONSE), 0);
	if (recv_size == SOCKET_ERROR) {
		printf("%s Recv failed with error code : %d",PREFIX , WSAGetLastError());
		closesocket(HTTP_SOCKET);
		return 0;
	}
	// Received data from client
	HTTP_REPONSE[recv_size] = '\0';

	// Parse HTTP_REPONSE
	char *HTTP_METHOD = strtok(HTTP_REPONSE, " ");
	char *HTTP_URL = strtok(NULL, " ");
	char *HTTP_VERSION = strtok(NULL, " ");

	// Trim new lines
	strtok(HTTP_VERSION, "\n");

	string request = Request(HTTP_URL);

	printf("%s %s %s %s\n", PREFIX, HTTP_METHOD, HTTP_URL, HTTP_VERSION);

	send(HTTP_SOCKET , request.c_str(), strlen(request.c_str()) , 0);
	closesocket(HTTP_SOCKET);
	return 0;
}


int main(int argc, char *argv[]){
	const char *PREFIX = "[SERVER]";

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
		CreateThread (NULL, 0, NewClient, (LPVOID) HTTP_SOCKET, 0, &threadId);
	}
	closesocket(s);
	WSACleanup();
	return 0;
}
