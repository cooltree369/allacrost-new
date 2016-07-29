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

#include "map_utils.h"

// Local map mode headers
#include "map.h"
#include "map_events.h"

using namespace std;

using namespace hoa_common;

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

void MapRecordData::CommitRecords() {
	CommonRecordGroup* global_group = MapMode::CurrentInstance()->GetGlobalRecordGroup();
	CommonRecordGroup* local_group = MapMode::CurrentInstance()->GetLocalRecordGroup();

	for (uint32 i = 0; i < _global_records.size(); ++i) {
		global_group->SetRecord(_global_records[i].first, _global_records[i].second);
	}
	for (uint32 i = 0; i < _local_records.size(); ++i) {
		local_group->SetRecord(_local_records[i].first, _local_records[i].second);
	}
}

///////////////////////////////////////////////////////////////////////////////
// MapEventData Class Functions
///////////////////////////////////////////////////////////////////////////////

void MapEventData::AddEvent(uint32 event_id, uint32 start_timing, bool launch_at_start) {
	if (event_id == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to add an event with an invalid ID (0). The event was not added" << endl;
		return;
	}

	_event_ids.push_back(event_id);
	_start_timings.push_back(start_timing);
	_launch_start.push_back(launch_at_start);
}


void MapEventData::StartEvents(bool launch_start) const {
	for (uint32 i = 0; i < _event_ids.size(); ++i) {
		if (_launch_start[i] == launch_start) {
			if (_start_timings[i] == 0) {
				MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_event_ids[i]);
			}
			else {
				MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_event_ids[i], _start_timings[i]);
			}
		}
	}
}



bool MapEventData::ValidateEvents() const {
	bool return_value = true;

	for (uint32 i = 0; i < _event_ids.size(); ++i) {
		if (MapMode::CurrentInstance()->GetEventSupervisor()->GetEvent(_event_ids[i]) == NULL) {
			return_value = false;
			IF_PRINT_WARNING(MAP_DEBUG) << "no event was registered for the event ID: " << _event_ids[i] << endl;
		}
	}

	return return_value;
}

} // namespace private_map

} // namespace hoa_map

