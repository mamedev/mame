// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Toshiba Pasopia 1600

    TODO:
    - charset ROM is WRONG! (needs a 8x16 or even a 16x16 one)
    - identify fdc type (needs a working floppy image)

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class paso1600_state : public driver_device
{
public:
	paso1600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic8259")
		, m_dma(*this, "8237dma")
		, m_crtc(*this, "crtc")
		, m_p_vram(*this, "vram")
		, m_p_gvram(*this, "gvram")
		, m_p_chargen(*this, "chargen")
		, m_p_pcg(*this, "pcg")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
	}

	void paso1600(machine_config &config);

private:
	uint8_t paso1600_pcg_r(offs_t offset);
	void paso1600_pcg_w(offs_t offset, uint8_t data);
	void paso1600_6845_address_w(uint8_t data);
	void paso1600_6845_data_w(uint8_t data);
	uint8_t key_r(offs_t offset);
	void key_w(offs_t offset, uint8_t data);
	uint16_t test_hi_r();
	uint8_t pc_dma_read_byte(offs_t offset);
	void pc_dma_write_byte(offs_t offset, uint8_t data);
	uint32_t screen_update_paso1600(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void paso1600_io(address_map &map) ATTR_COLD;
	void paso1600_map(address_map &map) ATTR_COLD;

	uint8_t m_crtc_vreg[0x100]{}, m_crtc_index = 0;
	struct {
		uint8_t portb = 0;
	} m_keyb;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<am9517a_device> m_dma;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<uint16_t> m_p_vram;
	required_shared_ptr<uint16_t> m_p_gvram;
	required_region_ptr<u8> m_p_chargen;
	required_region_ptr<u8> m_p_pcg;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

#define mc6845_h_char_total     (m_crtc_vreg[0])
#define mc6845_h_display        (m_crtc_vreg[1])
#define mc6845_h_sync_pos       (m_crtc_vreg[2])
#define mc6845_sync_width       (m_crtc_vreg[3])
#define mc6845_v_char_total     (m_crtc_vreg[4])
#define mc6845_v_total_adj      (m_crtc_vreg[5])
#define mc6845_v_display        (m_crtc_vreg[6])
#define mc6845_v_sync_pos       (m_crtc_vreg[7])
#define mc6845_mode_ctrl        (m_crtc_vreg[8])
#define mc6845_tile_height      (m_crtc_vreg[9]+1)
#define mc6845_cursor_y_start   (m_crtc_vreg[0x0a])
#define mc6845_cursor_y_end     (m_crtc_vreg[0x0b])
#define mc6845_start_addr       (((m_crtc_vreg[0x0c]<<8) & 0x3f00) | (m_crtc_vreg[0x0d] & 0xff))
#define mc6845_cursor_addr      (((m_crtc_vreg[0x0e]<<8) & 0x3f00) | (m_crtc_vreg[0x0f] & 0xff))
#define mc6845_light_pen_addr   (((m_crtc_vreg[0x10]<<8) & 0x3f00) | (m_crtc_vreg[0x11] & 0xff))
#define mc6845_update_addr      (((m_crtc_vreg[0x12]<<8) & 0x3f00) | (m_crtc_vreg[0x13] & 0xff))


uint32_t paso1600_state::screen_update_paso1600(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if 0
	static int test_x;

	if(machine().input().code_pressed(KEYCODE_Z))
		test_x++;

	if(machine().input().code_pressed(KEYCODE_X))
		test_x--;

	popmessage("%d",test_x);

	uint32_t count = 0;

	for(int y=0;y<475;y++)
	{
		count &= 0xffff;

		for(int x=0;x<test_x/16;x++)
		{
			for(int xi=0;xi<16;xi++)
			{
				int pen = (m_p_gvram[count] >> xi) & 1;

				if(y < 475 && x*16+xi < 640) /* TODO: safety check */
					bitmap.pix(y, x*16+xi) = m_palette->pen(pen);
			}

			count++;
		}
	}
#endif

//  popmessage("%d %d %d",mc6845_h_display,mc6845_v_display,mc6845_tile_height);

	for(int y=0;y<mc6845_v_display;y++)
	{
		for(int x=0;x<mc6845_h_display;x++)
		{
			int tile = m_p_vram[x+y*mc6845_h_display] & 0xff;
			int color = (m_p_vram[x+y*mc6845_h_display] & 0x700) >> 8;

			for(int yi=0;yi<19;yi++)
			{
				for(int xi=0;xi<8;xi++)
				{
					int pen = (m_p_chargen[tile*8+(yi >> 1)] >> (7-xi) & 1) ? color : -1;

					if(yi & 0x10)
						pen = -1;

					if(pen != -1)
						if(y*19 < 475 && x*8+xi < 640) /* TODO: safety check */
							bitmap.pix(y*19+yi, x*8+xi) = m_palette->pen(pen);
				}
			}
		}
	}

	/* quick and dirty way to do the cursor */
	if(0)
	for(int yi=0;yi<mc6845_tile_height;yi++)
	{
		for(int xi=0;xi<8;xi++)
		{
			if((mc6845_cursor_y_start & 0x60) != 0x20 && mc6845_h_display)
			{
				int x = mc6845_cursor_addr % mc6845_h_display;
				int y = mc6845_cursor_addr / mc6845_h_display;
				bitmap.pix(y*mc6845_tile_height+yi, x*8+xi) = m_palette->pen(7);
			}
		}
	}

	return 0;
}

uint8_t paso1600_state::paso1600_pcg_r(offs_t offset)
{
	return m_p_pcg[offset];
}

void paso1600_state::paso1600_pcg_w(offs_t offset, uint8_t data)
{
	m_p_pcg[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 3);
}

void paso1600_state::paso1600_6845_address_w(uint8_t data)
{
	m_crtc_index = data;
	m_crtc->address_w(data);
}

void paso1600_state::paso1600_6845_data_w(uint8_t data)
{
	m_crtc_vreg[m_crtc_index] = data;
	m_crtc->register_w(data);
}

uint8_t paso1600_state::key_r(offs_t offset)
{
	switch(offset)
	{
		case 3:
			if(m_keyb.portb == 1)
				return 0;
	}

	return 0xff;
}

void paso1600_state::key_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 3: m_keyb.portb = data; break;
	}
}

