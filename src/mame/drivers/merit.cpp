// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

  Merit trivia games

  driver by Pierpaolo Prazzoli

  Games known to be needed:
  - Some other missing categories romsets and new / old revision

  TODO:
  - add flipscreen according to schematics
  - pitboss: dip switches
  - general - add named output notifiers

Notes: it's important that "user1" is 0xa0000 bytes with empty space filled
       with 0xff, because the built-in roms test checks how many question roms
       the games has and the type of each one.
       The  type is stored in one byte in an offset which change for every game,
       using it in a form of protection.

       Rom type byte legend:
       0 -> 0x02000 bytes rom
       1 -> 0x04000 bytes rom
       2 -> 0x08000 bytes rom
       3 -> 0x10000 bytes rom

       ---------------------------------------------------------------------------

       Trivia Whiz ? Horizontal and Vertical can accept 10 question roms, with
       4 categories, but they only use 3 categories.

       ---------------------------------------------------------------------------

       The Couples: Year is uncertain,title screen says "1988"
       PCB marking (set 2) and (set 3) says "230188"
       service mode says "6221-51 U5-0 12/02/86"

       ---------------------------------------------------------------------------

       Casino Five: Requires a Keyboard to be attached to switch the game from the
       "High Score" mode to the "Points Replay" mode by entering a custom code.
       The Points Replay mode displays additional information on the service mode
       screen such as Max Bet, Percentage & Points Played / Won. Is the High Score
       or Points Replay mode controlled via a location in NVRAM or in RAM?
       Casino Five has optional Custom Ads which require the CRT-254 Video
       Billboard card & Keyboard to set up.

       ---------------------------------------------------------------------------

       The Pit Boss (2214-04): Has "Custom Ads" display always enabled? How do you
       disable it, is it controlled (stored) via NVRAM?

       ---------------------------------------------------------------------------

Merit Riviera Notes - There are several known versions:
  Riviera Hi-Score
  Riviera Super Star (not dumped)
  Riviera Montana Version (with journal printer, not dumped)
  Riviera Tennessee Draw (not dumped)
  Michigan Superstar Draw Poker (not dumped)
  Americana

  There are several law suites over the Riviera games. Riviera Distributors Inc. bought earlier versions
  of the various video poker games from Merit. RDI then licensed the games to Michigan Coin Op-Vending
  Inc. The legal battles over true ownership started in 2004 and carried on through at least 09/01/2011.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "machine/nvram.h"

#define MASTER_CLOCK            (XTAL_10MHz)
#define CPU_CLOCK               (MASTER_CLOCK / 4)
#define PIXEL_CLOCK             (MASTER_CLOCK / 1)
#define CRTC_CLOCK              (MASTER_CLOCK / 8)

#define NUM_PENS                (16)
#define RAM_PALETTE_SIZE        (1024)


class merit_state : public driver_device
{
public:
	merit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram_attr(*this, "raattr"),
		m_ram_video(*this, "ravideo"),
		m_backup_ram(*this, "backup_ram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen") { }

	void dodge_nvram_init(nvram_device &nvram, void *base, size_t size);
	pen_t m_pens[NUM_PENS];
	required_shared_ptr<UINT8> m_ram_attr;
	required_shared_ptr<UINT8> m_ram_video;
	UINT8 *m_ram_palette;
	UINT8 m_lscnblk;
	int m_extra_video_bank_bit;
	int m_question_address;
	int m_decryption_key;
	optional_shared_ptr<UINT8> m_backup_ram;
	DECLARE_READ8_MEMBER(questions_r);
	DECLARE_WRITE8_MEMBER(low_offset_w);
	DECLARE_WRITE8_MEMBER(med_offset_w);
	DECLARE_WRITE8_MEMBER(high_offset_w);
	DECLARE_READ8_MEMBER(palette_r);
	DECLARE_WRITE8_MEMBER(palette_w);
	DECLARE_WRITE8_MEMBER(casino5_bank_w);
	DECLARE_CUSTOM_INPUT_MEMBER(rndbit_r);
	DECLARE_WRITE_LINE_MEMBER(hsync_changed);
	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	DECLARE_WRITE8_MEMBER(led1_w);
	DECLARE_WRITE8_MEMBER(led2_w);
	DECLARE_WRITE8_MEMBER(misc_w);
	DECLARE_WRITE8_MEMBER(misc_couple_w);
	DECLARE_DRIVER_INIT(couple);
	DECLARE_DRIVER_INIT(key_5);
	DECLARE_DRIVER_INIT(key_4);
	DECLARE_DRIVER_INIT(key_7);
	DECLARE_DRIVER_INIT(key_0);
	DECLARE_DRIVER_INIT(key_2);
	DECLARE_DRIVER_INIT(dtrvwz5);
	virtual void machine_start();
	DECLARE_MACHINE_START(casino5);
	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


void merit_state::machine_start()
{
	m_question_address = 0;
	m_ram_palette = auto_alloc_array(machine(), UINT8, RAM_PALETTE_SIZE);

	save_pointer(NAME(m_ram_palette), RAM_PALETTE_SIZE);
	save_item(NAME(m_lscnblk));
	save_item(NAME(m_extra_video_bank_bit));
	save_item(NAME(m_question_address));
	save_item(NAME(m_decryption_key));
}


READ8_MEMBER(merit_state::questions_r)
{
	UINT8 *questions = memregion("user1")->base();
	int address;

	switch(m_question_address >> 16)
	{
		case 0x30: address = 0x00000;
			break;

		case 0x31: address = 0x10000;
			break;

		case 0x32: address = 0x20000;
			break;

		case 0x33: address = 0x30000;
			break;

		case 0x34: address = 0x40000;
			break;

		case 0x35: address = 0x50000;
			break;

		case 0x36: address = 0x60000;
			break;

		case 0x37: address = 0x70000;
			break;

		case 0x28: address = 0x80000;
			break;

		case 0x18: address = 0x90000;
			break;

/* not used? only 10 roms are tested in service mode
 * b0 b1 b2 b3 b4 b5 b6 b7 a8 98
 *      case 0xb0: address = 0xa0000;
 *          break;
 */

		default: logerror("read unknown question rom: %02X\n",m_question_address >> 16);
			return 0xff;
	}

	address |= m_question_address & 0xffff;

	return questions[address];
}

WRITE8_MEMBER(merit_state::low_offset_w)
{
	offset = (offset & 0xf0) | ((offset - m_decryption_key) & 0x0f);
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	m_question_address = (m_question_address & 0xffff00) | offset;
}

WRITE8_MEMBER(merit_state::med_offset_w)
{
	offset = (offset & 0xf0) | ((offset - m_decryption_key) & 0x0f);
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	m_question_address = (m_question_address & 0xff00ff) | (offset << 8);
}

WRITE8_MEMBER(merit_state::high_offset_w)
{
	offset = BITSWAP8(offset,7,6,5,4,0,1,2,3);
	m_question_address = (m_question_address & 0x00ffff) | (offset << 16);
}

READ8_MEMBER(merit_state::palette_r)
{
	int co;

	co = ((m_ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	return m_ram_palette[co];
}

WRITE8_MEMBER(merit_state::palette_w)
{
	int co;

//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
	data &= 0x0f;

	co = ((m_ram_attr[offset] & 0x7F) << 3) | (offset & 0x07);
	m_ram_palette[co] = data;

}


MC6845_BEGIN_UPDATE( merit_state::crtc_begin_update )
{
	int dim, bit0, bit1, bit2;

	for (int i=0; i < NUM_PENS; i++)
	{
		dim = BIT(i,3) ? 255 : 127;
		bit0 = BIT(i,0);
		bit1 = BIT(i,1);
		bit2 = BIT(i,2);
		m_pens[i] = rgb_t(dim*bit0, dim*bit1, dim*bit2);
	}
}


MC6845_UPDATE_ROW( merit_state::crtc_update_row )
{
	UINT8 *gfx[2];
	UINT16 x = 0;
	int rlen;

	gfx[0] = memregion("gfx1")->base();
	gfx[1] = memregion("gfx2")->base();
	rlen = memregion("gfx2")->bytes();

	//ma = ma ^ 0x7ff;
	for (UINT8 cx = 0; cx < x_count; cx++)
	{
		int i;
		int attr = m_ram_attr[ma & 0x7ff];
		int region = (attr & 0x40) >> 6;
		int addr = ((m_ram_video[ma & 0x7ff] | ((attr & 0x80) << 1) | (m_extra_video_bank_bit)) << 4) | (ra & 0x0f);
		int colour = (attr & 0x7f) << 3;
		UINT8   *data;

		addr &= (rlen-1);
		data = gfx[region];

		for (i = 7; i>=0; i--)
		{
			int col = colour;

			col |= (BIT(data[0x0000 | addr],i)<<2);
			if (region==0)
			{
				col |= (BIT(data[rlen | addr],i)<<1);
				col |= (BIT(data[rlen<<1 | addr],i)<<0);
			}
			else
				col |= 0x03;

			col = m_ram_palette[col & 0x3ff];
			bitmap.pix32(y, x) = m_pens[col ? col & (NUM_PENS-1) : (m_lscnblk ? 8 : 0)];

			x++;
		}
		ma++;
	}
}


WRITE_LINE_MEMBER(merit_state::hsync_changed)
{
	/* update any video up to the current scanline */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());
}

WRITE_LINE_MEMBER(merit_state::vsync_changed)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(merit_state::led1_w)
{
	/* 5 button lamps player 1 */
	set_led_status(machine(), 0,~data & 0x01);
	set_led_status(machine(), 1,~data & 0x02);
	set_led_status(machine(), 2,~data & 0x04);
	set_led_status(machine(), 3,~data & 0x08);
	set_led_status(machine(), 4,~data & 0x10);
}

WRITE8_MEMBER(merit_state::led2_w)
{
	/* 5 button lamps player 2 */
	set_led_status(machine(), 5,~data & 0x01);
	set_led_status(machine(), 6,~data & 0x02);
	set_led_status(machine(), 7,~data & 0x04);
	set_led_status(machine(), 8,~data & 0x08);
	set_led_status(machine(), 9,~data & 0x10);

	/* coin counter */
	coin_counter_w(machine(),0,0x80-(data & 0x80));
}

WRITE8_MEMBER(merit_state::misc_w)
{
	flip_screen_set(~data & 0x10);
	m_extra_video_bank_bit = (data & 2) << 8;
	m_lscnblk = (data >> 3) & 1;

	/* other bits unknown */
}

WRITE8_MEMBER(merit_state::misc_couple_w)
{
	flip_screen_set(~data & 0x10);
	m_extra_video_bank_bit = (data & 2) << 8;
	m_lscnblk = (data >> 3) & 1;

	/* other bits unknown */

	/*kludge to avoid jumps on ram area in The Couples*/
	m_backup_ram[0x1011] = 0xc9; //ret
}

WRITE8_MEMBER(merit_state::casino5_bank_w)
{
	if ( data == 0 )
	{
		membank("bank1")->set_entry(1);
		membank("bank2")->set_entry(1);
	}
	else if ( data == 0xff )
	{
		membank("bank1")->set_entry(0);
		membank("bank2")->set_entry(0);
	}
	else
	{
		logerror( "Uknown banking write %02x\n", data );
	}
}

CUSTOM_INPUT_MEMBER(merit_state::rndbit_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( pitboss_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( casino5_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank2")
	AM_RANGE(0x6000, 0x6fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x7000, 0x7000) AM_WRITE(casino5_bank_w)
	AM_RANGE(0x7001, 0x7fff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigappg_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dodge_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

/* Address decoding is done by prom u13 on crt200a hardware. It decodes
 * the following addr lines: 2,3,9,13,14,15 ==> E20C
 * ==> mirror 1DF3 & ~effective_addr_lines
 * */

static ADDRESS_MAP_START( trvwhiz_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4c00, 0x4cff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0x5400, 0x54ff) AM_WRITE(low_offset_w)
	AM_RANGE(0x5800, 0x58ff) AM_WRITE(med_offset_w)
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc000, 0xc003) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( trvwhiz_io_map, AS_IO, 8, merit_state )
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x8100, 0x8100) AM_DEVWRITE("aysnd", ay8910_device, data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( phrcraze_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xc00c, 0xc00f) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( phrcraze_io_map, AS_IO, 8, merit_state )
	AM_RANGE(0xc004, 0xc004) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xc104, 0xc104) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_device, data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( tictac_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tictac_io_map, AS_IO, 8, merit_state )
	AM_RANGE(0xc00c, 0xc00c) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0xc10c, 0xc10c) AM_MIRROR(0x1cf3) AM_DEVWRITE("aysnd", ay8910_device, data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( trvwhziv_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dtrvwz5_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xb000, 0xb0ff) AM_ROM /* protection? code jumps here */
	AM_RANGE(0xc004, 0xc007) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_MIRROR(0x1df0) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xce00, 0xceff) AM_READWRITE(questions_r, high_offset_w)
	AM_RANGE(0xd600, 0xd6ff) AM_WRITE(low_offset_w)
	AM_RANGE(0xda00, 0xdaff) AM_WRITE(med_offset_w)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x05f0) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( couple_map, AS_PROGRAM, 8, merit_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("backup_ram")
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_SHARE("raattr")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("ravideo")
	AM_RANGE(0xf800, 0xfbff) AM_READWRITE(palette_r, palette_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( meritpoker )

	PORT_START("IN0") /* Pins #65 through #58 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME( "Hold 1 / Take / Lo" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME( "Hold 5 / Double Up / Hi" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME( "Cash Out / Hi-Score" )

	PORT_START("IN1") /* Pins #57 through #51 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )       /* AKA Diagnostics */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2") /* Pins #46 through #41 of J3 in decending order (usually P2 controls - Not used!) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* MUST be "LOW" or Riviera Hi-Score rev A will hang */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, merit_state,rndbit_r, NULL)

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( bigappg )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hold" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Jackpot" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Take Half Option" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Raise Option" )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unlimited Double Up" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Double Up" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x00, "50" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "99" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x08)
INPUT_PORTS_END

static INPUT_PORTS_START( riviera )
	PORT_INCLUDE( meritpoker )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Auto Hold" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Bonus Jackpot" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3") /* Flyer suggests this might be "10-IN-A-ROW" bonus */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Raise Option" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Points Per Coin" )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Double Up" )     PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0xc0, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x00, "50" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "99" ) PORT_CONDITION("DSW", 0x08, EQUALS, 0x08)
