// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Michael Zapf
/*
    strata.h: header file for strata.c
*/

class strataflash_device : public device_t, public device_nvram_interface
{
public:
	strataflash_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// 8-bit access
	DECLARE_READ8_MEMBER( read8 );
	DECLARE_WRITE8_MEMBER( write8 );

	// 16-bit access
	DECLARE_READ16_MEMBER( read16 );
	DECLARE_WRITE16_MEMBER( write16 );

protected:
	// device-level overrides
	void device_config_complete();

	void device_start();

	void nvram_default();
	void nvram_read(emu_file &file);
	void nvram_write(emu_file &file);

private:

	// bus width for 8/16-bit handlers
	enum bus_width_t
	{
		bw_8,
		bw_16
	};

	UINT16 read8_16(address_space& space, offs_t offset, bus_width_t bus_width);
	void   write8_16(address_space& space, offs_t offset, UINT16 data, bus_width_t bus_width);

	enum fm_mode_t
	{
		FM_NORMAL,      // normal read/write
		FM_READID,      // read ID
		FM_READQUERY,   // read query
		FM_READSTATUS,  // read status
		FM_WRITEPART1,  // first half of programming, awaiting second
		FM_WRBUFPART1,  // first part of write to buffer, awaiting second
		FM_WRBUFPART2,  // second part of write to buffer, awaiting third
		FM_WRBUFPART3,  // third part of write to buffer, awaiting fourth
		FM_WRBUFPART4,  // fourth part of write to buffer
		FM_CLEARPART1,  // first half of clear, awaiting second
		FM_SETLOCK,     // first half of set master lock/set block lock
		FM_CONFPART1,   // first half of configuration, awaiting second
		FM_WRPROTPART1  // first half of protection program, awaiting second
	};

	fm_mode_t   m_mode;             // current operation mode
	int         m_hard_unlock;      // 1 if RP* pin is at Vhh (not fully implemented)
	int         m_status;           // current status
	int         m_master_lock;      // master lock flag
	offs_t      m_wrbuf_base;       // start address in write buffer command
	int         m_wrbuf_len;        // count converted into byte length in write buffer command
	int         m_wrbuf_count;      // current count in write buffer command
	UINT8*      m_wrbuf;            // write buffer used by write buffer command
	UINT8*      m_flashmemory;      // main FEEPROM area
	UINT8*      m_blocklock;        // block lock flags
	UINT8*      m_prot_regs;        // protection registers
};

extern const device_type STRATAFLASH;

#define MCFG_STRATAFLASH_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, STRATAFLASH, 0)
