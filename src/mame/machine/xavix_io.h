// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_XAVIX_IO_H
#define MAME_MACHINE_XAVIX_IO_H

class xavix_io_device : public device_t
{
public:
	xavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_in0_cb.bind(); }
	auto read_1_callback() { return m_in1_cb.bind(); }

	auto write_0_callback() { return m_out0_cb.bind(); }
	auto write_1_callback() { return m_out1_cb.bind(); }

	DECLARE_WRITE8_MEMBER(xav_7a0x_dir_w);
	DECLARE_WRITE8_MEMBER(xav_7a0x_dat_w);

	DECLARE_READ8_MEMBER(xav_7a0x_dir_r);
	DECLARE_READ8_MEMBER(xav_7a0x_dat_r);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_read8 m_in0_cb;
	devcb_read8 m_in1_cb;

	devcb_write8 m_out0_cb;
	devcb_write8 m_out1_cb;

	uint8_t m_dir[2];
	uint8_t m_dat[2];
};

DECLARE_DEVICE_TYPE(XAVIXIO, xavix_io_device)

#endif // MAME_MACHINE_XAVIX_IO_H
