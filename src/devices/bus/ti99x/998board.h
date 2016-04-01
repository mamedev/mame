// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99/8 main board logic

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    See 998board.c for documentation

    Michael Zapf

*****************************************************************************/

#ifndef __MAPPER8__
#define __MAPPER8__

#include "emu.h"
#include "ti99defs.h"
#include "machine/tmc0430.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"
#include "sound/tms5220.h"
#include "gromport.h"
#include "bus/ti99_peb/peribox.h"

extern const device_type MAINBOARD8;

#define VAQUERRO_TAG "vaquerro"
#define MOFETTA_TAG "mofetta"
#define AMIGO_TAG "amigo"
#define OSO_TAG "oso"

#define SRAM_SIZE 2048
#define DRAM_SIZE 65536

class mainboard8_device;
extern const device_type VAQUERRO;
extern const device_type MOFETTA;
extern const device_type AMIGO;
extern const device_type OSO;


enum
{
	SGMSEL = 1,
	TSGSEL = 2,
	P8GSEL = 4,
	P3GSEL = 8,
	VIDSEL = 16
};

/*
    Wait state generator (part of Vaquerro)
*/
class waitstate_generator
{
public:
	waitstate_generator() :
		m_counting(false),
		m_generate(false),
		m_counter(0),
		m_addressed(true),
		m_ready(true)   { };
	void select_in(bool addressed);
	virtual void ready_in(line_state ready) =0;
	virtual void clock_in(line_state clkout) =0;
	void treset_in(line_state reset);

	int select_out();
	void init(int select_value) { m_selvalue = select_value; }

	line_state ready_out();

	bool is_counting();
	bool is_generating();
	bool is_ready();

protected:
	// Two flipflops
	bool m_counting;
	bool m_generate;
	// Counter
	int  m_counter;

	// Select value (indicates selected line)
	int  m_selvalue;

	// Line state flags
	bool m_addressed;
	bool m_ready;
};

class grom_waitstate_generator : public waitstate_generator
{
public:
	void ready_in(line_state ready) override;
	void clock_in(line_state clkout) override;
};

class video_waitstate_generator : public waitstate_generator
{
public:
	void ready_in(line_state ready) override { };
	void clock_in(line_state clkout) override;
};

/*
    Custom chip: Vaquerro
*/
class vaquerro_device : public device_t
{
public:
	vaquerro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void device_start() override;
	void device_reset() override;

	line_state ready();
	void treset();

	DECLARE_READ8_MEMBER( read );
	DECLARE_SETADDRESS_DBIN_MEMBER( set_address );

	DECLARE_READ_LINE_MEMBER( sprd_out );
	DECLARE_READ_LINE_MEMBER( spwt_out );
	DECLARE_READ_LINE_MEMBER( sccs_out );
	DECLARE_READ_LINE_MEMBER( sromcs_out );

	// Collective select line query
	int gromcs_out();

	DECLARE_READ_LINE_MEMBER( vdprd_out );
	DECLARE_READ_LINE_MEMBER( vdpwt_out );
	DECLARE_READ_LINE_MEMBER( lascsq_out );
	DECLARE_READ_LINE_MEMBER( ggrdy_out );
	DECLARE_WRITE_LINE_MEMBER( hold_cpu );

	DECLARE_WRITE_LINE_MEMBER( crus_in );
	DECLARE_WRITE_LINE_MEMBER( crusgl_in );
	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( memen_in );

	DECLARE_WRITE_LINE_MEMBER( sgmry );
	DECLARE_WRITE_LINE_MEMBER( tsgry );
	DECLARE_WRITE_LINE_MEMBER( p8gry );
	DECLARE_WRITE_LINE_MEMBER( p3gry );

private:
	// Memory cycle state
	bool m_memen;

	// Waiting for video
	bool m_video_wait;

	// State of the CRUS line
	line_state m_crus;

	// Are the GROM libraries turned on?
	bool m_crugl;

	// Do we have a logical address space match?
	bool m_lasreq = false;

	// Keep the decoding result (opens the SRY gate)
	bool m_grom_or_video = false;

	// Select lines
	bool m_spwt;
	bool m_sccs;
	bool m_sromcs;
	bool m_sprd;
	bool m_vdprd;
	bool m_vdpwt;

	// Collective GROM select state
	int m_gromsel;

	// Outgoing READY
	line_state m_ggrdy;

	// Outgoing READY latch (common flipflop driving SRY)
	bool m_sry;

	// Holds the A14 address line state. We need this for the clock_in method.
	line_state m_a14;

	// Keeps the recent DBIN level
	line_state m_dbin_level;

	// Wait state logic components
	grom_waitstate_generator m_sgmws, m_tsgws, m_p8gws, m_p3gws;
	video_waitstate_generator m_vidws;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;
};

