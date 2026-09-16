// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HAL2 Audio Controller emulation

**********************************************************************/

#include "emu.h"
#include "hal2.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_READS       (1U << 2)
#define LOG_WRITES      (1U << 3)
#define LOG_ALL         (LOG_UNKNOWN | LOG_READS | LOG_WRITES)

#define VERBOSE         (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_HAL2, hal2_device, "hal2", "SGI HAL2")

hal2_device::hal2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SGI_HAL2, tag, owner, clock)
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
{
}

void hal2_device::device_start()
{
	save_item(NAME(m_isr));
	save_item(NAME(m_iar));
	save_item(NAME(m_idr));
	save_item(NAME(m_codeca_ctrl));
	save_item(NAME(m_codeca_channel));
	save_item(NAME(m_codeca_clock));
	save_item(NAME(m_codeca_channel_count));
	save_item(NAME(m_codecb_ctrl));
	save_item(NAME(m_codecb_channel));
	save_item(NAME(m_codecb_clock));
	save_item(NAME(m_codecb_channel_count));
	save_item(NAME(m_bres_clock_sel));
	save_item(NAME(m_bres_clock_inc));
	save_item(NAME(m_bres_clock_modctrl));
	save_item(NAME(m_bres_clock_freq));
	save_item(NAME(m_curr_dac));
}

void hal2_device::device_reset()
{
	m_isr = 0;
	m_iar = 0;
	memset(m_idr, 0, sizeof(uint16_t) * 4);
	memset(m_codeca_ctrl, 0, sizeof(uint16_t) * 2);
	m_codeca_channel = 0;
	m_codeca_clock = 0;
	m_codeca_channel_count = 0;
	memset(m_codecb_ctrl, 0, sizeof(uint16_t) * 2);
	m_codecb_channel = 0;
	m_codecb_clock = 0;
	m_codecb_channel_count = 0;
	memset(m_bres_clock_sel, 0, sizeof(uint16_t) * 3);
	memset(m_bres_clock_inc, 0, sizeof(uint16_t) * 3);
	memset(m_bres_clock_modctrl, 0, sizeof(uint16_t) * 3);
	memset(m_bres_clock_freq, 0, sizeof(uint16_t) * 3);
	for (int i = 0; i < 3; i++)
	{
		m_bres_clock_rate[i] = attotime::zero;
	}
	m_curr_dac = 0;
}

uint16_t hal2_device::read(offs_t offset)
{
	switch (offset)
	{
	case STATUS_REG:
		LOGMASKED(LOG_READS, "%s: HAL2 Status Read: %08x\n", machine().describe_context(), m_isr);
		return m_isr;
	case REVISION_REG:
		LOGMASKED(LOG_READS, "%s: HAL2 Revision Read: 0x4010\n", machine().describe_context());
		return 0x4010;

	case INDIRECT_DATA0_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 0 Read: %04x\n", machine().describe_context(), m_idr[0]);
		return m_idr[0];

	case INDIRECT_DATA1_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 1 Read: %04x\n", machine().describe_context(), m_idr[1]);
		return m_idr[1];

	case INDIRECT_DATA2_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 2 Read: %04x\n", machine().describe_context(), m_idr[2]);
		return m_idr[2];

	case INDIRECT_DATA3_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 3 Read: %04x\n", machine().describe_context(), m_idr[3]);
		return m_idr[3];
	}
	LOGMASKED(LOG_READS | LOG_UNKNOWN, "%s: Unknown HAL2 read: %08x\n", machine().describe_context(), 0x1fbd8000 + offset*4);
	return 0;
}

