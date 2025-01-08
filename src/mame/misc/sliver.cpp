// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*
Sliver - Hollow Corp.1996
driver by Tomasz Slanina

Custom blitter + background framebuffer + oki for sound.

The background images on this hardware are in JPEG format, the Zoran chips are
hardware JPEG decompression chips.

TODO:
- verify OKI rom banking  (bank num inverted or not)
- fix transparency problems in some stages


PCB Layout
----------

WS16-AJ-940820
|---------|-----------|------------------------------------|
|   TL084 |   i8031   |   KA-1                             |
|         |-----------||--------|                          |
|  AD-65    KA-2       |ACTEL   |   KA-6  KA-7  KA-8  KA-9 |
|                      |A1020B  |                          |
|           KA-3       |PL84C   |                          |
|               PAL    |        |                          |
|                      |--------|                          |
|             AT76C176                62256        62256   |
|                                                          |
|J             KM75C02                62256        62256   |
|A                                                         |
|M                 PAL                62256        62256   |
|M                                                         |
|A                                    62256        62256   |
|                                                  HY534256|
|   DSW1           PAL                 |-------|   HY534256|
|          PAL     PAL                 |ZORAN  |   HY534256|
|                            |-------| |ZR36011|   HY534256|
|   DSW2   62256    62256    |ZORAN  | |-------|           |
|          KA-4     KA-5     |ZR36050|                     |
|                            |-------|        |-------|    |
|        |------------------|16MHz            |FUJI   |    |
|        |                  |                 |MD0204 |    |
|        |    MC68000P10    |                 |-------|    |
|        |                  |           KA-12  KA-11  KA-10|
|24MHz   |------------------|                              |
|----------------------------------------------------------|
Notes:
      i8031    - Intel 8031 CPU, clock 8.000MHz (DIP40)
      68000    - Motorola MC68000 CPU, clock 12.000MHz (DIP64)
      AT76C176 - Atmel AT76C176 1-Channel 6-Bit AD/DA Convertor with Clamp Circuit (DIP28).
                 When removed, text and _some_ graphics turn black (palette related use)
                 This chip is compatible to Fujitsu MB40176
      A1020B   - Actel A1020B FPGA (PLCC84)
      ZR36050  - Zoran ZR36050PQC-21 DF4B9423G (QFP100)
      ZR36011  - Zoran ZR36011PQC JAPAN 079414 (QFP100)
      MD0204   - Fuji MD0204 JAPAN F39D110 (QFP128) - Memory controller
      62256    - 32K x8 SRAM (DIP28)
      HY534256 - Hyundai 256K x4 (1MBit) DRAM (DIP20)
      KM75C02  - Samsung KM75C02 FIFO RAM (DIP28)
      AD-65    - Clone OKI M6295 (QFP44), clock 1.000MHz, sample rate = 1000000Hz / 132
      VSync    - 60Hz
*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "video/ramdac.h"

#include "screen.h"
#include "speaker.h"

#include "jpeglib.h"


namespace {

#define FIFO_SIZE 1024
#define IO_SIZE     0x100
#define COMMAND_SIZE 8
#define x_offset 0x45
#define y_offset 0xe

class sliver_state : public driver_device
{
public:
	sliver_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_palette(*this, "palette")
	{ }

	void sliver(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	uint16_t m_io_offset = 0;
	uint16_t m_io_reg[IO_SIZE]{};
	uint16_t m_fifo[FIFO_SIZE]{};
	uint16_t m_fptr = 0;

	uint16_t m_jpeg1 = 0;
	uint16_t m_jpeg2 = 0;
	int m_jpeg_x = 0;
	int m_jpeg_y = 0;
	int m_tmp_counter = 0;

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<palette_device> m_palette;
	bitmap_rgb32 m_bitmap_fg;
	bitmap_rgb32 m_bitmap_bg;

	uint16_t m_tempbuf[8];

	void fifo_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fifo_clear_w(uint16_t data);
	void fifo_flush_w(uint16_t data);
	void jpeg1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void jpeg2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void io_offset_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void io_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_w(uint16_t data);
	void oki_setbank(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(obj_irq_cb);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void plot_pixel_rgb(int x, int y, uint32_t r, uint32_t g, uint32_t b);
	void plot_pixel_pal(int x, int y, u8 addr);
	void blit_gfx();
	void render_jpeg();

	void oki_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void sliver_map(address_map &map) ATTR_COLD;
	void soundmem_io(address_map &map) ATTR_COLD;
	void soundmem_prg(address_map &map) ATTR_COLD;
};

void sliver_state::machine_start()
{
	membank("okibank")->configure_entries(0, 4, memregion("oki")->base() + 0x20000, 0x20000);

	save_item(NAME(m_io_offset));
	save_item(NAME(m_io_reg));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fptr));
	save_item(NAME(m_tempbuf));
	save_item(NAME(m_tmp_counter));
}

void sliver_state::plot_pixel_rgb(int x, int y, uint32_t r, uint32_t g, uint32_t b)
{
//  printf("plot %d %d %d\n", r,g,b);

	if (y < 0 || x < 0 || x > 383 || y > 255)
		return;

	m_bitmap_bg.pix(y, x) = r | (g<<8) | (b<<16);
}

void sliver_state::plot_pixel_pal(int x, int y, u8 addr)
{
	if (y < 0 || x < 0 || x > 383 || y > 255)
		return;

	m_bitmap_fg.pix(y, x) = m_palette->pen(addr);
}

void sliver_state::fifo_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_tmp_counter < 8)
	{
		COMBINE_DATA(&m_tempbuf[m_tmp_counter]);
		m_tmp_counter++;
		if (m_tmp_counter == 8) // copy 8 bytes to fifo,  every byte should be copied directly, but it's easier to copy whole commands
		{
			do
			{
				m_fifo[m_fptr++]=m_tempbuf[8-m_tmp_counter];
				if (m_fptr > (FIFO_SIZE - 1))
				{
					m_fptr=FIFO_SIZE-1;
				}
			}
			while (--m_tmp_counter > 0);
		}
	}
}

