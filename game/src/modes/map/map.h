///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
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
*** This file contains the interface for map mode, active when the player is
*** exploring town or dungeon maps. The map environments of Allacrost are
*** quite extensive, thus this code is responsible for processing many things.
*** This includes handling all tile images, objects, sprites, map events,
*** dialogue, and more.
***
*** Each individual map is represented by it's own object of the MapMode class.
*** It is our intention in the future to retain more than one map in memory at
*** once to reduce or eliminate loading times when the player transitions
*** between maps.
*** ***************************************************************************/

#ifndef __MAP_HEADER__
#define __MAP_HEADER__

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "mode_manager.h"
#include "notification.h"
#include "script.h"
#include "video.h"
#include "system.h"

// Common files
#include "common.h"

// Local map mode headers
#include "map_utils.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the map code. Don't use this namespace anywhere else!
namespace private_map {

//! \brief Used to set the current music track to invalid, effectively stops any music from playing
const uint32 INVALID_TRACK = 0xFFFFFFFF;

/** ****************************************************************************
*** \brief A notification event class describing sprite collisions
***
*** Whenever a sprite of any type moves on the map and has a collision, one of these
*** notification events is generated to describe the type and particulars about the
*** collision. This can be used by a map script to determine whether to play a sound,
*** switch the context of the sprite, or take some other action.
***
*** \note Because collision resolution changes the position of the sprite, you can
*** not rely on the position of the sprite when the notification event is being processed.
*** This is why this class has members that retain the position of the sprite as the collision
*** happened.
*** ***************************************************************************/
class MapCollisionNotificationEvent : public hoa_notification::NotificationEvent {
public:
	/** \param type The type of collision that occurred
	*** \param sprite The sprite that had the collision
	*** \note You should \b not use this constructor for object-type collisions
	**/
	MapCollisionNotificationEvent(COLLISION_TYPE type, VirtualSprite* sprite) :
		NotificationEvent("map", "collision"), collision_type(type), sprite(sprite), object(NULL) { _CopySpritePosition(); }

	/** \param type The type of collision that occurred (should be COLLISION_OBJECT)
	*** \param sprite The sprite that had the collision
	*** \param object The object that the sprite collided with
	*** \note You should \b only use this constructor for object-type collisions
	**/
	MapCollisionNotificationEvent(COLLISION_TYPE type, VirtualSprite* sprite, MapObject* object) :
		NotificationEvent("map", "collision"), collision_type(type), sprite(sprite), object(object) { _CopySpritePosition(); }

	//! \brief Returns a string representation of the collision data stored in this object
	const std::string DEBUG_PrintInfo();

	//! \brief The type of collision that caused the notification to be generated
	COLLISION_TYPE collision_type;

	//! \brief The sprite that had the collision
	VirtualSprite* sprite;

	//! \brief Saved position data from the sprite at the time of the collision
	uint16 x_position, y_position;
	float x_offset, y_offset;

	//! \brief The object that the sprite collided with, if it was an object type collision. Otherwise will be NULL
	MapObject* object;

private:
	//! \brief Retains the state of the sprite's position data in the class members
	void _CopySpritePosition();
}; // class MapCollisionNotificationEvent : public hoa_notification::NotificationEvent

} // namespace private_map

