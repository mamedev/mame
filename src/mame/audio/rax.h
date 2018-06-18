// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Acclaim RAX Sound Board

****************************************************************************/
#ifndef MAME_AUDIO_RAX_H
#define MAME_AUDIO_RAX_H

#pragma once

#include "cpu/adsp2100/adsp2100.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/dmadac.h"


class acclaim_rax_device : public device_t
{
public:
	// construction/destruction
	acclaim_rax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	READ16_MEMBER( data_r );
	WRITE16_MEMBER( data_w );

	READ16_MEMBER(adsp_control_r);
	WRITE16_MEMBER(adsp_control_w);
	WRITE16_MEMBER(ram_bank_w);
	WRITE16_MEMBER(rom_bank_w);

	READ16_MEMBER(host_r);
	WRITE16_MEMBER(host_w);

	void update_data_ram_bank();
	void adsp_irq(int which);
	void recompute_sample_rate(int which);

	TIMER_DEVICE_CALLBACK_MEMBER( dma_timer_callback );

	void adsp_data_map(address_map &map);
	void adsp_io_map(address_map &map);
	void adsp_program_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<adsp2181_device>    m_cpu;
	required_shared_ptr<uint32_t>       m_adsp_pram;
	required_memory_bank                m_adsp_data_bank;

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
	uint8_t*        m_rom;


	/* sound output */
	uint16_t        m_size[2];
	uint16_t        m_incs[2];
	dmadac_sound_device *m_dmadac[2];
	timer_device    *m_reg_timer[2];
	timer_device    *m_sport_timer;
	uint32_t        m_ireg[2];
	uint16_t        m_ireg_base[2];

	uint32_t        m_data_bank;
	uint32_t        m_rom_bank;
	uint32_t        m_dmovlay_val;

	required_device<generic_latch_16_device> m_data_in;
	required_device<generic_latch_16_device> m_data_out;

	timer_device *m_dma_timer;

	WRITE32_MEMBER(adsp_sound_tx_callback);

	TIMER_DEVICE_CALLBACK_MEMBER(adsp_irq0);
	TIMER_DEVICE_CALLBACK_MEMBER(sport0_irq);
	WRITE32_MEMBER(dmovlay_callback);
};

// device type definition
DECLARE_DEVICE_TYPE(ACCLAIM_RAX, acclaim_rax_device)

#endif // MAME_AUDIO_RAX_H
