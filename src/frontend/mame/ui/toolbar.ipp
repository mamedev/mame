// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_FRONTEND_UI_TOOLBAR_IPP
#define MAME_FRONTEND_UI_TOOLBAR_IPP
#pragma once

namespace ui {

namespace {

// TODO: move this to external image files and zlib compress them into a source file as part of the build process
char const *const toolbar_icons_svg[] = {
		// favourites star
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='104' width='104'>"
			"<path fill='#ffef0f' stroke='#ffdf1f' stroke-width='8' stroke-linejoin='round' d='m 50,6 11,35 h 37 l -30,22 12,35 -30,-21 -30,21 12,-35 -30,-22 h 37 z' />"
		"</svg>",
		// save diskette
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='100' width='100'>"
			"<path fill='#ffcf0f' d='m 0,3 a 3,3 0 0,1 3,-3 h 90 l 7,7 v 90 a 3,3 0 0,1 -3,3 h -94 a 3,3 0 0,1 -3,-3 z m 3,89 v 4 h 5 v -4 z m 89,0 v 4 h 5 v -4 z' />"
			"<path fill='#ffbf1f' d='m 10,0 h 67 v 32 a 3,3 0 0,1 -3,3 h -61 a 3,3 0 0,1 -3,-3 z' />"
			"<path fill='#b0b0b0' d='m 24,0 h 53 v 31 a 3,3 0 0,1 -3,3 h -47 a 3,3 0 0,1 -3,-3 z m 31,5 v 25 h 13 v -25 z' />"
			"<path fill='#ffbf1f' d='m 10,47 a 3,3 0 0,1 3,-3 h 74 a 3,3 0 0,1 3,3 v 53 h -80 z' />"
			"<path fill='#e7e7e7' d='m 11,48 a 3,3 0 0,1 3,-3 h 72 a 3,3 0 0,1 3,3 v 40 h -78 z' />"
			"<path fill='#f71f1f' d='m 11,87 h 78 v 9 a 3,3 0 0,1 -3,3 h -72 a 3,3 0 0,1 -3,-3 z' />"
		"</svg>",
		// audit magnifying glass
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='100' width='100'>"
			"<path fill-opacity='0' stroke='#bfbfbf' stroke-linecap='butt' stroke-width='8' d='m 68,68 -10,-10' />"
			"<path fill-opacity='0' stroke='#cf8f3f' stroke-linecap='round' stroke-width='16' d='m 92,92 -22,-22' />"
			"<circle cx='36' cy='36' r='36' fill='#cfcfcf' />"
			"<circle cx='36' cy='36' r='30' fill='#9ebeff' />"
			"<path fill-opacity='0' stroke='#b9cef7' stroke-linecap='round' stroke-width='10' d='m 16,36 a 20,20 0 0,1 20,-20' />"
		"</svg>",
		// info
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='100' width='100'>"
			"<circle cx='50' cy='50' r='47' fill='#001fff' stroke='#3f56ff' stroke-width='6' stroke-opacity='0.8' />"
			"<circle cx='50' cy='20' r='10' fill='#ffffff' />"
			"<path fill='#ffffff' d='m 59,38 v 34 a 10,4 0 0,0 10,4 v 8 h -36 v -8 a 10,4 0 0,0 10,-4 v -23 a 8,4 0 0,0 -8,-4 v -6 z' />"
		"</svg>",
		// previous menu
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='100' width='100'>"
			"<rect y='8' x='8' height='84' width='84' fill='#3f56ff' stroke='#3f56ff' stroke-width='16' stroke-opacity='0.8' stroke-linejoin='round' />"
			"<rect y='10' x='10' height='80' width='80' fill='#001fff' stroke='#001fff' stroke-width='8' stroke-linejoin='round' />"
			"<path fill='#ffffff' stroke='#ffffff' stroke-width='8' stroke-linejoin='round' d='m 16,46 28,-28 v 16 q 40,12 40,48 q -10,-24 -40,-24 v 16 z' />"
		"</svg>",
		// exit
		u8"<?xml version='1.0' encoding='UTF-8' standalone='no'?>"
		"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.1' height='100' width='100'>"
			"<rect y='8' x='8' height='84' width='84' fill='#ff3f3f' fill-opacity='0.8' stroke='#ff3f3f' stroke-opacity='0.8' stroke-width='16' stroke-linejoin='round' />"
			"<rect y='10' x='10' height='80' width='80' fill='#ff0000' stroke='#ff0000' stroke-width='8' stroke-linejoin='round' />"
			"<path fill='#ffffff' stroke='#ffffff' stroke-width='8' stroke-linejoin='round' d='m 16,24 8,-8 26,26 26,-26 8,8 -26,26 26,26 -8,8 -26,-26 -26,26 -8,-8 26,-26 z' />"
		"</svg>" };

enum
{
	TOOLBAR_BITMAP_FAVORITE,
	TOOLBAR_BITMAP_SAVE,
	TOOLBAR_BITMAP_AUDIT,
	TOOLBAR_BITMAP_INFO,
	TOOLBAR_BITMAP_PREVMENU,
	TOOLBAR_BITMAP_EXIT
};

constexpr size_t UI_TOOLBAR_BUTTONS = std::size(toolbar_icons_svg);

} // anonymous namespace

} // namespace ui

#endif // MAME_FRONTEND_UI_TOOLBAR_IPP
