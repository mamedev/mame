// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS022 */
#ifndef MAME_MACHINE_IGS022_H
#define MAME_MACHINE_IGS022_H

#pragma once


class igs022_device : public device_t
{
public:
	igs022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void IGS022_handle_command();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u32        m_kb_regs[0x100];

	void IGS022_do_dma(u16 src, u16 dst, u16 size, u16 mode);
	void IGS022_reset();

private:
	optional_shared_ptr<u16> m_sharedprotram;
	required_memory_region m_rom;
};


DECLARE_DEVICE_TYPE(IGS022, igs022_device)

#endif // MAME_MACHINE_IGS022_H
