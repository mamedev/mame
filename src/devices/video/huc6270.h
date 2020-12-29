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
         /IRQ -|                                               |-SP8G
        /BUSY -|                                               |-DISP
           A0 -|                                               |-/HSYN
           A1 -|                                               |-/VSYN
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
    /HSYN: Horizontal sync
    /VSYN: Vertical sync
    MA0-15: Memory address
    MD0-15: Memory data
    VD0-7: Pixel data output
    SP8G: Sprite/Background flag, Also highest bit of pixel data output
    A0-1: Host interface address
    D0-15: Host interface data
    8/16: 8 or 16 bit host interface mode?
    DISP: Unknown, display related?

**********************************************************************/

#ifndef MAME_VIDEO_HUC6270_H
#define MAME_VIDEO_HUC6270_H

#pragma once


class huc6270_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_irq_changed_cb.bind(); }

	// 8 bit I/O
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	// TODO: 16 bit I/O used?
	/*
	u16 read16(offs_t offset);
	void write16(offs_t offset, u16 data);
	*/
	// VD0-7, SP8G output 
	u16 next_pixel();
	inline u16 time_until_next_event()
	{
		return m_horz_to_go * 8 + m_horz_steps;
	}

	// /VSYN pin
	DECLARE_WRITE_LINE_MEMBER( vsync_changed );
	// /HSYN pin
	DECLARE_WRITE_LINE_MEMBER( hsync_changed );

	static const uint16_t HUC6270_SPRITE     = 0x0100;    // sprite colour information
	static const uint16_t HUC6270_BACKGROUND = 0x0000;    // background colour information

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_vram_space_config;

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


	/* VRAM space */
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_vram_cache;

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

	static constexpr uint8_t vram_increments[4] = { 1, 32, 64, 128 };
};


DECLARE_DEVICE_TYPE(HUC6270, huc6270_device)

#endif // MAME_VIDEO_HUC6270_H