void sliver_state::blit_gfx()
{
	int tmpptr=0;
	const uint8_t *rom = memregion("user1")->base();

	while (tmpptr < m_fptr)
	{
		int x,y;
		u8 romdata;
		int w,h;
		int romoffs=m_fifo[tmpptr+0]+(m_fifo[tmpptr+1] << 8)+(m_fifo[tmpptr+2] << 16);

		w=m_fifo[tmpptr+3]+1;
		h=m_fifo[tmpptr+4]+1;

		if (m_fifo[tmpptr+7] == 0)
		{
			for (y=0; y < h; y++)
			{
				for (x=0; x < w; x++)
				{
					romdata = rom[romoffs&0x1fffff];
					if (romdata)
					{
						plot_pixel_pal(m_fifo[tmpptr+5]+m_fifo[tmpptr+3]-x, m_fifo[tmpptr+6]+m_fifo[tmpptr+4]-y, romdata);
					}
					romoffs++;
				}
			}
		}
		tmpptr+=COMMAND_SIZE;
	}
}

void sliver_state::fifo_clear_w(uint16_t data)
{
	m_bitmap_fg.fill(0);
	m_fptr=0;
	m_tmp_counter=0;
}

void sliver_state::fifo_flush_w(uint16_t data)
{
	blit_gfx();
}


void sliver_state::jpeg1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_jpeg1);
}

void sliver_state::render_jpeg()
{
	int x;
	int addr = (int)m_jpeg2 + (((int)m_jpeg1) << 16);

	m_bitmap_bg.fill(0);
	if (addr < 0)
	{
		return;
	}

	//printf("access address %04x\n", addr);

	/* Access libJPEG */
	{
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		JSAMPARRAY buffer;

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);

		jpeg_mem_src(&cinfo, memregion("user2")->base()+addr, memregion("user2")->bytes()-addr);

		jpeg_read_header(&cinfo, true);
		jpeg_start_decompress(&cinfo);

		int row_stride = cinfo.output_width * cinfo.output_components;

		buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			int y = cinfo.output_scanline;

			for (x = 0; x < row_stride/3; x++)
			{
				uint8_t b = buffer[0][(x*3)];
				uint8_t g = buffer[0][(x*3)+1];
				uint8_t r = buffer[0][(x*3)+2];
				plot_pixel_rgb(x - x_offset + m_jpeg_x, y - y_offset - m_jpeg_y, r, g, b);

			}

		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);


	}

}

void sliver_state::jpeg2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_jpeg2);

	render_jpeg();

}

void sliver_state::io_offset_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_io_offset);
}

void sliver_state::io_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_io_offset < IO_SIZE)
	{
		int tmpx, tmpy;
		COMBINE_DATA(&m_io_reg[m_io_offset]);

		tmpy = m_io_reg[0x1a] + (m_io_reg[0x1b] << 8) - m_io_reg[0x20]; //0x20  ???
		tmpx = m_io_reg[0x1e] + (m_io_reg[0x1f] << 8);

		if (tmpy != m_jpeg_y || tmpx != m_jpeg_x)
		{
			m_jpeg_x = tmpx;
			m_jpeg_y = tmpy;
			render_jpeg();
		}
	}
	else
	{
		logerror("I/O access out of range: %x\n", m_io_offset);
	}
}

void sliver_state::sound_w(uint16_t data)
{
	m_soundlatch->write(data & 0xff);
	m_audiocpu->set_input_line(MCS51_INT0_LINE, HOLD_LINE);
}