/** ****************************************************************************
*** \brief Handles the game execution while the player is exploring maps.
***
*** This class contains all of the structures that together compose each map.
*** Each map seen in the game is comprised of two Lua files: a map data file and
*** a map sript file. The map data file holds the static physical properties
*** of the map. This includes things like the dimensions of the map, the tilesets
*** that are used, the map contexts, and so on. This file is generated by the
*** map editor. A single map data file may be used by more than one map script file.
***
*** The map script file contains the custom programming that makes maps dynamic
*** and interesting. The script contains additional map data not found in the map
*** data file, such as lines of dialogue, map sprite properties, and map zones. Each
*** script file defines which map data file should be loaded. The script file consists of
*** several functions, some which must always be defined such as Load(), Update(), and Draw().
***
*** Maps are composed by a series of tiles and objects. Tiles are 32x32 pixel
*** squares that are adjacent to one another on a map, and together make up the
*** map's background environment. Objects are variable sized entities that are
*** usually living, animated creatures (sprites), but may be something static
*** such as a large tree. Tiles and objects are drawn in multiple interwieving
*** layers to emulate a 3D environment for the game. Additionally each map has
*** a collision grid composed of 16x16 pixel elements that determine which
*** quadrants of each tile may be occupied by sprites or other objects.
***
*** Another important concept to MapMode is that of contexts. Each map has at
*** least one context and can have up to a maximum of 32 contexts. Every context
*** has its own collision grid and its own set of tiles. Map objects and sprites
*** exist in one of these context and may change their context at any time.
*** Objects and sprites can not interact with one another when they are not
*** within the same context, and typically only objects that are in the same
*** context as the map camera are visible on the screen. You can think of each
*** context as essentially its own map, and the set of contexts as a set of maps
*** that work with one another to create a cohesive environment.
***
*** Because this game mode is so complex, the MapMode class shares its
*** responsibilities with several smaller classes that manage a
*** particular area of map code, such as tiles or objects. These sub-manager
*** classes should be viewed as extensions of the MapMode class.
*** ***************************************************************************/
class MapMode : public hoa_mode_manager::GameMode {
	friend void hoa_defs::BindModeCode();

public:
	/** \param script_filename The name of the Lua file that contains all of the map scripting code
	*** \note The script file
	**/
	MapMode(std::string script_filename);

	~MapMode();

	//! \brief Resets appropriate class members. Called whenever the MapMode object is made the active game mode.
	void Reset();

	//! \brief Updates the game and calls various sub-update functions depending on the current state of map mode.
	void Update();

	//! \brief The highest level draw function that will call the appropriate lower-level draw functions
	void Draw();

	// The methods below this line are not intended to be used outside of the map code

	//! \brief Empties the state stack and places an invalid state on top
	void ResetState();

	/** \brief Pushes a state type to the top of the state stack, making it the active state
	*** \param state The state to push onto the stack
	**/
	void PushState(private_map::MAP_STATE state);

	//! \brief Removes the highest item in the state stack
	void PopState();

	/** \brief Retrieves the current map state
	*** \return The top-most item on the map state stack
	**/
	private_map::MAP_STATE CurrentState();

	/** \brief Opens the tablespace of the map script
	*** \param use_global Has the same effect as in ReadScriptDescriptor#OpenTable
	**/
	void OpenScriptTablespace(bool use_global = false)
		{ _map_script.OpenTable(_script_tablespace, use_global); }

	//! \brief Adds a new zone to the map
	void AddZone(private_map::MapZone *zone);

	/** \brief Checks if a GlobalEnemy with the specified id is already loaded in the MapMode#_enemies container
	*** \param id The id of the enemy to find
	*** \return True if the enemy is loaded
	**/
	bool IsEnemyLoaded(uint32 id) const;

	void PlayMusic(uint32 track_num);

	/** \brief A convenience function for moving the virtual focus sprite to a new position
	*** \param x The x coordinator to move to
	*** \param y The y coordinator to move to
	**/
	void MoveVirtualFocus(uint16 x, uint16 y);

    void MoveVirtualFocus(uint16 x, uint16 y, uint32 duration);

	//! \brief Returns true if the player may enter a battle upon colliding with an enemy sprite
    bool AttackAllowed()
		{ return (CurrentState() != private_map::STATE_DIALOGUE && CurrentState() != private_map::STATE_TREASURE && !IsCameraOnVirtualFocus()); }

	/** \brief Instantly changes the current context
	*** \param new_context The context to change to. If it is invalid or the same as the current context, no change will take place
	**/
	void ContextTransitionInstant(private_map::MAP_CONTEXT new_context);

	/** \brief Begins the transition to the new context by blending the outgoing and incoming contexts together
	*** \param new_context The context to change to. If it is invalid or the same as the current context, no change will take place
	*** \param time The total time that the transition should take. If set to zero, DEFAULT_CONTEXT_TRANSITION_TIME will be used
	**/
	void ContextTransitionBlend(private_map::MAP_CONTEXT new_context, uint32 time);

	/** \brief Begins the transition to the new context by fading the screen to black, then fading back into the new context
	*** \param new_context The context to change to. If it is invalid or the same as the current context, no change will take place
	*** \param time The total time that the transition should take. If set to zero, DEFAULT_CONTEXT_TRANSITION_TIME will be used
	**/
	void ContextTransitionBlackColor(private_map::MAP_CONTEXT new_context, uint32 time)
		{ ContextTransitionColor(new_context, time, hoa_video::Color::black); }

