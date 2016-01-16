// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS022 */


class igs022_device : public device_t
{
public:
	igs022_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	UINT16* m_sharedprotram;
	void IGS022_handle_command();

protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT32        m_kb_regs[0x100];

	void IGS022_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode);
	void IGS022_reset();

};



extern const device_type IGS022;
