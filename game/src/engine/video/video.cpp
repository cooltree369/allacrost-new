///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    video.cpp
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Source file for video engine interface.
*** ***************************************************************************/

#include "video.h"
#include "audio.h"
#include "script.h"
#include "system.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video::private_video;
using namespace hoa_audio;
using namespace hoa_script;
using namespace hoa_system;

template<> hoa_video::VideoEngine* Singleton<hoa_video::VideoEngine>::_singleton_reference = NULL;

namespace hoa_video {

VideoEngine* VideoManager = NULL;
bool VIDEO_DEBUG = false;

//-----------------------------------------------------------------------------
// Static variable for the Color class
//-----------------------------------------------------------------------------

Color Color::clear (0.0f, 0.0f, 0.0f, 0.0f);
Color Color::white (1.0f, 1.0f, 1.0f, 1.0f);
Color Color::gray  (0.5f, 0.5f, 0.5f, 1.0f);
Color Color::black (0.0f, 0.0f, 0.0f, 1.0f);
Color Color::red   (1.0f, 0.0f, 0.0f, 1.0f);
Color Color::orange(1.0f, 0.4f, 0.0f, 1.0f);
Color Color::yellow(1.0f, 1.0f, 0.0f, 1.0f);
Color Color::green (0.0f, 1.0f, 0.0f, 1.0f);
Color Color::aqua  (0.0f, 1.0f, 1.0f, 1.0f);
Color Color::blue  (0.0f, 0.0f, 1.0f, 1.0f);
Color Color::violet(0.0f, 0.0f, 1.0f, 1.0f);
Color Color::brown (0.6f, 0.3f, 0.1f, 1.0f);



float Lerp(float alpha, float initial, float final) {
	return alpha * final + (1.0f - alpha) * initial;
}



void RotatePoint(float& x, float& y, float angle) {
	float original_x = x;
	float cos_angle = cosf(angle);
	float sin_angle = sinf(angle);

	x = x * cos_angle - y * sin_angle;
	y = y * cos_angle + original_x * sin_angle;
}

//-----------------------------------------------------------------------------
// VideoEngine class
//-----------------------------------------------------------------------------

VideoEngine::VideoEngine() :
	_initialized(false)
{
	_target = VIDEO_TARGET_SDL_WINDOW;
	_x_cursor = 0;
	_y_cursor = 0;
	_screen_width = 0;
	_screen_height = 0;
	_fullscreen = false;
	_temp_width = 0;
	_temp_height = 0;
	_temp_fullscreen = false;
	_smooth_textures = true;
	_advanced_display = false;
	_x_shake = 0;
	_y_shake = 0;
	_gamma_value = 1.0f;
	_gl_error_code = GL_NO_ERROR;
	_lightning_active = false;
	_lightning_current_time = 0;
	_lightning_end_time = 0;

	_fps_sum = 0;
	_fps_display = false;
	_current_sample = 0;
	_number_samples = 0;

	_current_context.blend = 0;
	_current_context.x_align = -1;
	_current_context.y_align = -1;
	_current_context.x_flip = 0;
	_current_context.y_flip = 0;
	_current_context.coordinate_system = CoordSys(0.0f, 1023.0f, 0.0f, 767.0f);
	_current_context.viewport = ScreenRect(0, 0, 100, 100);
	_current_context.scissor_rectangle = ScreenRect(0, 0, 1023, 767);
	_current_context.scissoring_enabled = false;

	strcpy(_next_temp_file, "00000000");

	for (uint32 sample = 0; sample < FPS_SAMPLES; sample++)
		 _fps_samples[sample] = 0;

	_light_overlay_enabled = false;
	_ambient_overlay_enabled = false;
	_ambient_x_speed = 0.0f;
	_ambient_y_speed = 0.0f;
	_ambient_x_shift = 0.0f;
	_ambient_y_shift = 0.0f;
	_lightning_active = false;
	_lightning_looped = false;
	_lightning_current_time = 0;
	_lightning_end_time = 0;
	_light_overlay_image.Load("", 1.0f, 1.0f);
	_lightning_overlay_image.Load("", 1.0f, 1.0f);
}


VideoEngine::~VideoEngine() {
	_particle_manager.Destroy();
	TextManager->SingletonDestroy();

	// Unload images prior to destroying the texture manager.
	// Otherwise bad things will happen (memory corruption).
	_default_menu_cursor.Clear();
	_rectangle_image.Clear();
	_light_overlay_image.Clear();
	_ambient_overlay_image.Clear();

	TextureManager->SingletonDestroy();
}



void VideoEngine::DrawFPS(uint32 frame_time) {
	if (!_fps_display)
		return;
	PushState();
	SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);

	// Calculate the FPS for the current frame
	uint32 current_fps = 1000;
	if (frame_time) {
		current_fps /= frame_time;
	}

	// The number of times to insert the current FPS sample into the fps_samples array
	uint32 number_insertions;

