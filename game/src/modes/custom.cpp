///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    custom.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for custom mode code
*** **************************************************************************/

#include "custom.h"

#include "mode_manager.h"
#include "script.h"

using namespace std;
using namespace hoa_utils;

using namespace hoa_mode_manager;
using namespace hoa_script;

namespace hoa_custom {

CustomMode::CustomMode(const std::string& script_filename, const std::string& options) :
	GameMode(MODE_MANAGER_CUSTOM_MODE),
	_load_complete(false),
	_options(options)
{
	if (_script_file.OpenFile(script_filename) == false) {
		PRINT_ERROR << "Failed to open custom mode script file: " << script_filename << endl;
		return;
	}
	_update_function = _script_file.ReadFunctionPointer("Reset");
	_update_function = _script_file.ReadFunctionPointer("Update");
	_update_function = _script_file.ReadFunctionPointer("Draw");
}



CustomMode::~CustomMode() {
	_script_file.CloseFile();
}



void CustomMode::Reset() {
	_script_file.ExecuteFunction(_reset_function);
	_load_complete = true;
}



void CustomMode::Update() {
	_script_file.ExecuteFunction(_update_function);
}



void CustomMode::Draw() {
	_script_file.ExecuteFunction(_draw_function);
}



void CustomMode::_Terminate(GameMode* new_mode) {
	ModeManager->Pop();
	if (new_mode != NULL) {
		ModeManager->Push(new_mode);
	}
}

} // namespace hoa_custom
