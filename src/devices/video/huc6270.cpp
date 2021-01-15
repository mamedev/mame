// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    NEC HuC6270 Video Display Controller

    The HuC6270 basically outputs a 9-bit stream of pixel data which
    holds a color index, a palette index, and an indication whether
    the pixel contains background data or from sprite data.

    This data can be used by a colour encoder to output graphics.

    A regular screen is displayed as follows:

        |<- HDS ->|<--       HDW       -->|<- HDE ->|<- HSW ->|
        |---------|-----------------------|---------|---------|
    VSW |                                                     |
        |---------|-----------------------|---------|---------|
    VDS |                                                     |
        |                  overscan                           |
        |---------|-----------------------|---------|---------|
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
    VDW | overscan|    active display     |      overscan     |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |         |                       |                   |
        |---------|-----------------------|---------|---------|
    VCR |                  overscan                           |
        |                                                     |
        |---------|-----------------------|---------|---------|
        ^end hsync
         ^start vsync (30 cycles after hsync)

Hardware notes:

    Pinouts (QFP80)

                                  )
                                  D
                                  N
                                  G
                                  (
                            5 4 3 3 2 1 0         3
                5 4 3 2 1 0 1 1 1 s 1 1 1 9 8 7 6 d 5 4 3 2 1 0
                A A A A A A D D D s D D D D D D D d D D D D D D
                M M M M M M M M M V M M M M M M M V M M M M M M
                | | | | | | | | | | | | | | | | | | | | | | | |
               /-----------------------------------------------\
          MA6 -|                                               |-/MRD
          MA7 -|                                               |-/MWR
          MA8 -|                                               |-VD0
          MA9 -|                                               |-VD1
         MA10 -|                                               |-VD2
         MA11 -|                                               |-VD3
    Vss4(GND) -|                                               |-VD4
         Vdd4 -|                    HuC6270                    |-Vss2(GND)
         MA12 -|                                               |-Vdd2
         MA13 -|                                               |-VD5
         MA14 -|                                               |-VD6
         MA15 -|                                               |-VD7
         /IRQ -|                                               |-SPBG
        /BUSY -|                                               |-DISP
           A0 -|                                               |-/HSYNC
           A1 -|                                               |-/VSYNC
               \-----------------------------------------------/
                | | | | | | | | | | | | | | | | | | | | | | | |
                / / / D D D D D D D V D D D D D D V D D D 8 C /
                C W R 1 1 1 1 1 1 9 s 8 7 6 5 4 3 d 2 1 0 / K R
                S R D 5 4 3 2 1 0   s             d       1   E
                                    1             1       6   S
                                    (                         E
                                    G                         T
                                    N
                                    D
                                    )

    CK: Input clock
    /RESET: Reset pin
    /CS: Chip select
    /WR: Write enable
    /RD: Read enable
    /IRQ: Interrupt output to host CPU
    /BUSY: Busy flag
    /MRD: Memory read enable
    /MWR: Memory write enable
    /HSYNC: Horizontal sync
    /VSYNC: Vertical sync
    DISP: Programmable; See below
    MA0-15: Memory address
    MD0-15: Memory data
    VD0-7: Pixel data output
    SPBG: Sprite/Background flag, Also highest bit of pixel data output
    A0-1: Host interface address
    D0-15: Host interface data
    8/16: 8 or 16 bit host interface mode?

	Register format
		Bit               Description
		fedcba98 76543210
	00 AR Register select (W)
		-------- ---xxxxx Register select

	00 SR Status (R)
		-------- -x------ BUSY (when VDC accessed VRAM)
		-------- --x----- VD (Vblank flag)
		-------- ---x---- DV (VRAM-VRAM DMA done)
		-------- ----x--- DS (VRAM-SATB DMA done)
		-------- -----x-- RR (Scanline interrupt flag)
		-------- ------x- OV (Sprite overflow flag)
		-------- -------x CR (Collision flag)

	02 Register data LSB
	03 Register data MSB
	MAWR Memory Address Write (00h W)
		xxxxxxxx xxxxxxxx Memory address for write

	MARR Memory Address Read (01h W)
		xxxxxxxx xxxxxxxx Memory address for read

	VWR Memory Data Write (02h W)
		xxxxxxxx xxxxxxxx data to memory

	VRR Memory Data Read (02h R)
		xxxxxxxx xxxxxxxx data from memory

	03h Reserved

	04h Reserved

	CR Control register (05h W)
		---xx--- -------- IW (VRAM address increment value)
		---00--- -------- +1
		---01--- -------- +20h
		---10--- -------- +40h
		---11--- -------- +80h
		-----x-- -------- DR (Dynamic RAM refresh)
		------xx -------- TE (DISP pin output select)
		------00 -------- H during Display
		------01 -------- L when Indicates the position in which Color Burst is inserted
		------10 -------- Internal horizontal sync
		------11 -------- Invaild
		-------- x------- BB (Background enable(1) / disable(0))
		-------- -x------ SB (Sprite enable(1) / disable(0))
		-------- 00------ Burst mode
		-------- --xx---- EX (External sync mode)
		-------- --00---- Both /VSYNC and /HSYNC are input
		-------- --01---- /VSYNC input, /HSYNC output
		-------- --10---- Invaild
		-------- --11---- Both /VSYNC and /HSYNC are output
		-------- ----xxxx IE (Interrupt enable)
		-------- ----x--- Vertical blank interrupt
		-------- -----x-- Scanline interrupt
		-------- ------x- Sprite overflow interrupt
		-------- -------x Collision interrupt

	RCR Scanline interrupt position (06h W)
		------xx xxxxxxxx Scanline interrupt position

	BXR Background X scroll (07h W)
		------xx xxxxxxxx Background X

	BYR Background Y scroll (08h W)
		-------x xxxxxxxx Background Y

	MWR Memory access width (09h W)
		-------- x------- CM (Fetch CG block in next scanline for 4 clock mode)
		-------- 0------- (CG0)CH0, CH1
		-------- 1------- (CG1)CH2, CH3
		-------- -xxx---- SCREEN (Background layer size)
		-------- -0------ 32 Tile height (256 pixel)
		-------- -1------ 64 Tile height (512 pixel)
		-------- --00---- 32 Tile width (256 pixel)
		-------- --01---- 64 Tile width (512 pixel)
		-------- --1----- 128 Tile width (1024 pixel)
		-------- ----xx-- SM (Sprite access width mode, see below)
		-------- ------xx VM (VRAM access width mode, see below)

	HSR Horizontal sync (0Ah W)
		-xxxxxxx -------- HDS (Horizontal display start position - 1 * 8)
		-------- ---xxxxx HSW (Horizontal sync pulse width - 1 * 8)

	HDR Horizontal display (0Bh W)
		-xxxxxxx -------- HDW (Horizontal display width - 1 * 8)
		-------- -xxxxxxx HDE (Horizontal display end position - 1 * 8)

	VPR Vertical sync (0Ch W)
		xxxxxxxx -------- VDS (Vertical display start position - 2)
		-------- ---xxxxx VSW (Vertical sync pulse width - 1)

	VDR Vertical display width (0Dh W)
		-------x xxxxxxxx VDW (Vertical display width - 1)

	VCR Vertical display end position (0Eh W)
		-------- xxxxxxxx VCR (Vertical display end position)

	DCR DMA control registers (0Fh W)
		-------- ---x---- DSR (Repeat VRAM-SATB DMA in every VBlanks)
		-------- ----x--- DI/D (Destination address increment(0) / decrement(1))
		-------- -----x-- SI/D (Source address increment(0) / decrement(1))
		-------- ------x- DVC (VRAM-VRAM DMA interrupt enable)
		-------- -------x DSC (VRAM-SATB DMA interrupt enable)

	SOUR VRAM-VRAM DMA source address (10h W)
		xxxxxxxx xxxxxxxx SOUR (VRAM-VRAM DMA source address)

	DESR VRAM-VRAM DMA destination address (11h W)
		xxxxxxxx xxxxxxxx DESR (VRAM-VRAM DMA destination address)

	LENR VRAM-VRAM DMA length counter (12h W)
		xxxxxxxx xxxxxxxx LENR (VRAM-VRAM DMA length counter - 1)

	DVSSR VRAM-SATB DMA source address (13h W)
		xxxxxxxx xxxxxxxx DVSSR (VRAM-SATB DMA source address)

	SM assignment
	+-----------+--------------+-----------------------------------------------+
	|  SM bit   |              |      Assignment for one character cycle       |
	+-----+-----+ Access width +-----+-----+-----+-----+-----+-----+-----+-----+
	|  3  |  2  |              |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  0  |  0  |      1       | SP0 | SP1 | SP2 | SP3 | SP0 | SP1 | SP2 | SP3 |
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  0  |  1  |      2       |  SP0/SP2  |  SP1/SP3  |  SP0/SP2  |  SP1/SP3  |*1
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  0  |      2       |    SP0    |    SP1    |    SP2    |    SP3    |
	+-----+-----+--------------+-----------+-----------+-----------+-----------+
	|  1  |  1  |      4       |        SP0, SP2       |        SP1, SP3       |*2
	+-----+-----+--------------+-----------------------+-----------------------+
	*1 LSB of pattern select bit is select (SP0, SP1) or (SP2, SP3).
	*2 SP0-SP3 are fetched during two consecutive character cycles.

	VM assignment
	+-----------+--------------+-----------------------------------------------+
	|  VM bit   |              |  Assignment for one character cycle (8 dot)   |
	+-----+-----+ Access width +-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  0  |              |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  0  |  0  |      1       | CPU | BAT | CPU |  -  | CPU | CG0 | CPU | CG1 |
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  0  |  1  |      2       |    BAT    |    CPU    |    CG0    |    CG1    |
	+-----+-----+--------------+-----+-----+-----+-----+-----+-----+-----+-----+
	|  1  |  0  |      2       |    BAT    |    CPU    |    CG0    |    CG1    |
	+-----+-----+--------------+-----------+-----------+-----------+-----------+
	|  1  |  1  |      4       |          BAT          |       CG0 or CG1      |
	+-----+-----+--------------+-----------------------+-----------------------+

	BAT data (single word per tile)
	Bit               Description
	fedcba98 76543210
	xxxx---- -------- CG color
	----xxxx xxxxxxxx Character code select (16 word each)

	SAT data (4 word per each sprites)
	Offset Bit               Description
	       fedcba98 76543210
	00     ------xx xxxxxxxx Y coordinate - 64
	01     ------xx xxxxxxxx X coordinate - 32
	02     -----xxx xxxxxxxx PC (Pattern code, 64 word each)
	       -------- -------x SG0/SG1, SG2/SG3 select, for 4 color mode
	03     x------- -------- Flip Y
	       --xx---- -------- CGY (Sprite height)
		   --00---- -------- 1 tile (16 pixel)
		   --01---- -------- 2 tiles (32 pixel)
		   --10---- -------- Not used
		   --11---- -------- 4 tiles (64 pixel)
		   ----x--- -------- Flip X
		   -------x -------- CGX (Sprite width)
		   -------0 -------- 1 tile (16 pixel)
		   -------1 -------- 2 tiles (32 pixel)
		   -------- x------- SPBG (Sprite VS Background priority)
		   -------- 0------- Behind background
		   -------- 1------- Above background
		   -------- ----xxxx SPRITE COLOR

	VD0-7, SPBG output
	Background:
	Bit         Description
	8 7654 3210
	0 ---- ---- 0 for background
	- xxxx ---- CG Color (when VD0-VD3 != 0)
	- ---- x--- CG3 (or 0 for 4 clocks, CG0 selected)
	- ---- -x-- CG2 (or 0 for 4 clocks, CG0 selected)
	- ---- --x- CG1 (or 0 for 4 clocks, CG1 selected)
	- ---- ---x CG0 (or 0 for 4 clocks, CG1 selected)
	- ---- 0000 Transparency, bit 7-4 is forced to 0

	Sprite:
	Bit         Description
	8 7654 3210
	1 ---- ---- 1 for sprite
	- xxxx ---- Sprite Color (when VD0-VD3 != 0)
	- ---- x--- SG3
	- ---- -x-- SG2
	- ---- --x- SG1
	- ---- ---x SG0
	- ---- 0000 Transparency, bit 7-4 is forced to 0

	Border:
	1 0000 0000 Border color, during blanking period


KNOWN ISSUES
  - Violent Soldier (probably connected):
    - In the intro some artefacts appear at the top of the
      screen every now and then.
  - In ccovell's splitres test not all sections seem to be aligned properly.
  - Side Arms: Seems to be totally broken.


TODO
  - Fix timing of VRAM-SATB DMA
  - Implement VRAM-VRAM DMA
  - DMA speeds differ depending on the dot clock selected in the huc6270
  - Convert VRAM bus to actual space address (optimization)

**********************************************************************/

#include "emu.h"
#include "huc6270.h"

//#define VERBOSE 1
#include "logmacro.h"


enum {
	MAWR = 0x00,
	MARR = 0x01,
	VxR = 0x02,
	CR = 0x05,
	RCR = 0x06,
	BXR = 0x07,
	BYR = 0x08,
	MWR = 0x09,
	HSR = 0x0A,
	HDR = 0x0B,
	VPR = 0x0C,
	VDW = 0x0D,
	VCR = 0x0E,
	DCR = 0x0F,
	SOUR = 0x10,
	DESR = 0x11,
	LENR = 0x12,
	DVSSR = 0x13
};

ALLOW_SAVE_TYPE(huc6270_device::v_state);
ALLOW_SAVE_TYPE(huc6270_device::h_state);


/* Bits in the VDC status register */
#define HUC6270_BSY         0x40    /* Set when the VDC accesses VRAM */
#define HUC6270_VD          0x20    /* Set when in the vertical blanking period */
#define HUC6270_DV          0x10    /* Set when a VRAM > VRAM DMA transfer is done */
#define HUC6270_DS          0x08    /* Set when a VRAM > SATB DMA transfer is done */
#define HUC6270_RR          0x04    /* Set when the current scanline equals the RCR register */
#define HUC6270_OR          0x02    /* Set when there are more than 16 sprites on a line */
#define HUC6270_CR          0x01    /* Set when sprite #0 overlaps with another sprite */


DEFINE_DEVICE_TYPE(HUC6270, huc6270_device, "huc6270", "Hudson HuC6270 VDC")


constexpr uint8_t huc6270_device::vram_increments[4];

huc6270_device::huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HUC6270, tag, owner, clock)
	, m_vram_size(0)
	, m_irq_changed_cb(*this)
{
}


