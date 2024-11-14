// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/*

Twins
Electronic Devices, 1994

PCB Layout
----------

ECOGAMES Twins
_________________________________________________________
| LABEL: VIDEOR-191193                                   |
| ____________________         _____________             |
| | U24 40PIN DEVICE |         | IMSG176P-66|        ____|
| |__________________|         |____________|        |
|                                                    |___
| ___________   74HC574  ___________       74HC574   ----|
| | D43256AC |           | D43256AC |                ----|
| |__________|  74LS245  |__________|      74LS245   ----|
|                                                    ----|
| 74LS245       74HC573  74LS245           74HC573   ----|
|  ____________   ____________  __________________   ----|
|  | 84256A-10L|  | 84256A-10L| | AY-3-8910A      |  ----|
|  |___________|  |___________| |_________________|  ----|
| _____________  _____________                       ----|
| | 1 27C4001  | | 2 27C4001  |                      ----|
| |____________| |____________| ___________________  ----|
|                               |U30 40PIN DEVICE |  ----|
| 74LS245      74LS245  74HC573 |_________________|  ----|
|                                                    ----|
| 74HC573      74HC573  74HC74      DS1232           ____|
|  __________________                                |
|  |NEC V30 9327N5   |  74HC04      24C2AB1          |___
|  |_________________|         XTAL8MHz                  |
|________________________________________________________|


ELECTRONIC DEVICES Twins

This is a very tiny PCB,
only about 6 inches square.

|-----------------------|
|     6116   16MHz 62256|
|TEST 6116 24C02        |
|             PAL  62256|
|J   62256 |--------|   |
|A         |TPC1020 | 2 |
|M   62256 |AFN-084C|   |
|M         |        | 1 |
|A         |--------|   |
| AY3-8910              |
|                 D70116|
|-----------------------|

Notes:
    V30 clock      : 8.000MHz (16/2)
    AY3-8910 clock : 2.000MHz (16/8)
    VSync          : 50Hz

seems a similar board to Hot Blocks

same TPC1020 AFN-084C chip
same 24c02 eeprom
V30 instead of I8088
AY3-8910 instead of YM2149 (compatible)

video is not banked in this case instead palette data is sent to the ports
strange palette format.

TODO:
- Proper fix for twins & twinsed2 crash after round 1 (MT #07516):
  after clearing 1-5 it pings i2c for a couple times, expect it to be 0 then 1 otherwise
  jumps to lalaland;
- Improve blitter / clear logic for Spider.
- Merge with hotblock.cpp;

Electronic Devices was printed on rom labels
1994 date string is in ROM

Spider PCB appears almost identical but uses additional 'blitter' features.
It is possible the Twins PCB has them too and doesn't use them.

twinsed1 is significantly changed hardware, uses a regular RAMDAC hookup for palette etc.

To access Service Mode:
- twins, twinsed2: you must boot with coin 1 and start 1 held down
  (there's a test button on the PCB tho?)
- spider, twinsed1: you must boot with P1 Left and P1 Right held down,
  this requires the -joystick_contradictory switch on the commandline;

*/

#include "emu.h"

#include "cpu/nec/nec.h"
#include "sound/ay8910.h"
#include "machine/bankdev.h"
#include "machine/i2cmem.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class twins_state : public driver_device
{
public:
	twins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_i2cmem(*this, "i2cmem")
		, m_overlay(*this, "overlay")
		, m_spritesinit(0)
		, m_videorambank(0)
	{ }

	void twins(machine_config &config);
	void init_twins();
	void init_twinsed2();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<i2cmem_device> m_i2cmem;
	required_device<address_map_bank_device> m_overlay;

	std::unique_ptr<u16 []> m_bgvram;
	std::unique_ptr<u16 []> m_fgvram;
	std::unique_ptr<u16 []> m_paletteram;
	uint16_t m_paloff = 0;
	int m_spritesinit;
	int m_spriteswidth = 0;
	int m_spritesaddr = 0;
	uint16_t m_videorambank;
	uint8_t* m_rom8 = nullptr;

	void base_config(machine_config &config);
	void video_config(machine_config &config);
	void sound_config(machine_config &config);
	void base_map(address_map &map) ATTR_COLD;
	void twins_map(address_map &map) ATTR_COLD;
	uint32_t screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t eeprom_r(offs_t offset, uint16_t mem_mask = ~0);
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void video_start() override ATTR_COLD;

	static constexpr u32 ram_size = 0x10000/2;

	inline u16* get_vram_base();
	uint16_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vram_rmw_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

private:
	void access_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t access_r(offs_t offset, uint16_t mem_mask = ~0);

	virtual void machine_start() override ATTR_COLD;

	void ramdac_map(address_map &map) ATTR_COLD;
	void twins_io(address_map &map) ATTR_COLD;
};

class twinsed1_state : public twins_state
{
public:
	twinsed1_state(const machine_config &mconfig, device_type type, const char *tag)
		: twins_state(mconfig, type, tag)
	{}

	void twinsed1(machine_config &config);

private:
	void twinsed1_io(address_map &map) ATTR_COLD;
	void porte_paloff0_w(uint8_t data);
	void twins_pal_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};

class spider_state : public twins_state
{
public:
	spider_state(const machine_config &mconfig, device_type type, const char *tag)
		: twins_state(mconfig, type, tag)
	{}

	void spider(machine_config &config);

private:
	uint32_t screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void spider_io(address_map &map) ATTR_COLD;
	void spider_paloff0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spider_port_18_r();
	uint16_t spider_port_1e_r();
	void spider_port_1a_w(uint16_t data);
	void spider_port_1c_w(uint16_t data);
	void spider_pal_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};

void twins_state::video_start()
{
	m_paloff = 0;

	save_item(NAME(m_paloff));
	save_item(NAME(m_spritesinit));
	save_item(NAME(m_spriteswidth));
	save_item(NAME(m_spritesaddr));

	m_bgvram = std::make_unique<u16 []>(ram_size);
	std::fill_n(m_bgvram.get(), ram_size, 0);
	save_pointer(NAME(m_bgvram), ram_size);

	m_fgvram = std::make_unique<u16 []>(ram_size);
	std::fill_n(m_fgvram.get(), ram_size, 0);
	save_pointer(NAME(m_fgvram), ram_size);

	const u16 palette_size = 0x100;
	m_paletteram = std::make_unique<u16 []>(palette_size);
	std::fill_n(m_paletteram.get(), palette_size, 0);
	save_pointer(NAME(m_paletteram), palette_size);

	save_item(NAME(m_videorambank));
}

void twins_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const videoram = util::little_endian_cast<uint8_t const>(m_bgvram.get());

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int count = (y * 320) + cliprect.left();
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			bitmap.pix(y, x) = videoram[count++];
	}
}

