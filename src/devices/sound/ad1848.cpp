// license:BSD-3-Clause
// copyright-holders:Carl

// Analog Devices AD1848, main codec in Windows Sound System adapters

#include "emu.h"
#include "sound/ad1848.h"

#include "sound/volt_reg.h"
#include "speaker.h"


const device_type AD1848 = device_creator<ad1848_device>;

ad1848_device::ad1848_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AD1848, "Analog Devices AD1848", tag, owner, clock, "ad1848", __FILE__),
	m_irq_cb(*this),
	m_drq_cb(*this),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac")
{
}

static MACHINE_CONFIG_FRAGMENT( ad1848_config )
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ldac", DAC_16BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.5) // unknown DAC
	MCFG_SOUND_ADD("rdac", DAC_16BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.5) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "ldac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "ldac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_SOUND_ROUTE_EX(0, "rdac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "rdac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END

machine_config_constructor ad1848_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ad1848_config );
}

void ad1848_device::device_start()
{
	m_timer = timer_alloc(0, nullptr);
	m_irq_cb.resolve_safe();
	m_drq_cb.resolve_safe();
	save_item(NAME(m_regs.idx));
	save_item(NAME(m_addr));
	save_item(NAME(m_stat));
	save_item(NAME(m_sam_cnt));
	save_item(NAME(m_samples));
	save_item(NAME(m_count));
	save_item(NAME(m_play));
	save_item(NAME(m_mce));
	save_item(NAME(m_trd));
	save_item(NAME(m_irq));
}

void ad1848_device::device_reset()
{
	memset(&m_regs.idx[0], '\0', sizeof(m_regs));
	m_addr = 0;
	m_stat = 0;
	m_sam_cnt = 0;
	m_samples = 0;
	m_play = false;
	m_irq = false;
}

READ8_MEMBER(ad1848_device::read)
{
	switch(offset)
	{
		case 0:
			return m_addr | (m_mce ? 0x40 : 0) | (m_trd ? 0x20 : 0);
		case 1:
			return m_regs.idx[m_addr];
		case 2:
			return m_stat | (m_irq ? 1 : 0);
		case 3:
			break; // capture
	}
	return 0;
}

WRITE8_MEMBER(ad1848_device::write)
{
	const int div_factor[] = {3072, 1536, 896, 768, 448, 384, 512, 2560};
	switch(offset)
	{
		case 0:
			m_addr = data & 0xf;
			m_mce = data & 0x40 ? true : false;
			m_trd = data & 0x20 ? true : false;
			break;
		case 1:
			if(m_addr == 12)
				return;
			m_regs.idx[m_addr] = data;
			switch(m_addr)
			{
				case 8:
					if(m_mce)
						return;
					m_regs.dform &= 0x7f;
					break;
				case 9:
				{
					m_play = (data & 1) ? true : false;
					attotime rate = m_play ? attotime::from_hz(((m_regs.dform & 1) ? XTAL_24_576MHz : XTAL_16_9344MHz)
							/ div_factor[(m_regs.dform >> 1) & 7]) : attotime::never;
					m_timer->adjust(rate, 0 , rate);
					m_drq_cb(m_play ? ASSERT_LINE : CLEAR_LINE);
					break;
				}
				case 14:
					m_count = (m_regs.ubase << 8) | m_regs.lbase;
					break;
			}
			break;
		case 2:
			m_irq_cb(CLEAR_LINE);
			m_irq = false;
			if(m_regs.iface & 1)
				m_play = true;
			break;
		case 3:
			break; // playback
	}
}

READ8_MEMBER(ad1848_device::dack_r)
{
	m_drq_cb(CLEAR_LINE);
	return 0; // not implemented
}

WRITE8_MEMBER(ad1848_device::dack_w)
{
	if(!m_play)
		return;
	m_samples = (m_samples << 8) | data;
	m_sam_cnt++;
	switch(m_regs.dform >> 4)
	{
		case 0:
		case 2:
		case 6:
			m_sam_cnt = 0;
			break;
		case 1:
		case 3:
		case 4:
		case 7:
			if(m_sam_cnt == 2)
				m_sam_cnt = 0;
			break;
		case 5:
			if(m_sam_cnt == 4)
				m_sam_cnt = 0;
			break;
	}
	if(!m_sam_cnt)
		m_drq_cb(CLEAR_LINE);
}

void ad1848_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(!m_play)
		return;
	switch(m_regs.dform >> 4)
	{
		case 0: // 8bit mono
			m_ldac->write(m_samples << 8);
			m_rdac->write(m_samples << 8);
			break;
		case 1: // 8bit stereo
			m_ldac->write(m_samples << 8);
			m_rdac->write(m_samples & 0xff00);
			break;
		case 2: // ulaw mono
		case 3: // ulaw stereo
		case 6: // alaw mono
		case 7: // alaw stereo
			break;
		case 4: // 16bit mono
			m_ldac->write(m_samples ^ 0x8000);
			m_rdac->write(m_samples ^ 0x8000);
			break;
		case 5: // 16bit stereo
			m_ldac->write(m_samples ^ 0x8000);
			m_rdac->write((m_samples >> 16) ^ 0x8000);
			break;
	}
	if(!m_count)
	{
		if(m_regs.pinc & 2)
		{
			if(m_trd)
				m_play = false;
			m_irq_cb(ASSERT_LINE);
			m_irq = true;
		}
		m_count = (m_regs.ubase << 8) | m_regs.lbase;
	}
	else
		m_count--;
	m_drq_cb(ASSERT_LINE);
}
