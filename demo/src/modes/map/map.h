///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in map mode (when the user is exploring town or dungeon maps). This
*** includes handling of tile images, sprites, and events that occur on the map.
***
*** Each individual map is represented by it's own object
*** of the MapMode class. At this time, the intention is to keep the three most
*** recently accessed maps in memory so there is no loading time when the player
*** backtraces his or her steps. When a new map is loaded and there are already
*** three
*** ***************************************************************************/

#ifndef __MAP_HEADER__
#define __MAP_HEADER__

#include "defs.h"
#include "utils.h"

#include "script.h"
#include "video.h"

#include "mode_manager.h"


//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! Determines whether the code in the hoa_map namespace should print debug statements or not.
extern bool MAP_DEBUG;

//! An internal namespace to be used only within the map code. Don't use this namespace anywhere else!
namespace private_map {

// ************************ MAP CONSTANTS ****************************

/** \name Screen Coordiante System Constants
*** \brief The number of rows and columns of map grid elements that compose the screen.
*** These are <b>not</b> the number of tiles that compose the screen. The number of tile
*** rows and columns that compose a screen are exactly one half of these numbers.
**/
//@{
const float SCREEN_COLS = 64.0f;
const float SCREEN_ROWS = 48.0f;
const float HALF_SCREEN_COLS = 32.0f;
const float HALF_SCREEN_ROWS = 24.0f;
const uint16 TILE_COLS = 32;
const uint16 TILE_ROWS = 24;
const uint16 HALF_TILE_COLS = 16;
const uint16 HALF_TILE_ROWS = 12;
//@}

/** \name Map State Constants
*** \brief Constants used for describing the current state of operation during map mode.
*** These constants are largely used to determine what
**/
//@{
//! \brief The standard state of the map, where the player is free to roam.
const uint8 EXPLORE      = 0x01;
//! \brief When a dialogue is in process, the map is in this state.
const uint8 DIALOGUE     = 0x02;
//! \brief When the map is in this state, the player can not control the action.
const uint8 OBSERVATION  = 0x04;
//@}

/** \name Map Context Constants
*** \brief Constants used to represent all 32 possible map contexts
**/
enum MAP_CONTEXT {
	MAP_CONTEXT_01 = 0x00000001,
	MAP_CONTEXT_02 = 0x00000002,
	MAP_CONTEXT_03 = 0x00000004,
	MAP_CONTEXT_04 = 0x00000008,
	MAP_CONTEXT_05 = 0x00000010,
	MAP_CONTEXT_06 = 0x00000020,
	MAP_CONTEXT_07 = 0x00000040,
	MAP_CONTEXT_08 = 0x00000080,
	MAP_CONTEXT_09 = 0x00000100,
	MAP_CONTEXT_10 = 0x00000200,
	MAP_CONTEXT_11 = 0x00000400,
	MAP_CONTEXT_12 = 0x00000800,
	MAP_CONTEXT_13 = 0x00001000,
	MAP_CONTEXT_14 = 0x00002000,
	MAP_CONTEXT_15 = 0x00004000,
	MAP_CONTEXT_16 = 0x00008000,
	MAP_CONTEXT_17 = 0x00010000,
	MAP_CONTEXT_18 = 0x00020000,
	MAP_CONTEXT_19 = 0x00040000,
	MAP_CONTEXT_20 = 0x00080000,
	MAP_CONTEXT_21 = 0x00100000,
	MAP_CONTEXT_22 = 0x00200000,
	MAP_CONTEXT_23 = 0x00400000,
	MAP_CONTEXT_24 = 0x00800000,
	MAP_CONTEXT_25 = 0x01000000,
	MAP_CONTEXT_26 = 0x02000000,
	MAP_CONTEXT_27 = 0x04000000,
	MAP_CONTEXT_28 = 0x08000000,
	MAP_CONTEXT_29 = 0x10000000,
	MAP_CONTEXT_30 = 0x20000000,
	MAP_CONTEXT_31 = 0x40000000,
	MAP_CONTEXT_32 = 0x80000000
};

/** ****************************************************************************
*** \brief Retains information about how the next map frame should be drawn.
***
*** This class is used by the MapMode class to determine how the next map frame
*** should be drawn. This includes which tiles will be visible and the offset
*** coordinates for the screen. Map objects also use this information to determine
*** where (and if) they should be drawn.
***
*** \note The MapMode class keeps an active object of this class with the latest
*** information about the map. It should be the only instance of this class that is
*** needed.
*** ***************************************************************************/
class MapFrame {
public:
	//! \brief The column and row indeces of the starting tile to draw (the top-left tile).
	int16 starting_col, starting_row;

