// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************************************************************
 *
 * Intel XScale PXA255 peripheral emulation
 *
 * TODO:
 *   Most things
 *
 **************************************************************************/

#include "emu.h"
#include "pxa255.h"

#include "screen.h"
#include "speaker.h"

#define VERBOSE_LEVEL (0)

DEFINE_DEVICE_TYPE(PXA255_PERIPHERALS, pxa255_periphs_device, "pxa255_periphs", "Intel XScale PXA255 Peripherals")

pxa255_periphs_device::pxa255_periphs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PXA255_PERIPHERALS, tag, owner, clock)
	, m_gpio0_set_func(*this)
	, m_gpio0_clear_func(*this)
	, m_gpio0_in_func(*this)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_dmadac(*this, "dac%u", 1U)
	, m_palette(*this, "palette")
{
}

inline void ATTR_PRINTF(3,4) pxa255_periphs_device::verboselog(int n_level, const char *s_fmt, ... )
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine().describe_context(), buf );
	}
}

/*

  PXA255 Inter-Integrated-Circuit Sound (I2S) Controller

  pg. 489 to 504, PXA255 Processor Developers Manual [278693-002].pdf

*/

READ32_MEMBER(pxa255_periphs_device::pxa255_i2s_r)
{
	switch(PXA255_I2S_BASE_ADDR | (offset << 2))
	{
		case PXA255_SACR0:
			verboselog(3, "pxa255_i2s_r: Serial Audio Controller Global Control Register: %08x & %08x\n", m_i2s_regs.sacr0, mem_mask);
			return m_i2s_regs.sacr0;
		case PXA255_SACR1:
			verboselog(3, "pxa255_i2s_r: Serial Audio Controller I2S/MSB-Justified Control Register: %08x & %08x\n", m_i2s_regs.sacr1, mem_mask);
			return m_i2s_regs.sacr1;
		case PXA255_SASR0:
			verboselog(3, "pxa255_i2s_r: Serial Audio Controller I2S/MSB-Justified Status Register: %08x & %08x\n", m_i2s_regs.sasr0, mem_mask);
			return m_i2s_regs.sasr0;
		case PXA255_SAIMR:
			verboselog(3, "pxa255_i2s_r: Serial Audio Interrupt Mask Register: %08x & %08x\n", m_i2s_regs.saimr, mem_mask);
			return m_i2s_regs.saimr;
		case PXA255_SAICR:
			verboselog(3, "pxa255_i2s_r: Serial Audio Interrupt Clear Register: %08x & %08x\n", m_i2s_regs.saicr, mem_mask);
			return m_i2s_regs.saicr;
		case PXA255_SADIV:
			verboselog(3, "pxa255_i2s_r: Serial Audio Clock Divider Register: %08x & %08x\n", m_i2s_regs.sadiv, mem_mask);
			return m_i2s_regs.sadiv;
		case PXA255_SADR:
			verboselog(5, "pxa255_i2s_r: Serial Audio Data Register: %08x & %08x\n", m_i2s_regs.sadr, mem_mask);
			return m_i2s_regs.sadr;
		default:
			verboselog(0, "pxa255_i2s_r: Unknown address: %08x\n", PXA255_I2S_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_i2s_w)
{
	switch(PXA255_I2S_BASE_ADDR | (offset << 2))
	{
		case PXA255_SACR0:
			verboselog(3, "pxa255_i2s_w: Serial Audio Controller Global Control Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.sacr0 = data & 0x0000ff3d;
			break;
		case PXA255_SACR1:
			verboselog(3, "pxa255_i2s_w: Serial Audio Controller I2S/MSB-Justified Control Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.sacr1 = data & 0x00000039;
			break;
		case PXA255_SASR0:
			verboselog(3, "pxa255_i2s_w: Serial Audio Controller I2S/MSB-Justified Status Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.sasr0 = data & 0x0000ff7f;
			break;
		case PXA255_SAIMR:
			verboselog(3, "pxa255_i2s_w: Serial Audio Interrupt Mask Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.saimr = data & 0x00000078;
			break;
		case PXA255_SAICR:
			verboselog(3, "pxa255_i2s_w: Serial Audio Interrupt Clear Register: %08x & %08x\n", data, mem_mask);
			if(m_i2s_regs.saicr & PXA255_SAICR_ROR)
			{
				m_i2s_regs.sasr0 &= ~PXA255_SASR0_ROR;
			}
			if(m_i2s_regs.saicr & PXA255_SAICR_TUR)
			{
				m_i2s_regs.sasr0 &= ~PXA255_SASR0_TUR;
			}
			break;
		case PXA255_SADIV:
			verboselog(3, "pxa255_i2s_w: Serial Audio Clock Divider Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.sadiv = data & 0x0000007f;
			for (int i = 0; i < 2; i++)
			{
				m_dmadac[i]->set_frequency(((double)147600000 / (double)m_i2s_regs.sadiv) / 256.0);
				m_dmadac[i]->enable(1);
			}
			break;
		case PXA255_SADR:
			verboselog(4, "pxa255_i2s_w: Serial Audio Data Register: %08x & %08x\n", data, mem_mask);
			m_i2s_regs.sadr = data;
			break;
		default:
			verboselog(0, "pxa255_i2s_w: Unknown address: %08x = %08x & %08x\n", PXA255_I2S_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 DMA controller

  pg. 151 to 182, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::pxa255_dma_irq_check()
{
	int set_irq = 0;
	for (int channel = 0; channel < 16; channel++)
	{
		if (m_dma_regs.dcsr[channel] & (PXA255_DCSR_ENDINTR | PXA255_DCSR_STARTINTR | PXA255_DCSR_BUSERRINTR))
		{
			m_dma_regs.dint |= 1 << channel;
			set_irq = 1;
		}
		else
		{
			m_dma_regs.dint &= ~(1 << channel);
		}
	}

	pxa255_set_irq_line(PXA255_INT_DMA, set_irq);
}

void pxa255_periphs_device::pxa255_dma_load_descriptor_and_start(int channel)
{
	// Shut down any transfers that are currently going on, software should be smart enough to check if a
	// transfer is running before starting another one on the same channel.
	if (m_dma_regs.timer[channel]->enabled())
	{
		m_dma_regs.timer[channel]->adjust(attotime::never);
	}

	// Load the next descriptor

	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_dma_regs.dsadr[channel] = space.read_dword(m_dma_regs.ddadr[channel] + 0x4);
	m_dma_regs.dtadr[channel] = space.read_dword(m_dma_regs.ddadr[channel] + 0x8);
	m_dma_regs.dcmd[channel]  = space.read_dword(m_dma_regs.ddadr[channel] + 0xc);
	m_dma_regs.ddadr[channel] = space.read_dword(m_dma_regs.ddadr[channel]);

	// Start our end-of-transfer timer
	switch(channel)
	{
		case 3:
			m_dma_regs.timer[channel]->adjust(attotime::from_hz((147600000 / m_i2s_regs.sadiv) / (4 * 64)) * (m_dma_regs.dcmd[channel] & 0x00001fff), channel);
			break;
		default:
			m_dma_regs.timer[channel]->adjust(attotime::from_hz(100000000) * (m_dma_regs.dcmd[channel] & 0x00001fff), channel);
			break;
	}

	// Interrupt as necessary
	if(m_dma_regs.dcmd[channel] & PXA255_DCMD_STARTIRQEN)
	{
		m_dma_regs.dcsr[channel] |= PXA255_DCSR_STARTINTR;
	}

	m_dma_regs.dcsr[channel] &= ~PXA255_DCSR_STOPSTATE;
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::pxa255_dma_dma_end)
{
	uint32_t sadr = m_dma_regs.dsadr[param];
	uint32_t tadr = m_dma_regs.dtadr[param];
	uint32_t count = m_dma_regs.dcmd[param] & 0x00001fff;

	address_space &space = m_maincpu->space(AS_PROGRAM);
	switch (param)
	{
		case 3:
			for (uint32_t index = 0; index < count; index += 4)
			{
				m_words[index >> 2] = space.read_dword(sadr);
				m_samples[(index >> 1) + 0] = (int16_t)(m_words[index >> 2] >> 16);
				m_samples[(index >> 1) + 1] = (int16_t)(m_words[index >> 2] & 0xffff);
				sadr += 4;
			}
			for (int index = 0; index < 2; index++)
			{
				m_dmadac[index]->flush();
				m_dmadac[index]->transfer(index, 2, 2, count/4, m_samples.get());
			}
			break;
		default:
			for (uint32_t index = 0; index < count;)
			{
				switch (m_dma_regs.dcmd[param] & PXA255_DCMD_SIZE)
				{
					case PXA255_DCMD_SIZE_8:
						space.write_byte(tadr, space.read_byte(sadr));
						index++;
						break;
					case PXA255_DCMD_SIZE_16:
						space.write_word(tadr, space.read_word(sadr));
						index += 2;
						break;
					case PXA255_DCMD_SIZE_32:
						space.write_dword(tadr, space.read_dword(sadr));
						index += 4;
						break;
					default:
						logerror( "pxa255_dma_dma_end: Unsupported DMA size\n" );
						break;
				}

				if (m_dma_regs.dcmd[param] & PXA255_DCMD_INCSRCADDR)
				{
					switch(m_dma_regs.dcmd[param] & PXA255_DCMD_SIZE)
					{
						case PXA255_DCMD_SIZE_8:
							sadr++;
							break;
						case PXA255_DCMD_SIZE_16:
							sadr += 2;
							break;
						case PXA255_DCMD_SIZE_32:
							sadr += 4;
							break;
						default:
							break;
					}
				}
				if(m_dma_regs.dcmd[param] & PXA255_DCMD_INCTRGADDR)
				{
					switch(m_dma_regs.dcmd[param] & PXA255_DCMD_SIZE)
					{
						case PXA255_DCMD_SIZE_8:
							tadr++;
							break;
						case PXA255_DCMD_SIZE_16:
							tadr += 2;
							break;
						case PXA255_DCMD_SIZE_32:
							tadr += 4;
							break;
						default:
							break;
					}
				}
			}
			break;
	}

	if (m_dma_regs.dcmd[param] & PXA255_DCMD_ENDIRQEN)
	{
		m_dma_regs.dcsr[param] |= PXA255_DCSR_ENDINTR;
	}

	if (!(m_dma_regs.ddadr[param] & PXA255_DDADR_STOP) && (m_dma_regs.dcsr[param] & PXA255_DCSR_RUN))
	{
		if (m_dma_regs.dcsr[param] & PXA255_DCSR_RUN)
		{
			pxa255_dma_load_descriptor_and_start(param);
		}
		else
		{
			m_dma_regs.dcsr[param] &= ~PXA255_DCSR_RUN;
			m_dma_regs.dcsr[param] |= PXA255_DCSR_STOPSTATE;
		}
	}
	else
	{
		m_dma_regs.dcsr[param] &= ~PXA255_DCSR_RUN;
		m_dma_regs.dcsr[param] |= PXA255_DCSR_STOPSTATE;
	}

	pxa255_dma_irq_check();
}

READ32_MEMBER(pxa255_periphs_device::pxa255_dma_r)
{
	switch(PXA255_DMA_BASE_ADDR | (offset << 2))
	{
		case PXA255_DCSR0:      case PXA255_DCSR1:      case PXA255_DCSR2:      case PXA255_DCSR3:
		case PXA255_DCSR4:      case PXA255_DCSR5:      case PXA255_DCSR6:      case PXA255_DCSR7:
		case PXA255_DCSR8:      case PXA255_DCSR9:      case PXA255_DCSR10:     case PXA255_DCSR11:
		case PXA255_DCSR12:     case PXA255_DCSR13:     case PXA255_DCSR14:     case PXA255_DCSR15:
			verboselog(4, "pxa255_dma_r: DMA Channel Control/Status Register %d: %08x & %08x\n", offset, m_dma_regs.dcsr[offset], mem_mask);
			return m_dma_regs.dcsr[offset];
		case PXA255_DINT:
			if (0) verboselog(3, "pxa255_dma_r: DMA Interrupt Register: %08x & %08x\n", m_dma_regs.dint, mem_mask);
			return m_dma_regs.dint;
		case PXA255_DRCMR0:     case PXA255_DRCMR1:     case PXA255_DRCMR2:     case PXA255_DRCMR3:
		case PXA255_DRCMR4:     case PXA255_DRCMR5:     case PXA255_DRCMR6:     case PXA255_DRCMR7:
		case PXA255_DRCMR8:     case PXA255_DRCMR9:     case PXA255_DRCMR10:    case PXA255_DRCMR11:
		case PXA255_DRCMR12:    case PXA255_DRCMR13:    case PXA255_DRCMR14:    case PXA255_DRCMR15:
		case PXA255_DRCMR16:    case PXA255_DRCMR17:    case PXA255_DRCMR18:    case PXA255_DRCMR19:
		case PXA255_DRCMR20:    case PXA255_DRCMR21:    case PXA255_DRCMR22:    case PXA255_DRCMR23:
		case PXA255_DRCMR24:    case PXA255_DRCMR25:    case PXA255_DRCMR26:    case PXA255_DRCMR27:
		case PXA255_DRCMR28:    case PXA255_DRCMR29:    case PXA255_DRCMR30:    case PXA255_DRCMR31:
		case PXA255_DRCMR32:    case PXA255_DRCMR33:    case PXA255_DRCMR34:    case PXA255_DRCMR35:
		case PXA255_DRCMR36:    case PXA255_DRCMR37:    case PXA255_DRCMR38:    case PXA255_DRCMR39:
			verboselog(3, "pxa255_dma_r: DMA Request to Channel Map Register %d: %08x & %08x\n", offset - (0x100 >> 2), 0, mem_mask);
			return m_dma_regs.drcmr[offset - (0x100 >> 2)];
		case PXA255_DDADR0:     case PXA255_DDADR1:     case PXA255_DDADR2:     case PXA255_DDADR3:
		case PXA255_DDADR4:     case PXA255_DDADR5:     case PXA255_DDADR6:     case PXA255_DDADR7:
		case PXA255_DDADR8:     case PXA255_DDADR9:     case PXA255_DDADR10:    case PXA255_DDADR11:
		case PXA255_DDADR12:    case PXA255_DDADR13:    case PXA255_DDADR14:    case PXA255_DDADR15:
			verboselog(3, "pxa255_dma_r: DMA Descriptor Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask);
			return m_dma_regs.ddadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DSADR0:     case PXA255_DSADR1:     case PXA255_DSADR2:     case PXA255_DSADR3:
		case PXA255_DSADR4:     case PXA255_DSADR5:     case PXA255_DSADR6:     case PXA255_DSADR7:
		case PXA255_DSADR8:     case PXA255_DSADR9:     case PXA255_DSADR10:    case PXA255_DSADR11:
		case PXA255_DSADR12:    case PXA255_DSADR13:    case PXA255_DSADR14:    case PXA255_DSADR15:
			verboselog(3, "pxa255_dma_r: DMA Source Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask);
			return m_dma_regs.dsadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DTADR0:     case PXA255_DTADR1:     case PXA255_DTADR2:     case PXA255_DTADR3:
		case PXA255_DTADR4:     case PXA255_DTADR5:     case PXA255_DTADR6:     case PXA255_DTADR7:
		case PXA255_DTADR8:     case PXA255_DTADR9:     case PXA255_DTADR10:    case PXA255_DTADR11:
		case PXA255_DTADR12:    case PXA255_DTADR13:    case PXA255_DTADR14:    case PXA255_DTADR15:
			verboselog(3, "pxa255_dma_r: DMA Target Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask);
			return m_dma_regs.dtadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DCMD0:      case PXA255_DCMD1:      case PXA255_DCMD2:      case PXA255_DCMD3:
		case PXA255_DCMD4:      case PXA255_DCMD5:      case PXA255_DCMD6:      case PXA255_DCMD7:
		case PXA255_DCMD8:      case PXA255_DCMD9:      case PXA255_DCMD10:     case PXA255_DCMD11:
		case PXA255_DCMD12:     case PXA255_DCMD13:     case PXA255_DCMD14:     case PXA255_DCMD15:
			verboselog(3, "pxa255_dma_r: DMA Command Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask);
			return m_dma_regs.dcmd[(offset - (0x200 >> 2)) >> 2];
		default:
			verboselog(0, "pxa255_dma_r: Unknown address: %08x\n", PXA255_DMA_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_dma_w)
{
	switch (PXA255_DMA_BASE_ADDR | (offset << 2))
	{
		case PXA255_DCSR0:      case PXA255_DCSR1:      case PXA255_DCSR2:      case PXA255_DCSR3:
		case PXA255_DCSR4:      case PXA255_DCSR5:      case PXA255_DCSR6:      case PXA255_DCSR7:
		case PXA255_DCSR8:      case PXA255_DCSR9:      case PXA255_DCSR10:     case PXA255_DCSR11:
		case PXA255_DCSR12:     case PXA255_DCSR13:     case PXA255_DCSR14:     case PXA255_DCSR15:
			if (0) verboselog(3, "pxa255_dma_w: DMA Channel Control/Status Register %d: %08x & %08x\n", offset, data, mem_mask);
			m_dma_regs.dcsr[offset] &= ~(data & 0x00000007);
			m_dma_regs.dcsr[offset] &= ~0x60000000;
			m_dma_regs.dcsr[offset] |= data & 0x60000000;
			if ((data & PXA255_DCSR_RUN) && !(m_dma_regs.dcsr[offset] & PXA255_DCSR_RUN))
			{
				m_dma_regs.dcsr[offset] |= PXA255_DCSR_RUN;
				if (data & PXA255_DCSR_NODESCFETCH)
				{
					verboselog(0, "              No-Descriptor-Fetch mode is not supported.\n");
					break;
				}

				pxa255_dma_load_descriptor_and_start(offset);
			}
			else if(!(data & PXA255_DCSR_RUN))
			{
				m_dma_regs.dcsr[offset] &= ~PXA255_DCSR_RUN;
			}

			pxa255_dma_irq_check();
			break;
		case PXA255_DINT:
			verboselog(3, "pxa255_dma_w: DMA Interrupt Register: %08x & %08x\n", data, mem_mask);
			m_dma_regs.dint &= ~data;
			break;
		case PXA255_DRCMR0:     case PXA255_DRCMR1:     case PXA255_DRCMR2:     case PXA255_DRCMR3:
		case PXA255_DRCMR4:     case PXA255_DRCMR5:     case PXA255_DRCMR6:     case PXA255_DRCMR7:
		case PXA255_DRCMR8:     case PXA255_DRCMR9:     case PXA255_DRCMR10:    case PXA255_DRCMR11:
		case PXA255_DRCMR12:    case PXA255_DRCMR13:    case PXA255_DRCMR14:    case PXA255_DRCMR15:
		case PXA255_DRCMR16:    case PXA255_DRCMR17:    case PXA255_DRCMR18:    case PXA255_DRCMR19:
		case PXA255_DRCMR20:    case PXA255_DRCMR21:    case PXA255_DRCMR22:    case PXA255_DRCMR23:
		case PXA255_DRCMR24:    case PXA255_DRCMR25:    case PXA255_DRCMR26:    case PXA255_DRCMR27:
		case PXA255_DRCMR28:    case PXA255_DRCMR29:    case PXA255_DRCMR30:    case PXA255_DRCMR31:
		case PXA255_DRCMR32:    case PXA255_DRCMR33:    case PXA255_DRCMR34:    case PXA255_DRCMR35:
		case PXA255_DRCMR36:    case PXA255_DRCMR37:    case PXA255_DRCMR38:    case PXA255_DRCMR39:
			verboselog(3, "pxa255_dma_w: DMA Request to Channel Map Register %d: %08x & %08x\n", offset - (0x100 >> 2), data, mem_mask);
			m_dma_regs.drcmr[offset - (0x100 >> 2)] = data & 0x0000008f;
			break;
		case PXA255_DDADR0:     case PXA255_DDADR1:     case PXA255_DDADR2:     case PXA255_DDADR3:
		case PXA255_DDADR4:     case PXA255_DDADR5:     case PXA255_DDADR6:     case PXA255_DDADR7:
		case PXA255_DDADR8:     case PXA255_DDADR9:     case PXA255_DDADR10:    case PXA255_DDADR11:
		case PXA255_DDADR12:    case PXA255_DDADR13:    case PXA255_DDADR14:    case PXA255_DDADR15:
			verboselog(3, "pxa255_dma_w: DMA Descriptor Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask);
			m_dma_regs.ddadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffff1;
			break;
		case PXA255_DSADR0:     case PXA255_DSADR1:     case PXA255_DSADR2:     case PXA255_DSADR3:
		case PXA255_DSADR4:     case PXA255_DSADR5:     case PXA255_DSADR6:     case PXA255_DSADR7:
		case PXA255_DSADR8:     case PXA255_DSADR9:     case PXA255_DSADR10:    case PXA255_DSADR11:
		case PXA255_DSADR12:    case PXA255_DSADR13:    case PXA255_DSADR14:    case PXA255_DSADR15:
			verboselog(3, "pxa255_dma_w: DMA Source Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask);
			m_dma_regs.dsadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffffc;
			break;
		case PXA255_DTADR0:     case PXA255_DTADR1:     case PXA255_DTADR2:     case PXA255_DTADR3:
		case PXA255_DTADR4:     case PXA255_DTADR5:     case PXA255_DTADR6:     case PXA255_DTADR7:
		case PXA255_DTADR8:     case PXA255_DTADR9:     case PXA255_DTADR10:    case PXA255_DTADR11:
		case PXA255_DTADR12:    case PXA255_DTADR13:    case PXA255_DTADR14:    case PXA255_DTADR15:
			verboselog(3, "pxa255_dma_w: DMA Target Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask);
			m_dma_regs.dtadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffffc;
			break;
		case PXA255_DCMD0:      case PXA255_DCMD1:      case PXA255_DCMD2:      case PXA255_DCMD3:
		case PXA255_DCMD4:      case PXA255_DCMD5:      case PXA255_DCMD6:      case PXA255_DCMD7:
		case PXA255_DCMD8:      case PXA255_DCMD9:      case PXA255_DCMD10:     case PXA255_DCMD11:
		case PXA255_DCMD12:     case PXA255_DCMD13:     case PXA255_DCMD14:     case PXA255_DCMD15:
			verboselog(3, "pxa255_dma_w: DMA Command Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask);
			m_dma_regs.dcmd[(offset - (0x200 >> 2)) >> 2] = data & 0xf067dfff;
			break;
		default:
			verboselog(0, "pxa255_dma_w: Unknown address: %08x = %08x & %08x\n", PXA255_DMA_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 OS Timer register

  pg. 138 to 142, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::pxa255_ostimer_irq_check()
{
	pxa255_set_irq_line(PXA255_INT_OSTIMER0, (m_ostimer_regs.oier & PXA255_OIER_E0) ? ((m_ostimer_regs.ossr & PXA255_OSSR_M0) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER1, (m_ostimer_regs.oier & PXA255_OIER_E1) ? ((m_ostimer_regs.ossr & PXA255_OSSR_M1) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER2, (m_ostimer_regs.oier & PXA255_OIER_E2) ? ((m_ostimer_regs.ossr & PXA255_OSSR_M2) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER3, (m_ostimer_regs.oier & PXA255_OIER_E3) ? ((m_ostimer_regs.ossr & PXA255_OSSR_M3) ? 1 : 0) : 0);
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::pxa255_ostimer_match)
{
	if (0) verboselog(3, "pxa255_ostimer_match channel %d\n", param);
	m_ostimer_regs.ossr |= (1 << param);
	m_ostimer_regs.oscr = m_ostimer_regs.osmr[param];
	pxa255_ostimer_irq_check();
}

READ32_MEMBER(pxa255_periphs_device::pxa255_ostimer_r)
{
	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Match Register 0: %08x & %08x\n", m_ostimer_regs.osmr[0], mem_mask);
			return m_ostimer_regs.osmr[0];
		case PXA255_OSMR1:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Match Register 1: %08x & %08x\n", m_ostimer_regs.osmr[1], mem_mask);
			return m_ostimer_regs.osmr[1];
		case PXA255_OSMR2:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Match Register 2: %08x & %08x\n", m_ostimer_regs.osmr[2], mem_mask);
			return m_ostimer_regs.osmr[2];
		case PXA255_OSMR3:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Match Register 3: %08x & %08x\n", m_ostimer_regs.osmr[3], mem_mask);
			return m_ostimer_regs.osmr[3];
		case PXA255_OSCR:
			if (0) verboselog(4, "pxa255_ostimer_r: OS Timer Count Register: %08x & %08x\n", m_ostimer_regs.oscr, mem_mask);
			// free-running 3.something MHz counter.  this is a complete hack.
			m_ostimer_regs.oscr += 0x300;
			return m_ostimer_regs.oscr;
		case PXA255_OSSR:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Status Register: %08x & %08x\n", m_ostimer_regs.ossr, mem_mask);
			return m_ostimer_regs.ossr;
		case PXA255_OWER:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", m_ostimer_regs.ower, mem_mask);
			return m_ostimer_regs.ower;
		case PXA255_OIER:
			if (0) verboselog(3, "pxa255_ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", m_ostimer_regs.oier, mem_mask);
			return m_ostimer_regs.oier;
		default:
			if (0) verboselog(0, "pxa255_ostimer_r: Unknown address: %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_ostimer_w)
{
	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Match Register 0: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.osmr[0] = data;
			if (m_ostimer_regs.oier & PXA255_OIER_E0)
			{
				m_ostimer_regs.timer[0]->adjust(attotime::from_hz(3846400) * (m_ostimer_regs.osmr[0] - m_ostimer_regs.oscr));
			}
			break;
		case PXA255_OSMR1:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Match Register 1: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.osmr[1] = data;
			if (m_ostimer_regs.oier & PXA255_OIER_E1)
			{
				m_ostimer_regs.timer[1]->adjust(attotime::from_hz(3846400) * (m_ostimer_regs.osmr[1] - m_ostimer_regs.oscr), 1);
			}
			break;
		case PXA255_OSMR2:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Match Register 2: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.osmr[2] = data;
			if (m_ostimer_regs.oier & PXA255_OIER_E2)
			{
				m_ostimer_regs.timer[2]->adjust(attotime::from_hz(3846400) * (m_ostimer_regs.osmr[2] - m_ostimer_regs.oscr), 2);
			}
			break;
		case PXA255_OSMR3:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Match Register 3: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.osmr[3] = data;
			if (m_ostimer_regs.oier & PXA255_OIER_E3)
			{
				//m_ostimer_regs.timer[3]->adjust(attotime::from_hz(3846400) * (m_ostimer_regs.osmr[3] - m_ostimer_regs.oscr), 3);
			}
			break;
		case PXA255_OSCR:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Count Register: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.oscr = data;
			break;
		case PXA255_OSSR:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Status Register: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.ossr &= ~data;
			pxa255_ostimer_irq_check();
			break;
		case PXA255_OWER:
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Watchdog Enable Register: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.ower = data & 0x00000001;
			break;
		case PXA255_OIER:
		{
			if (0) verboselog(3, "pxa255_ostimer_w: OS Timer Interrupt Enable Register: %08x & %08x\n", data, mem_mask);
			m_ostimer_regs.oier = data & 0x0000000f;
			for (int index = 0; index < 4; index++)
			{
				if (m_ostimer_regs.oier & (1 << index))
				{
					//m_ostimer_regs.timer[index]->adjust(attotime::from_hz(200000000) * m_ostimer_regs.osmr[index], index);
				}
			}
			break;
		}
		default:
			verboselog(0, "pxa255_ostimer_w: Unknown address: %08x = %08x & %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 Interrupt registers

  pg. 124 to 132, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::pxa255_update_interrupts()
{
	m_intc_regs.icfp = (m_intc_regs.icpr & m_intc_regs.icmr) & m_intc_regs.iclr;
	m_intc_regs.icip = (m_intc_regs.icpr & m_intc_regs.icmr) & (~m_intc_regs.iclr);
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, m_intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE,  m_intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
}

void pxa255_periphs_device::pxa255_set_irq_line(uint32_t line, int irq_state)
{
	m_intc_regs.icpr &= ~line;
	m_intc_regs.icpr |= irq_state ? line : 0;
	pxa255_update_interrupts();
}

READ32_MEMBER(pxa255_periphs_device::pxa255_intc_r)
{
	switch (PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", m_intc_regs.icip, mem_mask);
			return m_intc_regs.icip;
		case PXA255_ICMR:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller Mask Register: %08x & %08x\n", m_intc_regs.icmr, mem_mask);
			return m_intc_regs.icmr;
		case PXA255_ICLR:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller Level Register: %08x & %08x\n", m_intc_regs.iclr, mem_mask);
			return m_intc_regs.iclr;
		case PXA255_ICFP:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", m_intc_regs.icfp, mem_mask);
			return m_intc_regs.icfp;
		case PXA255_ICPR:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller Pending Register: %08x & %08x\n", m_intc_regs.icpr, mem_mask);
			return m_intc_regs.icpr;
		case PXA255_ICCR:
			if (0) verboselog(3, "pxa255_intc_r: Interrupt Controller Control Register: %08x & %08x\n", m_intc_regs.iccr, mem_mask);
			return m_intc_regs.iccr;
		default:
			verboselog(0, "pxa255_intc_r: Unknown address: %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_intc_w)
{
	switch (PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog(3, "pxa255_intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_ICMR:
			if (0) verboselog(3, "pxa255_intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask);
			m_intc_regs.icmr = data & 0xfffe7f00;
			break;
		case PXA255_ICLR:
			if (0) verboselog(3, "pxa255_intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask);
			m_intc_regs.iclr = data & 0xfffe7f00;
			break;
		case PXA255_ICFP:
			if (0) verboselog(3, "pxa255_intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_ICPR:
			if (0) verboselog(3, "pxa255_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_ICCR:
			if (0) verboselog(3, "pxa255_intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask);
			m_intc_regs.iccr = data & 0x00000001;
			break;
		default:
			verboselog(0, "pxa255_intc_w: Unknown address: %08x = %08x & %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 General-Purpose I/O registers

  pg. 105 to 124, PXA255 Processor Developers Manual [278693-002].pdf

*/

READ32_MEMBER(pxa255_periphs_device::pxa255_gpio_r)
{
	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
		{
			uint32_t value = m_gpio_regs.gplr0;
			if (!m_gpio0_in_func.isnull())
			{
				value |= m_gpio0_in_func(space, 0);
			}
			verboselog(3, "pxa255_gpio_r: GPIO Pin-Level Register 0: %08x & %08x\n", m_gpio_regs.gplr0 | (1 << 1), mem_mask);
			return value;
		}
		case PXA255_GPLR1:
			verboselog(3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 1: %08x & %08x\n", m_gpio_regs.gplr1, mem_mask);
			return 0xff9fffff;
		/*
		    0x200000 = flip screen
		*/
		case PXA255_GPLR2:
			verboselog(3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 2: %08x & %08x\n", m_gpio_regs.gplr2, mem_mask);
			return m_gpio_regs.gplr2;
		case PXA255_GPDR0:
			verboselog(3, "pxa255_gpio_r: GPIO Pin Direction Register 0: %08x & %08x\n", m_gpio_regs.gpdr0, mem_mask);
			return m_gpio_regs.gpdr0;
		case PXA255_GPDR1:
			verboselog(3, "pxa255_gpio_r: GPIO Pin Direction Register 1: %08x & %08x\n", m_gpio_regs.gpdr1, mem_mask);
			return m_gpio_regs.gpdr1;
		case PXA255_GPDR2:
			verboselog(3, "pxa255_gpio_r: GPIO Pin Direction Register 2: %08x & %08x\n", m_gpio_regs.gpdr2, mem_mask);
			return m_gpio_regs.gpdr2;
		case PXA255_GPSR0:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 0: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GPSR1:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 1: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GPSR2:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 2: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GPCR0:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 0: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GPCR1:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 1: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GPCR2:
			verboselog(3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 2: %08x & %08x\n", machine().rand(), mem_mask);
			return machine().rand();
		case PXA255_GRER0:
			verboselog(3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", m_gpio_regs.grer0, mem_mask);
			return m_gpio_regs.grer0;
		case PXA255_GRER1:
			verboselog(3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", m_gpio_regs.grer1, mem_mask);
			return m_gpio_regs.grer1;
		case PXA255_GRER2:
			verboselog(3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", m_gpio_regs.grer2, mem_mask);
			return m_gpio_regs.grer2;
		case PXA255_GFER0:
			verboselog(3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", m_gpio_regs.gfer0, mem_mask);
			return m_gpio_regs.gfer0;
		case PXA255_GFER1:
			verboselog(3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", m_gpio_regs.gfer1, mem_mask);
			return m_gpio_regs.gfer1;
		case PXA255_GFER2:
			verboselog(3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", m_gpio_regs.gfer2, mem_mask);
			return m_gpio_regs.gfer2;
		case PXA255_GEDR0:
			verboselog(3, "pxa255_gpio_r: GPIO Edge Detect Status Register 0: %08x & %08x\n", m_gpio_regs.gedr0, mem_mask);
			return m_gpio_regs.gedr0;
		case PXA255_GEDR1:
			verboselog(3, "pxa255_gpio_r: GPIO Edge Detect Status Register 1: %08x & %08x\n", m_gpio_regs.gedr1, mem_mask);
			return m_gpio_regs.gedr1;
		case PXA255_GEDR2:
			verboselog(3, "pxa255_gpio_r: GPIO Edge Detect Status Register 2: %08x & %08x\n", m_gpio_regs.gedr2, mem_mask);
			return m_gpio_regs.gedr2;
		case PXA255_GAFR0_L:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", m_gpio_regs.gafr0l, mem_mask);
			return m_gpio_regs.gafr0l;
		case PXA255_GAFR0_U:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", m_gpio_regs.gafr0u, mem_mask);
			return m_gpio_regs.gafr0u;
		case PXA255_GAFR1_L:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", m_gpio_regs.gafr1l, mem_mask);
			return m_gpio_regs.gafr1l;
		case PXA255_GAFR1_U:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", m_gpio_regs.gafr1u, mem_mask);
			return m_gpio_regs.gafr1u;
		case PXA255_GAFR2_L:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", m_gpio_regs.gafr2l, mem_mask);
			return m_gpio_regs.gafr2l;
		case PXA255_GAFR2_U:
			verboselog(3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", m_gpio_regs.gafr2u, mem_mask);
			return m_gpio_regs.gafr2u;
		default:
			verboselog(0, "pxa255_gpio_r: Unknown address: %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_gpio_w)
{
	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
			verboselog(3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 0: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_GPLR1:
			verboselog(3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 1: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_GPLR2:
			verboselog(3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 2: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_GPDR0:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Direction Register 0: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpdr0 = data;
			break;
		case PXA255_GPDR1:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Direction Register 1: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpdr1 = data;
			break;
		case PXA255_GPDR2:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Direction Register 2: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpdr2 = data;
			break;
		case PXA255_GPSR0:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Set Register 0: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr0 |= data & m_gpio_regs.gpdr0;
			if (!m_gpio0_set_func.isnull())
			{
				m_gpio0_set_func(space, 0, data);
			}
			break;
		case PXA255_GPSR1:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Set Register 1: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr1 |= data & m_gpio_regs.gpdr1;
			break;
		case PXA255_GPSR2:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Set Register 2: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr2 |= data & m_gpio_regs.gpdr2;
			break;
		case PXA255_GPCR0:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Clear Register 0: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr0 &= ~(data & m_gpio_regs.gpdr0);
			if (!m_gpio0_clear_func.isnull())
			{
				m_gpio0_clear_func(space, 0, data);
			}
			break;
		case PXA255_GPCR1:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Clear Register 1: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr1 &= ~(data & m_gpio_regs.gpdr1);
			break;
		case PXA255_GPCR2:
			verboselog(3, "pxa255_gpio_w: GPIO Pin Output Clear Register 2: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gpsr2 &= ~(data & m_gpio_regs.gpdr2);
			break;
		case PXA255_GRER0:
			verboselog(3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.grer0 = data;
			break;
		case PXA255_GRER1:
			verboselog(3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.grer1 = data;
			break;
		case PXA255_GRER2:
			verboselog(3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.grer2 = data;
			break;
		case PXA255_GFER0:
			verboselog(3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gfer0 = data;
			break;
		case PXA255_GFER1:
			verboselog(3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gfer1 = data;
			break;
		case PXA255_GFER2:
			verboselog(3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask);
			m_gpio_regs.gfer2 = data;
			break;
		case PXA255_GEDR0:
			verboselog(3, "pxa255_gpio_w: GPIO Edge Detect Status Register 0: %08x & %08x\n", m_gpio_regs.gedr0, mem_mask);
			m_gpio_regs.gedr0 &= ~data;
			break;
		case PXA255_GEDR1:
			verboselog(3, "pxa255_gpio_w: GPIO Edge Detect Status Register 1: %08x & %08x\n", m_gpio_regs.gedr1, mem_mask);
			m_gpio_regs.gedr1 &= ~data;
			break;
		case PXA255_GEDR2:
			verboselog(3, "pxa255_gpio_w: GPIO Edge Detect Status Register 2: %08x & %08x\n", m_gpio_regs.gedr2, mem_mask);
			m_gpio_regs.gedr2 &= ~data;
			break;
		case PXA255_GAFR0_L:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", m_gpio_regs.gafr0l, mem_mask);
			m_gpio_regs.gafr0l = data;
			break;
		case PXA255_GAFR0_U:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", m_gpio_regs.gafr0u, mem_mask);
			m_gpio_regs.gafr0u = data;
			break;
		case PXA255_GAFR1_L:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", m_gpio_regs.gafr1l, mem_mask);
			m_gpio_regs.gafr1l = data;
			break;
		case PXA255_GAFR1_U:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", m_gpio_regs.gafr1u, mem_mask);
			m_gpio_regs.gafr1u = data;
			break;
		case PXA255_GAFR2_L:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", m_gpio_regs.gafr2l, mem_mask);
			m_gpio_regs.gafr2l = data;
			break;
		case PXA255_GAFR2_U:
			verboselog(3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", m_gpio_regs.gafr2u, mem_mask);
			m_gpio_regs.gafr2u = data;
			break;
		default:
			verboselog(0, "pxa255_gpio_w: Unknown address: %08x = %08x & %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 LCD Controller

  pg. 265 to 310, PXA255 Processor Developers Manual [278693-002].pdf

*/

void pxa255_periphs_device::pxa255_lcd_load_dma_descriptor(address_space & space, uint32_t address, int channel)
{
	m_lcd_regs.dma[channel].fdadr = space.read_dword(address);
	m_lcd_regs.dma[channel].fsadr = space.read_dword(address + 0x04);
	m_lcd_regs.dma[channel].fidr  = space.read_dword(address + 0x08);
	m_lcd_regs.dma[channel].ldcmd = space.read_dword(address + 0x0c);
	verboselog(4, "pxa255_lcd_load_dma_descriptor, address = %08x, channel = %d\n", address, channel);
	verboselog(4, "    DMA Frame Descriptor: %08x\n", m_lcd_regs.dma[channel].fdadr );
	verboselog(4, "    DMA Frame Source Address: %08x\n", m_lcd_regs.dma[channel].fsadr );
	verboselog(4, "    DMA Frame ID: %08x\n", m_lcd_regs.dma[channel].fidr );
	verboselog(4, "    DMA Command: %08x\n", m_lcd_regs.dma[channel].ldcmd );
}

void pxa255_periphs_device::pxa255_lcd_irq_check()
{
	if(((m_lcd_regs.lcsr & PXA255_LCSR_BS)  != 0 && (m_lcd_regs.lccr0 & PXA255_LCCR0_BM)  == 0) ||
	   ((m_lcd_regs.lcsr & PXA255_LCSR_EOF) != 0 && (m_lcd_regs.lccr0 & PXA255_LCCR0_EFM) == 0) ||
	   ((m_lcd_regs.lcsr & PXA255_LCSR_SOF) != 0 && (m_lcd_regs.lccr0 & PXA255_LCCR0_SFM) == 0))
	{
		pxa255_set_irq_line(PXA255_INT_LCD, 1);
	}
	else
	{
		pxa255_set_irq_line(PXA255_INT_LCD, 0);
	}
}

void pxa255_periphs_device::pxa255_lcd_dma_kickoff(int channel)
{
	if(m_lcd_regs.dma[channel].fdadr != 0)
	{
		attotime period = attotime::from_hz(20000000) * (m_lcd_regs.dma[channel].ldcmd & 0x000fffff);

		m_lcd_regs.dma[channel].eof->adjust(period, channel);

		if(m_lcd_regs.dma[channel].ldcmd & PXA255_LDCMD_SOFINT)
		{
			m_lcd_regs.liidr = m_lcd_regs.dma[channel].fidr;
			m_lcd_regs.lcsr |= PXA255_LCSR_SOF;
			pxa255_lcd_irq_check();
		}

		if(m_lcd_regs.dma[channel].ldcmd & PXA255_LDCMD_PAL)
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = m_lcd_regs.dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index += 2)
			{
				uint16_t color = space.read_word((m_lcd_regs.dma[channel].fsadr &~ 1) + index);
				m_pxa255_lcd_palette[index >> 1] = (((((color >> 11) & 0x1f) << 3) | (color >> 13)) << 16) | (((((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3)) << 8) | (((color & 0x1f) << 3) | ((color >> 2) & 0x7));
				m_palette->set_pen_color(index >> 1, (((color >> 11) & 0x1f) << 3) | (color >> 13), (((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3), ((color & 0x1f) << 3) | ((color >> 2) & 0x7));
			}
		}
		else
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = m_lcd_regs.dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index++)
			{
				m_pxa255_lcd_framebuffer[index] = space.read_byte(m_lcd_regs.dma[channel].fsadr + index);
			}
		}
	}
}

void pxa255_periphs_device::pxa255_lcd_check_load_next_branch(int channel)
{
	if(m_lcd_regs.fbr[channel] & 1)
	{
		verboselog(4, "pxa255_lcd_check_load_next_branch: Taking branch\n" );
		m_lcd_regs.fbr[channel] &= ~1;
		address_space &space = m_maincpu->space(AS_PROGRAM);
		//m_lcd_regs.fbr[channel] = (space.read_dword(m_lcd_regs.fbr[channel] & 0xfffffff0) & 0xfffffff0) | (m_lcd_regs.fbr[channel] & 0x00000003);
		//printf( "%08x\n", m_lcd_regs.fbr[channel] );
		pxa255_lcd_load_dma_descriptor(space, m_lcd_regs.fbr[channel] & 0xfffffff0, 0);
		m_lcd_regs.fbr[channel] = (space.read_dword(m_lcd_regs.fbr[channel] & 0xfffffff0) & 0xfffffff0) | (m_lcd_regs.fbr[channel] & 0x00000003);
		pxa255_lcd_dma_kickoff(0);
		if(m_lcd_regs.fbr[channel] & 2)
		{
			m_lcd_regs.fbr[channel] &= ~2;
			if(!(m_lcd_regs.lccr0 & PXA255_LCCR0_BM))
			{
				m_lcd_regs.lcsr |= PXA255_LCSR_BS;
			}
		}
	}
	else
	{
		if (0) verboselog(3, "pxa255_lcd_check_load_next_branch: Not taking branch\n" );
	}
}

TIMER_CALLBACK_MEMBER(pxa255_periphs_device::pxa255_lcd_dma_eof)
{
	if (0) verboselog(3, "End of frame callback\n" );
	if(m_lcd_regs.dma[param].ldcmd & PXA255_LDCMD_EOFINT)
	{
		m_lcd_regs.liidr = m_lcd_regs.dma[param].fidr;
		m_lcd_regs.lcsr |= PXA255_LCSR_EOF;
	}
	pxa255_lcd_check_load_next_branch(param);
	pxa255_lcd_irq_check();
}

READ32_MEMBER(pxa255_periphs_device::pxa255_lcd_r)
{
	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:      // 0x44000000
			verboselog(3, "pxa255_lcd_r: LCD Control 0: %08x & %08x\n", m_lcd_regs.lccr0, mem_mask);
			return m_lcd_regs.lccr0;
		case PXA255_LCCR1:      // 0x44000004
			verboselog(3, "pxa255_lcd_r: LCD Control 1: %08x & %08x\n", m_lcd_regs.lccr1, mem_mask);
			return m_lcd_regs.lccr1;
		case PXA255_LCCR2:      // 0x44000008
			verboselog(3, "pxa255_lcd_r: LCD Control 2: %08x & %08x\n", m_lcd_regs.lccr2, mem_mask);
			return m_lcd_regs.lccr2;
		case PXA255_LCCR3:      // 0x4400000c
			verboselog(3, "pxa255_lcd_r: LCD Control 3: %08x & %08x\n", m_lcd_regs.lccr3, mem_mask);
			return m_lcd_regs.lccr3;
		case PXA255_FBR0:       // 0x44000020
			verboselog(4, "pxa255_lcd_r: LCD Frame Branch Register 0: %08x & %08x\n", m_lcd_regs.fbr[0], mem_mask);
			return m_lcd_regs.fbr[0];
		case PXA255_FBR1:       // 0x44000024
			verboselog(3, "pxa255_lcd_r: LCD Frame Branch Register 1: %08x & %08x\n", m_lcd_regs.fbr[1], mem_mask);
			return m_lcd_regs.fbr[1];
		case PXA255_LCSR:       // 0x44000038
			verboselog(4, "pxa255_lcd_r: LCD Status Register: %08x & %08x\n", m_lcd_regs.lcsr, mem_mask);
			return m_lcd_regs.lcsr;
		case PXA255_LIIDR:      // 0x4400003c
			verboselog(3, "pxa255_lcd_r: LCD Interrupt ID Register: %08x & %08x\n", m_lcd_regs.liidr, mem_mask);
			return m_lcd_regs.liidr;
		case PXA255_TRGBR:      // 0x44000040
			verboselog(3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", m_lcd_regs.trgbr, mem_mask);
			return m_lcd_regs.trgbr;
		case PXA255_TCR:        // 0x44000044
			verboselog(3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", m_lcd_regs.tcr, mem_mask);
			return m_lcd_regs.tcr;
		case PXA255_FDADR0:     // 0x44000200
			if (0) verboselog(3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", m_lcd_regs.dma[0].fdadr, mem_mask);
			return m_lcd_regs.dma[0].fdadr;
		case PXA255_FSADR0:     // 0x44000204
			verboselog(3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 0: %08x & %08x\n", m_lcd_regs.dma[0].fsadr, mem_mask);
			return m_lcd_regs.dma[0].fsadr;
		case PXA255_FIDR0:      // 0x44000208
			verboselog(3, "pxa255_lcd_r: LCD DMA Frame ID Register 0: %08x & %08x\n", m_lcd_regs.dma[0].fidr, mem_mask);
			return m_lcd_regs.dma[0].fidr;
		case PXA255_LDCMD0:     // 0x4400020c
			if (0) verboselog(3, "pxa255_lcd_r: LCD DMA Command Register 0: %08x & %08x\n", m_lcd_regs.dma[0].ldcmd & 0xfff00000, mem_mask);
			return m_lcd_regs.dma[0].ldcmd & 0xfff00000;
		case PXA255_FDADR1:     // 0x44000210
			verboselog(3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", m_lcd_regs.dma[1].fdadr, mem_mask);
			return m_lcd_regs.dma[1].fdadr;
		case PXA255_FSADR1:     // 0x44000214
			verboselog(3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 1: %08x & %08x\n", m_lcd_regs.dma[1].fsadr, mem_mask);
			return m_lcd_regs.dma[1].fsadr;
		case PXA255_FIDR1:      // 0x44000218
			verboselog(3, "pxa255_lcd_r: LCD DMA Frame ID Register 1: %08x & %08x\n", m_lcd_regs.dma[1].fidr, mem_mask);
			return m_lcd_regs.dma[1].fidr;
		case PXA255_LDCMD1:     // 0x4400021c
			verboselog(3, "pxa255_lcd_r: LCD DMA Command Register 1: %08x & %08x\n", m_lcd_regs.dma[1].ldcmd & 0xfff00000, mem_mask);
			return m_lcd_regs.dma[1].ldcmd & 0xfff00000;
		default:
			verboselog(0, "pxa255_lcd_r: Unknown address: %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(pxa255_periphs_device::pxa255_lcd_w)
{
	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:      // 0x44000000
			verboselog(3, "pxa255_lcd_w: LCD Control 0: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.lccr0 = data & 0x00fffeff;
			break;
		case PXA255_LCCR1:      // 0x44000004
			verboselog(3, "pxa255_lcd_w: LCD Control 1: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.lccr1 = data;
			break;
		case PXA255_LCCR2:      // 0x44000008
			verboselog(3, "pxa255_lcd_w: LCD Control 2: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.lccr2 = data;
			break;
		case PXA255_LCCR3:      // 0x4400000c
			verboselog(3, "pxa255_lcd_w: LCD Control 3: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.lccr3 = data;
			break;
		case PXA255_FBR0:       // 0x44000020
			verboselog(4l, "pxa255_lcd_w: LCD Frame Branch Register 0: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.fbr[0] = data & 0xfffffff3;
			if(!m_lcd_regs.dma[0].eof->enabled())
			{
				if (0) verboselog(3, "ch0 EOF timer is not enabled, taking branch now\n" );
				pxa255_lcd_check_load_next_branch(0);
				pxa255_lcd_irq_check();
			}
			break;
		case PXA255_FBR1:       // 0x44000024
			verboselog(3, "pxa255_lcd_w: LCD Frame Branch Register 1: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.fbr[1] = data & 0xfffffff3;
			if(!m_lcd_regs.dma[1].eof->enabled())
			{
				verboselog(3, "ch1 EOF timer is not enabled, taking branch now\n" );
				pxa255_lcd_check_load_next_branch(1);
				pxa255_lcd_irq_check();
			}
			break;
		case PXA255_LCSR:       // 0x44000038
			verboselog(4, "pxa255_lcd_w: LCD Controller Status Register: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.lcsr &= ~data;
			pxa255_lcd_irq_check();
			break;
		case PXA255_LIIDR:      // 0x4400003c
			verboselog(3, "pxa255_lcd_w: LCD Controller Interrupt ID Register: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_TRGBR:      // 0x44000040
			verboselog(3, "pxa255_lcd_w: TMED RGB Seed Register: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.trgbr = data & 0x00ffffff;
			break;
		case PXA255_TCR:        // 0x44000044
			verboselog(3, "pxa255_lcd_w: TMED Control Register: %08x & %08x\n", data, mem_mask);
			m_lcd_regs.tcr = data & 0x00004fff;
			break;
		case PXA255_FDADR0:     // 0x44000200
			verboselog(4, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", data, mem_mask);
			if(!m_lcd_regs.dma[0].eof->enabled())
			{
				pxa255_lcd_load_dma_descriptor(space, data & 0xfffffff0, 0);
			}
			else
			{
				m_lcd_regs.fbr[0] &= 0x00000003;
				m_lcd_regs.fbr[0] |= data & 0xfffffff0;
			}
			break;
		case PXA255_FSADR0:     // 0x44000204
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 0: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_FIDR0:      // 0x44000208
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 0: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_LDCMD0:     // 0x4400020c
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 0: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_FDADR1:     // 0x44000210
			verboselog(4, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", data, mem_mask);
			if(!m_lcd_regs.dma[1].eof->enabled())
			{
				pxa255_lcd_load_dma_descriptor(space, data & 0xfffffff0, 1);
			}
			else
			{
				m_lcd_regs.fbr[1] &= 0x00000003;
				m_lcd_regs.fbr[1] |= data & 0xfffffff0;
			}
			break;
		case PXA255_FSADR1:     // 0x44000214
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 1: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_FIDR1:      // 0x44000218
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 1: %08x & %08x\n", data, mem_mask);
			break;
		case PXA255_LDCMD1:     // 0x4400021c
			verboselog(4, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 1: %08x & %08x\n", data, mem_mask);
			break;
		default:
			verboselog(0, "pxa255_lcd_w: Unknown address: %08x = %08x & %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

void pxa255_periphs_device::device_start()
{
	for (int index = 0; index < 16; index++)
	{
		m_dma_regs.timer[index] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pxa255_periphs_device::pxa255_dma_dma_end),this));
	}

	for (int index = 0; index < 4; index++)
	{
		m_ostimer_regs.timer[index] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pxa255_periphs_device::pxa255_ostimer_match),this));
	}

	m_lcd_regs.dma[0].eof = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pxa255_periphs_device::pxa255_lcd_dma_eof),this));
	m_lcd_regs.dma[1].eof = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pxa255_periphs_device::pxa255_lcd_dma_eof),this));

	m_pxa255_lcd_palette = make_unique_clear<uint32_t[]>(0x100);
	m_pxa255_lcd_framebuffer = make_unique_clear<uint8_t[]>(0x100000);
	m_words = make_unique_clear<uint32_t[]>(0x800);
	m_samples = make_unique_clear<int16_t[]>(0x1000);

	m_gpio0_set_func.resolve_safe();
	m_gpio0_clear_func.resolve_safe();
	m_gpio0_in_func.resolve_safe(0xffffffff);
}

void pxa255_periphs_device::device_reset()
{
	for (int index = 0; index < 16; index++)
	{
		m_dma_regs.dcsr[index] = 0x00000008;
	}

	memset(&m_intc_regs, 0, sizeof(m_intc_regs));

	m_lcd_regs.trgbr = 0x00aa5500;
	m_lcd_regs.tcr = 0x0000754f;
}

uint32_t pxa255_periphs_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y <= (m_lcd_regs.lccr2 & PXA255_LCCR2_LPP); y++)
	{
		uint32_t *dst = &bitmap.pix32(y);
		for (int x = 0; x <= (m_lcd_regs.lccr1 & PXA255_LCCR1_PPL); x++)
		{
			*dst++ = m_pxa255_lcd_palette[m_pxa255_lcd_framebuffer[y * ((m_lcd_regs.lccr1 & PXA255_LCCR1_PPL) + 1) + x]];
		}
	}
	return 0;
}

void pxa255_periphs_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024, 1024);
	screen.set_visarea(0, 295, 0, 479);
	screen.set_screen_update(FUNC(pxa255_periphs_device::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}