	if (_number_samples == 0) {
		// If the FPS display is uninitialized, set the entire FPS array to the current FPS
		_number_samples = FPS_SAMPLES;
		number_insertions = FPS_SAMPLES;
	}
	else if (current_fps >= 500) {
		 // If the game is going at 500 fps or faster, 1 insertion is enough
		number_insertions = 1;
	}
	else {
		// Find if there's a discrepancy between the current frame time and the averaged one.
		// If there's a large difference, add extra samples so the FPS display "catches up" more quickly.
		float avg_frame_time = 1000.0f * FPS_SAMPLES / _fps_sum;
		int32 time_difference = static_cast<int32>(avg_frame_time) - static_cast<int32>(frame_time);

		if (time_difference < 0)
			time_difference = -time_difference;

		if (time_difference <= static_cast<int32>(MAX_FTIME_DIFF))
			number_insertions = 1;
		else
			number_insertions = FPS_CATCHUP; // Take more samples to catch up to the current FPS
	}

	// Insert the current_fps samples into the fps_samples array for the number of times specified
	for (uint32 j = 0; j < number_insertions; j++) {
		_fps_sum -= _fps_samples[_current_sample];
		_fps_sum += current_fps;
		_fps_samples[_current_sample] = current_fps;
		_current_sample = (_current_sample + 1) % FPS_SAMPLES;
	}

	uint32 avg_fps = _fps_sum / FPS_SAMPLES;

	// The text to display to the screen
	char fps_text[16];
	sprintf(fps_text, "FPS: %d", avg_fps);

	Move(930.0f, 720.0f); // Upper right hand corner of the screen
	Text()->Draw(fps_text, TextStyle("text20", Color::white));
	PopState();
} // void GUISystem::_DrawFPS(uint32 frame_time)



bool VideoEngine::SingletonInitialize() {
	// check to see if the singleton is already initialized
	if (_initialized)
		return true;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		PRINT_ERROR << "SDL video initialization failed" << endl;
		return false;
	}



	return true;
} // bool VideoEngine::SingletonInitialize()



bool VideoEngine::FinalizeInitialization() {
	// Create instances of the various sub-systems
	TextureManager = TextureController::SingletonCreate();
	TextManager = TextSupervisor::SingletonCreate();

	// Initialize all sub-systems
	if (TextureManager->SingletonInitialize() == false) {
		PRINT_ERROR << "could not initialize texture manager" << endl;
		return false;
	}

	if (TextManager->SingletonInitialize() == false) {
		PRINT_ERROR << "could not initialize text manager" << endl;
		return false;
	}

	if (SetDefaultCursor("img/menus/cursor.png") == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: problem loading default menu cursor" << endl;
	}

	// Prepare the screen for rendering
	Clear();
	Display(0);
	Clear();

	// TEMP: this is a hack and should be removed when we can support procedural images
	if (_rectangle_image.Load("") == false) {
		PRINT_ERROR << "_rectangle_image could not be created" << endl;
		return false;
	}

	_initialized = true;
	return true;
}



void VideoEngine::SetInitialResolution(int32 width, int32 height) {
	// Get the current system color depth and resolution
	const SDL_VideoInfo* video_info(0);
	video_info = SDL_GetVideoInfo();

	if (video_info) {
		// Set the resolution to be the highest possible (lower than the user one)
		if (video_info->current_w >= width && video_info->current_h >= height) {
			SetResolution(width, height);
		}
		else if (video_info->current_w >= 1024 && video_info->current_h >= 768) {
			SetResolution(1024, 768);
		}
		else if (video_info->current_w >= 800 && video_info->current_h >= 600) {
			SetResolution(800, 600);
		}
		else {
			SetResolution(640, 480);
		}
	}
	else {
		// Default resoltion if we could not retrieve the resolution of the user
		SetResolution(width, height);
	}
}

//-----------------------------------------------------------------------------
// VideoEngine class - General methods
//-----------------------------------------------------------------------------

void VideoEngine::SetTarget(VIDEO_TARGET target) {
	if (target <= VIDEO_TARGET_INVALID || target >= VIDEO_TARGET_TOTAL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to set video engine to an invalid target: " << target << endl;
		return;
	}

	_target = target;
}



void VideoEngine::SetDrawFlags(int32 first_flag, ...) {
	int32 flag = first_flag;
	va_list args;

	va_start(args, first_flag);
	while (flag != 0) {
		switch (flag) {
		case VIDEO_X_LEFT: _current_context.x_align = -1; break;
		case VIDEO_X_CENTER: _current_context.x_align = 0; break;
		case VIDEO_X_RIGHT: _current_context.x_align = 1; break;

		case VIDEO_Y_TOP: _current_context.y_align = 1; break;
		case VIDEO_Y_CENTER: _current_context.y_align = 0; break;
		case VIDEO_Y_BOTTOM: _current_context.y_align = -1; break;

		case VIDEO_X_NOFLIP: _current_context.x_flip = 0; break;
		case VIDEO_X_FLIP: _current_context.x_flip = 1; break;

		case VIDEO_Y_NOFLIP: _current_context.y_flip = 0; break;
		case VIDEO_Y_FLIP: _current_context.y_flip = 1; break;

		case VIDEO_NO_BLEND: _current_context.blend = 0; break;
		case VIDEO_BLEND: _current_context.blend = 1; break;
		case VIDEO_BLEND_ADD: _current_context.blend = 2; break;

		default:
			IF_PRINT_WARNING(VIDEO_DEBUG) << "Unknown flag in argument list: " << flag << endl;
			break;
		}
		flag = va_arg(args, int32);
	}
	va_end(args);
}



