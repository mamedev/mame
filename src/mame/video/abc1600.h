// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Mover emulation

**********************************************************************/
#ifndef MAME_VIDEO_ABC1600_H
#define MAME_VIDEO_ABC1600_H

#pragma once


#include "video/mc6845.h"
#include "emupal.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define ABC1600_MOVER_TAG "mover"

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> abc1600_mover_device

class abc1600_mover_device :  public device_t,
								public device_memory_interface
{
public:
	// construction/destruction
	abc1600_mover_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void vram_map(address_map &map);
	virtual void crtc_map(address_map &map);
	virtual void iowr0_map(address_map &map);
	virtual void iowr1_map(address_map &map);
	virtual void iowr2_map(address_map &map);

	void mover_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	inline uint16_t get_drmsk();
	inline void get_shinf();
	inline uint16_t get_wrmsk();
	inline uint16_t barrel_shift(uint16_t gmdr);
	inline uint16_t word_mixer(uint16_t rot);
	inline void clock_mfa_x();
	inline void clock_mfa_y();
	inline void clock_mta_x();
	inline void clock_mta_y();
	inline void load_mfa_x();
	inline void load_mta_x();
	inline void compare_mta_x();
	inline void compare_mta_y();
	inline void load_xy_reg();
	void mover();

	DECLARE_READ8_MEMBER( video_ram_r );
	DECLARE_WRITE8_MEMBER( video_ram_w );

	DECLARE_READ8_MEMBER( iord0_r );
	DECLARE_WRITE8_MEMBER( ldsx_hb_w );
	DECLARE_WRITE8_MEMBER( ldsx_lb_w );
	DECLARE_WRITE8_MEMBER( ldsy_hb_w );
	DECLARE_WRITE8_MEMBER( ldsy_lb_w );
	DECLARE_WRITE8_MEMBER( ldtx_hb_w );
	DECLARE_WRITE8_MEMBER( ldtx_lb_w );
	DECLARE_WRITE8_MEMBER( ldty_hb_w );
	DECLARE_WRITE8_MEMBER( ldty_lb_w );
	DECLARE_WRITE8_MEMBER( ldfx_hb_w );
	DECLARE_WRITE8_MEMBER( ldfx_lb_w );
	DECLARE_WRITE8_MEMBER( ldfy_hb_w );
	DECLARE_WRITE8_MEMBER( ldfy_lb_w );
	DECLARE_WRITE8_MEMBER( wrml_w );
	DECLARE_WRITE8_MEMBER( wrdl_w );
	DECLARE_WRITE8_MEMBER( wrmask_strobe_hb_w );
	DECLARE_WRITE8_MEMBER( wrmask_strobe_lb_w );
	DECLARE_WRITE8_MEMBER( enable_clocks_w );
	DECLARE_WRITE8_MEMBER( flag_strobe_w );
	DECLARE_WRITE8_MEMBER( endisp_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update);

	inline uint16_t read_videoram(offs_t offset);
	inline void write_videoram(offs_t offset, uint16_t data, uint16_t mask);
	inline uint16_t get_crtca(uint16_t ma, uint8_t ra, uint8_t column);

	const address_space_config m_space_config;

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<uint16_t> m_wrmsk_rom;
	required_region_ptr<uint8_t> m_shinf_rom;
	required_region_ptr<uint16_t> m_drmsk_rom;

	int m_endisp;               // enable display
	int m_clocks_disabled;      // clocks disabled
	uint16_t m_gmdi;              // video RAM data latch
	uint16_t m_wrm;               // write mask latch
	uint8_t m_ms[16];             // mover sequence control
	uint8_t m_ds[16];             // display sequence control
	uint8_t m_flag;               // flags
	uint16_t m_xsize;             // X size
	uint16_t m_ysize;             // Y size
	int m_udx;                  // up/down X
	int m_udy;                  // up/down Y
	uint16_t m_xfrom;             // X from
	uint16_t m_xto;               // X to
	uint16_t m_yto;               // Y to
	uint16_t m_ty;                // to Y
	uint32_t m_mfa;               // mover from address
	uint32_t m_mta;               // mover to address
	uint8_t m_sh;                 //
	uint16_t m_mdor;              //
	int m_hold_1w_cyk;          //
	int m_wrms0;                //
	int m_wrms1;                //
	int m_rmc;                  // row match count
	int m_cmc;                  // column match count
	int m_amm;                  // active mover mask
};


// device type definition
DECLARE_DEVICE_TYPE(ABC1600_MOVER, abc1600_mover_device)



#endif // MAME_VIDEO_ABC1600_H
