// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  Xicor 28-series Parallel EEPROM read and write logic,
  including write protection command sequences.
  Caters for different speeds such as X28C256, X28HC256, etc.
  Caters for different storage sizes such as X28C64, X28C256, X28C010,
  XM28C020, etc.

***************************************************************************/

#ifndef MAME_MACHINE_X28_H
#define MAME_MACHINE_X28_H

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// x28_device: Xicor 28-series EEPROM with paged write and software write protection

/**
 * Template parameters
 *
 * AddressBits:
 *   The number of bits in the address bus.
 *   _28_64 EEPROMs store 64 kbits = 8 kbytes = 13 address bits,
 *   _28_256 ones store 256 kbits = 32 kbytes = 15 address bits,
 *   _28_512 EEPROMs store 512 kbits = 64 kbytes = 16 address bits,
 *   _28_010 ones store 1024 kbits = 128 kbytes = 17 address bits.
 *
 * PageSizeBytes:
 *   These EEPROMs support writing an entire page at a time. These page sizes vary
 *   by EEPROM size and by manufacturer. 64 and 128 byte page sizes are both common.
 *
 * TBLCUsec:
 *   The EEPROM's T_BLC, the "Byte Load Cycle Time", in microseconds.
 *   After a byte write, if another byte on the same page is written within T_BLC,
 *   those writes will be combined in a single programming cycle. Or in other words,
 *   the programming cycle starts T_BLC after the most recent write.
 *   Xicor X28C256 has T_BLC = 100 microseconds.
 *   If TBLCUsec == 0, then there is no timed end to the Byte Load Cycle, so we
 *   _must_ trigger programming in some other way. The only way to do that is to triggger
 *   programming on read(), so that is what we do. See also ProgramOnRead below, for
 *   explicitly enabling this behaviour.
 *
 * TWCUsec:
 *   The EEPROM's T_WC, the "Write Cycle Time", in microseconds. This is the amount
 *   of time it takes for the EEPROM to complete is programming cycle.
 *   Xicor X28C256 has T_WC typ = 5ms = 5000 microseconds, max = 10ms = 10000 microseconds.
 *   Xicor X28HC256 has T_WC typ = 3ms = 3000 microseconds, max = 5ms = 5000 microseconds.
 *
 * ProgramOnRead:
 *   Instead of waiting for the TBLCUsec interval to elapse, whenever a Read hapens,
 *   immediately start any pending programming cycle.
 *   This is not present on real devices but here it allows us to create a 'fast' eeprom.
 *   In particular, if we know that the client always follows writes with reads to verify
 *   that the programming cycle completes, then we can set TBLCUsec to 0 and thus implicitly
 *   ProgramOnRead to true. We now have a device that obeys the EEPROM write
 *   protection commands but does not have to wait for T_BLC to expire before writing the
 *   buffered data to storage.
 *   If we then also set TWCUsec to 0, we have a device that also immediately writes the
 *   buffered data to storage, giving us a very fast EEPROM device.
 *   This can be used for example for external EEPROM cartridges, where emulating the precise
 *   timing of a particular chip is unlikely to be very important, and instead, allowing
 *   that cartridge to be as quick as possible may give a more pleasant user experience.
 */
template<
	int AddressBits,
	uint32_t PageSizeBytes,
	uint32_t TBLCUsec,
	uint32_t TWCUsec,
	bool ProgramOnRead = false
>
class x28_device : public device_t
{
public:

	static constexpr uint32_t ADDRESS_BITS = AddressBits;
	static constexpr uint32_t TOTAL_SIZE_BYTES = 1 << AddressBits;
	static constexpr uint32_t ADDRESS_MASK = TOTAL_SIZE_BYTES - 1;
	static constexpr uint32_t PAGE_SIZE_BYTES = PageSizeBytes;
	static constexpr uint32_t T_BLC_USEC = TBLCUsec;
	static constexpr uint32_t T_WC_USEC = TWCUsec;
	static constexpr bool PROGRAM_ON_READ = (TBLCUsec == 0) || ProgramOnRead;

	static constexpr uint32_t PAGE_OFFSET_MASK = PageSizeBytes - 1;
	static constexpr uint32_t PAGE_MASK = ~(PAGE_OFFSET_MASK);

	static constexpr uint8_t INVERSE_DATA_BIT = 1 << 7;
	static constexpr uint8_t TOGGLE_BIT = 1 << 6;

	// construction/destruction
	x28_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	x28_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~x28_device() {}

	void write(uint32_t offset, uint8_t data);
	uint8_t read(uint32_t offset);

	void reset();

	// Allow clients direct access to stored data.
	std::array<uint8_t, TOTAL_SIZE_BYTES> &data() {
		return m_storage;
	}

#ifndef X28_VISIBLE_FOR_TESTING
protected:
#endif // X28_VISIBLE_FOR_TESTING

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// Change State to a new internal state
	void change_to_state(int ns);

	// Error in the internal state machine, return to the correct idle internal state
	void state_machine_error();

	// Change State to a new command processing state
	void change_to_command_state(int ns);

	// Error in the command state machine, return to the correct idle internal state
	void command_state_machine_error();

	// internal state
	enum {
		// idle state: reads work as normal, writes will succeed or fail depending on
		// m_write_enabled - except for those writes that are part of one of the protection
		// enable or disable sequences.
		STATE_IDLE,

		// After detecting the third write that initiates a protection enable sequence,
		// writing A0 to address 5555 (1555 on X28C64).
		// At this point the device will accept one page write, before then going into
		// or remaining in write-protected mode.
		STATE_PROTECTED_WRITE,

