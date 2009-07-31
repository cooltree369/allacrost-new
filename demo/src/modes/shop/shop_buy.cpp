///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_buy.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for buy menus of shop mode
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "input.h"
#include "video.h"

#include "global.h"

#include "shop.h"
#include "shop_buy.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** BuyInterface class methods
// *****************************************************************************

BuyInterface::BuyInterface() :
	_current_datalist(0),
	_list_window(NULL),
	_info_window(NULL)
{
	_list_window = ShopMode::CurrentInstance()->GetListWindow();
	_info_window = ShopMode::CurrentInstance()->GetInfoWindow();
}



BuyInterface::~BuyInterface() {
	for (uint32 i = 0; i < _object_lists.size(); i++) {
		delete _object_lists[i];
	}
}



void BuyInterface::Initialize() {
	// Used to temporarily hold a pointer to a valid shop object
	ShopObject* obj = NULL;
	// Pointer to the container of all objects that are bought/sold/traded in the ship
	map<uint32, ShopObject>* shop_objects = ShopMode::CurrentInstance()->GetShopObjects();
	// Bit-vector that indicates what types of objects are sold in the shop
	uint8 obj_types = ShopMode::CurrentInstance()->GetDealTypes();
	// The number of object categories in this buy menu (not including the "all" category)
	uint8 num_obj_categories = 0;
	// Holds the index within the _object_data vector where the container for a specific object type is
	vector<uint32> type_index(8, 0);

	// ---------- (1): Populating the _object_data container with an entry for each type of object dealt in the shop
	_object_data.push_back(vector<ShopObject*>()); // This first entry represents all objects
	uint32 next_index = 1; // Used to set the appropriate data in the type_index vector
	uint8 bit_x = 0x01; // Used to do a bit-by-bit analysis of the obj_types variable
	for (uint32 i = 0; i < type_index.size(); i++, bit_x <<= 1) {
		// Check if the type is available by doing a bit-wise comparison
		if (obj_types & bit_x) {
			num_obj_categories++;
			type_index[i] = next_index++;
			_object_data.push_back(vector<ShopObject*>());
		}
	}

	// ---------- (2): Populate the object containers
	for (map<uint32, ShopObject>::iterator i = shop_objects->begin(); i != shop_objects->end(); i++) {
		obj = &(i->second);

		if (obj->IsSoldInShop() == true) {
			_object_data[0].push_back(obj);

			switch (obj->GetObject()->GetObjectType()) {
				case GLOBAL_OBJECT_ITEM:
					_object_data[type_index[0]].push_back(obj);
					break;
				case GLOBAL_OBJECT_WEAPON:
					_object_data[type_index[1]].push_back(obj);
					break;
				case GLOBAL_OBJECT_HEAD_ARMOR:
					_object_data[type_index[2]].push_back(obj);
					break;
				case GLOBAL_OBJECT_TORSO_ARMOR:
					_object_data[type_index[3]].push_back(obj);
					break;
				case GLOBAL_OBJECT_ARM_ARMOR:
					_object_data[type_index[4]].push_back(obj);
					break;
				case GLOBAL_OBJECT_LEG_ARMOR:
					_object_data[type_index[5]].push_back(obj);
					break;
				case GLOBAL_OBJECT_SHARD:
					_object_data[type_index[6]].push_back(obj);
					break;
				case GLOBAL_OBJECT_KEY_ITEM:
					_object_data[type_index[7]].push_back(obj);
					break;
				default:
					IF_PRINT_WARNING(SHOP_DEBUG) << "added object of unknown type: " << obj->GetObject()->GetObjectType() << endl;
					break;
			}
		}
	}

	// ---------- (3): Create the buy object lists using the object data that is now ready
	for (uint32 i = 0; i < _object_data.size(); i++) {
		BuyList* new_list = new BuyList();
		new_list->identifier_list.SetOwner(_list_window);
		new_list->properties_list.SetOwner(_list_window);
		new_list->PopulateList(&(_object_data[i]));

		_object_lists.push_back(new_list);
	}

	// ---------- (4): Initialize the list headers and object type icons
	_identifier_header.SetOwner(_list_window);
	_identifier_header.SetPosition(100.0f, 370.0f);
	_identifier_header.SetDimensions(400.0f, 30.0f, 1, 1, 1, 1);
	_identifier_header.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_identifier_header.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_identifier_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_identifier_header.AddOption(MakeUnicodeString("Name"));

	_properties_header.SetOwner(_list_window);
	_properties_header.SetPosition(500.0f, 370.0f);
	_properties_header.SetDimensions(250.0f, 30.0f, 4, 1, 4, 1);
	_properties_header.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	_properties_header.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_properties_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_properties_header.AddOption(MakeUnicodeString("Price"));
	_properties_header.AddOption(MakeUnicodeString("Stock"));
	_properties_header.AddOption(MakeUnicodeString("Own"));
	_properties_header.AddOption(MakeUnicodeString("Buy"));

	const vector<StillImage>& category_images = ShopMode::CurrentInstance()->GetObjectCategoryImages();
	if (num_obj_categories == 1) {
		num_obj_categories++;
	}
	_category_list.SetOwner(_list_window);
	_category_list.SetPosition(30.0f, 370.0f);
	_category_list.SetDimensions(60.0f, 360.0f, 1, num_obj_categories, 1, num_obj_categories);
	_category_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_category_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_category_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	_category_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	for (uint32 i = 0; i < 8; i++) {
		if (obj_types & (0x01 << i)) {
			uint32 this_option_index = _category_list.GetNumberOptions();
			_category_list.AddOption();
			_category_list.AddOptionElementImage(this_option_index, &(category_images[i]));
			_category_list.GetEmbeddedImage(this_option_index)->SetDimensions(45.0f, 45.0f);
		}
	}
}



