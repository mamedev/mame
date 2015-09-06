// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/*
    ATMEL AT29 family

    Michael Zapf
    August 2015
*/

#ifndef __AT29X__
#define __AT29X__

#include "emu.h"

extern const device_type AT29C020;
extern const device_type AT29C040;
extern const device_type AT29C040A;

enum  s_cmd_t
{
	CMD_0 = 0,
	CMD_1,
	CMD_2
};

enum  s_pgm_t
{
	PGM_0 = 0,
	PGM_1,
	PGM_2,
	PGM_3
};

class at29x_device : public device_t, public device_nvram_interface
{
public:
	at29x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	virtual void device_start(void);
	virtual void device_reset(void);
	virtual void device_stop(void);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void       nvram_default();
	void       nvram_read(emu_file &file);
	void       nvram_write(emu_file &file);

	int        get_sector_number(offs_t address) { return address / m_sector_size; }

	int        m_memory_size;   // bytes
	int        m_word_width;
	int        m_device_id;
	int        m_sector_size;
	int        m_cycle_time;        // ms
	int        m_boot_block_size;
	int        m_version;
	int        m_address_mask;
	int        m_sector_mask;

private:
	void        sync_flags(void);

	UINT8*      m_eememory;

	bool        m_lower_bbl;              // set when lower boot block lockout is enabled
	bool        m_higher_bbl;             // set when upper boot block lockout is enabled
	bool        m_sdp;                    // set when in software data protect mode

	bool        m_id_mode;                // set when in chip id mode
	s_cmd_t     m_cmd;                    // command state
	bool        m_enabling_bbl;           // set when a boot block lockout command is expecting its parameter
	bool        m_long_sequence;          // set if 0x80 command has just been executed (some command require this prefix)
	s_pgm_t     m_pgm;                    // programming state
	bool        m_enabling_sdb;           // set when a sdp enable command is in progress
	bool        m_disabling_sdb;          // set when a sdp disable command is in progress
	bool        m_toggle_bit;             // indicates flashing in progress (toggles for each query)

	UINT8*      m_programming_buffer;
	int         m_programming_last_offset;
	emu_timer*  m_programming_timer;
};

/*
    Variants
*/
class at29c020_device : public at29x_device
{
public:
	at29c020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class at29c040_device : public at29x_device
{
public:
	at29c040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class at29c040a_device : public at29x_device
{
public:
	at29c040a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

#define MCFG_AT29C020_ADD(_tag )    \
	MCFG_DEVICE_ADD(_tag, AT29C020, 0)

#define MCFG_AT29C040_ADD(_tag )    \
	MCFG_DEVICE_ADD(_tag, AT29C040, 0)

#define MCFG_AT29C040A_ADD(_tag )    \
	MCFG_DEVICE_ADD(_tag, AT29C040A, 0)

#endif
