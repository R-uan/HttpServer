#include <iostream>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>

#define PORT 8080
#define NUM_REQUESTS 5

void sendRequest(int id)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket creation failed");
		return;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connection failed");
		close(sock);
		return;
	}

	std::string message = "Request from thread " + std::to_string(id);
	send(sock, message.c_str(), message.size(), 0);

	char buffer[1024] = {0};
	read(sock, buffer, sizeof(buffer) - 1);
	std::cout << "Response from server: " << buffer << std::endl;

	close(sock);
}

int main()
{
	std::vector<std::thread> threads;

	for (int i = 0; i < NUM_REQUESTS; ++i)
	{
		threads.emplace_back(sendRequest, i);
	}

	for (auto &t : threads)
	{
		t.join();
	}

	return 0;
}
