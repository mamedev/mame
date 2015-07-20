// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.h

    General functions for Atari games.

***************************************************************************/

#ifndef __MACHINE_ATARIGEN__
#define __MACHINE_ATARIGEN__

#include "machine/nvram.h"
#include "machine/er2055.h"
#include "machine/eeprompar.h"
#include "video/atarimo.h"
#include "cpu/m6502/m6502.h"
#include "sound/okim6295.h"
#include "includes/slapstic.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ATARI_CLOCK_14MHz   XTAL_14_31818MHz
#define ATARI_CLOCK_20MHz   XTAL_20MHz
#define ATARI_CLOCK_32MHz   XTAL_32MHz
#define ATARI_CLOCK_50MHz   XTAL_50MHz



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ATARI_SOUND_COMM_ADD(_tag, _soundcpu, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_SOUND_COMM, 0) \
	atari_sound_comm_device::static_set_sound_cpu(*device, _soundcpu); \
	devcb = &atari_sound_comm_device::static_set_main_int_cb(*device, DEVCB_##_intcb);



#define MCFG_ATARI_VAD_ADD(_tag, _screen, _intcb) \
	MCFG_DEVICE_ADD(_tag, ATARI_VAD, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen) \
	devcb = &atari_vad_device::static_set_scanline_int_cb(*device, DEVCB_##_intcb);

#define MCFG_ATARI_VAD_PLAYFIELD(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":playfield"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE("^" _gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_COLS, 64,64) }

#define MCFG_ATARI_VAD_PLAYFIELD2(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":playfield2"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE("^" _gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_COLS, 64,64) \
	MCFG_TILEMAP_TRANSPARENT_PEN(0) }

#define MCFG_ATARI_VAD_ALPHA(_class, _gfxtag, _getinfo) \
	{ std::string fulltag(device->tag()); fulltag.append(":alpha"); device_t *device; \
	MCFG_TILEMAP_ADD(fulltag.c_str()) \
	MCFG_TILEMAP_GFXDECODE("^" _gfxtag) \
	MCFG_TILEMAP_BYTES_PER_ENTRY(2) \
	MCFG_TILEMAP_INFO_CB_DEVICE(DEVICE_SELF_OWNER, _class, _getinfo) \
	MCFG_TILEMAP_TILE_SIZE(8,8) \
	MCFG_TILEMAP_LAYOUT_STANDARD(SCAN_ROWS, 64,32) \
	MCFG_TILEMAP_TRANSPARENT_PEN(0) }

#define MCFG_ATARI_VAD_MOB(_config, _gfxtag) \
	{ std::string fulltag(device->tag()); fulltag.append(":mob"); device_t *device; \
	MCFG_ATARI_MOTION_OBJECTS_ADD(fulltag.c_str(), "^^screen", _config) \
	MCFG_ATARI_MOTION_OBJECTS_GFXDECODE("^" _gfxtag) }



#define MCFG_ATARI_EEPROM_2804_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARI_EEPROM_2804, 0)

#define MCFG_ATARI_EEPROM_2816_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARI_EEPROM_2816, 0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define PORT_ATARI_COMM_SOUND_TO_MAIN_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, sound_to_main_ready)

#define PORT_ATARI_COMM_MAIN_TO_SOUND_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, main_to_sound_ready)


// ======================> atari_sound_comm_device

// device type definition
extern const device_type ATARI_SOUND_COMM;

class atari_sound_comm_device :  public device_t
{
public:
	// construction/destruction
	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_sound_cpu(device_t &device, const char *cputag);
	template<class _Object> static devcb_base &static_set_main_int_cb(device_t &device, _Object object) { return downcast<atari_sound_comm_device &>(device).m_main_int_cb.set_callback(object); }

	// getters
	DECLARE_READ_LINE_MEMBER(main_to_sound_ready) { return m_main_to_sound_ready ? ASSERT_LINE : CLEAR_LINE; }
	DECLARE_READ_LINE_MEMBER(sound_to_main_ready) { return m_sound_to_main_ready ? ASSERT_LINE : CLEAR_LINE; }

	// main cpu accessors (forward internally to the atari_sound_comm_device)
	DECLARE_WRITE8_MEMBER(main_command_w);
	DECLARE_READ8_MEMBER(main_response_r);
	DECLARE_WRITE16_MEMBER(sound_reset_w);

	// sound cpu accessors
	void sound_cpu_reset() { synchronize(TID_SOUND_RESET, 1); }
	DECLARE_WRITE8_MEMBER(sound_response_w);
	DECLARE_READ8_MEMBER(sound_command_r);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_READ8_MEMBER(sound_irq_ack_r);
	INTERRUPT_GEN_MEMBER(sound_irq_gen);

	// additional helpers
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_gen);

protected:
	// sound I/O helpers
	void update_sound_irq();
	void delayed_sound_reset(int param);
	void delayed_sound_write(int data);
	void delayed_6502_write(int data);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// timer IDs
	enum
	{
		TID_SOUND_RESET,
		TID_SOUND_WRITE,
		TID_6502_WRITE
	};

	// configuration state
	const char *        m_sound_cpu_tag;
	devcb_write_line   m_main_int_cb;

	// internal state
	m6502_device *      m_sound_cpu;
	bool                m_main_to_sound_ready;
	bool                m_sound_to_main_ready;
	UINT8               m_main_to_sound_data;
	UINT8               m_sound_to_main_data;
	UINT8               m_timed_int;
	UINT8               m_ym2151_int;
};



