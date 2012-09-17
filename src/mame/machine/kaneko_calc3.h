/* CALC 3 */

#define CALC3_VERBOSE_OUTPUT 0


struct calc3_t
{
	int mcu_status;
	int mcu_command_offset;
	UINT16 mcu_crc;
	UINT8 decryption_key_byte;
	UINT8 alternateswaps;
	UINT8 shift;
	UINT8 subtracttype;
	UINT8 mode;
	UINT8 blocksize_offset;
	UINT16 dataend;
	UINT16 database;
	int data_header[2];
	UINT32 writeaddress;
	UINT32 writeaddress_current;
	UINT16 dsw_addr;
	UINT16 eeprom_addr;
	UINT16 poll_addr;
	UINT16 checksumaddress;
};




class kaneko_calc3_device : public device_t
{
public:
	kaneko_calc3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(calc3_mcu_ram_r);
	DECLARE_WRITE16_MEMBER(calc3_mcu_ram_w);

	DECLARE_WRITE16_MEMBER(calc3_mcu_com0_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com1_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com2_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com3_w);

	void reset_run_timer();

	void calc3_mcu_run(running_machine &machine);

protected:
	virtual void device_start();
	virtual void device_reset();

private:
	UINT16* m_calc3_mcuram;
	void calc3_mcu_init(running_machine &machine);
	void initial_scan_tables(running_machine& machine);

	calc3_t m_calc3;
	void calc3_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_);
	UINT8 shift_bits(UINT8 dat, int bits);
	int calc3_decompress_table(running_machine& machine, int tabnum, UINT8* dstram, int dstoffset);

	emu_timer* m_runtimer;

};


extern const device_type KANEKO_CALC3;