INPUT_PORTS_END

static INPUT_PORTS_START( rivierab )
	PORT_INCLUDE( riviera )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1") /* No Auto Hold feature for this set */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dodge ) /* Same as "meritpoker" but not verified correct. Will correct / verify when these clones work */
	PORT_INCLUDE( meritpoker )
INPUT_PORTS_END

static INPUT_PORTS_START( pitboss ) /* PCB pinout maps 12 lamp outputs - Where are they mapped? */

	PORT_START("IN0") /* Pins #65 through #58 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1/P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1/P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1/P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1/P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1/P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1/P2 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Q) PORT_NAME("P1/P2 Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // pulling this LOW causes "unathorized conversion" msg.

	PORT_START("IN1") /* Pins #57 through #51 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* AKA Diagnostics - Seems to reset the game */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )    PORT_COCKTAIL PORT_CODE(KEYCODE_E) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Hands Per Game" )    PORT_DIPLOCATION("Special:1,2") /* Pins #52 & #51?? Listed as "Switch Common Ground" */
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "5" ) /* Duplicate setting - Likely not used */

	PORT_START("IN2") /* Pins #46 through #41 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_CODE(KEYCODE_W) PORT_NAME("P2 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, merit_state,rndbit_r, NULL)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Max Double Up" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, "Once" )
	PORT_DIPSETTING(    0x00, "Twice" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Free Hands" )    PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, "100,000+ & 200,000+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mroundup ) // todo: Find were Player 2 "Play" is mapped, all "IPT_UNKNOWN" below checked and nothing seems to work
	PORT_START("IN0") /* Pins #65 through #58 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_Q) PORT_NAME("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset Points") /* Counts down player point if pressed instead of "Play" */

	PORT_START("IN1") /* Pins #57 through #51 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   /* AKA Diagnostics - Seems to reset the game */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )    PORT_COCKTAIL PORT_CODE(KEYCODE_E) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0x00, "85%" ) /* Duplicate */
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )


	PORT_START("IN2") /* Pins #46 through #41 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A) PORT_NAME("P2 Button 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S) PORT_NAME("P2 Button 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D) PORT_NAME("P2 Button 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F) PORT_NAME("P2 Button 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G) PORT_NAME("P2 Button 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // pulling this LOW causes "unathorized conversion" msg.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, merit_state,rndbit_r, NULL)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Foto Finish" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Black Jack" )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Points Per Coin" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1 Point" )
	PORT_DIPSETTING(    0x00, "5 Points" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Counter Top" )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossa )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, "Coin Lockout" )      PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, "2 Coins" )
	PORT_DIPSETTING(    0x20, "10 Coins" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossa1 )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x30, "Coin Lockout" )      PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x30, "2 Coins" )
	PORT_DIPSETTING(    0x20, "10 Coins" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:8") /* No Free Hand Bonus for this set */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pitbossb )
	PORT_INCLUDE( pitboss )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( casino5 )

	PORT_START("IN0") /* Pins #65 through #58 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Points")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Play")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* pulling this LOW makes the horse racing game to not work */

	PORT_START("IN1") /* Pins #57 through #51 of J3 in decending order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) /* Runs basic Diagnostics on roms */
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )    /* AKA Diagnostics - Shows simple Statistics */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )    /* 1 displays additional screens in attract mode - custom ads screen (requires optional Keyboard to set up) */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0xc0, "Percentage Out" )     /* Controls Percentage out, 75%, 80%, 85% & 90%  as per manual's "Tab Positions" - Need to verify values */
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPSETTING(    0x80, "80%" )
	PORT_DIPSETTING(    0xc0, "85%" )
	PORT_DIPSETTING(    0x40, "90%" )

	PORT_START("IN2") /* Pins #46 through #41 of J3 in decending order (usually P2 controls - Not used!) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* pulling this LOW causes "Unathorized conversion" */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, merit_state,rndbit_r, NULL)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable Draw Poker" )     PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Enable Black Jack" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Enable Dice Game" )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Enable Foto Finish" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Enable Acey Deucey" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Jokers in Deck" )      PORT_DIPLOCATION("SW1:6") /* Only used in "Points Replay" mode ? */
	PORT_DIPSETTING(    0x00, "Bet 4 Points" )
	PORT_DIPSETTING(    0x20, "Always" )
	PORT_DIPNAME( 0xc0, 0x00, "Maximum Bet" )       PORT_DIPLOCATION("SW1:7,8") /* Only used in "Points Replay" mode */
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0xc0, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END


/* While on the "Books" (press "0") screen:
 * Some games have a hidden test mode, press Service Key (F2) to access it Example: Phrase Craze
 *  For other games:
 *   keep service test button pressed to clear the coin counter.
 *   keep it pressed for 10 seconds to clear all the memory.
 *
 * When NOT on the Books screen, Service Key acts like a reset.
 */
