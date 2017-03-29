// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS022 */
#ifndef MAME_MACHINE_IGS022_H
#define MAME_MACHINE_IGS022_H


class igs022_device : public device_t
{
public:
	igs022_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t* m_sharedprotram;
	void IGS022_handle_command();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint32_t        m_kb_regs[0x100];

	void IGS022_do_dma(uint16_t src, uint16_t dst, uint16_t size, uint16_t mode);
	void IGS022_reset();

};


extern const device_type IGS022;

#endif // MAME_MACHINE_IGS022_H
