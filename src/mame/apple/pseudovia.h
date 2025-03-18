// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_PSEUDOVIA_H
#define MAME_APPLE_PSEUDOVIA_H

#pragma once

class pseudovia_device :  public device_t
{
public:
	pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }
	auto readconfig_handler() { return m_in_config_handler.bind(); }
	auto readvideo_handler() { return m_in_video_handler.bind(); }
	auto readmsc_handler() { return m_in_msc_handler.bind(); }
	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }
	auto writeconfig_handler() { return m_out_config_handler.bind(); }
	auto writevideo_handler() { return m_out_video_handler.bind(); }
	auto writemsc_handler() { return m_out_msc_handler.bind(); }
	auto irq_callback() { return m_write_irq.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	template <u8 mask> void slot_irq_w(int state);
	void vbl_irq_w(int state);
	void scc_irq_w(int state);
	void asc_irq_w(int state);
	void scsi_irq_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line m_write_irq;
	devcb_read8 m_in_a_handler, m_in_b_handler, m_in_config_handler, m_in_video_handler, m_in_msc_handler;
	devcb_write8 m_out_a_handler, m_out_b_handler, m_out_config_handler, m_out_video_handler, m_out_msc_handler;

	uint8_t m_pseudovia_regs[256], m_pseudovia_ier, m_pseudovia_ifr;

	void pseudovia_recalc_irqs();
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE_PSEUDOVIA, pseudovia_device)

#endif // MAME_APPLE_PSEUDOVIA_H
