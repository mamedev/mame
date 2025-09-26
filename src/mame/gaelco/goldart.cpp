// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
  _____________________________________________________________________
  |                    _____________  _______DALLAS____________        |
  |                    |  ROM U6    | |                        |   __  |
  |                    |____________| |KM62256BLG-7L    BATT   |   | | |
  |                        ____       |       DS5002FP         |   |J| |
  |             ____       |U7 |      |________________________|   |P| |
  |             356N       |___|   _________      ____________     |6| |
  |                                74HCT132N|     CXK5814P-35L     |_| |
  |                      _________ _________                 _________ |
  |                      74HCT373N||74HC04B1|   _________    74HCT245N |
  |            ________  _____________          |        |   _________ |
  | __         |74F112N| | ROM U11    |    TPC1020BFN-084C   74HCT245N |
  | | |        _____     |____________|         |        |   _________ |
  | |J|        |XTAL|    _________ _________    |________|   74HCT273E |
  | |P|        |____|    |SN74F32N||74LS257_|                _________ |
  | |1|        ______    _________ _________  _____________  74HCT273E |
  | |_|        | U10 |   |ULN2003A||74LS257_| UM611024AK-20| _________ |
  |  ______    ______    ______________________    ______    74HCT273E |
  |  |FUSE_|   |_JP2_|   |_________JP3_________|   |_JP5_|             |
  |____________________________________________________________________|

 JP1 = Power (9 pins)
 JP2 = Serial DB9 (unused)
 JP3 = Darts board (20 pins)
 JP5 = Video out (6 pins)
 JP6 = Buttons (15 pins)

 XTAL = 32.000 MHz

 U7 = Oki
 U10 = Unpopulated socket for Max 202

 The UM611024AK-20 is a '128K X 8BIT HIGH SPEED CMOS SRAM' so likely the video RAM
 http://www.datasheetcatalog.com/datasheets_pdf/U/T/6/1/UT611024.shtml

 There is another version with the Dallas DS5002FP replaced with a PIC16C54, but using
 the same data and OKI ROMs.

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class goldart_state : public driver_device
{
public:
	goldart_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_data(*this, "data"),
		m_io_in0(*this, "IN0")
	{ }

	void goldart(machine_config& config);
	void goldartp(machine_config& config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_data;
	required_ioport m_io_in0;

	void port1_w(uint8_t data);
	uint8_t port1_r();

	uint32_t screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	void main_prgmap(address_map &map) ATTR_COLD;
	void main_datamap(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint8_t[]> m_ram2;

	uint8_t m_port1 = 0;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
};

void goldart_state::port1_w(uint8_t data)
{
	logerror("%s: port1_w %02x\n", machine().describe_context(), data);
	m_port1 = data;
}

uint8_t goldart_state::port1_r()
{
	uint8_t const ret = m_port1;
	if (!machine().side_effects_disabled())
		logerror("%s: port1_r %02x\n", machine().describe_context(), ret);
	return ret;
}

void goldart_state::video_start()
{
}

uint32_t goldart_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int count = (y * 192) + (cliprect.min_x >> 1);
		uint16_t *const dstptr_bitmap = &bitmap.pix(y);
		for (int x = cliprect.min_x & ~1; x <= cliprect.max_x; x += 2)
		{
			uint8_t const data = m_ram[count];
			uint8_t const data2 = m_ram2[count];

			count++;

			dstptr_bitmap[x] = ((data & 0xf0) >> 4)  | (data2 & 0xf0);
			dstptr_bitmap[x + 1] = (data & 0x0f) | ((data2 & 0x0f) << 4);
		}
	}

	return 0;
}

