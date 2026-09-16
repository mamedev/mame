// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_I82357_H
#define MAME_MACHINE_I82357_H

#pragma once

#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"

class i82357_device : public device_t
{
public:
	i82357_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_rtc_address_cb() { return m_out_rtc_address.bind(); }
	auto out_int_cb() { return m_pic[0].lookup()->out_int_callback(); }
	auto out_nmi_cb() { return m_out_nmi.bind(); }
	auto out_spkr_cb() { return m_out_spkr.bind(); }

	u32 eisa_irq_ack() { return m_pic[0]->acknowledge(); }

	void in_iochk(int state);
	void in_parity(int state);

	void map(address_map &map) ATTR_COLD;

protected:
	// standard device_t overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void nmi_reg_w(u8 data);
	void nmi_ext_w(u8 data);
	u8 nmi_reg_r() { return m_nmi_reg; }
	u8 nmi_ext_r() { return m_nmi_ext; }

	TIMER_CALLBACK_MEMBER(nmi_check);

private:
	required_device_array<pic8259_device, 2> m_pic;
	required_device_array<pit8254_device, 2> m_pit;
	required_device_array<eisa_dma_device, 2> m_dma;

	devcb_write8 m_out_rtc_address;
	devcb_write_line m_out_nmi;
	devcb_write_line m_out_spkr;

	enum nmi_reg_mask : u8
	{
		NMI_T1C2_GATE      = 0x01,
		NMI_SPEAKER_DATA   = 0x02,
		NMI_PARITY_DISABLE = 0x04,
		NMI_IOCHK_DISABLE  = 0x08,
		NMI_REFRESH        = 0x10,
		NMI_T1C2_STATE     = 0x20,
		NMI_IOCHK          = 0x40,
		NMI_PARITY         = 0x80,

		NMI_WMASK          = 0x0f,
		NMI_NMI            = 0xc0,
	};
	enum nmi_ext_mask : u8
	{
		NMI_EXT_BUS_RESET   = 0x01, // bus reset (read/write)
		NMI_EXT_EN_IOPORT   = 0x02,
		NMI_EXT_EN_FAILSAFE = 0x04,
		NMI_EXT_EN_TIMEOUT  = 0x08,
		NMI_EXT_8US_TIMEOUT = 0x10, // 8us EISA bus master timeout occurred
		NMI_EXT_IOPORT      = 0x20, // NMI I/O port interrupt pending
		NMI_EXT_TIMEOUT     = 0x40, // EISA bus master timeout occurred, NMI requested
		NMI_EXT_FAILSAFE    = 0x80, // Fail-safe timer is active, NMI requested

		NMI_EXT_WMASK       = 0x0f,
		NMI_EXT_NMI         = 0xe0,
	};
	u8 m_elcr[2];

	emu_timer *m_nmi_check;
	bool m_out_nmi_asserted;
	bool m_nmi_enabled;
	u8 m_nmi_reg;
	u8 m_nmi_ext;
};

DECLARE_DEVICE_TYPE(I82357, i82357_device)

#endif // MAME_MACHINE_I82357_H
