// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Geneve main board components.
    See genboard.c for documentation.

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/
#ifndef MAME_BUS_TI99_INTERNAL_GENBOARD_H
#define MAME_BUS_TI99_INTERNAL_GENBOARD_H

#pragma once

#include "bus/ti99/ti99defs.h"
#include "bus/ti99/peb/peribox.h"
#include "video/v9938.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/sn76496.h"
#include "machine/mm58274c.h"
#include "machine/at29x.h"
#include "machine/ram.h"

enum
{
	GENEVE_GM_TURBO = 1,
	GENEVE_GM_TIM = 2
};

enum
{
	GENEVE_098 = 0,
	GENEVE_100,
	GENEVE_PFM512,
	GENEVE_PFM512A
};

#define GENEVE_KEYBOARD_TAG   "gkeyboard"
#define GENEVE_MAPPER_TAG     "gmapper"
#define GENEVE_MOUSE_TAG      "gmouse"
#define GENEVE_CLOCK_TAG      "mm58274c"
#define GENEVE_PFM512_TAG      "pfm512"
#define GENEVE_PFM512A_TAG     "pfm512a"

#define GENEVE_SRAM_TAG  "sram"
#define GENEVE_DRAM_TAG  "dram"
#define GENEVE_SRAM_PAR_TAG  ":sram"
#define GENEVE_DRAM_PAR_TAG  ":dram"

namespace bus { namespace ti99 { namespace internal {

/*****************************************************************************/

class geneve_keyboard_device : public device_t
{
public:
	geneve_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_WRITE_LINE_MEMBER( reset_line );
	DECLARE_WRITE_LINE_MEMBER( send_scancodes );
	DECLARE_WRITE_LINE_MEMBER( clock_control );
	uint8_t get_recent_key();

	template <class Object> devcb_base &set_int_callback(Object &&cb) { return m_interrupt.set_callback(std::forward<Object>(cb)); }

protected:
	void               device_start() override;
	void               device_reset() override;
	ioport_constructor device_input_ports() const override;
	void               device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	devcb_write_line  m_interrupt;    // Keyboard interrupt to console
	required_ioport_array<8> m_keys;

private:
	static constexpr unsigned KEYQUEUESIZE = 256;
	static constexpr unsigned MAXKEYMSGLENGTH = 10;
	static constexpr unsigned KEYAUTOREPEATDELAY = 30;
	static constexpr unsigned KEYAUTOREPEATRATE = 6;

	void    post_in_key_queue(int keycode);
	void    signal_when_key_available();
	void    poll();

	bool    m_key_reset;
	int     m_key_queue_length;
	uint8_t   m_key_queue[KEYQUEUESIZE];
	int     m_key_queue_head;
	bool    m_key_in_buffer;
	uint32_t  m_key_state_save[4];
	bool    m_key_numlock_state;

	int     m_key_ctrl_state;
	int     m_key_alt_state;
	int     m_key_real_shift_state;

	bool    m_key_fake_shift_state;
	bool    m_key_fake_unshift_state;

	int     m_key_autorepeat_key;
	int     m_key_autorepeat_timer;

	bool    m_keep_keybuf;
	bool    m_keyboard_clock;

	emu_timer*      m_timer;
};

#define MCFG_GENEVE_KBINT_HANDLER( _intcallb ) \
	devcb = &downcast<bus::ti99::internal::geneve_keyboard_device &>(*device).set_int_callback(DEVCB_##_intcallb);

/*****************************************************************************/

class geneve_mapper_device : public device_t
{
public:
	geneve_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_geneve_mode(bool geneve);
	void set_direct_mode(bool direct);
	void set_cartridge_size(int size);
	void set_cartridge_writable(int base, bool write);
	void set_video_waitstates(bool wait);
	void set_extra_waitstates(bool wait);

	DECLARE_READ8_MEMBER( readm );
	DECLARE_WRITE8_MEMBER( writem );
	DECLARE_SETOFFSET_MEMBER( setoffset );

	DECLARE_INPUT_CHANGED_MEMBER( settings_changed );

	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( dbin_in );

	// PFM support
	DECLARE_WRITE_LINE_MEMBER( pfm_select_lsb );
	DECLARE_WRITE_LINE_MEMBER( pfm_select_msb );
	DECLARE_WRITE_LINE_MEMBER( pfm_output_enable );

	template <class Object> devcb_base &set_ready_callback(Object &&cb) { return m_ready.set_callback(std::forward<Object>(cb)); }

protected:
	void    device_start() override;
	void    device_reset() override;

private:
	// GROM simulation
	bool    m_gromwaddr_LSB;
	bool    m_gromraddr_LSB;
	int     m_grom_address;
	DECLARE_READ8_MEMBER( read_grom );
	DECLARE_WRITE8_MEMBER( write_grom );

	// wait states
	void        set_wait(int min);
	void        set_ext_wait(int min);
	bool        m_video_waitstates;
	bool        m_extra_waitstates;
	bool        m_ready_asserted;

	bool        m_read_mode;

	bool        m_debug_no_ws;

	// Mapper function
	typedef struct
	{
		int     function;
		offs_t  offset;
		offs_t  physaddr;
	} decdata;

	bool    m_geneve_mode;
	bool    m_direct_mode;
	int     m_cartridge_size;
	bool    m_cartridge_secondpage;
	bool    m_cartridge6_writable;
	bool    m_cartridge7_writable;
	int     m_map[8];

	void    decode(address_space& space, offs_t offset, bool read_mode, decdata* dec);
	decdata m_decoded;

	// Genmod modifications
	bool    m_turbo;
	bool    m_genmod;
	bool    m_timode;

	// PFM mod (0 = none, 1 = AT29C040, 2 = AT29C040A)
	DECLARE_READ8_MEMBER( read_from_pfm );
	DECLARE_WRITE8_MEMBER( write_to_pfm );
	void    set_boot_rom(int selection);
	int     m_pfm_mode;
	int     m_pfm_bank;
	bool    m_pfm_output_enable;

	// SRAM access
	int     m_sram_mask;
	int     m_sram_val;

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Counter for the wait states.
	int     m_waitcount;
	int     m_ext_waitcount;

	// Devices
	required_device<mm58274c_device>     m_clock;
	required_device<tms9995_device>      m_cpu;
	required_device<at29c040_device>     m_pfm512;
	required_device<at29c040a_device>    m_pfm512a;
	required_device<sn76496_base_device> m_sound;

	required_device<bus::ti99::internal::geneve_keyboard_device> m_keyboard;
	required_device<v9938_device>           m_video;
	required_device<bus::ti99::peb::peribox_device>         m_peribox;
	uint8_t*                  m_eprom;
	required_device<ram_device> m_sram;
	required_device<ram_device> m_dram;
};

#define MCFG_GENEVE_READY_HANDLER( _intcallb ) \
	devcb = &downcast<bus::ti99::internal::geneve_mapper_device &>(*device).set_ready_callback(DEVCB_##_intcallb);

} } } // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(GENEVE_KEYBOARD, bus::ti99::internal, geneve_keyboard_device)
DECLARE_DEVICE_TYPE_NS(GENEVE_MAPPER,   bus::ti99::internal, geneve_mapper_device)

#endif // MAME_BUS_TI99_INTERNAL_GENBOARD_H