static INPUT_PORTS_START( merittrivia )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* Allows Test / Service menu from the "Books" */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( phrcraze )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPNAME( 0x04, 0x04, "Topic \"8\"" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
	/*  PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

static INPUT_PORTS_START( phrcrazs )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, "XXX-Rated Sex Topic" )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Phraze" )      PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "800K" )
	PORT_DIPSETTING(    0x08, "1M" )
	PORT_DIPSETTING(    0x00, "1.5M" )
	PORT_DIPNAME( 0x20, 0x20, "Random Sex Category" )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
/* PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

static INPUT_PORTS_START( phrcraza )
	PORT_INCLUDE( phrcraze )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* Phraze Craze (6221-40, U5-0) will hang if pulled HIGH */
INPUT_PORTS_END

static INPUT_PORTS_START( tictac )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Lightning Round 1 Credit" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "Upright 1 Player" )
	PORT_DIPSETTING(    0x00, "Upright 2 Players" )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
/*  PORT_DIPSETTING(    0x40, "Upright 1 Player" ) */
INPUT_PORTS_END

static INPUT_PORTS_START( trivia )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "On 0 Points" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Continue" )
	PORT_DIPSETTING(    0x00, "Game Over" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( trvwhziv )
	PORT_INCLUDE( trivia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")   /* no coinage DSW */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dtrvwh5 )
	PORT_INCLUDE( merittrivia )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Answers Shown" )     PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Hi Scores Retained" )    PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Cocktail Type" )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Regular Cocktail" )
	PORT_DIPSETTING(    0x00, "Single Side Cocktail" )
INPUT_PORTS_END

static INPUT_PORTS_START( couple )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Number of Attempts" )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "99" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x02, 0x02, "Tries Per Coin" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
/*2 Coins for 2 Credits?I think this is an invalid setting,it doesn't even work correctly*/
/*  PORT_DIPSETTING(    0x00, DEF_STR( 2C_2C ) ) */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Clear RAM" )         PORT_DIPLOCATION("SW1:8") /* Service Mode shows this as "NOT USED" */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*Different DSWs*/
static INPUT_PORTS_START( couplep )
	PORT_INCLUDE( couple )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Bonus Play" )    PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "at 150.000" )
	PORT_DIPSETTING(    0x00, "at 200.000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void merit_state::dodge_nvram_init(nvram_device &nvram, void *base, size_t size)
{
	memset(base, 0x00, size);
	reinterpret_cast<UINT8 *>(base)[0x1040] = 0xc9; /* ret */
}

MACHINE_START_MEMBER(merit_state,casino5)
{
	merit_state::machine_start();
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x2000, 0x2000);
	membank("bank2")->configure_entries(0, 2, memregion("maincpu")->base() + 0x6000, 0x2000);
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

static MACHINE_CONFIG_START( pitboss, merit_state )
	MCFG_CPU_ADD("maincpu",Z80, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(pitboss_map)
	MCFG_CPU_IO_MAP(trvwhiz_io_map)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(merit_state, led1_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(merit_state, misc_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 512, 0, 512, 256, 0, 256)   /* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(merit_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(merit_state, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(merit_state, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(merit_state, vsync_changed))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, CRTC_CLOCK)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(merit_state, led2_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( casino5, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(casino5_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MACHINE_START_OVERRIDE(merit_state,casino5)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bigappg, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bigappg_map)
	MCFG_CPU_IO_MAP(tictac_io_map)

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dodge, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dodge_map)
	MCFG_CPU_IO_MAP(tictac_io_map)

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", merit_state, dodge_nvram_init)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tictac, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tictac_map)
	MCFG_CPU_IO_MAP(tictac_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( trvwhiz, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(trvwhiz_map)
	MCFG_CPU_IO_MAP(trvwhiz_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dtrvwz5, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dtrvwz5_map)
	MCFG_CPU_IO_MAP(tictac_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( phrcraze, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(phrcraze_map)
	MCFG_CPU_IO_MAP(phrcraze_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( trvwhziv, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(trvwhziv_map)
	MCFG_CPU_IO_MAP(tictac_io_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( couple, pitboss )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(couple_map)
	MCFG_CPU_IO_MAP(tictac_io_map)

	MCFG_DEVICE_REMOVE("ppi8255_1")
	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(merit_state, led1_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(merit_state, misc_couple_w))
MACHINE_CONFIG_END



ROM_START( pitboss ) /* Program roms on a CTR-202 daughter card - Internal designation: PBVBREV0 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-04_u5-0.u5",   0x0000, 0x2000, CRC(10b782e7) SHA1(158819898ad81506c47b76ffe2a949ee7208740f) ) /* Games included in this set are: */
	ROM_LOAD( "2214-04_u6-0.u6",   0x2000, 0x2000, CRC(c3fd6510) SHA1(8c89fd2cbcb6f12fa6427883700971f7c39f6ccf) ) /* Joker Poker, Blackjack, Foto Finish & The Dice Game */
	ROM_RELOAD( 0x4000, 0x2000 )
	ROM_LOAD( "2214-04_u7-0.u7",   0x6000, 0x4000, CRC(c5cf7060) SHA1(4a3209ad24ae649348b0e0470fc446d37b667975) ) /* 27128 eprom */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.rom.u40",  0x0000, 0x2000, CRC(db62c5ec) SHA1(a9967eb51436f342902fa3ce9c43d4d1ec5e0f3c) )
ROM_END

ROM_START( pitbossa ) /* Roms also found labeled simply as "PBHD" U5 through U7 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-03_u5-0c.u5", 0x0000, 0x2000, CRC(97f870bd) SHA1(b1b01abff0385e3b0585e49f78b93bcf56e434ef) ) /* Internal designation: M4A4REV0 */
	ROM_LOAD( "2214-03_u6-0.u6",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) /* Games included in this set are: */
	ROM_LOAD( "2214-03_u7-0.u7",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) /* Joker Poker, Blackjack, Foto Finish & The Dice Game */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) /* Cheltenham PA. 19012      */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitbossa1 ) /* Specific build for localized region with no Free Hand Bonus */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2214-03_u5-1c.u5", 0x0000, 0x2000, CRC(cf985f96) SHA1(d0e1c3887fe87b92c52410215b5eec600a793c50) ) /* Internal designation: M4A4REV0 */
	ROM_LOAD( "2214-03_u6-0.u6",  0x2000, 0x2000, CRC(086e699b) SHA1(a1d1eafaac9262f924f175961aa52c6d8e779bf0) ) /* Games included in this set are: */
	ROM_LOAD( "2214-03_u7-0.u7",  0x4000, 0x2000, CRC(023e8cb8) SHA1(cdb180a94d801137466c13ddfaf65918cb608c5a) ) /* Joker Poker, Blackjack, Foto Finish & The Dice Game */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) /* Cheltenham PA. 19012      */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( pitbossb ) /* Roms also found labeled simply as "PSB1" U5 through U7 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5-0c.u5", 0x0000, 0x2000, CRC(d8902656) SHA1(06da829201f6141a6b23afa0e277a3c7a122c26e) ) /* Internal designation: PSB1REV0 */
	ROM_LOAD( "u6-0.u6",  0x2000, 0x2000, CRC(bf903b01) SHA1(1f5f69cfd3eb105bd9bad071016931a79defa16b) ) /* Games included in this set are: */
	ROM_LOAD( "u7-0.u7",  0x4000, 0x2000, CRC(306351b9) SHA1(32cd243aa65571ee7fc72971b6a16beeb4ed9d85) ) /* Joker Poker, Blackjack, Super Slots & The Dice Game */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) ) /* Shows: */
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) ) /* (c) 1983 Merit industries */
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) ) /* Cheltenham PA. 19012      */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

/* Known to exist is Pit Boss version M4A2 (confirmed via manual) and likely a M4A3 as well (not confirmed, but M4A4 is dumped) */

ROM_START( pitbossc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m4a1.u5",   0x0000, 0x2000, CRC(f5284472) SHA1(9170b90d06caa382be29feb2f6e80993bba1e07e) ) /* Internal designation: M4A1REV0 */
	ROM_LOAD( "m4a1.u6",   0x2000, 0x2000, CRC(dd8df5fe) SHA1(dab8c1077058263729b2589dd9bf9989ad53be1c) ) /* Games included in this set are: */
	ROM_LOAD( "m4a1.u7",   0x4000, 0x2000, CRC(5fa5d436) SHA1(9f3fd81eae7f378268f3b4af8fd299ffb97d7fb6) ) /* Draw Poker, Blackjack, Acey Deucey & The Dice Game */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr2.u39",  0x0000, 0x2000, CRC(f9613e7b) SHA1(1e8cafe142a235d65b43c7e46a79ed4f6272b61c) ) /* Shows: */
	ROM_LOAD( "chr2.u38",  0x2000, 0x2000, CRC(7af28902) SHA1(04f685389958d581aaf2c86940d1b8b8cec05d7a) ) /* (c) 1983 Merit industries   Phila. PA. */
	ROM_LOAD( "chr2.u37",  0x4000, 0x2000, CRC(ea6f0c59) SHA1(f2c0ff99518c2cec3eb1b4042fa3754a702c0e34) ) /* All Rights Reserverd                   */

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr2.u40",  0x0000, 0x2000, CRC(40c94dce) SHA1(86611e3a1048b2a3fffcc0110811656a2d0fc4a5) )
ROM_END

ROM_START( casino5 ) /* Standard version, the rom set with 3315-02 U5-1 is the "Minnesota" version and is undumped */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3315-02_u5-0.u5", 0x0000, 0x2000, CRC(abe240d8) SHA1(296eb3251dd51147d6984a8c08c3be22e5ed8e86) ) /* Program roms on a CTR-202A daughter card */
	ROM_LOAD( "3315-02_u6-0.u6", 0x2000, 0x4000, CRC(4d9f0c57) SHA1(d19b4b4f42d329ea35907d17c15a55b954b07295) )
	ROM_LOAD( "3315-02_u7-0.u7", 0x6000, 0x4000, CRC(d3bc510d) SHA1(6222badabf629dd6334591867596f811883aed52) ) /* There is known to be a 3315-02 U7-0-A version (not dumped) */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "u40", 0x0000, 0x2000, CRC(b13a3fb1) SHA1(25760aa27c88b8be248a87df724bf8797d179e7a) )
ROM_END