		// When a write operation starts, this selects a page that is being written,
		// and writes the byte to an internal buffer.
		// As long as the next write is to the same page (higher address bits remain
		// the same) and the next write is initiated within T_BLC, more bytes can be
		// written, and those too will be written to the internal buffer.
		// If no more writes happen within T_BLC, thw programming cycle starts,
		// during which time the buffer will be saved to the corresponding page in
		// the persistent EEPROM storage.
		STATE_BUFFERING,

		// Buffered data is being programmed into the EEPROM.
		STATE_PROGRAMMING,
	};

	// Command processing state
	enum {
		// Not in any command sequence.
		COMMAND_STATE_NONE = 0,

		// after detecting the first write that initiates a command sequence,
		// writing AA to address 5555 (1555 on X28C64)
		COMMAND_STATE_1,

		// after detecting the second write  that initiates a command sequence,
		// writing 55 to address 2AAA (0AAA on X28C64)
		COMMAND_STATE_2,
		// If in this state the client writes A0 to 5555 (1555 on X28C64),
		// that concludes the Protection Enable command sequence and enters
		// the Protected Write state: allowing writes to one page, at the end of which
		// the device will enter the Write Protected state.
		// If instead the client writes  80 to 5555 (1555 on X28C64), this continues
		// the Protection Disable command sequence.


		// after detecting the third write that initiates a protection disable command sequence,
		// writing 80 to address 5555 (1555 on X28C64)
		COMMAND_STATE_PROTECION_DISABLE_3,

		// after detecting the fourth write that initiates a protection disable command sequence,
		// writing AA to address 5555 (1555 on X28C64)
		COMMAND_STATE_PROTECION_DISABLE_4,

		// after detecting the fifth write that initiates a protection disable command sequence.
		// writing 55 to address 2AAA (0AAA on X28C64)
		COMMAND_STATE_PROTECION_DISABLE_5,

		// after detecting the sixth write in the protection disable command sequence,
		// writing 20 to address 5555 (1555 on X28C64),
		// the device will return to COMMAND_STATE_NONE and STATE_IDLE with m_write_enabled = false.
	};

  std::array<uint8_t, TOTAL_SIZE_BYTES> m_storage;
  bool m_program_buffer_to_eeprom;
	emu_timer *m_start_programming_timer;
	emu_timer *m_programming_completed_timer;
	uint32_t m_last_written_offset;
	uint8_t m_toggle_bit = 0;
	int m_state = STATE_IDLE;
	int m_command_state = COMMAND_STATE_NONE;
	bool m_write_enabled = true;
	int m_buffering_page = 0;
	std::array<uint8_t, PageSizeBytes> m_page_buffer;

	// Timer callbacks
	void start_programming_cycle(s32 param = 0);
	void programming_cycle_complete(s32 param = 0);
};

// Concrete devices

// Real devices:

// X28C64: 64kbit == 8k bytes, 64 bytes per page
class x28c64_device : public x28_device<13, 64, 100, 5000>
{
public:
	x28c64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// X28C256: 256kbit == 32k bytes, 64 bytes per page
class x28c256_device : public x28_device<15, 64, 100, 5000>
{
public:
	x28c256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// X28HC256: 256kbit == 32k bytes, 64 bytes per page, T_WC = 3 ms.
class x28hc256_device : public x28_device<15, 64, 100, 3000>
{
public:
	x28hc256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// X28C512: 512kbit == 64k bytes, 128 bytes per page
class x28c512_device : public x28_device<16, 128, 100, 5000>
{
public:
	x28c512_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// X28C010: 1Mbit = 128k bytes, 256 bytes per page
class x28c010_device : public x28_device<17, 256, 100, 5000>
{
public:
	x28c010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// XM28C020: 2Mbit = 256 kbytes, 128 bytes per page;
// comprised of 4 X28C513LCC:s on a single substrate
class xm28c020_device : public x28_device<18, 128, 100, 5000>
{
public:
	xm28c020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// XM28C040: 4Mbit = 512 kbytes, 256 bytes per page;
// comprised of 4 X28C010:s on a single substrate
class xm28c040_device : public x28_device<19, 256, 100, 5000>
{
public:
	xm28c040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// Better-than-real devices:

// X28F256: 256kbit = 32k bytes, 64 bytes per page, like the X25C256, but 
// with T_WC=0, i.e., an infinitely fast write cycle.
// This uses only the Byte Load Cycle timer and also programs immediately on reading;
// The Write Cycle is infinitely quick, any pending writes are immediately committed
// and ready to be returned without needing Toggle Bit polling or /DATA polling.
class x28f256_device : public x28_device<15, 64, 100, 0, true>
{
public:
	x28f256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// device type declarations
DECLARE_DEVICE_TYPE(X28C64, x28c64_device)
DECLARE_DEVICE_TYPE(X28C256, x28c256_device)
DECLARE_DEVICE_TYPE(X28HC256, x28hc256_device)
DECLARE_DEVICE_TYPE(X28C512, x28c512_device)
DECLARE_DEVICE_TYPE(X28C010, x28c010_device)
DECLARE_DEVICE_TYPE(XM28C020, xm28c020_device)
DECLARE_DEVICE_TYPE(XM28C040, xm28c040_device)
DECLARE_DEVICE_TYPE(X28F256, x28f256_device)

#include "x28.ipp"

#endif // MAME_MACHINE_X28_H
