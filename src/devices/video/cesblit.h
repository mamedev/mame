// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    CES Blitter, with two layers and double buffering (Xilinx FPGA)

***************************************************************************/

#ifndef MAME_VIDEO_CESBLIT_H
#define MAME_VIDEO_CESBLIT_H

#pragma once

/***************************************************************************
    INTERFACE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CESBLIT_ADD(_tag, _screen, _clock) \
	MCFG_DEVICE_ADD(_tag, CESBLIT, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen)

#define MCFG_CESBLIT_MAP    MCFG_DEVICE_PROGRAM_MAP

#define MCFG_CESBLIT_COMPUTE_ADDR(_compute_addr) \
	downcast<cesblit_device &>(*device).set_compute_addr(_compute_addr);

#define MCFG_CESBLIT_IRQ_CB(_devcb) \
	devcb = &downcast<cesblit_device &>(*device).set_irq_callback(DEVCB_##_devcb);

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
	cesblit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_compute_addr(compute_addr_t compute_addr)  { m_compute_addr = compute_addr; }
	template <class Object> devcb_base &set_irq_callback(Object &&cb) { return m_blit_irq_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE16_MEMBER(color_w);
	DECLARE_WRITE16_MEMBER(addr_hi_w);
	DECLARE_WRITE16_MEMBER(regs_w);
	DECLARE_READ16_MEMBER(status_r);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
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
