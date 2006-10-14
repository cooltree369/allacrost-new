////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the global game manager
*** ***************************************************************************/

#include <iostream>

#include "global.h"
#include "utils.h"
#include "video.h"
#include "script.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_utils;



namespace hoa_global {

GameGlobal *GlobalManager = NULL;

bool GLOBAL_DEBUG = false;

SINGLETON_INITIALIZE(GameGlobal);



// ****************************************************************************
// ***** GlobalCharacterParty
// ****************************************************************************

void GlobalCharacterParty::AddCharacter(GlobalCharacter* character) {
	// Make sure that the character is not already in the party
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] == character)
			return;
	}

	_characters.push_back(character);
} // void GlobalCharacterParty::AddCharacter(GlobalCharacter* character)


GlobalCharacter* GlobalCharacterParty::RemoveCharacter(GlobalCharacter* character) {
	GlobalCharacter* removed = NULL;

	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] == character) {
			removed = _characters[i];
		}

		// Shift all other characters in the vector if the character to remove has already been found
		if (removed != NULL) {
			_characters[i-1] = _characters[i];
		}
	}

	// If the removed character was found, remove the last element from the vector
	if (removed != NULL) {
		_characters.pop_back();
	}

	return removed;

} // GlobalCharacterParty* GlobalCharacterParty::RemoveCharacter(GlobalCharacter* character)

// ****************************************************************************
// ***** GameGlobal
// ****************************************************************************

GameGlobal::GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal constructor invoked" << endl;
	
	SetItemName(HP_POTION, "HP Potion");
	SetItemIconPath(HP_POTION, "img/icons/inventory/health_potion.png");
}

GameGlobal::~GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal destructor invoked" << endl;
	for (uint32 i = 0; i < _characters.size(); i++) {
		delete _characters[i];
	}
	
	// Clean up inventory items
	for (uint32 i = 0; i < _inventory.size(); ++i) {
		delete _inventory[i];
	}
}

// Initialize GameGlobal members
bool GameGlobal::SingletonInitialize() {
	return true;
}

void GameGlobal::AddCharacter(GlobalCharacter *ch) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: Adding new character to party: " << MakeStandardString(ch->GetName()) << endl;
	_characters.push_back(ch);
	// Check size of active party if less then 4, add to party
	if (_party.GetPartySize() < 4)
		_party.AddCharacter(ch);
}

GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i]->GetID() == id) {
			return _characters[i];
		}
	}
	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}


//----------------------------------------------
// GameGlobal::GetParty
//----------------------------------------------
vector<GlobalCharacter *> GameGlobal::GetParty()
{
/*
	vector<uint32> characters = _party.GetCharacters();
	vector<GlobalCharacter *> ret;
	for (vector<uint32>::iterator p = characters.begin(); p != characters.end(); ++p)
		ret.push_back(GetCharacter((*p)));
	
	return ret;
*/
	return _party.GetCharacters();
}



//-------------------------------------
// GlobalCharacterParty::AddItemToInventory
//-------------------------------------
void GameGlobal::AddItemToInventory(GlobalObject *obj)
{
	vector<GlobalObject *>::iterator i = _inventory.begin();
	for (; i != _inventory.end(); i++)
	{
		if ((*i)->GetID() == obj->GetID())
		{
			(*i)->SetCount((*i)->GetCount() + 1);
			return;
		}
	}
	_inventory.push_back(obj);
}

void GameGlobal::RemoveFromInventory(GlobalObject *obj)
{
	for (vector<GlobalObject*>::iterator i = _inventory.begin(); i != _inventory.end(); i++)
	{
		if ((*i)->GetID() == obj->GetID())
		{
			cout << "removing item: " << *i << endl;
			delete *i;
			// TEMP: this just pops out the last element, not the element requested!
			_inventory.pop_back();
			return;
		}
	}

	cerr << "GLOBAL ERROR: requested to remove an inventory item that was not in the inventory" << endl;
}

} // namespace hoa_global
