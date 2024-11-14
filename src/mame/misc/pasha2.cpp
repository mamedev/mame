// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************

Pasha Pasha 2
Dong Sung, 1998

3PLAY
|--------------------------------------------------|
|      DA1311    UM53   AD-65    DREAM  9.6MHz     |
|KA22065  TL062  UM51   AD-65          RESET_SW    |
|   VOL1   VOL2                  1MHz   TL7705     |
|                                                  |
|    DSW2(8)            20MHz    GM71C18163        |
|     93C46                                   UM2  |
|           PAL                                    |
|J                                                 |
|A                                                 |
|M                      E1-16XT             AT89C52|
|M                                                 |
|A                                  U102           |
|           6116                                   |
|           6116       U3           U101           |
|                                                  |
|                                                  |
|                                             12MHz|
|                    A42MX16                       |
|    DSW1(8)                                       |
| UCN5801                                          |
| UCN5801                                          |
|         16MHz                                    |
|--------------------------------------------------|
Notes:
      U3         - 27C040 EPROM (DIP32)
      UM2/UM51/53- 29F040 EPROM (PLCC32)
      U101/102   - Each location contains a small adapter board plugged into a DIP42 socket. Each
                   adapter board holds 2x Intel E28F016S5 TSOP40 16M FlashROMs. On the PCB under the ROMs
                   it's marked '32MASK'. However, the adapter boards are not standard. If you try to read
                   the ROMs while they are _ON-THE-ADAPTER_ as a 32M DIP42 EPROM (such as 27C322), the
                   FlashROMs are damaged and the PCB no longer works :(
                   Thus, the FlashROMs must be removed and read separately!
                   The small adapter boards with their respective FlashROMs are laid out like this........

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U102
                   |                              |
                   |------------------------------|

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U101
                   |                              |
                   |------------------------------|

      A42MX16    - Actel A42MX16 FPGA (QFP160)
      AT89C52    - Atmel AT89C52 Microcontroller w/8k internal FlashROM, clock 12MHz (DIP40)
      E1-16XT    - Hyperstone E1-16XT CPU, clock 20MHz
      DREAM      - ATMEL DREAM SAM9773 Single Chip Synthesizer/MIDI with Effects and Serial Interface, clock 9.6MHz (TQFP80)
      AD-65      - Oki compatible M6295 sound chip, clock 1MHz
      5493R45    - ISSI 5493R45-001 128k x8 SRAM (SOJ32)
      GM71C18163 - Hynix 1M x16 DRAM (SOJ42)
      VSync      - 60Hz
      HSync      - 15.15kHz

 driver by Pierpaolo Prazzoli

 TODO:
 - eeprom - is it used?
 - irq2 - sound related? reads the 2 unmapped input registers.
 - irq3 - it only writes a 0 into memory and changes a register
 - pasha2: simulate music (DREAM chip)
 - zdrum: with DRC it fatal errors with 'Unimplemented: generate_adds (c00013ae)' if let in attract for a while or at the end of the 'Flash memory check' in test mode)
          with interpreter the game hangs itself with 'Assertion failed: file flash.dat'
 - zdrum: proper inputs (coin seems to freeze the game - gets stuck in a loop at c0001386, can be forced out of it with do PC=c0001392 and the coin gets credited) -
          right arrow not found yet, second left drum switch flickers on and off continuously in i/o test)
 - zdrum: when coined up via debugger and then started, the game itself reports an assert with DRC, gets to the select character screen and then hangs with the interpreter
 - zdrum: settings aren't kept after reset, possibly needs eeprom hook up
 - zdrum: dump hard disc and emulate MP3 board

*********************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "pasha2.lh"


namespace {

class pasha2_state : public driver_device
{
public:
	pasha2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_wram(*this, "wram")
		, m_mainbank(*this, "mainbank")
		, m_lamps_r(*this, "lamp_p%u_r", 1U)
		, m_lamps_g(*this, "lamp_p%u_g", 1U)
		, m_lamps_b(*this, "lamp_p%u_b", 1U)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki%u", 1U)
		, m_palette(*this, "palette")
	{ }

	void pasha2(machine_config &config);
	void zdrum(machine_config &config);

	void init_pasha2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_wram;

	required_memory_bank m_mainbank;

	// output finders
	output_finder<3> m_lamps_r;
	output_finder<3> m_lamps_g;
	output_finder<3> m_lamps_b;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device_array<okim6295_device, 2> m_oki;
	required_device<palette_device> m_palette;

	// video-related
	u8 m_vbuffer = 0;
	bitmap_ind16 m_bg_bitmap[2];
	bitmap_ind16 m_fg_bitmap[2];