	/** \brief Begins the transition to the new context by fading the screen to a color, then fading back into the new context
	*** \param new_context The context to change to. If it is invalid or the same as the current context, no change will take place
	*** \param time The total time that the transition should take. If set to zero, DEFAULT_CONTEXT_TRANSITION_TIME will be used
	*** \param color The color to use for the transition
	**/
	void ContextTransitionColor(private_map::MAP_CONTEXT new_context, uint32 time, hoa_video::Color color);

	//! \brief Class member accessor functions
	//@{
	static MapMode* const CurrentInstance()
		{ return _current_instance; }

	const hoa_utils::ustring& GetMapName() const
		{ return _map_name; }

	hoa_common::CommonRecordGroup* GetGlobalRecordGroup() const
		{ return _global_record_group; }

	hoa_common::CommonRecordGroup* GetLocalRecordGroup()
		{ return &_local_record_group; }

	hoa_script::ReadScriptDescriptor& GetMapScript()
		{ return _map_script; }

	private_map::TileSupervisor* GetTileSupervisor() const
		{ return _tile_supervisor; }

	private_map::ObjectSupervisor* GetObjectSupervisor() const
		{ return _object_supervisor; }

	private_map::EventSupervisor* GetEventSupervisor() const
		{ return _event_supervisor; }

	private_map::DialogueSupervisor* GetDialogueSupervisor() const
		{ return _dialogue_supervisor; }

	private_map::TreasureSupervisor* GetTreasureSupervisor() const
		{ return _treasure_supervisor; }

	const private_map::MapFrame& GetMapFrame() const
		{ return _map_frame; }

	private_map::VirtualSprite* GetCamera() const
		{ return _camera; }

	void SetCamera(private_map::VirtualSprite* sprite)
		{ _camera = sprite; }

	void SetCamera(private_map::VirtualSprite* sprite, uint32 duration);

	bool IsCameraOnVirtualFocus() const
		{ return (_camera == _virtual_focus); }

	bool IsCameraOnPlayerSprite() const
		{ return (_camera == _player_sprite); }

	//! \brief Returns true if the camera is in the process of moving to a new focus point
	bool IsCameraMoving() const
		{ return _camera_timer.IsRunning(); }

	private_map::VirtualSprite* GetPlayerSprite() const
		{ return _player_sprite; }

	void SetPlayerSprite(private_map::VirtualSprite* sprite)
		{ _player_sprite = sprite; }

	private_map::VirtualSprite* GetVirtualFocus() const
		{ return _virtual_focus; }

	//! \brief Empties out the draw layer order
	void ClearLayerOrder()
		{ _layer_order.clear(); }

	/** \brief Adds a tile layer to the draw layer order
	*** \param layer_id The ID of the tile layer to add
	**/
	void AddTileLayerToOrder(uint32 layer_id);

	/** \brief Adds an object layer to the draw layer order
	*** \param layer_id The ID of the object layer to add
	**/
	void AddObjectLayerToOrder(uint32 layer_id);

	uint8 GetNumMapContexts() const
		{ return _num_map_contexts; }

	private_map::MAP_CONTEXT GetCurrentContext() const
		{ return _current_context; }

	//! \brief Returns true if dialogue icons appear above sprites with dialogue
	bool IsDialogueIconsVisible() const
		{ return _dialogue_icons_visible; }

	/** \brief Toggles the display of dialogue icons above a sprite's head
	*** \param state Set to true to make the icons visible, or false to hide them
	**/
	void ShowDialogueIcons(bool state)
		{ _dialogue_icons_visible = state; }

	//! \brief Returns true if the stamina bar in the lower right corner of the screen is not visible
	bool IsStaminaBarVisible() const
		{ return _stamina_bar_visible; }

	/** \brief Toggles the display of the stamina bar in the lower right corner of the screen
	*** \param state Set to true to make the stamina bar visible, or false to hide it
	**/
	void ShowStaminaBar(bool state)
		{ _stamina_bar_visible = state; }

