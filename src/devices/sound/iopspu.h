// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony PlayStation 2 IOP SPU device skeleton
*
*   To Do:
*     Everything
*
*/

#ifndef MAME_MACHINE_IOPSPU_H
#define MAME_MACHINE_IOPSPU_H

#pragma once

#include "cpu/mips/mips1.h"
#include "machine/iopintc.h"

class iop_spu_device : public device_t, public device_sound_interface
{
public:
	template <typename T, typename U>
	iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&iop_tag, U &&intc_tag)
		: iop_spu_device(mconfig, tag, owner, clock)
	{
		m_iop.set_tag(std::forward<T>(iop_tag));
		m_intc.set_tag(std::forward<U>(intc_tag));
	}

	iop_spu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~iop_spu_device() override;

	uint16_t read(offs_t offset, uint16_t mem_mask = ~0);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t reg_read(int bank, uint32_t offset, uint16_t mem_mask);
	void reg_write(int bank, uint32_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t port_read(int bank);
	void port_write(int bank, uint16_t data);

	void dma_write(int bank, uint32_t data);
	void dma_done(int bank);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// HACK: This timer is currently used to trigger an interrupt after the auto-DMA-transferred buffer would have been
	//       mixed and played back, as the PS2 BIOS pulls a null return address and crashes if we trigger the auto-DMA-complete
	//       interrupt at the same time as the DMA-complete interrupt.
	TIMER_CALLBACK_MEMBER(autodma_done_timer_hack);

	enum
	{
		STATUS_DMA_DONE     = (1 <<  7),
		STATUS_DMA_ACTIVE   = (1 << 10)
	};

	class voice_t
	{
	public:
		voice_t() {}

		void write(uint32_t offset, uint16_t data);
		uint16_t read(uint32_t offset);

	protected:
		uint16_t m_unknown[8];
	};

	required_device<iop_device> m_iop;
	required_device<iop_intc_device> m_intc;
	std::unique_ptr<uint16_t[]> m_ram;

	struct iop_spu_core_t
	{
		voice_t m_voices[24];

		uint32_t m_start_port_addr;
		uint32_t m_curr_port_addr;

		uint16_t m_status;
		uint16_t m_unknown_0x19a;
		uint16_t m_autodma_ctrl;

		emu_timer *m_autodma_done_timer_hack;
	};

	iop_spu_core_t m_core[2];

	sound_stream *m_stream;
};

DECLARE_DEVICE_TYPE(SONYIOP_SPU, iop_spu_device)

#endif // MAME_MACHINE_IOPSPU_H
