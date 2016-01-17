// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Mover emulation

**********************************************************************/

#pragma once

#ifndef __ABC1600_MOVER__
#define __ABC1600_MOVER__

#include "emu.h"
#include "video/mc6845.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define ABC1600_MOVER_TAG "mover"


#define MCFG_ABC1600_MOVER_ADD() \
	MCFG_DEVICE_ADD(ABC1600_MOVER_TAG, ABC1600_MOVER, 0)



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> abc1600_mover_device

class abc1600_mover_device :  public device_t,
								public device_memory_interface
{
public:
	// construction/destruction
	abc1600_mover_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_ADDRESS_MAP(vram_map, 8);
	virtual DECLARE_ADDRESS_MAP(crtc_map, 8);
	virtual DECLARE_ADDRESS_MAP(io_map, 8);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	inline UINT16 get_drmsk();
	inline void get_shinf();
	inline UINT16 get_wrmsk();
	inline UINT16 barrel_shift(UINT16 gmdr);
	inline UINT16 word_mixer(UINT16 rot);
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

	inline UINT16 read_videoram(offs_t offset);
	inline void write_videoram(offs_t offset, UINT16 data, UINT16 mask);
	inline UINT16 get_crtca(UINT16 ma, UINT8 ra, UINT8 column);

	const address_space_config m_space_config;

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<UINT16> m_wrmsk_rom;
	required_region_ptr<UINT8> m_shinf_rom;
	required_region_ptr<UINT16> m_drmsk_rom;

	int m_endisp;               // enable display
	int m_clocks_disabled;      // clocks disabled
	UINT16 m_gmdi;              // video RAM data latch
	UINT16 m_wrm;               // write mask latch
	UINT8 m_ms[16];             // mover sequence control
	UINT8 m_ds[16];             // display sequence control
	UINT8 m_flag;               // flags
	UINT16 m_xsize;             // X size
	UINT16 m_ysize;             // Y size
	int m_udx;                  // up/down X
	int m_udy;                  // up/down Y
	UINT16 m_xfrom;             // X from
	UINT16 m_xto;               // X to
	UINT16 m_yto;               // Y to
	UINT16 m_ty;                // to Y
	UINT32 m_mfa;               // mover from address
	UINT32 m_mta;               // mover to address
	UINT8 m_sh;                 //
	UINT16 m_mdor;              //
	int m_hold_1w_cyk;          //
	int m_wrms0;                //
	int m_wrms1;                //
	int m_rmc;                  // row match count
	int m_cmc;                  // column match count
	int m_amm;                  // active mover mask
};


// device type definition
extern const device_type ABC1600_MOVER;



#endif