/*
  Read one row of tile data from video ram
*/
inline void huc6270_device::fetch_bat_tile_row()
{
	const uint16_t bat_data = m_vram[m_bat_address & m_vram_mask];
	const uint16_t tile_palette = (bat_data >> 8) & 0xF0;
	// background bpp can be selected; low 2bit, high 2bit, or 4bit - verified from manual
	uint16_t data1 = ((m_mwr & 0x83) == 0x83) ? 0 : m_vram[(((bat_data & 0x0FFF) << 4) + m_bat_row + 0) & m_vram_mask]; // CG0
	uint16_t data2 = (data1 >> 7) & 0x1FE;
	uint16_t data3 = ((m_mwr & 0x83) == 0x03) ? 0 : m_vram[(((bat_data & 0x0FFF) << 4) + m_bat_row + 8) & m_vram_mask]; // CG1
	uint16_t data4 = (data3 >> 5) & 0x7F8;
	data3 <<= 2;

	for (int i = 7; i >= 0; i--)
	{
		uint16_t c = (data1 & 0x01) | (data2 & 0x02) | (data3 & 0x04) | (data4 & 0x08);

		/* Colour 0 for background tiles is always taken from palette 0 */
		if (c)
			c |= tile_palette;

		m_bat_tile_row[i] = c;

		data1 >>= 1;
		data2 >>= 1;
		data3 >>= 1;
		data4 >>= 1;
	}
}


