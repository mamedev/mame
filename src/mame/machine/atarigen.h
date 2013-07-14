/***************************************************************************

    atarigen.h

    General functions for Atari games.

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

#ifndef __MACHINE_ATARIGEN__
#define __MACHINE_ATARIGEN__

#include "devcb2.h"
#include "machine/nvram.h"
#include "machine/er2055.h"
#include "cpu/m6502/m6502.h"
#include "sound/okim6295.h"


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
	devcb = &atari_sound_comm_device::set_main_int_cb(*device, DEVCB2_##_intcb); \


#define MCFG_ATARI_VIDEO_CONTROLLER_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, ATARI_VIDEO_CONTROLLER, 0) \



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> atari_sound_comm_device

class atari_sound_comm_device :  public device_t
{
public:
	// construction/destruction
	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	static void static_set_sound_cpu(device_t &device, const char *cputag);
	template<class _Object> static devcb2_base &set_main_int_cb(device_t &device, _Object object) { return downcast<atari_sound_comm_device &>(device).m_main_int_cb.set_callback(object); }
	
	// getters
	bool main_to_sound_ready() const { return m_main_to_sound_ready; }
	bool sound_to_main_ready() const { return m_sound_to_main_ready; }

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
	const char *		m_sound_cpu_tag;
	devcb2_write_line	m_main_int_cb;

	// internal state
	m6502_device *		m_sound_cpu;
	bool                m_main_to_sound_ready;
	bool                m_sound_to_main_ready;
	UINT8               m_main_to_sound_data;
	UINT8               m_sound_to_main_data;
	UINT8               m_timed_int;
	UINT8               m_ym2151_int;
};


// device type definition
extern const device_type ATARI_SOUND_COMM;



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

struct atarivc_state_desc
{
	UINT32 latch1;                          /* latch #1 value (-1 means disabled) */
	UINT32 latch2;                          /* latch #2 value (-1 means disabled) */
	UINT32 rowscroll_enable;                /* true if row-scrolling is enabled */
	UINT32 palette_bank;                    /* which palette bank is enabled */
	UINT32 pf0_xscroll;                     /* playfield 1 xscroll */
	UINT32 pf0_xscroll_raw;                 /* playfield 1 xscroll raw value */
	UINT32 pf0_yscroll;                     /* playfield 1 yscroll */
	UINT32 pf1_xscroll;                     /* playfield 2 xscroll */
	UINT32 pf1_xscroll_raw;                 /* playfield 2 xscroll raw value */
	UINT32 pf1_yscroll;                     /* playfield 2 yscroll */
	UINT32 mo_xscroll;                      /* sprite xscroll */
	UINT32 mo_yscroll;                      /* sprite xscroll */
};


struct atarigen_screen_timer
{
	screen_device *screen;
	emu_timer *         scanline_interrupt_timer;
	emu_timer *         scanline_timer;
	emu_timer *         atarivc_eof_update_timer;
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
	INTERRUPT_GEN_MEMBER(scanline_int_gen);
	DECLARE_WRITE16_MEMBER(scanline_int_ack_w);
	DECLARE_WRITE_LINE_MEMBER(sound_int_write_line);
	INTERRUPT_GEN_MEMBER(sound_int_gen);
	DECLARE_WRITE16_MEMBER(sound_int_ack_w);
	INTERRUPT_GEN_MEMBER(video_int_gen);
	DECLARE_WRITE16_MEMBER(video_int_ack_w);

	// EEPROM helpers
	WRITE16_MEMBER(eeprom_enable_w);
	WRITE16_MEMBER(eeprom_w);
	WRITE32_MEMBER(eeprom32_w);
	READ16_MEMBER(eeprom_r);
	READ32_MEMBER(eeprom_upper32_r);

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

	// video controller
	void atarivc_eof_update(emu_timer &timer, screen_device &screen);
	void atarivc_reset(screen_device &screen, UINT16 *eof_data, int playfields);
	void atarivc_w(screen_device &screen, offs_t offset, UINT16 data, UINT16 mem_mask);
	UINT16 atarivc_r(screen_device &screen, offs_t offset);
	inline void atarivc_update_pf_xscrolls()
	{
		m_atarivc_state.pf0_xscroll = m_atarivc_state.pf0_xscroll_raw + ((m_atarivc_state.pf1_xscroll_raw) & 7);
		m_atarivc_state.pf1_xscroll = m_atarivc_state.pf1_xscroll_raw + 4;
	}
	void atarivc_common_w(screen_device &screen, offs_t offset, UINT16 newword);