	/** \brief Prevents the display of any map introduction graphic or text
	*** \note Because the introduction visuals are only displayed for a brief period of time when a map first loads,
	*** it really only makes sense to call this method from the map's load function.
	**/
	void DisableIntroductionVisuals()
		{ _intro_timer.Finish(); }

	/** \note This sets the track, but does not stop current music or play new music. It is mostly
	*** used in the map script's Load() function to set the track that should begin playing when the
	*** map mode instance becomes active for the first time.
	**/
	void SetCurrentTrack(uint32 track)
		{ _current_track = track; }

	/** \brief Transitions to the supplied game mode from the current game mode with a graphical effect.
	*** \param game_mode that is to be transitioned to from the current mode.
	**/
	void _TransitionToMode(GameMode *);

	const hoa_video::AnimatedImage& GetDialogueIcon() const
		{ return _dialogue_icon; }

	const hoa_video::StillImage& GetLocationGraphic() const
		{ return _location_graphic; }

	//@}

private:
	// ----- Members : Names and Identifiers -----

	/** \brief A reference to the current instance of MapMode
	*** This is used by other map clases to be able to refer to the map that they exist in.
	**/
	static MapMode* _current_instance;

	//! \brief The name of the Lua file that holds the map data
	std::string _data_filename;

	//! \brief The name of the Lua file that contains the map's scripting code
	std::string _script_filename;

	//! \brief The name of the tablespace that encapsulates the map script file
	std::string _script_tablespace;

	//! \brief The name of the map, as it will be read by the player in the game.
	hoa_utils::ustring _map_name;

	//! \brief A pointer to the object containing all of the global record information for the map
	hoa_common::CommonRecordGroup* _global_record_group;

	/** \brief Another record group that is local to the map mode instance
	*** \note Data entered to the local record is permanently destroyed when the MapMode instance destructor is called
	**/
	hoa_common::CommonRecordGroup _local_record_group;

	/** \brief The interface to the file which contains all the map's stored data and subroutines.
	*** This class generally performs a large amount of communication with this script continuously.
	*** The script remains open for as long as the MapMode object exists.
	**/
	hoa_script::ReadScriptDescriptor _map_script;

	// ----- Members : Supervisor Class Objects and Script Functions -----

	//! \brief Instance of helper class to map mode. Responsible for tile related operations.
	private_map::TileSupervisor* _tile_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for object and sprite related operations.
	private_map::ObjectSupervisor* _object_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for updating and managing active map events.
	private_map::EventSupervisor* _event_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for dialogue execution and display operations.
	private_map::DialogueSupervisor* _dialogue_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for processing all information related to treasure discovery.
	private_map::TreasureSupervisor* _treasure_supervisor;

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

	// ----- Members : Properties and State -----

	//! \brief Retains information needed to correctly draw the next map frame
	private_map::MapFrame _map_frame;

	//! \brief A pointer to the map sprite that the map camera will focus on
	private_map::VirtualSprite* _camera;

	/** \brief A pointer to the map sprite that represents the sprite that the player currently controls
	*** \note Typically, a map script will set this when the map loads and will not change the player sprite
	*** to any other value. It is not a requirement for a map to set this member to be set to a non-NULL value.
	**/
	private_map::VirtualSprite* _player_sprite;

	/** \brief A "virtual sprite" that can serve as a focus point for the camera. This sprite is not visible to
	*** the player nor does it have any collision detection properties. Usually, the camera focuses on the player's
	*** sprite rather than this object, but it is useful for scripted sequences and other things.
	**/
	private_map::VirtualSprite* _virtual_focus;

	//! \brief The way in x-direction, the camera will move
	float _delta_x;

	//! \brief The way in y-direction, the camera will move
	float _delta_y;

	//! \brief The number of contexts that this map uses (at least 1, at most 32)
	uint8 _num_map_contexts;

	//! \brief The currently active map context
	private_map::MAP_CONTEXT _current_context;

	/** \brief While transitioning between two contexts, holds the value of the context we are
	*** transitioning from
	**/
	private_map::MAP_CONTEXT _previous_context;

	//! \brief Retains the type of transition that is being performed
	private_map::MAP_CONTEXT_TRANSITION_TYPE _transition_type;

	//! \brief Holds the color to transition with if doing a color transition type
	hoa_video::Color _transition_color;