void hal2_device::write(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case STATUS_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Status Write: 0x%04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_WRITES, "    HAL2 Global Reset %s\n", (data & ISR_GLOBAL_RESET) ? "Inactive" : "Active");
		LOGMASKED(LOG_WRITES, "    HAL2 Codec Reset %s\n", (data & ISR_CODEC_RESET) ? "Inactive" : "Active");
		m_isr &= ~0x1c;
		m_isr |= data & 0x1c;
		break;
	case INDIRECT_ADDRESS_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Address Register Write: %04x\n", machine().describe_context(), data);
		m_iar = data;
		switch (data & IAR_TYPE)
		{
		case 0x1000:
			LOGMASKED(LOG_WRITES, "    DMA Port\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        Synth In\n");
				break;
			case 0x0200:
				LOGMASKED(LOG_WRITES, "        AES In\n");
				break;
			case 0x0300:
				LOGMASKED(LOG_WRITES, "        AES Out\n");
				break;
			case 0x0400:
			{
				LOGMASKED(LOG_WRITES, "        Codec A (DAC) Out\n");
				const uint32_t param = (data & IAR_PARAM) >> IAR_PARAM_SHIFT;
				switch (param)
				{
					case 1:
						LOGMASKED(LOG_WRITES, "            Control 1\n");
						if (data & IAR_ACCESS_SEL)
						{
							m_idr[0] = m_codeca_ctrl[0];
						}
						else
						{
							m_codeca_ctrl[0] = m_idr[0];
							m_codeca_channel = m_idr[0] & 3;
							m_codeca_clock = (m_idr[0] >> 3) & 3;
							m_codeca_channel_count = (m_idr[0] >> 8) & 3;
						}
						break;
					case 2:
						LOGMASKED(LOG_WRITES, "            Control 2\n");
						if (data & IAR_ACCESS_SEL)
						{
							m_idr[0] = m_codeca_ctrl[1];
						}
						else
						{
							m_codeca_ctrl[1] = m_idr[0];
						}
						break;
					default:
						LOGMASKED(LOG_WRITES, "            Unknown Register\n");
						break;
				}
				break;
			}
			case 0x0500:
			{
				LOGMASKED(LOG_WRITES, "        Codec B (ADC) Out\n");
				const uint32_t param = (data & IAR_PARAM) >> IAR_PARAM_SHIFT;
				switch (param)
				{
					case 1:
						LOGMASKED(LOG_WRITES, "            Control 1\n");
						if (data & IAR_ACCESS_SEL)
						{
							m_idr[0] = m_codecb_ctrl[0];
						}
						else
						{
							m_codecb_ctrl[0] = m_idr[0];
							m_codecb_channel = m_idr[0] & 3;
							m_codecb_clock = (m_idr[0] >> 3) & 3;
							m_codecb_channel_count = (m_idr[0] >> 8) & 3;
						}
						break;
					case 2:
						LOGMASKED(LOG_WRITES, "            Control 2\n");
						if (data & IAR_ACCESS_SEL)
						{
							m_idr[0] = m_codecb_ctrl[1];
						}
						else
						{
							m_codecb_ctrl[1] = m_idr[0];
						}
						break;
					default:
						LOGMASKED(LOG_WRITES, "            Unknown Register\n");
						break;
				}
				break;
			}
			case 0x0600:
				LOGMASKED(LOG_WRITES, "        Synth Control\n");
				break;
			default:
				LOGMASKED(LOG_WRITES, "        Unknown\n");
				break;
			}
			break;
		case 0x2000:
		{
			LOGMASKED(LOG_WRITES, "    Bresenham\n");
			uint32_t clock_gen = (data & IAR_NUM) >> IAR_NUM_SHIFT;
			if (clock_gen >= 1 && clock_gen <= 3)
			{
				LOGMASKED(LOG_WRITES, "        Bresenham Clock Gen %d\n", clock_gen);
				clock_gen--;
				const uint32_t param = (data & IAR_PARAM) >> IAR_PARAM_SHIFT;
				if (param == 1)
				{
					LOGMASKED(LOG_WRITES, "            Control 1 (Clock Select)\n");
					if (data & IAR_ACCESS_SEL)
					{
						m_idr[0] = m_bres_clock_sel[clock_gen];
					}
					else
					{
						m_bres_clock_sel[clock_gen] = m_idr[0];
						switch (m_idr[0])
						{
							case 0:  LOGMASKED(LOG_WRITES, "                48kHz\n"); break;
							case 1:  LOGMASKED(LOG_WRITES, "                44.1kHz\n"); break;
							case 2:  LOGMASKED(LOG_WRITES, "                Off\n"); break;
							default: LOGMASKED(LOG_WRITES, "                Unknown (%d)\n", m_idr[0]); break;
						}
						update_clock_freq(clock_gen);
					}
				}
				else if (param == 2)
				{
					LOGMASKED(LOG_WRITES, "            Control 2 (Inc/Mod Ctrl)\n");
					if (data & IAR_ACCESS_SEL)
					{
						m_idr[0] = m_bres_clock_inc[clock_gen];
						m_idr[1] = m_bres_clock_modctrl[clock_gen];
					}
					else
					{
						m_bres_clock_inc[clock_gen] = m_idr[0];
						m_bres_clock_modctrl[clock_gen] = m_idr[1];
						LOGMASKED(LOG_WRITES, "                Inc:%04x, ModCtrl:%04x\n", m_idr[0], m_idr[1]);
						update_clock_freq(clock_gen);
					}
				}
				else
				{
					LOGMASKED(LOG_WRITES, "            Unknown Param\n");
				}
			}
			else
			{
				LOGMASKED(LOG_WRITES, "        Unknown\n");
			}
			break;
		}

		case 0x3000:
			LOGMASKED(LOG_WRITES, "    Unix Timer\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        Unix Timer\n");
				break;
			default:
				LOGMASKED(LOG_WRITES, "        Unknown\n");
				break;
			}
			break;

		case 0x9000:
			LOGMASKED(LOG_WRITES, "    Global DMA Control\n");
			switch (data & IAR_NUM)
			{
			case 0x0100:
				LOGMASKED(LOG_WRITES, "        DMA Control\n");
				break;
			default:
				LOGMASKED(LOG_WRITES, "        Unknown\n");
				break;
			}
			break;
		}

		if (data & IAR_ACCESS_SEL)
			LOGMASKED(LOG_WRITES, "    Read\n");
		else
			LOGMASKED(LOG_WRITES, "    Write\n");

		LOGMASKED(LOG_WRITES, "    Parameter: %01x\n", (data & IAR_PARAM) >> 2);
		return;

	case INDIRECT_DATA0_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 0 Write: %04x\n", machine().describe_context(), data);
		m_idr[0] = data;
		return;

	case INDIRECT_DATA1_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 1 Write: %04x\n", machine().describe_context(), data);
		m_idr[1] = data;
		return;

	case INDIRECT_DATA2_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 2 Write: %04x\n", machine().describe_context(), data);
		m_idr[2] = data;
		return;

	case INDIRECT_DATA3_REG:
		LOGMASKED(LOG_WRITES, "%s: HAL2 Indirect Data Register 3 Write: %04x\n", machine().describe_context(), data);
		m_idr[3] = data;
		return;

	default:
		LOGMASKED(LOG_WRITES, "%s: Unknown HAL2 Write: %08x = %04x\n", machine().describe_context(), 0x1fbd8000 + offset*4, data);
		break;
	}
}

