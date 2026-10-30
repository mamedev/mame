// license:BSD-3-Clause
// copyright-holders:David Haywood,Peter Wilhelmsen

/*
IGS games based on M68000 + IGS023 for video.
PGM-like but with different sound hardware.

TODO:
* verify NT3580 peripherals and NT3570 synthesis against hardware
* complete inputs for xypmd
* complete panel inputs, hopper outputs and DIP switches for xypmda
*/


#include "emu.h"

#include "igs023_video.h"
//#include "igs025.h"

#include "cpu/m68000/m68000.h"
#include "machine/nt3580.h"
#include "machine/timer.h"
#include "sound/nt3570.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class igs_68k_023vid_state : public driver_device
{
public:
	igs_68k_023vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "subcpu"),
		m_screen(*this, "screen"),
		m_video(*this, "igs023"),
		m_mainram(*this, "sram"),
		m_in0(*this, "IN0"),
		m_dsw(*this, "DSW%u", 1U)
	{ }

	void xypmd(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	void main_program_base_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<nt3580_device> m_soundcpu;
	required_device<screen_device> m_screen;
	required_device<igs023_video_device> m_video;
	required_shared_ptr<uint16_t> m_mainram;

	required_ioport m_in0;
	required_ioport_array<2> m_dsw;

	u8 m_io_select = 0;

private:
	u16 sound_status_r() { return 0xff00 | m_soundcpu->host_status_r(); }
	u16 input_r();
	void io_data_w(u8 data);

	void screen_vblank(int state);

	void main_program_xypmd_map(address_map &map) ATTR_COLD;

	u8 m_input_sel = 0;
};

class xypmda_state : public igs_68k_023vid_state
{
public:
	xypmda_state(const machine_config &mconfig, device_type type, const char *tag) :
		igs_68k_023vid_state(mconfig, type, tag),
		m_joystick(*this, "IN%u", 1U)
	{ }

	void xypmda(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 input_xypmda_r();
	void io_data_xypmda_w(u8 data);
	void screen_vblank_xypmda(int state);
	void main_program_xypmda_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_joystick;
	u8 m_dsw_select = 0xff;
};


void igs_68k_023vid_state::machine_start()
{
	save_item(NAME(m_input_sel));
	save_item(NAME(m_io_select));
}

void xypmda_state::machine_start()
{
	igs_68k_023vid_state::machine_start();
	save_item(NAME(m_dsw_select));
}


void igs_68k_023vid_state::screen_vblank(int state)
{
	// Apart from IRQ1 and IRQ2 all other IRQs point to exceptions

	// rising edge
	if (state)
	{
		m_video->get_sprites();
		m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE); // only IRQ with useful code
	}
	else
	{
		// m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE); // does nothing
	}
}

void xypmda_state::screen_vblank_xypmda(int state)
{
	// Apart from IRQ2 all other IRQs point to exceptions

	// rising edge
	if (state)
	{
		m_video->get_sprites();
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
	}
	else
	{
		// no other possible IRQ vectors
	}
}


void igs_68k_023vid_state::io_data_w(u8 data)
{
	m_input_sel = data;
	if (m_io_select == 1)
		// V401 board: strobe on bit 0, /reset on bit 1.
		m_soundcpu->host_control_w(data << 1);
}

void xypmda_state::io_data_xypmda_w(u8 data)
{
	if (m_io_select == 1)
		// Earlier board: strobe on bit 1, /reset on bit 2.
		m_soundcpu->host_control_w(data);
	else if (m_io_select == 2)
		m_dsw_select = data;
}

u16 xypmda_state::input_xypmda_r()
{
	// The host selects DIP banks with output register 2, then reads register 0.
	// Coin/service inputs are read separately through register 3.
	if (m_io_select == 0)
	{
		u16 data = 0xffff;
		for (unsigned i = 0; i < 2; i++)
		{
			if (!BIT(m_dsw_select, i))
				data &= m_dsw[i]->read();
		}
		return data;
	}
	else if (m_io_select == 3)
	{
		return m_in0->read();
	}
	else
	{
		return 0xffff;
	}
}

u16 igs_68k_023vid_state::input_r() // TODO: not totally correct
{
	u16 ret = m_in0->read();

	if (!BIT(m_input_sel, 2))
		ret &= m_dsw[0]->read();
	if (!BIT(m_input_sel, 3))
		ret &= m_dsw[1]->read();

	return ret;
}


