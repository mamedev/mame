// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_MACHINE_PSEUDOVIA_H
#define MAME_MACHINE_PSEUDOVIA_H

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

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

	template <u8 mask> void slot_irq_w(int state);
	void vbl_irq_w(int state);
	void scc_irq_w(int state);
	virtual void asc_irq_w(int state);
	void scsi_irq_w(int state);
	void scsi_drq_w(int state);
	void slot_irq_w(int state);

protected:
	pseudovia_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void pseudovia_recalc_irqs();

	uint8_t m_pseudovia_regs[0x100];
	uint8_t m_live_main_ints;

	devcb_write_line m_write_irq;
	devcb_read8 m_in_a_handler, m_in_b_handler, m_in_config_handler, m_in_video_handler, m_in_msc_handler;
	devcb_write8 m_out_a_handler, m_out_b_handler, m_out_config_handler, m_out_video_handler, m_out_msc_handler;
};

// V8 version has the same decode as RBV but ASC IRQs are level triggered
class v8_pseudovia_device: public pseudovia_device
{
public:
	v8_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write(offs_t offset, uint8_t data) override;

	virtual void asc_irq_w(int state) override;
};

// Sonora version decodes full A4-A0 and has 2 additional registers
class sonora_pseudovia_device: public pseudovia_device
{
public:
	sonora_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	virtual void asc_irq_w(int state) override;
};

// MSC version decodes A7-A0 (0x100 bytes) and mirrors every 0x100 bytes
class msc_pseudovia_device: public pseudovia_device
{
public:
	msc_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

	virtual void asc_irq_w(int state) override;
};

// more VIA-like version found in PowerBook 14x/16x/170/180
class pb030_pseudovia_device: public pseudovia_device
{
public:
	pb030_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
};

// maximally VIA-like version  found in IOSB/PrimeTime/PrimeTime II based machines
class quadra_pseudovia_device : public pseudovia_device
{
public:
	quadra_pseudovia_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void pseudovia_recalc_irqs() override;
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE_PSEUDOVIA, pseudovia_device)
DECLARE_DEVICE_TYPE(APPLE_V8_PSEUDOVIA, v8_pseudovia_device)
DECLARE_DEVICE_TYPE(APPLE_SONORA_PSEUDOVIA, sonora_pseudovia_device)
DECLARE_DEVICE_TYPE(APPLE_MSC_PSEUDOVIA, msc_pseudovia_device)
DECLARE_DEVICE_TYPE(APPLE_PB030_PSEUDOVIA, pb030_pseudovia_device)
DECLARE_DEVICE_TYPE(APPLE_QUADRA_PSEUDOVIA, quadra_pseudovia_device)

#endif // MAME_MACHINE_PSEUDOVIA_H