uint8_t goldart_state::mem_r(offs_t offset)
{
	// must be some control bits (or DS5002FP memory access isn't correct) as registers map over ROM/RAM with no obvious way to select at the moment
	// and we need to be able to access full range of each ROM bank at least

	// sometimes bit 0x40 is set in port1, but it doesn't seem a simple RAM/ROM select
	// also bit 0x20 has been seen set too

	int const bank = m_port1 & 0x07;
	uint8_t ret = 0x00;

	ret = m_data[(bank * 0x10000) + offset];
	if (!machine().side_effects_disabled())
		logerror("%s: mem_r %02x:%04x: %02x (from ROM?)\n", machine().describe_context(), m_port1, offset, ret);

	if (offset == 0xfff3)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfff4)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfff8)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfffb)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfffc)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		// oki?
		return machine().rand();
	}

	if (offset == 0xfffd)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		// oki?
		return machine().rand();
	}

	if (offset == 0xfffe)
	{
	//  logerror("%s: mem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return m_io_in0->read();
	}

	return ret;
}

void goldart_state::mem_w(offs_t offset, uint8_t data)
{
	// registers seem to control write modes? (palette select bits, overwrite / transparent drawing etc.)

	// address fffe appears to be significant as it gets set with data fetched from a the same table as
	// the source gfx addresses (1 byte to bank register, 2 bytes to data pointer, 1 byte to fffe)

	// TODO: bugs(?) in ds5002 memory partitioning emulation mean that from 0x7000-0x7fff at least never
	// actually hit VRAM, hence the first gap in the title screen

	if (offset < 0xfe00)
	{
		/* below 0xfe00 (or up to 0xd800 at least - 192*288) is the bitmap area
		  the code at 0D42 writes each pixel as 4 bits in a byte write, two writes for each address
		  with the 4 bits that shouldn't change being set to 0xf
		  before this, the palette select for the object being drawn is written to 0xfffe
		  there is 128kb of video ram, so this makes sense */

		if ((m_ram[0xfffe] & 0xf0) != 0xf0) // maybe, there is no 16th palette? but this gets set on the lower part of the title screen (incorrectly due to memory access errors, or changes data fetches?)
		{
			if ((data & 0x0f) != 0x0f)
			{
				m_ram[offset] = (m_ram[offset] & 0xf0) | (data & 0x0f);
				m_ram2[offset] = (m_ram2[offset] & 0xf0) | (m_ram[0xfffe] & 0xf0) >> 4;
			}

			if ((data & 0xf0) != 0xf0)
			{
				m_ram[offset] = (m_ram[offset] & 0x0f) | (data & 0xf0);
				m_ram2[offset] = (m_ram2[offset] & 0x0f) | (m_ram[0xfffe] & 0xf0);
			}
		}
		else
		{
			m_ram[offset] = machine().rand();
		}
	}
	else if (offset < 0xffe0)
	{
		// fe00 - ffdf is the palette (15 palettes)
		m_ram[offset] = data;

		int const index = (offset & 0x1fe) >> 1;

		uint16_t const pal = (m_ram[(offset & 0xfffe)] << 8) | (m_ram[(offset & 0xfffe) | 1]);

		m_palette->set_pen_color(index, ((pal >> 10) & 0x1f) << 3, ((pal >> 5) & 0x1f) << 3, (pal & 0x1f) << 3);
	}
	else
	{
		// ffe0 - ffef unused?
		// fff0 - ffff are control registers
		m_ram[offset] = data;

		// fff3 / fff4 : written just after below

		// fff8 / fff9 / fffa : written together in a single piece of code at 34C1

		// fffb :  code to write 01/02/04/08

		// fffc :   oki?
		// fffd :   oki?

		// fffe :   xxxx ----   x = pen value to be copied to other ram area on pixel wirtes
	}

	if (offset<0xd800)
		logerror("%s: mem_w %04x: %02x (to VRAM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x bank %02x\n", machine().describe_context(), offset, data, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
	else if (offset<0xfe00)
		logerror("%s: mem_w %04x: %02x (to non-screen VRAM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x bank %02x\n", machine().describe_context(), offset, data, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
	else if (offset<0xffe0)
		logerror("%s: mem_w %04x: %02x (to palette)\n", machine().describe_context(), offset, data);
	else
		logerror("%s: mem_w %04x: %02x (to REGS?)\n", machine().describe_context(), offset, data);

}


void goldart_state::main_prgmap(address_map &map)
{
	map(0x00000, 0x07fff).readonly().share("sram");
}

void goldart_state::main_datamap(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(goldart_state::mem_r), FUNC(goldart_state::mem_w));
	map(0x10000, 0x17fff).ram().share("sram");
}

static INPUT_PORTS_START( goldart )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // not sure what these are but pressing them on the Covielsa screen brings up something different
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void goldart_state::machine_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x10000);
	m_ram2 = make_unique_clear<uint8_t[]>(0x10000);

	save_item(NAME(m_port1));
	save_pointer(NAME(m_ram), 0x10000);
	save_pointer(NAME(m_ram2), 0x10000);

	m_port1 = 0;
}

