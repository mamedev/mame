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

 XATL = 32.000 MHz

 U7 = Oki
 U10 = Unpopulated socket for Max 202

 The UM611024AK-20 is a '128K X 8BIT HIGH SPEED CMOS SRAM' so likely the video RAM
 http://www.datasheetcatalog.com/datasheets_pdf/U/T/6/1/UT611024.shtml

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
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
		m_mcu(*this, "mcu"),
		m_palette(*this, "palette"),
		m_data(*this, "data")
	{ }

	void goldart(machine_config& config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<ds5002fp_device> m_mcu;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_data;

	void mcu_port1_w(uint8_t data);
	uint8_t mcu_port1_r();


	uint32_t screen_update_goldart(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	void dallas_rom(address_map &map) ATTR_COLD;
	void dallas_ram(address_map &map) ATTR_COLD;

	uint8_t m_ram[0x10000];
	uint8_t m_ram2[0x10000];

	uint8_t m_port1 = 0;

	uint8_t hostmem_r(offs_t offset);
	void hostmem_w(offs_t offset, uint8_t data);
};

void goldart_state::mcu_port1_w(uint8_t data)
{
	logerror("%s: mcu_port1_w %02x\n", machine().describe_context(), data);
	m_port1 = data;
}

uint8_t goldart_state::mcu_port1_r()
{
	uint8_t ret = m_port1;
	logerror("%s: mcu_port1_r %02x\n", machine().describe_context(), ret);
	return ret;
}

void goldart_state::video_start()
{
}

uint32_t goldart_state::screen_update_goldart(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	int count = 0;
	for (int y = 0; y < 288; y++)
	{
		for (int x = 0; x < 192; x++)
		{
			uint16_t *const dstptr_bitmap = &bitmap.pix(y);
			uint8_t const data = m_ram[count];
			uint8_t const data2 = m_ram2[count];

			count++;

			dstptr_bitmap[x*2] = ((data&0xf0)>>4)  | (data2 & 0xf0);
			dstptr_bitmap[(x*2)+1] = (data&0x0f) | ((data2 & 0x0f)<<4);
		}
	}

	return 0;
}

uint8_t goldart_state::hostmem_r(offs_t offset)
{
	// must be some control bits (or DS5002FP memory access isn't correct) as registers map over ROM/RAM with no obvious way to select at the moment
	// and we need to be able to access full range of each ROM bank at least

	// sometimes bit 0x40 is set in port1, but it doesn't seem a simple RAM/ROM select
	// also bit 0x20 has been seen set too

	int bank = m_port1 & 0x07;
	uint8_t ret = 0x00;

	ret = m_data[(bank * 0x10000) + offset];
	logerror("%s: hostmem_r %02x:%04x: %02x (from ROM?)\n", machine().describe_context(), m_port1, offset, ret);

	if (offset == 0xfff3)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfff4)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfff8)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfffb)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return machine().rand();
	}

	if (offset == 0xfffc)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		// oki?
		return machine().rand();
	}

	if (offset == 0xfffd)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		// oki?
		return machine().rand();
	}

	if (offset == 0xfffe)
	{
	//  logerror("%s: hostmem_r %04x: %02x (from ROM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02 bank %02xx\n", machine().describe_context(), offset, ret, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
		return ioport("IN0")->read();
	}

	return ret;
}

