// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Secure module (ESAM) on Sealy Z80 boards, high-level emulation

    Used by Sealy lucky168, wired to the lines the other Sealy Z80 boards use for
    their 93C46.  The chip is an eight-pin DIP marked "Watchdata e2829 0F10"
    (read from a PCB photo); it is undumped and no datasheet has been found.

    Deduced from the host code, not confirmed on documentation: the host speaks
    ISO 7816 T=0 and, next to the standard GET CHALLENGE, EXTERNAL
    AUTHENTICATE, VERIFY, SELECT, READ/UPDATE BINARY and GET RESPONSE, uses
    the CLA 80 management commands ERASE DF (0E), CREATE FILE (E0) and WRITE
    KEY (D4) common to Chinese PBOC-style card operating systems (Watchdata
    sells TimeCOS; what this part actually runs is unknown).  The host creates
    MF 3F00 (type 38), a key file (type 3F, 0x50 bytes) and transparent
    EF 001D (type 28, 0x100 bytes).

    Only what the host uses is implemented: one DES external authentication
    key, one 8-byte PIN and the single binary file.  Access conditions are not
    modelled beyond requiring both authentication and PIN for file access, the
    ATR is minimal (3B 00) and GET RESPONSE has nothing to return.

    The bit period is 748 ticks of the device clock, i.e. the period of the
    host's bit-banging loop when the device is clocked like the 6 MHz Z80.  The
    real chip clock is unknown.

    NVRAM image (this layout is specific to the HLE, not the chip's EEPROM):
      000-002  "SC" 01        image signature
      004      provisioned objects: 01 MF, 02 key file, 04 EF 001D,
                                    08 external authentication key, 10 PIN
      008-00f  external authentication key (DES)
      010-017  PIN
      020-11f  EF 001D contents
*/
#ifndef MAME_SEALY_SEALY_Z80_ESAM_H
#define MAME_SEALY_SEALY_Z80_ESAM_H

#pragma once

#include <array>
#include <initializer_list>


class sealy_z80_esam_device : public device_t, public device_nvram_interface
{
public:
	sealy_z80_esam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void clk_w(int state); // clock enable
	void rst_w(int state); // reset, active low
	void io_w(int state);  // host side of the bidirectional I/O line
	int io_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	static constexpr uint32_t ETU = 748;
	static constexpr uint32_t TX_CAPACITY = 512;
	static constexpr uint32_t RX_CAPACITY = 260;
	static constexpr uint32_t SYSTEM_SIZE = 0x20;
	static constexpr uint32_t FILE_SIZE = 0x100;

	enum : uint8_t
	{
		OBJ_MF = 0x01,
		OBJ_KEY_FILE = 0x02,
		OBJ_DATA_FILE = 0x04,
		OBJ_DES_KEY = 0x08,
		OBJ_PIN = 0x10
	};

	optional_memory_region m_default_image;

	// persistent contents
	std::array<uint8_t, SYSTEM_SIZE> m_system;
	std::array<uint8_t, FILE_SIZE> m_file;

	// transmit queue: start time and value of each byte sent to the host
	std::array<uint64_t, TX_CAPACITY> m_tx_start;
	std::array<uint8_t, TX_CAPACITY> m_tx_byte;
	uint32_t m_tx_head;
	uint32_t m_tx_count;
	uint64_t m_tx_end;

	// receiver
	std::array<uint8_t, RX_CAPACITY> m_rx;
	uint32_t m_rx_count;
	uint32_t m_payload;
	uint64_t m_rx_sample;
	uint8_t m_rx_bit;
	uint8_t m_rx_value;
	uint8_t m_parity;
	bool m_receiving;
	bool m_parity_ok;
	bool m_overflow;
	uint64_t m_error_until;

	// lines
	bool m_line;
	bool m_clock;
	bool m_reset;

	// session
	bool m_selected;
	bool m_authenticated;
	bool m_verified;
	bool m_challenge_valid;
	uint64_t m_challenge;

	uint64_t now() const;
	bool has(uint8_t objects) const;
	void clear_serial();
	void reset_session();
	void enqueue(uint64_t t, std::initializer_list<uint8_t> bytes);
	void enqueue(uint64_t t, const uint8_t *bytes, uint32_t count);
	void status(uint64_t t, uint8_t sw1, uint8_t sw2 = 0);
	bool bytes_equal(uint32_t offset, std::initializer_list<uint8_t> expected) const;
	void management(uint64_t t);
	void command(uint64_t t);
	void byte_received(uint64_t t, uint8_t value);
	void advance(uint64_t t);
	int line_state(uint64_t t) const;
};

DECLARE_DEVICE_TYPE(SEALY_Z80_ESAM, sealy_z80_esam_device)

#endif // MAME_SEALY_SEALY_Z80_ESAM_H