void igs_68k_023vid_state::main_program_base_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x800000, 0x81ffff).ram().share(m_mainram); // RAM test tests the full 0x20000, while in game it only seems to use 0x10000
	map(0x900000, 0x907fff).mirror(0x0f8000).m(m_video, FUNC(igs023_video_device::videoram_map));
	map(0xa00000, 0xa013ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xa01400, 0xa023ff).ram(); // palette RAM test fails otherwise
	map(0xb00000, 0xb0ffff).m(m_video, FUNC(igs023_video_device::videoregs_map));
}

void igs_68k_023vid_state::main_program_xypmd_map(address_map &map)
{
	main_program_base_map(map);

	map(0x080000, 0x08ffff).ram(); // why is this tested like it was mainram / spriteram? bug in code?
	map(0x080000, 0x080001).umask16(0x00ff).w(m_soundcpu, FUNC(nt3580_device::host_data_w));

	map(0x090000, 0x090001).r(FUNC(igs_68k_023vid_state::sound_status_r));
	map(0x090018, 0x090019).portr("IN1");

	// 025 or just I/O?
	map(0x0c0000, 0x0c0001).nopr();
	map(0x0c0000, 0x0c0001).umask16(0x00ff).lw8(NAME([this] (u8 data) { m_io_select = data; }));
	map(0x0c0002, 0x0c0003).r(FUNC(igs_68k_023vid_state::input_r));
	map(0x0c0002, 0x0c0003).umask16(0x00ff).w(FUNC(igs_68k_023vid_state::io_data_w));
}

void xypmda_state::main_program_xypmda_map(address_map &map)
{
	main_program_base_map(map);

	// I/O / IGS025 etc. is different here
	map(0x080100, 0x080101).lr16(NAME([this] () -> u16 { return 0xff00 | m_soundcpu->host_status_r(); }));
	// Joystick inputs are on the upper byte; each player's eight switches
	// correspond to the P1J/P2J entries in the game's TEST I/O table.
	map(0x080110, 0x080111).lr16(NAME([this] () -> u16 { return (m_joystick[1]->read() << 8) | 0x00ff; }));
	map(0x080120, 0x080121).lr16(NAME([this] () -> u16 { return (m_joystick[0]->read() << 8) | 0x00ff; }));
	map(0x080200, 0x080201).umask16(0x00ff).w(m_soundcpu, FUNC(nt3580_device::host_data_w));

	map(0x0a0000, 0x0a0001).portr("COINS");
	map(0x0a0002, 0x0a0003).lr16(NAME([] () -> u16 { return 0xffff; }));

	// definitely the same as 0x0c0000 on the parent
	map(0x0e0000, 0x0e0001).nopr();
	map(0x0e0000, 0x0e0001).umask16(0x00ff).lw8(NAME([this] (u8 data) { m_io_select = data; }));
	map(0x0e0002, 0x0e0003).r(FUNC(xypmda_state::input_xypmda_r));
	map(0x0e0002, 0x0e0003).umask16(0x00ff).w(FUNC(xypmda_state::io_data_xypmda_w));
}


static INPUT_PORTS_START( xypmd )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( xypmda )
	PORT_INCLUDE(xypmd)

	PORT_MODIFY("IN0")
	// TEST I/O identifies bit 4 as P2J_START and bit 5 as P1J_START.
	// Coin, key-in and key-out switches are on the separate 0x0a0000 port.
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("%p Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("%p Double Up")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("%p Cancel")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("%p Payout")

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2) PORT_NAME("%p Bet")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_PLAYER(2) PORT_NAME("%p Double Up")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_PLAYER(2) PORT_NAME("%p Cancel")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 )        PORT_PLAYER(2) PORT_NAME("%p Payout")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Hopper Switch")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Hopper Switch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P2 Key In")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Key Out")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(0x10, 0x10, "Players") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(   0x00, "1")
	PORT_DIPSETTING(   0x10, "2")
	PORT_DIPNAME(0x60, 0x40, DEF_STR(Controls)) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(   0x60, "Panel")
	PORT_DIPSETTING(   0x40, DEF_STR(Joystick))
INPUT_PORTS_END