void sliver_state::sliver_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();

	map(0x100001, 0x100001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x100003, 0x100003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x100005, 0x100005).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x300002, 0x300003).noprw(); // bit 0 tested, writes 0xe0 and 0xc0 - both r and w at the end of interrupt code

	map(0x300004, 0x300005).w(FUNC(sliver_state::io_offset_w)); //unknown i/o device
	map(0x300006, 0x300007).w(FUNC(sliver_state::io_data_w));

	map(0x400000, 0x400001).portr("P1_P2");
	map(0x400002, 0x400003).portr("SYSTEM");
	map(0x400004, 0x400005).portr("DSW");
	map(0x400006, 0x400007).w(FUNC(sliver_state::fifo_data_w));
	map(0x400008, 0x400009).w(FUNC(sliver_state::fifo_clear_w));
	map(0x40000a, 0x40000b).w(FUNC(sliver_state::fifo_flush_w));
	map(0x40000c, 0x40000d).w(FUNC(sliver_state::jpeg1_w));
	map(0x40000e, 0x40000f).w(FUNC(sliver_state::jpeg2_w));

	map(0x400010, 0x400015).nopw(); //unknown
	map(0x400016, 0x400017).w(FUNC(sliver_state::sound_w));
	map(0x400018, 0x400019).nopw(); //unknown

	map(0xff0000, 0xffffff).ram();
}

// Sound CPU

void sliver_state::oki_setbank(uint8_t data)
{
	int bank=(data^0xff)&3; //xor or not ?
	membank("okibank")->set_entry(bank);
}

void sliver_state::soundmem_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void sliver_state::soundmem_io(address_map &map)
{
	map(0x0100, 0x0100).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0101, 0x0101).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void sliver_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("okibank");
}

void sliver_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap_bg);
	m_screen->register_screen_bitmap(m_bitmap_fg);

	save_item(NAME(m_jpeg1));
	save_item(NAME(m_jpeg2));
	save_item(NAME(m_jpeg_x));
	save_item(NAME(m_jpeg_y));

	m_jpeg1 = 0;
	m_jpeg2 = 0;
	m_jpeg_x = 0;
	m_jpeg_y = 0;
}

void sliver_state::device_post_load()
{
	render_jpeg();
}

uint32_t sliver_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap      (bitmap, m_bitmap_bg, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_bitmap_fg, 0, 0, 0, 0, cliprect, 0);
	return 0;
}

static INPUT_PORTS_START( sliver )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //jpeg ready flag
	PORT_BIT( 0xffa4, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0020, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0000, "Longer" )
	PORT_DIPSETTING(      0x0040, "Long" )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, "Short" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "2 Player Mode" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Draw Insert" )       PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Joystick Input Mode" )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, "Step" )
	PORT_DIPSETTING(      0x0000, "Continuous" )
	PORT_DIPNAME( 0x4000, 0x4000, "Game Paused (Test)" )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x0000, "SW2:8" )    /* Listed as "UNUSED (MUST ON)" */
INPUT_PORTS_END

void sliver_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

TIMER_DEVICE_CALLBACK_MEMBER ( sliver_state::obj_irq_cb )
{
	m_maincpu->set_input_line(3, HOLD_LINE);
}

void sliver_state::sliver(machine_config &config)
{
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sliver_state::sliver_map);
	m_maincpu->set_vblank_int("screen", FUNC(sliver_state::irq4_line_hold));

	TIMER(config, "obj_actel").configure_periodic(FUNC(sliver_state::obj_irq_cb), attotime::from_hz(60)); /* unknown clock, causes "obj actel ready error" without this */
	// irq 2 valid but not used?

	I8051(config, m_audiocpu, 8000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sliver_state::soundmem_prg);
	m_audiocpu->set_addrmap(AS_IO, &sliver_state::soundmem_io);
	m_audiocpu->port_out_cb<1>().set(FUNC(sliver_state::oki_setbank));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 384-1-16, 0*8, 240-1);
	m_screen->set_screen_update(FUNC(sliver_state::screen_update));

	PALETTE(config, "palette").set_entries(0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &sliver_state::ramdac_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	okim6295_device &oki(OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH));
	oki.set_addrmap(0, &sliver_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.6);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.6);
}

