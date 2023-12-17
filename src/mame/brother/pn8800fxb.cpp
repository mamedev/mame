// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Brother PN-8800FXB "Super PowerNote"

    Hardware:
    - HD64180RF6X CPU
    - TC551001BFL-70 (128k RAM)
    - 2x HY6264A (2x 8k, VRAM?)
    - HG62F33R32FH UC2836-A (gate array)
    - HD63266F FDC
    - RC224ATF (modem)
    - TC8521AM RTC
    - XTAL XT1 16.000312 MHz (near modem), XT2 12.228MHz (near CPU)
    - XTAL XT3 32.768kHz (near RTC), XT4 18.0MHz (near gate array)
    - XTAL XT5 16.0MHz (near FDC)

    TODO:
    - Almost everything, draws the initial screen

    Notes:
    - There is a serially connected daughterbord containing the Bookman logic

****************************************************************************/

#include "emu.h"

#include "cpu/z180/z180.h"

#include "emupal.h"
#include "screen.h"

namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pn8800fxb_state : public driver_device
{
public:
	pn8800fxb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void pn8800fxb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80180_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	std::unique_ptr<uint8_t[]> m_vram;
	uint16_t m_video_addr;
	uint8_t m_video_ctrl;
	uint8_t m_mem_change;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void video_addr_lo_w(uint8_t data);
	void video_addr_hi_w(uint8_t data);
	uint8_t video_data_r();
	void video_data_w(uint8_t data);
	uint8_t video_ctrl_r();
	void video_ctrl_w(uint8_t data);

	void mem_change_w(uint8_t data);
	void bank_select_w(uint8_t data);
	uint8_t lo_mem_r(offs_t offset);
	void lo_mem_w(offs_t offset, uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pn8800fxb_state::mem_map(address_map &map)
{
	map(0x00000, 0x01fff).rom();
	map(0x02000, 0x07fff).rw(FUNC(pn8800fxb_state::lo_mem_r), FUNC(pn8800fxb_state::lo_mem_w));
	map(0x08000, 0x3ffff).rom();
	map(0x40000, 0x41fff).unmaprw();
	map(0x42000, 0x45fff).rom().region("maincpu", 0x2000);
	map(0x46000, 0x4ffff).unmaprw();
	map(0x50000, 0x5ffff).bankr(m_rombank);
	map(0x60000, 0x7ffff).ram();
}

void pn8800fxb_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // z180 internal
	// 70 cursor
	// 71 cursor
	map(0x72, 0x72).w(FUNC(pn8800fxb_state::video_addr_lo_w));
	map(0x73, 0x73).w(FUNC(pn8800fxb_state::video_addr_hi_w));
	map(0x74, 0x74).rw(FUNC(pn8800fxb_state::video_data_r), FUNC(pn8800fxb_state::video_data_w));
	map(0x75, 0x75).rw(FUNC(pn8800fxb_state::video_ctrl_r), FUNC(pn8800fxb_state::video_ctrl_w));
	// 76 display offset
	// 77 cursor control
	// 78-7f mode select
	// 80-87 fdc
	// 88-8f input
	map(0x90, 0x90).mirror(0x07).w(FUNC(pn8800fxb_state::mem_change_w));
	// 98-9f rs break
	// a0-a7 rs232
	// a8-af not used
	// b0-b7 power supply
	// b8-bf keyboard
	// c0-c7 cdcc data
	// c8-cf cdcc control
	// d0-df rtc
	map(0xe0, 0xe0).mirror(0x07).w(FUNC(pn8800fxb_state::bank_select_w));
	// e8-ef ga test
	// f0-f7 buzzer
	// f8-ff interrupt ack
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( pn8800fxb )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void pn8800fxb_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(131, 238, 47));
	palette.set_pen_color(1, rgb_t(31, 84, 67));
}

uint32_t pn8800fxb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(0)->mark_all_dirty();

	const pen_t *const pen = m_palette->pens();

	for (int y = 0; y < 22; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			//uint8_t attr = m_vram[y * 0x100 + x * 2 + 0];
			uint8_t code = m_vram[y * 0x100 + x * 2 + 1];

			// draw 9 lines
			for (int l = 0; l < 9; l++)
			{
				uint8_t data = 0x00;

				// line 9 always blank?
				if (l < 8)
					data = m_vram[0x3000 + (code << 3) + l];

				// draw 8 pixels of the character
				for (int b = 0; b < 8; b++)
				{
					bitmap.pix(y * 9 + l, x * 8 + b) = pen[BIT(data, 7 - b)];
				}
			}
		}
	}

	return 0;
}