void VideoEngine::Clear() {
	//! \todo glClearColor is a state change operation. It should only be called when the clear color changes
	Clear(Color::black);
}



void VideoEngine::Clear(const Color &c) {
	SetViewport(0.0f, 100.0f, 0.0f, 100.0f);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	TextureManager->_debug_num_tex_switches = 0;

	if (CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << CreateGLErrorString() << endl;
	}
}



void VideoEngine::Display(uint32 frame_time) {
	if (_screen_fader.IsFadeActive() == true)
		_screen_fader.Update(frame_time);
	if (_ambient_overlay_enabled == true)
		_UpdateAmbientOverlay(frame_time);
	if (_lightning_active == true)
		_UpdateLightning(frame_time);
	// Update all particle effects
	_particle_manager.Update(frame_time);

	// Update shaking effect
	PushState();
	SetStandardCoordSys();
	_UpdateShake(frame_time);

	// This must be called before DrawFPS, because we only want to count
	// texture switches related to the game's normal operation, not the
	// ones used to draw the video engine debugging text
	if (_advanced_display)
		_DEBUG_ShowAdvancedStats();

	if (TextureManager->debug_current_sheet >= 0)
		TextureManager->DEBUG_ShowTexSheet();

	DrawFPS(frame_time); // Draw FPS Counter If We Need To

	PopState();

	SDL_GL_SwapBuffers();

} // void VideoEngine::Display(uint32 frame_time)



const std::string VideoEngine::CreateGLErrorString() {
	const GLubyte* error_string = gluErrorString(_gl_error_code);

	if (error_string == NULL)
		return ("Unknown GL error code: " + NumberToString(_gl_error_code));
	else
		return (char*)error_string;
}

//-----------------------------------------------------------------------------
// VideoEngine class - Screen size and resolution methods
//-----------------------------------------------------------------------------

void VideoEngine::GetPixelSize(float& x, float& y) {
	x = fabs(_current_context.coordinate_system.GetRight() - _current_context.coordinate_system.GetLeft()) / _screen_width;
	y = fabs(_current_context.coordinate_system.GetTop() - _current_context.coordinate_system.GetBottom()) / _screen_height;
}



bool VideoEngine::ApplySettings() {
	if (_target == VIDEO_TARGET_SDL_WINDOW) {
		// Losing GL context, so unload images first
		if (TextureManager && TextureManager->UnloadTextures() == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to delete OpenGL textures during a context change" << endl;
		}

		int32 flags = SDL_OPENGL;

		if (_temp_fullscreen == true) {
			flags |= SDL_FULLSCREEN;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 2);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

		if (SDL_SetVideoMode(_temp_width, _temp_height, 0, flags) == false) {
			// RGB values of 1 for each and 8 for depth seemed to be sufficient.
			// 565 and 16 here because it works with them on this computer.
			// NOTE from prophile: this ought to be changed to 5558
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

			if (SDL_SetVideoMode(_temp_width, _temp_height, 0, flags) == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "SDL_SetVideoMode() failed with error: " << SDL_GetError() << endl;

				_temp_fullscreen = _fullscreen;
				_temp_width = _screen_width;
				_temp_height = _screen_height;

				if (TextureManager && _screen_width > 0) { // Test to see if we already had a valid video mode
					TextureManager->ReloadTextures();
				}
				return false;
			}
		}

		// Only now that SDL_SetVideoMode(...) has been called can we make OpenGL calls
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_STENCIL_TEST);
		_current_context.scissoring_enabled = false;
		glDisable(GL_SCISSOR_TEST);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		// Turn off writing to the depth buffer
		glDepthMask(GL_FALSE);

		_screen_width = _temp_width;
		_screen_height = _temp_height;
		_fullscreen = _temp_fullscreen;

		if (TextureManager)
			TextureManager->ReloadTextures();

		return true;
	} // if (_target == VIDEO_TARGET_SDL_WINDOW)

	// Used by the Allacrost editor, which uses QT4
	else if (_target == VIDEO_TARGET_QT_WIDGET) {
		_screen_width = _temp_width;
		_screen_height = _temp_height;
		_fullscreen = _temp_fullscreen;

		return true;
	}

	return false;
} // bool VideoEngine::ApplySettings()

//-----------------------------------------------------------------------------
// VideoEngine class - Coordinate system and viewport methods
//-----------------------------------------------------------------------------