ROM_START( sliver )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ka-4.bin", 0x00001, 0x20000, CRC(4906367f) SHA1(cc030930ffe7018ba6c362cab136798d027db7d8) )
	ROM_LOAD16_BYTE( "ka-5.bin", 0x00000, 0x20000, CRC(f260dabc) SHA1(3727cb8aa652809386075b39a1d85d5b20973702) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 8031 */
	ROM_LOAD( "ka-1.bin", 0x000000, 0x10000, CRC(56e616a2) SHA1(f8952aba62ae0410e300d99e95dc8b752543af1e) )

	ROM_REGION( 0xa0000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ka-2.bin", 0x00000, 0x20000, CRC(3df96eb0) SHA1(ec3dfc29da08f6525a1c708839f83094a6784f72) )
	ROM_LOAD( "ka-3.bin", 0x20000, 0x80000, CRC(33ee929c) SHA1(a652ad68c547248ef5fa1ed8006b7ac7aef76383) )

	ROM_REGION( 0x200000, "user1", 0 ) /* Graphics (not tiles) */
	ROM_LOAD16_BYTE( "ka-8.bin", 0x000000, 0x80000, CRC(dbfd7489) SHA1(4a7b07d041dce04a8d8d6688698164f988baefc9) )
	ROM_LOAD16_BYTE( "ka-6.bin", 0x000001, 0x80000, CRC(bd182316) SHA1(a22db9f73a2865f59630183c14201aeede821642) )
	ROM_LOAD16_BYTE( "ka-9.bin", 0x100000, 0x40000, CRC(71f044ba) SHA1(bd88bfaa0249de9fd8eb8bd25eae0126744a9046) )
	ROM_LOAD16_BYTE( "ka-7.bin", 0x100001, 0x40000, CRC(1c5d6fb9) SHA1(372533264eb41a5f57b2a59eb039adb6334f36c5) )

	ROM_REGION( 0x180000, "user2", 0 ) /* JPEG(!) compressed GFX */
	ROM_LOAD( "ka-10.bin", 0x000000, 0x80000, CRC(a6824271) SHA1(2eefa4e61491f7b72ccde744fa6f88a1a3c60c92) )
	ROM_LOAD( "ka-11.bin", 0x080000, 0x80000, CRC(4ae121ff) SHA1(ece7cc07483801a0d436def977d72dc7b1a07c8f) )
	ROM_LOAD( "ka-12.bin", 0x100000, 0x80000, CRC(0901e142) SHA1(68ebd38beeedf53414a831c01813881feee33446) )
ROM_END

ROM_START( slivera )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ka-4.bin", 0x00001, 0x20000, CRC(044d1046) SHA1(dd6da44e65dccae7e2ea0c72c3f8d1452ae8c9e2) ) // sldh
	ROM_LOAD16_BYTE( "ka-5.bin", 0x00000, 0x20000, CRC(c2e8b785) SHA1(84444495c4ecb8da50fef0998dbd9b4352f46582) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 8031 */
	ROM_LOAD( "ka-1.bin", 0x000000, 0x10000, CRC(56e616a2) SHA1(f8952aba62ae0410e300d99e95dc8b752543af1e) )

	ROM_REGION( 0xa0000, "oki", 0 ) /* Samples */
	ROM_LOAD( "ka-2.bin", 0x00000, 0x20000, CRC(3df96eb0) SHA1(ec3dfc29da08f6525a1c708839f83094a6784f72) )
	ROM_LOAD( "ka-3.bin", 0x20000, 0x80000, CRC(33ee929c) SHA1(a652ad68c547248ef5fa1ed8006b7ac7aef76383) )

	ROM_REGION( 0x200000, "user1", 0 ) /* Graphics (not tiles) */
	ROM_LOAD16_BYTE( "ka-8.bin", 0x000000, 0x80000, CRC(dbfd7489) SHA1(4a7b07d041dce04a8d8d6688698164f988baefc9) )
	ROM_LOAD16_BYTE( "ka-6.bin", 0x000001, 0x80000, CRC(bd182316) SHA1(a22db9f73a2865f59630183c14201aeede821642) )
	ROM_LOAD16_BYTE( "ka-9.bin", 0x100000, 0x40000, CRC(71f044ba) SHA1(bd88bfaa0249de9fd8eb8bd25eae0126744a9046) )
	ROM_LOAD16_BYTE( "ka-7.bin", 0x100001, 0x40000, CRC(1c5d6fb9) SHA1(372533264eb41a5f57b2a59eb039adb6334f36c5) )

	ROM_REGION( 0x180000, "user2", 0 ) /* JPEG(!) compressed GFX */
	ROM_LOAD( "ka-10.bin", 0x000000, 0x80000, CRC(639ad3ca) SHA1(d3c6a071aac62a3048e9f5bf2eb835619aa1a83b) ) // sldh
	ROM_LOAD( "ka-11.bin", 0x080000, 0x80000, CRC(47c05898) SHA1(51f7bb4ccaa5440a31aae9c02ed255243a3c8e22) ) // sldh
	// no rom 12 on PCB, played through the game and doesn't seem to be required for this gfx set, all girls present.
ROM_END

} // Anonymous namespace


GAME( 1996, sliver,  0,        sliver, sliver, sliver_state, empty_init, ROT0, "Hollow Corp", "Sliver (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, slivera, sliver,   sliver, sliver, sliver_state, empty_init, ROT0, "Hollow Corp", "Sliver (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
