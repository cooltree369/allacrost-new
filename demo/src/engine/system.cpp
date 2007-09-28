////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   system.cpp
*** \author Tyler Olsen, roots@allacrost.org
*** \author Andy Gardner, chopperdave@allacrost.org
*** \brief  Source file for system code management
*** ***************************************************************************/

#include "system.h"
#include "audio.h"
#include "script.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_script;
using namespace hoa_mode_manager;

template<> hoa_system::GameSystem* Singleton<hoa_system::GameSystem>::_singleton_reference = NULL;

namespace hoa_system {

GameSystem* SystemManager = NULL;
bool SYSTEM_DEBUG = false;

// -----------------------------------------------------------------------------
// SystemTimer Class
// -----------------------------------------------------------------------------

SystemTimer::SystemTimer() :
	_state(SYSTEM_TIMER_INVALID),
	_duration(0),
	_number_loops(0),
	_mode_owner(NULL),
	_time_expired(0),
	_times_completed(0)
{}



SystemTimer::~SystemTimer() {
	// If the timer is still in the invalid state, the SystemManager never received a reference to it.
	if (_state == SYSTEM_TIMER_INVALID)
		return;

	// Remove the reference to this timer object from the SystemManager
	SystemManager->_system_timers.erase(this);
}



void SystemTimer::Initialize(uint32 duration, int32 number_loops, hoa_mode_manager::GameMode* mode_owner) {
	// If the state is invalid, this is the first time that this timer has been initialized and we need to pass it
	// along to the SystemManager
	if (_state == SYSTEM_TIMER_INVALID) {
		SystemManager->_system_timers.insert(this);
	}

	_duration = duration;
	_number_loops = number_loops;
	_mode_owner = mode_owner;

	_state = SYSTEM_TIMER_INITIAL;
	_time_expired = 0;
	_times_completed = 0;
}



void SystemTimer::SetDuration(uint32 duration) {
	if (IsInitial()) {
		_duration = duration;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetDuration() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::SetNumberLoops(int32 number_loops) {
	if (IsInitial()) {
		_number_loops = number_loops;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetNumberLoops() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::SetModeOwner(hoa_mode_manager::GameMode* mode_owner) {
	if (IsInitial()) {
		_mode_owner = mode_owner;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetModeOwner() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::_UpdateTimer() {
	if (IsRunning() == false)
		return;

	_time_expired += SystemManager->GetUpdateTime();

	if (_time_expired >= _duration) {
		_times_completed++;

		// Checks if infinite looping is enabled
		if (_number_loops < 0) { 
			_time_expired -= _duration;
		}
		// Checks if the number of loops have expired
		else if (_times_completed >= static_cast<uint32>(_number_loops)) {
			_time_expired = 0;
			_state = SYSTEM_TIMER_FINISHED;
		}
		// Otherwise, there are still additional loops to complete
		else {
			_time_expired -= _duration;
		}
	}
}

// -----------------------------------------------------------------------------
// GameSystem Class
// -----------------------------------------------------------------------------

GameSystem::GameSystem() {
	if (SYSTEM_DEBUG)
		cout << "SETTINGS: GameSystem constructor invoked" << endl;

	_not_done = true;
	_language = "en"; // Default language is English
}



GameSystem::~GameSystem() {
	if (SYSTEM_DEBUG)
		cout << "SETTINGS: GameSystem destructor invoked" << endl;
}



bool GameSystem::SingletonInitialize() {
	// TODO: Initialize the gettext library
// 	setlocale(LC_ALL, "");
// 	bindtextdomain(PACKAGE, DATADIR);
// 	textdomain(PACKAGE);

	ReadScriptDescriptor settings_data;

	if (settings_data.OpenFile("dat/config/settings.lua") == false) {
		cerr << "SYSTEM ERROR: failed to load settings from data file" << endl;
		return false;
	}

	settings_data.OpenTable("settings");
	settings_data.OpenTable("video_settings");
// 	SetFullScreen(settings_data.ReadBool("full_screen"));
	settings_data.CloseTable();

	settings_data.OpenTable("audio_settings");
	AudioManager->SetMusicVolume(settings_data.ReadFloat("music_vol"));
	AudioManager->SetSoundVolume(settings_data.ReadFloat("sound_vol"));
	settings_data.CloseTable();
	settings_data.CloseTable();

	if (settings_data.IsErrorDetected()) {
		cerr << "SETTINGS WARNING: some errors occured during read operations from data file:" << endl;
		cerr << settings_data.GetErrorMessages() << endl;
	}
	settings_data.CloseFile();
	return true;
}


// Set up the timers before the main game loop begins
void GameSystem::InitializeTimers() {
	_last_update = SDL_GetTicks();
	_update_time = 1; // Set to non-zero, otherwise bad things may happen...
	_hours_played = 0;
	_minutes_played = 0;
	_seconds_played = 0;
	_milliseconds_played = 0;
	_system_timers.clear();
}



void GameSystem::UpdateTimers() {
	uint32 tmp;

	// ----- (1): Update the simple game timer
	tmp = _last_update;
	_last_update = SDL_GetTicks();
	_update_time = _last_update - tmp;

	// ----- (2): Update the game play timer
	_milliseconds_played += _update_time;
	if (_milliseconds_played >= 1000) {
		_seconds_played += _milliseconds_played / 1000;
		_milliseconds_played = _milliseconds_played % 1000;
		if (_seconds_played >= 60) {
			_minutes_played += _seconds_played / 60;
			_seconds_played = _seconds_played % 60;
			if (_minutes_played >= 60) {
				_hours_played += _minutes_played / 60;
				_minutes_played = _minutes_played % 60;
			}
		}
	}

	// ----- (3): Update all SystemTimer objects
	for (set<SystemTimer*>::iterator i = _system_timers.begin(); i != _system_timers.end(); i++)
		(*i)->_UpdateTimer();
}



void GameSystem::ExamineSystemTimers() {
	GameMode* active_mode = ModeManager->GetTop();
	GameMode* timer_mode = NULL;

	for (set<SystemTimer*>::iterator i = _system_timers.begin(); i != _system_timers.end(); i++) {
		timer_mode = (*i)->GetModeOwner();
		if (timer_mode == NULL)
			continue;
		
		if (timer_mode == active_mode)
			(*i)->Run();
		else
			(*i)->Pause();
	}
}



void GameSystem::SetLanguage(std::string lang) {
	// A 2 character string is the only allowable argument
	if (lang.size() != 2) {
		return;
	}
// 	for (uint32 i = 0; i < SUPPORTED_LANGUAGES.size(); i++) {
// 		if (lang == SUPPORTED_LANGUAGES[i]) {
// 			_language = lang;
// 			return;
// 		}
// 	}
//
// 	cerr << "SETTINGS ERROR: attempt to set unsupported language \"" << lang << "\" failed" << endl;
	_language = lang;
}

} // namespace hoa_system
