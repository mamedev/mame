// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
Konami 053251
------
Priority encoder.

The chip has inputs for 5 layers (CI0-CI4); only 4 are used (CI1-CI4)
CI0-CI2 are 9(=5+4) bits inputs, CI3-CI4 8(=4+4) bits

The input connctions change from game to game. E.g. in Simpsons,
CI0 = grounded (background color)
CI1 = sprites
CI2 = FIX
CI3 = A
CI4 = B

in lgtnfght:
CI0 = grounded
CI1 = sprites
CI2 = FIX
CI3 = B
CI4 = A

there are three 6 bit priority inputs, PR0-PR2

simpsons:
PR0 = 111111
PR1 = xxxxx0 x bits coming from the sprite attributes
PR2 = 111111

lgtnfght:
PR0 = 111111
PR1 = 1xx000 x bits coming from the sprite attributes
PR2 = 111111

also two shadow inputs, SDI0 and SDI1 (from the sprite attributes)

the chip outputs the 11 bit palette index, CO0-CO10, and two shadow bits.

16 internal registers; registers are 6 bits wide (input is D0-D5)
For the most part, their meaning is unknown
All registers are write only.
There must be a way to enable/disable the three external PR inputs.
Some games initialize the priorities of the sprite & background layers,
others don't. It isn't clear whether the data written to those registers is
actually used, since the priority is taken from the external ports.

 0  priority of CI0 (higher = lower priority)
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 3f = 111111
    xmen:     05 = 000101  default value
    xmen:     09 = 001001  used to swap CI0 and CI2
 1  priority of CI1 (higher = lower priority)
    punkshot: 28 = 101000
    lgtnfght: unused?
    simpsons: unused?
    xmen:     02 = 000010
 2  priority of CI2 (higher = lower priority)
    punkshot: 24 = 100100
    lgtnfght: 24 = 100100
    simpsons: 04 = 000100
    xmen:     09 = 001001  default value
    xmen:     05 = 000101  used to swap CI0 and CI2
 3  priority of CI3 (higher = lower priority)
    punkshot: 34 = 110100
    lgtnfght: 34 = 110100
    simpsons: 28 = 101000
    xmen:     00 = 000000
 4  priority of CI4 (higher = lower priority)
    punkshot: 2c = 101100  default value
    punkshot: 3c = 111100  used to swap CI3 and CI4
    punkshot: 26 = 100110  used to swap CI1 and CI4
    lgtnfght: 2c = 101100
    simpsons: 18 = 011000
    xmen:     fe = 111110
 5  unknown
    punkshot: unused?
    lgtnfght: 2a = 101010
    simpsons: unused?
    xmen: unused?
 6  unknown
    punkshot: 26 = 100110
    lgtnfght: 30 = 110000
    simpsons: 17 = 010111
    xmen:     03 = 000011 (written after initial tests)
 7  unknown
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 27 = 100111
    xmen:     07 = 000111 (written after initial tests)
 8  unknown
    punkshot: unused?
    lgtnfght: unused?
    simpsons: 37 = 110111
    xmen:     ff = 111111 (written after initial tests)
 9  ----xx CI0 palette index base (CO9-CO10)
    --xx-- CI1 palette index base (CO9-CO10)
    xx---- CI2 palette index base (CO9-CO10)
10  ---xxx CI3 palette index base (CO8-CO10)
    xxx--- CI4 palette index base (CO8-CO10)
11  unknown
    punkshot: 00 = 000000
    lgtnfght: 00 = 000000
    simpsons: 00 = 000000
    xmen:     00 = 000000 (written after initial tests)
12  unknown
    punkshot: 04 = 000100
    lgtnfght: 04 = 000100
    simpsons: 05 = 000101
    xmen:     05 = 000101
13  unused
14  unused
15  unused


*/

#include "emu.h"
#include "k053251.h"
#include "konami_helper.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K053251 = &device_creator<k053251_device>;

k053251_device::k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K053251, "K053251 Priority Encoder", tag, owner, clock, "k053251", __FILE__),
	//m_dirty_tmap[5],
	//m_ram[16],
	m_tilemaps_set(0)
	//m_palette_index[5]
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k053251_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053251_device::device_start()
{
	save_item(NAME(m_ram));
	save_item(NAME(m_tilemaps_set));
	save_item(NAME(m_dirty_tmap));

	machine().save().register_postload(save_prepost_delegate(FUNC(k053251_device::reset_indexes), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k053251_device::device_reset()
{
	int i;

	m_tilemaps_set = 0;

	for (i = 0; i < 0x10; i++)
		m_ram[i] = 0;

	for (i = 0; i < 5; i++)
		m_dirty_tmap[i] = 0;

	reset_indexes();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_MEMBER( k053251_device::write )
{
	int i, newind;

	data &= 0x3f;

	if (m_ram[offset] != data)
	{
		m_ram[offset] = data;
		if (offset == 9)
		{
			/* palette base index */
			for (i = 0; i < 3; i++)
			{
				newind = 32 * ((data >> 2 * i) & 0x03);
				if (m_palette_index[i] != newind)
				{
					m_palette_index[i] = newind;
					m_dirty_tmap[i] = 1;
				}
			}

			if (!m_tilemaps_set)
				space.machine().tilemap().mark_all_dirty();
		}
		else if (offset == 10)
		{
			/* palette base index */
			for (i = 0; i < 2; i++)
			{
				newind = 16 * ((data >> 3 * i) & 0x07);
				if (m_palette_index[3 + i] != newind)
				{
					m_palette_index[3 + i] = newind;
					m_dirty_tmap[3 + i] = 1;
				}
			}

			if (!m_tilemaps_set)
				space.machine().tilemap().mark_all_dirty();
		}
	}
}

WRITE16_MEMBER( k053251_device::lsb_w )
{
	if (ACCESSING_BITS_0_7)
		write(space, offset, data & 0xff);
}

WRITE16_MEMBER( k053251_device::msb_w )
{
	if (ACCESSING_BITS_8_15)
		write(space, offset, (data >> 8) & 0xff);
}

int k053251_device::get_priority( int ci )
{
	return m_ram[ci];
}

int k053251_device::get_palette_index( int ci )
{
	return m_palette_index[ci];
}

int k053251_device::get_tmap_dirty( int tmap_num )
{
	assert(tmap_num < 5);
	return m_dirty_tmap[tmap_num];
}

void k053251_device::set_tmap_dirty( int tmap_num, int data )
{
	assert(tmap_num < 5);
	m_dirty_tmap[tmap_num] = data ? 1 : 0;
}

void k053251_device::reset_indexes()
{
	m_palette_index[0] = 32 * ((m_ram[9] >> 0) & 0x03);
	m_palette_index[1] = 32 * ((m_ram[9] >> 2) & 0x03);
	m_palette_index[2] = 32 * ((m_ram[9] >> 4) & 0x03);
	m_palette_index[3] = 16 * ((m_ram[10] >> 0) & 0x07);
	m_palette_index[4] = 16 * ((m_ram[10] >> 3) & 0x07);
}

// debug handlers

READ16_MEMBER( k053251_device::lsb_r )
{
	return(m_ram[offset]);
}       // PCU1

READ16_MEMBER( k053251_device::msb_r )
{
	return(m_ram[offset] << 8);
}       // PCU1
