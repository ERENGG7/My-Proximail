#ifndef EMAIL_UI_H
#define EMAIL_UI_H

#include "network_client.h"
//port to connet -6000
//host 127 0 0 1-local host for test!
#ifdef WIN32
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#else
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

#include "stb_image.h"
#include "texture.h"
#include "sodium_init.h"
#include "message.h"
#include <sodium.h>
#include <chrono>

#include <atomic>
#include <thread>
#include <mutex>

/** 
*@brief Login box ui logic
*/
class LoginBox {
private:
	Network_Client& client;

	//name password buffers:
	char emailInputBuffer[50];
	char passwordInputBuffer[50];

	bool wrongEmailPassword{ false };
public:
	LoginBox(Network_Client& c);
	void drawLoginBox(const bool& connectedToServer,bool& logedIn, char* outEmail, size_t size);
};
/** 
*@brief Registration box ui logic
*/
class RegistrationBox {
private:
	Network_Client& client;
	//buffer for name password and verify password:
	char createEmailInputBuffer[50];
	char createPasswordInputBuffer[50];
	char reEnterPasswordInputBuffer[50];
	bool uncorrectNamePassword{ false };
	void drawInputs();
public:
	RegistrationBox(Network_Client& e);
	void drawRegistrationBox(const bool&connectedToServer,bool& logedIn, char* outEmail, size_t size);
};
class UILogic {
private:
	Network_Client& client;
	LoginBox loginBox;
	RegistrationBox registrationBox;
#ifdef WIN32
	Dx9Texture picture;
#else
	GLTexture picture;
#endif
	char email[50];
	char senderBuffer[50];
	char messageBuffer[1024];
	bool initializedTexture{ false };

	bool connectedToServer{ false };

	bool logedIn{ false };	
	bool logIn = { false };
	bool signUp{ false };

	bool messageNotSend { false };

	bool loginInitialized{ false };
	std::atomic<bool> stopWorker{ false };
	std::thread worker;

	uint32_t notifications;
	std::vector<Message>inboxMessages;
	std::vector<Message>sendMessages;
	std::vector<std::string>proxiUsers;

	std::mutex dataMutex;
	std::mutex clientMutex;

	void startWorker();
	void  stopWorkerThread();
	bool initTextures(LPDIRECT3DDEVICE9 device);
public:
	UILogic(Network_Client& c);
	~UILogic();
	void drawMessageBox();
	void drawUI(LPDIRECT3DDEVICE9 device, bool& open);
};
#endif