	// video functions
	void vbuffer_set_w(u16 data);
	void vbuffer_clear_w(u16 data);
	void bg_bitmap_w(offs_t offset, u8 data);
	void fg_bitmap_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// peripheral handlers
	template<int Chip> void oki_bank_w(offs_t offset, u16 data);
	void misc_w(offs_t offset, u16 data);
	void pasha2_lamps_w(u16 data);

	// speedup functions
	u16 pasha2_speedup_r(offs_t offset);

	// address maps
	void pasha2_io(address_map &map) ATTR_COLD;
	void pasha2_map(address_map &map) ATTR_COLD;
	void zdrum_audio_map(address_map &map) ATTR_COLD;
};


void pasha2_state::misc_w(offs_t offset, u16 data)
{
	if (offset)
	{
		if (data & 0x0800)
		{
			const u16 bank = data & 0xf000;

			switch (bank)
			{
				case 0x8000:
				case 0x9000:
				case 0xa000:
				case 0xb000:
				case 0xc000:
				case 0xd000:
					m_mainbank->set_entry((bank>>12) & 7); break;
			}
		}
	}
}

void pasha2_state::vbuffer_set_w(u16 data)
{
	m_vbuffer = 1;
}

void pasha2_state::vbuffer_clear_w(u16 data)
{
	m_vbuffer = 0;
}

void pasha2_state::bg_bitmap_w(offs_t offset, u8 data)
{
	m_bg_bitmap[m_vbuffer].pix(offset >> 9, offset & 0x1ff) = (data & 0xff) | 0x100;
}

void pasha2_state::fg_bitmap_w(offs_t offset, u8 data)
{
	// handle overlapping pixels without writing them
	if ((data & 0xff) == 0xff)
		return;

	m_fg_bitmap[m_vbuffer].pix(offset >> 9, offset & 0x1ff) = data & 0xff;
}

template<int Chip>
void pasha2_state::oki_bank_w(offs_t offset, u16 data)
{
	if (offset)
		m_oki[Chip]->set_rom_bank(data & 1);
}

void pasha2_state::pasha2_lamps_w(u16 data)
{
	for (int p = 0; p < 3; p++)
	{
		m_lamps_r[p] = BIT(data, (p << 2) | 0);
		m_lamps_g[p] = BIT(data, (p << 2) | 1);
		m_lamps_b[p] = BIT(data, (p << 2) | 2);
	}
}

void pasha2_state::pasha2_map(address_map &map)
{
	// MEM0 (DRAM)
	map(0x00000000, 0x001fffff).ram().share("wram");
	// MEM1 (VRAM and Video registers)
	map(0x40000000, 0x4001ffff).ram().w(FUNC(pasha2_state::bg_bitmap_w));
	map(0x40020000, 0x4003ffff).ram().w(FUNC(pasha2_state::fg_bitmap_w));
	map(0x40060000, 0x40060001).nopw();
	map(0x40064000, 0x40064001).nopw();
	map(0x40068000, 0x40068001).nopw();
	map(0x4006c000, 0x4006c001).nopw();
	map(0x40070000, 0x40070001).w(FUNC(pasha2_state::vbuffer_clear_w));
	map(0x40074000, 0x40074001).w(FUNC(pasha2_state::vbuffer_set_w));
	map(0x40078000, 0x40078001).nopw(); //once at startup -> to disable the eeprom?
	// MEM2 (Bankswitched ROM)
	map(0x80000000, 0x803fffff).bankr("mainbank");
	// MEM3 (Palette, Boot ROM)
	map(0xe0000000, 0xe00001ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette"); //tilemap? palette?
	map(0xe0000200, 0xe00003ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext"); // "
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0);
}

void pasha2_state::pasha2_io(address_map &map)
{
	map(0x08, 0x0b).nopr(); //sound status?
	map(0x18, 0x1b).nopr(); //sound status?
	map(0x20, 0x23).w(FUNC(pasha2_state::pasha2_lamps_w));
	map(0x40, 0x43).portr("COINS");
	map(0x60, 0x63).portr("DSW");
	map(0x80, 0x83).portr("INPUTS");
	map(0xa0, 0xa3).nopw(); //soundlatch?
	map(0xc0, 0xc3).w(FUNC(pasha2_state::misc_w));
	map(0xe3, 0xe3).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe7, 0xe7).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe8, 0xeb).w(FUNC(pasha2_state::oki_bank_w<0>));
	map(0xec, 0xef).w(FUNC(pasha2_state::oki_bank_w<1>));
}

void pasha2_state::zdrum_audio_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0xfff80000, 0xffffffff).rom().region("audiocpu", 0);
}

static INPUT_PORTS_START( pasha2 )
	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 2 physical dip-switches
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