/*
    Custom chip: Mofetta
*/
class mofetta_device : public device_t
{
public:
	mofetta_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void device_start() override;
	void device_reset() override;

	DECLARE_WRITE8_MEMBER( cruwrite );
	DECLARE_SETADDRESS_DBIN_MEMBER( set_address );

	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( msast_in );
	DECLARE_WRITE_LINE_MEMBER( lascs_in );
	DECLARE_WRITE_LINE_MEMBER( pmemen_in );
	DECLARE_WRITE_LINE_MEMBER( skdrcs_in );

	DECLARE_READ8_MEMBER( rom1cs_out );
	DECLARE_READ_LINE_MEMBER( gromclk_out );

	DECLARE_READ_LINE_MEMBER( alccs_out );
	DECLARE_READ_LINE_MEMBER( prcs_out );
	DECLARE_READ_LINE_MEMBER( cmas_out );
	DECLARE_READ_LINE_MEMBER( dbc_out );

	DECLARE_READ_LINE_MEMBER( rom1cs_out );
	DECLARE_READ_LINE_MEMBER( rom1am_out );
	DECLARE_READ_LINE_MEMBER( rom1al_out );

private:
	// Memory cycle state
	bool m_pmemen;

	// Logical access
	bool m_lasreq;

	// DRAM access
	bool m_skdrcs;

	// Holds the decoding result; essentially names the selected line
	bool m_gromclk_up;

	// Have we got the upper word of the address?
	bool m_gotfirstword;

	// Address latch
	int m_address_latch;

	// Most significant byte of the 24-bit address
	int m_prefix;

	// CRU select of the 1700 device
	bool m_alcpg;

	// CRU select of the 2700 device
	bool m_txspg;

	// ROM1 select lines
	bool m_rom1cs;
	bool m_rom1am;
	bool m_rom1al;

	// OSO select
	bool m_alccs;

	// Pascal ROM select line
	bool m_prcs;

	// Cartridge port select line
	bool m_cmas;

	// GROM clock count (as frequency divider)
	int m_gromclock_count;

	// Remember last msast state for edge detection
	line_state m_msast;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;
};

/*
    Custom chip: Amigo
*/
class amigo_device : public device_t
{
public:
	amigo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void device_start() override;
	void device_reset() override;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_SETOFFSET_MEMBER( set_address );

	DECLARE_WRITE_LINE_MEMBER( srdy_in );
	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( crus_in );
	DECLARE_WRITE_LINE_MEMBER( lascs_in );
	DECLARE_WRITE_LINE_MEMBER( memen_in );

	DECLARE_WRITE_LINE_MEMBER( holda_in );

	DECLARE_READ_LINE_MEMBER( cpury_out );
	DECLARE_READ_LINE_MEMBER( sramcs_out );
	DECLARE_READ_LINE_MEMBER( skdrcs_out );

	void connect_sram(UINT8* sram) { m_sram = sram; }
	bool mapper_accessed() { return m_mapper_accessed; }

private:
	// Memory cycle state
	bool m_memen;

	// DMA methods for loading/saving maps
	void mapper_load();
	void mapper_save();

	// Address mapper registers. Each offset is selected by the first 4 bits
	// of the logical address.
	UINT32  m_base_register[16];

	// Indicates a logical space access
	bool m_logical_space;

	// Physical address
	UINT32  m_physical_address;

	// Pointer to SRAM where AMIGO needs to upload/download its map values
	UINT8* m_sram;

	// Pointer to mainboard
	mainboard8_device* m_mainboard;

	// Keep the system ready state
	line_state m_srdy;

	// Outgoing READY level
	line_state m_ready_out;

	// Keep the CRUS setting
	line_state m_crus;

	// State of the address creation
	int m_amstate;

	// Protection flags
	int m_protflag;

	// Accessing SRAM
	bool m_sram_accessed;

	// Accessing DRAM
	bool m_dram_accessed;

	// Accessing the mapper
	bool m_mapper_accessed;

	// HOLDA flag
	bool m_hold_acknowledged;

	// Address in SRAM during DMA
	UINT32  m_sram_address;

	// Number of the currently loaded/save base register
	int m_basereg;

	// Latched value for mapper DMA transfer
	UINT32 m_mapvalue;
};

/*
    Custom chip: OSO
*/
class oso_device : public device_t
{
public:
	oso_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void device_start() override;

private:
	UINT8 m_data;
	UINT8 m_status;
	UINT8 m_control;
	UINT8 m_xmit;
};

class mainboard8_device : public device_t
{
public:
	mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Memory space
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_SETOFFSET_MEMBER( setoffset );

	// I/O space
	DECLARE_READ8Z_MEMBER( crureadz );
	DECLARE_WRITE8_MEMBER( cruwrite );