	//! \brief The number of columns and rows of tiles to draw on the screen.
	uint8 num_draw_cols, num_draw_rows;

	//! \brief The x and y position screen coordinates to start drawing tiles from.
	float tile_x_start, tile_y_start;

	/** \brief The position coordinates of the screen edges.
	*** These members are in terms of the map grid 16x16 pixel coordinates that map objects use.
	*** The presense of these coordinates make it easier for map objects to figure out whether or
	*** not they should be drawn on the screen. Note that these are <b>not</b> used as drawing
	*** cursor positions, but rather are map grid coordinates indicating where the screen edges lie.
	**/
	float left_edge, right_edge, top_edge, bottom_edge;
}; // class MapFrame


/** ****************************************************************************
*** \brief A container class for node information in pathfinding.
***
*** This class is used in the MapMode#_FindPath function to find an optimal
*** path from a given source to a destination.
*** *****************************************************************************/
class PathNode {
public:
	/** \brief The coordinates for this node
	*** These coordinates correspond to the MapMode#_walkable 2D vector, where
	*** each element is a 16x16 pixel space on the map.
	**/
	//@{
	int16 row, col;
	//@}

	//! \name Path Scoring Members
	//@{
	//! \brief The total score for this node (f = g + h).
	int16 f_score;

	//! \brief The score for this node relative to the source.
	int16 g_score;

	//! \brief The Manhattan distance from this node to the destination.
	int16 h_score;
	//@}

	//PathNode *parent;
	int16 parent_row, parent_col;

	PathNode() :
		row(-1), col(-1), f_score(0), g_score(0), h_score(0), parent_row( 0 ), parent_col( 0 ) {}

	PathNode(int16 r, int16 c) :
		row(r), col(c), f_score(0), g_score(0), h_score(0), parent_row( 0 ), parent_col( 0 ) {}

	//! \brief Overloaded comparison operator checks that tile.row and tile.col are equal
	bool operator==(const PathNode& that) const
		{ return ((this->row == that.row) && (this->col == that.col)); }

	//! \brief Overloaded comparison operator checks that tile.row or tile.col are not equal
	bool operator!=(const PathNode& that) const
		{ return ((this->row != that.row) || (this->col != that.col)); }

	//! \brief Overloaded comparison operator only used for path finding. It compares the two f_scores.
	bool operator<(const PathNode& that) const
		{ return this->f_score > that.f_score; }
}; // class PathNode

} // namespace private_map


/** ****************************************************************************
*** \brief Handles the game execution while the player is exploring maps.
***
*** This class contains all of the structures that together compose each map, as
*** well as some other information. The methods provided by this class are those
*** methods that are either commonly used, or require high performance. Each map
*** has a Lua script file in which the map data is permanently retained and
*** various script subroutines exist that modify the map's behavior. Keep in mind
*** that this class alone does not represent all of the data nor all of the code
*** that is used in a particular map, as the map's Lua file may retain some of
*** this information to itself.
***
*** Maps are composed by a series of tiles and objects. Tiles are 32x32 pixel
*** squares that are adjacent to one another on a map, and together make up the
*** map's background environment. Objects are variable sized entities that are
*** usually living, animated creatures (sprites), but may be something static
*** such as a large tree. Tiles and objects are drawn in multiple interwieving
*** layers to emulate a 3D environment for the game.
***
*** \note Although the drawing coordinates are in terms of 32x32 tiles, the rest
*** of the map follows a 16x16 grid for collision detection, pathfinding, etc.
*** Because the positions of map objects are defined in terms of this 16x16 grid,
*** that means that when drawing the images, the position must be converted to
*** the 32x32 grid.
*** ***************************************************************************/
class MapMode : public hoa_mode_manager::GameMode {
	friend class private_map::MapFrame;
	friend class private_map::MapObject;
	friend class private_map::PhysicalObject;
	friend class private_map::MapTreasure;
	friend class private_map::VirtualSprite;
	friend class private_map::MapSprite;
	friend class private_map::EnemySprite;
	friend class private_map::DialogueManager;
	friend class private_map::TreasureMenu;
	friend class private_map::MapDialogue;
	friend class private_map::SpriteAction;
	friend class private_map::ActionPathMove;
	friend class private_map::EnemyZone;

