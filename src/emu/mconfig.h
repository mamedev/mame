/***************************************************************************

    mconfig.h

    Machine configuration macros and functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MCONFIG_H__
#define __MCONFIG_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// by convention, tags should all lowercase and between 1-15 characters
#define MIN_TAG_LENGTH			1
#define MAX_TAG_LENGTH			15



// ----- flags for video_attributes -----

// should VIDEO_UPDATE by called at the start of VBLANK or at the end?
#define	VIDEO_UPDATE_BEFORE_VBLANK		0x0000
#define	VIDEO_UPDATE_AFTER_VBLANK		0x0004

// indicates VIDEO_UPDATE will add container bits its
#define VIDEO_SELF_RENDER				0x0008

// automatically extend the palette creating a darker copy for shadows
#define VIDEO_HAS_SHADOWS				0x0010

// automatically extend the palette creating a brighter copy for highlights
#define VIDEO_HAS_HIGHLIGHTS			0x0020

// force VIDEO_UPDATE to be called even for skipped frames
#define VIDEO_ALWAYS_UPDATE				0x0080

// calls VIDEO_UPDATE for every visible scanline, even for skipped frames
#define VIDEO_UPDATE_SCANLINE			0x0100



#define NVRAM_HANDLER_NAME(name)	nvram_handler_##name
#define NVRAM_HANDLER(name)			void NVRAM_HANDLER_NAME(name)(running_machine &machine, emu_file *file, int read_or_write)
#define NVRAM_HANDLER_CALL(name)	NVRAM_HANDLER_NAME(name)(machine, file, read_or_write)

#define MEMCARD_HANDLER_NAME(name)	memcard_handler_##name
#define MEMCARD_HANDLER(name)		void MEMCARD_HANDLER_NAME(name)(running_machine &machine, emu_file &file, int action)
#define MEMCARD_HANDLER_CALL(name)	MEMCARD_HANDLER_NAME(name)(machine, file, action)


// NULL versions
#define nvram_handler_0 			NULL
#define memcard_handler_0			NULL



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
struct gfx_decode_entry;
class driver_device;
class screen_device;



// various callback functions
typedef void   (*nvram_handler_func)(running_machine &machine, emu_file *file, int read_or_write);
typedef void   (*memcard_handler_func)(running_machine &machine, emu_file &file, int action);



// ======================> machine_config

// machine configuration definition
class machine_config
{
	DISABLE_COPYING(machine_config);

	friend class running_machine;

public:
	// construction/destruction
	machine_config(const game_driver &gamedrv, emu_options &options);
	~machine_config();

	// getters
	const game_driver &gamedrv() const { return m_gamedrv; }
	device_t &root_device() const { assert(m_root_device != NULL); return *m_root_device; }
	screen_device *first_screen() const;
	emu_options &options() const { return m_options; }
	inline device_t *device(const char *tag) const { return root_device().subdevice(tag); }
	template<class _DeviceClass> inline _DeviceClass *device(const char *tag) const { return downcast<_DeviceClass *>(device(tag)); }

	// public state
	attotime				m_minimum_quantum;			// minimum scheduling quantum
	astring					m_perfect_cpu_quantum;		// tag of CPU to use for "perfect" scheduling
	INT32					m_watchdog_vblank_count;	// number of VBLANKs until the watchdog kills us
	attotime				m_watchdog_time;			// length of time until the watchdog kills us

	// legacy callbacks
	nvram_handler_func		m_nvram_handler;			// NVRAM save/load callback
	memcard_handler_func	m_memcard_handler;			// memory card save/load callback

	// other parameters
	UINT32					m_video_attributes;			// flags describing the video system
	const gfx_decode_entry *m_gfxdecodeinfo;			// pointer to array of graphics decoding information
	UINT32					m_total_colors;				// total number of colors in the palette
	const char *			m_default_layout;			// default layout for this machine

	// helpers during configuration; not for general use
	device_t *device_add(device_t *owner, const char *tag, device_type type, UINT32 clock);
	device_t *device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock);
	device_t *device_remove(device_t *owner, const char *tag);
	device_t *device_find(device_t *owner, const char *tag);

private:
	// internal state
	const game_driver &		m_gamedrv;
	emu_options &			m_options;
	device_t *				m_root_device;
};



//**************************************************************************
//  MACHINE CONFIG MACROS
//**************************************************************************

// start/end tags for the machine driver
#define MACHINE_CONFIG_NAME(_name) construct_machine_config_##_name

#define MACHINE_CONFIG_START(_name, _class) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner) \
{ \
	device_t *device = NULL; \
	(void)device; \
	devcb2_base *devcb = NULL; \
	(void)devcb; \
	if (owner == NULL) owner = config.device_add(NULL, "root", &driver_device_creator<_class>, 0); \

#define MACHINE_CONFIG_FRAGMENT(_name) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner) \
{ \
	device_t *device = NULL; \
	(void)device; \
	devcb2_base *devcb = NULL; \
	(void)devcb; \
	assert(owner != NULL); \

#define MACHINE_CONFIG_DERIVED(_name, _base) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner) \
{ \
	device_t *device = NULL; \
	(void)device; \
	devcb2_base *devcb = NULL; \
	(void)devcb; \
	owner = MACHINE_CONFIG_NAME(_base)(config, owner); \
	assert(owner != NULL); \

#define MACHINE_CONFIG_DERIVED_CLASS(_name, _base, _class) \
ATTR_COLD device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner) \
{ \
	device_t *device = NULL; \
	(void)device; \
	devcb2_base *devcb = NULL; \
	(void)devcb; \
	if (owner == NULL) owner = config.device_add(NULL, "root", &driver_device_creator<_class>, 0); \
	owner = MACHINE_CONFIG_NAME(_base)(config, owner); \

#define MACHINE_CONFIG_END \
	return owner; \
}

// use this to declare external references to a machine driver
#define MACHINE_CONFIG_EXTERN(_name) \
	extern device_t *MACHINE_CONFIG_NAME(_name)(machine_config &config, device_t *owner)


// importing data from other machine drivers
#define MCFG_FRAGMENT_ADD(_name) \
	MACHINE_CONFIG_NAME(_name)(config, owner);


// scheduling parameters
#define MCFG_QUANTUM_TIME(_time) \
	config.m_minimum_quantum = _time; \

#define MCFG_QUANTUM_PERFECT_CPU(_cputag) \
	owner->subtag(config.m_perfect_cpu_quantum, _cputag); \



// watchdog configuration
#define MCFG_WATCHDOG_VBLANK_INIT(_count) \
	config.m_watchdog_vblank_count = _count; \

#define MCFG_WATCHDOG_TIME_INIT(_time) \
	config.m_watchdog_time = _time; \


// core functions
#define MCFG_NVRAM_HANDLER(_func) \
	config.m_nvram_handler = NVRAM_HANDLER_NAME(_func); \

#define MCFG_MEMCARD_HANDLER(_func) \
	config.m_memcard_handler = MEMCARD_HANDLER_NAME(_func); \

#define MCFG_NVRAM_HANDLER_CLEAR() \
	config.m_nvram_handler = NULL; \


// core video parameters
#define MCFG_VIDEO_ATTRIBUTES(_flags) \
	config.m_video_attributes = _flags; \

#define MCFG_GFXDECODE(_gfx) \
	config.m_gfxdecodeinfo = GFXDECODE_NAME(_gfx); \

#define MCFG_PALETTE_LENGTH(_length) \
	config.m_total_colors = _length; \

#define MCFG_DEFAULT_LAYOUT(_layout) \
	config.m_default_layout = &(_layout)[0]; \


// add/remove devices
#define MCFG_DEVICE_ADD(_tag, _type, _clock) \
	device = config.device_add(owner, _tag, _type, _clock); \

#define MCFG_DEVICE_REPLACE(_tag, _type, _clock) \
	device = config.device_replace(owner, _tag, _type, _clock); \

#define MCFG_DEVICE_REMOVE(_tag) \
	device = config.device_remove(owner, _tag); \

#define MCFG_DEVICE_MODIFY(_tag)	\
	device = config.device_find(owner, _tag); \


#endif	/* __MCONFIG_H__ */
