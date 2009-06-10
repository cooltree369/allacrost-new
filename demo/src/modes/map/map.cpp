///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode interface.
*** ***************************************************************************/

// Allacrost engines
#include "audio.h"
#include "script.h"
#include "input.h"
#include "system.h"

// Allacrost globals
#include "global.h"

// Other mode headers
#include "menu.h"
#include "pause.h"

// Local map mode headers
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_tiles.h"
#include "map_treasure.h"
#include "map_zones.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_menu;
using namespace hoa_pause;
using namespace hoa_map::private_map;

namespace hoa_map {

// Initialize static class variables
MapMode *MapMode::_current_map = NULL;
bool MapMode::_show_dialogue_icons = true;

// ****************************************************************************
// ********** MapMode Class Functions
// ****************************************************************************

MapMode::MapMode(string filename) :
	GameMode(),
	_map_filename(filename),
	_map_tablespace(""), // will be properly initialized in the Load() function
	_map_event_group(NULL),
	_tile_supervisor(NULL),
	_object_supervisor(NULL),
	_dialogue_supervisor(NULL),
	_event_supervisor(NULL),
	_treasure_supervisor(NULL),
	_num_map_contexts(0),
	_current_context(MAP_CONTEXT_01),
	_run_stamina(10000),
	_ignore_input(false),
	_run_forever(false),
	_run_disabled(false),
	_camera(NULL)
{
	mode_type = MODE_MANAGER_MAP_MODE;
	_current_map = this;

	_ResetState();
	_PushState(STATE_EXPLORE);

	// Create the event group name by modifying the filename to consists only of alphanumeric characters and underscores
	// This will make it a valid identifier name in Lua syntax
	string event_group_name = _map_filename;
	std::replace(event_group_name.begin(), event_group_name.end(), '/', '_');
	std::replace(event_group_name.begin(), event_group_name.end(), '.', '_');

	if (GlobalManager->DoesEventGroupExist(event_group_name) == false) {
		GlobalManager->AddNewEventGroup(event_group_name);
	}
	_map_event_group = GlobalManager->GetEventGroup(event_group_name);

	_tile_supervisor = new TileSupervisor();
	_object_supervisor = new ObjectSupervisor();
	_dialogue_supervisor = new DialogueSupervisor();
	_event_supervisor = new EventSupervisor();
	_treasure_supervisor = new TreasureSupervisor();

	_intro_timer.Initialize(7000, 0, this);

	// TODO: Load the map data in a seperate thread
	_Load();

	// Load miscellaneous map graphics
	vector<uint32> timings(16, 100); // holds the timing data for the new dialogue animation; 16 frames at 100ms each
	_new_dialogue_icon.SetDimensions(2, 2);
	if (_new_dialogue_icon.LoadFromFrameSize("img/misc/dialogue_icon.png", timings, 32, 32) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the new dialogue icon image" << endl;

	if (_stamina_bar_background.Load("img/misc/stamina_bar_background.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the the stamina bar background image" << endl;

	if (_stamina_bar_infinite_overlay.Load("img/misc/stamina_bar_infinite_overlay.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load the the stamina bar infinite overlay image" << endl;
}



MapMode::~MapMode() {
	for (uint32 i = 0; i < _music.size(); i++)
		_music[i].FreeAudio();
	_music.clear();

	for (uint32 i = 0; i < _sounds.size(); i++)
		_sounds[i].FreeAudio();
	_sounds.clear();

	for (uint32 i = 0; i < _enemies.size(); i++)
		delete(_enemies[i]);
	_enemies.clear();

	delete(_tile_supervisor);
	delete(_object_supervisor);
	delete(_dialogue_supervisor);
	delete(_event_supervisor);
	delete(_treasure_supervisor);

	_map_script.CloseFile();
}



void MapMode::Reset() {
	// Reset video engine context properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	// Let all other map classes know that this is now the active map
	MapMode::_current_map = this;

	// Make the map location known globally to other code that may need to know this information
	GlobalManager->SetLocation(MakeUnicodeString(_map_filename), _location_graphic.GetFilename());

	// TEMP: This will need to be scripted later
	if (_music.size() > 0 && _music.back().GetState() != AUDIO_STATE_PLAYING) {
		_music.back().Play();
	}

	_intro_timer.Run();
}



void MapMode::Update() {
	// TODO: we need to detect if a battle is about to occur and if so, fade the screen gradually from
	// map mode into the battle

	// TODO: instead of doing this every frame, see if it can be done only when the _camera pointer is modified
	_current_context = _camera->GetContext();

	// Process quit and pause events unconditional to the state of map mode
	if (InputManager->QuitPress() == true) {
		ModeManager->Push(new PauseMode(true));
		return;
	}
	else if (InputManager->PausePress() == true) {
		ModeManager->Push(new PauseMode(false));
		return;
	}


	// ---------- (1) Call the map script's update function
	if (_update_function)
		ScriptCallFunction<void>(_update_function);

	// ---------- (3) Update all animated tile images
	_tile_supervisor->Update();
	_object_supervisor->Update();
	_object_supervisor->SortObjects();

	switch (_CurrentState()) {
		case STATE_EXPLORE:
			_UpdateExplore();
			break;
		case STATE_SCENE:
			break;
		case STATE_DIALOGUE:
			_dialogue_supervisor->Update();
			break;
		case STATE_TREASURE:
			_treasure_supervisor->Update();
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "map was set in an unknown state: " << _CurrentState() << endl;
			_ResetState();
			break;
	}

	// ---------- (5) Update all active map events
	_event_supervisor->Update();
} // void MapMode::Update()



void MapMode::Draw() {
	_CalculateMapFrame();
	if (_draw_function)
		ScriptCallFunction<void>(_draw_function);
	else
		_DrawMapLayers();
	_DrawGUI();
	if (_CurrentState() == STATE_DIALOGUE) {
		_dialogue_supervisor->Draw();
	}
} // void MapMode::_Draw()



void MapMode::_ResetState() {
	_state_stack.clear();
	_state_stack.push_back(STATE_INVALID);
}



void MapMode::_PushState(MAP_STATE state) {
	_state_stack.push_back(state);
}



void MapMode::_PopState() {
	_state_stack.pop_back();
	if (_state_stack.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "stack was empty after operation, reseting state stack" << endl;
		_state_stack.push_back(STATE_INVALID);
	}
}



MAP_STATE MapMode::_CurrentState() {
	if (_state_stack.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "stack was empty, reseting state stack" << endl;
		_state_stack.push_back(STATE_INVALID);
	}
	return _state_stack.back();
}



void MapMode::_Load() {
	// ---------- (1) Open map script file and read in the basic map properties and tile definitions
	if (_map_script.OpenFile(_map_filename) == false) {
		return;
	}

	// Determine the map's tablespacename and then open it. The tablespace is the name of the map file without
	// file extension or path information (for example, 'dat/maps/demo.lua' has a tablespace name of 'demo').
	int32 period = _map_filename.find(".");
	int32 last_slash = _map_filename.find_last_of("/");
	_map_tablespace = _map_filename.substr(last_slash + 1, period - (last_slash + 1));
	_map_script.OpenTable(_map_tablespace);

	// Read the number of map contexts, the name of the map, and load the location graphic image
	_num_map_contexts = _map_script.ReadUInt("num_map_contexts");
	_map_name = MakeUnicodeString(_map_script.ReadString("map_name"));
	if (_location_graphic.Load("img/menus/locations/" + _map_script.ReadString("location_filename")) == false) {
		PRINT_ERROR << "failed to load location graphic image: " << _location_graphic.GetFilename() << endl;
	}

	// ---------- (2) Instruct the supervisor classes to perform their portion of the load operation
	_tile_supervisor->Load(_map_script, this);
	_object_supervisor->Load(_map_script);

	// ---------- (3) Load map sounds and music
	vector<string> sound_filenames;
	_map_script.ReadStringVector("sound_filenames", sound_filenames);

	for (uint32 i = 0; i < sound_filenames.size(); i++) {
		_sounds.push_back(SoundDescriptor());
		if (_sounds.back().LoadAudio(sound_filenames[i]) == false) {
			PRINT_ERROR << "failed to load map sound: " << sound_filenames[i] << endl;
			return;
		}
	}

	vector<string> music_filenames;
	_map_script.ReadStringVector("music_filenames", music_filenames);
	for (uint32 i = 0; i < music_filenames.size(); i++) {
		_music.push_back(MusicDescriptor());
		if (_music.back().LoadAudio(music_filenames[i]) == false) {
			PRINT_ERROR << "failed to load map music: " << music_filenames[i] << endl;
			return;
		}
	}

	// ---------- (4) Create and store all enemies that may appear on this map
	vector<int32> enemy_ids;
	_map_script.ReadIntVector("enemy_ids", enemy_ids);
	for (uint32 i = 0; i < enemy_ids.size(); i++) {
		_enemies.push_back(new GlobalEnemy(enemy_ids[i]));
	}

	// ---------- (5) Call the map script's custom load function and get a reference to all other script function pointers
	ScriptObject map_table(luabind::from_stack(_map_script.GetLuaState(), hoa_script::private_script::STACK_TOP));
	ScriptObject function = map_table["Load"];
	ScriptCallFunction<void>(function, this, _dialogue_supervisor);

	_update_function = _map_script.ReadFunctionPointer("Update");
	_draw_function = _map_script.ReadFunctionPointer("Draw");

	// ---------- (6) Prepare all sprites with dialogue
	// This is done at this stage because the map script's load function creates the sprite and dialogue objects. Only after
	// both sets are created can we determine which sprites have active dialogue.

	// TODO: Need to figure out a new function appropriate for this code?
	// TEMP: The line below is very bad to do, but is necessary for the UpdateDialogueStatus function to work correctly
	_current_map = this;
	for (map<uint16, MapObject*>::iterator i = _object_supervisor->_all_objects.begin(); i != _object_supervisor->_all_objects.end(); i++) {
		if (i->second->GetType() == SPRITE_TYPE) {
			MapSprite* sprite = dynamic_cast<MapSprite*>(i->second);
			sprite->UpdateDialogueStatus();
		}
	}

	_map_script.CloseAllTables();
} // void MapMode::_Load()



void MapMode::_UpdateExplore() {
	// First go to menu mode if the user requested it
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode(_map_name, _location_graphic.GetFilename());
		ModeManager->Push(MM);
		return;
	}

	// Update the running state of the camera object. Check if the player wishes to continue running and if so,
	// update the stamina value if the operation is permitted
	_camera->is_running = false;
	if (_run_disabled == false && InputManager->CancelState() == true &&
		(InputManager->UpState() || InputManager->DownState() || InputManager->LeftState() || InputManager->RightState()))
	{
		if (_run_forever) {
			_camera->is_running = true;
		}
		else if (_run_stamina > SystemManager->GetUpdateTime() * 2) {
			_run_stamina -= (SystemManager->GetUpdateTime() * 2);
			_camera->is_running = true;
		}
		else {
			_run_stamina = 0;
		}
	}
	// Regenerate the stamina at 1/2 the consumption rate
	else if (_run_stamina < 10000) {
		_run_stamina += SystemManager->GetUpdateTime();
		if (_run_stamina > 10000)
			_run_stamina = 10000;
	}

	// If the user requested a confirm event, check if there is a nearby object that the player may interact with
	// Interactions are currently limited to dialogue with sprites and opening of treasures
	if (InputManager->ConfirmPress()) {
		MapObject* obj = _object_supervisor->FindNearestObject(_camera);

		if (obj && (obj->GetType() == SPRITE_TYPE)) {
			MapSprite *sp = reinterpret_cast<MapSprite*>(obj);

			if (sp->HasAvailableDialogue()) {
				_dialogue_supervisor->BeginDialogue(sp);
				return;
			}
		}
		else if (obj && obj->GetType() == TREASURE_TYPE) {
			MapTreasure* treasure = reinterpret_cast<MapTreasure*>(obj);

			if (treasure->IsEmpty() == false) {
				treasure->Open();
			}
		}
	}

	// Detect movement input from the user
	if (InputManager->UpState() || InputManager->DownState() || InputManager->LeftState() || InputManager->RightState()) {
		_camera->moving = true;
	}
	else {
		_camera->moving = false;
	}

	// Determine the direction of movement. Priority of movement is given to: up, down, left, right.
	// In the case of diagonal movement, the direction that the sprite should face also needs to be deduced.
	if (_camera->moving == true) {
		if (InputManager->UpState())
		{
			if (InputManager->LeftState())
				_camera->SetDirection(MOVING_NORTHWEST);
			else if (InputManager->RightState())
				_camera->SetDirection(MOVING_NORTHEAST);
			else
				_camera->SetDirection(NORTH);
		}
		else if (InputManager->DownState())
		{
			if (InputManager->LeftState())
				_camera->SetDirection(MOVING_SOUTHWEST);
			else if (InputManager->RightState())
				_camera->SetDirection(MOVING_SOUTHEAST);
			else
				_camera->SetDirection(SOUTH);
		}
		else if (InputManager->LeftState()) {
			_camera->SetDirection(WEST);
		}
		else if (InputManager->RightState()) {
			_camera->SetDirection(EAST);
		}
	} // if (_camera->moving == true)
} // void MapMode::_HandleInputExplore()



void MapMode::_CalculateMapFrame() {
	// ---------- (1) Determine the center position coordinates for the camera
	float camera_x, camera_y; // Holds the final X, Y coordinates of the camera
	float x_pixel_length, y_pixel_length; // The X and Y length values that coorespond to a single pixel in the current coodinate system
	float rounded_x_offset, rounded_y_offset; // The X and Y position offsets of the camera, rounded to perfectly align on a pixel boundary

	// TODO: the call to GetPixelSize() will return the same result every time so long as the coordinate system did not change. If we never
	// change the coordinate system in map mode, then this should be done only once and the calculated values should be saved for re-use.
	// However, we've discussed the possiblity of adding a zoom feature to maps, in which case we need to continually re-calculate the pixel size
	VideoManager->GetPixelSize(x_pixel_length, y_pixel_length);
	rounded_x_offset = FloorToFloatMultiple(_camera->x_offset, x_pixel_length);
	rounded_y_offset = FloorToFloatMultiple(_camera->y_offset, y_pixel_length);
	camera_x = static_cast<float>(_camera->x_position) + rounded_x_offset;
	camera_y = static_cast<float>(_camera->y_position) + rounded_y_offset;

	// ---------- (2) Calculate all four screen edges and determine
	// Determine the draw coordinates of the top left corner using the camera's current position
	_draw_info.tile_x_start = 1.0f - rounded_x_offset;
	if (IsOddNumber(_camera->x_position))
		_draw_info.tile_x_start -= 1.0f;

	_draw_info.tile_y_start = 2.0f - rounded_y_offset;
	if (IsOddNumber(_camera->y_position))
		_draw_info.tile_y_start -= 1.0f;

	// The starting  row and column of tiles to draw is determined by the map camera's position
	_draw_info.starting_col = (_camera->x_position / 2) - HALF_TILE_COLS;
	_draw_info.starting_row = (_camera->y_position / 2) - HALF_TILE_ROWS;

	_draw_info.screen_edges.top    = camera_y - HALF_SCREEN_ROWS;
	_draw_info.screen_edges.bottom = camera_y + HALF_SCREEN_ROWS;
	_draw_info.screen_edges.left   = camera_x - HALF_SCREEN_COLS;
	_draw_info.screen_edges.right  = camera_x + HALF_SCREEN_COLS;

	// ---------- (3) Check for boundary conditions and re-adjust as necessary so we don't draw outside the map area

	// Usually the map centers on the camera's position, but when the camera becomes too close to
	// the edges of the map, we need to modify the drawing properties of the frame.

	// Camera exceeds the left boundary of the map
	if (_draw_info.starting_col < 0) {
		_draw_info.starting_col = 0;
		_draw_info.tile_x_start = 1.0f;
		_draw_info.screen_edges.left = 0.0f;
		_draw_info.screen_edges.right = SCREEN_COLS;
	}
	// Camera exceeds the right boundary of the map
	else if (_draw_info.starting_col + TILE_COLS >= _tile_supervisor->_num_tile_cols) {
		_draw_info.starting_col = static_cast<int16>(_tile_supervisor->_num_tile_cols - TILE_COLS);
		_draw_info.tile_x_start = 1.0f;
		_draw_info.screen_edges.right = static_cast<float>(_object_supervisor->_num_grid_cols);
		_draw_info.screen_edges.left = _draw_info.screen_edges.right - SCREEN_COLS;
	}

	// Camera exceeds the top boundary of the map
	if (_draw_info.starting_row < 0) {
		_draw_info.starting_row = 0;
		_draw_info.tile_y_start = 2.0f;
		_draw_info.screen_edges.top = 0.0f;
		_draw_info.screen_edges.bottom = SCREEN_ROWS;
	}
	// Camera exceeds the bottom boundary of the map
	else if (_draw_info.starting_row + TILE_ROWS >= _tile_supervisor->_num_tile_rows) {
		_draw_info.starting_row = static_cast<int16>(_tile_supervisor->_num_tile_rows - TILE_ROWS);
		_draw_info.tile_y_start = 2.0f;
		_draw_info.screen_edges.bottom = static_cast<float>(_object_supervisor->_num_grid_rows);
		_draw_info.screen_edges.top = _draw_info.screen_edges.bottom - SCREEN_ROWS;
	}

	// ---------- (4) Determine the number of rows and columns of tiles that need to be drawn

	// When the tile images align perfectly with the screen, we can afford to draw one less row or column of tiles
	if (IsFloatInRange(_draw_info.tile_x_start, 0.999f, 1.001f)) {
		_draw_info.num_draw_cols = TILE_COLS;
	}
	else {
		_draw_info.num_draw_cols = TILE_COLS + 1;
	}
	if (IsFloatInRange(_draw_info.tile_y_start, 1.999f, 2.001f)) {
		_draw_info.num_draw_rows = TILE_ROWS;
	}
	else {
		_draw_info.num_draw_rows = TILE_ROWS + 1;
	}

	// Comment this out to print out debugging info about each map frame that is drawn
// 	printf("--- MAP DRAW INFO ---\n");
// 	printf("Starting row, col: [%d, %d]\n", _draw_info.starting_row, _draw_info.starting_col);
// 	printf("# draw rows, cols: [%d, %d]\n", _draw_info.num_draw_rows, _draw_info.num_draw_cols);
// 	printf("Camera position:   [%f, %f]\n", camera_x, camera_y);
// 	printf("Tile draw start:   [%f, %f]\n", _draw_info.tile_x_start, _draw_info.tile_y_start);
// 	printf("Edges (T,B,L,R):   [%f, %f, %f, %f]\n", _draw_info.screen_edges.top, _draw_info.screen_edges.bottom,
// 		_draw_info.screen_edges.left, _draw_info.screen_edges.right);
} // void MapMode::_CalculateMapFrame()



void MapMode::_DrawMapLayers() {
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);

	_tile_supervisor->DrawLowerLayer(&_draw_info);
	_tile_supervisor->DrawMiddleLayer(&_draw_info);

	_object_supervisor->DrawGroundObjects(&_draw_info, false); // First draw pass of ground objects
	_object_supervisor->DrawPassObjects(&_draw_info);
	_object_supervisor->DrawGroundObjects(&_draw_info, true); // Second draw pass of ground objects

	_tile_supervisor->DrawUpperLayer(&_draw_info);

	_object_supervisor->DrawSkyObjects(&_draw_info);
} // void MapMode::_DrawMapLayers()



void MapMode::_DrawGUI() {
	// TODO: figure out what this color represents and create an approximate name for it
	const Color unknown(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color lighter_green(0.419f, 0.894f, 0.0f, 1.0f);
	const Color light_green(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color medium_green(0.0509f, 0.556f, 0.0509f, 1.0f);
	const Color darkish_green(0.352f, 0.4f, 0.352f, 1.0f);
	const Color dark_green(0.0196f, 0.207f, 0.0196f, 1.0f);
	const Color bright_yellow(0.937f, 1.0f, 0.725f, 1.0f);

	// ---------- (1) Draw the introductory location name and graphic if necessary
	if (_intro_timer.IsFinished() == false) {
		uint32 time = _intro_timer.GetTimeExpired();

		Color blend(1.0f, 1.0f, 1.0f, 1.0f);
		if (time < 2000) { // Fade in
			blend.SetAlpha((static_cast<float>(time) / 2000.0f));
		}
		else if (time > 5000) { // Fade out
			blend.SetAlpha(1.0f - static_cast<float>(time - 5000) / 2000.0f);
		}

		VideoManager->PushState();
		VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		VideoManager->Move(512.0f, 100.0f);
		_location_graphic.Draw(blend);
		VideoManager->MoveRelative(0.0f, -80.0f);
		VideoManager->Text()->Draw(_map_name, TextStyle("map", blend, VIDEO_TEXT_SHADOW_DARK));
		VideoManager->PopState();
	}

	// ---------- (2) Draw the stamina bar in the lower right corner
	// TODO: the code in this section needs better comments to explain what each coloring step is doing
	float fill_size = static_cast<float>(_run_stamina) / 10000.0f;

	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	// Draw the background image
	VideoManager->Move(780, 747);
	_stamina_bar_background.Draw();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);

	// Draw the base color of the bar
	VideoManager->Move(800, 740);
	VideoManager->DrawRectangle(200 * fill_size, 10, unknown);

	// Shade the bar with a faux lighting effect
	VideoManager->Move(800,739);
	VideoManager->DrawRectangle(200 * fill_size, 2, dark_green);
	VideoManager->Move(800, 737);
	VideoManager->DrawRectangle(200 * fill_size, 7, darkish_green);

	// Only do this if the bar is at least 4 pixels long
	if ((200 * fill_size) >= 4) {
		VideoManager->Move(801, 739);
		VideoManager->DrawRectangle((200 * fill_size) -2, 1, darkish_green);

		VideoManager->Move(801, 738);
		VideoManager->DrawRectangle(1, 2, medium_green);
		VideoManager->Move(800 + (fill_size * 200 - 2), 738); // Automatically reposition to be at moving endcap
		VideoManager->DrawRectangle(1, 2, medium_green);
	}

	VideoManager->Move(800, 736);
	VideoManager->DrawRectangle(200 * fill_size, 5, medium_green);

	// Only do this if the bar is at least 4 pixels long
	if ((200 * fill_size) >= 4) {
		VideoManager->Move(801, 735);
		VideoManager->DrawRectangle(1, 1, lighter_green);
		VideoManager->Move(800 + (fill_size * 200 - 2), 735); // automatically reposition to be at moving endcap
		VideoManager->DrawRectangle(1, 1, lighter_green);
		VideoManager->Move(800, 734);
		VideoManager->DrawRectangle(200 * fill_size, 2, lighter_green);
	}

	// Only do this if the bar is at least 6 pixels long
	if ((200 * fill_size) >= 6) {
		VideoManager->Move(802, 733);
		VideoManager->DrawRectangle((200 * fill_size) - 4, 1, bright_yellow);
	}

	if (_run_forever) { // Draw the infinity symbol over the stamina bar
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Move(780, 747);
		_stamina_bar_infinite_overlay.Draw();
	}

	VideoManager->PopState();

	// ---------- (3) Draw the treasure menu if necessary
	if (_treasure_supervisor->IsActive() == true)
		_treasure_supervisor->Draw();
} // void MapMode::_DrawGUI()



void MapMode::_AddGroundObject(MapObject *obj) {
	_object_supervisor->_ground_objects.push_back(obj);
	_object_supervisor->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddPassObject(MapObject *obj) {
	_object_supervisor->_pass_objects.push_back(obj);
	_object_supervisor->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddSkyObject(MapObject *obj) {
	_object_supervisor->_sky_objects.push_back(obj);
	_object_supervisor->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddZone(MapZone *zone) {
	_object_supervisor->_zones.push_back(zone);
}



uint16 MapMode::_GetGeneratedObjectID() {
	return ++(_object_supervisor->_last_id);
}

} // namespace hoa_map

