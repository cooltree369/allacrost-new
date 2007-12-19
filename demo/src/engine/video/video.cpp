///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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


#include <cassert>
#include <cstdarg>
#include <math.h>
#include <vector>

#include "video.h"
#include "script.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video::private_video;

template<> hoa_video::GameVideo* Singleton<hoa_video::GameVideo>::_singleton_reference = NULL;

namespace hoa_video {

GameVideo* VideoManager = NULL;
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
// GameVideo class
//-----------------------------------------------------------------------------

GameVideo::GameVideo() :
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
	_uses_lights = false;
	_light_overlay = INVALID_TEXTURE_ID;
	_advanced_display = false;
	_fps_display = false;
	_x_shake = 0;
	_y_shake = 0;
	_gamma_value = 1.0f;
	_fog_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_fog_intensity = 0.0f;
	_light_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_gl_error_code = GL_NO_ERROR;
	_animation_counter = 0;
	_current_frame_diff = 0;
	_lightning_active = false;
	_lightning_current_time = 0;
	_lightning_end_time = 0;

	_current_context.blend = 0;
	_current_context.x_align = -1;
	_current_context.y_align = -1;
	_current_context.x_flip = 0;
	_current_context.y_flip = 0;
	_current_context.coordinate_system = CoordSys(0.0f, 1023.0f, 0.0f, 767.0f);
	_current_context.text_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_current_context.viewport = ScreenRect(0, 0, 100, 100);
	_current_context.scissor_rectangle = ScreenRect(0, 0, 1023, 767);
	_current_context.scissoring_enabled = false;

	strcpy(_next_temp_file, "00000000");
}



GameVideo::~GameVideo() {
	_particle_manager.Destroy();
	GUIManager->SingletonDestroy();
	TextManager->SingletonDestroy();

	_default_menu_cursor.Clear();
	_rectangle_image.Clear();

	TextureManager->SingletonDestroy();
}



bool GameVideo::SingletonInitialize() {
	// check to see if the singleton is already initialized
	if (_initialized)
		return true;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		PRINT_ERROR << "SDL video initialization failed" << endl;
		return false;
	}

	return true;
} // bool GameVideo::SingletonInitialize()



