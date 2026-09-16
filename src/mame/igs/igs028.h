// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS 028 */
#ifndef MAME_IGS_IGS028_H
#define MAME_IGS_IGS028_H

#pragma once


class igs028_device : public device_t
{
public:
	igs028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void handle_command();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	optional_shared_ptr<uint16_t> m_sharedprotram;
	required_region_ptr<uint8_t> m_rom;

	uint32_t prot_addr(uint16_t addr);
	uint32_t read_reg(uint16_t addr);
	void write_reg( uint16_t addr, uint32_t val );
	void do_dma(uint16_t src, uint16_t dst, uint16_t size, uint16_t mode);
};


DECLARE_DEVICE_TYPE(IGS028, igs028_device)

#endif // MAME_IGS_IGS028_H
