// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    CES Blitter, with two layers and double buffering (Xilinx FPGA)

***************************************************************************/

#ifndef MAME_VIDEO_CESBLIT_H
#define MAME_VIDEO_CESBLIT_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cesblit_device

class cesblit_device :  public device_t,
						public device_video_interface,
						public device_memory_interface
{
public:
	typedef int (*compute_addr_t) (uint16_t reg_low, uint16_t reg_mid, uint16_t reg_high);

	// construction/destruction
	template <typename T>
	cesblit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: cesblit_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	cesblit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_compute_addr(compute_addr_t compute_addr)  { m_compute_addr = compute_addr; }
	auto irq_callback() { return m_blit_irq_cb.bind(); }

	void color_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void addr_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t status_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	void do_blit();

	address_space_config m_space_config;
	address_space *m_space;

	devcb_write_line m_blit_irq_cb; // blit finished irq

	bitmap_ind16 m_bitmap[2][2];
	uint16_t m_regs[0x12/2];
	uint16_t m_color;
	uint16_t m_addr_hi;
	compute_addr_t m_compute_addr;
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

// device type definition
DECLARE_DEVICE_TYPE(CESBLIT, cesblit_device)

#endif // MAME_VIDEO_CESBLIT_H