void igs_68k_023vid_state::xypmd(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_68k_023vid_state::main_program_xypmd_map);

	NT3580(config, m_soundcpu, 8_MHz_XTAL);

	//IGS025(config, "igs025");

	// video hardware
	SCREEN(config, m_screen); // TODO: verify everything once emulation works
	m_screen->set_raw(50_MHz_XTAL/5, 640, 0, 448, 264, 0, 224); // copied from igs/pgm.cpp, correct?
	m_screen->set_screen_update("igs023", FUNC(igs023_video_device::screen_update));
	m_screen->screen_vblank().set(FUNC(igs_68k_023vid_state::screen_vblank));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x1400 / 2);

	IGS023_VIDEO(config, m_video);
	m_video->set_palette("palette");
	m_video->set_screen(m_screen);
	m_video->read_spriteram_callback().set([this](offs_t offset) { return m_mainram[offset]; });

	// sound hardware
	SPEAKER(config, "mono").front_center();

	nt3570_device &sound(NT3570(config, "nt3570", 8_MHz_XTAL));
	m_soundcpu->sound_read_callback().set(sound, FUNC(nt3570_device::read));
	m_soundcpu->sound_write_callback().set(sound, FUNC(nt3570_device::write));
	sound.set_device_rom_tag("samples");
	sound.irq_callback().set(m_soundcpu, FUNC(nt3580_device::sound_irq_w));
	sound.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void xypmda_state::xypmda(machine_config &config)
{
	xypmd(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &xypmda_state::main_program_xypmda_map);

	m_screen->screen_vblank().set(FUNC(xypmda_state::screen_vblank_xypmda));
}
/*
Xingyun Paomadi Super, IGS, 2003
Hardware Info By Guru
---------------------

IGS PCB NO-0255-1
  |----------------------------------------------|
  | IDC34    SOCKET.U11     |------|  TL082  VOL |
|-|                         |NOVATEK      OM8383S|
|1  TLP521(x23)        8MHz |NT3570F     TL082   |
|8           DATA.U13       |------|             |
|W                                               |
|A                          |------|      7805   |
|Y           24257          |NOVATEK             |
|-|                         |NT3580F             |
  |          24257 24257    |------|        T518B|
  | TEXT.U5  |---------|                         |
|-|          |  IGS023 |  PAL V-401CN.U26        |
|1  CG.U6    |         |  PAL     68000   24258  |
|0           |         |  PAL                    |
|W  CG.U7    |         | |-----|          24258  |
|A           |---------| |IGS025  20MHz      BATT|
|Y         DSW1     50MHz| S8  |             SW3 |
|-|ULN2004 DSW2          |-----|   TLP521(x17)   |
  | TLP521(x6)  |--|       JAMMA          |--|   |
  |-------------|  |----------------------|  |---|
Notes:
      68000 - Motorola MC68000FN20 CPU. Clock Input 20MHz
              Note: This is a 20MHz part without clock divider and runs at 20MHz.
      24257 - Winbond W24257AJ-8N 32kB x8-bit SRAM (VRAM)
      24258 - Winbond W24258S-55LL 32kB x8-bit SRAM (Work RAM)
     IGS023 - Custom IGS023 Graphics Chip (also used on IGS PGM)
     IGS025 - Custom IGS025 Chip with Sticker 'S8'
    V-401CN - 27C4096 EPROM (main program)
    TEXT.U5 - 27C160 EPROM
      CG.U6 - 27C4096 EPROM
      CG.U7 - 27C160 EPROM
       DATA - 27512 Mask ROM
 SOCKET.U11 - Empty Socket. ROM is likely to be missing as the socket is clean. PCB printed 'MUSIC1' and '27C080'.
              There is space for another unpopulated ROM below this one marked 'MUSIC2' and '27C080'.
      IDC34 - IDC34 34-Pin Flat Cable Connector for I/O and Control Expansion (same connector seen on PGM)
    NT3580F - Novatek NT3580F-AM019 QFP100 Sound CPU with 6502 Core. Clock Input 8.000MHz. DATA ROM connects to this chip.
    NT3570F - Novatek NT3570F QFP80 Sound Chip. Clock Input 8.000MHz. This chip connects to the opamps so this is the sound chip.
              ROM U11 connects to this chip. Both Novatek chips are dated 1999.
      T518B - Mitsumi T518B Reset Chip
       7805 - LM7805 5V Linear Voltage Regulator
     TLP521 - Toshiba TLP521 Opto-Coupler
       BATT - 3.6V Ni-Cad Battery (both W24258 RAMs are battery-backed)
        SW3 - Toggle Switch for NVRAM Clear and Reset
     DSW1/2 - 8-Position DIP Switch
    ULN2003 - ULN2003 7-Channel Darlington Transistor Array
      TL082 - Texas Instruments TL082 JFET Input Dual Operational Amplifier
    OM8383S - Philips OM8383S Audio Power Amplifier
*/

