#define STB_IMAGE_IMPLEMENTATION

#include "email_ui.h"
#include "ui_helpers.h"
#include <algorithm>


LoginBox::LoginBox(Network_Client& c) : client(c) {
	memset(emailInputBuffer, 0, sizeof(emailInputBuffer));
	memset(passwordInputBuffer, 0, sizeof(passwordInputBuffer));
}
/**
* @brief draws the login box logic
* @param boolean for connection to server
* @param for users login that will be changed to true after login
* @param users email
* @param emails size 
*/
void LoginBox::drawLoginBox(const bool& connectedToServer,bool& logedIn, char* outEmail, size_t size) {

	ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLUE);
	ImGui::Text("Enter email:");
	//email input draw:
	ImGui::PushID("email input");

	ImGui::PushItemWidth(300);
	ImGui::InputText("", emailInputBuffer, sizeof(emailInputBuffer));
	ImGui::PopID();
	ImGui::PopItemWidth();
	ImGui::Text("Enter password");
	//input text for password:
	ImGui::PushID("name input");
	ImGui::PushItemWidth(300);

	ImGui::InputText("", passwordInputBuffer,
		sizeof(passwordInputBuffer),
		ImGuiInputTextFlags_Password);
	ImGui::PopID();
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	ImGui::PushID("ButtonOne");
	if ((strlen(emailInputBuffer) > 0 &&
		strlen(passwordInputBuffer) > 0) &&
		connectedToServer &&
		ImGui::Button("Log in")) {

		if (client.log_in(emailInputBuffer, passwordInputBuffer)) {
			strncpy_s(outEmail, size, emailInputBuffer, _TRUNCATE);
			memset(emailInputBuffer, 0, sizeof(emailInputBuffer));
			wrongEmailPassword = false;
			logedIn = true;
		}
		else
			wrongEmailPassword = true;
		sodium_memzero(passwordInputBuffer, sizeof(passwordInputBuffer));
	}
	ImGui::PopID();
	if (wrongEmailPassword)
		MessageUI::textInRed("Wrong email or password");
}
/*
* @brief draws the registration box logic.
* @param boolean for connection to server.
* @param for users login that will be changed to true after sign up.
* @param users email.
* @param emails size
*/
void RegistrationBox::drawInputs() {
	ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLUE);
	ImGui::Text("Create username");

	ImGui::PushID("createusername");
	ImGui::PushItemWidth(300);
	ImGui::InputText("", createEmailInputBuffer,
		sizeof(createEmailInputBuffer));
	ImGui::PopItemWidth();
	ImGui::PopID();

	//password
	ImGui::Text("Create password");
	ImGui::PushID("createpassword");
	ImGui::PushItemWidth(300);
	ImGui::InputText("",
		createPasswordInputBuffer,
		sizeof(createPasswordInputBuffer),
		ImGuiInputTextFlags_Password);
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::Text("Reenter password");

	//reenter password
	ImGui::PushID("repassword");
	ImGui::PushItemWidth(300);
	ImGui::InputText("",
		reEnterPasswordInputBuffer,
		sizeof(reEnterPasswordInputBuffer),
		ImGuiInputTextFlags_Password);
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::PopStyleColor();
}
RegistrationBox::RegistrationBox(Network_Client& e) :client(e) {
	memset(createEmailInputBuffer, 0, sizeof(createEmailInputBuffer));
	memset(createPasswordInputBuffer, 0, sizeof(createPasswordInputBuffer));
	memset(reEnterPasswordInputBuffer, 0, sizeof(reEnterPasswordInputBuffer));
}
void RegistrationBox::drawRegistrationBox(const bool& connectedToServer,bool& logedIn, char* outEmail, size_t size) {
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, UIColors::BLUE);
	ImGui::BeginChild("infoforregistration", ImVec2(300, 100), false);
	MessageUI::infoForAccount();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	drawInputs();
	if (strlen(createEmailInputBuffer) > 0
		&& strlen(createPasswordInputBuffer) > 0
		&& strlen(reEnterPasswordInputBuffer) > 0) {
		if (connectedToServer && ImGui::Button("Create account")) {
			if (strcmp(createPasswordInputBuffer, reEnterPasswordInputBuffer) != 0
				|| !client.makeRegistration(createEmailInputBuffer, createPasswordInputBuffer))
				uncorrectNamePassword = true;
			else {
				sprintf_s(outEmail, size, "%s@proximail.com", createEmailInputBuffer);
				uncorrectNamePassword = false;
				memset(createEmailInputBuffer, 0, sizeof(createEmailInputBuffer));
				logedIn = true;
			}
			sodium_memzero(createPasswordInputBuffer, sizeof(createPasswordInputBuffer));
			sodium_memzero(reEnterPasswordInputBuffer, sizeof(reEnterPasswordInputBuffer));
		}
	}
	if (uncorrectNamePassword) {
		MessageUI::incorrectCreatedPassword();
	}
}
/**
* @brief starts thread for reading data from server in every two seconds.
* @note it works until application is closed.
*/
void UILogic::startWorker() {
    //strat thread for reading from server:
	worker = std::thread([this]() {
		while (!stopWorker.load()) {
			// Fetch data
			auto inbox = client.getInbox(email);
			auto notifs = client.getNotifications(email);
			auto users = client.getUsersEmails();
			auto history = client.getSentMessages(email);

			//sort notifications: 
			std::reverse(inbox.begin(), inbox.end());
			std::reverse(history.begin(), history.end());
			// Protect shared data
			{
				std::lock_guard<std::mutex> lock(dataMutex);
				inboxMessages = std::move(inbox);
				notifications = notifs;
				proxiUsers = std::move(users);
				sendMessages = std::move(history);
			}

			// Sleep in small chunks for fast shutdown
			for (int i = 0; i < 20 && !stopWorker.load(); i++)
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		});
}
void UILogic::stopWorkerThread() {
	stopWorker.store(true);
	if (worker.joinable())
		worker.join();
}
//initialize picture:
bool UILogic::initTextures(LPDIRECT3DDEVICE9 device) {
	if (!picture.LoadFromFile(device, "logo/logo.jpg")) {
		IM_ASSERT(false && "Failed to load logo texture!");
		std::abort();
		return false;
	}
	return true;
}
void UILogic::drawMessageBox() {
	ImGui::BeginChild("text", ImVec2(880, 200), false);
	ImGui::Text("Send to:");
	ImGui::PushID("senderBuffer");
	ImGui::PushItemWidth(880);
	ImGui::InputText("", senderBuffer, sizeof(senderBuffer));
	ImGui::PopItemWidth();
	ImGui::PopID();
	ImGui::Text("Content: ");
	ImGui::PushItemWidth(880);//push width
	ImGui::PushID("sendmessage");
	ImGui::InputText("", messageBuffer, sizeof(messageBuffer));
	ImGui::PopID();
	ImGui::PopItemWidth();//pop width
	ImGui::PushStyleColor(ImGuiCol_Button, UIColors::YELLOW);
	//text:
	ImGui::TextWrapped("%s", messageBuffer);

	ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLUE);
	if ((strlen(senderBuffer) > 0 &&
		strlen(messageBuffer) > 0) &&
		ImGui::Button("send email")) {

		if (!client.sendEmail(email, senderBuffer, messageBuffer))
			messageNotSend = true;
		else {
			messageNotSend = false;
			memset(senderBuffer, 0, sizeof(senderBuffer));
			memset(messageBuffer, 0, sizeof(messageBuffer));
		}
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	if (messageNotSend) {
		MessageUI::textInRed("receiver not found");
	}
	ImGui::EndChild();
}
UILogic::UILogic(Network_Client& c)
	: client(c),
	loginBox(c),
	registrationBox(c) {
	memset(email, 0, sizeof(email));
}
UILogic::~UILogic() {
	stopWorkerThread();
}
/**
* @brief draws the main logic of the application.
* @param dx9texture.
* @param boolean that when user closes the application it becomse false.
*/
void UILogic::drawUI(LPDIRECT3DDEVICE9 device, bool& open) {
	if (!initializedTexture) {
		initTextures(device);
		initializedTexture = true;
	}

	ImGui::SetNextWindowSize(ImVec2(800, 420), ImGuiCond_Always);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, UIColors::SKY_BLUE);
	ImGui::PushStyleColor(ImGuiCol_Button, UIColors::BLUE); // <--- Push #2

	ImGui::Begin("Proxmail", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	static bool tryToConnectToServer = false;
	if (!tryToConnectToServer) {
		connectedToServer = client.connectToServer("192.168.1.10", 6000);
		tryToConnectToServer = true;
	}

	if (!logedIn) {
		ImGui::PushStyleColor(ImGuiCol_ChildBg, UIColors::BLUE);
		ImGui::BeginChild("info", ImVec2(385, 380), false);
		picture.Show(ImVec2(390, 180));
		MessageUI::infoText();
		ImGui::EndChild();
		ImGui::PopStyleColor(); // Pops ChildBg

		ImGui::SetCursorPos(ImVec2(430, 30));
		ImGui::BeginChild("clindOne", ImVec2(320, 380), false);
		if (!connectedToServer)
			MessageUI::textInRed("connection to server failed");

		ImGui::SetWindowFontScale(1.3f);
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::YELLOW); 
		ImGui::Text("\t   Get Started!");
		ImGui::PopStyleColor(); 
		ImGui::SetWindowFontScale(1.0f);

		if (ImGui::Button("Log in", ImVec2(300, 40))) {
			logIn = true;
			signUp = !logIn;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLACK);
		ImGui::Text("\t      --------------or--------------");
		ImGui::PopStyleColor();

		if (ImGui::Button("Sign up", ImVec2(300, 40))) {
			signUp = true;
			logIn = !signUp;
		}

		if (logIn)
			loginBox.drawLoginBox(connectedToServer,logedIn, email, sizeof(email));
		if (signUp)
			registrationBox.drawRegistrationBox(connectedToServer,logedIn, email, sizeof(email));
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	else {
		if (!loginInitialized) {
			loginInitialized = true;
			startWorker();
		}
		ImGui::SetWindowFontScale(1.4f);
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLUE);
		ImGui::Text("Wellcome %s", email);
		ImGui::PopStyleColor();
		ImGui::SetWindowFontScale(1.0f);
		
		static constexpr ImVec2 CHILD_SIZE = ImVec2(290, 424);
		picture.Show(ImVec2(280, 220));
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::YELLOW); //push yellow
		ImGui::PushStyleColor(ImGuiCol_ChildBg, UIColors::BLUE);

		ImGui::BeginChild("send", ImVec2(280, 200), true);
		{
			std::lock_guard lock(dataMutex);
			ImGui::Text("Notifications: %i", notifications);
			ImGui::PushStyleColor(ImGuiCol_Button, UIColors::YELLOW);
			ImGui::PushStyleColor(ImGuiCol_Text, UIColors::BLUE);
			if (ImGui::Button("Mark as viwewd"))
				client.resetNotifications(email);
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		ImGui::Text("History:");
		//For loop for send messages:
		{
			std::lock_guard<std::mutex> lock(dataMutex);
			if (!sendMessages.empty()) {
				for (const auto& message : sendMessages) {
					ImGui::TextWrapped("%s\n", message.timestamp.c_str());
					ImGui::TextWrapped("send to: %s", message.sender.c_str());
					ImGui::TextWrapped("%s", message.content.c_str());
				}
			}
		}
		ImGui::EndChild();
		//inbox:
		ImGui::SetCursorPos(ImVec2(300, 69));

		ImGui::BeginChild("inbox", CHILD_SIZE, true);
		ImGui::Text("\tInbox:");
		
		{
			std::lock_guard<std::mutex> lock(dataMutex);
			if (!inboxMessages.empty()) {
				for (const auto& message : inboxMessages) {
					ImGui::TextWrapped("%s\n", message.timestamp.c_str());
					ImGui::TextWrapped("from %s", message.sender.c_str());
					ImGui::TextWrapped("%s", message.content.c_str());
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		//users child:
		ImGui::BeginChild("users", CHILD_SIZE, true);
		ImGui::Text("Users: ");
		{
			std::lock_guard<std::mutex> lock(dataMutex);
			for (size_t i = 0; i < proxiUsers.size(); i++) {
				ImGui::BeginDisabled();
				ImGui::PushID(i);
				ImGui::Button(proxiUsers[i].c_str(), ImVec2(250, 30));
				ImGui::PopID();
				ImGui::EndDisabled();
			}
		}
		ImGui::EndChild();
		drawMessageBox();

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		
		ImGui::PopStyleColor();
	}
	ImGui::End();
	ImGui::PopStyleColor();
}