void VideoEngine::SetViewport(float left, float right, float bottom, float top) {
	if (left > right) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "left argument was greater than right argument" << endl;
		return;
	}
	if (bottom > top) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "bottom argument was greater than top argument" << endl;
		return;
	}

	int32 l = static_cast<int32>(left * _screen_width * .01f);
	int32 b = static_cast<int32>(bottom * _screen_height * .01f);
	int32 r = static_cast<int32>(right * _screen_width * .01f);
	int32 t = static_cast<int32>(top * _screen_height * .01f);

	if (l < 0)
		l = 0;
	if (b < 0)
		b = 0;
	if (r > _screen_width)
		r = _screen_width;
	if (t > _screen_height)
		t = _screen_height;

	_current_context.viewport = ScreenRect(l, b, r - l, t - b);
	glViewport(l, b, r - l, t - b);
}



void VideoEngine::SetCoordSys(const CoordSys& coordinate_system) {
	_current_context.coordinate_system = coordinate_system;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(_current_context.coordinate_system.GetLeft(), _current_context.coordinate_system.GetRight(),
		_current_context.coordinate_system.GetBottom(), _current_context.coordinate_system.GetTop(), -1, 1);

 	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// This small translation is supposed to help with pixel-perfect 2D rendering in OpenGL.
	// Reference: http://www.opengl.org/resources/faq/technical/transformations.htm#tran0030
	glTranslatef(0.375, 0.375, 0);
}



void VideoEngine::EnableScissoring() {
	_current_context.scissoring_enabled = true;
	glEnable(GL_SCISSOR_TEST);
}



void VideoEngine::DisableScissoring() {
	_current_context.scissoring_enabled = false;
	glDisable(GL_SCISSOR_TEST);
}



void VideoEngine::SetScissorRect(float left, float right, float bottom, float top) {
	_current_context.scissor_rectangle = CalculateScreenRect(left, right, bottom, top);

	glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
		static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height),
		static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
		static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height)
	);
}



void VideoEngine::SetScissorRect(const ScreenRect& rect) {
	_current_context.scissor_rectangle = rect;

	glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
		static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height),
		static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
		static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height)
	);
}



ScreenRect VideoEngine::CalculateScreenRect(float left, float right, float bottom, float top) {
	ScreenRect rect;

	int32 scr_left = _ScreenCoordX(left);
	int32 scr_right = _ScreenCoordX(right);
	int32 scr_bottom = _ScreenCoordY(bottom);
	int32 scr_top = _ScreenCoordY(top);

	int32 temp;
	if (scr_left > scr_right) {
		temp = scr_left;
		scr_left = scr_right;
		scr_right = temp;
	}

	if (scr_top > scr_bottom) {
		temp = scr_top;
		scr_top = scr_bottom;
		scr_bottom = temp;
	}

	rect.top = scr_top;
	rect.left = scr_left;
	rect.width = scr_right - scr_left;
	rect.height = scr_bottom - scr_top;

	return rect;
}

//-----------------------------------------------------------------------------
// VideoEngine class - Transformation methods
//-----------------------------------------------------------------------------

void VideoEngine::Move(float x, float y) {
	glLoadIdentity();
	glTranslatef(x, y, 0);
	_x_cursor = x;
	_y_cursor = y;
}



void VideoEngine::MoveRelative(float x, float y) {
	glTranslatef(x, y, 0);
	_x_cursor += x;
	_y_cursor += y;
}




void VideoEngine::PushState() {
	// Push current modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	_context_stack.push(_current_context);
}



void VideoEngine::PopState() {
	// Restore the most recent context information and pop it from stack
	if (_context_stack.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no video states were saved on the stack" << endl;
		return;
	}

	_current_context = _context_stack.top();
	_context_stack.pop();
    
	// Restore the coordinate_system, i.e. the GL_PROJECTION matrix
	// NOTE: This will implicitely lead to _current_context.coordinate_system = _current_context.coordinate_system
	// NOTE: Another option is to also push and pop the matrix of GL_PROJECTION. However, the maximum size of the
	//       corresponding stack is only ensured to be 2. Therefore, for portrability the is not performed here.
	SetCoordSys(_current_context.coordinate_system);

	// Restore the modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glViewport(_current_context.viewport.left, _current_context.viewport.top, _current_context.viewport.width, _current_context.viewport.height);

	if (_current_context.scissoring_enabled) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
			static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height),
			static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RESOLUTION_WIDTH)) * _current_context.viewport.width),
			static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RESOLUTION_HEIGHT)) * _current_context.viewport.height)
		);
	}
	else {
		glDisable(GL_SCISSOR_TEST);
	}
}



void VideoEngine::SetTransform(float matrix[16]) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(matrix);
}