void huc6270_device::add_sprite(int index, int x, int pattern, int line, int flip_x, int palette, int priority, int sat_lsb)
{
	int i = m_sprites_this_line;

	if (i < 16)
	{
		uint32_t b0, b1, b2, b3;
		int j;

		if (flip_x)
			flip_x = 0x0F;

		pattern += ((line >> 4) << 1);

		if ((m_mwr & 0x0c) == 0x04)
		{
			if (!sat_lsb)
			{
				b0 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x00) & m_vram_mask];
				b1 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x10) & m_vram_mask] << 1;
			}
			else
			{
				b0 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x20) & m_vram_mask];
				b1 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x30) & m_vram_mask] << 1;
			}
			b2 = 0;
			b3 = 0;
		}
		else
		{
			b0 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x00) & m_vram_mask];
			b1 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x10) & m_vram_mask] << 1;
			b2 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x20) & m_vram_mask] << 2;
			b3 = m_vram[((pattern * 0x40) + (line & 0x0F) + 0x30) & m_vram_mask] << 3;
		}

		for (j = 15; j >= 0; j--)
		{
			uint8_t data = (b3 & 0x08) | (b2 & 0x04) | (b1 & 0x02) | (b0 & 0x01);

			if (data)
			{
				data |= palette << 4;

				if (x + (j ^ flip_x) < 1024)
				{
					if (!m_sprite_row[x + (j ^ flip_x)])
					{
						m_sprite_row[x + (j ^ flip_x)] = (priority ? 0x4000 : 0x0000) | (index << 8) | data;
					}
					else
					{
						if (!(m_sprite_row[x + (j ^ flip_x)] & 0xFF00))
						{
							/* Sprite 0 collission */
							m_sprite_row[x + (j ^ flip_x)] |= 0x8000;
						}
					}
				}
			}

			b0 >>= 1;
			b1 >>= 1;
			b2 >>= 1;
			b3 >>= 1;
		}

		m_sprites_this_line += 1;
	}
}


