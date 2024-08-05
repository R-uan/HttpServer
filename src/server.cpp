#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

struct HttpStatusCode
{
	int httpCode;
	std::string httpStatus;
};

enum HttpStatus
{
	OK = 200,
	NotFound = 404,
	InternalServerError = 500
};

HttpStatusCode getHttpStatusCode(HttpStatus status);
void handleRequest(int client_fd, std::string &request);
std::string readFileAsString(const std::string &filePath);
std::string buildResponse(const std::string &content, const std::string &contentType, HttpStatus status);

int main()
{
	//
	//	How to create a socket:
	//		The socket is created with the socket method that returns a file description
	//		A file description is basically an ID used to identify open files and connections.
	//		This file description is what we're going to use to reference the socket, as if it was a pointer.
	//
	//	Parameters
	//		1: defines the type of domain that the socket will use. In this case AF_INET means that
	//		the IP will be IPv4.
	//
	//		2: defines the kind of connection that will be used. SOCK_STREAM defines that the connection will be
	//		of TCP.
	//
	//		3: defines the protocol used. 0 means that the system should select whichever is the default for the kind
	//		in this case will be TCP.
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		perror("Failed to create socket.");
		exit(EXIT_FAILURE);
	}

	std::cout << "Socket created. File Descriptor: " << server_fd << std::endl;

	//
	//	sockaddr is the properties used to bind the socket into a IP address and PORT. The struct has these values
	//	and is later used for it's purpose.
	//
	//	sin_family declares which type of domain is being used, in this case IPv4
	//	sin_port declares the port that the socket will listen on.
	//	sin_addr.s_addr declares the IP that the socket will listen on.

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = INADDR_ANY;

	//
	//	Binding the socket to the address struct.
	if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("Unable to bind socket to address");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Socket bound to address." << std::endl;

	if (listen(server_fd, BACKLOG) < 0)
	{
		perror("Failed to listen to the socket.");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << "Listening to socket on " << PORT << std::endl;

	while (true)
	{
		int client_fd = accept(server_fd, nullptr, nullptr);
		if (client_fd < 0)
		{
			perror("Failed to accept request.");
			close(server_fd);
			exit(EXIT_FAILURE);
		}

		char buffer[BUFFER_SIZE];
		int requestVal = read(client_fd, buffer, BUFFER_SIZE);
		if (requestVal > 0)
		{
			std::string request(buffer);
			handleRequest(client_fd, request);
		}

		close(client_fd);
	}

	close(server_fd);
	exit(EXIT_SUCCESS);
}

std::string readFileAsString(const std::string &filePath)
{
	std::ifstream file(filePath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void handleRequest(int client_fd, std::string &request)
{
	std::string response;
	if (request.find("GET / HTTP/1.1") != std::string::npos)
	{
		std::string content = readFileAsString("./assets/index.html");
		response = buildResponse(content, "text/html", HttpStatus::OK);
	}
	else if (request.find("GET /about HTTP/1.1") != std::string::npos)
	{
		std::string content = readFileAsString("./assets/about.html");
		response = buildResponse(content, "text/html", HttpStatus::OK);
	}
	else if (request.find("GET /index.css HTTP/1.1") != std::string::npos)
	{
		std::string content = readFileAsString("./assets/index.css");
		response = buildResponse(content, "text/css", HttpStatus::OK);
	}
	else
	{
		response = "HTTP/1.1 404 Not Found\r\n";
		response += "Connection: close\r\n";
		response += "\r\n";
		response += readFileAsString("./assets/not-found.html");
	}

	send(client_fd, response.c_str(), response.size(), 0);
}

std::string buildResponse(const std::string &content, const std::string &contentType, HttpStatus statusCode)
{
	std::string response;
	HttpStatusCode status = getHttpStatusCode(statusCode);
	response = "HTTP/1.1 " + std::to_string(status.httpCode) + " " + status.httpStatus + "\r\n";
	response += "Content-Type: " + contentType + "\r\n";
	response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
	response += "Connection: close \r\n";
	response += "\r\n";
	response += content;
	return response;
}

HttpStatusCode getHttpStatusCode(HttpStatus status)
{
	switch (status)
	{
	case OK:
		return {200, "OK"};
	case NotFound:
		return {404, "Not Found"};
	case InternalServerError:
		return {500, "Internal Server Error"};
	default:
		return {0, "Unknown Status"};
	}
}