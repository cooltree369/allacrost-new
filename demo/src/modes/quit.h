////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    quit.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for quit mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in quit mode (when the quit menu is active). The quit menu gives the user
*** three options: Quit to BootScreen, Quit Game, and Cancel. Therefore the game
*** won't quit unless the user requests a quit event twice in a row.
***
*** \note When the user enters this mode, the game will sleep for small periods
*** of time so that the game isn't using up 100% of the CPU.
*** ***************************************************************************/

#ifndef __QUIT_HEADER__
#define __QUIT_HEADER__

#include "utils.h"
#include <string>
#include "defs.h"
#include "mode_manager.h"
#include "video.h"
#include "gui.h"


//! All calls to quit mode are wrapped in this namespace.
namespace hoa_quit {

//! Determines whether the code in the hoa_quit namespace should print debug statements or not.
extern bool QUIT_DEBUG;

//! An internal namespace to be used only within the quit code. Don't use this namespace anywhere else!
namespace private_quit {

//! \name Quit Option Constants
//@{
//! \brief Constants for different quit options that the user can select.
const uint8 QUIT_GAME        = 0;
const uint8 QUIT_TO_BOOTMENU = 1;
const uint8 QUIT_CANCEL      = 2;
//@}

} // namespace private_quit

/** ****************************************************************************
*** \brief A mode pushed onto the game mode stack to confirm a user's quit command.
***
*** There are three options that the user may select when in quit mode:
*** - Quit Game          :: Exits the game.
*** - Quit to Bootscreen :: Cleans off the game mode stack and returns to the boot screen.
*** - Cancel             :: Removes QuitMode from the top of the stack and continues the game.
***
*** This game mode can be reached in two different ways:
*** - Pressing the Ctrl + Q keys at the same time
*** - Clicking the cursor on the close button of the game's window
***
*** If either of these events occurs and QuitMode is already the active game mode,
*** that effectively selects the Quit Game option and the game exits without further
*** warning.
***
*** \note 1) Selecting the "Quit Game" option while in boot mode will not push
*** this mode on the game stack. The game will exit with no further confirmation.
***
*** \note 2) During some scenes of the game you might need the audio to be synchronized
*** with the flow of action. If the user tries to quit the game you will want to pause
*** the audio so the audio doesn't go out of synch with the action. In order to do this,
*** when you begin such a scene you need to call the SetPauseVolumeAction() function of
*** the  GameSettings class with the argument SETTINGS_PAUSE_AUDIO. When you are finished
*** with this type of scene, you must must MUST remember to set this member back to its
*** original value.
*** ***************************************************************************/
class QuitMode : public hoa_mode_manager::GameMode {
private:
	//! The currently selected quit option
	uint8 _quit_type;
	//! An image of the last frame shown on the screen before QuitMode was called.
	hoa_video::StillImage _saved_screen;
	//! An image of the quit menu.
	hoa_video::StillImage _quit_menu;
	//! The option box which presents the quit options that the user can select.
	hoa_video::OptionBox _option_box;

	//! Helper function which quits the game
	void _QuitGame();
	//! Helper function which quits to the boot menu
	void _QuitToBootMenu();
	//! Helper function which cancels out of quit mode
	void _Cancel();

public:
  QuitMode();
  ~QuitMode();

	//! Resets appropriate class members. Called whenever PauseMode is made the active game mode.
	void Reset();
	//! Updates the game state by the amount of time that has elapsed
  void Update();
	//! Draws the next frame to be displayed on the screen
  void Draw();
	
}; // class QuitMode

} // namespace hoa_quit

#endif