void hal2_device::update_clock_freq(int clock_gen)
{
	switch (m_bres_clock_sel[clock_gen])
	{
		case 0: // 48kHz
			m_bres_clock_freq[clock_gen] = 48000;
			break;
		case 1: // 44.1kHz
			m_bres_clock_freq[clock_gen] = 44100;
			break;
		default:
			m_bres_clock_freq[clock_gen] = 0;
			m_bres_clock_rate[clock_gen] = attotime::zero;
			return;
	}

	const uint32_t mod = 0x10000 - ((m_bres_clock_modctrl[clock_gen] + 1) - m_bres_clock_inc[clock_gen]);
	if (mod == 0)
		m_bres_clock_rate[clock_gen] = attotime::from_ticks(1, m_bres_clock_freq[clock_gen] * m_bres_clock_inc[clock_gen]);
	else
		m_bres_clock_rate[clock_gen] = attotime::from_ticks(mod, m_bres_clock_freq[clock_gen] * m_bres_clock_inc[clock_gen]);
}

attotime hal2_device::get_rate(const uint32_t channel)
{
	if ((channel == m_codeca_channel || channel == (3 - m_codeca_channel)) && m_codeca_clock > 0 && m_codeca_channel_count > 0)
		return m_bres_clock_rate[m_codeca_clock - 1] / m_codeca_channel_count;
	if ((channel == m_codecb_channel || channel == (3 - m_codecb_channel)) && m_codecb_clock > 0 && m_codecb_channel_count > 0)
		return m_bres_clock_rate[m_codecb_clock - 1] / m_codecb_channel_count;
	return attotime::zero;
}

void hal2_device::dma_write(uint32_t channel, int16_t data)
{
	if (channel >= 2)
		return;

	if (m_curr_dac)
		m_rdac->write(data);
	else
		m_ldac->write(data);

	m_curr_dac++;
	if (m_curr_dac == m_codeca_channel_count)
		m_curr_dac = 0;
}

void hal2_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "speaker", 0.25, 0);

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0);
	m_rdac->add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}
