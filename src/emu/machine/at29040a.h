// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/*
    ATMEL 29040a

    Michael Zapf
    September 2010: Rewritten as device
    February 2012: Rewritten as class
*/

#ifndef __AT29040__
#define __AT29040__

#include "emu.h"

extern const device_type AT29040A;

/*
    at29c40a state

    Command states (CMD_0 is the initial state):
    CMD_0: default state
    CMD_1: state after writing aa to 5555
    CMD_2: state after writing 55 to 2aaa

    Programming states (s_programming_0 is the initial state):
    PGM_0: default state
    PGM_1: a program and enable/disable lock command has been executed, but programming has not actually started.
    PGM_2: the programming buffer is being written to
    PGM_3: the programming buffer is being burnt to flash ROM
*/
enum  s_cmd_t
{
	CMD_0 = 0x0,
	CMD_1 = 0x1,
	CMD_2 = 0x2
};

enum  s_pgm_t
{
	PGM_0 = 0x0,
	PGM_1 = 0x1,
	PGM_2 = 0x2,
	PGM_3 = 0x3
};

class at29040a_device : public device_t, public device_nvram_interface
{
public:
	at29040a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	virtual void device_start(void);
	virtual void device_reset(void);
	virtual void device_stop(void);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void nvram_default();
	void nvram_read(emu_file &file);
	void nvram_write(emu_file &file);

private:
	void        sync_flags(void);

	UINT8*      m_eememory;

	bool        m_lower_bbl;        /* set when lower boot block lockout is enabled */
	bool        m_higher_bbl;       /* set when upper boot block lockout is enabled */
	bool        m_sdp;              /* set when in software data protect mode */

	bool        m_id_mode;          /* set when in chip id mode */
	s_cmd_t     m_cmd;              /* command state */
	bool        m_enabling_bbl;     /* set when a boot block lockout command is expecting its parameter */
	bool        m_long_sequence;    /* set if 0x80 command has just been executed (some command require this prefix) */
	s_pgm_t     m_pgm;              /* programming state */
	bool        m_enabling_sdb;     /* set when a sdp enable command is in progress */
	bool        m_disabling_sdb;    /* set when a sdp disable command is in progress */
	//bool        m_dirty;            /* set when the memory contents should be set */
	bool        m_toggle_bit;       // indicates flashing in progress (toggles for each query)
	UINT8*      m_programming_buffer;
	int         m_programming_last_offset;
	emu_timer*  m_programming_timer;
};

#define MCFG_AT29040A_ADD(_tag )    \
	MCFG_DEVICE_ADD(_tag, AT29040A, 0)

#endif