uint32_t twins_state::screen_update_twins(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen());
	draw_background(bitmap, cliprect);
	return 0;
}

void spider_state::draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const videoram = util::little_endian_cast<uint8_t const>(m_fgvram.get());

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int count = (y * 320) + cliprect.left();
		for(int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u8 const pixel = videoram[count++];
			if (pixel)
				bitmap.pix(y, x) = pixel;
		}
	}
}

uint32_t spider_state::screen_update_spider(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen());
	draw_background(bitmap, cliprect);
	draw_foreground(bitmap, cliprect);
	return 0;
}

void twins_state::machine_start()
{
	m_rom8 = memregion("ipl")->base();
}

uint16_t twins_state::eeprom_r(offs_t offset, uint16_t mem_mask)
{
//  printf("%08x: eeprom_r %04x\n", m_maincpu->pc(), mem_mask);
//  return m_i2cmem->read_sda();// | 0xfffe;
	// TODO: bit 1, i2c clock readback?

	return m_i2cmem->read_sda();
}

void twins_state::eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  printf("%08x: eeprom_w %04x %04x\n", m_maincpu->pc(), data, mem_mask);
	int i2c_clk = BIT(data, 1);
	int i2c_mem = BIT(data, 0);
	m_i2cmem->write_scl(i2c_clk);
	m_i2cmem->write_sda(i2c_mem);
}

void twinsed1_state::twins_pal_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paletteram[m_paloff]);

	{
		int dat,r,g,b;
		dat = m_paletteram[m_paloff];

		r = dat & 0x1f;
		r = bitswap<8>(r,7,6,5,0,1,2,3,4);

		g = (dat>>5) & 0x1f;
		g = bitswap<8>(g,7,6,5,0,1,2,3,4);

		b = (dat>>10) & 0x1f;
		b = bitswap<8>(b,7,6,5,0,1,2,3,4);

		m_palette->set_pen_color(m_paloff, pal5bit(r),pal5bit(g),pal5bit(b));
	}

	m_paloff = (m_paloff + 1) & 0xff;
}

// ??? weird...
void twinsed1_state::porte_paloff0_w(uint8_t data)
{
//  printf("porte_paloff0_w %04x\n", data);
	m_paloff = 0;
}

inline u16 *twins_state::get_vram_base()
{
	return (m_videorambank & 1) ? m_fgvram.get() : m_bgvram.get();
}

