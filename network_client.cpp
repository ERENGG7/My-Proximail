
#include "network_client.h"
#include <stdexcept>
#include <thread>

/*
* @brief initializing Winsock.
* @throws std::runtime_error if WSAStartup fails.
* @note This implementation is specific to the Windows API.
*/
Winsock::Winsock() {

#ifdef _WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw std::runtime_error("WSAStartup failed");
#endif
}
Winsock::~Winsock() {
#ifdef _WIN32
	WSACleanup();
#endif
}

Socket::Socket() :client(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {
	if (client == invalid_socket)
		throw std::runtime_error("Socket create failed" + std::to_string(GET_LAST_ERROR()));

}
Socket::~Socket() {
	if (client != invalid_socket) CLOSE_SOCKET(client);
	client = invalid_socket;
}

Socket::Socket(Socket&& other)noexcept :client(other.client) {
	other.client = invalid_socket;
}
Socket& Socket::operator=(Socket&& other) noexcept {
	if (this != &other) {
		if (client != invalid_socket)
			CLOSE_SOCKET(client);
		client = other.client;
		other.client = invalid_socket;
	}
	return *this;
}
/*
* @brief returns operator if funtion is waiting for paran SOCKET.
*/
Socket::operator SocketHandle() const {
	return client;
}
/*
* @brief sends message to the server.
* @note Ensures that the entire message buffer is transmitted.
* @param message that will be send.
* @return true if the message is send successfully.
*/
bool Network_Client::sendMessage(const std::string& message) {
	if (clientSocket == invalid_socket) return false;

	int totalSent{ 0 };
	int messageSize{ (int)message.size() };

	while (totalSent < messageSize) {
		int sent{ send(clientSocket,
			message.c_str() + totalSent,
			messageSize - totalSent, 0)
		};

		if (sent == socket_error)
			return false;
		totalSent += sent;
	}
	return true;
}
/*
* @brief receives message and removes \n \r.
* @return the message.
*/
std::string Network_Client::receiveMessage() {
	char buffer[4096];
	std::string response;

	while (true) {
		int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytes == socket_error || bytes == 0)
			break; 
		response.append(buffer, bytes);
		if (response.find('\n') != std::string::npos)
			break;
	}
	response.erase(std::remove(response.begin(), response.end(), '\r'), response.end());
	response.erase(std::remove(response.begin(), response.end(), '\n'), response.end());
	return response;
}
/*
* @brief connects to the server.
* @param ip in string.
* @param the servers port.
* If inet_pton or connect throw error the function returns false.
* @return true if connection is successfull.
*/
bool Network_Client::connectToServer(const std::string& ip, const uint16_t& port) {
	try {
		sockaddr_in addr{};
		addr.sin_family = AF_INET;   
		addr.sin_port = htons(port); 
		if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
			throw std::runtime_error("inet failed" + std::to_string(GET_LAST_ERROR()));
		}
		if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			throw std::runtime_error("Connect failed" + std::to_string(GET_LAST_ERROR()));
		}
		return true;
	}
	catch (...) {
		return false;
	}
}
//log in function:
bool Network_Client::log_in(const std::string& email, const std::string& password) {
	std::string request = "LOGIN|" + email + "|" + password + "\n";
	std::lock_guard<std::mutex> lock(socketLock);
	if (!sendMessage(request)) return false;

	std::string response = receiveMessage();
	return response == "OK";
}
//registration function:
bool Network_Client::makeRegistration(const std::string& email, const std::string& password) {
	std::string request = "REGISTRATION|" + email + "|" + password + "\n";
	std::lock_guard<std::mutex> lock(socketLock);
	if (!sendMessage(request)) return false;

	std::string response = receiveMessage();
	return response == "OK";

}
//reset notifications:
bool Network_Client::resetNotifications(const std::string& email) {
	std::string request = "MARK AS VIEWED|" + email + '\n';
	std::lock_guard<std::mutex> lock(socketLock);
	if (!sendMessage(request)) return false;

	std::string resp = receiveMessage();
	return resp == "OK";
}
//function for sending email:
bool Network_Client::sendEmail(const std::string& senderEmail,
	const std::string& recipientEmail,
	const std::string& content) {
	std::string request = "SEND EMAIL|" + senderEmail + "|" + recipientEmail + "|" + content + '\n';
	std::lock_guard<std::mutex> lock(socketLock);
	if (!sendMessage(request)) return false;

	std::string resp = receiveMessage();
	return resp == "OK";
}
//get notifications count:
unsigned Network_Client::getNotifications(const std::string& email) {
	std::string request = "GET NOTIFICATIONS|" + email + '\n';
	std::lock_guard<std::mutex> lock(socketLock);
	sendMessage(request);
	std::string response = receiveMessage();
	//requests type-unsigned
	try {
		return std::stoul(response); //unsignednumber
	}
	catch (...) {
		return 0;
	}
}
std::vector<Message> Network_Client::getInbox(const std::string& email) {
	std::vector<Message> messages;
	std::string request = "GET INBOX|" + email + '\n';
	std::lock_guard<std::mutex> lock(socketLock);
	sendMessage(request);

	std::string response = receiveMessage();
	std::vector<std::string>splitMessageStrings;
	size_t start{ 0 }, end;

	if (!response.empty()) {
		while ((end = response.find('|', start)) != std::string::npos) {
			auto token = response.substr(start, end - start);
			if (!token.empty())
				splitMessageStrings.push_back(token);
			start = end + 1;
		}
		if (start < response.size())
			splitMessageStrings.push_back(response.substr(start));

		for (const auto& msgStr : splitMessageStrings) {
			size_t senderEnd = msgStr.find(';');
			size_t contentEnd = msgStr.find(';', senderEnd + 1);
			if (senderEnd != std::string::npos && contentEnd != std::string::npos) {
				Message msg;
				msg.sender = msgStr.substr(0, senderEnd);
				msg.content = msgStr.substr(senderEnd + 1, contentEnd - senderEnd - 1);
				msg.timestamp = msgStr.substr(contentEnd + 1);
				messages.push_back(std::move(msg));
			}
		}
	}
	return messages;
}
/*
* @brief gets all users send messages form server and separates them.
* @param users email.
* @return std::vector with users messages. 
*/
std::vector<Message> Network_Client::getSentMessages(const std::string& email) {
	std::vector<Message> messages;
	std::string request = "GET SENT MESSAGES|" + email + '\n';
	std::lock_guard<std::mutex> lock(socketLock);
	sendMessage(request);

	std::string response = receiveMessage();
	std::vector<std::string>splitMessageStrings;
	size_t start{ 0 }, end;
	if (!response.empty()) {
		while ((end = response.find('|', start)) != std::string::npos) {
			auto token = response.substr(start, end - start);
			if (!token.empty())
				splitMessageStrings.push_back(token);
			start = end + 1;
		}
		if (start < response.size())
			splitMessageStrings.push_back(response.substr(start));

		for (const auto& msgStr : splitMessageStrings) {
			size_t senderEnd = msgStr.find(';');
			size_t contentEnd = msgStr.find(';', senderEnd + 1);
			if (senderEnd != std::string::npos && contentEnd != std::string::npos) {
				Message msg;
				msg.sender = msgStr.substr(0, senderEnd);
				msg.content = msgStr.substr(senderEnd + 1, contentEnd - senderEnd - 1);
				msg.timestamp = msgStr.substr(contentEnd + 1);
				messages.push_back(std::move(msg));
			}
		}
	}
	return messages;
}
/*
* @brief gets all email accounts from server.
* @return std::vector with email accounts.
*/
std::vector<std::string>Network_Client::getUsersEmails() {
	std::vector<std::string>emails;
	std::string request = "GET USERS|\n";
	std::lock_guard<std::mutex> lock(socketLock);
	sendMessage(request);
	std::string response = receiveMessage();

	size_t start{ 0 }, end;
	//split emails:
	while ((end = response.find(';', start)) != std::string::npos) {
		auto token = response.substr(start, end - start);
		if (!token.empty())
			emails.push_back(token);
		start = end + 1;
	}
	if (start < response.size())
		emails.push_back(response.substr(start));
	return emails;
}