ROM_START( mroundup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kru1cd_u5.u5",  0x0000, 0x2000, CRC(7bd90672) SHA1(5f763e807370df991cc2c4abaeb8184c42149a0e) ) /* Internal designation: RUM1HRV0 */
	ROM_LOAD( "kru1cd_u6.u6",  0x2000, 0x2000, CRC(fccffb0d) SHA1(57cbfb7e2bf1050cb2da8b4cc7cdd7e18356bcd2) ) /* Games included in this set are: */
	ROM_LOAD( "kru1cd_u7.u7",  0x4000, 0x2000, CRC(72131230) SHA1(e7c08b537848a7c6e6e987c6d0644a031bb238d4) ) /* Draw Poker, Blackjack, Foto Finish */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chr7.u39",   0x0000, 0x2000, CRC(6662f607) SHA1(6b423f8de011d196700839af0be37effbf87383f) )
	ROM_LOAD( "chr7.u38",   0x2000, 0x2000, CRC(a014b44f) SHA1(906d426b1de75f26030c19dcd599b6570909f510) )
	ROM_LOAD( "chr7.u37",   0x4000, 0x2000, CRC(cb12e139) SHA1(06fe91281faae5d0c0ae4b3cd8ad103bd3995c38) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "chr7.u40",   0x0000, 0x2000, CRC(52298162) SHA1(79aa6c4ab6bec6450d882615e64f61cfef934153) )
ROM_END

ROM_START( riviera ) /* PAL16L8ANC labeled DEC-003 at U13 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-4a.u5", 0x0000, 0x8000, CRC(0bc8cf26) SHA1(da52010be2d44a240160bb1a13288b35e8feade2) ) /* 08 U5-4A 111287 2131-84A, label shows (c) 1988 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )
ROM_END

ROM_START( rivieraa ) /* PAL16L8ANC labeled DEC-003 at U13 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-4.u5", 0x0000, 0x8000, CRC(ce0b00f2) SHA1(c467c2c08d0bbadf80d67f41e17127e08ce3b3ff) ) /* 08 U5-4 111786 2131-84, label shows (c) 1987 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )
ROM_END

ROM_START( rivierab ) /* PAL16L8ANC labeled DEC-003 at U13 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-08_u5-2d.u5", 0x0000, 0x8000, CRC(64c6892b) SHA1(d245d4a9933e3b21279542da0cb6ee641569ef6c) ) /* 08 U5-2D 022086 2131-82d, label shows (c) 1985 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "hisc_u39.u39", 0x00000, 0x2000, CRC(1814c2ea) SHA1(fecc5dc1c0a56cbc7b68ee6a52222de348d6cc79) )
	ROM_LOAD( "hisc_u38.u38", 0x02000, 0x2000, CRC(ef1d7a80) SHA1(539662bee187a300a6f1bcded954758c87171219) )
	ROM_LOAD( "hisc_u37.u37", 0x04000, 0x2000, CRC(f6e709f8) SHA1(02905be912d0aa794f82926462f854e8e67dc407) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hisc_u40.u40", 0x00000, 0x2000, CRC(6d2a1ca8) SHA1(96ef3e0914c2b213ed9c9082fa3e27d75d52a8ec) )
ROM_END

ROM_START( bigappg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-13_u5-0.u5", 0x0000, 0x8000, CRC(47bad6fd) SHA1(87f6c603b52e184f82179869d7b58453cbd34814) ) /* 2131-13 U5-0 111786 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "haip_u39.u39", 0x0000, 0x2000, CRC(506e20b4) SHA1(cd28d4f56f83ed3f7cfa44280975c0a6af474707) )
	ROM_LOAD( "haip_u38.u38", 0x2000, 0x2000, CRC(1924feeb) SHA1(c4656a04e2284a265554ea9774d0201c44864809) )
	ROM_LOAD( "haip_u37.u37", 0x4000, 0x2000, CRC(12a04867) SHA1(8e3a6050d854ccc906ae473a104a16d610e4f1da) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "haip_u40.u40", 0x0000, 0x2000, CRC(ac4983b8) SHA1(a552a15f813c331de67eaae2ed42cc037b26c5bd) )
ROM_END

ROM_START( dodgectya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-0d.u5", 0x0000, 0x8000, CRC(ef71b268) SHA1(c85f2c8e7e9cd89b4720699814d8fcfbecf4dc1b) ) /* 2131-82 U5-0D 884111 2131 820*/

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) /* These 3 roms: 1st & 2nd half identical - Verified correct */
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	/* No U40 char rom - Verified on 4 PCBs */

	ROM_REGION( 0x0800, "crt209", ROMREGION_ERASEFF )
	ROM_LOAD( "crt-209_2131-82", 0x00000, 0x0800, NO_DUMP ) /* 2816 EEPROM in Z80 epoxy CPU module */
ROM_END

