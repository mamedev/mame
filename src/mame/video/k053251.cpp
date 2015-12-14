// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
/*
xexex:
[:k053251] write 0, 00
[:k053251] write 1, 00
[:k053251] write 2, 00
[:k053251] write 3, 00
[:k053251] write 4, 00
[:k053251] write 5, 3e
[:k053251] write 6, 3e
[:k053251] write 7, 3e
[:k053251] write 8, 3e
[:k053251] write 9, 0e  221100 - palette extra bits
[:k053251] write a, 1a  444333 - palette extra bits
[:k053251] write b, 00
[:k053251] write c, 06

[:k053251] write 0, 00
[:k053251] write 1, 08
[:k053251] write 2, 02
[:k053251] write 3, 04
[:k053251] write 4, 06
[:k053251] write 5, 3e
[:k053251] write c, 07

asterix:
[:k053251] write 0, 29
[:k053251] write 1, 10
[:k053251] write 2, 30
[:k053251] write 3, 00
[:k053251] write 4, 38
[:k053251] write 6, 28
[:k053251] write 9, 18
[:k053251] write a, 09
[:k053251] write b, 00
[:k053251] write c, 05

[:k053251] write 0, 29
[:k053251] write 1, 10
[:k053251] write 2, 30
[:k053251] write 3, 00
[:k053251] write 4, 38
[:k053251] write 6, 28
[:k053251] write 9, 18
[:k053251] write a, 09

[:k053251] write 0, 21
[:k053251] write 1, 10
[:k053251] write 2, 30
[:k053251] write 3, 00
[:k053251] write 4, 38
[:k053251] write 6, 28
[:k053251] write 9, 18
[:k053251] write a, 09

Konami 053251
------
Priority encoder.

The chip has inputs for 5 layers (CI0-CI4); only 4 are used (CI1-CI4)
CI0-CI2 are 9(=5+4) bits inputs, CI3-CI4 8(=4+4) bits

The input connections change from game to game. E.g. in Simpsons,
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

#include "k053251.h"
#include "kvideodac.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

const device_type K053251 = device_creator<k053251_device>;

DEVICE_ADDRESS_MAP_START(map, 8, k053251_device)
	AM_RANGE(0x00, 0x04) AM_WRITE(inpri_w)
	AM_RANGE(0x05, 0x08) AM_WRITE(rega_w)
	AM_RANGE(0x09, 0x0a) AM_WRITE(cblk_w)
	AM_RANGE(0x0b, 0x0f) AM_WRITE(regb_w)
ADDRESS_MAP_END

WRITE8_MEMBER(k053251_device::rega_w)
{
	data &= 0x3f;
	logerror("reg%c %02x\n", offset+'5', data);
}

WRITE8_MEMBER(k053251_device::regb_w)
{
	data &= 0x3f;
	logerror("reg%c %02x\n", offset+'b', data);
}

k053251_device::k053251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K053251, "K053251 Priority Encoder", tag, owner, clock, "k053251", __FILE__),
	//m_dirty_tmap[5],
	//m_ram[16],
	m_tilemaps_set(0)
	//m_palette_index[5]
{
}

void k053251_device::set_shadow_layer(int layer)
{
	m_shadow_layer = layer;
}

WRITE8_MEMBER(k053251_device::inpri_w)
{
	data &= 0x3f;
	m_inpri[offset] = data;
	logerror("layer %d inpri %02x\n", offset, data);
}

WRITE8_MEMBER(k053251_device::cblk_w)
{
	data &= 0x3f;
	m_cblk[offset] = data;
	if(offset)
		logerror("cblk 3=%d 4=%d\n", data & 7, data >> 3);
	else
		logerror("cblk 0=%d 1=%d 2=%d\n", data & 3, (data >> 2) & 3, (data >> 4) & 3);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k053251_device::device_start()
{
	m_init_cb.bind_relative_to(*owner());
	m_update_cb.bind_relative_to(*owner());
	memset(m_bitmaps, 0, sizeof(m_bitmaps));

	save_item(NAME(m_inpri));
	save_item(NAME(m_cblk));

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
	memset(m_inpri, 0, sizeof(m_inpri));
	memset(m_cblk, 0, sizeof(m_cblk));

	int i;

	m_tilemaps_set = 0;

	for (i = 0; i < 0x10; i++)
		m_ram[i] = 0;

	for (i = 0; i < 5; i++)
		m_dirty_tmap[i] = 0;

	reset_indexes();
}

void k053251_device::bitmap_update(bitmap_ind16 **bitmaps, const rectangle &cliprect)
{
	if(!m_bitmaps[0] || m_bitmaps[0]->width() != bitmaps[0]->width() || m_bitmaps[0]->height() != bitmaps[0]->height()) {
		if(m_bitmaps[0])
			for(int i=0; i<BITMAP_COUNT; i++)
				delete m_bitmaps[i];
		for(int i=0; i<BITMAP_COUNT; i++)
			m_bitmaps[i] = new bitmap_ind16(bitmaps[0]->width(), bitmaps[0]->height());
		if(!m_init_cb.isnull())
			m_init_cb(m_bitmaps);
	}

	m_update_cb(m_bitmaps, cliprect);

	uint16_t l0pal = (m_cblk[0] << 9) & 0x600;
	uint16_t l1pal = (m_cblk[0] << 7) & 0x600;
	uint16_t l2pal = (m_cblk[0] << 5) & 0x600;
	uint16_t l3pal = (m_cblk[1] << 8) & 0x700;
	uint16_t l4pal = (m_cblk[1] << 5) & 0x700;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++) {
		const uint16_t *c0 = &m_bitmaps[LAYER0_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *c1 = &m_bitmaps[LAYER1_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *c2 = &m_bitmaps[LAYER2_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *c3 = &m_bitmaps[LAYER3_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *c4 = &m_bitmaps[LAYER4_COLOR]->pix16(y, cliprect.min_x);
		const uint16_t *a0 = &m_bitmaps[LAYER0_ATTR ]->pix16(y, cliprect.min_x);
		const uint16_t *a1 = &m_bitmaps[LAYER1_ATTR ]->pix16(y, cliprect.min_x);
		const uint16_t *a2 = &m_bitmaps[LAYER2_ATTR ]->pix16(y, cliprect.min_x);
		uint16_t *dc = &bitmaps[kvideodac_device::BITMAP_COLOR]->pix16(y, cliprect.min_x);
		uint16_t *da = &bitmaps[kvideodac_device::BITMAP_ATTRIBUTES]->pix16(y, cliprect.min_x);

		const uint16_t *const *shadp = m_shadow_layer == LAYER0_ATTR ? &a0 : m_shadow_layer == LAYER1_ATTR ? &a1 : &a2;

		for(int x = cliprect.min_x; x <= cliprect.max_x; x++) {
			uint8_t pri = 0x3f;
			uint16_t attr = 0x0000, color = 0x0000;
			uint16_t cc, ca;
			uint16_t shada = **shadp;

			cc = *c0++ & 0x1ff;
			ca = *a0++;
			if(1)			if(cc & 0xf) {
				uint8_t lpri = ca & 0x3f;
				attr = 0x8000;
				color = cc | l0pal;
				pri = lpri;
			}

			cc = *c1++ & 0x1ff;
			ca = *a1++;
			if(1)			if(cc & 0xf) {
				uint8_t lpri = m_inpri[1];
				if(!attr || lpri < pri) {
					attr = 0x8000;
					color = cc | l1pal;
					pri = lpri;
				}
			}

			cc = *c2++ & 0x0ff;
			ca = *a2++;
			if(1)			if(cc & 0xf) {
				uint8_t lpri = m_inpri[2];
				if(!attr || lpri < pri) {
					attr = 0x8000;
					color = cc | l2pal;
					pri = lpri;
				}
			}

			cc = *c3++ & 0x0ff;
			if(1)			if(cc & 0xf) {
				uint8_t lpri = m_inpri[3];
				if(!attr || lpri < pri) {
					attr = 0x8000;
					color = cc | l3pal;
					pri = lpri;
				}
			}

			cc = *c4++ & 0x0ff;
			if(1)			if(cc & 0xf) {
				uint8_t lpri = m_inpri[4];
				if(!attr || lpri < pri) {
					attr = 0x8000;
					color = cc | l4pal;
					pri = lpri;
				}
			}
			*dc++ = color;
			*da++ = attr | pri | ((shada & 3) << 8);
		}
	}
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_MEMBER( k053251_device::write )
{
	int i, newind;
	logerror("write %x, %02x\n", offset, data);
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
