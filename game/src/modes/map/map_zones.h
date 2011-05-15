///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_zones.h
*** \author  Guillaume Anctil, drakkoon@allacrost.org
*** \brief   Header file for map mode zones.
*** ***************************************************************************/

#ifndef __MAP_ZONES_HEADER__
#define __MAP_ZONES_HEADER__

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engine
#include "system.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Represents a rectangular area on a map.
***
*** The area is represented by the coordinates of the top-left and bottom-right
*** corners. Both are represented in the row / column format of collision grid
*** elements. Zone sections can only include entire grid elements, not portions
*** of an element.
***
*** \note The primary intent of this class is to be able to combine several
*** ZoneSections to create a non-rectangular shape. This is how a map zone is
*** formed.
*** ***************************************************************************/
class ZoneSection {
public:
	ZoneSection(uint16 left, uint16 right, uint16 top, uint16 bottom) :
		left_col(left), right_col(right), top_row(top), bottom_row(bottom)
		{}

	//! \brief Collision grid columns for the top and bottom section of the area
	uint16 left_col, right_col;

	//! \brief Collision grid rows for the top and bottom section of the area
	uint16 top_row, bottom_row;
}; // class ZoneSection


/** ****************************************************************************
*** \brief Represents a zone on a map that can take any shape
***
*** The area is made up of many ZoneSection instances, so it can be any shape
*** (specifically, any combination of rectangular shapes). A MapZone itself
*** is not very useful, but serves as a base for other derived classes. Note
*** that a MapZone may be relevant in only certain map contexts but not others.
***
*** \note ZoneSections in the MapZone may overlap without any problem. In general,
*** however, you should try to create a MapZone using as few ZoneSections as possible
*** to improve performance.
*** ***************************************************************************/
class MapZone {
	// This friend declaration is necessary because EnemyZone, although it dervies from MapZone, also keeps a pointer
	// to a MapZone object and needs to access the protected members and methods of this object pointer.
	friend class EnemyZone;

public:
	MapZone()
		{}

	/** \brief Constructs a map zone that is initialized with a single zone section
	*** \param left_col The left edge of the section to add
	*** \param right_col The right edge of the section to add
	*** \param top_row The top edge of the section to add
	*** \param bottom_row The bottom edge of the section to add
	**/
	MapZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row);

	virtual ~MapZone()
		{}

	/** \brief Adds a new zone section to the map zone
	*** \param left_col The left edge of the section to add
	*** \param right_col The right edge of the section to add
	*** \param top_row The top edge of the section to add
	*** \param bottom_row The bottom edge of the section to add
	***
	*** \note The value of left shoult be less than right, and top should be less that
	*** bottom. If these conditions are not true, a warning will be printed and the section
	*** will not be added.
	**/
	virtual void AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row);

	/** \brief Returns true if the position coordinates are located inside the zone (inclusive to the zone boundary edges)
	*** \param pos_x The x position to check
	*** \param pos_y The y position to check
	***
	*** \note This function ignores the fractional part of map coordinates for performance reasons. So whenever an object is being
	*** checked as to whether or not it may be found in this zone, the floating point portion of its map coordinates are not taken
	*** into account.
	**/
	bool IsInsideZone(uint16 pos_x, uint16 pos_y);

	//! \brief Updates the state of the zone
	virtual void Update()
		{}

protected:
	//! \brief The rectangular sections which compose the map zone
	std::vector<ZoneSection> _sections;

	/** \brief Returns random x, y position coordinates within the zone
	*** \param x A reference where to store the value of the x position
	*** \param y A reference where to store the value of the x position
	**/
	void _RandomPosition(uint16& x, uint16& y);
}; // class MapZone


/** ****************************************************************************
*** \brief Represents an area where enemy sprites spawn and roam in.
***
*** This zone will spawn enemy sprites somewhere within its boundaries. It also
*** regenerates dead enemies after a certain amount of time. The enemies can be
*** constrained within the zone area or be made free to roam the entire map
*** after spawning. If desired, the class has the option to declare seperate zones
*** for where enemies may spawn and where they may roam.
***
*** \note It makes no sense to use seperate zones for spawning if the enemies are
*** not restrained in their roaming. If free roaming is enabled for this zone, construct
*** the zone sections the standard way.
***
*** \note By default, enemies are restricted to move about within their zones and
*** a default regeneration timer is used. These properties can be changed after
*** the class object is constructed.
***
*** \todo Consider adding a call that will disable additional enemies from spawning in
*** the zone, or limit the maximum number of enemies that may ever spawn in the zone.
*** ***************************************************************************/
class EnemyZone : public MapZone {
public:
	EnemyZone();

	/** \brief Constructs an enemy zone that is initialized with a single zone section
	*** \param left_col The left edge of the section to add
	*** \param right_col The right edge of the section to add
	*** \param top_row The top edge of the section to add
	*** \param bottom_row The bottom edge of the section to add
	**/
	EnemyZone(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row);

	~EnemyZone()
		{ if (_spawn_zone != NULL) delete _spawn_zone; }

	EnemyZone(const EnemyZone& copy);

	EnemyZone& operator=(const EnemyZone& copy);