void goldart_state::hostmem_w(offs_t offset, uint8_t data)
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

		int index = (offset & 0x1fe)>>1;

		uint16_t pal = (m_ram[(offset&0xfffe)] << 8) | (m_ram[(offset&0xfffe)+1]);

		m_palette->set_pen_color(index, ((pal >> 10) & 0x1f)<<3, ((pal >> 5) & 0x1f)<<3, (pal & 0x1f)<<3);
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
		logerror("%s: hostmem_w %04x: %02x (to VRAM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x bank %02x\n", machine().describe_context(), offset, data, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
	else if (offset<0xfe00)
		logerror("%s: hostmem_w %04x: %02x (to non-screen VRAM?) %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x bank %02x\n", machine().describe_context(), offset, data, m_ram[0xfff0], m_ram[0xfff1], m_ram[0xfff2], m_ram[0xfff3], m_ram[0xfff4], m_ram[0xfff5], m_ram[0xfff6], m_ram[0xfff7], m_ram[0xfff8], m_ram[0xfff9], m_ram[0xfffa], m_ram[0xfffb], m_ram[0xfffc], m_ram[0xfffd], m_ram[0xfffe], m_ram[0xffff], m_port1);
	else if (offset<0xffe0)
		logerror("%s: hostmem_w %04x: %02x (to palette)\n", machine().describe_context(), offset, data);
	else
		logerror("%s: hostmem_w %04x: %02x (to REGS?)\n", machine().describe_context(), offset, data);


}


void goldart_state::dallas_rom(address_map &map)
{
	map(0x00000, 0x07fff).readonly().share("sram");
}

void goldart_state::dallas_ram(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(goldart_state::hostmem_r), FUNC(goldart_state::hostmem_w));
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
	save_item(NAME(m_port1));
	save_item(NAME(m_ram));
	save_item(NAME(m_ram2));

	m_port1 = 0;
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	std::fill(std::begin(m_ram2), std::end(m_ram2), 0);
}

void goldart_state::machine_reset()
{
}

void goldart_state::goldart(machine_config &config)
{
	/* basic machine hardware */
	ds5002fp_device &mcu(DS5002FP(config, "mcu", 12000000));
	mcu.set_addrmap(AS_PROGRAM, &goldart_state::dallas_rom);
	mcu.set_addrmap(AS_IO, &goldart_state::dallas_ram);
	mcu.set_vblank_int("screen", FUNC(goldart_state::irq0_line_hold));
	// only uses port 1?
	mcu.port_out_cb<1>().set(FUNC(goldart_state::mcu_port1_w));
	mcu.port_in_cb<1>().set(FUNC(goldart_state::mcu_port1_r));

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256*2, 512);
	screen.set_visarea(0, (192*2)-1, 0, 288-1);
	screen.set_screen_update(FUNC(goldart_state::screen_update_goldart));
	screen.set_palette("palette");

	PALETTE(config, m_palette, palette_device::BLACK, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

/* Different versions of the internal code exist (0x6000-0x6fff code is VERY different between them)
   the one we're using for now was taken from the Spanish set but doesn't seem region specific as
   all strings are referenced through tables of pointers in the external ROM

   Code at 692F is potentially incorrect should be 0E or 1F
   Code at 6ABF is potentially incorrect should be AE or 74

   dump is tested and runs attract mode correctly on PCB
*/


ROM_START( goldart )
	ROM_REGION( 0x8000, "sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, BAD_DUMP CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "u11_e_262.bin", 0x00000, 0x80000, CRC(325551e0) SHA1(4fe8d71d448de3f8a9b5751bad6e90d2e556cb8f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u6_e.bin", 0x00000, 0x80000, CRC(dd9dc689) SHA1(11871ba815372c06f8b1367d2897c37953db7bdd) )
ROM_END

ROM_START( goldartp )
	ROM_REGION( 0x8000, "sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, BAD_DUMP CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "p-262.u11", 0x00000, 0x80000, CRC(fa6537b0) SHA1(a4c3ac8f5139b18f0688beaa374c75a6f0aabcd2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "p-262.u6", 0x00000, 0x80000, CRC(4177e78b) SHA1(1099568b97a08c33a7da1bf46fc106f25af15e90) )
ROM_END

} // Anonymous namespace


GAME( 1994, goldart,       0,        goldart,     goldart,      goldart_state, empty_init, ROT0, "Covielsa / Gaelco",   "Goldart (Spain)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1994, goldartp,      goldart,  goldart,     goldart,      goldart_state, empty_init, ROT0, "Covielsa / Gaelco",   "Goldart (Portugal)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
