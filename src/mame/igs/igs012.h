// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert

#ifndef MAME_IGS_IGS012_H
#define MAME_IGS_IGS012_H

#pragma once

class igs012_device : public device_t
{
public:
	igs012_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void prot_reset_w(u16 data);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void prot_mode_w(offs_t offset, u8 data);
	void prot_inc_w(offs_t offset, u8 data);
	void prot_dec_inc_w(offs_t offset, u8 data);
	void prot_dec_copy_w(offs_t offset, u8 data);
	void prot_copy_w(offs_t offset, u8 data);
	void prot_swap_w(offs_t offset, u8 data);
	u16 prot_r();

	u8 m_prot = 0;
	u8 m_prot_swap = 0;
	u8 m_prot_mode = 0;
};


DECLARE_DEVICE_TYPE(IGS012, igs012_device)

#endif // MAME_IGS_IGS012_H
