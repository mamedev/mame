// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 IOP SPU device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPSPU_H
#define MAME_MACHINE_IOPSPU_H

#pragma once

#include "emu.h"
#include "cpu/mips/r3000.h"

class iop_spu_device : public device_t
{
public:
	template <typename T>
    iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&iop_tag)
    	: iop_spu_device(mconfig, tag, owner, clock)
    {
		m_iop.set_tag(std::forward<T>(iop_tag));
	}

    iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	uint16_t port_read();
	void port_write(uint16_t data);

	void dma_write(uint32_t data);
	void dma_done();

protected:
    virtual void device_start() override;
    virtual void device_reset() override;

	enum
	{
		STATUS_DMA_DONE		= (1 <<  7),
		STATUS_DMA_ACTIVE	= (1 << 10)
	};

	required_device<iop_device> m_iop;
	std::unique_ptr<uint16_t[]> m_ram;

	uint32_t m_status;
	uint32_t m_start_port_addr;
	uint32_t m_curr_port_addr;
	uint32_t m_unknown_0x19a;
};

DECLARE_DEVICE_TYPE(SONYIOP_SPU, iop_spu_device)

#endif // MAME_MACHINE_IOPSPU_H