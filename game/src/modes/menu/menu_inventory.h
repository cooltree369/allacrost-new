///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu_views.h
*** \author  Daniel Steuernol steu@allacrost.org
*** \author  Andy Gardner chopperdave@allacrost.org
*** \brief   Header file for inventory menu.
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
		*** \brief Represents the inventory window to browse the party's inventory
		***
		*** This handles item use.  You can also view all items by category.
		*** ***************************************************************************/
		class InventoryWindow : public hoa_gui::MenuWindow {
			friend class hoa_menu::MenuMode;

		public:
			InventoryWindow();

			~InventoryWindow();

			/** \brief Toggles the inventory window being in the active context for the player
			*** \param new_status Activates the inventory window when true, de-activates it when false
			**/
			void Activate(bool new_status);

			/** \brief Indicates whether the inventory window is in the active context
			*** \return True if the inventory window is in the active context
			**/
			bool IsActive()
			{
				return _active_box;
			}

			//! If the inventory window is ready to cancel out, or cancel out a sub-window
			//bool CanCancel();

			/*!
			* \brief Updates the inventory window.  Handles key presses, switches window context, etc.
			*/
			void Update();

			/*!
			* \brief Draw the inventory window
			* \return success/failure
			*/
			void Draw();

		private:
			//! Used for char portraits in bottom menu
			std::vector<hoa_video::StillImage> _portraits;

			//! Used for the current dungeon
			hoa_video::StillImage _location_graphic;

			//! Flag to specify the active option box
			uint32 _active_box;

			//! OptionBox to display all of the items
			hoa_gui::OptionBox _inventory_items;

			//! OptionBox to choose character
			hoa_gui::OptionBox _char_select;

			//! OptionBox to choose item category
			hoa_gui::OptionBox _item_categories;

			//! TextBox that holds the selected object's description
			hoa_gui::TextBox _description;

			//! Vector of GlobalObjects that corresponds to _inventory_items
			std::vector< hoa_global::GlobalObject* > _item_objects;

			/*!
			* \brief Updates the item text in the inventory items
			*/
			void _UpdateItemText();

			/*!
			* \brief Initializes inventory items option box
			*/
			void _InitInventoryItems();

			/*!
			* \brief Initializes char select option box
			*/
			void _InitCharSelect();

			/*!
			* \brief Initializes item category select option box
			*/
			void _InitCategory();

			template <class T> std::vector<hoa_global::GlobalObject*> _GetItemVector(std::vector<T*>* inv);
		}; // class InventoryWindow : public hoa_video::MenuWindow

/*!
* \brief Converts a vector of GlobalItem*, etc. to a vector of GlobalObjects*
* \return the same vector, with elements of type GlobalObject*
*/
		template <class T> std::vector<hoa_global::GlobalObject*> InventoryWindow::_GetItemVector(std::vector<T*>* inv) {
			std::vector<hoa_global::GlobalObject*> obj_vector;

			for (typename std::vector<T*>::iterator i = inv->begin(); i != inv->end(); i++) {
				obj_vector.push_back(*i);
			}

			return obj_vector;
		}

	} //end private_menu

} //end hoa_menu