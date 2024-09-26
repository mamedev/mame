// license:BSD-3-Clause
// copyright-holders:Brett Wyer, Raphael Nabet
/*****************************************************************************
 *
 * includes/corvushd.h
 *
 * Implementation of the Corvus Systems Flat Cable Hard Disk controller
 *
 * Corvus Model 6 (5 MB): IMI 5006H hard disk (-chs 144,4,20 -ss 512)
 * Corvus Model 11 (10 MB): IMI 5012H hard disk (-chs 358,3,20 -ss 512)
 * Corvus Model 20 (20 MB): IMI 5018H hard disk (-chs 388,5,20 -ss 512)
 *
 ****************************************************************************/

#ifndef MAME_MACHINE_CORVUSHD_H
#define MAME_MACHINE_CORVUSHD_H

#pragma once

#include "imagedev/harddriv.h"
#include <cctype>

class corvus_hdc_device :  public device_t
{
public:
	// Status Register Bits
	static constexpr uint8_t CONTROLLER_BUSY      = 0x80; // Set = Busy, Clear = Ready
	static constexpr uint8_t CONTROLLER_DIRECTION = 0x40; // Set = Controller->Host, Clear = Host->Controller

	// construction/destruction
	corvus_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();
	void write(uint8_t data);
	uint8_t status_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(process_timeout);
	TIMER_CALLBACK_MEMBER(complete_function);

private:
	static constexpr unsigned MAX_COMMAND_SIZE = 4096;   // The maximum size of a command packet (the controller only has 5K of RAM...)

	// Sector addressing scheme for Rev B/H drives used in various commands (Called a DADR in the docs)
	struct dadr_t {
		uint8_t address_msn_and_drive;// Most significant nibble: Most significant nibble of sector address, Least significant nibble: Drive #
		uint8_t address_lsb;          // Least significant byte of sector address
		uint8_t address_mid;          // Middle byte of sector address
	};

	uint8_t   m_status;             // Controller status byte (DIRECTION + BUSY/READY)

	// Prep mode
	bool    m_prep_mode;          // Whether the controller is in Prep Mode or not
	uint8_t   m_prep_drv;           // If in prep mode, Corvus drive id (1..15) being prepped

	// Physical drive info
	uint8_t   m_sectors_per_track;  // Number of sectors per track for this drive
	uint8_t   m_tracks_per_cylinder;// Number of tracks per cylinder (heads)
	uint16_t  m_cylinders_per_drive;// Number of cylinders per drive

	// Command Processing
	uint16_t  m_offset;             // Current offset into raw_data buffer
	bool    m_awaiting_modifier;  // We've received a two-byte command and we're waiting for the mod
	uint16_t  m_recv_bytes;         // Number of bytes expected to be received from Host
	uint16_t  m_xmit_bytes;         // Number of bytes expected to be transmitted to host

	// Timing-related values
	uint16_t  m_last_cylinder;      // Last cylinder accessed - for calculating seek times
	uint32_t  m_delay;              // Delay in microseconds for callback
	emu_timer   *m_timeout_timer; // Four-second timer for timeouts
	emu_timer *m_cmd_timer;
	bool   m_invalid_command_flag;       // I hate this, but it saves a lot more tests