	friend void hoa_defs::BindModesToLua();
public:
	MapMode(std::string filename);

	~MapMode();

	//! \brief Resets appropriate class members. Called whenever the MapMode object is made the active game mode.
	void Reset();

	//! \brief Updates the game and calls various sub-update functions depending on the state of map mode.
	void Update();

	//! \brief Handles the drawing of everything on the map and makes sub-draw function calls as appropriate.
	void Draw();

private:
	/** \brief A reference to the current instance of MapMode
	*** This is used for callbacks from Lua, as well as for map objects to be able to refer to the
	*** map that they exist in.
	**/
	static MapMode *_current_map;

	/** \brief A reference to the instance of MapMode which most recently had its constructor called
	*** This is not the same as _current_map, and this pointer should only be used in conjunction with
	*** loading code. The pointer is only updated when the MapMode constructor is called.
	**/
	static MapMode *_loading_map;

	//! \brief Indicates if dialogue icons should be drawn or not.
	static bool _show_dialogue_icons;

	/** \brief A timer used for when the player first enters the map
	*** This timer is set to 7000 ms (7 seconds) and is used to display the map's location graphic
	*** and name at the top center of the screen. The graphic and text are faded in for the first
	*** two seconds, drawn opaquely for the next three seconds, and faded out in the final two seconds.
	**/
	hoa_system::SystemTimer _intro_timer;

	//! \brief The name of the script file that contains the map.
	std::string _map_filename;

	//! \brief The name of the map, as it will be read by the player in the game.
	hoa_utils::ustring _map_name;

	//! \brief A pointer to the object containing all of the event information for the map
	hoa_global::GlobalEventGroup* _map_event_group;

	//! \brief Holds an image that represents an outline of the location, used primarily in MenuMode
	hoa_video::StillImage _location_graphic;

	//! \brief Indicates the current state that the map is in, such as when a dialogue is taking place.
	uint8 _map_state;

	//! \brief The time elapsed since the last Update() call to MapMode.
	uint32 _time_elapsed;

	/** \brief The number of elements in the map grid
	*** The number of map grid rows and columns is always equal to twice that of the number of
	*** tile rows and tile columns.
	**/
	uint16 _num_grid_rows, _num_grid_cols;

	//! \brief Holds the most recently generated object ID number
	uint16 _lastID;

	//! \brief While true, all user input commands to map mode are ignored
	bool _ignore_input;

	//! \brief If true, the player's stamina will not drain as they run
	bool _run_forever;

	//! \brief While true, the player is not allowed to run at all.
	bool _run_disabled;

	/** \brief The amount of stamina
	*** This value ranges from 0 (empty) to 10000 (full). Stamina takes 10 seconds to completely fill from
	*** the empty state and 5 seconds to empty from the full state.
	**/
	uint32 _run_stamina;

	//! \brief Retains information needed to correctly draw the next map frame.
	private_map::MapFrame _draw_info;

	/** \brief The interface to the file which contains all the map's stored data and subroutines.
	*** This class generally performs a large amount of communication with this script continuously.
	*** The script remains open for as long as the MapMode object exists.
	**/
	hoa_script::ReadScriptDescriptor _map_script;

	/** \brief A script function which assists with the MapMode#Update method
	*** This function implements any custom update code that the specific map needs to be performed.
	*** The most common operation that this script function performs is to check for trigger conditions
	*** that cause map events to occur
	**/
	ScriptObject _update_function;