void pn8800fxb_state::video_addr_lo_w(uint8_t data)
{
	m_video_addr = (m_video_addr & 0xff00) | (data << 0);
}

void pn8800fxb_state::video_addr_hi_w(uint8_t data)
{
	data &= 0x7f;
	m_video_addr = (data << 8) | (m_video_addr & 0x00ff);
}

uint8_t pn8800fxb_state::video_data_r()
{
	return m_vram[m_video_addr];
}

void pn8800fxb_state::video_data_w(uint8_t data)
{
	m_vram[m_video_addr] = data;

	// auto-increment, assume wrap
	m_video_addr = (m_video_addr + 1) & 0x7fff;
}

uint8_t pn8800fxb_state::video_ctrl_r()
{
	// 7-------  mm
	// -6------  not used
	// --5-----  8r
	// ---4----  8lp
	// ----3---  grph
	// -----2--  not used
	// ------1-  rev
	// -------0  disp

	return m_video_ctrl;
}

void pn8800fxb_state::video_ctrl_w(uint8_t data)
{
	logerror("video_ctrl_w: %02x\n", data);

	m_video_ctrl = data;
}

static const gfx_layout charlayout =
{
	8, 8,
	512,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*8
};

static GFXDECODE_START( gfx )
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t pn8800fxb_state::lo_mem_r(offs_t offset)
{
	uint8_t data = 0xff;
	offset += 0x2000;

	if (offset < 0x6000)
		data = m_maincpu->space(AS_PROGRAM).read_byte(0x60000 + offset);
	else
		data = memregion("maincpu")->base()[offset];

	return data;
}

void pn8800fxb_state::lo_mem_w(offs_t offset, uint8_t data)
{
	offset += 0x2000;

	if (offset < 0x6000)
		m_maincpu->space(AS_PROGRAM).write_byte(0x60000 + offset, data);
	else
		logerror("write 6000-7fff %04x %02x\n", offset, data);
}

void pn8800fxb_state::mem_change_w(uint8_t data)
{
	// 7-------  dua
	// -6------  ramin
	// --5432--  not used
	// ------1-  dicsel
	// -------0  rgex

	logerror("mem_change_w: dua %d ramin %d dicsel %d rgex %d\n", BIT(data, 7), BIT(data, 6), BIT(data, 1), BIT(data, 0));

	m_mem_change = data;
}

void pn8800fxb_state::bank_select_w(uint8_t data)
{
	// 7654----  not used
	// ----3210  bank selection

	logerror("bank_select_w: %02x\n", data);

	if (data < 12)
		m_rombank->set_entry(data ^ 8); // TODO, but works for now
}

void pn8800fxb_state::machine_start()
{
	// allocate space for vram
	m_vram = std::make_unique<uint8_t[]>(0x4000);

	// init gfxdecode
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, charlayout, m_vram.get() + 0x3000, 0, 1, 0));

	// configure rom banking (first 0x40000 fixed?)
	m_rombank->configure_entries(0, 12, memregion("maincpu")->base() + 0x40000, 0x10000);

	// register for save states
	save_pointer(NAME(m_vram), 0x4000);
	save_item(NAME(m_video_addr));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_mem_change));
}

void pn8800fxb_state::machine_reset()
{
	m_mem_change = 0x00;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void pn8800fxb_state::pn8800fxb(machine_config &config)
{
	Z80180(config, m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pn8800fxb_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &pn8800fxb_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_refresh_hz(70.23);
	screen.set_screen_update(FUNC(pn8800fxb_state::screen_update));

	PALETTE(config, m_palette, FUNC(pn8800fxb_state::palette), 2);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pn8800 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD("uc8254-a-pn88.5", 0x000000, 0x100000, CRC(d9601c1a) SHA1(1699714befeaf2fe17232c1b4f49d4242f5367f4))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY     FULLNAME      FLAGS
COMP( 1996, pn8800, 0,      0,      pn8800fxb, pn8800fxb, pn8800fxb_state, empty_init, "Brother",  "PN-8800FXB", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
