// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef ISA_DECTALK_H_
#define ISA_DECTALK_H_

#include "emu.h"
#include "isa.h"
#include "sound/dac.h"
#include "cpu/i86/i186.h"
#include "cpu/tms32010/tms32010.h"

class dectalk_isa_device : public device_t,
						public device_isa8_card_interface
{
public:
	dectalk_isa_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ8_MEMBER(read);

	DECLARE_WRITE16_MEMBER(status_w);
	DECLARE_READ16_MEMBER(cmd_r);
	DECLARE_WRITE16_MEMBER(data_w);
	DECLARE_READ16_MEMBER(data_r);
	DECLARE_READ16_MEMBER(host_irq_r);
	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);
	DECLARE_WRITE16_MEMBER(dac_w);
	DECLARE_READ16_MEMBER(dsp_dma_r);
	DECLARE_WRITE16_MEMBER(dsp_dma_w);
	DECLARE_WRITE16_MEMBER(output_ctl_w);
	DECLARE_READ16_MEMBER(bio_line_r);
	DECLARE_WRITE16_MEMBER(irq_line_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT16 m_cmd, m_stat, m_data, m_dsp_dma, m_ctl;
	UINT8 m_dma, m_vol, m_bio;

	required_device<i80186_cpu_device> m_cpu;
	required_device<dac_device> m_dac;
	required_device<cpu_device> m_dsp;
};

extern const device_type ISA8_DECTALK;

#endif /* ISA_DECTALK_H_ */
