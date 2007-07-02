///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script_modify.cpp
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org
*** \brief   Source file for the ModifyScriptDescriptor class.
*** ***************************************************************************/

#include "script.h"
#include "script_modify.h"
#include "script_read.h"
#include "script_write.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;

namespace hoa_script {

ModifyScriptDescriptor::~ModifyScriptDescriptor() {
	if (IsFileOpen()) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ModifyScriptDescriptor destructor was called when file was still open: "
				<< _filename << endl;
		CloseFile();
	}
	
	_filename = "";
	_access_mode = SCRIPT_CLOSED;
	_error_messages.clear();
	_open_tables.clear();
}

//-----------------------------------------------------------------------------
// File Access Functions
//-----------------------------------------------------------------------------

bool ModifyScriptDescriptor::OpenFile(const std::string& file_name) {
	if (ScriptManager->IsFileOpen(file_name) == true) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ModifyScriptDescriptor::OpenFile() attempted to open file that is already opened: "
				<< file_name << endl;
		return false;
	}

	// Increases the global stack size by 1 element. That is needed because the new thread will be pushed in the
	// stack and we have to be sure there is enough space there.
	lua_checkstack(ScriptManager->GetGlobalState(),1);
	_lstack = lua_newthread(ScriptManager->GetGlobalState());

	// Attempt to load the Lua file.
	if (lua_dofile(_lstack, file_name.c_str()) != 0) {
		cerr << "SCRIPT ERROR: ModifyScriptDescriptor::OpenFile() could not open the file " << file_name << endl;
		_access_mode = SCRIPT_CLOSED;
		return false;
	}

	_filename = file_name;
	_access_mode = SCRIPT_READ;
	ScriptManager->_AddOpenFile(this);
	return true;
} // bool ModifyScriptDescriptor::OpenFile(std::string file_name)



bool ModifyScriptDescriptor::OpenFile() {
	if (_filename == "") {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ModifyScriptDescriptor::OpenFile(), could not open file "
				<< "because of an invalid file name (empty string)." << endl;
		return false;
	}

	return OpenFile(_filename);
}



void ModifyScriptDescriptor::CloseFile() {
	if (IsFileOpen() == false) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ModifyScriptDescriptor::CloseFile() could not close the "
				<< "file because it was not open." << endl;
		return;
	}

	if (SCRIPT_DEBUG && IsErrorDetected()) {
		cerr << "SCRIPT WARNING: In ModifyScriptDescriptor::CloseFile(), the file " << _filename
			<< " had error messages remaining. They are as follows:" << endl;
		cerr << _error_messages.str() << endl;
	}

	_lstack = NULL;
	_error_messages.clear();
	_open_tables.clear();
	_access_mode = SCRIPT_CLOSED;
	ScriptManager->_RemoveOpenFile(this);
}

//-----------------------------------------------------------------------------
// Commit Function Definitions
//-----------------------------------------------------------------------------

void ModifyScriptDescriptor::CommitChanges(bool leave_closed) {
	WriteScriptDescriptor file; // The file to write the modified Lua state out to
	string temp_filename = "TEMP" + _filename;

	if (file.OpenFile(temp_filename) == false) {
		if (SCRIPT_DEBUG)
			_error_messages << "* ModifyScriptDescriptor::CommitChanges() failed because it could not open "
				<< "the file to write the modifications to" << endl;
		return;
	}

	// Write the global tables to the file. This in turn will write all other tables that are members of
	// the global tables, or members of those tables, and so on.
	object globals(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	_CommitTable(file, globals);

	file.CloseFile(); // Close the temporary file we were writing to
	CloseFile(); // Close this file as well as it is about to be over-written

	// Now overwrite this file with the temporary file written, remove the temporary file, and re-open the new file
	
	if (MoveFile(temp_filename, _filename) == false) {
		_error_messages << "* ModifyScriptDescriptor::CommitChanges() failed because after writing the temporary file "
			<< temp_filename << ", it could not be moved to overwrite the original filename " << _filename << endl;
	}

	if (leave_closed == false)
		OpenFile();
} // void ModifyScriptDescriptor::CommitChanges(bool leave_closed)



void ModifyScriptDescriptor::_CommitTable(WriteScriptDescriptor& file, const luabind::object& table) {
	bool key_is_numeric;  // Set to true when a variable's key is not a string
	int32 num_key = 0;    // Holds the current numeric key
	string str_key = "";  // Holds the current string key

	for (luabind::iterator it(table), end; it != end; ++it) {
		try {
			num_key = object_cast<int32>(it.key());
			key_is_numeric = true;
		} catch (...) {
			str_key = object_cast<string>(it.key());
			key_is_numeric = false;
		}
	
		switch (luabind::type(*it)) {
			case LUA_TBOOLEAN:
				if (key_is_numeric)
					file.WriteBool(num_key, object_cast<bool>(*it));
				else
					file.WriteBool(str_key, object_cast<bool>(*it));
				break;
			case LUA_TNUMBER:
				try {
					if (key_is_numeric)
						file.WriteInt(num_key, object_cast<int32>(*it));
					else
						file.WriteInt(str_key, object_cast<int32>(*it));
				} catch (...) {
					if (key_is_numeric)
						file.WriteFloat(num_key, object_cast<float>(*it));
					else
						file.WriteFloat(str_key, object_cast<float>(*it));
				}
				break;
			case LUA_TSTRING:
				if (key_is_numeric)
					file.WriteString(num_key, object_cast<string>(*it));
				else
					file.WriteString(str_key, object_cast<string>(*it));
				break;
			case LUA_TTABLE:
				if (key_is_numeric)
					file.BeginTable(num_key);
				else
					file.BeginTable(str_key);
				_CommitTable(file, object(*it));
				file.EndTable();
				break;
			case LUA_TNIL:
			case LUA_TFUNCTION:
			case LUA_TUSERDATA:
			case LUA_TLIGHTUSERDATA:
			case LUA_TTHREAD:
			default:
				if (SCRIPT_DEBUG) {
					_error_messages << "* ModifyScriptDescriptor::_CommitTable() detected a ";
					if (type(*it) == LUA_TNIL) {
						_error_messages << "nil value ";
					} else if (type(*it) == LUA_TFUNCTION) {
						_error_messages << "function ";
					} else if (type(*it) == LUA_TFUNCTION) {
						_error_messages << "function ";
					} else if (type(*it) == LUA_TUSERDATA) {
						_error_messages << "user data ";
					} else if (type(*it) == LUA_TLIGHTUSERDATA) {
						_error_messages << "light user data ";
					} else if (type(*it) == LUA_TTHREAD) {
						_error_messages << "thread ";
					} else {
						_error_messages << "unknown data type ";
					}
					
					if (key_is_numeric)
						_error_messages << "key: " << num_key;
					else
						_error_messages << str_key;
					_error_messages << ". It was not written to the modified file." << endl;
				}
				break;
		} // switch (luabind::type(*it))
	} // for (luabind::iterator it(table), end; it != end; ++it)
} // void ModifyScriptDescriptor::_CommitTable(WriteScriptDescriptor& file, luabind::object& table)

} // namespace hoa_script
