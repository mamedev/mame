// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************
 * emufwd.h
 *
 * Forward declarations for MAME famework.
 *
 * Please place forward declarations here rather than littering them
 * throughout headers in src/emu.  It makes it much easier to update
 * them and remove obsolete ones.
 **********************************************************************/
#ifndef MAME_EMU_EMUFWD_H
#define MAME_EMU_EMUFWD_H

#pragma once

#include "utilfwd.h"

#include <type_traits>


//----------------------------------
// 3rdparty
//----------------------------------

// declared in expat.h
struct XML_ParserStruct;



//----------------------------------
// osd
//----------------------------------

// declared in modules/output/output_module.h
class output_module;

// declared in osdepend.h
class osd_font;
class osd_interface;



//----------------------------------
// emu
//----------------------------------

// declared in addrmap.h
class address_map;
class address_map_entry;

// declared in bookkeeping.h
class bookkeeping_manager;

// declared in config.h
enum class config_type : int;
enum class config_level : int;
class configuration_manager;

// declared in crsshair.h
class crosshair_manager;

// declared in debug/debugcmd.h
class debugger_commands;

// declared in debug/debugcon.h
class debugger_console;

// declared in debug/debugcpu.h
class debugger_cpu;
class device_debug;

// declared in debug/debugvw.h
class debug_view;
class debug_view_manager;

// declared in debug/express.h
class parsed_expression;
class symbol_table;

// declared in debug/points.h
class debug_breakpoint;
class debug_watchpoint;
class debug_registerpoint;
class debug_exceptionpoint;

// declared in debug/srcdbg_provider.h
class srcdbg_provider_base;
class file_line;

// declared in debugger.h
class debugger_manager;

// declared in devcb.h
class devcb_base;
template <typename Input, std::make_unsigned_t<Input> DefaultMask> class devcb_write;

// declared in devfind.h
class device_resolver_base;
class finder_base;
template <class DeviceClass, bool Required> class device_finder;

// declared in device.h
class device_interface;
class device_t;

// declared in didisasm.h
class device_disasm_interface;

// declared in diexec.h
class device_execute_interface;

// declared in digfx.h
struct gfx_decode_entry;

// declared in diimage.h
class device_image_interface;

// declared in dimemory.h
class device_memory_interface;

// declared in dinetwork.h
class device_network_interface;

// declared in dipalette.h
class device_palette_interface;

// declared in distate.h
class device_state_interface;

// declared in drawgfx.h
class gfx_element;

// declared in driver.h
class driver_device;

// declared in emumem.h
class address_space;
class memory_bank;
class memory_manager;
class memory_region;
class memory_share;
class memory_view;

// declared in emuopts.h
class emu_options;

// declared in fileio.h
class emu_file;

// declared in http.h
class http_manager;

// declared in gamedrv.h
class game_driver;

// declared in input.h
class input_manager;

// declared in inputdev.h
class input_class;
class input_device;
class input_device_item;

// declared in image.h
class image_manager;

// declared in ioport.h
class analog_field;
struct input_device_default;
class ioport_field;
struct ioport_field_live;
class ioport_list;
class ioport_manager;
class ioport_port;
struct ioport_port_live;

// declared in machine.h
class running_machine;

// declared in mconfig.h
namespace emu::detail { class machine_config_replace; }
struct internal_layout;
class machine_config;

// declared in main.h
class machine_manager;

// declared in natkeyboard.h
class natural_keyboard;

// declared in network.h
class network_manager;

// declared in output.h
class output_manager;

// declared in render.h
class render_container;
class render_manager;
class render_target;
class render_texture;

// declared in rendertypes.h
struct render_bounds;

// declared in rendfont.h
class render_font;

// declared in rendlay.h
class layout_element;
class layout_view_item;
class layout_view;
class layout_file;

// declared in romentry.h
class rom_entry;

// declared in romload.h
class rom_load_manager;

// declared in schedule.h
class device_scheduler;
class emu_timer;

// declared in screen.h
class screen_device;

// declared in softlist.h
class software_info;
class software_part;

// declared in softlist_dev.h
class software_list_device;
class software_list_loader;

// declared in sound.h
class sound_manager;
class sound_stream;

// declared in speaker.h
class speaker_device;

// declared in tilemap.h
class tilemap_device;
class tilemap_manager;
class tilemap_t;

// declared in ui/uimain.h
class ui_manager;

// declared in uiinput.h
class ui_input_manager;

// declared in validity.h
class validity_checker;

// declared in video.h
class video_manager;

#endif // MAME_EMU_EMUFWD_H