	// ----- Members : Running and Stamina -----

	//! \brief If true, the player is not allowed to run.
	bool _run_disabled;

	//! \brief When true, the player has activated running for their sprite
	bool _run_state;

	/** \brief A counter for the player's stamina
	*** This value ranges from STAMINA_EMPTY to STAMINA_FULL. It takes twice as long to regenerate stamina as
	*** it does to consume it when running.
	**/
	uint32 _run_stamina;

	//! \brief If true, the player's stamina will not drain for actions and the stamina bar will not be shown
	bool _unlimited_stamina;

	//! \brief When true, icons will be drawn above sprites that have dialogue to share
	bool _dialogue_icons_visible;

	//! \brief When false, the stamina bar in the lower right corner of the screen will not be drawn
	bool _stamina_bar_visible;

	//! \brief Index of current music track to play
	uint32 _current_track;

	/** \brief Maintains a stack state for the different modes of operation that the map may be in
	*** The top (back) of the stack is the active mode
	**/
	std::vector<private_map::MAP_STATE> _state_stack;

	//! \brief Holds pointers to the tile and object layers in their correct draw order
	std::vector<private_map::MapLayer*> _layer_order;

	// ----- Members : Timing and Graphics -----

	/** \brief A timer used for when the player first enters the map
	*** This timer is set to 7000 ms (7 seconds) and is used to display the map's location graphic
	*** and name at the top center of the screen. The graphic and text are faded in for the first
	*** two seconds, drawn opaquely for the next three seconds, and faded out in the final two seconds.
	**/
	hoa_system::SystemTimer _intro_timer;

	//! \brief A timer used for assisting with camera movement
	hoa_system::SystemTimer _camera_timer;

	//! \brief A timer used to transition between two contexts smoothly
	hoa_system::SystemTimer _context_transition_timer;

	//! \brief Freestyle art image of the current map
	hoa_video::StillImage _location_graphic;

	//! \brief An icon graphic which appears over the heads of NPCs who have dialogue that has not been read by the player
	hoa_video::AnimatedImage _dialogue_icon;

	//! \brief Image which underlays the stamina bar for running
	hoa_video::StillImage _stamina_bar_background;

	//! \brief Image which overlays the stamina bar to show that the player has unlimited running
	hoa_video::StillImage _stamina_bar_infinite_overlay;

	// ----- Members : Containers -----

	//! \brief The music that the map will need to make use of
	std::vector<hoa_audio::MusicDescriptor> _music;

	//! \brief The sounds that the map needs available to it
	std::vector<hoa_audio::SoundDescriptor> _sounds;

	/** \brief A container for the various foes which may appear on this map
	*** These enemies are kept at their initial stats. When they are passed to battle mode,
	*** a copy of each enemy is made and initialized there.
	**/
	std::vector<hoa_global::GlobalEnemy*> _enemies;

	// ----- Methods -----

	//! \brief Opens both the map data and script files and loads all necessary data from them
	void _LoadMapFiles();

	/** \brief Checks if the map can begin transitioning to a new context
	*** \param new_context The context to change to
	*** \return True if the transition is okay to proceed. False if the context argument is invalid or
	*** another transition is already in progress
	**/
	bool _IsContextTransitionValid(private_map::MAP_CONTEXT new_context);

	//! \brief A helper function to Update() that is called only when the map is in the explore state
	void _UpdateExplore();

	//! \brief Calculates information about how to draw the next map frame
	void _CalculateMapFrame();

	//! \brief Draws all visible map tiles and sprites to the screen
	void _DrawMapLayers();

	//! \brief Draws all GUI visuals, such as dialogue icons and the stamina bar
	void _DrawGUI();

	//! \brief Draws the transition effect that occurs during game mode transitions from map mode.
	void _DrawModeTransition();

	/** \brief Updates during the map mode transition, checking when the transition is complete
	*** to know when to push the game mode that is to be transitioned to.
	**/
	void _UpdateModeTransition();

	//! \brief When true the map mode is transitioning and the screen should be faded out
	bool _fade_out;

        //! \brief Variable to store game mode to be transitioned into from map mode
	hoa_mode_manager::GameMode* _transition_mode;

}; // class MapMode

} // namespace hoa_map;

#endif