void goldart_state::machine_reset()
{
}

void goldart_state::goldart(machine_config &config)
{
	// basic machine hardware
	ds5002fp_device &maincpu(DS5002FP(config, "maincpu", 32_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &goldart_state::main_prgmap);
	maincpu.set_addrmap(AS_IO, &goldart_state::main_datamap);
	maincpu.set_vblank_int("screen", FUNC(goldart_state::irq0_line_hold));
	// only uses port 1?
	maincpu.port_out_cb<1>().set(FUNC(goldart_state::port1_w));
	maincpu.port_in_cb<1>().set(FUNC(goldart_state::port1_r));

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256*2, 512);
	screen.set_visarea(0, (192*2)-1, 0, 288-1);
	screen.set_screen_update(FUNC(goldart_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette, palette_device::BLACK, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void goldart_state::goldartp(machine_config &config)
{
	// basic machine hardware
	PIC16C54(config, "maincpu", 12'000'000); // Unknown clock

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256*2, 512);
	screen.set_visarea(0, (192*2)-1, 0, 288-1);
	screen.set_screen_update(FUNC(goldart_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, m_palette, palette_device::BLACK, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

/* Different versions of the internal code exist (0x6000-0x6fff code is VERY different between them).
   The one we're using for now is the Gaelco "official" archived one (for repairs, etc.). */

ROM_START( goldart )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "u11_e_262.u11", 0x00000, 0x80000, CRC(325551e0) SHA1(4fe8d71d448de3f8a9b5751bad6e90d2e556cb8f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u6_e.u6", 0x00000, 0x80000, CRC(dd9dc689) SHA1(11871ba815372c06f8b1367d2897c37953db7bdd) )
ROM_END

ROM_START( goldartfr )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "francia_dianas_794c_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(0d9c7d2c) SHA1(616652d5d07454293d00807a94c072f059528ed7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_so_fra_27c040.u6", 0x00000, 0x80000, CRC(727ce7b7) SHA1(533290aa97e33124a7697d72a9a108f0ab503ac5) )
ROM_END

ROM_START( goldartgr )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "alema_diana_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(f0119b2b) SHA1(f60c77e9352fdb8e6c00fd347d6af634da6f5ae3) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_son_aleman_15-2-95_27c4001.u6", 0x00000, 0x80000, CRC(fd494229) SHA1(41c2f9f185987510863116a95dc4f7cd6b6bb17c) )
ROM_END

ROM_START( goldartpt )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "p-262.u11", 0x00000, 0x80000, CRC(fa6537b0) SHA1(a4c3ac8f5139b18f0688beaa374c75a6f0aabcd2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "p-262.u6", 0x00000, 0x80000, CRC(4177e78b) SHA1(1099568b97a08c33a7da1bf46fc106f25af15e90) )
ROM_END

ROM_START( goldartuk )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "g.b_diana_5017_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(efd8bfc1) SHA1(d4d01a5d6d618ed2ecabc959a88eb14b1bbf6241) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "diana_so_uk_257a_26-7_27c040.u6", 0x00000, 0x80000, CRC(a93afb8b) SHA1(c7a5fc4e74a0743ffc729ec3214f318141a82cc0) )
ROM_END

// PIC16C54-based sets

ROM_START( goldartp )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "u11_e_262.u11", 0x00000, 0x80000, CRC(325551e0) SHA1(4fe8d71d448de3f8a9b5751bad6e90d2e556cb8f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u6_e.u6", 0x00000, 0x80000, CRC(dd9dc689) SHA1(11871ba815372c06f8b1367d2897c37953db7bdd) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "e_645b_gal16v8.bin",  0x000, 0x117, CRC(52545c28) SHA1(7967cd26f83d6bb437f6899dce2985f374787022) )
	ROM_LOAD( "e_645c_gal16v8.bin",  0x000, 0x117, CRC(05fd5d56) SHA1(67b33728914900fed9af2e280ca394659c7006e7) )
	ROM_LOAD( "e_645d_gal16v8.bin",  0x000, 0x117, CRC(6fd3c1ce) SHA1(36de47497b7f5751da3555d2051e96e78d1ca04b) )
	ROM_LOAD( "i_645c_gal16v8.bin",  0x000, 0x117, CRC(64dc7a3c) SHA1(c2be029ef886a5865ecd85f5efd03a8e059c9168) )
	ROM_LOAD( "m_p3138_gal16v8.bin", 0x000, 0x117, CRC(909dab7b) SHA1(e9f4bb239fa7843743e85e236ae0c744784a3b3f) )
	ROM_LOAD( "m_p3238_gal16v8.bin", 0x000, 0x117, CRC(e9e538d9) SHA1(9ea73a903a06111843fe64ae55cb29ee88803334) )
ROM_END

ROM_START( goldartpfr )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "francia_dianas_794c_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(0d9c7d2c) SHA1(616652d5d07454293d00807a94c072f059528ed7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_so_fra_27c040.u6", 0x00000, 0x80000, CRC(727ce7b7) SHA1(533290aa97e33124a7697d72a9a108f0ab503ac5) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "e_645b_gal16v8.bin",  0x000, 0x117, CRC(52545c28) SHA1(7967cd26f83d6bb437f6899dce2985f374787022) )
	ROM_LOAD( "e_645c_gal16v8.bin",  0x000, 0x117, CRC(05fd5d56) SHA1(67b33728914900fed9af2e280ca394659c7006e7) )
	ROM_LOAD( "e_645d_gal16v8.bin",  0x000, 0x117, CRC(6fd3c1ce) SHA1(36de47497b7f5751da3555d2051e96e78d1ca04b) )
	ROM_LOAD( "i_645c_gal16v8.bin",  0x000, 0x117, CRC(64dc7a3c) SHA1(c2be029ef886a5865ecd85f5efd03a8e059c9168) )
	ROM_LOAD( "m_p3138_gal16v8.bin", 0x000, 0x117, CRC(909dab7b) SHA1(e9f4bb239fa7843743e85e236ae0c744784a3b3f) )
	ROM_LOAD( "m_p3238_gal16v8.bin", 0x000, 0x117, CRC(e9e538d9) SHA1(9ea73a903a06111843fe64ae55cb29ee88803334) )
ROM_END

ROM_START( goldartpgr )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "alema_diana_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(f0119b2b) SHA1(f60c77e9352fdb8e6c00fd347d6af634da6f5ae3) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_son_aleman_15-2-95_27c4001.u6", 0x00000, 0x80000, CRC(fd494229) SHA1(41c2f9f185987510863116a95dc4f7cd6b6bb17c) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "e_645b_gal16v8.bin",  0x000, 0x117, CRC(52545c28) SHA1(7967cd26f83d6bb437f6899dce2985f374787022) )
	ROM_LOAD( "e_645c_gal16v8.bin",  0x000, 0x117, CRC(05fd5d56) SHA1(67b33728914900fed9af2e280ca394659c7006e7) )
	ROM_LOAD( "e_645d_gal16v8.bin",  0x000, 0x117, CRC(6fd3c1ce) SHA1(36de47497b7f5751da3555d2051e96e78d1ca04b) )
	ROM_LOAD( "i_645c_gal16v8.bin",  0x000, 0x117, CRC(64dc7a3c) SHA1(c2be029ef886a5865ecd85f5efd03a8e059c9168) )
	ROM_LOAD( "m_p3138_gal16v8.bin", 0x000, 0x117, CRC(909dab7b) SHA1(e9f4bb239fa7843743e85e236ae0c744784a3b3f) )
	ROM_LOAD( "m_p3238_gal16v8.bin", 0x000, 0x117, CRC(e9e538d9) SHA1(9ea73a903a06111843fe64ae55cb29ee88803334) )
ROM_END

ROM_START( goldartppt )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "p-262.u11", 0x00000, 0x80000, CRC(fa6537b0) SHA1(a4c3ac8f5139b18f0688beaa374c75a6f0aabcd2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "p-262.u6", 0x00000, 0x80000, CRC(4177e78b) SHA1(1099568b97a08c33a7da1bf46fc106f25af15e90) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "e_645b_gal16v8.bin",  0x000, 0x117, CRC(52545c28) SHA1(7967cd26f83d6bb437f6899dce2985f374787022) )
	ROM_LOAD( "e_645c_gal16v8.bin",  0x000, 0x117, CRC(05fd5d56) SHA1(67b33728914900fed9af2e280ca394659c7006e7) )
	ROM_LOAD( "e_645d_gal16v8.bin",  0x000, 0x117, CRC(6fd3c1ce) SHA1(36de47497b7f5751da3555d2051e96e78d1ca04b) )
	ROM_LOAD( "i_645c_gal16v8.bin",  0x000, 0x117, CRC(64dc7a3c) SHA1(c2be029ef886a5865ecd85f5efd03a8e059c9168) )
	ROM_LOAD( "m_p3138_gal16v8.bin", 0x000, 0x117, CRC(909dab7b) SHA1(e9f4bb239fa7843743e85e236ae0c744784a3b3f) )
	ROM_LOAD( "m_p3238_gal16v8.bin", 0x000, 0x117, CRC(e9e538d9) SHA1(9ea73a903a06111843fe64ae55cb29ee88803334) )
ROM_END

ROM_START( goldartpuk )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "g.b_diana_5017_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(efd8bfc1) SHA1(d4d01a5d6d618ed2ecabc959a88eb14b1bbf6241) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "diana_so_uk_257a_26-7_27c040.u6", 0x00000, 0x80000, CRC(a93afb8b) SHA1(c7a5fc4e74a0743ffc729ec3214f318141a82cc0) )

	ROM_REGION( 0x117, "plds", 0 )
	ROM_LOAD( "e_645b_gal16v8.bin",  0x000, 0x117, CRC(52545c28) SHA1(7967cd26f83d6bb437f6899dce2985f374787022) )
	ROM_LOAD( "e_645c_gal16v8.bin",  0x000, 0x117, CRC(05fd5d56) SHA1(67b33728914900fed9af2e280ca394659c7006e7) )
	ROM_LOAD( "e_645d_gal16v8.bin",  0x000, 0x117, CRC(6fd3c1ce) SHA1(36de47497b7f5751da3555d2051e96e78d1ca04b) )
	ROM_LOAD( "i_645c_gal16v8.bin",  0x000, 0x117, CRC(64dc7a3c) SHA1(c2be029ef886a5865ecd85f5efd03a8e059c9168) )
	ROM_LOAD( "m_p3138_gal16v8.bin", 0x000, 0x117, CRC(909dab7b) SHA1(e9f4bb239fa7843743e85e236ae0c744784a3b3f) )
	ROM_LOAD( "m_p3238_gal16v8.bin", 0x000, 0x117, CRC(e9e538d9) SHA1(9ea73a903a06111843fe64ae55cb29ee88803334) )
ROM_END


} // Anonymous namespace

//    YEAR, NAME,       PARENT,  MACHINE,  INPUT,   CLASS,         INIT,       ROT,  COMPANY,             FULLNAME

GAME( 1994, goldart,    0,       goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Spain)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, goldartfr,  goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Jeutel",   "Goldart (France, Covielsa license)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, goldartgr,  goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Germany)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, goldartpt,  goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Portugal)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, goldartuk,  goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (United Kingdom)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 199?, goldartp,   goldart, goldartp, goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (PIC16C54, Spain)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, goldartpfr, goldart, goldartp, goldart, goldart_state, empty_init, ROT0, "Gaelco / Jeutel",   "Goldart (PIC16C54, France, Covielsa license)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, goldartpgr, goldart, goldartp, goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (PIC16C54, Germany)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, goldartppt, goldart, goldartp, goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (PIC16C54, Portugal)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 199?, goldartpuk, goldart, goldartp, goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (PIC16C54, United Kingdom)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
