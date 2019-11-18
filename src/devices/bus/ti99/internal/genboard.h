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
#include "bus/pc_kbd/pc_kbdc.h"

enum
{
	GENEVE_GM_TURBO = 1,
	GENEVE_GM_TIM = 2
};

enum
{
	GENEVE_EPROM = 0,
	GENEVE_PFM512,
	GENEVE_PFM512A
};

#define GENEVE_GATE_ARRAY_TAG     "gatearray"
#define GENEVE_MOUSE_TAG      "gmouse"
#define GENEVE_CLOCK_TAG      "mm58274c"
#define GENEVE_PFM512_TAG      "pfm512"
#define GENEVE_PFM512A_TAG     "pfm512a"
#define GENEVE_KEYBOARD_CONN_TAG "keybconn"

#define GENEVE_SRAM_TAG  "sram"
#define GENEVE_DRAM_TAG  "dram"
#define GENEVE_SRAM_PAR_TAG  ":sram"
#define GENEVE_DRAM_PAR_TAG  ":dram"

namespace bus { namespace ti99 { namespace internal {

/*****************************************************************************/

class geneve_gate_array_device : public device_t
{
public:
	geneve_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_geneve_mode(bool geneve);
	void set_direct_mode(bool direct);
	void set_cartridge_size(int size);
	void set_cartridge_writable(int base, bool write);
	void set_video_waitstates(bool wait);
	void set_extra_waitstates(bool wait);

	uint8_t readm(offs_t offset);
	void writem(offs_t offset, uint8_t data);
	void setaddress(offs_t offset, uint8_t busctrl);

	DECLARE_INPUT_CHANGED_MEMBER( settings_changed );

	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( dbin_in );

	// Keyboard support
	DECLARE_WRITE_LINE_MEMBER( set_keyboard_clock );
	DECLARE_WRITE_LINE_MEMBER( enable_shift_register );
	DECLARE_WRITE_LINE_MEMBER( kbdclk );
	DECLARE_WRITE_LINE_MEMBER( kbddata );

	// PFM support
	DECLARE_WRITE_LINE_MEMBER( pfm_select_lsb );
	DECLARE_WRITE_LINE_MEMBER( pfm_select_msb );
	DECLARE_WRITE_LINE_MEMBER( pfm_output_enable );

	auto ready_cb() { return m_ready.bind(); }
	auto kbdint_cb() { return m_keyint.bind(); }

protected:
	geneve_gate_array_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void    common_reset();

	/*
	Constants for mapper decoding. Naming scheme:
	M=Mapper, L=Logical space; P=Physical space
	*/
	typedef enum
	{
		MUNDEF=0,

		MLVIDEO,
		MLMAPPER,
		MLKEY,
		MLSOUND,
		MLCLOCK,
		MLGROM,

		MPDRAM,
		MPEXP,
		MPEPROM,
		MPSRAM,

		MBOX
	} decfunct_t;

	// Mapper function
	typedef struct
	{
		int     function;       // must be a fundamental type to be saveable
		offs_t  offset;         // Logical address
		offs_t  physaddr;       // Physical address
		int     wait;           // Wait states
	} decdata;

	uint8_t*                  m_eprom;
	int     m_pbox_prefix;
	int     m_boot_rom;

private:
	void    device_start() override;
	virtual void device_reset() override;

	// GROM simulation
	bool    m_gromwaddr_LSB;
	bool    m_gromraddr_LSB;
	int     m_grom_address;
	uint8_t read_grom(offs_t offset);
	void write_grom(offs_t offset, uint8_t data);

	// wait states
	void    set_wait(int min);
	void    set_video_waitcount(int min);
	bool    m_video_waitstates;
	bool    m_extra_waitstates;
	bool    m_ready_asserted;

	bool    m_read_mode;

	bool    m_debug_no_ws;
	bool    m_geneve_mode;
	bool    m_direct_mode;
	int     m_cartridge_size;
	bool    m_cartridge_secondpage;
	bool    m_cartridge6_writable;
	bool    m_cartridge7_writable;
	int     m_map[8];

	// The result of decoding
	decdata m_decoded;

	// Static decoder entry for the logical space
	// Not all entries apply for native mode, e.g. there is no GROM in native
	// mode. In that case the base and mask are 0000, and the entry must be
	// skipped. Speech is accessible in the physical space in native mode.
	// All entries have a wait state count of 1.
	typedef struct
	{
		offs_t      genbase;       // Base address in native mode
		int         genmask;       // Bits that also match this entry
		offs_t      tibase;        // Base address in TI mode
		int         timask;        // Bits that also match this entry
		int         writeoff;      // Additional offset in TI mode for writing
		decfunct_t  function;      // Decoded function
		const char* description;   // Good for logging
	} logentry_t;

	// Static decoder entry for the physical space
	// There are no differences between native mode and TI mode.
	typedef struct
	{
		offs_t      base;           // Base address
		int         mask;           // Bits that also match this entry
		decfunct_t  function;       // Decoded function
		int         wait;           // Wait states
		const char* description;    // Good for logging
	} physentry_t;

	static const geneve_gate_array_device::logentry_t s_logmap[];
	static const geneve_gate_array_device::physentry_t s_physmap[];

	void    decode_logical(bool reading, decdata* dec);
	void    map_address(bool reading, decdata* dec);
	void    decode_physical(decdata* dec);
	// This is the hook for Genmod. The normal Geneve just does nothing here.
	virtual void decode_mod(decdata* dec) { };

	// PFM mod (0 = none, 1 = AT29C040, 2 = AT29C040A)
	uint8_t boot_rom(offs_t offset);
	void write_to_pfm(offs_t offset, uint8_t data);
	int     m_pfm_bank;
	bool    m_pfm_output_enable;

	// SRAM access
	int     m_sram_mask;
	int     m_sram_val;

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Keyboard interrupt
	devcb_write_line m_keyint;

	// Counter for the wait states.
	int     m_waitcount;
	int     m_video_waitcount;

	// Keyboard support
	uint16_t m_keyboard_shift_reg;
	line_state m_keyboard_last_clock;
	line_state m_keyboard_data_in;
	bool m_shift_reg_enabled;
	void shift_reg_changed();

	// Devices
	required_device<tms9995_device>      m_cpu;
	required_device<sn76496_base_device> m_sound;
	required_device<v9938_device>        m_video;
	required_device<mm58274c_device>     m_rtc;
	required_device<ram_device>          m_sram;
	required_device<ram_device>          m_dram;

	required_device<bus::ti99::peb::peribox_device> m_peribox;

	required_device<at29c040_device>     m_pfm512;
	required_device<at29c040a_device>    m_pfm512a;

	required_device<pc_kbdc_device> m_keyb_conn;
};

class genmod_gate_array_device : public geneve_gate_array_device
{
public:
	genmod_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void decode_mod(decdata* dec) override;
	void device_reset() override;
	DECLARE_INPUT_CHANGED_MEMBER( setgm_changed );

private:
	// Genmod modifications
	bool    m_gm_timode;
	bool    m_turbo;
};

} } } // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(GENEVE_GATE_ARRAY,   bus::ti99::internal, geneve_gate_array_device)
DECLARE_DEVICE_TYPE_NS(GENMOD_GATE_ARRAY,   bus::ti99::internal, genmod_gate_array_device)

#endif // MAME_BUS_TI99_INTERNAL_GENBOARD_H