uint16_t twins_state::vram_r(offs_t offset)
{
	u16 *vram = get_vram_base();
	return vram[offset];
}

void twins_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	u16 *vram = get_vram_base();
	COMBINE_DATA(&vram[offset]);
}

// TODO: confirm this area being present on twins versions
void twins_state::vram_rmw_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	u16 *vram = get_vram_base();

//  printf("spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);

	for (int i = 0; i < m_spriteswidth; i++)
	{
		uint8_t data;

		data = (m_rom8[(m_spritesaddr * 2) + 1]);

		if (data)
			vram[offset] = (vram[offset] & 0x00ff) | data << 8;

		data = m_rom8[(m_spritesaddr*2)];

		if (data)
			vram[offset] = (vram[offset] & 0xff00) | data;

		m_spritesaddr ++;
		offset++;

		offset &= 0x7fff;
	}
}

uint16_t twins_state::access_r(offs_t offset, uint16_t mem_mask)
{
	return m_overlay->read16(offset, mem_mask);
}

void twins_state::access_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?

	if (m_spritesinit == 1)
	{
	//  printf("spider_blitter_w %08x %04x %04x (init?) (base?)\n", offset * 2, data, mem_mask);

		m_spritesinit = 2;
		m_spritesaddr = offset;
	}
	else if (m_spritesinit == 2)
	{
	//  printf("spider_blitter_w %08x %04x %04x (init2) (width?)\n", offset * 2, data, mem_mask);
		m_spriteswidth = offset & 0xff;
		if (m_spriteswidth == 0)
			m_spriteswidth = 80;

		m_spritesinit = 0;
	}
	else
		m_overlay->write16(offset, data, mem_mask);
}

void spider_state::spider_pal_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// ths first write doesn't appear to be a palette value
	if (m_paloff!=0)
	{
		COMBINE_DATA(&m_paletteram[m_paloff-1]);
		int dat,r,g,b;
		dat = m_paletteram[m_paloff-1];

		r = dat & 0x1f;
		g = (dat>>5) & 0x1f;
		b = (dat>>10) & 0x1f;
		m_palette->set_pen_color(m_paloff-1, pal5bit(r),pal5bit(g),pal5bit(b));
	}
	else
	{
	//  printf("first palette write %04x\n", data);
	}

	m_paloff++;

	if (m_paloff == 0x101)
		m_paloff = 0;
}


void spider_state::spider_paloff0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this seems to be video ram banking
	COMBINE_DATA(&m_videorambank);
}

void spider_state::spider_port_1a_w(uint16_t data)
{
	// writes 1
}


void spider_state::spider_port_1c_w(uint16_t data)
{
	// done before the 'sprite' read / writes
	// might clear a buffer?

	// game is only animating sprites at 30fps, maybe there's some double buffering too?

//  data written is always 00, only seems to want the upper layer to be cleared
//  otherwise you get garbage sprites between rounds and the bg incorrectly wiped

	for (int i = 0; i < ram_size; i++)
		m_fgvram[i] = 0x0000;
}


uint16_t spider_state::spider_port_18_r()
{
	// read before each blitter command
	// seems to put the bus in a state where the next 2 bus access offsets (anywhere) are the blitter params
	m_spritesinit = 1;

	return 0xff;
}

uint16_t spider_state::spider_port_1e_r()
{
	// done before each sprite pixel 'write'
	// the data read is the data written, but only reads one pixel??
	return 0xff;
}

void twins_state::twins_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(twins_state::access_r), FUNC(twins_state::access_w));
}

void twins_state::base_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).rw(FUNC(twins_state::vram_r), FUNC(twins_state::vram_w));
	map(0x20000, 0x2ffff).w(FUNC(twins_state::vram_rmw_w));
	map(0x20000, 0xfffff).rom().region("ipl", 0x20000);
}

void twinsed1_state::twinsed1_io(address_map &map)
{
	map(0x0000, 0x0001).nopr();
	map(0x0000, 0x0003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x0002, 0x0002).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x0004, 0x0005).rw(FUNC(twinsed1_state::eeprom_r), FUNC(twinsed1_state::eeprom_w));
	map(0x0006, 0x0007).w(FUNC(twinsed1_state::twins_pal_w));
	map(0x000e, 0x000f).w(FUNC(twinsed1_state::porte_paloff0_w));
}

/* The Ecogames set and the Electronic Devices second set has different palette hardware
   and a different port map than Electronic Devices first set */

