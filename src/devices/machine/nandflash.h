// license:BSD-3-Clause
// copyright-holders:Raphael Nabet

#ifndef MAME_MACHINE_NANDFLASH_H
#define MAME_MACHINE_NANDFLASH_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> nand_device
class nand_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	nand_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	auto rnb_wr_callback() { return m_write_rnb.bind(); }

	int is_present();
	int is_protected();
	int is_busy();

	uint8_t data_r();
	void command_w(uint8_t data);
	void address_w(uint8_t data);
	void data_w(uint8_t data);

protected:
	enum sm_mode_t : uint8_t
	{
		SM_M_INIT,       // initial state
		SM_M_READ,       // read page data
		SM_M_PROGRAM,    // program page data
		SM_M_ERASE,      // erase block data
		SM_M_READSTATUS, // read status
		SM_M_READID,     // read ID
		SM_M_30,
		SM_M_RANDOM_DATA_INPUT,
		SM_M_RANDOM_DATA_OUTPUT
	};

	enum pointer_sm_mode_t : uint8_t
	{
		SM_PM_A, // accessing first 256-byte half of 512-byte data field
		SM_PM_B, // accessing second 256-byte half of 512-byte data field
		SM_PM_C  // accessing spare field
	};

	nand_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	optional_memory_region m_region;

	uint32_t m_page_data_size;       // 256 for a 2MB card, 512 otherwise
	uint32_t m_page_total_size;      // 264 for a 2MB card, 528 otherwise
	uint32_t m_num_pages;            // 8192 for a 4MB card, 16184 for 8MB, 32768 for 16MB,
								// 65536 for 32MB, 131072 for 64MB, 262144 for 128MB...
								// 0 means no card loaded
	int32_t m_log2_pages_per_block; // log2 of number of pages per erase block (usually 4 or 5)

	std::unique_ptr<uint8_t[]> m_feeprom_data; // FEEPROM data area
	std::unique_ptr<uint8_t[]> m_data_uid_ptr; // TODO: this probably should be contained to smartmed

	sm_mode_t m_mode;                 // current operation mode
	pointer_sm_mode_t m_pointer_mode; // pointer mode

	uint32_t m_page_addr; // page address pointer
	uint32_t m_byte_addr;          // byte address pointer
	uint32_t m_addr_load_ptr;      // address load pointer

	uint8_t m_status;             // current status
	uint8_t m_accumulated_status; // accumulated status

	std::unique_ptr<uint8_t[]> m_pagereg; // page register used by program command
	uint8_t m_id[5];                      // chip ID

	bool m_mode_3065;

	// Palm Z22 NAND has 512 + 16 byte pages but, for some reason, Palm OS writes 512 + 64 bytes when
	// programming a page, so we need to keep track of the number of bytes written so we can ignore the
	// last 48 (64 - 16) bytes or else the first 48 bytes get overwritten
	uint32_t m_program_byte_count;

	uint32_t m_id_len;
	uint32_t m_col_address_cycles;
	uint32_t m_row_address_cycles;
	uint32_t m_sequential_row_read;

	devcb_write_line m_write_rnb;
};

class samsung_k9f5608u0d_device : public nand_device
{
public:
	samsung_k9f5608u0d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f5608u0dj_device : public nand_device
{
public:
	samsung_k9f5608u0dj_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f5608u0b_device : public nand_device
{
public:
	samsung_k9f5608u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f2808u0b_device : public nand_device
{
public:
	samsung_k9f2808u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f1g08u0b_device : public nand_device
{
public:
	samsung_k9f1g08u0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f1g08u0m_device : public nand_device
{
public:
	samsung_k9f1g08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9lag08u0m_device : public nand_device
{
public:
	samsung_k9lag08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class samsung_k9f2g08u0m_device : public nand_device
{
public:
	samsung_k9f2g08u0m_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_tc58256aft_device : public nand_device
{
public:
	toshiba_tc58256aft_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// device type definition
DECLARE_DEVICE_TYPE(NAND, nand_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F5608U0D, samsung_k9f5608u0d_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F5608U0DJ, samsung_k9f5608u0dj_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F5608U0B, samsung_k9f5608u0b_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F2808U0B, samsung_k9f2808u0b_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F1G08U0B, samsung_k9f1g08u0b_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F1G08U0M, samsung_k9f1g08u0m_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9LAG08U0M, samsung_k9lag08u0m_device)
DECLARE_DEVICE_TYPE(SAMSUNG_K9F2G08U0M, samsung_k9f2g08u0m_device)
DECLARE_DEVICE_TYPE(TOSHIBA_TC58256AFT, toshiba_tc58256aft_device)

#endif // MAME_MACHINE_NANDFLASH_H