static INPUT_PORTS_START( zdrum )
	PORT_START("COINS") // no apparent effects on gameplay or i/o test, but for otherwise noted
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) // coin, but game enters a loop when pressed
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service in i/o test, seems to have no effect during gameplay
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x800, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 2 physical dip-switches, but the settings seem to be done via service mode, so possibly unused?
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME( "Drum switch right 2" )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // TODO: what's up with this? should probably be //  PORT_NAME( "Drum switch left 3" ), but it flickers on and off regardless
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME( "Drum switch left 1" )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME( "Drum switch left 2" )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME( "Drum switch right 3" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME( "Drum switch right 1" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "Left arrow" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) // all the following but 0x800 don't seem to have any effect
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // isn't shown in the i/o test but it's used to move between items in the service mode
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void pasha2_state::video_start()
{
	// 2 512x256 8bpp bitmaps, Both are double buffered
	for (int i = 0; i < 2; i++)
	{
		m_bg_bitmap[i].allocate(512, 256);
		m_fg_bitmap[i].allocate(512, 256);

		m_bg_bitmap[i].fill(0x100); // Palette space 0x100-0x1ff
		m_fg_bitmap[i].fill(0);     // Palette space 0x000-0x0ff

		save_item(NAME(m_bg_bitmap[i]), i);
		save_item(NAME(m_fg_bitmap[i]), i);
	}
}

u32 pasha2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bg_bitmap[(m_vbuffer ^ 1)], 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_fg_bitmap[(m_vbuffer ^ 1)], 0, 0, 0, 0, cliprect, 0);

	return 0;
}

void pasha2_state::machine_start()
{
	m_lamps_r.resolve();
	m_lamps_g.resolve();
	m_lamps_b.resolve();

	m_mainbank->configure_entries(0, 6, memregion("bankeddata")->base(), 0x400000);
	m_mainbank->set_entry(0);

	save_item(NAME(m_vbuffer));
}

void pasha2_state::machine_reset()
{
	m_vbuffer = 0;
}