void twins_state::twins_io(address_map &map)
{
	map(0x0000, 0x0001).nopr();
	map(0x0000, 0x0000).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x0002, 0x0002).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x0004, 0x0004).rw("ramdac", FUNC(ramdac_device::pal_r), FUNC(ramdac_device::pal_w));
	map(0x0008, 0x0008).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x0010, 0x0010).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x0018, 0x0019).r(FUNC(twins_state::eeprom_r)).w(FUNC(twins_state::eeprom_w));
}

void spider_state::spider_io(address_map &map)
{
	map(0x0000, 0x0001).nopr();
	map(0x0000, 0x0003).w("aysnd", FUNC(ay8910_device::address_data_w)).umask16(0x00ff);
	map(0x0002, 0x0002).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x0004, 0x0005).rw(FUNC(spider_state::eeprom_r), FUNC(spider_state::eeprom_w));
	map(0x0008, 0x0009).w(FUNC(spider_state::spider_pal_w));
	map(0x0010, 0x0011).w(FUNC(spider_state::spider_paloff0_w));

	map(0x0018, 0x0019).r(FUNC(spider_state::spider_port_18_r));
	map(0x001a, 0x001b).w(FUNC(spider_state::spider_port_1a_w));
	map(0x001c, 0x001d).w(FUNC(spider_state::spider_port_1c_w));
	map(0x001e, 0x001f).r(FUNC(spider_state::spider_port_1e_r));
}

void twins_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

static INPUT_PORTS_START(twins)
	PORT_START("P1")    // 8bit
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("P2")    // 8bit
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

void twins_state::base_config(machine_config &config)
{
	I2C_24C02(config, m_i2cmem);

	ADDRESS_MAP_BANK(config, m_overlay).set_map(&twins_state::base_map).set_options(ENDIANNESS_LITTLE, 16, 24, 0x100000);
}

void twins_state::video_config(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(8000000, 512, 0, 320, 312, 0, 204); // Common PAL values, HSync of 15.625 kHz unverified
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void twins_state::sound_config(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(16'000'000)/8)); // verified on PCB
	aysnd.port_a_read_callback().set_ioport("P1");
	aysnd.port_b_read_callback().set_ioport("P2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void twinsed1_state::twinsed1(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &twinsed1_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &twinsed1_state::twinsed1_io);

	video_config(config);
	m_screen->set_screen_update(FUNC(twinsed1_state::screen_update_twins));

	base_config(config);
	PALETTE(config, m_palette).set_entries(0x100);

	sound_config(config);
}

void twins_state::twins(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, XTAL(16'000'000)/2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &twins_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &twins_state::twins_io);

	video_config(config);
	m_screen->set_screen_update(FUNC(twins_state::screen_update_twins));

	PALETTE(config, m_palette).set_entries(256);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &twins_state::ramdac_map);
	ramdac.set_split_read(0);

	base_config(config);

	sound_config(config);
}

void spider_state::spider(machine_config &config)
{
	// basic machine hardware
	V30(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &spider_state::twins_map);
	m_maincpu->set_addrmap(AS_IO, &spider_state::spider_io);

	// video hardware
	video_config(config);
	m_screen->set_screen_update(FUNC(spider_state::screen_update_spider));

	PALETTE(config, m_palette).set_entries(0x100);

	base_config(config);

	sound_config(config);
}


// ECOGAMES Twins
ROM_START( twins )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "2.u8", 0x000000, 0x080000, CRC(1ec942b0) SHA1(627deb739c50f93c4cb61b8baf2a07213f1613b3) )
	ROM_LOAD16_BYTE( "1.u9", 0x000001, 0x080000, CRC(4417ff34) SHA1(be992128fe48556a0a7c018953702b4ce9076526) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02.u15", 0x000, 0x100, CRC(2ff05b0e) SHA1(df6854446ba83f4a13ddf68bd2d0bc35be21be79) )
ROM_END

ROM_START( twinsa )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "l.u8", 0x000000, 0x080000, CRC(19d16ba0) SHA1(2c42d7e1cde0f722dc5ebe7771fcc461b9a60962) )
	ROM_LOAD16_BYTE( "h.u9", 0x000001, 0x080000, CRC(9352b56e) SHA1(f9977d1d38941dc710e07d6d95cb6b88abc4a069) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02.u15", 0x000, 0x100, CRC(2ff05b0e) SHA1(df6854446ba83f4a13ddf68bd2d0bc35be21be79) )
ROM_END

// Electronic Devices Twins
ROM_START( twinsed1 )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "1.bin", 0x000000, 0x080000, CRC(d5ef7b0d) SHA1(7261dca5bb0aef755b4f2b85a159b356e7ac8219) )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x080000, CRC(8a5392f4) SHA1(e6a2ecdb775138a87d27aa4ad267bdec33c26baa) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02.u15", 0x000, 0x100, CRC(2ff05b0e) SHA1(df6854446ba83f4a13ddf68bd2d0bc35be21be79) )
ROM_END