bool GameVideo::FinalizeInitialization() {
	// Create instances of the various sub-systems
	TextureManager = TextureController::SingletonCreate();
	TextManager = TextSupervisor::SingletonCreate();
	GUIManager = GUISupervisor::SingletonCreate();
	
	if (TextureManager->SingletonInitialize() == false) {
		PRINT_ERROR << "could not initialize texture manager" << endl;
		return false;
	}

	// Initialize all sub-systems
	if (TextManager->SingletonInitialize() == false) {
		PRINT_ERROR << "could not initialize text manager" << endl;
		return false;
	}

	if (GUIManager->SingletonInitialize() == false) {
		PRINT_ERROR << "could not initialize GUI manager" << endl;
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

	if (_rectangle_image.Load("") == false) {
		PRINT_ERROR << "_rectangle_image could not be created" << endl;
		return false;
	}

	_initialized = true;
	return true;
}



void GameVideo::SetInitialResolution(int32 width, int32 height) {
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
// GameVideo class - General methods
//-----------------------------------------------------------------------------

void GameVideo::SetTarget(VIDEO_TARGET target) {
	if (target <= VIDEO_TARGET_INVALID || target >= VIDEO_TARGET_TOTAL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to set video engine to an invalid target: " << target << endl;
		return;
	}

	_target = target;
}



void GameVideo::SetDrawFlags(int32 first_flag, ...) {
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



void GameVideo::Clear() {
	//! \todo glClearColor is a state change operation. It should only be called when the clear color changes
	if (_uses_lights)
		Clear(_light_color);
	else
		Clear(Color::black);
}



void GameVideo::Clear(const Color &c) {
	SetViewport(0.0f, 100.0f, 0.0f, 100.0f);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	TextureManager->_debug_num_tex_switches = 0;

	if (CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << CreateGLErrorString() << endl;
	}
}



void GameVideo::Display(uint32 frame_time) {
	// Update all particle effects
	_particle_manager.Update(frame_time);

	// Update shaking effect
	PushState();
	SetCoordSys(0, 1024, 0, 768);
	_UpdateShake(frame_time);

	// Update lightning timer
	_lightning_current_time += frame_time;

	if (_lightning_current_time > _lightning_end_time)
		_lightning_active = false;

	// Draw a screen overlay if we are in the process of fading fading
	if (_screen_fader.ShouldUseFadeOverlay()) {
		Color fade_color = _screen_fader.GetFadeOverlayColor();
		StillImage fade_overlay;
		fade_overlay.SetColor(fade_color);
		fade_overlay.Load("", 1024.0f, 768.0f);
		SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		PushState();
		Move(0, 0);
		fade_overlay.Draw();
		PopState();
	}

	// This must be called before DrawFPS, because we only want to count
	// texture switches related to the game's normal operation, not the
	// ones used to draw the video engine debugging text
	if (_advanced_display)
		_DEBUG_ShowAdvancedStats();

	if (_fps_display)
		DrawFPS(frame_time);

	if (TextureManager->debug_current_sheet >= 0)
		TextureManager->DEBUG_ShowTexSheet();

	PopState();

	SDL_GL_SwapBuffers();

	_screen_fader.Update(frame_time);

	// Update animation timers
	int32 old_frame_index = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;
	_animation_counter += frame_time;
	int32 current_frame_index = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;
	_current_frame_diff = current_frame_index - old_frame_index;
} // void GameVideo::Display(uint32 frame_time)



const std::string GameVideo::CreateGLErrorString() {
	const GLubyte* error_string = gluErrorString(_gl_error_code);

	if (error_string == NULL)
		return ("Unknown GL error code: " + NumberToString(_gl_error_code));
	else
		return (char*)error_string;
}

//-----------------------------------------------------------------------------
// GameVideo class - Screen size and resolution methods
//-----------------------------------------------------------------------------

void GameVideo::GetPixelSize(float& x, float& y) {
	x = (_current_context.coordinate_system.GetRight() - _current_context.coordinate_system.GetLeft()) / _screen_width;
	y = (_current_context.coordinate_system.GetTop() - _current_context.coordinate_system.GetBottom()) / _screen_height;
}



bool GameVideo::ApplySettings() {
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
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

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

		// Turn off writing to the depth buffer
		glDepthMask(GL_FALSE);

		_screen_width = _temp_width;
		_screen_height = _temp_height;
		_fullscreen = _temp_fullscreen;

		if (TextureManager)
			TextureManager->ReloadTextures();
		EnableFog(_fog_color, _fog_intensity);

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
} // bool GameVideo::ApplySettings()

//-----------------------------------------------------------------------------
// GameVideo class - Coordinate system and viewport methods
//-----------------------------------------------------------------------------

void GameVideo::SetViewport(float left, float right, float bottom, float top) {
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

	_current_context.viewport = ScreenRect(l, b, r - l + 1, t - b + 1);
	glViewport(l, b, r - l + 1, t - b + 1);
}



void GameVideo::SetCoordSys(const CoordSys& coordinate_system) {
	_current_context.coordinate_system = coordinate_system;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(_current_context.coordinate_system.GetLeft(), _current_context.coordinate_system.GetRight(),
		_current_context.coordinate_system.GetBottom(), _current_context.coordinate_system.GetTop(), -1, 1);

 	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



void GameVideo::EnableScissoring() {
	_current_context.scissoring_enabled = true;
	glEnable(GL_SCISSOR_TEST);
}



void GameVideo::DisableScissoring() {
	_current_context.scissoring_enabled = false;
	glDisable(GL_SCISSOR_TEST);
}



void GameVideo::SetScissorRect(float left, float right, float bottom, float top) {
	_current_context.scissor_rectangle = CalculateScreenRect(left, right, bottom, top);

	glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
		static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height),
		static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
		static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height)
	);
}



void GameVideo::SetScissorRect(const ScreenRect& rect) {
	_current_context.scissor_rectangle = rect;

	glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
		static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height),
		static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
		static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height)
	);
}



ScreenRect GameVideo::CalculateScreenRect(float left, float right, float bottom, float top) {
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
// GameVideo class - Transformation methods
//-----------------------------------------------------------------------------

void GameVideo::Move(float x, float y) {
	glLoadIdentity();
	glTranslatef(x, y, 0);
	_x_cursor = x;
	_y_cursor = y;
}



void GameVideo::MoveRelative(float x, float y) {
	glTranslatef(x, y, 0);
	_x_cursor += x;
	_y_cursor += y;
}




void GameVideo::PushState() {
	// Push current modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	_context_stack.push(_current_context);
}



void GameVideo::PopState() {
	// Restore the most recent context information and pop it from stack
	if (_context_stack.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no video states were saved on the stack" << endl;
		return;
	}

	_current_context = _context_stack.top();
	_context_stack.pop();

	// Restore the modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glViewport(_current_context.viewport.left, _current_context.viewport.top, _current_context.viewport.width, _current_context.viewport.height);

	if (_current_context.scissoring_enabled) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(static_cast<GLint>((_current_context.scissor_rectangle.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
			static_cast<GLint>((_current_context.scissor_rectangle.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height),
			static_cast<GLsizei>((_current_context.scissor_rectangle.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH)) * _current_context.viewport.width),
			static_cast<GLsizei>((_current_context.scissor_rectangle.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT)) * _current_context.viewport.height)
		);
	}
	else {
		glDisable(GL_SCISSOR_TEST);
	}
}



void GameVideo::SetTransform(float matrix[16]) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(matrix);
}



