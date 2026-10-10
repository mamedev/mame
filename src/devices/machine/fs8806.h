// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    fs8806.h

    FameG FS8806 authentication / secure-counter chip

***************************************************************************/

#ifndef MAME_MACHINE_FS8806_H
#define MAME_MACHINE_FS8806_H

#pragma once

#include "i2chle.h"

class fs8806_device : public device_t, public i2c_hle_interface
{
public:
	fs8806_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// the 8-byte DES key is unique to each customer's chips
	fs8806_device &set_key(u64 key) { m_key = key; return *this; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// i2c_hle_interface implementation
	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;
	virtual bool has_subaddress() const override { return false; }
	virtual void i2c_stop() override;
	virtual const char *get_tag() override { return tag(); }

private:
	void handle_packet();
	void make_response(u8 length);

	void crypt(u8 *dst, const u8 *src, int length, bool encrypt) const;
	static u16 crc(const u8 *data, int length);

	u64 m_key;
	u64 m_sk[16];

	u8 m_rx[80];        // longest inbound packet is 8 + 0x40 header/payload plus CRC
	u8 m_rx_len;
	u8 m_tx[18];
	u8 m_tx_len;

	u8 m_eeprom[96];    // FIXME: should be non-volatile
	u8 m_challenge[8];  // last block presented by command 0xb1
	u8 m_verify[8];     // last block presented by command 0xb3
	u16 m_counter;      // packets accepted since the last reset
};

DECLARE_DEVICE_TYPE(FS8806, fs8806_device)

#endif // MAME_MACHINE_FS8806_H
