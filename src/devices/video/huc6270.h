// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6270 interface

**********************************************************************/

#ifndef __HUC6270_H_
#define __HUC6270_H_

#include "emu.h"

#define MCFG_HUC6270_VRAM_SIZE(_size) \
	huc6270_device::set_vram_size(*device, _size);

#define MCFG_HUC6270_IRQ_CHANGED_CB(_devcb) \
	devcb = &huc6270_device::set_irq_changed_callback(*device, DEVCB_##_devcb);

class huc6270_device :  public device_t
{
public:
	// construction/destruction
	huc6270_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	static void set_vram_size(device_t &device, UINT32 vram_size) { downcast<huc6270_device &>(device).m_vram_size = vram_size; }
	template<class _Object> static devcb_base &set_irq_changed_callback(device_t &device, _Object object) { return downcast<huc6270_device &>(device).m_irq_changed_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ16_MEMBER( next_pixel );
	inline DECLARE_READ16_MEMBER( time_until_next_event )
	{
		return m_horz_to_go * 8 + m_horz_steps;
	}

	DECLARE_WRITE_LINE_MEMBER( vsync_changed );
	DECLARE_WRITE_LINE_MEMBER( hsync_changed );

	static const UINT16 HUC6270_SPRITE     = 0x0100;    // sprite colour information
	static const UINT16 HUC6270_BACKGROUND = 0x0000;    // background colour information

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

private:

	enum huc6270_v_state {
		HUC6270_VSW,
		HUC6270_VDS,
		HUC6270_VDW,
		HUC6270_VCR
	};

	enum huc6270_h_state {
		HUC6270_HDS,
		HUC6270_HDW,
		HUC6270_HDE,
		HUC6270_HSW
	};


	/* Size of Video ram (mandatory) */
	UINT32 m_vram_size;

	/* Callback for when the irq line may have changed (mandatory) */
	devcb_write_line    m_irq_changed_cb;

	UINT8   m_register_index;

	/* HuC6270 registers */
	UINT16  m_mawr;
	UINT16  m_marr;
	UINT16  m_vrr;
	UINT16  m_vwr;
	UINT16  m_cr;
	UINT16  m_rcr;
	UINT16  m_bxr;
	UINT16  m_byr;
	UINT16  m_mwr;
	UINT16  m_hsr;
	UINT16  m_hdr;
	UINT16  m_vpr;
	UINT16  m_vdw;
	UINT16  m_vcr;
	UINT16  m_dcr;
	UINT16  m_sour;
	UINT16  m_desr;
	UINT16  m_lenr;
	UINT16  m_dvssr;
	UINT8   m_status;

	/* To keep track of external hsync and vsync signals */
	int m_hsync;
	int m_vsync;

	/* internal variables */
	huc6270_v_state m_vert_state;
	huc6270_h_state m_horz_state;
	int m_vd_triggered;
	int m_vert_to_go;
	int m_horz_to_go;
	int m_horz_steps;
	int m_raster_count;
	int m_dvssr_written;
	int m_satb_countdown;
	int m_dma_enabled;
	UINT16 m_byr_latched;
	UINT16 m_bxr_latched;
	UINT16 m_bat_address;
	UINT16 m_bat_address_mask;
	UINT16 m_bat_row;
	UINT16 m_bat_column;
	UINT8 m_bat_tile_row[8];
	/* Internal sprite attribute table. SATB DMA is used to transfer data
	   from VRAM to this internal table.
	*/
	UINT16 m_sat[4*64];
	int m_sprites_this_line;
	int m_sprite_row_index;
	UINT16  m_sprite_row[1024];
	std::unique_ptr<UINT16[]>  m_vram;
	UINT16  m_vram_mask;

	const static UINT8 vram_increments[4];
};


extern const device_type HUC6270;


#endif