ROM_START( xypmd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-401cn.u26", 0x00000, 0x80000, CRC(84f4f46c) SHA1(c04d3aaf531caba6cdf8296570ce24964bd7a077) ) // version string at 0x360dc

	ROM_REGION( 0x10000, "subcpu", 0 ) // NT3580 external program, first 8 KiB mapped at E000
	ROM_LOAD( "data.u13", 0x00000, 0x10000, CRC(7c0d8c8f) SHA1(d36ae4749fd248c399741f2024f2f44cf22536b8) ) // 111xxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x200000, "igs023",  0 )
	ROM_LOAD( "text.u5", 0x000000, 0x200000, CRC(253b8517) SHA1(3f583410ab7083d5f45a5e23f73bddd18b000260) )

	ROM_REGION16_LE( 0x200000, "igs023:sprcol", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x200000, CRC(1c6764f2) SHA1(ed1efcab927bdc439247d422df5dedc72fce5682) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_LE( 0x80000, "igs023:sprmask", 0 )
	ROM_LOAD( "cg.u6", 0x00000, 0x80000, CRC(20ff0cb3) SHA1(4562996675fe62563f393817f40395c8bce37c5f) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "u11", 0x000000, 0x100000, NO_DUMP ) // probably removed from this PCB, possibly or even probably the same as xypmda
ROM_END

/*
Unknown IGS
Hardware Info By Guru
---------------------

IGS PCB NO-0198-2
Basically the same PCB as IGS PCB No-0255-1 with parts shuffled.
IGS did **MANY** board re-designs with no additional functionality.

Notes:
      68000 - Clock 20MHz
   2x 61256 - 32kB x8-bit SRAM (Main Work RAM)
   3x 61256 - 32kB x8-bit SRAM (VRAM)
     IGS023 - Custom IGS023 Graphics Chip (also used on IGS PGM)
     IGS025 - Custom IGS025 Chip with Sticker 'T2'
    CG/TEXT - vs 0255-1 PCB, this board has EPROMs replaced with SOP40 and SOP44 mask ROMs
*/

ROM_START( xypmda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "prg.u36", 0x00000, 0x80000, CRC(99d6c58c) SHA1(c8db8689c18ef05ad52ef0476033b62f778e6abf) ) // no version string

	ROM_REGION( 0x10000, "subcpu", 0 ) // NT3580 external program, first 8 KiB mapped at E000
	ROM_LOAD( "data.u33", 0x00000, 0x10000, CRC(5e3e3558) SHA1(ca9cdb4e8d124b4a7341ef6597c7ccdbbe124138) ) // 111xxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x200000, "igs023",  0 )
	ROM_LOAD( "igs_t1801.u30", 0x000000, 0x200000, CRC(253b8517) SHA1(3f583410ab7083d5f45a5e23f73bddd18b000260) )

	ROM_REGION16_LE( 0x100000, "igs023:sprcol", 0 )
	// Data is NOT identical to cg.u7 (size difference is irrelevant, cg.u7 just has blank data in 2nd half)
	// Unusual however, as the sprcol and sprmask ROMs are essentially a pair. Check if one is bad or if
	// some colours have been changed intentionally
	ROM_LOAD( "igs_a1802.u40", 0x000000, 0x100000, CRC(5bf791cc) SHA1(df23c8a25a26410ec4021948403bb4111810d7af) )

	ROM_REGION16_LE( 0x80000, "igs023:sprmask", 0 )
	ROM_LOAD( "igs_a1803.u39", 0x00000, 0x80000, CRC(20ff0cb3) SHA1(4562996675fe62563f393817f40395c8bce37c5f) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "igs_s1804_speech_v100.u32", 0x000000, 0x100000, CRC(d95220ee) SHA1(72259856bc2a12059ff481f7aab5ecc3118edd18) )
ROM_END

} // anonymous namespace



GAME( 2003, xypmd,  0,     xypmd,  xypmd,  igs_68k_023vid_state, empty_init, ROT0, "IGS", "Xingyun Paomadi Super (V401CN)", MACHINE_NOT_WORKING ) // sample ROM undumped
GAME( 1999, xypmda, xypmd, xypmda, xypmda, xypmda_state,         empty_init, ROT0, "IGS", "Xingyun Paomadi (unknown ver)",  MACHINE_NOT_WORKING )
