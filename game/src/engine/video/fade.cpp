///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <cstdarg>
#include <math.h>

#include "fade.h"
#include "video.h"

using namespace std;

using namespace hoa_utils;

namespace hoa_video {

namespace private_video {

ScreenFader::ScreenFader() :
	_current_color(0.0f, 0.0f, 0.0f, 0.0f),
	_initial_color(0.0f, 0.0f, 0.0f, 0.0f),
	_final_color(0.0f, 0.0f, 0.0f, 0.0f),
	_current_time(0),
	_end_time(0),
	_is_fading(false),
	_use_fade_overlay(false),
	_fade_overlay_color(0.0f, 0.0f, 0.0f, 0.0f),
	_fade_modulation(1.0f),
	_interpolate_rgb_values(false)
{
	_fade_image.Load("", 1024.0f, 768.0f);
}



void ScreenFader::BeginFade(const Color &final, uint32 time) {
	_is_fading = true;
	_end_time = time;

	_initial_color = _current_color;
	_final_color = final;
	_current_time = 0;

	// Figure out if this is a simple fade or if an overlay is required
	// A simple fade is defined as a fade from clear to black, from black
	// to clear, or from somewhere between clear and black to either clear
	// or black. More simply, it's a fade where both the initial and final
	// color's RGB values are zeroed out
	_use_fade_overlay = true;
	if ((IsFloatEqual(_initial_color[0], 0.0f) && IsFloatEqual(_initial_color[1], 0.0f)
		&& IsFloatEqual(_initial_color[2], 0.0f) && IsFloatEqual(_final_color[0], 0.0f)
		&& IsFloatEqual(_final_color[1], 0.0f) && IsFloatEqual(_final_color[2], 0.0f)))
	{
		_use_fade_overlay = false;
	}
	else {
		_fade_modulation = 1.0f;
	}

	// If we are fading to or from transparent, then the RGB values do not need to be interpolated
	if (IsFloatEqual(_final_color[3], 0.0f)) {
		_interpolate_rgb_values = true;
		_current_color[0] = _initial_color[0];
		_current_color[1] = _initial_color[1];
		_current_color[2] = _initial_color[2];
	}
	else if (IsFloatEqual(_initial_color[3], 0.0f)) {
		_interpolate_rgb_values = true;
		_current_color[0] = _final_color[0];
		_current_color[1] = _final_color[1];
		_current_color[2] = _final_color[2];
	}
	else {
		_interpolate_rgb_values = false;
	}

	Update(0); // Do an initial update
} // void ScreenFader::BeginFade(const Color &final, uint32 time)



void ScreenFader::Update(uint32 time) {
	if (_is_fading == false)
		return;

	// Check for fading finish condition
	if (_current_time >= _end_time) {
		_current_color = _final_color;
		_is_fading = false;

		if (_use_fade_overlay == true) {
			// Check if we have faded to black or clear. If so, we can use modulation
			if (IsFloatEqual(_final_color[3], 0.0f) || (IsFloatEqual(_final_color[0], 0.0f)
				&& IsFloatEqual(_final_color[1], 0.0f) && IsFloatEqual(_final_color[2], 0.0f)))
			{
				_use_fade_overlay = false;
				_fade_modulation = 1.0f - _final_color[3];
			}
		}
		else {
			_fade_modulation = 1.0f - _final_color[3];
		}

		return;
	}

	// Calculate the new interpolated color
	float percent_complete = static_cast<float>(_current_time) / static_cast<float>(_end_time);

	if (_interpolate_rgb_values == true) {
		_current_color[0] = Lerp(percent_complete, _initial_color[0], _final_color[0]);
		_current_color[1] = Lerp(percent_complete, _initial_color[1], _final_color[1]);
		_current_color[2] = Lerp(percent_complete, _initial_color[2], _final_color[2]);
	}
	_current_color[3] = Lerp(percent_complete, _initial_color[3], _final_color[3]);

	if (_use_fade_overlay == false)
		_fade_modulation = 1.0f - _current_color[3];
	else
		_fade_overlay_color = _current_color;

	_current_time += time;
	_fade_image.SetColor(_current_color);
} // void FadeScreen::Update(uint32 time)


void ScreenFader::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->PushState();
	VideoManager->Move(0.0f, 0.0f);
	_fade_image.Draw();
	VideoManager->PopState();
}


} // namespace private_video

}  // namespace hoa_video