	// Control lines
	DECLARE_WRITE_LINE_MEMBER( clock_in );
	DECLARE_WRITE_LINE_MEMBER( dbin_in );
	DECLARE_WRITE_LINE_MEMBER( msast_in );
	DECLARE_WRITE_LINE_MEMBER( crus_in );
	DECLARE_WRITE_LINE_MEMBER( ptgen_in );
	DECLARE_WRITE_LINE_MEMBER( reset_console );
	DECLARE_WRITE_LINE_MEMBER( hold_cpu );
	DECLARE_WRITE_LINE_MEMBER( ggrdy_in );

	DECLARE_WRITE_LINE_MEMBER( holda_line );

	template<class _Object> static devcb_base &set_ready_wr_callback(device_t &device, _Object object)
	{
		return downcast<mainboard8_device &>(device).m_ready.set_callback(object);
	}

	template<class _Object> static devcb_base &set_reset_wr_callback(device_t &device, _Object object)
	{
		return downcast<mainboard8_device &>(device).m_console_reset.set_callback(object);
	}

	template<class _Object> static devcb_base &set_hold_wr_callback(device_t &device, _Object object)
	{
		return downcast<mainboard8_device &>(device).m_hold_line.set_callback(object);
	}

	void set_paddress(int address);

	// Ready lines from GROMs
	DECLARE_WRITE_LINE_MEMBER( system_grom_ready );
	DECLARE_WRITE_LINE_MEMBER( ptts_grom_ready );
	DECLARE_WRITE_LINE_MEMBER( p8_grom_ready );
	DECLARE_WRITE_LINE_MEMBER( p3_grom_ready );
	DECLARE_WRITE_LINE_MEMBER( sound_ready );
	DECLARE_WRITE_LINE_MEMBER( speech_ready );
	DECLARE_WRITE_LINE_MEMBER( pbox_ready );

	// Emulation
	// void set_gromport(gromport_device* dev) { m_gromport = dev; }

protected:
	void device_start(void) override;
	void device_reset(void) override;
	machine_config_constructor device_mconfig_additions() const override;

private:
	// Holds the state of the A14 line
	bool    m_A14_set;

	// Propagates the end of the memory cycle
	void cycle_end();

	// Original logical address.
	int     m_logical_address;

	// Mapped physical address.
	int     m_physical_address;

	// Hold the address space value so that we can use it in other methods.
	address_space*  m_space;

	// Indicates that a byte is waiting on the data bus (see m_latched_data)
	bool    m_pending_write;

	// Hold the value of the data bus. In a real machine, the data bus continues
	// to show that value, but in this emulation we have a push mechanism.
	UINT8   m_latched_data;

	// Hold the level of the GROMCLK line
	int m_gromclk;

	// Selecting GROM libraries
	void select_groms();

	// Previous select state
	int m_prev_grom;

	// Ready states
	bool m_speech_ready;
	bool m_sound_ready;
	bool m_pbox_ready;

	// Keeps the recent DBIN level
	line_state m_dbin_level;

	// Ready line to the CPU
	devcb_write_line m_ready;

	// Reset line to the main system
	devcb_write_line m_console_reset;

	// Hold line to the main system
	devcb_write_line m_hold_line;

	// Custom chips
	required_device<vaquerro_device> m_vaquerro;
	required_device<mofetta_device>  m_mofetta;
	required_device<amigo_device>    m_amigo;
	required_device<oso_device>       m_oso;

	// Debugging
	line_state m_last_ready;

	// Video processor
	tms9118_device* m_video;

	// Sound generator
	sn76496_base_device* m_sound;

	// Speech processor
	cd2501ecd_device* m_speech;

	// System GROM library
	tmc0430_device* m_sgrom[3];

	// Text-to-speech GROM library
	tmc0430_device* m_tsgrom[8];

	// Pascal 8 GROM library
	tmc0430_device* m_p8grom[8];

	// Pascal 3 GROM library
	tmc0430_device* m_p3grom[3];

	// Gromport (cartridge port)
	gromport_device* m_gromport;

	// Peripheral box
	peribox_device* m_peb;

	// Memory
	std::unique_ptr<UINT8[]>    m_sram;
	std::unique_ptr<UINT8[]>    m_dram;

	// ROM area of the system.
	UINT8*   m_rom0;
	UINT8*   m_rom1;
	UINT8*   m_pascalrom;
};

#define MCFG_MAINBOARD8_READY_CALLBACK(_write) \
	devcb = &mainboard8_device::set_ready_wr_callback(*device, DEVCB_##_write);

#define MCFG_MAINBOARD8_RESET_CALLBACK(_write) \
	devcb = &mainboard8_device::set_reset_wr_callback(*device, DEVCB_##_write);

#define MCFG_MAINBOARD8_HOLD_CALLBACK(_write) \
	devcb = &mainboard8_device::set_hold_wr_callback(*device, DEVCB_##_write);


#endif