void VideoEngine::EnableAmbientOverlay(const string &filename, float x_speed, float y_speed) {
	// Clear any image data before trying to load a new image
	 _ambient_overlay_image.Clear();

	// Note: The StillImage class handles clearing an image when loading another one.
	if (_ambient_overlay_image.Load(filename) == true) {
		_ambient_x_speed = x_speed;
		_ambient_y_speed = y_speed;
		_ambient_overlay_enabled = true;
	}
	else {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to load ambient overlay image: " << filename
			<< ", ambient overlay will be disabled" << endl;
		_ambient_overlay_enabled = false;
	}
}



bool VideoEngine::LoadLightningEffect(const string& file, uint32 effect_number) {
	_lightning_data.clear();
	_lightning_sounds.clear();
	_lightning_current_time = 0;
	_lightning_end_time = 0;
	_next_lightning_sound = 0;
	_lightning_active = false;

	ReadScriptDescriptor script;
	if (script.OpenFile(file) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to open file: " << file
			<< ", the lightning effect will be disabled" << endl;
		_lightning_active = false;
		return false;
	}

	uint32 number_of_effects = script.ReadUInt("number_of_effects");
	if (effect_number > number_of_effects) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to load effect number: " << effect_number
			<< ", because it exceeded the total number of effects: " << number_of_effects << endl;
		_lightning_data.clear();
		_lightning_sounds.clear();
		script.CloseFile();
		return false;
	}

	// Read the light intensity data
	script.OpenTable("light_intensities");
	script.ReadFloatVector(effect_number, _lightning_data);
	script.CloseTable();

	// Check the data to make sure that values are appropriate.
	for (uint32 i = 0; i < _lightning_data.size(); i++) {
		if (_lightning_data[i] < 0.0f) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "for effect number: " << effect_number
				<< "invalid light_intensities data was discovered at index: " << i
				<< " (" << _lightning_data[i] << "), setting value to 0.0f" << endl;
			_lightning_data[i] = 0.0f;
		}
		else if (_lightning_data[i] > 1.0f) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "for effect number: " << effect_number
				<< "invalid light_intensities data was discovered at index: " << i
				<< " (" << _lightning_data[i] << "), setting value to 1.0f" << endl;
			_lightning_data[i] = 1.0f;
		}
	}

	// Read the sounds and sound timing data
	vector<string> sound_filenames;
	vector<uint32> sound_times;
	script.OpenTable("sound_filenames");
	script.ReadStringVector(effect_number, sound_filenames);
	script.CloseTable();
	script.OpenTable("sound_times");
	script.ReadUIntVector(effect_number, sound_times);
	script.CloseTable();

	// Make sure that the table sizes are the same
	if (sound_filenames.size() != sound_times.size()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to load effect number: " << effect_number
			<< ", because the size of the sound_filenames and sound_times tables were unequal" << endl;
		_lightning_data.clear();
		_lightning_sounds.clear();
		script.CloseFile();
		return false;
	}

	// Determine the amount of time the entire sequence will take. Each entry in the data correspodns to 10 milliseconds
	uint32 sequence_time = _lightning_data.size() * 10;

	// Check the sound_timings to make sure that they all occur within the sequence time range
	for (uint32 i = 0; i < sound_times.size(); ++i) {
		if (sound_times[i] > sequence_time) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "for effect number: " << effect_number << ", the total time of the "
				<< "sequence was determined to be " << sequence_time << " milliseconds, but a sound was defined "
				<< "to play at the " << sound_times[i] << " millisecond mark. As a result, this sound will never "
				<< "be played in this sequence" << endl;
		}
	}

	// Now load the sound data into the proper vector container
	_lightning_sounds.resize(sound_filenames.size());
	for (uint32 i = 0; i < _lightning_sounds.size(); ++i) {
		_lightning_sounds[i].filename = sound_filenames[i];
		_lightning_sounds[i].time = sound_times[i];
	}

	// Reset the effect timers
	_lightning_current_time = 0;
	_lightning_end_time = sequence_time;

	script.CloseFile();
	return true;
}



void VideoEngine::EnableLightning(bool loop) {
	if (_lightning_data.empty() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "function called when no lighting effect appeared to be loaded" << endl;
		return;
	}

	_lightning_active = true;
	_lightning_looped = loop;
}



void VideoEngine::DrawLightning() {
	if (_lightning_active == false)
		return;

	// Convert milliseconds elapsed into data points elapsed
	float t = _lightning_current_time / 10.0f;

	int32 rounded_t = static_cast<int32>(t);
	t -= rounded_t;

	// Safety check
	if (rounded_t + 1 >= static_cast<int32>(_lightning_data.size()))
		return;

	// Get two separate data points and blend them together in a linear interpolation
	float data1 = _lightning_data[rounded_t];
	float data2 = _lightning_data[rounded_t + 1];
	float intensity = data1 * (1 - t) + data2 * t;

	PushState();
	SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	Move(0.0f, 0.0f);
	_lightning_overlay_image.Draw(Color(1.0f, 1.0f, 1.0f, intensity));
	PopState();
}