void BuyInterface::MakeActive() {
	_list_window->Show();
	_info_window->Show();
}



void BuyInterface::MakeInactive() {
	_list_window->Hide();
	_info_window->Hide();
}



void BuyInterface::Update() {
	BuyList* selected_list = _object_lists[_current_datalist];
	uint32 selected_entry = selected_list->identifier_list.GetSelection();
	ShopObject* selected_object = _object_data[_current_datalist][selected_entry];

	if (InputManager->ConfirmPress()) {
		// TODO: Bring up an "instant purchase" confirmation menu
	}
	else if (InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}

	// Up/down changes the selected object in the current list
	else if (InputManager->UpPress()) {
		selected_list->identifier_list.InputUp();
		selected_list->properties_list.InputUp();
	}
	else if (InputManager->DownPress()) {
		selected_list->identifier_list.InputDown();
		selected_list->properties_list.InputDown();
	}

	// Left/right change the quantity of the object to buy
	else if (InputManager->LeftPress()) {
		if (selected_object->GetBuyCount() == 0) {
			ShopMode::CurrentInstance()->GetSound("bump")->Play();
		}
		else {
			selected_object->DecrementBuyCount();
			selected_list->RefreshEntry(selected_entry);
			ShopMode::CurrentInstance()->GetSound("cancel")->Play();
		}
	}
	else if (InputManager->RightPress()) {
		if (selected_object->GetBuyCount() >= selected_object->GetStockCount()) {
			ShopMode::CurrentInstance()->GetSound("bump")->Play();
		}
		else {
			selected_object->IncrementBuyCount();
			selected_list->RefreshEntry(selected_entry);
			ShopMode::CurrentInstance()->GetSound("confirm")->Play();
		}
	}

	// Left select/right select change the category being viewed
	else if (InputManager->LeftSelectPress()) {
		if (GetNumberObjectCategories() > 1) {
			_current_datalist = (_current_datalist == 0) ? (_object_data.size() - 1) : (_current_datalist - 1);
			selected_list = _object_lists[_current_datalist];
			selected_list->RefreshList();
			_UpdateSelectedCategory();
		}
	}
	else if (InputManager->RightSelectPress()) {
		if (GetNumberObjectCategories() > 1) {
			_current_datalist = (_current_datalist >= (_object_data.size() - 1)) ? 0 : (_current_datalist + 1);
			selected_list = _object_lists[_current_datalist];
			selected_list->RefreshList();
			_UpdateSelectedCategory();
		}
	}
}