void pasha2_state::pasha2(machine_config &config)
{
	// basic machine hardware
	E116XT(config, m_maincpu, 20_MHz_XTAL*4);     // 4x internal multiplier
	m_maincpu->set_addrmap(AS_PROGRAM, &pasha2_state::pasha2_map);
	m_maincpu->set_addrmap(AS_IO, &pasha2_state::pasha2_io);
	m_maincpu->set_vblank_int("screen", FUNC(pasha2_state::irq0_line_hold));

	AT89C52(config, "audiocpu", 12_MHz_XTAL);     // clock from docs
	// TODO : ports are unimplemented; P0,P1,P2,P3 and Serial Port Used

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 383, 0, 239);
	screen.set_screen_update(FUNC(pasha2_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
	m_palette->set_membits(8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 1_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki[1], 1_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	//and ATMEL DREAM SAM9773
}

void pasha2_state::zdrum(machine_config &config)
{
	pasha2(config);

	m_maincpu->set_force_no_drc(true); // gets a bit further

	e116xt_device &audiocpu(E116XT(config.replace(), "audiocpu", 45_MHz_XTAL)); // type unknown, but it does look like Hyperstone code
	audiocpu.set_addrmap(AS_PROGRAM, &pasha2_state::zdrum_audio_map);

	// TODO: MP3 hw, also PCB should be stereo according to test mode
}

ROM_START( pasha2 )
	ROM_REGION16_BE( 0x80000, "maincpu", 0 ) // Hyperstone CPU Code
	ROM_LOAD( "pp2.u3",       0x00000, 0x80000, CRC(1c701273) SHA1(f465323a1d3f2fd752c51c178fafe4cc866e28d6) )

	ROM_REGION16_BE( 0x400000*6, "bankeddata", ROMREGION_ERASEFF ) // data roms
	ROM_LOAD16_BYTE( "pp2-u2.u101",  0x000000, 0x200000, CRC(85c4a2d0) SHA1(452b24b74bd0b65d2d6852486e2917f94e21ecc8) )
	ROM_LOAD16_BYTE( "pp2-u1.u101",  0x000001, 0x200000, CRC(96cbd04e) SHA1(a4e7dd61194584b3c4217674d78ab2fd96b7b2e0) )
	ROM_LOAD16_BYTE( "pp2-u2.u102",  0x400000, 0x200000, CRC(2097d88c) SHA1(7597578e6ddca00909feac35d9d7331f783b2bd6) )
	ROM_LOAD16_BYTE( "pp2-u1.u102",  0x400001, 0x200000, CRC(7a3492fb) SHA1(de72c4d10e17eaf2b7531f637b42cbb3d07819b5) )
	// empty space, but no empty sockets on the pcb

	ROM_REGION( 0x2000, "audiocpu", 0 ) // AT89C52
	ROM_LOAD( "89c52.bin",  0x0000, 0x2000, CRC(9ce43ce4) SHA1(8027a3549b38e9a2e7bb8f518a0defcaf9743371) ) // music play 1.0

	ROM_REGION( 0x80000, "sam9773", 0 ) // SAM9773 sound data
	ROM_LOAD( "pp2.um2",      0x00000, 0x80000, CRC(86814b37) SHA1(70f8a94410e362669570c39e00492c0d69de6b17) )

	ROM_REGION( 0x80000, "oki1", 0 ) // Oki Samples
	ROM_LOAD( "pp2.um51",     0x00000, 0x80000, CRC(3b1b1a30) SHA1(1ea1266d280a2b96ac4ef9fe8ee7b1a5f7861672) )

	ROM_REGION( 0x80000, "oki2", 0 ) // Oki Samples
	ROM_LOAD( "pp2.um53",     0x00000, 0x80000, CRC(8a29ad03) SHA1(3e9b0c86d8e3bb0b7691f68ad45431f6f9e8edbd) )
ROM_END

// PARA8B16 main PCB, very similar to the Pasha Pasha 2 PCB but with no AT89C52 and SAM9773
// PARA MP3 MAIN Ver. 1.00 2000.1 plug in PCB (Xilinx XC9536, Micronas MAS3507D F10, Micronas DAC3550A C2, 14.7456 XTAL)
// MUTALISK V3.02 PCB which plugs in the MP3 PCB (stickered IC 'Catos Creative Staffs' - probably an Hyperstone core given the ROM content -, 45.0000000 MHz XTAL, GM71V65163CT5 RAM, D27C040 ROM)
ROM_START( zdrum )
	ROM_REGION16_BE( 0x80000, "maincpu", 0 ) // Hyperstone CPU Code
	ROM_LOAD( "u3.bin", 0x00000, 0x80000, CRC(296c6ee8) SHA1(67814ac87828f578591ec2069ad96861fe29de6c) )

	ROM_REGION16_BE( 0x400000*6, "bankeddata", ROMREGION_ERASEFF ) // data roms
	ROM_LOAD16_WORD_SWAP( "u101.bin", 0x000000, 0x400000, CRC(e9bc892e) SHA1(f50cde278693a38cfad1185a89c41e2e2029c5ac) ) // Dumped as 27C322, but actually E28F916S5 * 2
	ROM_LOAD16_WORD_SWAP( "u102.bin", 0x400000, 0x400000, CRC(954e67a1) SHA1(cf1553fd70e27fab2d77df07ccd2866c198b5b35) ) // "

	ROM_REGION( 0x80000, "audiocpu", 0 ) // on the MUTALISK PCB
	ROM_LOAD( "mutalisk_u6.bin", 0x00000, 0x80000, CRC(7d044372) SHA1(41c63644ce048cec2079fb5ec8a2da9c66841c48) ) // 27C4000DC

	ROM_REGION( 0x80000, "oki1", 0 )
	ROM_LOAD( "um51.bin", 0x00000, 0x80000, CRC(b1a22ea2) SHA1(2814d212faf99504bd19602c4183fd8a4763c44c) ) // AM29F040

	ROM_REGION( 0x80000, "oki2", 0 )
	ROM_LOAD( "um53.bin", 0x00000, 0x80000, CRC(372022da) SHA1(809d103bdb365c9ff9e32b6a3032040b6a91ce88) ) // MBM29F040A

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "u507.bin", 0x000, 0x117, CRC(681ddf27) SHA1(0f9696918e512eb3840750c9386b4312a9e937ab) ) // PALCE16V8H

	DISK_REGION( "ide:0:hdd" ) // should contain the songs
	DISK_IMAGE( "zdrum", 0, NO_DUMP )
ROM_END

u16 pasha2_state::pasha2_speedup_r(offs_t offset)
{
	if(m_maincpu->pc() == 0x8302)
		m_maincpu->spin_until_interrupt();

	return m_wram[(0x95744 / 2) + offset];
}

void pasha2_state::init_pasha2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x95744, 0x95747, read16sm_delegate(*this, FUNC(pasha2_state::pasha2_speedup_r)));
}

} // anonymous namespace


GAMEL( 1998, pasha2, 0, pasha2, pasha2, pasha2_state, init_pasha2, ROT0, "Dong Sung",        "Pasha Pasha 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_pasha2 )
GAME ( 2000, zdrum,  0, zdrum,  zdrum,  pasha2_state, empty_init,  ROT0, "PARA Enterprises", "Zooty Drum",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // missing HD dump, MP3 board emulation. Inputs seem to be read differently.