// ======================> atari_vad_device

// device type definition
extern const device_type ATARI_VAD;

class atari_vad_device :    public device_t,
							public device_video_interface
{
public:
	// construction/destruction
	atari_vad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &static_set_scanline_int_cb(device_t &device, _Object object) { return downcast<atari_vad_device &>(device).m_scanline_int_cb.set_callback(object); }

	// getters
	tilemap_device *alpha() const { return m_alpha_tilemap; }
	tilemap_device *playfield() const { return m_playfield_tilemap; }
	tilemap_device *playfield2() const { return m_playfield2_tilemap; }
	atari_motion_objects_device *mob() const { return m_mob; }

	// read/write handlers
	DECLARE_READ16_MEMBER(control_read);
	DECLARE_WRITE16_MEMBER(control_write);

	// playfield/alpha tilemap helpers
	DECLARE_WRITE16_MEMBER(alpha_w);
	DECLARE_WRITE16_MEMBER(playfield_upper_w);
	DECLARE_WRITE16_MEMBER(playfield_latched_lsb_w);
	DECLARE_WRITE16_MEMBER(playfield_latched_msb_w);
	DECLARE_WRITE16_MEMBER(playfield2_latched_msb_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// timer IDs
	enum
	{
		TID_SCANLINE_INT,
		TID_TILEROW_UPDATE,
		TID_EOF
	};

	// internal helpers
	void internal_control_write(offs_t offset, UINT16 newword);
	void update_pf_xscrolls();
	void update_parameter(UINT16 newword);
	void update_tilerow(emu_timer &timer, int scanline);
	void eof_update(emu_timer &timer);

	// configuration state
	devcb_write_line   m_scanline_int_cb;

	// internal state
	optional_device<tilemap_device> m_alpha_tilemap;
	required_device<tilemap_device> m_playfield_tilemap;
	optional_device<tilemap_device> m_playfield2_tilemap;
	optional_device<atari_motion_objects_device> m_mob;
	optional_shared_ptr<UINT16> m_eof_data;

	emu_timer *         m_scanline_int_timer;
	emu_timer *         m_tilerow_update_timer;
	emu_timer *         m_eof_timer;

	UINT32              m_palette_bank;            // which palette bank is enabled
	//UINT32              m_pf0_xscroll;             // playfield 1 xscroll
	UINT32              m_pf0_xscroll_raw;         // playfield 1 xscroll raw value
	UINT32              m_pf0_yscroll;             // playfield 1 yscroll
	UINT32              m_pf1_xscroll_raw;         // playfield 2 xscroll raw value
	UINT32              m_pf1_yscroll;             // playfield 2 yscroll
	UINT32              m_mo_xscroll;              // sprite xscroll
	UINT32              m_mo_yscroll;              // sprite xscroll

	UINT16              m_control[0x40/2];          // control data
};


// ======================> atari_eeprom_device

// device type definition
extern const device_type ATARI_EEPROM_2804;
extern const device_type ATARI_EEPROM_2816;

class atari_eeprom_device : public device_t
{
protected:
	// construction/destruction
	atari_eeprom_device(const machine_config &mconfig, device_type devtype, const char *name, const char *tag, device_t *owner, const char *shortname, const char *file);

public:
	// unlock controls
	DECLARE_READ8_MEMBER(unlock_read);
	DECLARE_WRITE8_MEMBER(unlock_write);
	DECLARE_READ16_MEMBER(unlock_read);
	DECLARE_WRITE16_MEMBER(unlock_write);
	DECLARE_READ32_MEMBER(unlock_read);
	DECLARE_WRITE32_MEMBER(unlock_write);

	// EEPROM read/write
	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal state
	required_device<eeprom_parallel_28xx_device> m_eeprom;

	// live state
	bool        m_unlocked;
};

class atari_eeprom_2804_device : public atari_eeprom_device
{
public:
	// construction/destruction
	atari_eeprom_2804_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};

class atari_eeprom_2816_device : public atari_eeprom_device
{
public:
	// construction/destruction
	atari_eeprom_2816_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
};



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

struct atarigen_screen_timer
{
	screen_device *screen;
	emu_timer *         scanline_interrupt_timer;
	emu_timer *         scanline_timer;
};


class atarigen_state : public driver_device
{
public:
	// construction/destruction
	atarigen_state(const machine_config &mconfig, device_type type, const char *tag);

	// users must call through to these
	virtual void machine_start();
	virtual void machine_reset();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// callbacks provided by the derived class
	virtual void update_interrupts() = 0;
	virtual void scanline_update(screen_device &screen, int scanline);

	// interrupt handling
	void scanline_int_set(screen_device &screen, int scanline);
	DECLARE_WRITE_LINE_MEMBER(scanline_int_write_line);
	INTERRUPT_GEN_MEMBER(scanline_int_gen);
	DECLARE_WRITE16_MEMBER(scanline_int_ack_w);

	DECLARE_WRITE_LINE_MEMBER(sound_int_write_line);
	INTERRUPT_GEN_MEMBER(sound_int_gen);
	DECLARE_WRITE16_MEMBER(sound_int_ack_w);

	INTERRUPT_GEN_MEMBER(video_int_gen);
	DECLARE_WRITE16_MEMBER(video_int_ack_w);

	// slapstic helpers
	void slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, int chipnum);
	void slapstic_update_bank(int bank);
	DECLARE_DIRECT_UPDATE_MEMBER(slapstic_setdirect);
	DECLARE_WRITE16_MEMBER(slapstic_w);
	DECLARE_READ16_MEMBER(slapstic_r);

	// sound helpers
	void set_volume_by_type(int volume, device_type type);
	void set_ym2151_volume(int volume);
	void set_ym2413_volume(int volume);
	void set_pokey_volume(int volume);
	void set_tms5220_volume(int volume);
	void set_oki6295_volume(int volume);

	// scanline timing
	void scanline_timer_reset(screen_device &screen, int frequency);
	void scanline_timer(emu_timer &timer, screen_device &screen, int scanline);

	// video helpers
	int get_hblank(screen_device &screen) const { return (screen.hpos() > (screen.width() * 9 / 10)); }
	void halt_until_hblank_0(device_t &device, screen_device &screen);

	// misc helpers
	void blend_gfx(int gfx0, int gfx1, int mask0, int mask1);

	// vector and early raster EAROM interface
	DECLARE_READ8_MEMBER( earom_r );
	DECLARE_WRITE8_MEMBER( earom_w );
	DECLARE_WRITE8_MEMBER( earom_control_w );

	// timer IDs
	enum
	{
		TID_SCANLINE_INTERRUPT,
		TID_SCANLINE_TIMER,
		TID_UNHALT_CPU,
		TID_ATARIGEN_LAST
	};

	// vector and early raster EAROM interface
	optional_device<er2055_device> m_earom;
	UINT8               m_earom_data;
	UINT8               m_earom_control;

	UINT8               m_scanline_int_state;
	UINT8               m_sound_int_state;
	UINT8               m_video_int_state;

	optional_shared_ptr<UINT16> m_xscroll;
	optional_shared_ptr<UINT16> m_yscroll;

	/* internal state */
	UINT8                   m_slapstic_num;
	UINT16 *                m_slapstic;
	UINT8                   m_slapstic_bank;
	dynamic_buffer          m_slapstic_bank0;
	offs_t                  m_slapstic_last_pc;
	offs_t                  m_slapstic_last_address;
	offs_t                  m_slapstic_base;
	offs_t                  m_slapstic_mirror;

	UINT32                  m_scanlines_per_callback;


	atarigen_screen_timer   m_screen_timer[2];
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;

	optional_device<atari_sound_comm_device> m_soundcomm;
	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_shared_ptr<UINT16> m_generic_paletteram_16;
	optional_device<atari_slapstic_device> m_slapstic_device;
};