	//
	// Union below represents both an input and output buffer and interpretations of it
	//
	union {
		//
		// Raw Buffer
		//
		uint8_t       raw_data[MAX_COMMAND_SIZE];
		//
		// Basic interpretation of code and modifier
		//
		struct {
			uint8_t   code;       // First byte of data is the code (command)
			uint8_t   modifier;   // Second byte of data is the modifier
		} command;
		//
		// Basic response code
		//
		struct {
			uint8_t   status;     // Status code returned by the command executed
		} single_byte_response;
		//
		// Read sector command
		//
		struct {
			uint8_t   code;       // Command code
			dadr_t  dadr;       // Encoded drive and sector to read
		} read_sector_command;
		//
		// 128-byte Read Sector response
		//
		struct {
			uint8_t   status;     // Status code returned by command executed
			uint8_t   data[128];  // Data returned from read
		} read_128_response;
		//
		// 256-byte Read Sector response
		//
		struct {
			uint8_t   status;     // Status code returned by command executed
			uint8_t   data[256];  // Data returned from read
		} read_256_response;
		//
		// 512-byte Read Sector response
		//
		struct {
			uint8_t   status;     // Status code returned by command executed
			uint8_t   data[512];  // Data returned by read
		} read_512_response;
		//
		// Write 128-byte sector command
		//
		struct {
			uint8_t   code;       // Command code
			dadr_t  dadr;       // Encoded drive and sector to write
			uint8_t   data[128];  // Data to be written
		} write_128_command;
		//
		// Write 256-byte sector command
		//
		struct {
			uint8_t   code;       // Command code
			dadr_t  dadr;       // Encoded drive and sector to write
			uint8_t   data[256];  // Data to be written
		} write_256_command;
		//
		// Write 512-byte sector command
		//
		struct {
			uint8_t   code;       // Command Code
			dadr_t  dadr;       // Encoded drive and sector to write
			uint8_t   data[512];  // Data to be written
		} write_512_command;
		//
		// Semaphore Lock command
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   modifier;   // Command code modifier
			uint8_t   name[8];    // Semaphore name
		} lock_semaphore_command;
		//
		// Semaphore Unlock command
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   modifier;   // Command code modifier
			uint8_t   name[8];    // Semaphore name
		} unlock_semaphore_command;
		//
		// Semaphore Lock/Unlock response
		//
		struct {
			uint8_t   status;     // Disk access status
			uint8_t   result;     // Semaphore action status
			uint8_t   unused[10]; // Unused
		} semaphore_locking_response;
		//
		// Initialize Semaphore table command
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   modifier;   // Command code modifier
			uint8_t   unused[3];  // Unused
		} init_semaphore_command;
		//
		// Semaphore Status command
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   modifier;   // Command code modifier
			uint8_t   zero_three; // Don't ask me...
			uint8_t   unused[2];  // Unused
		} semaphore_status_command;
		//
		// Semaphore Status response
		//
		struct {
			uint8_t   status;     // Disk access status
			uint8_t   table[256]; // Contents of the semaphore table
		} semaphore_status_response;
		//
		// Get Drive Parameters command (0x10)
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   drive;      // Drive number (starts at 1)
		} get_drive_parameters_command;
		//
		// Get Drive Parameters command response
		//
		struct {
			uint8_t   status;                     // Status code returned by command executed
			uint8_t   firmware_desc[31];          // Firmware string description
			uint8_t   firmware_rev;               // Firmware revision number
			uint8_t   rom_version;                // ROM Version
			struct {
				uint8_t   sectors_per_track;      // Sectors/Track
				uint8_t   tracks_per_cylinder;    // Tracks/Cylinder (heads)
				struct {
					uint8_t   lsb;
					uint8_t   msb;
				} cylinders_per_drive;          // Byte-flipped Cylinders/Drive
			} track_info;
			struct {
				uint8_t   lsb;                    // Least significant byte
				uint8_t   midb;                   // Middle byte
				uint8_t   msb;                    // Most significant byte
			} capacity;                         // 24-bit value, byte-flipped (lsb..msb)
			uint8_t   unused[16];
			uint8_t   interleave;                 // Interleave factor
			struct {
				uint8_t   mux_parameters[12];
				uint8_t   pipe_name_table_ptr[2]; // Pointer to table of 64 entries, 8 bytes each (table of names)
				uint8_t   pipe_ptr_table_ptr[2];  // Pointer to table of 64 entries, 8 bytes each.  See pp. 29 - Mass Storage GTI
				uint8_t   pipe_area_size[2];      // Size of pipe area (lsb, msb)
				struct {
					uint8_t   track_offset[2];
				} vdo_table[7];                 // Virtual drive table
				uint8_t   lsi11_vdo_table[8];
				uint8_t   lsi11_spare_table[8];
			} table_info;
			uint8_t   drive_number;               // Physical drive number
			struct {
				uint8_t   lsb;                    // Least
				uint8_t   midb;                   // Middle
				uint8_t   msb;                    // Most
			} physical_capacity;                // Physical capacity of drive
		} drive_param_response;
		//
		// 2-byte Boot command (0x14)
		//
		struct {
			uint8_t   code;       // Command code
			uint8_t   boot_block; // Which boot block to read (0-7)
		} old_boot_command;
		//
		// Put drive into prep mode command (0x11)
		//
		struct {
			uint8_t   code;               // Command code
			uint8_t   drive;              // Drive number (starts at 1)
			uint8_t   prep_block[512];    // Machine code payload
		} prep_mode_command;
		//
		// Read Firmware command (Prep Mode 0x32)
		//
		struct {
			uint8_t   code;       // Command Code
			uint8_t   encoded_h_s;// Encoded Head (bits 7-5) / Sector (bits 4-0)
		} read_firmware_command;
		//
		// Write Firmware command (Prep Mode 0x33)
		//
		struct {
			uint8_t   code;       // Command Code
			uint8_t   encoded_h_s; // Encoded Head (bits 7-5) / Sector (bits 4-0)
			uint8_t   data[512];  // Data to be written
		} write_firmware_command;
		//
		// Format Drive command (Prep Mode 0x01)
		//
		// Note that the following is a BLATANT ASSUMPTION.  Technically, the Format Drive command
		// uses a variable-length buffer for the pattern.  Unfortunately, the docs don't explain how to determine the
		// length of the buffer passed.  I assume it's a timeout; however, the docs happen to say that
		// all Corvus diagnostic programs send 513 bytes total, including the command, so I'm going with that.
		//
		struct {
			uint8_t   code;       // Command Code
			uint8_t   pattern[512]; // Pattern to be written
		} format_drive_revbh_command;
		//
		// Verify Drive command (Prep Mode 0x07)
		//
		// On the real Corvus controller, this is a variable length response.  If the
		// number of bad sectors is greater than zero, an additional four bytes will
		// follow for each bad sector.  We don't emulate bad sectors, so we always
		// return a count of 0.  That makes this a fixed length response of 2 bytes.
		//
		struct {
			uint8_t   status;       // Disk access status
			uint8_t   bad_sectors;  // Number of bad sectors (always zero)
		} verify_drive_response;
	} m_buffer;

	// Structure of Block #1, the Disk Parameter Block
	struct disk_parameter_block_t {
		struct {
			uint8_t   lsb;
			uint8_t   msb;
		} spared_track[8];          // Spared track table (0xffff indicates end)
		uint8_t   interleave;         // Interleave factor
		uint8_t   reserved;
		struct {
			uint8_t track_offset[2];  // Virtual drive offsets (lsb, msb) 0xffff indicates unused
		} vdo_table[7];
		uint8_t   lsi11_vdo_table[8];
		uint8_t   lsi11_spare_table[8];
		uint8_t   reserved2[432];
		struct {
			uint8_t   lsb;
			uint8_t   msb;
		} revh_spare_table[16];
	};

	// Structure of Block #3, the Constellation Parameter Block
	struct constellation_parameter_block_t {
		uint8_t   mux_parameters[12];
		uint8_t   pipe_name_table_ptr[2];
		uint8_t   pipe_ptr_table_ptr[2];
		uint8_t   pipe_area_size[2];
		uint8_t   reserved[470];
		uint8_t   software_protection[12];
		uint8_t   serial_number[12];
	};

	// Structure of Block #7, the Semaphore Table Block
	struct semaphore_table_block_t {
		union {
			uint8_t   semaphore_table[256];           // Table consists of 256 bytes
			struct {
				uint8_t   semaphore_name[8];          // Each semaphore name is 8 bytes
			} semaphore_entry[32];                  // 32 Entries
		} semaphore_block;
		uint8_t   unused[256];                        // Remaining half of block is unused
	};

	// Command size structure (number of bytes to xmit and recv for each command)
	struct corvus_cmd_t {
		uint16_t  recv_bytes;                         // Number of bytes from host for this command
		uint16_t  xmit_bytes;                         // Number of bytes to return to host
	};

	void dump_buffer(uint8_t *buffer, uint16_t length);
	bool parse_hdc_command(uint8_t data);
	uint8_t corvus_write_sector(uint8_t drv, uint32_t sector, uint8_t *buffer, int len);
	uint8_t corvus_write_logical_sector(dadr_t *dadr, uint8_t *buffer, int len);
	uint8_t corvus_read_sector(uint8_t drv, uint32_t sector, uint8_t *buffer, int len);
	uint8_t corvus_read_logical_sector(dadr_t *dadr, uint8_t *buffer, int len);
	uint8_t corvus_lock_semaphore(uint8_t *name);
	uint8_t corvus_unlock_semaphore(uint8_t *name);
	uint8_t corvus_init_semaphore_table();
	uint8_t corvus_get_drive_parameters(uint8_t drv);
	uint8_t corvus_read_boot_block(uint8_t block);
	uint8_t corvus_enter_prep_mode(uint8_t drv, uint8_t *prep_block);
	uint8_t corvus_exit_prep_mode();
	uint8_t corvus_read_firmware_block(uint8_t head, uint8_t sector);
	uint8_t corvus_write_firmware_block(uint8_t head, uint8_t sector, uint8_t *buffer);
	uint8_t corvus_format_drive(uint8_t *pattern, uint16_t len);
	harddisk_image_device *corvus_hdc_file(int id);
	void corvus_process_command_packet(bool local_invalid_command_flag);

	corvus_cmd_t corvus_cmd[0xf5][0xc1];     // Command sizes and their return sizes
	corvus_cmd_t corvus_prep_cmd[0x82];      // Prep Command sizes and their return sizes
};


// device type definition
DECLARE_DEVICE_TYPE(CORVUS_HDC, corvus_hdc_device)

#endif // MAME_MACHINE_CORVUSHD_H
