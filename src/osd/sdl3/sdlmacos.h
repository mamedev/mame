// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
//============================================================
//
//  sdlmacos.h - macOS application activation helpers
//
//============================================================
#ifndef MAME_OSD_SDL3_SDLMACOS_H
#define MAME_OSD_SDL3_SDLMACOS_H

#pragma once

// true if AppKit considers this application active
bool sdl_macos_app_active();

// true if the window server considers this application frontmost
bool sdl_macos_app_frontmost();

// ask the Dock to become the active application
void sdl_macos_activate_dock();

// ask for this application to become active
void sdl_macos_activate_self();

#endif // MAME_OSD_SDL3_SDLMACOS_H
