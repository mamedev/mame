// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************

  28-series Parallel EEPROM sich as Xicor X28 etc.
  Caters for different speeds such as X28C256, X28HC256, etc.
  Caters for different storage sizes such as X28C64, X28C256, etc.

***************************************************************************/

#ifndef MAME_MACHINE_X28_IPP
#define MAME_MACHINE_X28_IPP

// Included here to provide context for editors; include guards in x28.h
// prevent this from actually doing anything.
#include "x28.h"

#pragma once


template<
	int AddressBits,
	uint32_t PageSizeBytes,
	uint32_t TBLCUsec,
	uint32_t TWCUsec,
	bool ProgramOnRead
>
void x28_device<AddressBits, PageSizeBytes, TBLCUsec, TWCUsec, ProgramOnRead>::write(uint32_t offset, uint8_t data)
{
	if (m_state == STATE_PROGRAMMING) {
		// An attempt to write during a programming cycle does nothing.
		// LOG("IN PROGRAMMING CYCLE: writing %02x @ %04x\n", data, offset);
		return;
	}

	if (m_t_blc_usec > 0) {
		// Adjust the time remaining for more writes to the same page.
		m_start_programming_timer->adjust(attotime::from_usec(m_t_blc_usec));
	}

	// LOG("write(%04x, %02x) in state %d\n", offset, data, m_state);
	if (offset >= TOTAL_SIZE_BYTES) {
		// Attempting to write outside the range of this device does nothing.
		return;
	}

	// Command sequence processing:
	if (m_state == STATE_IDLE) {
		// Detect if this is the initiation of a command sequence, but only if the current state is IDLE
		if ((offset == (0x5555 & ADDRESS_MASK)) && (data == 0xaa)){
			change_to_command_state(COMMAND_STATE_1);
		}
	} else if (m_state == STATE_BUFFERING) {
		// Detect if this is the second write in a command sequence
		if (m_command_state == COMMAND_STATE_1) {
			if ((offset == (0x2aaa & ADDRESS_MASK)) && (data == 0x55)) {
				// We're firmly within a protection command sequence.
				// Inhibit the actual writing of data during the subsequence programming cycle.
				m_program_buffer_to_eeprom = false;
				// LOG("m_program_buffer_to_eeprom -> %d\r\n", m_program_buffer_to_eeprom);
				change_to_command_state(COMMAND_STATE_2);
			} else {
				// Not, after all in a command sequence.
				change_to_command_state(COMMAND_STATE_NONE);
			}
		} else if (m_command_state == COMMAND_STATE_2) {
			if ((offset == (0x5555 & ADDRESS_MASK)) && (data == 0xa0)) {
				// We've received a complete "enable write protection" command, so we:
				// - Enable write protection, i.e., disable writes;
				m_software_data_protection_enabled = true;
				// - Note that we are no longer in a command sequence;
				change_to_command_state(COMMAND_STATE_NONE);
				// - Also enter the overall "protected write" state to potentially accept some writes.
				change_to_state(STATE_PROTECTED_WRITE);
				// - Note that this was the last written offset
				m_last_written_offset = offset;
				// - And that concludes what we do in this write cycle.
				return;
			} else if ((offset == (0x5555 & ADDRESS_MASK)) && (data == 0x80)) {
				change_to_command_state(COMMAND_STATE_PROTECION_DISABLE_3);
			} else  {
				command_state_machine_error();
			}
		} else if (m_command_state == COMMAND_STATE_PROTECION_DISABLE_3) {
			if ((offset == (0x5555 & ADDRESS_MASK)) && (data == 0xaa)) {
				change_to_command_state(COMMAND_STATE_PROTECION_DISABLE_4);
			} else {
				command_state_machine_error();
			}
		} else if (m_command_state == COMMAND_STATE_PROTECION_DISABLE_4) {
			if ((offset == (0x2aaa & ADDRESS_MASK)) && (data == 0x55)) {
				change_to_command_state(COMMAND_STATE_PROTECION_DISABLE_5);
			} else {
				command_state_machine_error();
			}
		} else if (m_command_state == COMMAND_STATE_PROTECION_DISABLE_5) {
			if ((offset == (0x5555 & ADDRESS_MASK)) && (data == 0x20)) {
				// We have now received a complete "disable write protection" command. So we:
				// - Disable write protection, i.e., enable writes.
				m_software_data_protection_enabled = false;
				// = Note that we're no longer in a command sequence.
				change_to_command_state(COMMAND_STATE_NONE);
				// - Write protection was disabled, and the preceding writes were just part of that command sequence.
				m_program_buffer_to_eeprom = false;
				// LOG("m_program_buffer_to_eeprom -> %d\r\n", m_program_buffer_to_eeprom);

				if (m_t_blc_usec > 0) {
					// Since we're explicitly starting the programming cycle, disable the timer
					m_start_programming_timer->enable(false);
				}

				// - Sart the programming cycle
				start_programming_cycle();
				// - and now we're done with this write.
				return;
			} else {
				command_state_machine_error();
			}
		}
	}

	if (m_state == STATE_IDLE || m_state == STATE_PROTECTED_WRITE) {
		// This is the first write that we will buffer.
		// At this point, the beginning of buffering data, we expect to program the data
		// we are about to buffer into EEPROM storage - unless of course we're write protected,
		// in which case we already know _not_ to program the buffer into the EEPROM.
		// If later on we detect a protection command sequence we will set this to 'false'
		// so that the command sequence (which will end up in the buffer)
		// does not get written to storage.
		m_program_buffer_to_eeprom = (!m_software_data_protection_enabled) || (m_state == STATE_PROTECTED_WRITE);
		// LOG("m_program_buffer_to_eeprom -> %d\r\n", m_program_buffer_to_eeprom);

		// We start to buffer a set of writes.
		// We do this even if we're write protected - m_program_buffer_to_eeprom protects us.
		change_to_state(STATE_BUFFERING);

		// We note which page we're starting to buffer, copy its current contents into the buffer
		// so the buffer can be written into on a byte by byte basis, before being written back
		// to storage during the programming cycle.
		m_buffering_page = offset & PAGE_MASK;
		const uint8_t *p = &(m_storage[m_buffering_page]);
		std::copy(p, p + PageSizeBytes, std::begin(m_page_buffer));
	}

	// Deliberately falling through after detecting the first write and changing state
	// from (IDLE or PROTECTED_WRITE) to BUFFERING

	if (m_state == STATE_BUFFERING) {
		// The datasheet for the X28C256 says that
		//   "the page address (A6 through A14) for each subsequent
		//   valid write cycle to the part during this operation must be
		//   the same as the initial page address."
		// but does not say anything about the chip verifying this, or the consequences
		// if this is no the case.
		// A valid interpretation is that the chip simply accepts the write into the buffer anyway,
		// at the appropriate offset within the page;
		// Another valid interpretation is to reject the write.
		// Here, I choose the latter: any writes to an address within a different page
		// are ignored, only those within the same page are accepted.
		if ((offset & PAGE_MASK) == m_buffering_page) {
			m_page_buffer[offset & PAGE_OFFSET_MASK] = data;
		}

		// Note where the last write occurred.
		m_last_written_offset = offset;
	}

	// if (m_software_data_protection_enabled) {
	//   LOG("X28C: write %02x to %x while write protected\n", data, offset);
	// }
}

