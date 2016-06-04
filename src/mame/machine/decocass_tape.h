// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, David Haywood
#ifndef __DECOCASS_TAPE_H__
#define __DECOCASS_TAPE_H__

class decocass_tape_device : public device_t
{
public:
	decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~decocass_tape_device() {}

	UINT8 get_status_bits();
	UINT8 is_present();
	void change_speed(INT8 newspeed);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* regions within the virtual tape */
	enum tape_region
	{
		REGION_LEADER,              /* in clear leader section */
		REGION_LEADER_GAP,          /* in gap between leader and BOT */
		REGION_BOT,                 /* in BOT hole */
		REGION_BOT_GAP,             /* in gap between BOT hole and data */
		REGION_DATA_BLOCK_0,        /* in data block 0 */
		REGION_DATA_BLOCK_255 = REGION_DATA_BLOCK_0 + 255,
		REGION_EOT_GAP,             /* in gap between data and EOT hole */
		REGION_EOT,                 /* in EOT hole */
		REGION_TRAILER_GAP,         /* in gap between trailer and EOT */
		REGION_TRAILER              /* in clear trailer section */
	};


	/* bytes within a data block on a virtual tape */
	enum tape_byte
	{
		BYTE_PRE_GAP_0,             /* 34 bytes of gap, clock held to 0, no data */
		BYTE_PRE_GAP_33 = BYTE_PRE_GAP_0 + 33,
		BYTE_LEADIN,                /* 1 leadin byte, clocked value 0x00 */
		BYTE_HEADER,                /* 1 header byte, clocked value 0xAA */
		BYTE_DATA_0,                /* 256 bytes of data, clocked */
		BYTE_DATA_255 = BYTE_DATA_0 + 255,
		BYTE_CRC16_MSB,             /* 2 bytes of CRC, clocked MSB first, then LSB */
		BYTE_CRC16_LSB,
		BYTE_TRAILER,               /* 1 trailer byte, clocked value 0xAA */
		BYTE_LEADOUT,               /* 1 leadout byte, clocked value 0x00 */
		BYTE_LONGCLOCK,             /* 1 longclock byte, clock held to 1, no data */
		BYTE_POSTGAP_0,             /* 34 bytes of gap, no clock, no data */
		BYTE_POSTGAP_33 = BYTE_POSTGAP_0 + 33,
		BYTE_BLOCK_TOTAL            /* total number of bytes in block */
	};

	// internal state
	emu_timer *         m_tape_timer;              /* timer for running the tape */
	INT8                m_speed;              /* speed: <-1=fast rewind, -1=reverse, 0=stopped, 1=normal, >1=fast forward */
	tape_region         m_region;             /* current region */
	tape_byte           m_bytenum;            /* byte number within a datablock */
	UINT8               m_bitnum;             /* bit number within a byte */
	UINT32              m_clockpos;           /* the current clock position of the tape */
	UINT32              m_numclocks;          /* total number of clocks on the entire tape */
	UINT16              m_crc16[256];         /* CRC16 for each block */
	optional_region_ptr<UINT8> m_tape_data;

	const char *describe_state();
	TIMER_CALLBACK_MEMBER( tape_clock_callback );
};

extern const device_type DECOCASS_TAPE;

#define MCFG_DECOCASS_TAPE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECOCASS_TAPE, 0)

#endif
