// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
N-Sub Daughterboard 97269-P-B
-----------------------------
Reference Image: http://images.arianchen.de/2017-01/030320171775.jpg
Wiring Notes:    http://files.arianchen.de/nsub/97269-P-B.txt

|---------------------------------------------------|
| PR-02   74393   PR-57   74175   74367   7417  (A) |
|                                                   |
|         PR-56   7410    74393   7400    74367 (B) |
|                                                   |
|         74161   74161   74157   74175   74365 (C) |
|                                                   |
| (1)     (2)     (3)     (4)     (5)     (6)       |
|         SEGA    97269-P-B                         |
| [---CON-F---]   [-----------CON-X-----------]     |
|---------------------------------------------------|
Notes:
      PR-02.A1 - MMI 6336-1J 256*8 PROM (DIP24, labelled 'PR-02')
      PR-56.B2 - Fujitsu MB7054 1024*4  (DIP18, labelled 'PR-56')
      PR-57.A3 - Fujitsu MB7054 1024*4  (DIP18, labelled 'PR-57')

      CON-F    - A 3.6 Pin Connector for monitor (different pinout!)
               01 - NC
               02 - NC
               03 - GND
               04 - KEY
               05 - NC
               06 - NC
               07 - CSYNC
               08 - GREEN
               09 - BLUE
               10 - RED

      CON-X    - A 15 Pin Connector which is wired to various places on the mainboard
               01 - VCC
               02 - GND
               03 - CSYNC (U53.1)
               04 - PALBANK-B2 (U18.7)
               05 - EXTBLANK (U84.8)
               06 - VIDEO (U54.13)
               07 - PALBANK-B3 (U18.2)
               08 - VSYNC (U53.12)
               09 - VBLANK (U17.5)
               10 - BLUE (U8.7)
               11 - GREEN (U8.9)
               12 - RED (U8.4)
               13 - HSYNC- (U13.9)
               14 - HRESET (U13.6)
               15 - SRCK (U54.7)
*/

#include "emu.h"
#include "vicdual-97269pb.h"

#define S97269PB_TAG     "s97269pb"

/*************************************
 *  97269PB PROMS
 *************************************/
ROM_START( s97269pb )
	ROM_REGION( 0x1000, S97269PB_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "pr-02.a1", 0x0000, 0x0100, CRC(8e633b5c) SHA1(7ee40abe0e8a56aae2cc5f1102fa8c334a19e075) )
	ROM_LOAD( "pr-56.b2", 0x0800, 0x0400, CRC(f79c8e91) SHA1(eafff661e2aa1a6ef64f51faa9b1dac7af843557) )
	ROM_LOAD( "pr-57.a3", 0x0c00, 0x0400, CRC(00d52430) SHA1(5f4de97cfe6fe39da75761e0f434de28d30f3e3b) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(S97269PB, s97269pb_device, "s97269pb", "N-Sub Daughterboard 97269-P-B")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s97269pb_device::device_rom_region() const
{
	return ROM_NAME( s97269pb );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s97269pb_device - constructor
//-------------------------------------------------

s97269pb_device::s97269pb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S97269PB, tag, owner, clock),
	m_prom_ptr(*this, "s97269pb")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s97269pb_device::device_start()
{
	save_item(NAME(m_palette_bank));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s97269pb_device::device_reset()
{
	m_palette_bank = 0;
}

void s97269pb_device::palette_bank_w(uint8_t data)
{
	m_palette_bank = data & 0x0c;
}

pen_t s97269pb_device::choose_pen(uint8_t x, uint8_t y, pen_t back_pen)
{
	if (m_palette_bank & 0x04)
	{
		// TODO - starfield
		// gradient is offset by about 10 pixels

		// bit 2 enables the gradient and starfield/gradient
		// bit 3 seems to be used for cocktail mode
		uint8_t offset = ((x + 5) & 0xff) / 2;
		if (m_palette_bank & 0x08)
			offset |= 0x80;

		uint8_t gradient_flags = m_prom_ptr[offset];

		switch (gradient_flags >> 4)
		{
			case 0x1:
				// blue-to-cyan
				return rgb_t(0, 0x80 + (gradient_flags & 0x0f) * 0x08, 0xff);
				break;

			case 0x4:
				// black-to-blue
				return rgb_t(0, 0, (gradient_flags & 0x0f) * 0x11);
				break;

			case 0xf:
			default:
				return back_pen;
				break;
		}
	}
	return back_pen;
}
