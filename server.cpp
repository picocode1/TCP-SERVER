#include <winsock2.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

string ReadFile(string fileName){
	ifstream file(fileName);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

int PATH(char *URL, const char PATH[]) { return std::string(URL).compare(PATH) == 0;}

int main(int argc, char *argv[]){

	const char *PREFIX = "[SERVER]";

	// Load html files before starting server
	string HTML_INDEX = ReadFile("public/index.html");
	string HTML_HELLO = ReadFile("public/hello.html");
	string HTML_404 = ReadFile("public/404.html");

	WSADATA wsa;
    SOCKET s, HTTP_SOCKET;
    struct sockaddr_in server, client;
    int addr_param;
 
    printf("%s Starting TCP Server.\n", PREFIX);
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) printf("%s WSAStartup failed with error code: %d", PREFIX, WSAGetLastError());

    printf("%s Successfully started TCP server.\n", PREFIX);

    //Create a socket
    if ((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) printf("%s Could not create socket: %d", PREFIX, WSAGetLastError());
 
    printf("%s Socket created.\n", PREFIX);
     
    //Setup the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(80);
     
    //Associate a local address with a socket.
    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) printf("%s Bind failed with error code: %d", PREFIX, WSAGetLastError());
    printf("%s Socket bind complete.\n", PREFIX);
 

    //Listen to incoming connections
    listen(s , 3);
	while (1){

		addr_param = sizeof(struct sockaddr_in);
		HTTP_SOCKET = accept(s, (struct sockaddr *)&client, &addr_param);
	
		if (HTTP_SOCKET == INVALID_SOCKET) printf("%s Accept failed with error code: %d", PREFIX, WSAGetLastError());


		char HTTP_REPONSE[2048];
		int recv_size = recv(HTTP_SOCKET, HTTP_REPONSE, sizeof(HTTP_REPONSE), 0);
		if (recv_size == SOCKET_ERROR) {
			printf("%s Recv failed with error code : %d",PREFIX , WSAGetLastError());
			closesocket(HTTP_SOCKET);
			break;
		}
		// Received data from client
		HTTP_REPONSE[recv_size] = '\0';

		// Parse HTTP_REPONSE
		char *HTTP_METHOD = strtok(HTTP_REPONSE, " ");
		char *HTTP_URL = strtok(NULL, " ");
		char *HTTP_VERSION = strtok(NULL, " ");

		// Trim new lines
		strtok(HTTP_VERSION, "\n");

		string request = "HTTP/1.1 200 OK\r\n";
		request += "Server: C++ Server\r\n";

		if (PATH(HTTP_URL, "/favicon.ico")) request += "Content-Type: image/x-icon\r\n";
		else if (PATH(HTTP_URL, "/")) {
			request += "Content-Type: text/html\r\n";
			request += "\r\n";
			request += HTML_INDEX;

			const char *replace = "@path";
			request.replace(request.find(replace), sizeof(replace) + 1, argv[0]);
		}
		else if (PATH(HTTP_URL, "/hello")) {
			request += "Content-Type: text/html\r\n";
			request += "\r\n";
			request += HTML_HELLO;
		}
		else {
			// 404 NOT FOUND
			request += "Content-Type: text/html\r\n";
			request += "\r\n";
			request += HTML_404;
		}

		// GET /hello HTTP/1.1
		printf("%s %s %s %s\n", PREFIX, HTTP_METHOD, HTTP_URL, HTTP_VERSION);

		send(HTTP_SOCKET , request.c_str(), strlen(request.c_str()) , 0);
		closesocket(HTTP_SOCKET);
	}
    closesocket(s);
    WSACleanup();
    return 0;
}