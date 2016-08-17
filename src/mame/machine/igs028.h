// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS 028 */



class igs028_device : public device_t
{
public:
	igs028_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT16* m_sharedprotram;

	void IGS028_handle(void);

protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT32 olds_prot_addr(UINT16 addr);
	UINT32 olds_read_reg(UINT16 addr);
	void olds_write_reg( UINT16 addr, UINT32 val );
	void IGS028_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode);
};



extern const device_type IGS028;
