// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    9h0-0008_card_reader.h

    Sega Toys 9H0-0008 barcode card reader

*******************************************************************************/
#ifndef MAME_SEGA_9H0_0008_CARD_READER_H
#define MAME_SEGA_9H0_0008_CARD_READER_H

#pragma once

#include "softlist_dev.h"

#include "9h0-0008_iox.h"

 /*
  * Each card encodes a number N as `(N << 2) | 0xc01` in exactly 12 bits.
  * Stripes for each bit have spacing between them.
  *
  * Bundled with denshaca, where barcode values are compared against a table at ROM address 0x80051240.
  */
class sega_beena_card_reader_device : public device_t, public device_sega_9h0_0008_iox_slot_interface
{
public:
	sega_beena_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u32 read(bool is_ready) override;
	void scan_card(int state);

protected:
	enum card_state : u8
	{
		IDLE = 0,
		START_WRITE_DATA,
		WRITE_DATA,
	};

	sega_beena_card_reader_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool is_sync_cycle, u8 num_bits, u8 num_hold);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	required_ioport m_io;

	u16 m_card_data;
	u8 m_card_data_i;
	u8 m_card_state;
	u8 m_card_hold_i;
	u8 m_card_status;

	bool m_is_sync_cycle;
	u8 m_num_bits;
	u8 m_num_hold;
};


 /*
  * Bundled with anpaabc, where barcode values are compared against a table at ROM address 0x8039e97c.
  */
class sega_beena_card_reader_2061_device : public sega_beena_card_reader_device
{
public:
	sega_beena_card_reader_2061_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


 /*
  * Each card encodes a number in exactly 16 bits. Valid barcodes always have the last bit set.
  * Stripes for each bit are contiguous.
  *
  * Values are compared against an in-memory table for tvochken at 0xc00d0f9c, taken from ROM address 0xa00579b4.
  */
class sega_tvochken_card_reader_device : public sega_beena_card_reader_device
{
public:
	sega_tvochken_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};


DECLARE_DEVICE_TYPE(BEENA_CARD_READER, sega_beena_card_reader_device)
DECLARE_DEVICE_TYPE(BEENA_CARD_READER_2061, sega_beena_card_reader_2061_device)
DECLARE_DEVICE_TYPE(TVOCHKEN_CARD_READER, sega_tvochken_card_reader_device)

#endif // MAME_SEGA_9H0_0008_CARD_READER_H