void BuyInterface::Draw() {
	_list_window->Draw();
	_identifier_header.Draw();
	_properties_header.Draw();
	_category_list.Draw();
	_object_lists[_current_datalist]->Draw();

	_info_window->Draw();
}



void BuyInterface::_UpdateSelectedCategory() {
	if (GetNumberObjectCategories() == 1) {
		return;
	}

	// If the all category is selected, show all of the category icons in full color
	if ((_HasAllCategory() == true) && (_current_datalist == 0)) {
		for (uint32 i = 0; i < _category_list.GetNumberOptions(); i++) {
			_category_list.GetEmbeddedImage(i)->DisableGrayScale();
		}
		return;
	}

	// Otherwise enable grayscale for all unselected object category icons
	for (uint32 i = 0; i < _category_list.GetNumberOptions(); i++) {
		if (i == (_current_datalist - 1)) {
			_category_list.GetEmbeddedImage(i)->DisableGrayScale();
		}
		else {
			_category_list.GetEmbeddedImage(i)->EnableGrayScale();
		}
	}
}

// *****************************************************************************
// ***** BuyList class methods
// *****************************************************************************

BuyList::BuyList() :
	object_data(NULL)
{
	identifier_list.SetPosition(100.0f, 330.0f);
	identifier_list.SetDimensions(400.0f, 300.0f, 1, 255, 1, 8);
	identifier_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	identifier_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	identifier_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	identifier_list.SetCursorOffset(-250.0f, 20.0f);
	identifier_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	identifier_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	properties_list.SetPosition(500.0f, 330.0f);
	properties_list.SetDimensions(250.0f, 300.0f, 4, 255, 4, 8);
	properties_list.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	properties_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	identifier_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	properties_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	properties_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	// NOTE: Both the identifier and properties lists will have SetOwner() called for the menu
	// window that they exist on. This is done by the BuyInterface class shortly after this
	// constructor returns
}



void BuyList::Clear() {
	object_data = NULL;
	identifier_list.ClearOptions();
	properties_list.ClearOptions();
}



void BuyList::PopulateList(vector<ShopObject*>* objects) {
	if (objects == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was given a NULL pointer argument" << endl;
		return;
	}

	object_data = objects;
	RefreshList();
}



void BuyList::RefreshList() {
	if (object_data == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}

	identifier_list.ClearOptions();
	properties_list.ClearOptions();

	ShopObject* shop_obj = NULL;
	for (uint32 i = 0; i < object_data->size(); i++) {
		shop_obj = (*object_data)[i];
		// Add an entry with the icon image of the object (scaled down by 4x to 30x30 pixels) followed by the object name
		identifier_list.AddOption(MakeUnicodeString("<" + shop_obj->GetObject()->GetIconImage().GetFilename() + "><30>")
			+ shop_obj->GetObject()->GetName());
		identifier_list.GetEmbeddedImage(i)->SetDimensions(30.0f, 30.0f);

		// Add an option for each object property in the order of: price, stock, number owned, and amount to buy
		properties_list.AddOption(MakeUnicodeString(NumberToString(shop_obj->GetBuyPrice())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
	}

	identifier_list.SetSelection(0);
	properties_list.SetSelection(0);
}



void BuyList::RefreshEntry(uint32 index) {
	if (object_data == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}
	if (index >= object_data->size()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "index argument was out of range: " << index << endl;
		return;
	}

	ShopObject* shop_obj = (*object_data)[index];
	// Update only the stock, number owned, and amount to buy. The price text does not require updating
	properties_list.SetOptionText((index * 4) + 1, MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
	properties_list.SetOptionText((index * 4) + 2, MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
	properties_list.SetOptionText((index * 4) + 3, MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
}



void BuyList::Update() {
	identifier_list.Update();
	properties_list.Update();
}



void BuyList::Draw() {
	identifier_list.Draw();
	properties_list.Draw();
}

} // namespace private_shop

} // namespace hoa_shop