void GameVideo::EnableSceneLighting(const Color& color) {
	_light_color = color;

	if (IsFloatEqual(color[3], 1.0f) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "color argrument had alpha not equal to 1.0f" << endl;
		_light_color[3] = 1.0f;
	}
}


void GameVideo::DisableSceneLighting() {
	_light_color = Color::white;
}



void GameVideo::EnableFog(const Color &color, float intensity) {
	// Set the parameters
	_fog_color = color;
	_fog_intensity = intensity;

	// Check if te intensity is within acceptable bounds
	if (_fog_intensity < 0.0f) {
		_fog_intensity = 0.0f;
		IF_PRINT_DEBUG(VIDEO_DEBUG) << "intensity argument was less than 0.0f" << endl;
	}
	else if (_fog_intensity > 1.0f) {
		_fog_intensity = 1.0f;
		IF_PRINT_DEBUG(VIDEO_DEBUG) << "intensity argument was greater than 1.0f" << endl;
	}


	// Apply the new settings with OpenGL
	if (IsFloatEqual(intensity, 0.0f)) {
		glDisable(GL_FOG);
	}
	else {
		glEnable(GL_FOG);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, 0.0f - intensity);
		glFogf(GL_FOG_END, 1.0f - intensity);
		glFogfv(GL_FOG_COLOR, (GLfloat *)color.GetColors());
	}
}



void GameVideo::DisableFog() {
	glDisable(GL_FOG);
	_fog_intensity = 0.0f;
}



void GameVideo::EnablePointLights() {
	_light_overlay = TextureManager->_CreateBlankGLTexture(1024, 1024);
	_uses_lights = true;
}


void GameVideo::DisablePointLights() {
	if (_light_overlay != INVALID_TEXTURE_ID) {
		TextureManager->_DeleteTexture(_light_overlay);
	}

	_light_overlay = INVALID_TEXTURE_ID;
	_uses_lights = false;
}



void GameVideo::ApplyLightingOverlay() {
	if (_light_overlay == INVALID_TEXTURE_ID) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "light overlay texture was invalid" << endl;
		return;
	}

	// Copy light overlay to opengl texture
	TextureManager->_BindTexture(_light_overlay);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);

	CoordSys temp_coords = _current_context.coordinate_system;

	SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);

	float mx = _screen_width / 1024.0f;
	float my = _screen_height / 1024.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	TextureManager->_BindTexture(_light_overlay);

	GLfloat vertices[8] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat tex_coords[8] = { 0.0f, 0.0f, mx, 0.0f, mx, my, 0.0f, my };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	SetCoordSys(temp_coords.GetLeft(), temp_coords.GetRight(), temp_coords.GetBottom(), temp_coords.GetTop());
}