void VideoEngine::DrawOverlays() {
	PushState();
	SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	// Draw the textured ambient overlay
	if (_ambient_overlay_enabled == true) {
		SetCoordSys(0.0f, VIDEO_STANDARD_RESOLUTION_WIDTH, 0.0f, VIDEO_STANDARD_RESOLUTION_HEIGHT);
		float width = _ambient_overlay_image.GetWidth();
		float height = _ambient_overlay_image.GetHeight();
		for (float x = _ambient_x_shift; x <= VIDEO_STANDARD_RESOLUTION_WIDTH; x = x + width) {
			for (float y = _ambient_y_shift; y <= VIDEO_STANDARD_RESOLUTION_HEIGHT; y = y + height) {
				Move(x, y);
				_ambient_overlay_image.Draw();
			}
		}
	}

	SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);

	// Draw the light overlay
	if (_light_overlay_enabled == true) {
		Move(0.0f, 0.0f);
		_light_overlay_image.Draw();
	}

	// Draw the lightning overlay
	if (_lightning_active) {
		DrawLightning();
	}

	// Draw a screen overlay if we are in the process of fading
	if (_screen_fader.IsFadeActive() == true) {
		_screen_fader.Draw();
	}

	PopState();
}



StillImage VideoEngine::CaptureScreen() throw(Exception) {
	// Static variable used to make sure the capture has a unique name in the texture image map
	static uint32 capture_id = 0;

	StillImage screen_image;

	// TEMP: temporary resolution until capture screen bug is fixed
// 	return screen_image;

	// Retrieve width/height of the viewport. viewport_dimensions[2] is the width, [3] is the height
	GLint viewport_dimensions[4];
	glGetIntegerv(GL_VIEWPORT, viewport_dimensions);
	screen_image.SetDimensions((float)viewport_dimensions[2], (float)viewport_dimensions[3]);

	// Set up the screen rectangle to copy
	ScreenRect screen_rect(0, viewport_dimensions[3], viewport_dimensions[2], viewport_dimensions[3]);

	// Create a new ImageTexture with a unique filename for this newly captured screen
	ImageTexture* new_image = new ImageTexture("capture_screen" + NumberToString(capture_id), "<T>", viewport_dimensions[2], viewport_dimensions[3]);
	new_image->AddReference();

	// Create a texture sheet of an appropriate size that can retain the capture
	TexSheet* temp_sheet = TextureManager->_CreateTexSheet(RoundUpPow2(viewport_dimensions[2]), RoundUpPow2(viewport_dimensions[3]), VIDEO_TEXSHEET_ANY, false);
	VariableTexSheet* sheet = dynamic_cast<VariableTexSheet*>(temp_sheet);

	// Ensure that texture sheet creation succeeded, insert the texture image into the sheet, and copy the screen into the sheet
	if (sheet == NULL) {
		delete new_image;
		throw Exception("could not create texture sheet to store captured screen", __FILE__, __LINE__, __FUNCTION__);
		screen_image.Clear();
		return screen_image;
	}
	if (sheet->InsertTexture(new_image) == false) {
		TextureManager->_RemoveSheet(sheet);
		delete new_image;
		throw Exception("could not insert captured screen image into texture sheet", __FILE__, __LINE__, __FUNCTION__);
		screen_image.Clear();
		return screen_image;
	}
	if (sheet->CopyScreenRect(0, 0, screen_rect) == false) {
		TextureManager->_RemoveSheet(sheet);
		delete new_image;
		throw Exception("call to TexSheet::CopyScreenRect() failed", __FILE__, __LINE__, __FUNCTION__);
		screen_image.Clear();
		return screen_image;
	}

	// Store the image element to the saved image (with a flipped y axis)
	screen_image._image_texture = new_image;
	screen_image._texture = new_image;

	// Vertically flip the texture image by swapping the v coordinates, since OpenGL returns the image upside down in the CopyScreenRect call
	float temp = new_image->v1;
	new_image->v1 = new_image->v2;
	new_image->v2 = temp;

	if (CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occurred: " << CreateGLErrorString() << endl;
	}

	capture_id++;
	return screen_image;
}



void VideoEngine::SetGamma(float value) {
	_gamma_value = value;

	// Limit min/max gamma
	if (_gamma_value > 2.0f) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to set gamma over 2.0f" << endl;
		_gamma_value = 2.0f;
	}
	else if (_gamma_value < 0.0f) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to set gamma below 0.0f" << endl;
		_gamma_value = 0.0f;
	}

	SDL_SetGamma(_gamma_value, _gamma_value, _gamma_value);
}