template<
	int AddressBits,
	uint32_t PageSizeBytes,
	uint32_t TBLCUsec,
	uint32_t TWCUsec,
	bool ProgramOnRead
>
uint8_t x28_device<AddressBits, PageSizeBytes, TBLCUsec, TWCUsec, ProgramOnRead>::read(uint32_t offset)
{
	if (m_command_state != COMMAND_STATE_NONE) {
		// Per the X28C256 datasheet regarding the command sequence,
		//   "Note: Once initiated, the sequence of write operations
		//   should not be interrupted."
		// A read operation would seem to interrupt that sequence, and thus end that sequence.
		change_to_state(STATE_BUFFERING);
	}

	if (m_program_on_read) {
		if (m_state == STATE_BUFFERING || m_state == STATE_PROTECTED_WRITE) {
			// We have some buffered data or a change to enable write protection;
			// immediately write any buffered data to storage:
			// First, cancel any existing programming cycle timer
			if (m_t_blc_usec > 0) {
				m_start_programming_timer->enable(false);
			}
			// Then, start the programming cycle. If T_WC is 0, this will in turn also change the
			// state to STATE_IDLE.
			start_programming_cycle();
		}
	}

	uint8_t data = m_storage[offset];

	if (m_program_buffer_to_eeprom || (m_state == STATE_PROGRAMMING)) {
		// "/DATA Polling"
		// While programming or preparing to progam, if the client reads back the
		// last written byte, the returned bit 7 is the inverse of what was written.
		// But according to at least one datasheet:
		//   "If the X28C64 is in the protected state and an illegal write
		//    operation is attempted /DATA Polling will not operate."
		// so we only perform this if we're actually going to be writing to storage.
		if (m_program_buffer_to_eeprom && (offset == m_last_written_offset)) {
			data = data ^ INVERSE_DATA_BIT;
		}

		// While programming or preparing to progam, bit 6 of what is returned
		// alternates between 0 and 1.
		data = (data & ~TOGGLE_BIT) | m_toggle_bit;
		m_toggle_bit ^= TOGGLE_BIT;
	}

	return data;
}

#endif // MAME_MACHINE_X28_IPP