	/** \brief Script function which assists with the MapMode#Draw method
	*** This function allows for drawing of custom map visuals. Usually this includes lighting or
	*** other visual effects for the map environment.
	**/
	ScriptObject _draw_function;

	//! \brief Instance of helper class to map mode. Responsible for tile related operations.
	private_map::TileManager* _tile_manager;

	//! \brief Instance of helper class to map mode. Responsible for object and sprite related operations.
// 	ObjectManager* _object_manager;

	/** \brief A 2D vector indicating which spots on the map sprites may walk on.
	*** This vector is kept seperate from the vector of tiles because each tile
	*** has 4 walkable uint32 bitflags associated with it. Note that sprite objects may
	*** come in various sizes, so not all sprites may fit through a narrow
	*** passage way.
	**/
	std::vector<std::vector<uint32> > _map_grid;

	/** \brief A map containing pointers to all of the sprites on a map.
	*** This map does not include a pointer to the MapMode#_camera nor MapMode#_virtual_focus
	*** sprites. The map key is used as the sprite's unique identifier for the map. Keys
	*** 1000 and above are reserved for map sprites that correspond to the character's party.
	**/
	std::map<uint16, private_map::MapObject*> _all_objects;

	/** \brief A container for all of the map objects located on the ground layer.
	*** The ground object layer is where most objects and sprites exist in Allacrost.
	**/
	std::vector<private_map::MapObject*> _ground_objects;

	/** \brief A container for all of the map objects located on the pass layer.
	*** The pass object layer is named so because objects on this layer can both be
	*** walked under or above by objects in the ground object layer. A good example
	*** of an object that would typically go on this layer would be a bridge. This
	*** layer usually has very few objects for the map. Also, objects on this layer
	*** are unaffected by the maps context. In other words, these objects are always
	*** drawn on the screen, regardless of the current context that the player is in.
	**/
	std::vector<private_map::MapObject*> _pass_objects;

	/** \brief A container for all of the map objects located on the sky layer.
	*** The sky object layer contains the last series of elements that are drawn on
	*** a map. These objects exist high in the sky above all other tiles and objects.
	*** Translucent clouds can make good use of this object layer, for instance.
	**/
	std::vector<private_map::MapObject*> _sky_objects;

	/** \brief A pointer to the map sprite that the map camera will focus on.
	*** \note Note that this member is a pointer to a map sprite, not a map object.
	*** However, this does not mean that the camera is not able to focus on non-sprite
	*** map objects. The MapMode#_virtual_focus member can be used to emulate that
	*** focus.
	**/
	private_map::VirtualSprite* _camera;

	/** \brief A "virtual sprite" that can serve as a focus point for the camera.
	*** This sprite is not visible to the player nor does it have any collision
	*** detection properties. Usually, the camera focuses on the player's sprite
	*** rather than this object, but it is useful for scripted sequences and other
	*** things.
	**/
	private_map::VirtualSprite *_virtual_focus;

	//! \brief Icon which appears over NPCs who have unread dialogue
	hoa_video::AnimatedImage _new_dialogue_icon;

	//! \brief Image which underlays the stamina bar for "running"
	hoa_video::StillImage _stamina_bar_background;

	//! \brief Image which overlays the stamina bar to show that the player has unlimited running
	hoa_video::StillImage _stamina_bar_infinite_overlay;

	//! \brief The music that the map will need to make use of.
	std::vector<hoa_audio::MusicDescriptor> _music;

	//! \brief The sounds that the map needs available to it.
	std::vector<hoa_audio::SoundDescriptor> _sounds;

	//! \brief This keeps a pointer to the active dialogue.
	private_map::DialogueManager* _dialogue_manager;

	//! \brief Class member object which processes all information related to treasure discovery
	private_map::TreasureMenu* _treasure_menu;

	//! \brief Container for map zones, used for various purposes such as spawning of enemies
	std::vector<private_map::MapZone*> _zones;

	//! \brief store the namespace of the current map
	std::string _map_namespace;

	// -------------------- Battle Data Retained by the Map

