///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    Menu_CharacterWindow.cpp
* \author  Daniel Steuernol steu@allacrost.org
* \author  Andy Gardner chopperdave@allacrost.org
* \brief   Source file for the character menu.
*****************************************************************************/

#include <iostream>
#include <sstream>

#include "audio.h"
#include "input.h"
#include "system.h"

#include "menu.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_global;
using namespace hoa_input;
using namespace hoa_system;

namespace hoa_menu {

	namespace private_menu {

		////////////////////////////////////////////////////////////////////////////////
		// CharacterWindow Class
		////////////////////////////////////////////////////////////////////////////////

		CharacterWindow::CharacterWindow() : _char_id(GLOBAL_CHARACTER_INVALID) {
		}

		CharacterWindow::~CharacterWindow() {
		}



		void CharacterWindow::SetCharacter(GlobalCharacter *character) {
			_char_id = character->GetID();
			_portrait.SetStatic(true);
			std::string portrait_filename = "img/portraits/face/" + character->GetFilename() + "_small.png";
			if (DoesFileExist(portrait_filename) == false) {
				PRINT_WARNING << "Portrait file not found: " << portrait_filename << endl;
				_portrait.Load("", 100.0f, 100.0f); // Load an empty image
			}
			else {
				_portrait.Load(portrait_filename, 100.0f, 100.0f);
			}
		}


		// Draw the window to the screen
		void CharacterWindow::Draw() {
			// Call parent Draw method, if failed pass on fail result
			MenuWindow::Draw();
			// check to see if this window is an actual character
			if (_char_id == hoa_global::GLOBAL_CHARACTER_INVALID)
				// no more to do here
				return;
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
			// Get the window metrics
			float x, y, w, h;
			GetPosition(x, y);
			GetDimensions(w, h);
			// Adjust the current position to make it look better
			y += 5;
			GlobalCharacter *character = GlobalManager->GetCharacter(_char_id);
			//Draw character portrait
			VideoManager->Move(x + 12, y + 8);
			_portrait.Draw();

			// Write character name
			VideoManager->MoveRelative(150, -5);
			VideoManager->Text()->Draw(character->GetName(), TextStyle("title22"));

			// Level
			VideoManager->MoveRelative(0, 20);
			VideoManager->Text()->Draw(UTranslate("Lv: ") + MakeUnicodeString(NumberToString(character->GetExperienceLevel())), TextStyle("text20"));

			// HP
			VideoManager->MoveRelative(0, 20);
			VideoManager->Text()->Draw(UTranslate("HP: ") + MakeUnicodeString(NumberToString(character->GetHitPoints()) +
				" (-" + NumberToString(character->GetHitPointFatigue()) + ")"), TextStyle("text20"));

			// SP
			VideoManager->MoveRelative(0, 20);
			VideoManager->Text()->Draw(UTranslate("SP: ") + MakeUnicodeString(NumberToString(character->GetSkillPoints()) +
				" (-" + NumberToString(character->GetSkillPointFatigue()) + ")"), TextStyle("text20"));

			// XP to level up
			VideoManager->MoveRelative(0, 20);
			VideoManager->Text()->Draw(UTranslate("XP to Next: ") +
				MakeUnicodeString(NumberToString(character->GetExperienceForNextLevel())), TextStyle("text20"));
			return;
		}	//CharacterWindow_draw

	} // namespace private_menu


	MessageWindow::MessageWindow(const ustring &message, float w, float h) :
		_message(message)
	{
		float start_x = (1024 - w) / 2;
		float start_y = (768 - h) / 2;

		MenuWindow::Create(w, h);
		MenuWindow::SetPosition(start_x, start_y);
		MenuWindow::Show();

		_textbox.SetPosition(30, 5);
		_textbox.SetDimensions(w, h);
		_textbox.SetTextStyle(TextStyle("text22"));
		_textbox.SetDisplayMode(VIDEO_TEXT_INSTANT);
		_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		_textbox.SetDisplayText(_message);
		_textbox.SetOwner(this);
	}

	MessageWindow::~MessageWindow()
	{
		MenuWindow::Destroy();
	}

	void MessageWindow::Draw()
	{
		MenuWindow::Draw();
		_textbox.Draw();
	}

} // namespace hoa_menu