void VideoEngine::MakeScreenshot(const std::string& filename) {
	private_video::ImageMemory buffer;

	// Retrieve the width and height of the viewport.
	GLint viewport_dimensions[4]; // viewport_dimensions[2] is the width, [3] is the height
	glGetIntegerv(GL_VIEWPORT, viewport_dimensions);

	// Buffer to store the image before it is flipped
	buffer.width = viewport_dimensions[2];
	buffer.height = viewport_dimensions[3];
	buffer.pixels = malloc(buffer.width * buffer.height * 3);
	buffer.rgb_format = true;

	// Read pixel data
	glReadPixels(0, 0, buffer.width, buffer.height, GL_RGB, GL_UNSIGNED_BYTE, buffer.pixels);

	if (CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << CreateGLErrorString() << endl;

		free(buffer.pixels);
		buffer.pixels = NULL;
		return;
	}

	// Vertically flip the image, then swap the flipped and original images
	void* buffer_temp = malloc(buffer.width * buffer.height * 3);
	for (int32 i=0; i < buffer.height; ++i) {
		memcpy((uint8*)buffer_temp + i * buffer.width * 3, (uint8*)buffer.pixels + (buffer.height - i - 1) * buffer.width * 3, buffer.width * 3);
	}
	void* temp = buffer.pixels;
	buffer.pixels = buffer_temp;
	buffer_temp = temp;

	buffer.SaveImage(filename, false);

	free(buffer_temp);
	free(buffer.pixels);
	buffer.pixels = NULL;
}

//-----------------------------------------------------------------------------
// _CreateTempFilename
//-----------------------------------------------------------------------------

std::string VideoEngine::_CreateTempFilename(const std::string &extension)
{
	// figure out the temp filename to return
	string file_name = "/tmp/allacrost";
	file_name += _next_temp_file;
	file_name += extension;

	// increment the 8-character temp name
	// Note: assume that the temp name is currently set to
	//       a valid name


	for(int32 digit = 7; digit >= 0; --digit)
	{
		++_next_temp_file[digit];

		if(_next_temp_file[digit] > 'z')
		{
			if(digit==0)
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _nextTempFile went past 'zzzzzzzz'" << endl;
				return file_name;
			}

			_next_temp_file[digit] = '0';
		}
		else
		{
			if(_next_temp_file[digit] > '9' && _next_temp_file[digit] < 'a')
				_next_temp_file[digit] = 'a';

			// if the digit did not overflow, then we don't need to carry over
			break;
		}
	}

	return file_name;
}




int32 VideoEngine::_ConvertYAlign(int32 y_align) {
	switch (y_align) {
		case VIDEO_Y_BOTTOM:
			return -1;
		case VIDEO_Y_CENTER:
			return 0;
		case VIDEO_Y_TOP:
			return 1;
		default:
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown value for argument flag: " << y_align << endl;
			return 0;
	}
}




int32 VideoEngine::_ConvertXAlign(int32 x_align) {
	switch (x_align) {
		case VIDEO_X_LEFT:
			return -1;
		case VIDEO_X_CENTER:
			return 0;
		case VIDEO_X_RIGHT:
			return 1;
		default:
			IF_PRINT_WARNING(VIDEO_DEBUG) << "unknown value for argument flag: " << x_align << endl;
			return 0;
	}
}


bool VideoEngine::SetDefaultCursor(const std::string &cursor_image_filename) {
	return _default_menu_cursor.Load(cursor_image_filename);
}



StillImage* VideoEngine::GetDefaultCursor() {
	if (_default_menu_cursor.GetWidth() != 0.0f)  // cheap test if image is valid
		return &_default_menu_cursor;
	else
		return NULL;
}




int32 VideoEngine::_ScreenCoordX(float x) {
	float percent;
	if (_current_context.coordinate_system.GetLeft() < _current_context.coordinate_system.GetRight())
		percent = (x - _current_context.coordinate_system.GetLeft()) /
			(_current_context.coordinate_system.GetRight() - _current_context.coordinate_system.GetLeft());
	else
		percent = (x - _current_context.coordinate_system.GetRight()) /
			(_current_context.coordinate_system.GetLeft() - _current_context.coordinate_system.GetRight());

	return static_cast<int32>(percent * static_cast<float>(_screen_width));
}



int32 VideoEngine::_ScreenCoordY(float y) {
	float percent;
	if (_current_context.coordinate_system.GetTop() < _current_context.coordinate_system.GetBottom())
		percent = (y - _current_context.coordinate_system.GetTop()) /
			(_current_context.coordinate_system.GetBottom() - _current_context.coordinate_system.GetTop());
	else
		percent = (y - _current_context.coordinate_system.GetBottom()) /
			(_current_context.coordinate_system.GetTop() - _current_context.coordinate_system.GetBottom());

	return static_cast<int32>(percent * static_cast<float>(_screen_height));
}



void VideoEngine::_UpdateAmbientOverlay(uint32 frame_time) {
	// Update the position of the overlay image based on the time elapsed
	float elapsed_ms = static_cast<float>(frame_time);
	_ambient_x_shift += elapsed_ms / 1000 * _ambient_x_speed;
	_ambient_y_shift += elapsed_ms / 1000 * _ambient_y_speed;

	float width = _ambient_overlay_image.GetWidth();
	float height = _ambient_overlay_image.GetHeight();

	// Shift to the left edge so that the ambient image can be drawn across the entire screen
	while (_ambient_x_shift > 0.0f) {
		_ambient_x_shift -= width;
	}
	// Handle negative x shifting
	if (_ambient_x_shift < 2 * -width) {
		_ambient_x_shift += width;
	}

	// Shift to the top edge so that the ambient image can be drawn across the entire screen
	while (_ambient_y_shift > 0.0f) {
		_ambient_y_shift -= height;
	}
	// Handle negative y shifting
	if (_ambient_y_shift < 2 * -height) {
		_ambient_y_shift += height;
	}
}



