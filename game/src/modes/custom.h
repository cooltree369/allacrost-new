///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    custom.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for custom game mode
*** **************************************************************************/

#ifndef __CUSTOM_HEADER__
#define __CUSTOM_HEADER__

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"
#include "script.h"

//! All code in custom mode are wrapped in this namespace
namespace hoa_custom {

/** ****************************************************************************
*** \brief A bare-bones mode that is almost completely implemented in a Lua script file
***
*** Custom modes are usually built for simple, short scenes such as displaying a background
*** graphic or text on a black screen. But as their name implies, custom modes are flexible
*** and can be designed to implement any sort of functionality desired. The scripts for custom
*** game mode implementations are usually found in lua/scripts/custom/.
***
*** \note There are several important things to keep in mind when implementing the Lua code for a custom mode:
*** - The video engine's coordinate system should be set appropriately on every call to Reset()
*** - User input needs to be handled in the Update() call. Otherwise expected changes such as quit and pause inputs by
***   the user will be ignored.
*** - If the script pushes a new mode to the game stack (via _Termiante() or other means), make sure that the binding
***   code that creates the GameMode object passes ownership to C++, so that the Lua garbage collector doesn't destroy
***   the object when the script file is closed
*** ***************************************************************************/
class CustomMode : public hoa_mode_manager::GameMode {
	friend void hoa_defs::BindModeCode();

public:
	/** \param script_filename The name of the Lua file that implements this custom mode
	*** \param options A string that may be used by the script functions to determine specific behaviors
	**/
	CustomMode(const std::string& script_filename, const std::string& options = "");

	~CustomMode();

	//! \brief Executs the _reset_function. Called whenever CustomMode is made the active game mode
	void Reset();

	//! \brief Executes the _update_function to process game logic
	void Update();

	//! \brief Executes the _draw_function to draw elements to the screen
	void Draw();

private:
	/** \brief Initially false, this member will get set to true after the first call to Reset() is completed
	*** The purpose of this member is to make sure that data that needs to be loaded by the class is done so only once.
	*** It is not modified any other time, and is exposed in Lua as a read-only member.
	**/
	bool _load_complete;

	/** \brief A string used to set specific options or state desired behaviors from the script
	*** This string could be used as simple text to display to the screen, or as a '`' delimited list of options that
	*** set different data or change the flow of execution in any of the script functions. How this string is used is
	*** completely up to the script, and it is made available to Lua in a read-only format.
	**/
	std::string _options;

	//! \brief The Lua file controlling this instance of CustomMode. The file remains open throughout the life of the class instance
	hoa_script::ReadScriptDescriptor _script_file;

	//! \brief A script function called whenever Reset() is invoked
	ScriptObject _reset_function;

	//! \brief A script function called whenever Update() is invoked
	ScriptObject _update_function;

	//! \brief A script function called whenever Draw() is invoked
	ScriptObject _draw_function;

	/** \brief Pops this instance from the top of the game mode stack and optionally pushes a new game mode to the stack
	*** \param new_mode A pointer to a GameMode object to push to the stack. If NULL, nothing will be pushed to the stack
	**/
	void _Terminate(GameMode* new_mode);
}; // class CustomMode : public hoa_mode_manager::GameMode

} // namespace hoa_custom

#endif // __CUSTOM_HEADER__