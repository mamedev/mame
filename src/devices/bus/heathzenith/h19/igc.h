// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Interactive Graphics Controller (IGC) for the Heath TLB

    Base class shared by the IGC variants. This header is private
    to bus/heathzenith/h19, drivers should only include tlb.h and cards.h.

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_H19_IGC_H
#define MAME_BUS_HEATHZENITH_H19_IGC_H

#pragma once

#include "tlbbase.h"


namespace bus::heathzenith::h19 {

INPUT_PORTS_EXTERN( igc_common );

/**
 * Heath TLB plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_tlb_device : public heath_tlb_device
{
public:
	heath_igc_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8   sigma_r(u8 offset) override;
	virtual void sigma_w(u8 offset, u8 data) override;

protected:
	heath_igc_tlb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual MC6845_UPDATE_ROW(crtc_update_row) override;

	virtual u8   sigma_ctrl_r();
	virtual void sigma_ctrl_w(u8 data);

	u8   sigma_video_mem_r();
	void sigma_video_mem_w(u8 val);

	void sigma_io_lo_addr_w(u8 val);
	void sigma_io_hi_addr_w(u8 val);

	void sigma_window_lo_addr_w(u8 val);
	void sigma_window_hi_addr_w(u8 val);

	void update_fill();


	std::unique_ptr<u8[]> m_p_graphic_ram;

	// Control Register
	bool m_pixel_video_enabled;
	bool m_character_video_disabled;
	bool m_video_invert_enabled;
	bool m_alternate_character_set_enabled;
	bool m_video_fill_enabled;
	bool m_read_address_increment_disabled;
	u8   m_memory_bank_select;

	u8   m_data_reg;

	u16  m_io_address;
	u16  m_window_address;

	// automatic video memory fill in progress
	attotime m_fill_start;
	u64      m_fill_count;

private:
	required_ioport m_joystick1, m_joystick2;
};

} // namespace bus::heathzenith::h19

DECLARE_DEVICE_TYPE(HEATH_IGC, device_heath_tlb_card_interface)

#endif // MAME_BUS_HEATHZENITH_H19_IGC_H