/***************************************************************************
    GENERAL ATARI NOTES
**************************************************************************##

    Atari 68000 list:

    Driver      Pr? Up? VC? PF? P2? MO? AL? BM? PH?
    ----------  --- --- --- --- --- --- --- --- ---
    arcadecl.c       *               *       *
    atarig1.c        *       *      rle  *
    atarig42.c       *       *      rle  *
    atarigt.c                *      rle  *
    atarigx2.c               *      rle  *
    atarisy1.c   *   *       *       *   *              270->260
    atarisy2.c   *   *       *       *   *              150->120
    badlands.c       *       *       *                  250->260
    batman.c     *   *   *   *   *   *   *       *      200->160 ?
    blstroid.c       *       *       *                  240->230
    cyberbal.c       *       *       *   *              125->105 ?
    eprom.c          *       *       *   *              170->170
    gauntlet.c   *   *       *       *   *       *      220->250
    klax.c       *   *       *       *                  480->440 ?
    offtwall.c       *   *   *       *                  260->260
    rampart.c        *               *       *          280->280
    relief.c     *   *   *   *   *   *                  240->240
    shuuz.c          *   *   *       *                  410->290 fix!
    skullxbo.c       *       *       *   *              150->145
    thunderj.c       *   *   *   *   *   *       *      180->180
    toobin.c         *       *       *   *              140->115 fix!
    vindictr.c   *   *       *       *   *       *      200->210
    xybots.c     *   *       *       *   *              235->238
    ----------  --- --- --- --- --- --- --- --- ---

    Pr? - do we have verifiable proof on priorities?
    Up? - have we updated to use new MO's & tilemaps?
    VC? - does it use the video controller?
    PF? - does it have a playfield?
    P2? - does it have a dual playfield?
    MO? - does it have MO's?
    AL? - does it have an alpha layer?
    BM? - does it have a bitmap layer?
    PH? - does it use the palette hack?

***************************************************************************/


#endif
