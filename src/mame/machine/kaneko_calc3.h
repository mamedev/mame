// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/* CALC 3 */

#define VERBOSE_OUTPUT 0



class kaneko_calc3_device : public device_t
{
public:
	kaneko_calc3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE16_MEMBER(mcu_com0_w);
	DECLARE_WRITE16_MEMBER(mcu_com1_w);
	DECLARE_WRITE16_MEMBER(mcu_com2_w);
	DECLARE_WRITE16_MEMBER(mcu_com3_w);

	void reset_run_timer();
	void mcu_run();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_shared_ptr<UINT16> m_mcuram;
	int m_mcu_status;
	int m_mcu_command_offset;
	UINT16 m_mcu_crc;
	UINT8 m_decryption_key_byte;
	UINT8 m_alternateswaps;
	UINT8 m_shift;
	UINT8 m_subtracttype;
	UINT8 m_mode;
	UINT8 m_blocksize_offset;
	UINT16 m_dataend;
	UINT16 m_database;
	int m_data_header[2];
	UINT32 m_writeaddress;
	UINT32 m_writeaddress_current;
	UINT16 m_dsw_addr;
	UINT16 m_eeprom_addr;
	UINT16 m_poll_addr;
	UINT16 m_checksumaddress;
	emu_timer* m_runtimer;

	enum
	{
		MCU_RUN_TIMER
	};

	void mcu_init();
	void initial_scan_tables();
	void mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_);
	UINT8 shift_bits(UINT8 dat, int bits);
	int decompress_table(int tabnum, UINT8* dstram, int dstoffset);
};


extern const device_type KANEKO_CALC3;