void huc6270_device::select_sprites()
{
	int i;

	m_sprites_this_line = 0;
	memset(m_sprite_row, 0, sizeof(m_sprite_row));
	m_sprite_row_index = 0x20;

	for (i = 0; i < 4 * 64; i += 4)
	{
		static const int cgy_table[4] = { 16, 32, 64, 64 };
		int cgy = (m_sat[i+3] >> 12) & 0x03;
		int height = cgy_table[cgy];
		int sprite_line = m_raster_count - m_sat[i];

		if (sprite_line >= 0 && sprite_line < height)
		{
			int pattern = m_sat[i+2] >> 1;
			int sat_lsb = m_sat[i+2] & 0x01;
			int palette = m_sat[i+3] & 0x0F;
			int priority = m_sat[i+3] & 0x80;
			int cgx = m_sat[i+3] & 0x0100;

			/* If CGY is set to 1, bit 1 of the sprite pattern index is forced to 0 */
			if (cgy & 1)
				pattern &= ~0x0002;

			/* If CGY is set to 2 or 3, bits 1 and 2 of the sprite pattern index are forced to 0 */
			if (cgy & 2)
				pattern &= ~0x0006;

			/* Recalculate line index when sprite is flipped vertically */
			if (m_sat[i+3] & 0x8000)
				sprite_line = (height - 1) - sprite_line;

			/* Is the sprite 32 pixels wide */
			if (cgx)
			{
				/* If CGX is set, bit 0 of the sprite pattern index is forced to 0 */
				pattern &= ~0x0001;

				/* Check for horizontal flip */
				if (m_sat[i+3] & 0x0800)
				{
					/* Add to our list of sprites for this line */
					add_sprite(i/4, m_sat[i+1], pattern + 1, sprite_line, 1, palette, priority, sat_lsb);
					add_sprite(i/4, m_sat[i+1] + 16, pattern, sprite_line, 1, palette, priority, sat_lsb);
				}
				else
				{
					/* Add to our list of sprites for this line */
					add_sprite(i/4, m_sat[i+1], pattern, sprite_line, 0, palette, priority, sat_lsb);
					add_sprite(i/4, m_sat[i+1] + 16, pattern + 1, sprite_line, 0, palette, priority, sat_lsb);
				}
			}
			else
			{
				/* Add to our list of sprites for this line */
				add_sprite(i/4, m_sat[i+1], pattern, sprite_line, m_sat[i+3] & 0x0800, palette, priority, sat_lsb);
			}
		}
	}

	/* Check for sprite overflow */
	if (m_sprites_this_line >= 16)
	{
		/* note: flag is set only if irq is taken, Mizubaku Daibouken relies on this behaviour */
		if (m_cr & 0x02)
		{
			m_status |= HUC6270_OR;
			m_irq_changed_cb(ASSERT_LINE);
		}
	}
}