	// playfield/alpha tilemap helpers
	DECLARE_WRITE16_MEMBER( alpha_w );
	DECLARE_WRITE32_MEMBER( alpha32_w );
	DECLARE_WRITE16_MEMBER( alpha2_w );
	void set_playfield_latch(int data);
	void set_playfield2_latch(int data);
	DECLARE_WRITE16_MEMBER( playfield_w );
	DECLARE_WRITE32_MEMBER( playfield32_w );
	DECLARE_WRITE16_MEMBER( playfield_large_w );
	DECLARE_WRITE16_MEMBER( playfield_upper_w );
	DECLARE_WRITE16_MEMBER( playfield_dual_upper_w );
	DECLARE_WRITE16_MEMBER( playfield_latched_lsb_w );
	DECLARE_WRITE16_MEMBER( playfield_latched_msb_w );
	DECLARE_WRITE16_MEMBER( playfield2_w );
	DECLARE_WRITE16_MEMBER( playfield2_latched_msb_w );

	// video helpers
	int get_hblank(screen_device &screen) const { return (screen.hpos() > (screen.width() * 9 / 10)); }
	void halt_until_hblank_0(device_t &device, screen_device &screen);
	DECLARE_WRITE16_MEMBER( paletteram_666_w );
	DECLARE_WRITE16_MEMBER( expanded_paletteram_666_w );
	DECLARE_WRITE32_MEMBER( paletteram32_666_w );

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
		TID_SOUND_RESET,
		TID_SOUND_WRITE,
		TID_6502_WRITE,
		TID_SCANLINE_TIMER,
		TID_ATARIVC_EOF,
		TID_UNHALT_CPU,
		TID_ATARIGEN_LAST
	};

	// vector and early raster EAROM interface
	optional_device<er2055_device> m_earom;
	UINT8               m_earom_data;
	UINT8               m_earom_control;

	optional_shared_ptr<UINT16> m_eeprom;
	optional_shared_ptr<UINT32> m_eeprom32;

	UINT8               m_scanline_int_state;
	UINT8               m_sound_int_state;
	UINT8               m_video_int_state;

	const UINT16 *      m_eeprom_default;

	optional_shared_ptr<UINT16> m_playfield;
	optional_shared_ptr<UINT16> m_playfield2;
	optional_shared_ptr<UINT16> m_playfield_upper;
	optional_shared_ptr<UINT16> m_alpha;
	optional_shared_ptr<UINT16> m_alpha2;
	optional_shared_ptr<UINT16> m_xscroll;
	optional_shared_ptr<UINT16> m_yscroll;

	optional_shared_ptr<UINT32> m_playfield32;
	optional_shared_ptr<UINT32> m_alpha32;

	tilemap_t *             m_playfield_tilemap;
	tilemap_t *             m_playfield2_tilemap;
	tilemap_t *             m_alpha_tilemap;
	tilemap_t *             m_alpha2_tilemap;

	optional_shared_ptr<UINT16> m_atarivc_data;
	optional_shared_ptr<UINT16> m_atarivc_eof_data;
	atarivc_state_desc      m_atarivc_state;

	/* internal state */
	bool                    m_eeprom_unlocked;

	UINT8                   m_slapstic_num;
	UINT16 *                m_slapstic;
	UINT8                   m_slapstic_bank;
	dynamic_buffer          m_slapstic_bank0;
	offs_t                  m_slapstic_last_pc;
	offs_t                  m_slapstic_last_address;
	offs_t                  m_slapstic_base;
	offs_t                  m_slapstic_mirror;

	UINT32                  m_scanlines_per_callback;

	UINT32                  m_actual_vc_latch0;
	UINT32                  m_actual_vc_latch1;
	UINT8                   m_atarivc_playfields;

	UINT32                  m_playfield_latch;
	UINT32                  m_playfield2_latch;

	atarigen_screen_timer   m_screen_timer[2];
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	
	optional_device<atari_sound_comm_device> m_soundcomm;
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
