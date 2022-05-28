// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef DEVICES_MACHINE_BBC_ELK_CASIN_H
#define DEVICES_MACHINE_BBC_ELK_CASIN_H

#pragma once


class bbc_elk_casin_device : public device_t
{
public:
	bbc_elk_casin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	bool input(double tap_val);
	int casin() { return m_casin; }
	bool timeout() { return m_timeout; }
	void reset();

	static constexpr int SAMPLING_FREQUENCY = 44'100;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	double m_last_tap_val = 0.0;
	int m_tap_val_length = 0;
	int m_len[4]{};
	int m_casin = 0;
	bool m_timeout = false;
};


DECLARE_DEVICE_TYPE(BBC_ELK_CASIN, bbc_elk_casin_device)

#endif // DEVICES_MACHINE_BBC_ELK_CASIN_H
