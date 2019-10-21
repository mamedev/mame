// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarigen.h

    General functions for Atari games.

***************************************************************************/

#ifndef MAME_MACHINE_ATARIGEN_H
#define MAME_MACHINE_ATARIGEN_H

#include "includes/slapstic.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ATARI_CLOCK_14MHz   XTAL(14'318'181)
#define ATARI_CLOCK_20MHz   XTAL(20'000'000)
#define ATARI_CLOCK_32MHz   XTAL(32'000'000)
#define ATARI_CLOCK_50MHz   XTAL(50'000'000)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define PORT_ATARI_COMM_SOUND_TO_MAIN_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, sound_to_main_ready)

#define PORT_ATARI_COMM_MAIN_TO_SOUND_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, main_to_sound_ready)


// ======================> atari_sound_comm_device

// device type definition
DECLARE_DEVICE_TYPE(ATARI_SOUND_COMM, atari_sound_comm_device)

class atari_sound_comm_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cputag)
		: atari_sound_comm_device(mconfig, tag, owner, (u32)0)
	{
		m_sound_cpu.set_tag(std::forward<T>(cputag));
	}

	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto int_callback() { return m_main_int_cb.bind(); }

	// getters
	DECLARE_READ_LINE_MEMBER(main_to_sound_ready) { return m_main_to_sound_ready ? ASSERT_LINE : CLEAR_LINE; }
	DECLARE_READ_LINE_MEMBER(sound_to_main_ready) { return m_sound_to_main_ready ? ASSERT_LINE : CLEAR_LINE; }

	// main cpu accessors (forward internally to the atari_sound_comm_device)
	void main_command_w(u8 data);
	u8 main_response_r();
	void sound_reset_w(u16 data = 0);

	// sound cpu accessors
	void sound_cpu_reset() { synchronize(TID_SOUND_RESET, 1); }
	void sound_response_w(u8 data);
	u8 sound_command_r();
	void sound_irq_ack_w(u8 data = 0);
	u8 sound_irq_ack_r();
	INTERRUPT_GEN_MEMBER(sound_irq_gen);
	void sound_irq();

	// additional helpers
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_gen);

protected:
	// sound I/O helpers
	void update_sound_irq();
	void delayed_sound_reset(int param);
	void delayed_sound_write(int data);
	void delayed_6502_write(int data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// timer IDs
	enum
	{
		TID_SOUND_RESET,
		TID_SOUND_WRITE,
		TID_6502_WRITE
	};

	// configuration state
	devcb_write_line   m_main_int_cb;

	// internal state
	required_device<m6502_device> m_sound_cpu;
	bool             m_main_to_sound_ready;
	bool             m_sound_to_main_ready;
	u8               m_main_to_sound_data;
	u8               m_sound_to_main_data;
	u8               m_timed_int;
	u8               m_ym2151_int;
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

protected:
	// users must call through to these
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// callbacks provided by the derived class
	virtual void update_interrupts() = 0;
	virtual void scanline_update(screen_device &screen, int scanline);

	// interrupt handling
	void scanline_int_set(screen_device &screen, int scanline);
	DECLARE_WRITE_LINE_MEMBER(scanline_int_write_line);
	void scanline_int_ack_w(u16 data = 0);

	DECLARE_WRITE_LINE_MEMBER(video_int_write_line);
	void video_int_ack_w(u16 data = 0);

	// slapstic helpers
	void slapstic_configure(cpu_device &device, offs_t base, offs_t mirror, u8 *mem);
	void slapstic_update_bank(int bank);
	DECLARE_WRITE16_MEMBER(slapstic_w);
	DECLARE_READ16_MEMBER(slapstic_r);

	// scanline timing
	void scanline_timer_reset(screen_device &screen, int frequency);
	void scanline_timer(emu_timer &timer, screen_device &screen, int scanline);

	// video helpers
	int get_hblank(screen_device &screen) const { return (screen.hpos() > (screen.width() * 9 / 10)); }
	void halt_until_hblank_0(device_t &device, screen_device &screen);

	// misc helpers
	void blend_gfx(int gfx0, int gfx1, int mask0, int mask1);

	// timer IDs
	enum
	{
		TID_SCANLINE_INTERRUPT,
		TID_SCANLINE_TIMER,
		TID_UNHALT_CPU,
		TID_ATARIGEN_LAST
	};

	u8               m_scanline_int_state;
	u8               m_video_int_state;

	optional_shared_ptr<u16> m_xscroll;
	optional_shared_ptr<u16> m_yscroll;

	/* internal state */
	u8               m_slapstic_num;
	u16 *            m_slapstic;
	u8               m_slapstic_bank;
	std::vector<u8>  m_slapstic_bank0;
	offs_t           m_slapstic_last_pc;
	offs_t           m_slapstic_last_address;
	offs_t           m_slapstic_base;
	offs_t           m_slapstic_mirror;

	u32              m_scanlines_per_callback;


	atarigen_screen_timer   m_screen_timer[2];
	required_device<cpu_device> m_maincpu;

	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<screen_device> m_screen;
	optional_device<atari_slapstic_device> m_slapstic_device;

private:
	static const atarigen_screen_timer *get_screen_timer(screen_device &screen);
};



/***************************************************************************
    GENERAL ATARI NOTES
**************************************************************************##

    Atari 68000 list:

    Driver        Pr? Up? VC? PF? P2? MO? AL? BM? PH?
    ----------    --- --- --- --- --- --- --- --- ---
    arcadecl.cpp       *               *       *
    atarig1.cpp        *       *      rle  *
    atarig42.cpp       *       *      rle  *
    atarigt.cpp                *      rle  *
    atarigx2.cpp               *      rle  *
    atarisy1.cpp   *   *       *       *   *              270->260
    atarisy2.cpp   *   *       *       *   *              150->120
    badlands.cpp       *       *       *                  250->260
    batman.cpp     *   *   *   *   *   *   *       *      200->160 ?
    blstroid.cpp       *       *       *                  240->230
    cyberbal.cpp       *       *       *   *              125->105 ?
    eprom.cpp          *       *       *   *              170->170
    gauntlet.cpp   *   *       *       *   *       *      220->250
    klax.cpp       *   *       *       *                  480->440 ?
    offtwall.cpp       *   *   *       *                  260->260
    rampart.cpp        *               *       *          280->280
    relief.cpp     *   *   *   *   *   *                  240->240
    shuuz.cpp          *   *   *       *                  410->290 fix!
    skullxbo.cpp       *       *       *   *              150->145
    thunderj.cpp       *   *   *   *   *   *       *      180->180
    toobin.cpp         *       *       *   *              140->115 fix!
    vindictr.cpp   *   *       *       *   *       *      200->210
    xybots.cpp     *   *       *       *   *              235->238
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


#endif // MAME_MACHINE_ATARIGEN_H
