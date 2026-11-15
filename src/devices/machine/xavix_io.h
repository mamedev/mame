// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_IO_H
#define MAME_TVGAMES_XAVIX_IO_H

class xavix_io_device : public device_t
{
public:
	xavix_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_0_callback() { return m_in0_cb.bind(); }
	auto read_1_callback() { return m_in1_cb.bind(); }

	auto write_0_callback() { return m_out0_cb.bind(); }
	auto write_1_callback() { return m_out1_cb.bind(); }

	void xav_7a0x_dir_w(offs_t offset, uint8_t data);
	void xav_7a0x_dat_w(offs_t offset, uint8_t data);

	uint8_t xav_7a0x_dir_r(offs_t offset);
	uint8_t xav_7a0x_dat_r(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read8 m_in0_cb;
	devcb_read8 m_in1_cb;

	devcb_write8 m_out0_cb;
	devcb_write8 m_out1_cb;

	uint8_t m_dir[2];
	uint8_t m_dat[2];
};

DECLARE_DEVICE_TYPE(XAVIXIO, xavix_io_device)

#endif // MAME_TVGAMES_XAVIX_IO_H