inline void huc6270_device::handle_vblank()
{
	if (!m_vd_triggered)
	{
		if (m_cr & 0x08)
		{
			m_status |= HUC6270_VD;
			m_irq_changed_cb(ASSERT_LINE);
		}

		/* Should we initiate a VRAM->SATB DMA transfer.
		   The timing for this is incorrect.
		 */
		if (m_dvssr_written || (m_dcr & 0x10))
		{
			int i;

			LOG("SATB transfer from %05x\n", m_dvssr << 1);
			for (i = 0; i < 4 * 64; i += 4)
			{
				m_sat[i + 0] = m_vram[(m_dvssr + i + 0) & m_vram_mask] & 0x03FF;
				m_sat[i + 1] = m_vram[(m_dvssr + i + 1) & m_vram_mask] & 0x03FF;
				m_sat[i + 2] = m_vram[(m_dvssr + i + 2) & m_vram_mask] & 0x07FF;
				m_sat[i + 3] = m_vram[(m_dvssr + i + 3) & m_vram_mask];
			}
			m_dvssr_written = 0;

			/* Generate SATB interrupt if requested */
			if (m_dcr & 0x01)
			{
				m_satb_countdown = 4;
//                  m_status |= HUC6270_DS;
//                  m_irq_changed_cb(ASSERT_LINE);
			}
		}

		m_vd_triggered = 1;
	}
}


inline void huc6270_device::next_vert_state()
{
	switch (m_vert_state)
	{
	case v_state::VSW:
		m_vert_state = v_state::VDS;
		m_vert_to_go = ((m_vpr >> 8) & 0xFF) + 2;
		break;

	case v_state::VDS:
		m_vert_state = v_state::VDW;
		m_vert_to_go = (m_vdw & 0x1FF) + 1;
		m_byr_latched = m_byr;
		m_vd_triggered = 0;
		break;

	case v_state::VDW:
		m_vert_state = v_state::VCR;
		m_vert_to_go = (m_vcr & 0xFF);
		handle_vblank();
		break;

	case v_state::VCR:
		m_vert_state = v_state::VSW;
		m_vert_to_go = (m_vpr & 0x1F) + 1;
		break;
	}
}