ROM_START( dodgectyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-50.u5", 0x0000, 0x8000, CRC(eb82515d) SHA1(d2c15bd633472f50b621ba90598559e345246d01) ) /* 2131-82 U5-50 987130 2131 825 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) /* These 3 roms: 1st & 2nd half identical - Verified correct */
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	/* No U40 char rom - Verified on 4 PCBs */

	ROM_REGION( 0x0800, "crt209", ROMREGION_ERASEFF )
	ROM_LOAD( "crt-209_2131-82", 0x00000, 0x0800, NO_DUMP ) /* 2816 EEPROM in Z80 epoxy CPU module */
ROM_END

ROM_START( dodgectyc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2131-82_u5-0_gt.u5", 0x0000, 0x8000, CRC(3858cd50) SHA1(1b1e208076df964afd68d01aa8d5489d36a934a5) ) /* 2131-82 U5-0 GT 982050 2131 820 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dodg_u39.u39", 0x00000, 0x8000, CRC(3b3376a1) SHA1(6880cdc29686ff7328717c3833ff826c278b023e) ) /* These 3 roms: 1st & 2nd half identical - Verified correct */
	ROM_LOAD( "dodg_u38.u38", 0x08000, 0x8000, CRC(654d5b00) SHA1(9e16330b2dc8821fc20a39eb42176fda23085bfc) )
	ROM_LOAD( "dodg_u37.u37", 0x10000, 0x8000, CRC(bc9e63d4) SHA1(2320f5a0545f18e1e42a3a45fedce912c36fbe13) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_ERASEFF )
	/* No U40 char rom - Verified on 4 PCBs */

	ROM_REGION( 0x0800, "crt209", ROMREGION_ERASEFF )
	ROM_LOAD( "crt-209_2131-82", 0x00000, 0x0800, NO_DUMP ) /* 2816 EEPROM in Z80 epoxy CPU module */
ROM_END

ROM_START( trvwzh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "gen-001_01a",  0x28000, 0x8000, CRC(1d8d353f) SHA1(6bd0cc5c67da81a48737f32bc49cbf235648c4c6) )
	ROM_LOAD( "gen-001_02a",  0x3c000, 0x4000, CRC(2000e3c3) SHA1(21737fde3d1a1b22da4590476e4e52ee1bab026f) ) /* 27128 eprom, others are 27256 */
	ROM_LOAD( "spo-001_01a",  0x48000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x58000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x68000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )
ROM_END

ROM_START( trvwzha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40", 0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) )
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "gen-001_01a",  0x28000, 0x8000, CRC(1d8d353f) SHA1(6bd0cc5c67da81a48737f32bc49cbf235648c4c6) )
	ROM_LOAD( "gen-001_02a",  0x3c000, 0x4000, CRC(2000e3c3) SHA1(21737fde3d1a1b22da4590476e4e52ee1bab026f) ) /* 27128 eprom, others are 27256 */
	ROM_LOAD( "sex-001_01a",  0x48000, 0x8000, CRC(32519098) SHA1(d070e02bb10e04964893903599a69a8943f9ac8a) )
	ROM_LOAD( "sex-001_02a",  0x88000, 0x8000, CRC(0be4ef9a) SHA1(c80080f1c853e1043bf7e47bea322540a8ac9195) )
ROM_END

/* question board only - this contained a variety of roms from the 'trvwzh' and 'trvwzha' sets as well as 2 unique general knowledge ones */
ROM_START( trvwzhb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-00_u5.u5", 0x0000, 0x2000, CRC(731fd5b1) SHA1(1074780321029446da0e6765b9e036b06b067a48) )
	ROM_LOAD( "6221-00_u6.u6", 0x2000, 0x2000, CRC(af6886c0) SHA1(48005b921d7ce33ffc0ba160be82053a26382a9d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40.u40",   0x0000, 0x2000, CRC(cea7319f) SHA1(663cd18a4699dfd5ad1d3357094eff247e9b4a47) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01a",  0x08000, 0x8000, CRC(ff45d92b) SHA1(10356bc6a04b2c53ecaf76cb0cba3ec70b4ba612) )
	ROM_LOAD( "ent-001_02a",  0x18000, 0x8000, CRC(902e26f7) SHA1(f13b816bfc507fb429fb3f44531de346a82c780d) )
	ROM_LOAD( "merit2_6.1",   0x28000, 0x8000, CRC(8a4bcde3) SHA1(528ae9d3ff0b98201f89fd6b93a712cd7f0e9ab4) ) // alt General Trivia - Need correct rom label
	ROM_LOAD( "merit2_6.2",   0x38000, 0x8000, CRC(ded7e124) SHA1(7e6e04ae79dceba70d83ccfde4f9d0ccc0737c78) ) // alt General Trivia - Need correct rom label
	ROM_LOAD( "spo-001_01a",  0x48000, 0x8000, CRC(ae111429) SHA1(ff551d7ac7ad367338e908805aeb78c59a747919) )
	ROM_LOAD( "spo-001_02a",  0x58000, 0x8000, CRC(ee9263b3) SHA1(1644ab01f17e3af1e193e509d64dcbb243d3eb80) )
	ROM_LOAD( "spo-001_03a",  0x68000, 0x8000, CRC(64181d34) SHA1(f84e28fc589b86ca6a596815871ed26602bcc095) )
ROM_END

ROM_START( trvwzv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-02_u5.u5", 0x0000, 0x2000, CRC(fd3531ac) SHA1(d11573df65676e704b28cc2d99fb004b48a358a4) )
	ROM_LOAD( "6221-02_u6.u6", 0x2000, 0x2000, CRC(29e43d0e) SHA1(ad610748fe37436880648078f5d1a305cb147c5d) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40.u40", 0x0000, 0x2000, CRC(1f0ff6e0) SHA1(5a31afde34aeb6f851389d093bb426e5cfdedbf2) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-001_01",  0x08000, 0x8000, CRC(c68ce954) SHA1(bf70fe64f095d5cfcf5347d83651b78c6c8bf05f) )
	ROM_LOAD( "ent-001_02",  0x18000, 0x8000, CRC(aac4ff63) SHA1(d68c4408b4dad976e317a33f2a4eaee39d90dbed) )
	ROM_LOAD( "gen-001_01",  0x28000, 0x8000, CRC(5deb1900) SHA1(b7e9407c37481ef8953e8283d45949d951302e92) )
	ROM_LOAD( "gen-001_02",  0x3c000, 0x4000, CRC(d2b53b6a) SHA1(f75334e47885086e277682daf018818a02ce1026) ) /* 27128 eprom, others are 27256 */
	ROM_LOAD( "spo-001_01",  0x48000, 0x8000, CRC(7b56315d) SHA1(4c8c63b80176bfac9594958a7043627012baada3) )
	ROM_LOAD( "spo-001_02",  0x58000, 0x8000, CRC(148b63ee) SHA1(9f3b222d979f23b313f379cbc06cc00d88d08c56) )
	ROM_LOAD( "spo-001_03",  0x68000, 0x8000, CRC(a6af8e41) SHA1(64f672bfa5fb2c0575103614986e53e238c5984f) )
ROM_END

ROM_START( trvwz2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-101.01a",  0x08000, 0x8000, CRC(3825ac47) SHA1(d0da047c4d30a26f496b3663cfda77c229279be8) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "ent-101.02a",  0x18000, 0x8000, CRC(a0153407) SHA1(e669957a5d4775bfa2c16960a2a909a3505c078b) )
	ROM_LOAD( "ent-101.03a",  0x28000, 0x8000, CRC(755b16ab) SHA1(277ea4110479ecdb2c772299ea04f4918cf7f561) )
	ROM_LOAD( "gen-101.01a",  0x38000, 0x8000, CRC(74d14039) SHA1(54b85581d60fb535d37a051f375e687a933600ea) )
	ROM_LOAD( "gen-101.02a",  0x48000, 0x8000, CRC(b1b930d8) SHA1(57be3ee1c0adcb549088818dc7efda64508b5647) ) /* These question roms have been found with the "trvwz3ha"    */
	ROM_LOAD( "spo-101.01a",  0x58000, 0x8000, CRC(9dc4ba98) SHA1(4ce2bbbd7436a0ba8140879d5d8614bddbd5a8ec) )
	ROM_LOAD( "spo-101.02a",  0x68000, 0x8000, CRC(9c106ad9) SHA1(1d1a5c91152283e3937a2df17cd57b8fe04072b7) )
	ROM_LOAD( "spo-101.03a",  0x78000, 0x8000, CRC(3d69c3a3) SHA1(9f16d45660f3cb15e44e9fc0d940a7b2b12819e8) )
	ROM_LOAD( "sex-101.01a",  0x88000, 0x8000, CRC(301d65c2) SHA1(48d260077e9c9ed82f6dfa176b1103723dc9e19a) )
	ROM_LOAD( "sex-101.02a",  0x98000, 0x8000, CRC(2596091b) SHA1(7fbb9c2c3f74e12714513928c8cf3769bf29fc8b) )

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */
ROM_END

ROM_START( trvwz2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "ent-101.01a",  0x08000, 0x8000, CRC(3825ac47) SHA1(d0da047c4d30a26f496b3663cfda77c229279be8) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "ent-101.02a",  0x18000, 0x8000, CRC(a0153407) SHA1(e669957a5d4775bfa2c16960a2a909a3505c078b) )
	ROM_LOAD( "ent-101.03a",  0x28000, 0x8000, CRC(755b16ab) SHA1(277ea4110479ecdb2c772299ea04f4918cf7f561) )
	ROM_LOAD( "gen-101.01a",  0x38000, 0x8000, CRC(74d14039) SHA1(54b85581d60fb535d37a051f375e687a933600ea) )
	ROM_LOAD( "gen-101.02a",  0x48000, 0x8000, CRC(b1b930d8) SHA1(57be3ee1c0adcb549088818dc7efda64508b5647) ) /* These question roms have been found with the "trvwz3ha"    */
	ROM_LOAD( "spo-101.01a",  0x58000, 0x8000, CRC(9dc4ba98) SHA1(4ce2bbbd7436a0ba8140879d5d8614bddbd5a8ec) )
	ROM_LOAD( "spo-101.02a",  0x68000, 0x8000, CRC(9c106ad9) SHA1(1d1a5c91152283e3937a2df17cd57b8fe04072b7) )
	ROM_LOAD( "spo-101.03a",  0x78000, 0x8000, CRC(3d69c3a3) SHA1(9f16d45660f3cb15e44e9fc0d940a7b2b12819e8) )
	ROM_LOAD( "merit2_4.0",   0x88000, 0x8000, CRC(069b59a3) SHA1(e3d75edd3a9271df73bf51f409f066547025abbe) ) // alt Sex Trivia - Need correct rom label
	ROM_LOAD( "merit2_4.1",   0x98000, 0x8000, CRC(938d319f) SHA1(5b5841692666c31f2c09cb318f7e106942fffea7) ) // alt Sex Trivia - Need correct rom label
ROM_END

ROM_START( trvwz3h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5", 0x0000, 0x2000, CRC(ad4ab519) SHA1(80e99f4e5542115e34074b41bbc69906e01a408f) ) /* Unknown revsion, but should 6221-xx something (maybe 6221-03??) */
	ROM_LOAD( "u6", 0x2000, 0x2000, CRC(21a44014) SHA1(331f8b4fa3f837de070b68b959c818122aedc68a) ) /* Unknown revsion, but should 6221-xx something (maybe 6221-03??) */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-001_01", 0x88000, 0x8000, CRC(77a2a734) SHA1(7ba662d275b7914c9dcc9532116086e091e6cf88) )
	ROM_LOAD( "sex-001_02", 0x98000, 0x8000, CRC(b064876b) SHA1(588300fb6603f334de41a9685b1fcf8c642b5c16) )

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */

	ROM_REGION( 0x8000, "misc", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x01f3, CRC(686d2ad0) SHA1(7aad0a1ed09942528eceaf4d7a5e1fd7601aeac7) ) /* PAL10L8CN */
ROM_END

ROM_START( trvwz3ha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-04_u5-0c.u5", 0x0000, 0x2000, CRC(e0a07f06) SHA1(02cde0fc4a62d108ecd3e2f7704b9166c31707f2) ) /* Also found labeled as "6221-04 U5-0D" */
	ROM_LOAD( "6221-04_u6-0c.u6", 0x2000, 0x2000, CRC(223482d6) SHA1(4d9dbce7505b98ccd8e2b55f6f86a59b213d72a1) ) /* Also found labeled as "6221-04 U6-0D" */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "triv_1_u39.u39", 0x0000, 0x2000, CRC(f8a5f5fb) SHA1(a511e1a2b5e887ef00dc919e9e664ccec2d36cfa) )
	ROM_LOAD( "triv_1_u38.u38", 0x2000, 0x2000, CRC(27621e52) SHA1(a7e88d329e2e774fef9bd8c5cefb4d8f1cfcba4c) )
	ROM_LOAD( "triv_1_u37.u37", 0x4000, 0x2000, CRC(f739b5dc) SHA1(fbf469b7f4cab50e06ec2def9344e3b9801a275e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "triv_1_u40b.u40", 0x0000, 0x2000, CRC(e829473f) SHA1(ba754d9377d955b409970494e1a14dbe1d359ee5) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) ) /* This set verified as all found on the same question board */
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex-002_03", 0x88000, 0x8000, CRC(2f37dcb0) SHA1(e96eeabfa62c0a56c2f888cf1abdfdcb059572c6) ) /* Shows in game as SEX TRIVIA III */
	ROM_LOAD( "sex-002_04", 0x98000, 0x8000, CRC(20bf245e) SHA1(1286fd2eb51c6125a7560da3e2390ec51b64fb43) ) /* Shows in game as SEX TRIVIA III */

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */

	ROM_REGION( 0x200, "misc", 0 )
	ROM_LOAD( "dec002.u13", 0x00000, 0x01f3, CRC(686d2ad0) SHA1(7aad0a1ed09942528eceaf4d7a5e1fd7601aeac7) )
ROM_END

ROM_START( trvwz3v )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-05_u5-0e.u5", 0x0000, 0x2000, CRC(97b8d320) SHA1(573945531113d8aae9418ba1e9a2063052227029) )
	ROM_LOAD( "6221-05_u6-0e.u6", 0x2000, 0x2000, CRC(2e86288d) SHA1(62c7024d8dfebed9bb05ea91302efe5d18cb7d2a) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-002_01", 0x08000, 0x8000, CRC(974dca96) SHA1(eb4a745c84307a1bbb220659877f97c28cd515ac) )
	ROM_LOAD( "spo-002_02", 0x18000, 0x8000, CRC(e15ef8d0) SHA1(51c946311ffe507aa9031044bc34e5ae8d3473ab) )
	ROM_LOAD( "spo-002_03", 0x28000, 0x8000, CRC(503115a1) SHA1(5e6630191465b3d2a590fab08b4f47f7408ecc44) )
	ROM_LOAD( "ent-002_01", 0x38000, 0x8000, CRC(0e4fe73d) SHA1(9aee22a5837637ec5e360b72e71555942df1d26f) )
	ROM_LOAD( "ent-002_02", 0x48000, 0x8000, CRC(f56c0935) SHA1(8e16133ad90829bbc0e0f2e9ee9c26e9d0c5057e) )
	ROM_LOAD( "ent-002_03", 0x58000, 0x8000, CRC(057f6676) SHA1(a93a7a76fc8b8263568a50b00a57f3abe76c9aa3) )
	ROM_LOAD( "gen-002_01", 0x68000, 0x8000, CRC(1fa46b86) SHA1(16d54d0932fe342399faf303eafa3c0b7ba2e202) )
	ROM_LOAD( "gen-002_02", 0x78000, 0x8000, CRC(b395cd97) SHA1(a42c7c1687eaba64a725888cd6413568cc90b010) )
	ROM_LOAD( "sex_triv_a", 0x88000, 0x8000, CRC(15d16703) SHA1(9184f63669e9ec93e88276777e1b7f209543c3e3) ) /* Actual label unknown, maybe sex-002_01? */
	ROM_LOAD( "sex_triv_b", 0x98000, 0x8000, CRC(647f3394) SHA1(636647ae620fd2f985b82e3516451e3bffd44040) ) /* Actual label unknown, maybe sex-002_02? */

	ROM_REGION( 0x0100, "prom", 0 )
	ROM_LOAD( "sc-002", 0x00000, 0x0100, CRC(94a8da8a) SHA1(8bdaee436481418425c36de24477c96ec0787916) ) /* N82S129N BPROM found on question board, unknown use */
ROM_END

ROM_START( trvwz4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-13_u5-0b.u5", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) ) /* 6221-13 U5-0B 03/17/86 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "tw4-05_ent-1", 0x08000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x18000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_rnp-1", 0x28000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x38000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_sex-1", 0x68000, 0x8000, CRC(976352b0) SHA1(5f89caca410704ba8a90da3167ba18e45fb21d43) )
	ROM_LOAD( "tw4-05_sex-2", 0x78000, 0x8000, CRC(5f148bc9) SHA1(2fd2cf819c2f395dcffad59857b3533fe3cce60b) )
	ROM_LOAD( "tw4-05_spo-1", 0x88000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x98000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
ROM_END

/* only the question board was dumped, but it contained a selection
  of roms from the above 'trvwz4' set, and one additional one which is Sex Trivia III
*/
ROM_START( trvwz4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-13_u5-0b.u5", 0x0000, 0x8000, CRC(bc23a1ab) SHA1(b9601f316e373c568c5b208de417617094046559) ) /* 6221-13 U5-0B 03/17/86 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trvs_u39.u39", 0x0000, 0x2000, CRC(b9d9a80e) SHA1(55b6a0d09f8619df93ba936e083835c859a557df) )
	ROM_LOAD( "trvs_u38.u38", 0x2000, 0x2000, CRC(8348083e) SHA1(260a4c1ae043e7ceac65a8818c23940d32275879) )
	ROM_LOAD( "trvs_u37.u37", 0x4000, 0x2000, CRC(b4d3c9f4) SHA1(dda99549306519c147d275d8c6af672e80a96b67) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trvs_u40a.u40", 0x0000, 0x2000, CRC(fbfae092) SHA1(b8569819952a5c805f11b6854d64b3ae9c857f97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "tw4-05_ent-1", 0x08000, 0x8000, CRC(1b317149) SHA1(94e882e9cc041ac8f292136c1ce2d21340ac5e7f) )
	ROM_LOAD( "tw4-05_ent-2", 0x18000, 0x8000, CRC(43d51697) SHA1(7af3f16f9519184ae63d8818bbc52a2ba897f275) )
	ROM_LOAD( "tw4-05_rnp-1", 0x28000, 0x8000, CRC(fee2d0b0) SHA1(9c9abec4ce693fc2d3976f3d499213c2ce67c197) )
	ROM_LOAD( "tw4-05_rnp-2", 0x38000, 0x8000, CRC(e54fc4bc) SHA1(4607974ed2bf83c475396fc1cbb1e09ad084ace8) )
	ROM_LOAD( "tw4-05_sbt-1", 0x48000, 0x8000, CRC(f1560804) SHA1(2ef0d587fbedfc342a12e913fa3c94eb8d67e2c5) )
	ROM_LOAD( "tw4-05_sbt-2", 0x58000, 0x8000, CRC(b0d6f6b2) SHA1(b08622d3775d1bb40c3b07ef932f3db4166ee284) )
	ROM_LOAD( "tw4-05_spo-1", 0x78000, 0x8000, CRC(5fe0c6a3) SHA1(17bdb5262ce4edf5f022f075537f6161e1397b46) )
	ROM_LOAD( "tw4-05_spo-2", 0x88000, 0x8000, CRC(3f3390e0) SHA1(50bd7b79268438584bb0f497ab0055b4d4864590) )
	ROM_LOAD( "merit2_5.0",   0x98000, 0x8000, CRC(e07d139f) SHA1(e364dcc628719c1bcdc119bdb2f3c98b5538c411) ) // sex trivia III - Need correct rom label
ROM_END

ROM_START( dtrvwz5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-70_u5-0a.u5", 0x0000, 0x8000, CRC(e5917a71) SHA1(2acebe337600cd490da1c6fb2d83e2e378e584f1) ) /* 6221-70 U5-0A 04/15/87 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "trv3.u39", 0x0000, 0x2000, CRC(81a34357) SHA1(87ae9db78f043dbdcd1d50473fc09284eceaf884) )
	ROM_LOAD( "trv3.u38", 0x2000, 0x2000, CRC(c4082020) SHA1(744dc8745f3d54754184571b64664ee5c1497fb4) )
	ROM_LOAD( "trv3.u37", 0x4000, 0x2000, CRC(5e5e6fb3) SHA1(c182233367de6c9cda0e49a5958bb07460a5f300) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trv3.u40", 0x0000, 0x2000, CRC(a2c934f2) SHA1(214cc1f47c11618457a7885712585c977107cab7) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "tw5-06_fif-1.1",  0x08000, 0x8000, CRC(300f245c) SHA1(9c380000ba5a6c826025e32f0e46932e234b46bc) )
	ROM_LOAD( "tw5-06_fif-2.2",  0x18000, 0x8000, CRC(99ee9cbe) SHA1(d6a4a604a070436b0acb1c774687f6c2266c8807) )
	ROM_LOAD( "tw5-06_six-1.3",  0x28000, 0x8000, CRC(87354939) SHA1(6e3de6df944da75e28d36dce3cca9b45a8936bf4) )
	ROM_LOAD( "tw5-06_six-2.4",  0x38000, 0x8000, CRC(ea8ed7ae) SHA1(2c084a88773e6f611a6cc6d847b9d74f5c8bfc77) )
	ROM_LOAD( "tw5-06_sev-1.5",  0x48000, 0x8000, CRC(fd5099aa) SHA1(81e978597aa348c77001f72763744491cfdad1d1) )
	ROM_LOAD( "tw5-06_sev-2.6",  0x58000, 0x8000, CRC(523520c8) SHA1(7dff9cda1ade5d3b4e573e77b7ec93ee8ae13c86) )
	ROM_LOAD( "tw5-06_eig-1.7",  0x68000, 0x8000, CRC(3a2a4562) SHA1(45565622d7057047b02050dcd34ff6f02663507d) )
	ROM_LOAD( "tw5-06_eig-2.8",  0x78000, 0x8000, CRC(cb7e9035) SHA1(d3344fb318f2241c07933c4b8e3525c219ea3aa6) )
	ROM_LOAD( "tw5-06_sx5-1.9",  0x88000, 0x8000, CRC(6ae2a208) SHA1(3cc935e616c247c6885319acc6a6ca92ee6fc3c0) )
	ROM_LOAD( "tw5-06_sx5-2.10", 0x98000, 0x8000, CRC(790184fc) SHA1(9c8b56852b31d3312f26a5901487f6b31d9e9b4f) )
ROM_END

/*

crt200 rev e-1
1985 merit industries

u39   u37                      z80b
u40   u38
           6845                 u5

      2016                      pb
      2016                      6264

                        ay3-8912
                        8255
                        8255


crt205a

pba pb9 pb8 pb7 pb6 pb5 pb4 pb3 pb2 pb1

*/

ROM_START( tictac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-23_u5-0c.u5", 0x00000, 0x8000, CRC(f0dd73f5) SHA1(f2988b84255ce5f7ea6d25150cdbae88b98e1be3) ) /* 6221-23 U5-0C 02/11/86 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "merit.u39", 0x00000, 0x2000, CRC(dd79e824) SHA1(d65ee1c758293ddf8a5f4913878a2867ba526e68) )
	ROM_LOAD( "merit.u38", 0x02000, 0x2000, CRC(e1bf0fab) SHA1(291261ea817c42d6e8a19c17a2d3706fed7d78c4) )
	ROM_LOAD( "merit.u37", 0x04000, 0x2000, CRC(94f9c7f8) SHA1(494389983fb62fe2d772c276e659b6b20c531933) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "merit.u40",    0x00000, 0x2000, CRC(ab0088eb) SHA1(23a05a4dc11a8497f4fc7e4a76085af15ff89cea) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-004_01a.1", 0x08000, 0x8000, CRC(71b398a9) SHA1(5ea07c409afd52c7d08592b30ff0ff3b72c3f8c3) ) /* Trivia categories are: */
	ROM_LOAD( "spo-004_02a.2", 0x18000, 0x8000, CRC(eb34672f) SHA1(c472fc4445fc434029a2740dfc1d9ab9b1ef9f87) ) /* Sports, Entertainment, General Interest & Sex Trivia III */
	ROM_LOAD( "spo-004_03a.3", 0x28000, 0x8000, CRC(8eea30b9) SHA1(fe1d0332106631f56bc6c57a888da9e4e63fa52f) )
	ROM_LOAD( "ent-004_01.4",  0x38000, 0x8000, CRC(3f45064d) SHA1(de109ac0b19fd1cd7f0020cc174c2da21708108c) )
	ROM_LOAD( "ent-004_02a.5", 0x48000, 0x8000, CRC(f1c446cd) SHA1(9a6f18defbb64e202ae12e1a59502b8f2d6a58a6) )
	ROM_LOAD( "ent-004_03.6",  0x58000, 0x8000, CRC(206cfc0d) SHA1(78f6b684713459a617096aa3ffe6e9e62583938c) )
	ROM_LOAD( "gen-004_01a.7", 0x68000, 0x8000, CRC(d1584173) SHA1(7a2190203f478f446cc70c473c345e7cc332e049) )
	ROM_LOAD( "gen-004_02a.8", 0x78000, 0x8000, CRC(d00ab1fd) SHA1(c94269c8a478e88f71aeca94c6f20fc05a9c62bd) )
	ROM_LOAD( "sex-004_01a.9", 0x88000, 0x8000, CRC(9333dbca) SHA1(dd87e6f69d60580fdb6f979398edbeb1a51be355) )
	ROM_LOAD( "sex-004_02a.a", 0x98000, 0x8000, CRC(6eda81f4) SHA1(6d64344691e3e52035a7d30fb3e762f0bd397db7) )
ROM_END

ROM_START( tictacv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-22_u5-0.u5", 0x00000, 0x8000,  CRC(c3acd686) SHA1(0c652e88675e2098be2f26e8f1acefc9e69d630f) ) /* 6221-22 U5-0 12/11/85 */

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ttts_u-39.u39", 0x00000, 0x2000, CRC(20103ed6) SHA1(52741ba8e3b57a32446d3bf4d6f6d8368954fd54) )
	ROM_LOAD( "ttts_u-38.u38", 0x02000, 0x2000, CRC(32e791b9) SHA1(f206aef25b3a9b7042d804e628c356f7d8d3cdbe) )
	ROM_LOAD( "ttts_u-37.u37", 0x04000, 0x2000, CRC(adf19f83) SHA1(af2c0b9782f8e93a7c5e2a5ecc937694773d8ad0) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "ttts_u-40.u40", 0x00000, 0x2000, CRC(c7071c98) SHA1(88e1b26f198cfbbd86b492356f60fc1b81b38d97) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "spo-004_01a.1", 0x08000, 0x8000, CRC(71b398a9) SHA1(5ea07c409afd52c7d08592b30ff0ff3b72c3f8c3) ) /* Trivia categories are: */
	ROM_LOAD( "spo-004_02a.2", 0x18000, 0x8000, CRC(eb34672f) SHA1(c472fc4445fc434029a2740dfc1d9ab9b1ef9f87) ) /* Sports, Entertainment, General Interest & Sex Trivia III */
	ROM_LOAD( "spo-004_03a.3", 0x28000, 0x8000, CRC(8eea30b9) SHA1(fe1d0332106631f56bc6c57a888da9e4e63fa52f) )
	ROM_LOAD( "ent-004_01.4",  0x38000, 0x8000, CRC(3f45064d) SHA1(de109ac0b19fd1cd7f0020cc174c2da21708108c) )
	ROM_LOAD( "ent-004_02a.5", 0x48000, 0x8000, CRC(f1c446cd) SHA1(9a6f18defbb64e202ae12e1a59502b8f2d6a58a6) )
	ROM_LOAD( "ent-004_03.6",  0x58000, 0x8000, CRC(206cfc0d) SHA1(78f6b684713459a617096aa3ffe6e9e62583938c) )
	ROM_LOAD( "gen-004_01a.7", 0x68000, 0x8000, CRC(d1584173) SHA1(7a2190203f478f446cc70c473c345e7cc332e049) )
	ROM_LOAD( "gen-004_02a.8", 0x78000, 0x8000, CRC(d00ab1fd) SHA1(c94269c8a478e88f71aeca94c6f20fc05a9c62bd) )
	ROM_LOAD( "sex-004_01a.9", 0x88000, 0x8000, CRC(9333dbca) SHA1(dd87e6f69d60580fdb6f979398edbeb1a51be355) )
	ROM_LOAD( "sex-004_02a.a", 0x98000, 0x8000, CRC(6eda81f4) SHA1(6d64344691e3e52035a7d30fb3e762f0bd397db7) )
ROM_END

ROM_START( phrcraze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-0a.u5", 0x00000, 0x8000, CRC(ccd33a0c) SHA1(869b66af4369f3b4bc19336ca2b8104c7f652de7) ) /* 6221-40 U5-0A 041686 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) // questions
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
ROM_END

ROM_START( phrcrazea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-0.u5", 0x00000, 0x8000, CRC(f9642d0a) SHA1(6e9b9929bc28f6c26c70a8b762a2755dc097dbc4) ) /* 6221-40 U5-0 040386 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
ROM_END

ROM_START( phrcrazeb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5281-40_u5-3a.u5", 0x00000, 0x8000, CRC(d04c7657) SHA1(0b59fbf553eb5b68544ee2f94cf8106ab30ff1ed) ) /* 6221-40 U5-3A 100086 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_sex-2a", 0x00000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x10000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_std-8a", 0x20000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_std-7a", 0x30000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-6a", 0x40000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-5a", 0x50000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-4a", 0x60000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-3a", 0x70000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-2a", 0x80000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-1a", 0x90000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
ROM_END

ROM_START( phrcrazec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-40_u5-3.u5", 0x00000, 0x8000, CRC(bd8b5612) SHA1(614436da4ed45e0d974b565c5c765bcc1b9d94b5) ) /* 6221-40 U5-3 070986 */

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "phrz_u37.u37", 0x00000, 0x8000, CRC(237e221a) SHA1(7aa69375c2b9a9e73e0e4ed207bf595368b2deb2) ) /* 1st & 2nd half identical, but correct and verified */
	ROM_LOAD( "phrz_u38.u38", 0x08000, 0x8000, CRC(bfa78b67) SHA1(1b51c0e00240f798fe717624e706cb15700bc2f9) )
	ROM_LOAD( "phrz_u39.u39", 0x10000, 0x8000, CRC(9ce22cb3) SHA1(b653afb8f13decd993e434aaad69a6e09ab65f83) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "phrz_u40.u40", 0x00000, 0x8000, CRC(17dcddd4) SHA1(51682bdbfb67cd0ccf20b97e8fa12d72f0fe82ed) ) /* 1st & 2nd half identical, but correct and verified */

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_sex-2a", 0x00000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x10000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
	ROM_LOAD( "phrz1-07_std-8a", 0x20000, 0x8000, CRC(423eecd6) SHA1(ca8d181ccba05acba8ebc57f20e0542eda00c917) )
	ROM_LOAD( "phrz1-07_std-7a", 0x30000, 0x8000, CRC(05dd3900) SHA1(bb13a3c5f84771c450fa88560cc74c5a1be1b876) )
	ROM_LOAD( "phrz1-07_std-6a", 0x40000, 0x8000, CRC(c5980f24) SHA1(a3b665c74aaa704ffa382f95adac70c7c46fb446) )
	ROM_LOAD( "phrz1-07_std-5a", 0x50000, 0x8000, CRC(7cb395a9) SHA1(48b3ac524e6ae23f885b9b767e77930a89a81f5f) )
	ROM_LOAD( "phrz1-07_std-4a", 0x60000, 0x8000, CRC(effc811b) SHA1(2479539965ed541be417bbe48a5e66a58a6294aa) )
	ROM_LOAD( "phrz1-07_std-3a", 0x70000, 0x8000, CRC(c4c7dcee) SHA1(81d879df3da0fbe1cf2247d92b3853104a99689d) )
	ROM_LOAD( "phrz1-07_std-2a", 0x80000, 0x8000, CRC(527b3025) SHA1(36dc129d2276909643e90ae3810c8341076fd88c) )
	ROM_LOAD( "phrz1-07_std-1a", 0x90000, 0x8000, CRC(367f1dfa) SHA1(01d69004c365acefb0e52ac12593a3874c16ab9d) )
ROM_END

ROM_START( phrcrazev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6221-45_u5-2.u5", 0x00000, 0x8000, CRC(6122b5bb) SHA1(9952b14334287a992eefefbdc887b9a9215304ef) ) /* 6221-45 U5-2 070886 - Vertical version */

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "u39.bin",      0x00000, 0x4000, BAD_DUMP CRC(adbd2cdc) SHA1(a1e9481bd6ee0f8915cea43eaad3ebdd54438eed) )
	ROM_LOAD( "u38.bin",      0x04000, 0x4000, BAD_DUMP CRC(3578f00d) SHA1(c6780a6ee1b5eb00258a89bceabbbe380d79d299) )
	ROM_LOAD( "u37.bin",      0x08000, 0x4000, BAD_DUMP CRC(962f18a3) SHA1(ec1c3e470c59905c0f56fce2703f6ff586849512) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "u40.bin",      0x00000, 0x4000, BAD_DUMP CRC(493172c8) SHA1(a76ff5d0d3dd56b0ee4352f03c9ce92f107d34ec) )

	ROM_REGION( 0xa0000, "user1", ROMREGION_ERASEFF ) /* questions */
	ROM_LOAD( "phrz1-07_std-1", 0x00000, 0x8000, CRC(0a016c5e) SHA1(1a24ecd7fe59b08c75a1b4575c7fe467cc7f0cf8) )
	ROM_LOAD( "phrz1-07_std-2", 0x10000, 0x8000, CRC(e67dc49e) SHA1(5265af228531dc16db7f7ee78da6e51ef9a1d772) )
	ROM_LOAD( "phrz1-07_std-3", 0x20000, 0x8000, CRC(5c79a653) SHA1(85a904465b347564e937074e2b18159604c83e51) )
	ROM_LOAD( "phrz1-07_std-4", 0x30000, 0x8000, CRC(9837f757) SHA1(01106114b6997fe6432e519101f95c83a1f7cc1e) )
	ROM_LOAD( "phrz1-07_std-5", 0x40000, 0x8000, CRC(dc9d8682) SHA1(46973da4298d0ed149c651498527c91b8ba57e0a) )
	ROM_LOAD( "phrz1-07_std-6", 0x50000, 0x8000, CRC(48e24f17) SHA1(f50c85505f6ab2360f0885494001f174224f8575) )
	/* empty space as per instructions for other "sex" category roms */
	/* "Sex" questions revision A */
	ROM_LOAD( "phrz1-07_sex-2a", 0x80000, 0x8000, CRC(7ef3bca7) SHA1(f25cd01f996882a500e1a800d924759cd1de255d) )
	ROM_LOAD( "phrz1-07_sex-1a", 0x90000, 0x8000, CRC(ed7604b8) SHA1(b1e841b50b8ef6ae95fafac1c34b6d0337a05d18) )
ROM_END

ROM_START( couple )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.1d",  0x00000, 0x8000, CRC(bc70337a) SHA1(ffc484bc3965f0780d3fa5d8801af27a7164a417) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )  /*video timing?*/
ROM_END

/*f205v's dump,same except for the first z80 rom,first noticeable differences are that
it doesn't jump to the backup ram area and it gives an extra play if you reach a certain
amount of points (there is a dip switch to select the trigger: 150.000 or 200.000*/
ROM_START( couplep )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "p_1.1d", 0x00000, 0x8000, CRC(4601ace6) SHA1(a824ceebf8b9ce77ef2c8e92636e4261f2ae0420) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )  /*video timing?*/
ROM_END

/*f205v's dump,this one looks like an intermediate release between set1 and set2;
it has same dips as set1, but remaining machine code is the same as set2*/
ROM_START( couplei )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "i_1.1d", 0x00000, 0x8000, CRC(760fa29e) SHA1(a37a1562028d9615adff3d2ef88e0156354c720a) )
	ROM_LOAD( "2.1e",  0x10000, 0x8000, CRC(17372a93) SHA1(e0f0980003473555c2543d98d1494f82afa49f1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "3.9c",  0x00000, 0x8000, CRC(f017399a) SHA1(baf4c1bea6a12b1d4c8838552503fbdb81378411) )
	ROM_LOAD( "4.9d",  0x08000, 0x8000, CRC(66da76c1) SHA1(8cdcec008d0d51704544069246e9eabb5d5958ea) )
	ROM_LOAD( "5.10c", 0x10000, 0x8000, CRC(fc22bcf4) SHA1(cf3f6872965cb264d56d3a0b5ab998541b9af4ef) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "6.10d", 0x00000, 0x8000, CRC(a6a9a73d) SHA1(f3cb1d434d730f6e00f48079eaf8b88f57779fa0) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "7.7a",  0x00000, 0x0800, CRC(6c36361e) SHA1(7a018eecf3d8b7cf8845dcfcf8067feb292933b2) )  /*video timing?*/
ROM_END

DRIVER_INIT_MEMBER(merit_state,key_0)
{
	m_decryption_key = 0;
}

DRIVER_INIT_MEMBER(merit_state,key_2)
{
	m_decryption_key = 2;
}

DRIVER_INIT_MEMBER(merit_state,key_4)
{
	m_decryption_key = 4;
}

DRIVER_INIT_MEMBER(merit_state,key_5)
{
	m_decryption_key = 5;
}

DRIVER_INIT_MEMBER(merit_state,key_7)
{
	m_decryption_key = 7;
}

DRIVER_INIT_MEMBER(merit_state,couple)
{
	UINT8 *ROM = memregion("maincpu")->base();

	#if 0 //quick rom compare test
	{
		int i,r;
		r = 0;
		for(i=0;i<0x2000;i++)
		{
			if(ROM[0x14000+i] == ROM[0x16000+i])
				r++;
		}
		osd_printf_debug("%02x (in HEX) identical bytes (no offset done)\n",r);
	}
	#endif

	/*The banked rom isn't a *real* banking,it's just a strange rom hook-up,the 2nd
	  and the 3rd halves are 100% identical(!),unless it's an error of TWO different
	  dumpers it's just the way it is,a.k.a. it's an "hardware" banking.
	  update 20060118 by f205v: now we have 3 dumps from 3 different boards and they
	  all behave the same...*/
	membank("bank1")->set_base(ROM + 0x10000 + (0x2000 * 2));
}

DRIVER_INIT_MEMBER(merit_state,dtrvwz5)
{
	int i;
	UINT8 *ROM = memregion("maincpu")->base();
	/* fill b000 - b0ff with ret 0xc9 */
	for ( i = 0xb000; i < 0xb100; i++ )
		ROM[i] = 0xc9;

	ROM[0xb000] = 0xc9; /* ret */

	/* called by subroutine which reads inputs */
	ROM[0xb001] = 0x7a; /* ld   a,d */
	ROM[0xb002] = 0xa4; /* and  h */
	ROM[0xb003] = 0x47; /* ld   b,a */
	ROM[0xb004] = 0x7b; /* ld   a,e */
	ROM[0xb005] = 0xa5; /* and  l */
	ROM[0xb006] = 0x4f; /* ld   c,a */
	ROM[0xb007] = 0x7a; /* ld   a,d */
	ROM[0xb008] = 0xb4; /* or   h */
	ROM[0xb009] = 0x57; /* ld   d,a */
	ROM[0xb00a] = 0x7b; /* ld   a,e */
	ROM[0xb00b] = 0xb5; /* or   l */
	ROM[0xb00c] = 0x5f; /* ld   e,a */
	ROM[0xb00a] = 0xc9; /* ret */

	m_decryption_key = 6;
}

/* Gambling type games */

GAME( 1983, pitboss,  0,       casino5,  pitboss,  driver_device,  0,   ROT0,  "Merit", "The Pit Boss (2214-04)",           MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossa, pitboss, pitboss,  pitbossa, driver_device,  0,   ROT0,  "Merit", "The Pit Boss (2214-03, U5-0C)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossa1,pitboss, pitboss,  pitbossa1,driver_device,  0,   ROT0,  "Merit", "The Pit Boss (2214-03, U5-1C)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossb, pitboss, pitboss,  pitbossa, driver_device,  0,   ROT0,  "Merit", "The Pit Boss (2214-02?)",          MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1983, pitbossc, pitboss, pitboss,  pitbossb, driver_device,  0,   ROT0,  "Merit", "The Pit Boss (2214-?)",            MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1984, casino5,  0,       casino5,  casino5,  driver_device,  0,   ROT0,  "Merit", "Casino Five (3315-02, U5-0)",       MACHINE_SUPPORTS_SAVE )

GAME( 1984, mroundup, 0,       pitboss,  mroundup, driver_device,  0,   ROT0,  "Merit", "The Round Up",                      MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )

GAME( 1987, riviera,  0,       dodge,    riviera,  driver_device,  0,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-4A)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1986, rivieraa, riviera, dodge,    riviera,  driver_device,  0,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-4)",  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1986, rivierab, riviera, dodge,    rivierab, driver_device,  0,   ROT0,  "Merit", "Riviera Hi-Score (2131-08, U5-2D)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1986, bigappg,  0,       bigappg,  bigappg,  driver_device,  0,   ROT0,  "Big Apple Games / Merit", "The Big Apple (2131-13, U5-0)",   MACHINE_SUPPORTS_SAVE )

GAME( 1986, dodgectya,dodgecty,dodge,    dodge,    driver_device,  0,   ROT0,  "Merit", "Dodge City (2131-82, U5-0D)",      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1986, dodgectyb,dodgecty,dodge,    dodge,    driver_device,  0,   ROT0,  "Merit", "Dodge City (2131-82, U5-50)",      MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1986, dodgectyc,dodgecty,dodge,    dodge,    driver_device,  0,   ROT0,  "Merit", "Dodge City (2131-82, U5-0 GT)",    MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )

/* Trivia and Word games */

GAME( 1985, trvwzh,   0,       trvwhiz,  trivia,   merit_state, key_0,  ROT0,  "Merit", "Trivia ? Whiz (6221-00)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwzha,  trvwzh,  trvwhiz,  trivia,   merit_state, key_0,  ROT0,  "Merit", "Trivia ? Whiz (6221-00, with Sex trivia)",                MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwzhb,  trvwzh,  trvwhiz,  trivia,   merit_state, key_0,  ROT0,  "Merit", "Trivia ? Whiz (6221-00, Alt Gen trivia)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwzv,   trvwzh,  trvwhiz,  trivia,   merit_state, key_0,  ROT90, "Merit", "Trivia ? Whiz (6221-02, Vertical)",                       MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz2,   0,       trvwhiz,  trivia,   merit_state, key_2,  ROT90, "Merit", "Trivia ? Whiz (6221-05, Edition 2)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz2a,  trvwz2,  trvwhiz,  trivia,   merit_state, key_2,  ROT90, "Merit", "Trivia ? Whiz (6221-05, Edition 2 Alt Sex trivia)",       MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz3h,  0,       trvwhiz,  trivia,   merit_state, key_0,  ROT0,  "Merit", "Trivia ? Whiz (6221-05, Edition 3)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz3ha, trvwz3h, trvwhiz,  trivia,   merit_state, key_0,  ROT0,  "Merit", "Trivia ? Whiz (6221-05, Edition 3 Sex trivia III)",       MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz3v,  trvwz3h, trvwhiz,  trivia,   merit_state, key_0,  ROT90, "Merit", "Trivia ? Whiz (6221-04, Edition 3 Vertical)",             MACHINE_SUPPORTS_SAVE )

GAME( 1985, trvwz4,   0,       trvwhziv, trvwhziv, merit_state, key_5,  ROT90, "Merit", "Trivia ? Whiz (6221-13, U5-0B Edition 4)",                MACHINE_SUPPORTS_SAVE )
GAME( 1985, trvwz4a,  trvwz4,  trvwhziv, trvwhziv, merit_state, key_5,  ROT90, "Merit", "Trivia ? Whiz (6221-13, U5-0B Edition 4 Alt Sex trivia)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, tictac,   0,       tictac,   tictac,   merit_state, key_4,  ROT0,  "Merit", "Tic Tac Trivia (6221-23, U5-0C Horizontal)",              MACHINE_SUPPORTS_SAVE )
GAME( 1985, tictacv,  tictac,  tictac,   tictac,   merit_state, key_4,  ROT90, "Merit", "Tic Tac Trivia (6221-22, U5-0 Vertical)",                 MACHINE_SUPPORTS_SAVE )

GAME( 1986, phrcraze, 0,       phrcraze, phrcraze, merit_state, key_7,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-0A)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazea,phrcraze,phrcraze, phrcraza, merit_state, key_7,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-0)",                            MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazeb,phrcraze,phrcraze, phrcrazs, merit_state, key_7,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-3A Expanded Questions)",        MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazec,phrcraze,phrcraze, phrcrazs, merit_state, key_7,  ROT0,  "Merit", "Phraze Craze (6221-40, U5-3 Expanded Questions)",         MACHINE_SUPPORTS_SAVE )
GAME( 1986, phrcrazev,phrcraze,phrcraze, phrcrazs, merit_state, key_7,  ROT90, "Merit", "Phraze Craze (6221-45, U5-2 Vertical)",                   MACHINE_SUPPORTS_SAVE )

GAME( 1987, dtrvwz5,  0,       dtrvwz5,  dtrvwh5,  merit_state, dtrvwz5,ROT0,  "Merit", "Deluxe Trivia ? Whiz (6221-70, U5-0A Edition 5)",         MACHINE_SUPPORTS_SAVE )

GAME( 1988, couple,   0,       couple,   couple,  merit_state,  couple, ROT0,  "Merit", "The Couples (set 1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, couplep,  couple,  couple,   couplep, merit_state,  couple, ROT0,  "Merit", "The Couples (set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION )
GAME( 1988, couplei,  couple,  couple,   couple,  merit_state,  couple, ROT0,  "Merit", "The Couples (set 3)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_UNEMULATED_PROTECTION )
