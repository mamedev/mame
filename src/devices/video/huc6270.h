// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6270 interface

**********************************************************************/

#ifndef MAME_VIDEO_HUC6270_H
#define MAME_VIDEO_HUC6270_H

#pragma once


class huc6270_device : public device_t
{
public:
	static constexpr u16 HUC6270_SPRITE     = 0x0100;    // sprite colour information
	static constexpr u16 HUC6270_BACKGROUND = 0x0000;    // background colour information

	// construction/destruction
	huc6270_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_vram_size(u32 vram_size) { m_vram_size = vram_size; }
	auto irq() { return m_irq_changed_cb.bind(); }

	// 8 bit accessor used for PC engine
	u8 read8(offs_t offset);
	void write8(offs_t offset, u8 data);

	// 16 bit accessor used for PC-FX
	u16 read16(offs_t offset);
	void write16(offs_t offset, u16 data);

	u16 next_pixel();
	u16 time_until_next_event()
	{
		return m_horz_to_go * 8 + m_horz_steps;
	}

	void vsync_changed(int state);
	void hsync_changed(int state);

	// pcfx can read AR thru some buffer latch
	u8 get_ar(offs_t offset) { return m_register_index; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	inline void fetch_bat_tile_row();
	void add_sprite(int index, int x, int pattern, int line, int flip_x, int palette, int priority, int sat_lsb);
	void select_sprites();
	inline void handle_vblank();
	inline void next_vert_state();
	inline void next_horz_state();
	inline void handle_dma();
	inline void regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

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
	u32 m_vram_size;

	/* Callback for when the irq line may have changed (mandatory) */
	devcb_write_line    m_irq_changed_cb;

	u8   m_register_index;

	/* HuC6270 registers */
	u16  m_mawr;
	u16  m_marr;
	u16  m_vrr;
	u16  m_vwr;
	u16  m_cr;
	u16  m_rcr;
	u16  m_bxr;
	u16  m_byr;
	u16  m_mwr;
	u16  m_hsr;
	u16  m_hdr;
	u16  m_vpr;
	u16  m_vdw;
	u16  m_vcr;
	u16  m_dcr;
	u16  m_sour;
	u16  m_desr;
	u16  m_lenr;
	u16  m_dvssr;
	u8   m_status;

	/* To keep track of external hsync and vsync signals */
	bool m_hsync;
	bool m_vsync;

	/* internal variables */
	v_state m_vert_state;
	h_state m_horz_state;
	bool m_vd_triggered;
	s32 m_vert_to_go;
	s32 m_horz_to_go;
	s32 m_horz_steps;
	s32 m_raster_count;
	bool m_dvssr_written;
	s32 m_satb_countdown;
	bool m_dma_enabled;
	u16 m_byr_latched;
	u16 m_bxr_latched;
	u16 m_bat_address;
	u16 m_bat_address_mask;
	u16 m_bat_row;
	u16 m_bat_column;
	u8 m_bat_tile_row[8];
	/* Internal sprite attribute table. SATB DMA is used to transfer data
	   from VRAM to this internal table.
	*/
	u16 m_sat[4*64];
	s32 m_sprites_this_line;
	u16 m_sprite_row_index;
	u16  m_sprite_row[1024];
	std::unique_ptr<u16[]>  m_vram;
	u16  m_vram_mask;

	static constexpr u8 vram_increments[4] = { 1, 32, 64, 128 };
};


DECLARE_DEVICE_TYPE(HUC6270, huc6270_device)

#endif // MAME_VIDEO_HUC6270_H