inline void huc6270_device::next_horz_state()
{
	switch (m_horz_state)
	{
	case h_state::HDS:
		m_bxr_latched = m_bxr;
		m_horz_state = h_state::HDW;
		m_horz_to_go = (m_hdr & 0x7F) + 1;
		{
			static const int width_shift[4] = { 5, 6, 7, 7 };
			uint16_t v;

			v = (m_byr_latched) & ((m_mwr & 0x40) ? 0x1FF : 0xFF);
			m_bat_row = v & 7;
			m_bat_address_mask = (1 << width_shift[(m_mwr >> 4) & 0x03]) - 1;
			m_bat_address = ((v >> 3) << (width_shift[(m_mwr >> 4) & 0x03]))
				| ((m_bxr_latched >> 3) & m_bat_address_mask);
			m_bat_column = m_bxr & 7;
			fetch_bat_tile_row();
		}
		break;

	case h_state::HDW:
		m_horz_state = h_state::HDE;
		m_horz_to_go = ((m_hdr >> 8) & 0x7F) + 1;
		break;

	case h_state::HDE:
		m_horz_state = h_state::HSW;
		m_horz_to_go = (m_hsr & 0x1F) + 1;
		break;

	case h_state::HSW:
		m_horz_state = h_state::HDS;
		m_horz_to_go = std::max(((m_hsr >> 8) & 0x7F), 2) + 1;

		/* If section has ended, advance to next vertical state */
		while (m_vert_to_go == 0)
			next_vert_state();

		/* Select sprites for the coming line */
		select_sprites();
		break;
	}
	m_horz_steps = 0;
}


u16 huc6270_device::next_pixel()
{
	uint16_t data = HUC6270_SPRITE;

	/* Check if we're on an active display line */
	if (m_vert_state == v_state::VDW)
	{
		/* Check if we're in active display area */
		if (m_horz_state == h_state::HDW)
		{
			uint8_t sprite_data = m_sprite_row[m_sprite_row_index] & 0x00FF;
			int collission = (m_sprite_row[m_sprite_row_index] & 0x8000) ? 1 : 0;

			if (m_cr & 0x80)
			{
				data = HUC6270_BACKGROUND | m_bat_tile_row[m_bat_column];
				if (sprite_data && (m_cr & 0x40))
				{
					if (m_sprite_row[m_sprite_row_index] & 0x4000)
					{
						data = HUC6270_SPRITE | sprite_data;
					}
					else
					{
						if (data == HUC6270_BACKGROUND)
						{
							data = HUC6270_SPRITE | sprite_data;
						}
					}
				}
			}
			else
			{
				if (m_cr & 0x40)
				{
					data = HUC6270_SPRITE | sprite_data;
				}
			}

			m_sprite_row_index = m_sprite_row_index + 1;
			m_bat_column += 1;
			if (m_bat_column >= 8)
			{
				m_bat_address = (m_bat_address & ~m_bat_address_mask)
					| ((m_bat_address + 1) & m_bat_address_mask);
				m_bat_column = 0;
				fetch_bat_tile_row();
			}

			if (collission && (m_cr & 0x01))
			{
				m_status |= HUC6270_CR;
				m_irq_changed_cb(ASSERT_LINE);
			}
		}
	}

	m_horz_steps++;
	if (m_horz_steps == 8)
	{
		m_horz_to_go -= 1;
		m_horz_steps = 0;
		while (m_horz_to_go == 0)
			next_horz_state();
	}
	return data;
}


//inline u16 huc6270_device::time_until_next_event()
//{
//  return m_horz_to_go * 8 + m_horz_steps;
//}


WRITE_LINE_MEMBER(huc6270_device::vsync_changed)
{
	state &= 0x01;
	if (m_vsync != state)
	{
		/* Check for high->low VSYNC transition */
		if (!state)
		{
			m_vert_state = v_state::VCR;
			m_vert_to_go = 0;

			while (m_vert_to_go == 0)
				next_vert_state();
		}
		else
		{
			/* Check for low->high VSYNC transition */
			// VBlank IRQ happens at the beginning of HDW period after VDW ends
			handle_vblank();
		}
	}

	m_vsync = state;
}


WRITE_LINE_MEMBER(huc6270_device::hsync_changed)
{
	state &= 0x01;

	if (m_hsync != state)
	{
		/* Check for low->high HSYNC transition */
		if (state)
		{
			if (m_satb_countdown)
			{
				m_satb_countdown--;

				if (m_satb_countdown == 0)
				{
					m_status |= HUC6270_DS;
					m_irq_changed_cb(ASSERT_LINE);
				}
			}

			m_horz_state = h_state::HSW;
			m_horz_to_go = 0;
			m_horz_steps = 0;
			m_byr_latched += 1;
			m_raster_count += 1;
			if (m_vert_to_go == 1 && m_vert_state == v_state::VDS)
			{
				m_raster_count = 0x40;
			}

			m_vert_to_go -= 1;

			while (m_horz_to_go == 0)
				next_horz_state();

			handle_dma();
		}
		else
		{
			/* Check for high->low HSYNC transition */
			// RCR IRQ happens near the end of the HDW period
			if (m_raster_count == m_rcr && (m_cr & 0x04))
			{
				m_status |= HUC6270_RR;
				m_irq_changed_cb(ASSERT_LINE);
			}
		}
	}

	m_hsync = state;
}