StillImage GameVideo::CaptureScreen() throw(Exception) {
	// Static variable used to make sure the capture has a unique name in the texture image map
	static uint32 capture_id = 0;

	StillImage screen_image;

	// TEMP: temporary resolution until capture screen bug is fixed
// 	return screen_image;

	// Retrieve width/height of the viewport. viewport_dimensions[2] is the width, [3] is the height
	GLint viewport_dimensions[4];
	glGetIntegerv(GL_VIEWPORT, viewport_dimensions);
	screen_image.SetDimensions(viewport_dimensions[2], viewport_dimensions[3]);

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



void GameVideo::SetGamma(float value) {
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



void GameVideo::MakeScreenshot(const std::string& filename) {
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

std::string GameVideo::_CreateTempFilename(const std::string &extension)
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




int32 GameVideo::_ConvertYAlign(int32 y_align) {
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




int32 GameVideo::_ConvertXAlign(int32 x_align) {
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


bool GameVideo::SetDefaultCursor(const std::string &cursor_image_filename) {
	return _default_menu_cursor.Load(cursor_image_filename);
}



StillImage* GameVideo::GetDefaultCursor() {
	if (_default_menu_cursor.GetWidth() != 0.0f)  // cheap test if image is valid
		return &_default_menu_cursor;
	else
		return NULL;
}




int32 GameVideo::_ScreenCoordX(float x) {
	float percent;
	if (_current_context.coordinate_system.GetLeft() < _current_context.coordinate_system.GetRight())
		percent = (x - _current_context.coordinate_system.GetLeft()) /
			(_current_context.coordinate_system.GetRight() - _current_context.coordinate_system.GetLeft());
	else
		percent = (x - _current_context.coordinate_system.GetRight()) /
			(_current_context.coordinate_system.GetLeft() - _current_context.coordinate_system.GetRight());

	return static_cast<int32>(percent * static_cast<float>(_screen_width));
}



int32 GameVideo::_ScreenCoordY(float y) {
	float percent;
	if (_current_context.coordinate_system.GetTop() < _current_context.coordinate_system.GetBottom())
		percent = (y - _current_context.coordinate_system.GetTop()) /
			(_current_context.coordinate_system.GetBottom() - _current_context.coordinate_system.GetTop());
	else
		percent = (y - _current_context.coordinate_system.GetBottom()) /
			(_current_context.coordinate_system.GetTop() - _current_context.coordinate_system.GetBottom());

	return static_cast<int32>(percent * static_cast<float>(_screen_height));
}



bool GameVideo::MakeLightning(const std::string &lit_file) {
	FILE *fp = fopen(lit_file.c_str(), "rb");
	if(!fp)
		return false;

	int32 data_size;
	if(!fread(&data_size, 4, 1, fp))
	{
		fclose(fp);
		return false;
	}

	// since this file was created on windows, it uses little endian byte order
	// Check if this processor uses big endian, and reorder bytes if so.

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		data_size = ((data_size & 0xFF000000) >> 24) |
		           ((data_size & 0x00FF0000) >> 8) |
		           ((data_size & 0x0000FF00) << 8) |
		           ((data_size & 0x000000FF) << 24);
	#endif

	uint8 *data = new uint8[data_size];

	if(!fread(data, data_size, 1, fp))
	{
		delete [] data;
		fclose(fp);
		return false;
	}

	fclose(fp);

	_lightning_data.clear();

	for(int32 j = 0; j < data_size; ++j)
	{
		float f = float(data[j]) / 255.0f;
		_lightning_data.push_back(f);
	}

	delete [] data;

	_lightning_active = true;
	_lightning_current_time = 0;
	_lightning_end_time = data_size * 1000 / 100;

	return true;
}



void GameVideo::DrawLightning() {
	if(!_lightning_active)
		return;

	// convert milliseconds elapsed into data points elapsed

	float t = _lightning_current_time * 100.0f / 1000.0f;

	int32 rounded_t = static_cast<int32>(t);
	t -= rounded_t;

	// get 2 separate data points and blend together (linear interpolation)

	float data1 = _lightning_data[rounded_t];
	float data2 = _lightning_data[rounded_t+1];

	float intensity = data1 * (1-t) + data2 * t;

	DrawFullscreenOverlay(Color(1.0f, 1.0f, 1.0f, intensity));
}



void GameVideo::DrawFullscreenOverlay(const Color& color) {
	PushState();

	SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
	SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	Move(0.0f, 0.0f);
	StillImage img;
	img.Load("", 1.0f, 1.0f);
	img.Draw(color);

	PopState();
}



void GameVideo::_DEBUG_ShowAdvancedStats() {
	char text[50];
	sprintf(text, "Switches: %d\nParticles: %d", TextureManager->_debug_num_tex_switches, _particle_manager.GetNumParticles());

	TextManager->SetDefaultFont("debug_font");

	Move(896.0f, 690.0f);
	TextManager->Draw(text);
}



void GameVideo::DrawGrid(float x, float y, float x_step, float y_step, const Color& c) {
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, &(vertices[0]));
	glDrawArrays(GL_LINES, 0, num_vertices);
	glDisableClientState(GL_VERTEX_ARRAY);

	PopState();
}



void GameVideo::DrawRectangle(float width, float height, const Color& color) {
	_rectangle_image._width = width;
	_rectangle_image._height = height;
	_rectangle_image._color[0] = color;

	_rectangle_image.Draw(color);
}



void GameVideo::DrawHalo(const StillImage &id, float x, float y, const Color &color) {
	PushMatrix();
	Move(x, y);

	char old_blend_mode = _current_context.blend;
	_current_context.blend = VIDEO_BLEND_ADD;
	id.Draw(color);
	_current_context.blend = old_blend_mode;
	PopMatrix();
}



void GameVideo::DrawLight(const StillImage &id, float x, float y, const Color &color) {
	if (_uses_lights == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called DrawLight() even though real lighting was not enabled!" << endl;
	}

	DrawHalo(id, x, y, color);
}



void GameVideo::DrawFPS(uint32 frame_time) {
	PushState();
	GUIManager->_DrawFPS(frame_time);
	PopState();
}

}  // namespace hoa_video
