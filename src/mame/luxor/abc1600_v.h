// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Mover emulation

**********************************************************************/
#ifndef MAME_LUXOR_ABC1600_V_H
#define MAME_LUXOR_ABC1600_V_H

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

	virtual void vram_map(address_map &map) ATTR_COLD;
	virtual void crtc_map(address_map &map) ATTR_COLD;
	virtual void iowr0_map(address_map &map) ATTR_COLD;
	virtual void iowr1_map(address_map &map) ATTR_COLD;
	virtual void iowr2_map(address_map &map) ATTR_COLD;

	void mover_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

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

	uint8_t video_ram_r(offs_t offset);
	void video_ram_w(offs_t offset, uint8_t data);

	uint8_t iord0_r();
	void ldsx_hb_w(uint8_t data);
	void ldsx_lb_w(uint8_t data);
	void ldsy_hb_w(uint8_t data);
	void ldsy_lb_w(uint8_t data);
	void ldtx_hb_w(uint8_t data);
	void ldtx_lb_w(uint8_t data);
	void ldty_hb_w(uint8_t data);
	void ldty_lb_w(uint8_t data);
	void ldfx_hb_w(uint8_t data);
	void ldfx_lb_w(uint8_t data);
	void ldfy_hb_w(uint8_t data);
	void ldfy_lb_w(uint8_t data);
	void wrml_w(offs_t offset, uint8_t data);
	void wrdl_w(offs_t offset, uint8_t data);
	void wrmask_strobe_hb_w(uint8_t data);
	void wrmask_strobe_lb_w(uint8_t data);
	void enable_clocks_w(uint8_t data);
	void flag_strobe_w(uint8_t data);
	void endisp_w(uint8_t data);

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

	int m_endisp = 0;               // enable display
	int m_clocks_disabled = 0;      // clocks disabled
	uint16_t m_gmdi = 0;              // video RAM data latch
	uint16_t m_wrm = 0;               // write mask latch
	uint8_t m_ms[16]{};             // mover sequence control
	uint8_t m_ds[16]{};             // display sequence control
	uint8_t m_flag = 0;               // flags
	uint16_t m_xsize = 0;             // X size
	uint16_t m_ysize = 0;             // Y size
	int m_udx = 0;                  // up/down X
	int m_udy = 0;                  // up/down Y
	uint16_t m_xfrom = 0;             // X from
	uint16_t m_xto = 0;               // X to
	uint16_t m_yto = 0;               // Y to
	uint16_t m_ty = 0;                // to Y
	uint32_t m_mfa = 0;               // mover from address
	uint32_t m_mta = 0;               // mover to address
	uint8_t m_sh = 0;                 //
	uint16_t m_mdor = 0;              //
	int m_hold_1w_cyk = 0;          //
	int m_wrms0 = 0;                //
	int m_wrms1 = 0;                //
	int m_rmc = 0;                  // row match count
	int m_cmc = 0;                  // column match count
	int m_amm = 0;                  // active mover mask
};


// device type definition
DECLARE_DEVICE_TYPE(ABC1600_MOVER, abc1600_mover_device)



#endif // MAME_LUXOR_ABC1600_V_H