inline void huc6270_device::handle_dma()
{
	/* Should we perform VRAM-VRAM dma.
	   The timing for this is incorrect.
	 */
	if (m_dma_enabled)
	{
		int desr_inc = (m_dcr & 0x0008) ? -1 : +1;
		int sour_inc = (m_dcr & 0x0004) ? -1 : +1;

		LOG("doing dma sour = %04x, desr = %04x, lenr = %04x\n", m_sour, m_desr, m_lenr);

		do {
			uint16_t data;

			// area 0x8000-0xffff cannot be r/w (open bus)
			if (m_sour <= m_vram_mask)
				data = m_vram[m_sour];
			else
				data = 0;

			if (m_desr <= m_vram_mask)
				m_vram[m_desr] = data;
			m_sour += sour_inc;
			m_desr += desr_inc;
			m_lenr -= 1;
		} while (m_lenr != 0xFFFF);

		if (m_dcr & 0x0002)
		{
			m_status |= HUC6270_DV;
			m_irq_changed_cb(ASSERT_LINE);
		}
		m_dma_enabled = 0;
	}
}

u16 huc6270_device::read16(offs_t offset, u16 mem_mask)
{
	uint16_t data = 0x0000;

	switch (offset & 1)
	{
		case 0x00:  /* status */
			data = m_status;
			if (ACCESSING_BITS_0_7 && !machine().side_effects_disabled())
			{
				m_status &= ~(HUC6270_VD | HUC6270_DV | HUC6270_RR | HUC6270_CR | HUC6270_OR | HUC6270_DS);
				m_irq_changed_cb(CLEAR_LINE);
			}
			break;

		case 0x01:
			data = m_vrr;
			if (ACCESSING_BITS_8_15 && !machine().side_effects_disabled())
			{
				if (m_register_index == VxR)
				{
					m_marr += vram_increments[(m_cr >> 11) & 3];

					if (m_marr <= m_vram_mask)
						m_vrr = m_vram[m_marr];
					else
					{
						// TODO: test with real HW
						m_vrr = 0;
						logerror("%s Open Bus VRAM read (register read) %04x\n",this->tag(),m_marr);
					}
				}
			}
			break;
	}
	return data;
}


void huc6270_device::write16(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("%s: huc6270 write %02x <- %04x & %04x ", machine().describe_context(), offset, data, mem_mask);

	switch (offset & 1)
	{
		case 0x00:  /* VDC register select */
			if (ACCESSING_BITS_0_7)
				m_register_index = data & 0x1F;
			break;

		case 0x01:  /* VDC data */
			switch (m_register_index)
			{
				case MAWR:      /* memory address write register */
					COMBINE_DATA(&m_mawr);
					break;

				case MARR:      /* memory address read register */
					COMBINE_DATA(&m_marr);
					if (m_marr <= m_vram_mask)
						m_vrr = m_vram[m_marr];
					else
					{
						// TODO: test with real HW
						m_vrr = 0;
						logerror("%s Open Bus VRAM read (memory address) %04x\n",this->tag(),m_marr);
					}
					break;

				case VxR:       /* vram write data */
					COMBINE_DATA(&m_vwr);
					if (ACCESSING_BITS_8_15)
					{
						// area 0x8000-0xffff is NOP and cannot be written to.
						if (m_mawr <= m_vram_mask)
							m_vram[m_mawr] = m_vwr;
						m_mawr += vram_increments[(m_cr >> 11) & 3];
					}
					break;

				case CR:        /* control register */
					COMBINE_DATA(&m_cr);
					break;

				case RCR:       /* raster compare register */
					COMBINE_DATA(&m_rcr);
					m_rcr &= 0x3ff;
//printf("%s: RCR set to %03x\n", machine().describe_context().c_str(), m_rcr);
//                  if (m_raster_count == m_rcr && m_cr & 0x04)
//                  {
//                      m_status |= HUC6270_RR;
//                      m_irq_changed_cb(ASSERT_LINE);
//                  }
					break;

				case BXR:       /* background x-scroll register */
					COMBINE_DATA(&m_bxr);
					m_bxr &= 0x3ff;
					break;

				case BYR:       /* background y-scroll register */
					COMBINE_DATA(&m_byr);
					m_byr &= 0x1ff;
					m_byr_latched = m_byr;
					break;

				case MWR:       /* memory width register */
					COMBINE_DATA(&m_mwr);
					break;

				case HSR:       /* horizontal sync register */
					COMBINE_DATA(&m_hsr);
					break;

				case HDR:       /* horizontal display register */
					COMBINE_DATA(&m_hdr);
					break;

				case VPR:       /* vertical sync register */
					COMBINE_DATA(&m_vpr);
					break;

				case VDW:       /* vertical display register */
					COMBINE_DATA(&m_vdw);
					break;

				case VCR:       /* vertical display end position register */
					COMBINE_DATA(&m_vcr);
					break;

				case DCR:       /* DMA control register */
					COMBINE_DATA(&m_dcr);
					break;

				case SOUR:      /* DMA source address register */
					COMBINE_DATA(&m_sour);
					break;

				case DESR:      /* DMA destination address register */
					COMBINE_DATA(&m_desr);
					break;

				case LENR:      /* DMA length register */
					COMBINE_DATA(&m_lenr);
					if (ACCESSING_BITS_8_15)
						m_dma_enabled = 1;
//logerror("DMA is not supported yet.\n");
					break;

				case DVSSR:     /* Sprite attribute table */
					COMBINE_DATA(&m_dvssr);
					m_dvssr_written = 1;
					break;
			}
			break;
	}
	LOG("\n");
}