/*
Shang Hay Twins
Electronic Devices

1x nec9328n8-v30-d70116c-8 (main)
2x ay-3-8910a (sound)
1x blank (z80?)
1x oscillator 8.000

2x M27c4001

1x jamma edge connector
1x trimmer (volume)

===

Author note: we're only emulating 1x ay-3-8910, assume being unused or dumper typo (unaccessed by the game)
twinsed2 is basically the same game as twins except having nudity pics (and inps are interchangeable between the sets)

*/

ROM_START( twinsed2 )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "lp.bin", 0x000000, 0x080000, CRC(4f07862e) SHA1(fbda1973f79c6938c7f026a4db706e78781c2df8) )
	ROM_LOAD16_BYTE( "hp.bin", 0x000001, 0x080000, CRC(aaf74b83) SHA1(09bd76b9fc5cb7ba6ffe1a2581ffd5633fe440b3) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02.u15", 0x000, 0x100, CRC(2ff05b0e) SHA1(df6854446ba83f4a13ddf68bd2d0bc35be21be79) )
ROM_END

ROM_START( spider )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "20.bin", 0x000001, 0x080000, CRC(25e15f11) SHA1(b728f35c817f60a294e38d66559da8977b94a1f5) )
	ROM_LOAD16_BYTE( "21.bin", 0x000000, 0x080000, CRC(ff224206) SHA1(d8d45850983542e811facc917d016841fc56a97f) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02", 0x000, 0x100, CRC(6f710d66) SHA1(1cc6d1134c5b81b7d0913f09c07d73675770d817) )
ROM_END

ROM_START( spidern )
	ROM_REGION16_LE( 0x100000, "ipl", 0 )
	ROM_LOAD16_BYTE( "22_gamart.bin", 0x000001, 0x080000, CRC(1ef32122) SHA1(b7ddb8b2456da22a25d5eb93f0228624847193e8) )
	ROM_LOAD16_BYTE( "21_gamart.bin", 0x000000, 0x080000, CRC(085c9936) SHA1(4307ab95d5c01393c781f55a30c003734cc5ee87) )

	ROM_REGION( 0x100, "i2cmem", 0 )
	ROM_LOAD("24c02", 0x000, 0x100, CRC(6f710d66) SHA1(1cc6d1134c5b81b7d0913f09c07d73675770d817) )
ROM_END

void twins_state::init_twins()
{
	u8 *rom = (u8 *)memregion("ipl")->base();

	rom[0x3497d] = 0x90;
	rom[0x3497e] = 0x90;

	rom[0x34986] = 0x90;
	rom[0x34987] = 0x90;
}

void twins_state::init_twinsed2()
{
	u8 *rom = (u8 *)memregion("ipl")->base();

	rom[0x349d3] = 0x90;
	rom[0x349d4] = 0x90;

	rom[0x349dc] = 0x90;
	rom[0x349dd] = 0x90;
}

} // anonymous namespace


GAME( 1993, twins,    0,      twins,    twins, twins_state,    init_twins,      ROT0, "Ecogames",                              "Twins (newer)",                             MACHINE_SUPPORTS_SAVE ) // 26/11/93 15:10:50
GAME( 1993, twinsa,   twins,  twins,    twins, twins_state,    init_twins,      ROT0, "Ecogames",                              "Twins (older)",                             MACHINE_SUPPORTS_SAVE ) // 23/11/93 13:13:33
GAME( 1994, twinsed1, twins,  twinsed1, twins, twinsed1_state, empty_init,      ROT0, "Ecogames (Electronic Devices license)", "Twins (Electronic Devices license, older)", MACHINE_SUPPORTS_SAVE ) // 18/01/94 16:07:56
GAME( 1994, twinsed2, twins,  twins,    twins, twins_state,    init_twinsed2,   ROT0, "Ecogames (Electronic Devices license)", "Twins (Electronic Devices license, newer)", MACHINE_SUPPORTS_SAVE ) // 19/01/94 11:10:22

GAME( 1994, spider,   0,      spider,   twins, spider_state,   empty_init,      ROT0, "Buena Vision",                          "Spider (Buena Vision, without nudity)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, spidern,  spider, spider,   twins, spider_state,   empty_init,      ROT0, "Buena Vision",                          "Spider (Buena Vision, with nudity)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
