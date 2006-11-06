////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    context.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for the Context class
***
*** The Context class holds the current state of the video engine. This is
*** used so that the context can be pushed and popped, so that a function which
*** changes a lot of internal settings leaves the video engine in the same state
*** it entered in.
*** ***************************************************************************/

#ifndef __CONTEXT_HEADER__
#define __CONTEXT_HEADER__

#include <string>
#include "utils.h"
#include "color.h"
#include "coord_sys.h"
#include "screen_rect.h"

namespace hoa_video {

namespace private_video {

/** ****************************************************************************
*** \brief Represents the current graphics context
***
*** The grahpics context includes properties such as draw flags, axis
*** transformations and the current coordinate system. The context must be
*** pushed/popped by any GameVideo class function which modifies this context.
***
*** \note Transformations are actually handled separately by the OpenGL
*** transformation stack
*** ***************************************************************************/
class Context {
public:
	//! \brief Flag to indicate whether normal alpha blending is to take place
	int8 blend;
	//! \brief Draw alignment flags to determine where an element is drawn relative to the cursor
	//@{
	int8 x_align;
	int8 y_align;
	//@}
	//! \brief Draw flip flags to determine if an element should be drawn flipped across an axis
	//@{
	int8 x_flip;
	int8 y_flip;
	//@}
	
	//! \brief The coordinate system being used by this context
	CoordSys coordinate_system;
	//! \brief The current font being used by this context
	std::string font;
	//! \brief The current font color being used by this context
	Color text_color;
	//! \brief Defines the screen subset to draw the graphics into
	ScreenRect viewport;
	//! \brief A rectangle to define which portions of the viewport should be cut away when drawing
	ScreenRect scissor_rectangle;
	//! \brief Used to enable or disable the scissoring rectangle
	bool scissoring_enabled;
}; // class Context

} // namespace private_video

} // namespace hoa_video

#endif   // __CONTEXT_HEADER__