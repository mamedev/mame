// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************

    Fujitsu MB88303 NMOS Television Display Controller (TVDC) emulation

************************************************************************
                            _____   _____
                /RESET   1 |*    \_/     | 22  Vcc
                   DO0   2 |             | 21  DA7
                   DO1   3 |             | 20  DA6
                   DO2   4 |             | 19  DA5
                   VOW   5 |             | 18  DA4
                  /VOB   6 |   MB88303   | 17  DA3
                /VSYNC   7 |             | 16  DA2
                /HSYNC   8 |             | 15  DA1
                    EX   9 |             | 14  DA0
                     X  10 |             | 13  ADM
                   Vss  11 |_____________| 12  LDI

************************************************************************/

#ifndef MAME_VIDEO_MB88303_H
#define MAME_VIDEO_MB88303_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb88303_device

class mb88303_device :  public device_t
{
public:
	static constexpr unsigned VISIBLE_COLUMNS = 20;
	static constexpr unsigned VISIBLE_LINES   = 9;

	// construction/destruction
	mb88303_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto vow_callback() { return m_write_vow.bind(); }
	auto vobn_callback() { return m_write_vobn.bind(); }
	auto do_callback() { return m_write_do.bind(); }

	void da_w(uint8_t data);
	void adm_w(int state);
	void reset_n_w(int state);
	void ldi_w(int state);
	void hsync_n_w(int state);
	void vsync_n_w(int state);

	void update_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum
	{
		HSZ0        = 0x01,
		HSZ1        = 0x02,
		VSZ0        = 0x04,
		VSZ1        = 0x08,
		BLK         = 0x10,
		BLKB        = 0x20,
		BLINK       = 0x40,

		HSZ0_BIT    = 0,
		HSZ1_BIT    = 1,
		VSZ0_BIT    = 2,
		VSZ1_BIT    = 3,
		BLK_BIT     = 4,
		BLKB_BIT    = 5,
		BLINK_BIT   = 6,
	};

	void hdpr_w(uint8_t data); // Horizontal Display Position Register
	void vdpr_w(uint8_t data); // Vertical Display Position Register
	void dcr_w(uint8_t data); // Display Control Register
	void gor_w(uint8_t data); // General Output Register

	void process_data();
	bool blank_display();
	bool show_background();
	bool enable_blinking();
	uint8_t hsize();
	uint8_t vsize();

	devcb_write_line m_write_vow;
	devcb_write_line m_write_vobn;
	devcb_write8 m_write_do;

	uint8_t m_da;
	uint8_t m_adm;
	uint8_t m_reset_n;
	uint8_t m_ldi;
	uint8_t m_hsync_n;
	uint8_t m_vsync_n;

	uint8_t m_display_mem[180];
	uint8_t m_horiz_display_pos;
	uint8_t m_vert_display_pos;
	uint8_t m_display_ctrl;
	uint8_t m_general_out;
	uint8_t m_address;

	bool m_addr_inc_mode;

	static const uint8_t s_character_data[0x40][7];
};


// device type definition
DECLARE_DEVICE_TYPE(MB88303, mb88303_device)

#endif // MAME_VIDEO_MB88303_H
