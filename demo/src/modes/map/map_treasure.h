///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_treasure.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode treasures.
*** *****************************************************************************/

#ifndef __MAP_TREASURE_HEADER__
#define __MAP_TREASURE_HEADER__

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "video.h"

// Local map mode headers
#include "map_objects.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Represents a treasure on the map which the player may access
***
*** A treasure is a specific type of physical object. Treasures may contain
*** multiple quantities and types of items, weapons, armor, or any other type of
*** global object. They may additionally contain any amount of drunes (money).
*** As one would expect, the contents of a treasure can only be retrieved by the
*** player once. Each treasure object on a map has a simple boolean in the
*** saved game file to determine whether the treasure has already been retrieved
*** by the player or not.
***
*** Image files for treasures are single row multi images where the frame ordering
*** goes from closed, to opening, to open. This means each map treasure has exactly
*** three animations. The closed and open animations are usually single frame images.
***
*** \todo Add support for more treasure features, such as locked chests, chests which
*** trigger a battle, etc.
*** ***************************************************************************/
class MapTreasure : public PhysicalObject {
	friend class TreasureMenu;

public:
	/** \param image_file The name of the multi image file to load for the treasure
	*** \param num_total_frames The total number of frame images in the multi image file
	*** \param num_closed_frames The number of frames to use as the closed animation (default value == 1)
	*** \param num_open_frames The number of frames to use as the open animation (default value == 1)
	*** \note The opening animation will be created based on the total number of frames in the image file
	*** subtracted by the number of closed and open frames. If this value is zero, then the opening animation
	*** will simply be the same as the open animation
	**/
	MapTreasure(std::string image_file, uint8 num_total_frames, uint8 num_closed_frames = 1, uint8 num_open_frames = 1);

	~MapTreasure();

	//! \brief Defines for all three treasure animations
	enum {
		TREASURE_CLOSED_ANIM = 0,
		TREASURE_OPENING_ANIM = 1,
		TREASURE_OPEN_ANIM = 2
	};

	//! \brief Loads the state of the chest from the event group corresponding to the current map
	void LoadSaved();

	//! \brief Changes the current animation if it has finished looping
	void Update();

	/** \name Lua Access Functions
	*** These functions are specifically written to enable Lua to access the members of this class.
 	**/
	//@{
	/** \brief Adds an object to the contents of the MapTreasure
	*** \param id The id of the GlobalObject to add
	*** \param number The number of the object to add (default == 1)
	*** \throw Exception An Exception object if nothing could be added to the treasure
	**/
	bool AddObject(uint32 id, uint32 number = 1);

	/** \brief Adds a number of drunes to be the chest's contents
	*** \note The overflow condition is not checked here: we just assume it will never occur
	**/
	void AddDrunes(uint32 amount)
		{ _drunes += amount; }

	//! \brief Indicates if the treasure contains any
	bool IsEmpty() const
		{ return _empty; }

	//! \brief Opens the treasure, which changes the active animation and initializes the treasure menu
	void Open();
	//@}

private:
	//! \brief Set to true when the contents of the treasure have been procured
	bool _empty;

	//! \brief The number of drunes contained in the chest
	uint32 _drunes;

	//! \brief The list of objects given to the player upon opening the treasure
	std::vector<hoa_global::GlobalObject*> _objects_list;
}; // class MapTreasure : public PhysicalObject


/** ***************************************************************************************
*** \brief Displays the contents of a discovered treasure in a menu window
***
*** Upon opening a treasure chest or other treasure-containing map object, this menu
*** will appear and list the amount of drunes found (if any), a list of the icon and name of
*** each GlobalObject found (items, equipment, etc), and a list of player options.
*** The player may select to view detailed information about a particular entry, go to menu mode,
*** and possibly other actions in the future.
***
*** The treasure menu is composed of three sets of windows. The action window is a small window
*** at the top of the menu that displays the action options in a horizontal list. The list window
*** displays the contents of the treasure in a larger window below the action window. This object
*** list is formatted vertically. The detail window shares the same area as the list window and
*** displays textual and visual detail about an object selected by the user from the list window.
***
*** Proper utilization of this class entails the following steps:
***
*** -# Call the Initialize method to show the menu with the treasure that has been obtained
*** -# Call the Update method to process user input and update the menu's appearance
*** -# Call the Draw method to draw the menu to the screen
*** -# Call the Finish method to hide the menu and add the treasure's contents to the player's
***    inventory
***
*** \todo Allow the player to use or equip selected treasure objects directly from the
*** action menu.
***
*** \todo Add visual scissoring to the list window so that the option list or detail text does
*** not exceed the window boundary when the text or list is exceedingly long.
***
*** \todo Instead of forcing the detail window to share the list window, maybe it would look
*** better if there was a separate detail window which "popped out" of the other two windows
*** and could be placed over them when it was visible? I think this would be much more visually
*** pleasing than the current implementation.
*** **************************************************************************************/
class TreasureMenu {
public:
	//! \brief The possible sub-windows that are selected, used for determining how to process user input
	enum SELECTION {
		ACTION_SELECTED = 0, //!< the list of actions a user may take in the treasure menu
		LIST_SELECTED = 1,   //!< active when the user is browsing the list of treasures
		DETAIL_SELECTED = 2  //!< set when the user is viewing details about a particular treasure
	};

	TreasureMenu();

	~TreasureMenu();

	/** \brief Un-hides the menu window and initializes it to display the contents of a new treasure
	*** \param treasure A pointer to the treasure to display the contents of
	**/
	void Initialize(MapTreasure* treasure);

	//! \brief Processes input events from the user and updates the showing/hiding progress of the window
	void Update();

	/** \brief Draws the window to the screen
	*** \note If the Initialize method has not been called with a valid treasure pointer beforehand, this
	*** method will print a warning and it will not draw anything to the screen.
	**/
	void Draw();

	//! \brief Hides the window and adds the treasure's contents to the player's inventory
	void Finish();

	//! \brief Returns true if the treasure menu is active
	bool IsActive() const
		{ return (_treasure != NULL); }

private:
	//! \brief A pointer to the treasure object to display the contents of
	MapTreasure* _treasure;

	//! \brief The currently selected sub-window for processing user input
	SELECTION _selection;

	//! \brief A vector containing pointers to objects which should be deleted upon finishing with the current treasure
	std::vector<hoa_global::GlobalObject*> _objects_to_delete;

	//! \brief Contains options for viewing, using, or equipping inventory, or for exiting the menu
	hoa_video::MenuWindow _action_window;

	//! \brief Lists all of the drunes and inventory objects contained in the treasure
	hoa_video::MenuWindow _list_window;

	//! \brief The available actions that a user can currently take. Displayed in the _action_window.
	hoa_video::OptionBox _action_options;

	//! \brief The name + quantity of all drunes and inventory objects earned. Displayed in the _list_window
	hoa_video::OptionBox _list_options;

	//! \brief A textbox that displays the detailed description about a selected treasure
	hoa_video::TextBox _detail_textbox;

	// ---------- Private methods

	//! \brief Processes user input when the action sub-window is selected
	void _UpdateAction();

	//! \brief Processes user input when the list sub-window is selected
	void _UpdateList();

	//! \brief Processes user input when the detailed view of a treasure object is selected
	void _UpdateDetail();
}; // class TreasureMenu

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_TREASURE_HEADER__
