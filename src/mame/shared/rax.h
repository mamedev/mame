// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Acclaim RAX Sound Board

****************************************************************************/
#ifndef MAME_SHARED_RAX_H
#define MAME_SHARED_RAX_H

#pragma once

#include "cpu/adsp2100/adsp2100.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/dmadac.h"


class acclaim_rax_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	acclaim_rax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t data_r();
	void data_w(uint16_t data);

	void update_data_ram_bank();
	void adsp_irq(int which);
	void recompute_sample_rate(int which);

	TIMER_DEVICE_CALLBACK_MEMBER(dma_timer_callback);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void adsp_sound_tx_callback(offs_t offset, uint32_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(adsp_irq0);
	TIMER_DEVICE_CALLBACK_MEMBER(sport0_irq);
	void dmovlay_callback(uint32_t data);

	uint16_t adsp_control_r(offs_t offset);
	void adsp_control_w(offs_t offset, uint16_t data);
	void ram_bank_w(uint16_t data);
	void rom_bank_w(uint16_t data);

	uint16_t host_r();
	void host_w(uint16_t data);

	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_io_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;

	required_device<adsp2181_device>    m_cpu;
	required_device_array<dmadac_sound_device, 2> m_dmadac;
	required_device<timer_device>       m_reg_timer;
	required_device<timer_device>       m_dma_timer;
	required_shared_ptr<uint32_t>       m_adsp_pram;
	required_memory_bank                m_adsp_data_bank;
	required_region_ptr<uint8_t>        m_rom;

	required_device<generic_latch_16_device> m_data_in;
	required_device<generic_latch_16_device> m_data_out;

	uint32_t m_adsp_snd_pf0;

	struct
	{
		uint16_t bdma_internal_addr;
		uint16_t bdma_external_addr;
		uint16_t bdma_control;
		uint16_t bdma_word_count;
	} m_adsp_regs;

	address_space *m_program;
	address_space *m_data;

	uint16_t        m_control_regs[32];


	// sound output
	uint16_t        m_size[2];
	uint16_t        m_incs[2];
	uint32_t        m_ireg[2];
	uint16_t        m_ireg_base[2];

	uint32_t        m_data_bank;
	uint32_t        m_rom_bank;
	uint32_t        m_dmovlay_val;

	std::unique_ptr<uint16_t[]> m_banked_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(ACCLAIM_RAX, acclaim_rax_device)

#endif // MAME_SHARED_RAX_H