uint16_t  paso1600_state::test_hi_r()
{
	return 0xffff;
}

void paso1600_state::paso1600_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).ram();
	map(0xb0000, 0xb0fff).ram().share("vram"); // tvram
	map(0xbfff0, 0xbffff).rw(FUNC(paso1600_state::paso1600_pcg_r), FUNC(paso1600_state::paso1600_pcg_w));
	map(0xc0000, 0xdffff).ram().share("gvram");// gvram
	map(0xe0000, 0xeffff).rom().region("kanji", 0);// kanji rom, banked via port 0x93
	map(0xfe000, 0xfffff).rom().region("ipl", 0);
}

void paso1600_state::paso1600_io(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x000f).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0010, 0x0011).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write)); // i8259
	map(0x001a, 0x001b).r(FUNC(paso1600_state::test_hi_r)); // causes RAM error otherwise?
	map(0x0030, 0x0033).rw(FUNC(paso1600_state::key_r), FUNC(paso1600_state::key_w)); //UART keyboard?
	map(0x0048, 0x0049).r(FUNC(paso1600_state::test_hi_r));
	map(0x0090, 0x0090).r(m_crtc, FUNC(mc6845_device::status_r)).w(FUNC(paso1600_state::paso1600_6845_address_w));
	map(0x0091, 0x0091).r(m_crtc, FUNC(mc6845_device::register_r)).w(FUNC(paso1600_state::paso1600_6845_data_w));
//  map(0x00d8,0x00df) //fdc, unknown type
// other undefined ports: 18, 1C, 92
}

/* Input ports */
static INPUT_PORTS_START( paso1600 )
INPUT_PORTS_END

static const gfx_layout paso1600_charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_paso1600 )
	GFXDECODE_ENTRY( "pcg", 0x0000, paso1600_charlayout, 0, 4 )
	GFXDECODE_ENTRY( "chargen", 0x0000, paso1600_charlayout, 0, 4 )
GFXDECODE_END


void paso1600_state::machine_start()
{
}


void paso1600_state::machine_reset()
{
}

uint8_t paso1600_state::pc_dma_read_byte(offs_t offset)
{
	//offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
	//  & 0xFF0000;

	address_space &program = m_maincpu->space(AS_PROGRAM);

	return program.read_byte(offset);
}


void paso1600_state::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	//offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
	//  & 0xFF0000;
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.write_byte(offset, data);
}

void paso1600_state::paso1600(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, 16000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &paso1600_state::paso1600_map);
	m_maincpu->set_addrmap(AS_IO, &paso1600_state::paso1600_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(paso1600_state::screen_update_paso1600));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_paso1600);
	PALETTE(config, m_palette).set_entries(8);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", 16000000/4));    /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	AM9517A(config, m_dma, 16000000/4);
	m_dma->in_memr_callback().set(FUNC(paso1600_state::pc_dma_read_byte));
	m_dma->out_memw_callback().set(FUNC(paso1600_state::pc_dma_write_byte));
}

ROM_START( paso1600 )
	ROM_REGION16_LE(0x2000,"ipl", 0)
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(cee4ebb7) SHA1(c23b30f8dc51f96c1c00e28aab61e77b50d261f0))

	ROM_REGION(0x2000,"pcg", ROMREGION_ERASE00)

	ROM_REGION( 0x800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "5t_2716.bin", 0x0000, 0x0800, BAD_DUMP CRC(b5693720) SHA1(d25327dfaa40b0f4144698e3bad43125fd8e46d0)) //stolen from pasopia - label and location unknown

	ROM_REGION16_LE( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, NO_DUMP)
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY    FULLNAME        FLAGS
COMP( 198?, paso1600, 0,      0,      paso1600, paso1600, paso1600_state, empty_init, "Toshiba", "Pasopia 1600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
