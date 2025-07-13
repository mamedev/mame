// license:BSD-3-Clause
// copyright-holders: NaokiS
/*

	Serial VFD device driver for two/three wire 1 line vfds up to 16 characters.
	
	TODO:
	 * Brightness control, just as soon as MAME supports brightness on segment displays.

*/

#ifndef MAME_VIDEO_SERIALVFD_H
#define MAME_VIDEO_SERIALVFD_H

#pragma once

class serial_vfd_device : 
	public device_t
{
public:
	serial_vfd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void write_reset(bool state);
	void write_data(bool state);
	void write_clock(bool state);

protected:
	virtual void device_start() override;
	//virtual void device_reset() override;

private:
	const uint16_t vfd_charMap[64] = {
		0x025F, 0x00F7, 0x038F, 0x0039, 0x030F, 0x0079, 0x0071, 0x00BD,
		0x00F6, 0x0309, 0x001E, 0x3070, 0x0038, 0x1836, 0x2836, 0x003F,
		0x00F3, 0x203F, 0x20F3, 0x00ED, 0x0301, 0x003E, 0x1430, 0x2436,
		0x3C00, 0x1A00, 0x1409, 0x0039, 0x2800, 0x0006, 0x2400, 0x0008,
		0x0000, 0x0909, 0x0120, 0x03CE, 0x03ED, 0x17E5, 0x295D, 0x1000,
		0x3000, 0x0C00, 0x3FC0, 0x03C0, 0xC000, 0x00C0, 0x4000, 0x1400, 
		0x143F, 0x0300, 0x00DB, 0x00CF, 0x00E6, 0x00ED, 0x00FD, 0x0007, 
		0x00FF, 0x00EF, 0x0009, 0x0401, 0x1408, 0x00C8, 0x2808, 0x0283
	};

	output_finder<16> m_vfd;

	uint8_t m_bitCount = 0;
	uint8_t m_cmd = 0x00;
	uint8_t m_digits = 0;
	uint8_t m_bright = 0x1F;
	uint8_t m_cursor = 0;
	bool m_data = 0, m_clock = 0, m_reset = 0, m_lastClock = 0;
	uint16_t m_buff[16];

	void run_command();

};

DECLARE_DEVICE_TYPE(SERIAL_VFD, serial_vfd_device)

#endif // MAME_VIDEO_SERIALVFD_H
