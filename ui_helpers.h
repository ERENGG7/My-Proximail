/**
* @file ui_helpers.h
* @brief Helper structures and functions for UI rendering using ImGui.
*
* This file provides:
*  - UIColors: predefined color constants for consistent UI styling.
*  - MessageUI: utility functions for displaying text messages in the UI,
*    including errors, info texts, and account creation hints.
*
* These helpers are used throughout the application to standardize colors,
* text formatting, and error/info display in ImGui-based interfaces.
*/
#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include "imgui_impl_dx11.h"
#include <fstream>
#include <sstream>

//colors:
struct UIColors {
	static constexpr ImVec4 BLACK = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	static constexpr ImVec4 YELLOW = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	static constexpr ImVec4 DARK_YELLOW = ImVec4(0.8f, 0.7f, 0.0f, 1.0f);
	static constexpr ImVec4 RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	static constexpr ImVec4 BLUE = ImVec4(0.0f, 0.0f, 0.3f, 1.0f);
	static constexpr ImVec4 SKY_BLUE = ImVec4(0.529f, 0.808f, 0.922f, 1.0f);
};
//text for error/wrong email password:
struct MessageUI {
	static std::string LoadTextFile(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return "Failed to load text file.";

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	static void textInRed(const char* text) {
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::RED);
		ImGui::TextWrapped("%s", text);
		ImGui::PopStyleColor();
	}
	static void infoText() {
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::YELLOW);
		static std::string registrationInfo = LoadTextFile("texts/main_info.txt");
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
		ImGui::TextUnformatted(registrationInfo.c_str());
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
	}
	//create account info:
	static void infoForAccount() {
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::YELLOW);

		static std::string registrationInfo = LoadTextFile("texts/email_info.txt");
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
		ImGui::TextUnformatted(registrationInfo.c_str());
		ImGui::PopTextWrapPos();

		ImGui::PopStyleColor();
	}
	static void incorrectCreatedPassword() {
		ImGui::PushStyleColor(ImGuiCol_Text, UIColors::RED);
		static std::string passwordWarning = LoadTextFile("texts/email_password_warning.txt");
		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
		ImGui::TextUnformatted(passwordWarning.c_str());
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
	}
};
#endif