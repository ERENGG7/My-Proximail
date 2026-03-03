#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

/**
* @file network_client.h
* @brief Cross-platform TCP client implementation.
*
* Supports Windows (WinSock2) and POSIX systems (Linux/macOS).
* Provides RAII wrappers for socket management and a high-level
* email-like messaging client interface.
*/

#ifdef _WIN32
//server port - 6000
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib,"ws2_32.lib")
 /**
  * Platform-independent socket type.
  * Windows: SOCKET
  */
using SocketHandle = SOCKET;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

/**
 * Platform-independent socket type.
 * Linux/macOS: int
 */
using SocketHandle = int;

#endif

#include <string>
#include <vector>
#include <stdint.h>
#include "message.h"
#include <mutex>

#ifdef _WIN32
constexpr SocketHandle invalid_socket = INVALID_SOCKET;
constexpr int socket_error = SOCKET_ERROR;
#else
constexpr SocketHandle invalid_socket = -1;
constexpr int socket_error = -1;
#endif
//closesocket commands:
//lasterror message 
#ifdef _WIN32
#define CLOSE_SOCKET(s) closesocket(s)
#define GET_LAST_ERROR() WSAGetLastError()
#else
#define CLOSE_SOCKET(s) close(s)
#define GET_LAST_ERROR() errno
#endif

/**
* @brief RAII wrapper for WSAStartup / WSACleanup.
*
* On Windows initializes Winsock in constructor
* and calls WSACleanup in destructor.
*
* Has no effect on Linux/macOS.
*/
struct Winsock {
	Winsock();
	~Winsock();

	Winsock(const Winsock&) = delete;
	Winsock& operator =(const Winsock&) = delete;
};

//class for socket:
class Socket {
	SocketHandle client{};
public:
	Socket();
	~Socket();
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	/**
	* @brief Move constructor transfers ownership.
	*/
	Socket(Socket&& other)noexcept;
	Socket& operator=(Socket&& other) noexcept;
	operator SocketHandle() const;
};
//class fornetwork client
/**
* @brief High-level TCP client for server communication.
*
* Thread-safe for send/receive operations (protected by mutex).
*/
class Network_Client {
private:
	Winsock wsa;
	Socket clientSocket;
	std::mutex socketLock;
public:

	bool sendMessage(const std::string& message);
	std::string receiveMessage();
	/**
	* @brief Connects to remote server.
	*
	* @param ip IPv4 address as string.
	* @param port TCP port number.
	* @return true on success, false on failure.
	*/
	bool connectToServer(const std::string& ip, const uint16_t& port);
	bool log_in(const std::string& email, const std::string& password);
	bool makeRegistration(const std::string& email, const std::string& password);
	bool resetNotifications(const std::string& email);
	bool sendEmail(const std::string& senderEmail, const std::string& recipientEmail, const std::string& content);
	unsigned getNotifications(const std::string& email);
	std::vector<Message> getInbox(const std::string& email);
	std::vector<Message> getSentMessages(const std::string& email);
	//INOBX function
	//Send messages function:
	std::vector<std::string>getUsersEmails();

};
#endif