void VideoEngine::_UpdateLightning(uint32 frame_time) {
	if (_lightning_active == false)
		return;

	_lightning_current_time += frame_time;

	// Check if the next sound is ready to be played and update to the next sound
	// Note: we let the _next_lightning_sound counter increment past the end of the _lightning_sounds container
	// to indicate when there are no more sounds to play in the sequence. Thus this loop has to be protected by
	// this if statement to make sure we do not make an out-of-bounds access error
	if (_next_lightning_sound < _lightning_sounds.size()) {
		// This is done in a loop in case two sounds are played at the same time, or very similar times
		while (_lightning_current_time >= _lightning_sounds[_next_lightning_sound].time) {
			AudioManager->PlaySound(_lightning_sounds[_next_lightning_sound].filename);
			_next_lightning_sound++;

			// If we just played the last sound in the sequence, exit the loop
			if (_next_lightning_sound >= _lightning_sounds.size()) {
				break;
			}
		}
	}

	// Check if the sequence has reached its end
	if (_lightning_current_time >= _lightning_end_time) {
		if (_lightning_looped == false) {
			DisableLightning();
		}
		else {
			// Restart the sequence timer, but don't throw away any "remainder time" that occurred during this loop
			_lightning_current_time = _lightning_current_time - _lightning_end_time;
			_next_lightning_sound = 0;
		}
	}
}



void VideoEngine::_DEBUG_ShowAdvancedStats() {
	char text[50];
	sprintf(text, "Switches: %d\nParticles: %d", TextureManager->_debug_num_tex_switches, _particle_manager.GetNumParticles());

	Move(896.0f, 690.0f);
	TextManager->Draw(text);
}



void VideoEngine::DrawLine(float x1, float y1, float x2, float y2, float width, const Color& color) {
	GLfloat vert_coords[] =
	{
		x1, y1,
		x2, y2
	};
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal blending
	glPushAttrib(GL_LINE_WIDTH);

	float pixel_width, pixel_height;
	GetPixelSize(pixel_width, pixel_height);
	glLineWidth(width * pixel_height);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glColor4fv((GLfloat*)color.GetColors());
	glVertexPointer(2, GL_FLOAT, 0, vert_coords);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopAttrib();
}



void VideoEngine::DrawGrid(float x, float y, float x_step, float y_step, const Color& c) {
	PushState();

	Move(0, 0);

	float x_max = _current_context.coordinate_system.GetRight();
	float y_max = _current_context.coordinate_system.GetBottom();

	vector<GLfloat> vertices;
	int32 num_vertices = 0;
	for (; x <= x_max; x += x_step) {
		vertices.push_back(x);
		vertices.push_back(_current_context.coordinate_system.GetBottom());
		vertices.push_back(x);
		vertices.push_back(_current_context.coordinate_system.GetTop());
		num_vertices += 2;
	}
	for (; y < y_max; y += y_step) {
		vertices.push_back(_current_context.coordinate_system.GetLeft());
		vertices.push_back(y);
		vertices.push_back(_current_context.coordinate_system.GetRight());
		vertices.push_back(y);
		num_vertices += 2;
	}
	glColor4fv(&c[0]);
	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &(vertices[0]));
	glDrawArrays(GL_LINES, 0, num_vertices);
	glDisableClientState(GL_VERTEX_ARRAY);

	PopState();
}



void VideoEngine::DrawRectangle(float width, float height, const Color& color) {
	_rectangle_image._width = width;
	_rectangle_image._height = height;
	_rectangle_image.SetColor(color);

	_rectangle_image.Draw();
}



void VideoEngine::DrawRectangleOutline(float left, float right, float bottom, float top, float width, const Color& color) {
	DrawLine(left, bottom, right, bottom, width, color);
	DrawLine(left, top, right, top, width, color);
	DrawLine(left, bottom, left, top, width, color);
	DrawLine(right, bottom, right, top, width, color);
}



void VideoEngine::DrawHalo(const ImageDescriptor& image, float x, float y, const Color &color) {
	PushMatrix();
	Move(x, y);
	int8 old_blend_mode = _current_context.blend;
	_current_context.blend = VIDEO_BLEND_ADD;
	image.Draw(color);
	_current_context.blend = old_blend_mode;
	PopMatrix();
}




void VideoEngine::DrawLight(float radius, float x, float y, const Color &color) {
	// This function has been left unimplemented as the old implementation was broken
	// FBOs have been removed and the old implementation was no longer valid
	// TODO: Re-implement this method
}

}  // namespace hoa_video