	/** \brief A container for the various foes which may appear on this map.
	*** These enemies are kept at their initial stats. They are passed to battle mode, where a copy of
	*** each enemy is made and initialized there.
	**/
	std::vector<hoa_global::GlobalEnemy*> _enemies;

	//! \brief Loads all map data as specified in the Lua file that defines the map.
	void _Load();

	// -------------------- Update Methods

	//! \brief Handles user input when the map is in the explore state.
	void _HandleInputExplore();

	//! \brief Handles user input when the map is in the dialogue state.
	void _HandleInputDialogue();

	/** \brief Finds the nearest interactable object within a certain distance.
	*** \param *sprite The sprite who is trying to find its nearest object.
	*** \return A pointer to the nearest interactable map object, or NULL if no such object was found.
	***
	*** An interactable object must be in the same context as the function argument is. For an object
	*** to be valid, it's collision rectangle must be no greater than 3 grid elements from the sprite's
	*** "calling" axis, and th
	***
	**/
	private_map::MapObject* _FindNearestObject(const private_map::VirtualSprite* sprite);

	/** \brief Determines if a map sprite's position is invalid because of a collision
	*** \param sprite A pointer to the map sprite to check
	*** \return True if a collision was detected, false if one was not
	***
	*** This method is invoked by the map sprite who wishes to check for its own collision. The
	*** collision detection is performed agains three types of obstacles:
	***
	*** -# Boundary conditions: where the sprite has walked off the map
	*** -# Tile collisions: where the sprite's collision rectangle overlaps with an unwalkable map grid tile.
	*** -# Object collision: where the sprite's collision rectangle overlaps that of another object's,
	***    where the object is in the same draw layer and context as the original sprite.
	***
	*** \note This function does <b>not</b> check if the MapSprite argument has its no_collision member
	*** set to false, but it <b>does</b> check that of the other MapObjects.
	**/
	bool _DetectCollision(private_map::VirtualSprite* sprite);

	/** \brief Finds a path from a sprite's current position to a destination
	*** \param sprite A pointer of the sprite to find the path for
	*** \param path A reference to a vector of PathNode objects to store the path
	*** \param dest The destination coordinates
	***
	*** This algorithm uses the A* algorithm to find a path from a source to a destination.
	*** This function ignores the position of all other objects and only concerns itself with
	*** which map grid elements are walkable.
	***
	*** \note If an error is detected, the function will return an empty path argument.
	**/
	void _FindPath(const private_map::VirtualSprite* sprite, std::vector<private_map::PathNode>& path, const private_map::PathNode& dest);

	// -------------------- Draw Methods

	//! \brief Calculates information about how to draw the next map frame.
	void _CalculateDrawInfo();

	//! \brief Draws all visible map tiles and sprites to the screen
	void _DrawMapLayers();

	//! \brief Draws all GUI visuals, such as dialogue icons and the run meter
	void _DrawGUI();

	// -------------------- Lua Binding Functions
	/** \name Lua Access Functions
	*** These methods exist not to allow outside C++ classes to access map data, but instead to
	*** allow Lua to make function calls to examine and modify the map's state.
	**/
	//@{
	void _AddGroundObject(private_map::MapObject *obj);

	void _AddPassObject(private_map::MapObject *obj);

	void _AddSkyObject(private_map::MapObject *obj);

	void _AddZone(private_map::MapZone *zone);

	uint16 _GetGeneratedObjectID()
		{ return ++_lastID; }

	void _SetMapState(uint8 state)
		{ _map_state = state; }

	void _SetCameraFocus(private_map::VirtualSprite *sprite)
		{ _camera = sprite; }

	uint8 _GetMapState() const
		{ return _map_state; }

	uint32 _GetTimeElapsed() const
		{ return _time_elapsed; }

	private_map::VirtualSprite* _GetCameraFocus() const
		{ return _camera; }

	static void _ShowDialogueIcons( bool state )
		{ _show_dialogue_icons = state; }

	static bool _IsShowingDialogueIcons()
		{ return _show_dialogue_icons; }
	//@}
}; // class MapMode

} // namespace hoa_map;

#endif