	/** \brief Adds a new enemy sprite to the zone
	*** \param enemy A pointer to the EnemySprite object instance to add
	*** \param map A pointer to the MapMode instance to add the EnemySprite to
	*** \param count The number of copies of this enemy to add
	**/
	void AddEnemy(EnemySprite* enemy, MapMode* map, uint8 count = 1);

	/** \brief Adds a new zone section to the zone where enemies may spawn
	*** \param left_col The left edge of the section to add
	*** \param right_col The right edge of the section to add
	*** \param top_row The top edge of the section to add
	*** \param bottom_row The bottom edge of the section to add
	***
	*** Calling this method automatically enables this zone to have seperate areas for spawning
	*** and roaming. Upon successfully adding a single spawn section, enemies will no longer be
	*** allowed to spawn in the sections added directly to the class object. Each spawn zone section
	*** must reside completely inside a roaming zone section for it to be added successfully. The reason
	*** for this is that we do not want to allow enemies to spawn at a location where they are "stuck" and
	*** can't roam anywhere. And for zones that do not restrict roaming, it makes no sense to add zones
	*** specific to spawning in the first place.
	***
	*** \note The value of left shoult be less than right, and top should be less that
	*** bottom. If these conditions are not true, a warning will be printed and the section
	*** will not be added.
	**/
	void AddSpawnSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row);

	//! \brief Decrements the number of active enemies by one
	void EnemyDead();

	//! \brief Gradually spawns enemy sprites in the zone
	void Update();

	//! \brief Returns true if this zone has seperate zones for roaming and spawning
	bool HasSeparateSpawnZone() const
		{ return (_spawn_zone != NULL); }

	//! \name Class Member Access Functions
	//@{
	bool IsRoamingRestrained() const
		{ return _roaming_restrained; }

	uint32 GetSpawnTime() const
		{ return _spawn_timer.GetDuration(); }

	void SetRoamingRestrained(bool restrain)
		{ _roaming_restrained = restrain; }

	//! \note Calling this function will reset the elapsed spawn time
	void SetSpawnTime(uint32 time)
		{ _spawn_timer.Reset(); _spawn_timer.SetDuration(time); _spawn_timer.Run(); }
	//@}

private:
	//! \brief If true, enemies of this zone are not allowed to roam outside of the zone boundaries
	bool _roaming_restrained;

	//! \brief The number of enemies that are currently not in the DEAD state
	uint8 _active_enemies;

	//! \brief Used for the respawning of enemies within the zone
	hoa_system::SystemTimer _spawn_timer;

	//! \brief An optional zone which specifies where enemies may spawn
	MapZone* _spawn_zone;

	/** \brief Contains all of the enemies that may exist in this zone.
	*** \note These sprites will be deleted by the map object manager, not the destructor of this class.
	**/
	std::vector<EnemySprite*> _enemies;
}; // class EnemyZone : public MapZone


/** ****************************************************************************
*** \brief Represents an area where the active map context may switch
***
*** This type of zone enables map sprites to transfer betweeen two map contexts.
*** Each zone section added is labeled as corresponding to one context or the
*** other. When a sprite stands upon a particular section, their context will
*** be set to the context of that section.
***
*** \todo In the future collision detection needs to be accounted for when two
*** objects are in the context zone but have different active map contexts.
*** Normally no collision detection is done between objects in different contexts,
*** but context zones need to be an exception to this rule.
***
*** \todo Currently the Update() function checks all ground objects to determine
*** if any context changes need to occur. This is a temporarily solution that needs
*** to be improved by the following:
***  - The class should have a container of objects currently located within the zone,
***    and check only those objects (when sprites are in motion, they can check if they
***    have stepped into a context zone there)
***  - Sky objects should also be able to change their context via context zones
***  - There should be an option for having the context zone not to apply to either the
***    ground or sky object layers
*** ***************************************************************************/
class ContextZone : public MapZone {
public:
	/** \brief The constructor requires the map contexts of the zone to be declared immediately
	*** \note These two context arguments can not be equal
	**/
	ContextZone(MAP_CONTEXT one, MAP_CONTEXT two);

	/** \brief Overrides the virtual base method
	***
	*** This method should not be called for this class as zone sections which are added need to be
	*** instructed as to which context they should belong to. The only reason this method is defined
	*** here is to override the MapZone class' base method. This function prints a warning and will not
	*** add the new section
	**/
	void AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row);

	/** \brief Adds a new rectangular section to the zone
	*** \param left_col The left edge of the section to add
	*** \param right_col The right edge of the section to add
	*** \param top_row The top edge of the section to add
	*** \param bottom_row The bottom edge of the section to add
	*** \param context True indicates that the new section belongs to context one, false to context two
	**/
	void AddSection(uint16 left_col, uint16 right_col, uint16 top_row, uint16 bottom_row, bool context);

	//! \brief Updates the active contexts of any map objects that exist within the zone
	void Update();

private:
	//! \brief The different map contexts that the context zone allows an object to transition between
	MAP_CONTEXT _context_one, _context_two;

	//! \brief Stores the context of each zone section. True indicates context one, false is context two
	std::vector<bool> _section_contexts;

	/** \brief Determines if a map object is inside the context zone
	*** \param object A pointer to the map object
	*** \return The index of the zone section where the object is located, or -1 if it is not in the zone
	**/
	int16 _IsInsideZone(MapObject* object);
}; // class ContextZone : public MapZone

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_ZONES_HEADER__
