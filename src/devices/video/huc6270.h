// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6270 interface

    Pinouts, QFP64

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
	*2 SPO-SP3 are fetched during two consecutive character cycles.

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
	Bit       Description
	8 7654 3210
	0 ---- ---- 0 for background
	- xxxx ---- CG Color (when VD0-VD3 != 0)
	- ---- x--- CG3 (or 0 for 4 clocks, CG0 selected)
	- ---- -x-- CG2 (or 0 for 4 clocks, CG0 selected)
	- ---- --x- CG1 (or 0 for 4 clocks, CG1 selected)
	- ---- ---x CG0 (or 0 for 4 clocks, CG1 selected)
	- ---- 0000 Transparency, bit 7-4 is forced to 0

	Sprite:
	Bit       Description
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

**********************************************************************/

#ifndef MAME_VIDEO_HUC6270_H
#define MAME_VIDEO_HUC6270_H

#pragma once


class huc6270_device : public device_t
{
public:
	// construction/destruction
	huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_vram_size(uint32_t vram_size) { m_vram_size = vram_size; }
	auto irq() { return m_irq_changed_cb.bind(); }

	// 8 bit I/O
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	// TODO: 16 bit I/O used?
	/*
	u16 read16(offs_t offset);
	void write16(offs_t offset, u16 data);
	*/
	// VD0-7, SPBG output 
	u16 next_pixel();
	inline u16 time_until_next_event()
	{
		return m_horz_to_go * 8 + m_horz_steps;
	}

	// /VSYNC pin related
	DECLARE_WRITE_LINE_MEMBER( vsync_changed );
	// /HSYNC pin related
	DECLARE_WRITE_LINE_MEMBER( hsync_changed );

	static const uint16_t HUC6270_SPRITE     = 0x0100;    // sprite colour information
	static const uint16_t HUC6270_BACKGROUND = 0x0000;    // background colour information

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	inline void fetch_bat_tile_row();
	void add_sprite( int index, int x, int pattern, int line, int flip_x, int palette, int priority, int sat_lsb );
	void select_sprites();
	inline void handle_vblank();
	inline void next_vert_state();
	inline void next_horz_state();
	inline void handle_dma();

private:
	enum class v_state : u8 {
		VSW,
		VDS,
		VDW,
		VCR
	};

	enum class h_state : u8 {
		HDS,
		HDW,
		HDE,
		HSW
	};


	/* Size of Video ram (mandatory) */
	uint32_t m_vram_size;

	/* Callback for when the irq line may have changed (mandatory) */
	devcb_write_line    m_irq_changed_cb;

	uint8_t   m_register_index;

	/* HuC6270 registers */
	uint16_t  m_mawr;
	uint16_t  m_marr;
	uint16_t  m_vrr;
	uint16_t  m_vwr;
	uint16_t  m_cr;
	uint16_t  m_rcr;
	uint16_t  m_bxr;
	uint16_t  m_byr;
	uint16_t  m_mwr;
	uint16_t  m_hsr;
	uint16_t  m_hdr;
	uint16_t  m_vpr;
	uint16_t  m_vdw;
	uint16_t  m_vcr;
	uint16_t  m_dcr;
	uint16_t  m_sour;
	uint16_t  m_desr;
	uint16_t  m_lenr;
	uint16_t  m_dvssr;
	uint8_t   m_status;

	/* To keep track of external hsync and vsync signals */
	int m_hsync;
	int m_vsync;

	/* internal variables */
	v_state m_vert_state;
	h_state m_horz_state;
	int m_vd_triggered;
	int m_vert_to_go;
	int m_horz_to_go;
	int m_horz_steps;
	int m_raster_count;
	int m_dvssr_written;
	int m_satb_countdown;
	int m_dma_enabled;
	uint16_t m_byr_latched;
	uint16_t m_bxr_latched;
	uint16_t m_bat_address;
	uint16_t m_bat_address_mask;
	uint16_t m_bat_row;
	uint16_t m_bat_column;
	uint8_t m_bat_tile_row[8];
	/* Internal sprite attribute table. SATB DMA is used to transfer data
	   from VRAM to this internal table.
	*/
	uint16_t m_sat[4*64];
	int m_sprites_this_line;
	int m_sprite_row_index;
	uint16_t  m_sprite_row[1024];
	std::unique_ptr<uint16_t[]>  m_vram;
	uint16_t  m_vram_mask;

	static constexpr uint8_t vram_increments[4] = { 1, 32, 64, 128 };
};


DECLARE_DEVICE_TYPE(HUC6270, huc6270_device)

#endif // MAME_VIDEO_HUC6270_H
