///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_CharacterWindow.h
*** \author  Daniel Steuernol steu@allacrost.org
*** \author  Andy Gardner chopperdave@allacrost.org
*** \brief   Header file for character menu.
***
*** This code handles the different menu windows that the user will see while the
*** is in menu mode. These windows are used for displaying inventory lists,
*** character statistics, and various other pieces of information.
*** ***************************************************************************/

#ifndef __MENU_VIEWS__
#define __MENU_VIEWS__

#include <string>
#include <vector>

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "gui.h"

#include "global.h"



namespace hoa_menu {

	namespace private_menu {

		//! \brief The different item categories
		enum ITEM_CATEGORY {
			ITEM_ALL = 0,
			ITEM_ITEM = 1,
			ITEM_WEAPONS = 2,
			ITEM_HEAD_ARMOR = 3,
			ITEM_TORSO_ARMOR = 4,
			ITEM_ARM_ARMOR = 5,
			ITEM_LEG_ARMOR = 6,
			ITEM_KEY = 7,
			ITEM_CATEGORY_SIZE = 8
		};

		//! \brief The different skill types
		enum SKILL_CATEGORY {
			SKILL_ALL = 0,
			SKILL_FIELD = 1,
			SKILL_BATTLE = 2,
			SKILL_CATEGORY_SIZE = 3
		};

		//! \brief The different equipment categories
		enum EQUIP_CATEGORY {
			EQUIP_WEAPON = 0,
			EQUIP_HEADGEAR = 1,
			EQUIP_BODYARMOR = 2,
			EQUIP_OFFHAND = 3,
			EQUIP_LEGGINGS = 4,
			EQUIP_CATEGORY_SIZE = 5
		};

		//! \brief The different option boxes that can be active for items
		enum ITEM_ACTIVE_OPTION {
			ITEM_ACTIVE_NONE = 0,
			ITEM_ACTIVE_CATEGORY = 1,
			ITEM_ACTIVE_LIST = 2,
			ITEM_ACTIVE_CHAR = 3,
			ITEM_ACTIVE_SIZE = 4
		};

		//! \brief The different option boxes that can be active for skills
		enum SKILL_ACTIVE_OPTION {
			SKILL_ACTIVE_NONE = 0,
			SKILL_ACTIVE_CHAR = 1,
			SKILL_ACTIVE_CATEGORY = 2,
			SKILL_ACTIVE_LIST = 3,
			SKILL_ACTIVE_CHAR_APPLY = 4,
			SKILL_ACTIVE_SIZE = 5
		};

		//! \brief The different option boxes that can be active for equipment
		enum EQUIP_ACTIVE_OPTION {
			EQUIP_ACTIVE_NONE = 0,
			EQUIP_ACTIVE_CHAR = 1,
			EQUIP_ACTIVE_SELECT = 2,
			EQUIP_ACTIVE_LIST = 3,
			EQUIP_ACTIVE_SIZE = 4
		};

		//! \brief The different option boxes that can be active for equipment
		enum FORM_ACTIVE_OPTION {
			FORM_ACTIVE_NONE = 0,
			FORM_ACTIVE_CHAR = 1,
			FORM_ACTIVE_SECOND = 2,
			FORM_ACTIVE_SIZE = 3
		};

		//! \brief Possible values from the confirm window
		enum CONFIRM_RESULT
		{
			CONFIRM_RESULT_YES = 0,
			CONFIRM_RESULT_NO = 1,
			CONFIRM_RESULT_NOTHING = 2,
			CONFIRM_RESULT_CANCEL = 3,
		};

		/** ****************************************************************************
		*** \brief Represents an individual character window
		***
		*** There should be one of these windows for each character in the game.
		*** It will contain all the information of the character and handle its draw
		*** placement.
		*** ***************************************************************************/
		class CharacterWindow : public hoa_gui::MenuWindow {
		private:
			//! The name of the character that this window corresponds) to
			uint32 _char_id;

			//! The image of the character
			hoa_video::StillImage _portrait;

		public:
			CharacterWindow();

			~CharacterWindow();

			/** \brief Set the character for this window
			*** \param character the character to associate with this window
			**/
			void SetCharacter(hoa_global::GlobalCharacter *character);

			/** \brief render this window to the screen
			*** \return success/failure
			**/
			void Draw();
		}; // class CharacterWindow : public hoa_video::MenuWindow
	}	//private_menu
}	//hoa_menu
#endif