u8 huc6270_device::read8(offs_t offset)
{
	const int shift = ((offset & 1) << 3);
	return (read16(offset >> 1, 0xff << shift) >> shift) & 0xff;
}


void huc6270_device::write8(offs_t offset, u8 data)
{
	const int shift = ((offset & 1) << 3);
	write16(offset >> 1, u16(data) << shift, 0xff << shift);
}

void huc6270_device::device_start()
{
	/* Resolve callbacks */
	m_irq_changed_cb.resolve_safe();

	m_vram = make_unique_clear<uint16_t[]>(m_vram_size/sizeof(uint16_t));
	m_vram_mask = (m_vram_size >> 1) - 1;

	save_pointer(NAME(m_vram), m_vram_size/sizeof(uint16_t));

	save_item(NAME(m_register_index));
	save_item(NAME(m_mawr));
	save_item(NAME(m_marr));
	save_item(NAME(m_vrr));
	save_item(NAME(m_vwr));
	save_item(NAME(m_cr));
	save_item(NAME(m_rcr));
	save_item(NAME(m_bxr));
	save_item(NAME(m_byr));
	save_item(NAME(m_mwr));
	save_item(NAME(m_hsr));
	save_item(NAME(m_hdr));
	save_item(NAME(m_vpr));
	save_item(NAME(m_vdw));
	save_item(NAME(m_vcr));
	save_item(NAME(m_dcr));
	save_item(NAME(m_sour));
	save_item(NAME(m_desr));
	save_item(NAME(m_lenr));
	save_item(NAME(m_dvssr));
	save_item(NAME(m_status));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vsync));
	save_item(NAME(m_vert_state));
	save_item(NAME(m_horz_state));
	save_item(NAME(m_vd_triggered));
	save_item(NAME(m_vert_to_go));
	save_item(NAME(m_horz_to_go));
	save_item(NAME(m_horz_steps));
	save_item(NAME(m_raster_count));
	save_item(NAME(m_dvssr_written));
	save_item(NAME(m_satb_countdown));
	save_item(NAME(m_dma_enabled));
	save_item(NAME(m_byr_latched));
	save_item(NAME(m_bxr_latched));
	save_item(NAME(m_bat_address));
	save_item(NAME(m_bat_address_mask));
	save_item(NAME(m_bat_row));
	save_item(NAME(m_bat_column));
	save_item(NAME(m_bat_tile_row));
	save_item(NAME(m_sat));
	save_item(NAME(m_sprites_this_line));
	save_item(NAME(m_sprite_row_index));
	save_item(NAME(m_sprite_row));
}


void huc6270_device::device_reset()
{
	m_mawr = 0;
	m_marr = 0;
	m_vrr = 0;
	m_vwr = 0;
	m_cr = 0;
	m_rcr = 0;
	m_bxr = 0;
	m_byr = 0;
	m_mwr = 0;
	m_hsr = 0x0202;     /* Take some defaults for horizontal timing */
	m_hdr = 0x041f;
	m_vpr = 0x0f02;     /* Take some defaults for vertical timing */
	m_vdw = 0x00ef;
	m_vcr = 0x0004;
	m_dcr = 0;
	m_sour = 0;
	m_lenr = 0;
	m_dvssr = 0;
	m_status = 0;
	m_vd_triggered = 0;
	m_dvssr_written = 0;
	m_satb_countdown = 0;
	m_raster_count = 0x4000;
	m_vert_to_go = 0;
	m_vert_state = v_state::VSW;
	m_horz_steps = 0;
	m_horz_to_go = 0;
	m_horz_state = h_state::HDS;
	m_hsync = 0;
	m_vsync = 0;
	m_dma_enabled = 0;
	m_byr_latched = 0;

	memset(m_sat, 0, sizeof(m_sat));
}
