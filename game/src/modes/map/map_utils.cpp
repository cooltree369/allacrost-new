///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode utility code
*** *****************************************************************************/

// Local map mode headers
#include "map_utils.h"
#include "map.h"

using namespace std;

namespace hoa_map {

bool MAP_DEBUG = false;

namespace private_map {

uint16 CalculateOppositeDirection(const uint16 direction) {
	switch (direction) {
		case NORTH:      return SOUTH;
		case SOUTH:      return NORTH;
		case WEST:       return EAST;
		case EAST:       return WEST;
		case NW_NORTH:   return SE_SOUTH;
		case NW_WEST:    return SE_EAST;
		case NE_NORTH:   return SW_SOUTH;
		case NE_EAST:    return SW_WEST;
		case SW_SOUTH:   return NE_NORTH;
		case SW_WEST:    return NE_EAST;
		case SE_SOUTH:   return NW_NORTH;
		case SE_EAST:    return NW_WEST;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "invalid direction argument: " << direction << endl;
			return SOUTH;
	}
}



bool MapRectangle::CheckIntersection(const MapRectangle& first, const MapRectangle& second) {
	if ((first.left > second.right) ||
		(first.right < second.left) ||
		(first.top > second.bottom) ||
		(first.bottom < second.top))
		return false;
	else
		return true;
}

///////////////////////////////////////////////////////////////////////////////
// MapRecordData Class Functions
///////////////////////////////////////////////////////////////////////////////

void MapRecordData::UpdateRecords() {
	if (global_record_name.empty() == false)
		MapMode::CurrentInstance()->GetGlobalRecordGroup()->SetRecord(global_record_name, global_record_value);
	if (local_record_name.empty() == false)
		MapMode::CurrentInstance()->GetLocalRecordGroup()->SetRecord(local_record_name, local_record_value);
}

} // namespace private_map

} // namespace hoa_map

