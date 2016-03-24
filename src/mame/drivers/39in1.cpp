// license:BSD-3-Clause
// copyright-holders:R. Belmont, Ryan Holtz, Andreas Naive
/**************************************************************************
 *
 * 39in1.c - bootleg MAME-based "39-in-1" arcade PCB
 * Skeleton by R. Belmont, thanks to the Guru
 * PXA255 Peripheral hookup by Ryan Holtz
 * Decrypt by Andreas Naive
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5TE instruction set without the FPU
 *    ARM standard MMU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 * TODO:
 *   PXA255 peripherals
 *
 **************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/eepromser.h"
#include "machine/pxa255.h"
#include "sound/dmadac.h"

class _39in1_state : public driver_device
{
public:
	_39in1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_eeprom(*this, "eeprom"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	UINT32 m_seed;
	UINT32 m_magic;
	UINT32 m_state;

	required_shared_ptr<UINT32> m_ram;

	PXA255_DMA_Regs m_dma_regs;
	PXA255_I2S_Regs m_i2s_regs;
	PXA255_OSTMR_Regs m_ostimer_regs;
	PXA255_INTC_Regs m_intc_regs;
	PXA255_GPIO_Regs m_gpio_regs;
	PXA255_LCD_Regs m_lcd_regs;

	dmadac_sound_device *m_dmadac[2];
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	UINT32 m_pxa255_lcd_palette[0x100];
	UINT8 m_pxa255_lcd_framebuffer[0x100000];


	//FILE* audio_dump;
	UINT32 m_words[0x800];
	INT16 m_samples[0x1000];
	DECLARE_READ32_MEMBER(pxa255_i2s_r);
	DECLARE_WRITE32_MEMBER(pxa255_i2s_w);
	DECLARE_READ32_MEMBER(pxa255_dma_r);
	DECLARE_WRITE32_MEMBER(pxa255_dma_w);
	DECLARE_READ32_MEMBER(pxa255_ostimer_r);
	DECLARE_WRITE32_MEMBER(pxa255_ostimer_w);
	DECLARE_READ32_MEMBER(pxa255_intc_r);
	DECLARE_WRITE32_MEMBER(pxa255_intc_w);
	DECLARE_READ32_MEMBER(pxa255_gpio_r);
	DECLARE_WRITE32_MEMBER(pxa255_gpio_w);
	DECLARE_READ32_MEMBER(pxa255_lcd_r);
	DECLARE_WRITE32_MEMBER(pxa255_lcd_w);
	DECLARE_READ32_MEMBER(cpld_r);
	DECLARE_WRITE32_MEMBER(cpld_w);
	DECLARE_READ32_MEMBER(prot_cheater_r);
	DECLARE_DRIVER_INIT(39in1);
	DECLARE_MACHINE_START(60in1);
	virtual void machine_start() override;
	UINT32 screen_update_39in1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pxa255_vblank_start);
	TIMER_CALLBACK_MEMBER(pxa255_dma_dma_end);
	TIMER_CALLBACK_MEMBER(pxa255_ostimer_match);
	TIMER_CALLBACK_MEMBER(pxa255_lcd_dma_eof);
	void pxa255_dma_irq_check();
	void pxa255_dma_load_descriptor_and_start(int channel);
	void pxa255_ostimer_irq_check();
	void pxa255_update_interrupts();
	void pxa255_set_irq_line(UINT32 line, int state);
	void pxa255_lcd_load_dma_descriptor(address_space & space, UINT32 address, int channel);
	void pxa255_lcd_irq_check();
	void pxa255_lcd_dma_kickoff(int channel);
	void pxa255_lcd_check_load_next_branch(int channel);
	void pxa255_start();
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};


#define VERBOSE_LEVEL ( 3 )

static inline void ATTR_PRINTF(3,4) verboselog( device_t& device, int n_level, const char* s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%s: %s", device.machine().describe_context(), buf );
		//printf( "%s: %s", device.machine().describe_context(), buf );
	}
}

/*

  PXA255 Inter-Integrated-Circuit Sound (I2S) Controller

  pg. 489 to 504, PXA255 Processor Developers Manual [278693-002].pdf

*/

READ32_MEMBER(_39in1_state::pxa255_i2s_r)
{
	PXA255_I2S_Regs *i2s_regs = &m_i2s_regs;

	switch(PXA255_I2S_BASE_ADDR | (offset << 2))
	{
		case PXA255_SACR0:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Controller Global Control Register: %08x & %08x\n", i2s_regs->sacr0, mem_mask );
			return i2s_regs->sacr0;
		case PXA255_SACR1:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Controller I2S/MSB-Justified Control Register: %08x & %08x\n", i2s_regs->sacr1, mem_mask );
			return i2s_regs->sacr1;
		case PXA255_SASR0:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Controller I2S/MSB-Justified Status Register: %08x & %08x\n", i2s_regs->sasr0, mem_mask );
			return i2s_regs->sasr0;
		case PXA255_SAIMR:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Interrupt Mask Register: %08x & %08x\n", i2s_regs->saimr, mem_mask );
			return i2s_regs->saimr;
		case PXA255_SAICR:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Interrupt Clear Register: %08x & %08x\n", i2s_regs->saicr, mem_mask );
			return i2s_regs->saicr;
		case PXA255_SADIV:
			verboselog(*this, 3, "pxa255_i2s_r: Serial Audio Clock Divider Register: %08x & %08x\n", i2s_regs->sadiv, mem_mask );
			return i2s_regs->sadiv;
		case PXA255_SADR:
			verboselog(*this, 5, "pxa255_i2s_r: Serial Audio Data Register: %08x & %08x\n", i2s_regs->sadr, mem_mask );
			return i2s_regs->sadr;
		default:
			verboselog(*this, 0, "pxa255_i2s_r: Unknown address: %08x\n", PXA255_I2S_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_i2s_w)
{
	PXA255_I2S_Regs *i2s_regs = &m_i2s_regs;

#if 0
	if(!audio_dump)
	{
		int count = 0;
		char filename[256];
		do
		{
			sprintf(filename, "39in1_%04d.raw", count++);
			audio_dump = fopen(filename, "rb");
		}while(audio_dump != NULL);
		count--;
		sprintf(filename, "39in1_%04d.raw", count);
		audio_dump = fopen(filename, "wb");
	}
#endif
	switch(PXA255_I2S_BASE_ADDR | (offset << 2))
	{
		case PXA255_SACR0:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Controller Global Control Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->sacr0 = data & 0x0000ff3d;
			break;
		case PXA255_SACR1:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Controller I2S/MSB-Justified Control Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->sacr1 = data & 0x00000039;
			break;
		case PXA255_SASR0:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Controller I2S/MSB-Justified Status Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->sasr0 = data & 0x0000ff7f;
			break;
		case PXA255_SAIMR:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Interrupt Mask Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->saimr = data & 0x00000078;
			break;
		case PXA255_SAICR:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Interrupt Clear Register: %08x & %08x\n", data, mem_mask );
			if(i2s_regs->saicr & PXA255_SAICR_ROR)
			{
				i2s_regs->sasr0 &= ~PXA255_SASR0_ROR;
			}
			if(i2s_regs->saicr & PXA255_SAICR_TUR)
			{
				i2s_regs->sasr0 &= ~PXA255_SASR0_TUR;
			}
			break;
		case PXA255_SADIV:
			verboselog(*this, 3, "pxa255_i2s_w: Serial Audio Clock Divider Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->sadiv = data & 0x0000007f;
			dmadac_set_frequency(&m_dmadac[0], 2, ((double)147600000 / (double)i2s_regs->sadiv) / 256.0);
			dmadac_enable(&m_dmadac[0], 2, 1);
			break;
		case PXA255_SADR:
			verboselog(*this, 4, "pxa255_i2s_w: Serial Audio Data Register: %08x & %08x\n", data, mem_mask );
			i2s_regs->sadr = data;
#if 0
			if(audio_dump)
			{
				fwrite(&data, 4, 1, audio_dump);
			}
#endif
			break;
		default:
			verboselog(*this, 0, "pxa255_i2s_w: Unknown address: %08x = %08x & %08x\n", PXA255_I2S_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 DMA controller (placeholder)

  pg. 151 to 182, PXA255 Processor Developers Manual [278693-002].pdf

*/

void _39in1_state::pxa255_dma_irq_check()
{
	PXA255_DMA_Regs *dma_regs = &m_dma_regs;
	int channel = 0;
	int set_intr = 0;

	for(channel = 0; channel < 16; channel++)
	{
		if (dma_regs->dcsr[channel] & (PXA255_DCSR_ENDINTR | PXA255_DCSR_STARTINTR | PXA255_DCSR_BUSERRINTR))
		{
			dma_regs->dint |= 1 << channel;
			set_intr = 1;
		}
		else
		{
			dma_regs->dint &= ~(1 << channel);
		}
	}

	pxa255_set_irq_line(PXA255_INT_DMA, set_intr);
}

void _39in1_state::pxa255_dma_load_descriptor_and_start(int channel)
{
	PXA255_DMA_Regs *dma_regs = &m_dma_regs;
	attotime period;

	// Shut down any transfers that are currently going on, software should be smart enough to check if a
	// transfer is running before starting another one on the same channel.
	if (dma_regs->timer[channel]->enabled())
	{
		dma_regs->timer[channel]->adjust(attotime::never);
	}

	// Load the next descriptor

	address_space &space = m_maincpu->space(AS_PROGRAM);
	dma_regs->dsadr[channel] = space.read_dword(dma_regs->ddadr[channel] + 0x4);
	dma_regs->dtadr[channel] = space.read_dword(dma_regs->ddadr[channel] + 0x8);
	dma_regs->dcmd[channel]  = space.read_dword(dma_regs->ddadr[channel] + 0xc);
	dma_regs->ddadr[channel] = space.read_dword(dma_regs->ddadr[channel]);

	// Start our end-of-transfer timer
	switch(channel)
	{
		case 3:
			period = attotime::from_hz((147600000 / m_i2s_regs.sadiv) / (4 * 64)) * (dma_regs->dcmd[channel] & 0x00001fff);
			break;
		default:
			period = attotime::from_hz(100000000) * (dma_regs->dcmd[channel] & 0x00001fff);
			break;
	}

	dma_regs->timer[channel]->adjust(period, channel);

	// Interrupt as necessary
	if(dma_regs->dcmd[channel] & PXA255_DCMD_STARTIRQEN)
	{
		dma_regs->dcsr[channel] |= PXA255_DCSR_STARTINTR;
	}

	dma_regs->dcsr[channel] &= ~PXA255_DCSR_STOPSTATE;
}

TIMER_CALLBACK_MEMBER(_39in1_state::pxa255_dma_dma_end)
{
	PXA255_DMA_Regs *dma_regs = &m_dma_regs;
	UINT32 sadr = dma_regs->dsadr[param];
	UINT32 tadr = dma_regs->dtadr[param];
	UINT32 count = dma_regs->dcmd[param] & 0x00001fff;
	UINT32 index = 0;
	UINT8 temp8;
	UINT16 temp16;
	UINT32 temp32;

	address_space &space = m_maincpu->space(AS_PROGRAM);
	switch(param)
	{
		case 3:
			for(index = 0; index < count; index += 4)
			{
				m_words[index >> 2] = space.read_dword(sadr);
				m_samples[(index >> 1) + 0] = (INT16)(m_words[index >> 2] >> 16);
				m_samples[(index >> 1) + 1] = (INT16)(m_words[index >> 2] & 0xffff);
				sadr += 4;
			}
			dmadac_transfer(&m_dmadac[0], 2, 2, 2, count/4, m_samples);
			break;
		default:
			for(index = 0; index < count;)
			{
				switch(dma_regs->dcmd[param] & PXA255_DCMD_SIZE)
				{
					case PXA255_DCMD_SIZE_8:
						temp8 = space.read_byte(sadr);
						space.write_byte(tadr, temp8);
						index++;
						break;
					case PXA255_DCMD_SIZE_16:
						temp16 = space.read_word(sadr);
						space.write_word(tadr, temp16);
						index += 2;
						break;
					case PXA255_DCMD_SIZE_32:
						temp32 = space.read_dword(sadr);
						space.write_dword(tadr, temp32);
						index += 4;
						break;
					default:
						printf( "pxa255_dma_dma_end: Unsupported DMA size\n" );
						break;
				}
				if(dma_regs->dcmd[param] & PXA255_DCMD_INCSRCADDR)
				{
					switch(dma_regs->dcmd[param] & PXA255_DCMD_SIZE)
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
				if(dma_regs->dcmd[param] & PXA255_DCMD_INCTRGADDR)
				{
					switch(dma_regs->dcmd[param] & PXA255_DCMD_SIZE)
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
	if(dma_regs->dcmd[param] & PXA255_DCMD_ENDIRQEN)
	{
		dma_regs->dcsr[param] |= PXA255_DCSR_ENDINTR;
	}
	if(!(dma_regs->ddadr[param] & PXA255_DDADR_STOP) &&
		(dma_regs->dcsr[param] & PXA255_DCSR_RUN))
	{
		if(dma_regs->dcsr[param] & PXA255_DCSR_RUN)
		{
			pxa255_dma_load_descriptor_and_start(param);
		}
		else
		{
			dma_regs->dcsr[param] &= ~PXA255_DCSR_RUN;
			dma_regs->dcsr[param] |= PXA255_DCSR_STOPSTATE;
		}
	}
	else
	{
		dma_regs->dcsr[param] &= ~PXA255_DCSR_RUN;
		dma_regs->dcsr[param] |= PXA255_DCSR_STOPSTATE;
	}
	pxa255_dma_irq_check();
}

READ32_MEMBER(_39in1_state::pxa255_dma_r)
{
	PXA255_DMA_Regs *dma_regs = &m_dma_regs;

	switch(PXA255_DMA_BASE_ADDR | (offset << 2))
	{
		case PXA255_DCSR0:      case PXA255_DCSR1:      case PXA255_DCSR2:      case PXA255_DCSR3:
		case PXA255_DCSR4:      case PXA255_DCSR5:      case PXA255_DCSR6:      case PXA255_DCSR7:
		case PXA255_DCSR8:      case PXA255_DCSR9:      case PXA255_DCSR10:     case PXA255_DCSR11:
		case PXA255_DCSR12:     case PXA255_DCSR13:     case PXA255_DCSR14:     case PXA255_DCSR15:
			verboselog(*this, 4, "pxa255_dma_r: DMA Channel Control/Status Register %d: %08x & %08x\n", offset, dma_regs->dcsr[offset], mem_mask );
			return dma_regs->dcsr[offset];
		case PXA255_DINT:
			if (0) verboselog(*this, 3, "pxa255_dma_r: DMA Interrupt Register: %08x & %08x\n", dma_regs->dint, mem_mask );
			return dma_regs->dint;
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
			verboselog(*this, 3, "pxa255_dma_r: DMA Request to Channel Map Register %d: %08x & %08x\n", offset - (0x100 >> 2), 0, mem_mask );
			return dma_regs->drcmr[offset - (0x100 >> 2)];
		case PXA255_DDADR0:     case PXA255_DDADR1:     case PXA255_DDADR2:     case PXA255_DDADR3:
		case PXA255_DDADR4:     case PXA255_DDADR5:     case PXA255_DDADR6:     case PXA255_DDADR7:
		case PXA255_DDADR8:     case PXA255_DDADR9:     case PXA255_DDADR10:    case PXA255_DDADR11:
		case PXA255_DDADR12:    case PXA255_DDADR13:    case PXA255_DDADR14:    case PXA255_DDADR15:
			verboselog(*this, 3, "pxa255_dma_r: DMA Descriptor Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask );
			return dma_regs->ddadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DSADR0:     case PXA255_DSADR1:     case PXA255_DSADR2:     case PXA255_DSADR3:
		case PXA255_DSADR4:     case PXA255_DSADR5:     case PXA255_DSADR6:     case PXA255_DSADR7:
		case PXA255_DSADR8:     case PXA255_DSADR9:     case PXA255_DSADR10:    case PXA255_DSADR11:
		case PXA255_DSADR12:    case PXA255_DSADR13:    case PXA255_DSADR14:    case PXA255_DSADR15:
			verboselog(*this, 3, "pxa255_dma_r: DMA Source Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask );
			return dma_regs->dsadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DTADR0:     case PXA255_DTADR1:     case PXA255_DTADR2:     case PXA255_DTADR3:
		case PXA255_DTADR4:     case PXA255_DTADR5:     case PXA255_DTADR6:     case PXA255_DTADR7:
		case PXA255_DTADR8:     case PXA255_DTADR9:     case PXA255_DTADR10:    case PXA255_DTADR11:
		case PXA255_DTADR12:    case PXA255_DTADR13:    case PXA255_DTADR14:    case PXA255_DTADR15:
			verboselog(*this, 3, "pxa255_dma_r: DMA Target Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask );
			return dma_regs->dtadr[(offset - (0x200 >> 2)) >> 2];
		case PXA255_DCMD0:      case PXA255_DCMD1:      case PXA255_DCMD2:      case PXA255_DCMD3:
		case PXA255_DCMD4:      case PXA255_DCMD5:      case PXA255_DCMD6:      case PXA255_DCMD7:
		case PXA255_DCMD8:      case PXA255_DCMD9:      case PXA255_DCMD10:     case PXA255_DCMD11:
		case PXA255_DCMD12:     case PXA255_DCMD13:     case PXA255_DCMD14:     case PXA255_DCMD15:
			verboselog(*this, 3, "pxa255_dma_r: DMA Command Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, 0, mem_mask );
			return dma_regs->dcmd[(offset - (0x200 >> 2)) >> 2];
		default:
			verboselog(*this, 0, "pxa255_dma_r: Unknown address: %08x\n", PXA255_DMA_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_dma_w)
{
	PXA255_DMA_Regs *dma_regs = &m_dma_regs;

	switch(PXA255_DMA_BASE_ADDR | (offset << 2))
	{
		case PXA255_DCSR0:      case PXA255_DCSR1:      case PXA255_DCSR2:      case PXA255_DCSR3:
		case PXA255_DCSR4:      case PXA255_DCSR5:      case PXA255_DCSR6:      case PXA255_DCSR7:
		case PXA255_DCSR8:      case PXA255_DCSR9:      case PXA255_DCSR10:     case PXA255_DCSR11:
		case PXA255_DCSR12:     case PXA255_DCSR13:     case PXA255_DCSR14:     case PXA255_DCSR15:
			if (0) verboselog(*this, 3, "pxa255_dma_w: DMA Channel Control/Status Register %d: %08x & %08x\n", offset, data, mem_mask );
			dma_regs->dcsr[offset] &= ~(data & 0x00000007);
			dma_regs->dcsr[offset] &= ~0x60000000;
			dma_regs->dcsr[offset] |= data & 0x60000000;
			if((data & PXA255_DCSR_RUN) && !(dma_regs->dcsr[offset] & PXA255_DCSR_RUN))
			{
				dma_regs->dcsr[offset] |= PXA255_DCSR_RUN;
				if(data & PXA255_DCSR_NODESCFETCH)
				{
					verboselog(*this, 0, "              No-Descriptor-Fetch mode is not supported.\n" );
					break;
				}

				pxa255_dma_load_descriptor_and_start(offset);
			}
			else if(!(data & PXA255_DCSR_RUN))
			{
				dma_regs->dcsr[offset] &= ~PXA255_DCSR_RUN;
			}

			pxa255_dma_irq_check();
			break;
		case PXA255_DINT:
			verboselog(*this, 3, "pxa255_dma_w: DMA Interrupt Register: %08x & %08x\n", data, mem_mask );
			dma_regs->dint &= ~data;
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
			verboselog(*this, 3, "pxa255_dma_w: DMA Request to Channel Map Register %d: %08x & %08x\n", offset - (0x100 >> 2), data, mem_mask );
			dma_regs->drcmr[offset - (0x100 >> 2)] = data & 0x0000008f;
			break;
		case PXA255_DDADR0:     case PXA255_DDADR1:     case PXA255_DDADR2:     case PXA255_DDADR3:
		case PXA255_DDADR4:     case PXA255_DDADR5:     case PXA255_DDADR6:     case PXA255_DDADR7:
		case PXA255_DDADR8:     case PXA255_DDADR9:     case PXA255_DDADR10:    case PXA255_DDADR11:
		case PXA255_DDADR12:    case PXA255_DDADR13:    case PXA255_DDADR14:    case PXA255_DDADR15:
			verboselog(*this, 3, "pxa255_dma_w: DMA Descriptor Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask );
			dma_regs->ddadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffff1;
			break;
		case PXA255_DSADR0:     case PXA255_DSADR1:     case PXA255_DSADR2:     case PXA255_DSADR3:
		case PXA255_DSADR4:     case PXA255_DSADR5:     case PXA255_DSADR6:     case PXA255_DSADR7:
		case PXA255_DSADR8:     case PXA255_DSADR9:     case PXA255_DSADR10:    case PXA255_DSADR11:
		case PXA255_DSADR12:    case PXA255_DSADR13:    case PXA255_DSADR14:    case PXA255_DSADR15:
			verboselog(*this, 3, "pxa255_dma_w: DMA Source Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask );
			dma_regs->dsadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffffc;
			break;
		case PXA255_DTADR0:     case PXA255_DTADR1:     case PXA255_DTADR2:     case PXA255_DTADR3:
		case PXA255_DTADR4:     case PXA255_DTADR5:     case PXA255_DTADR6:     case PXA255_DTADR7:
		case PXA255_DTADR8:     case PXA255_DTADR9:     case PXA255_DTADR10:    case PXA255_DTADR11:
		case PXA255_DTADR12:    case PXA255_DTADR13:    case PXA255_DTADR14:    case PXA255_DTADR15:
			verboselog(*this, 3, "pxa255_dma_w: DMA Target Address Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask );
			dma_regs->dtadr[(offset - (0x200 >> 2)) >> 2] = data & 0xfffffffc;
			break;
		case PXA255_DCMD0:      case PXA255_DCMD1:      case PXA255_DCMD2:      case PXA255_DCMD3:
		case PXA255_DCMD4:      case PXA255_DCMD5:      case PXA255_DCMD6:      case PXA255_DCMD7:
		case PXA255_DCMD8:      case PXA255_DCMD9:      case PXA255_DCMD10:     case PXA255_DCMD11:
		case PXA255_DCMD12:     case PXA255_DCMD13:     case PXA255_DCMD14:     case PXA255_DCMD15:
			verboselog(*this, 3, "pxa255_dma_w: DMA Command Register %d: %08x & %08x\n", (offset - (0x200 >> 2)) >> 2, data, mem_mask );
			dma_regs->dcmd[(offset - (0x200 >> 2)) >> 2] = data & 0xf067dfff;
			break;
		default:
			verboselog(*this, 0, "pxa255_dma_w: Unknown address: %08x = %08x & %08x\n", PXA255_DMA_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 OS Timer register

  pg. 138 to 142, PXA255 Processor Developers Manual [278693-002].pdf

*/

void _39in1_state::pxa255_ostimer_irq_check()
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	pxa255_set_irq_line(PXA255_INT_OSTIMER0, (ostimer_regs->oier & PXA255_OIER_E0) ? ((ostimer_regs->ossr & PXA255_OSSR_M0) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER1, (ostimer_regs->oier & PXA255_OIER_E1) ? ((ostimer_regs->ossr & PXA255_OSSR_M1) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER2, (ostimer_regs->oier & PXA255_OIER_E2) ? ((ostimer_regs->ossr & PXA255_OSSR_M2) ? 1 : 0) : 0);
	//pxa255_set_irq_line(PXA255_INT_OSTIMER3, (ostimer_regs->oier & PXA255_OIER_E3) ? ((ostimer_regs->ossr & PXA255_OSSR_M3) ? 1 : 0) : 0);
}

TIMER_CALLBACK_MEMBER(_39in1_state::pxa255_ostimer_match)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	if (0) verboselog(*this, 3, "pxa255_ostimer_match channel %d\n", param);
	ostimer_regs->ossr |= (1 << param);
	ostimer_regs->oscr = ostimer_regs->osmr[param];
	pxa255_ostimer_irq_check();
}

READ32_MEMBER(_39in1_state::pxa255_ostimer_r)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 0: %08x & %08x\n", ostimer_regs->osmr[0], mem_mask );
			return ostimer_regs->osmr[0];
		case PXA255_OSMR1:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 1: %08x & %08x\n", ostimer_regs->osmr[1], mem_mask );
			return ostimer_regs->osmr[1];
		case PXA255_OSMR2:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 2: %08x & %08x\n", ostimer_regs->osmr[2], mem_mask );
			return ostimer_regs->osmr[2];
		case PXA255_OSMR3:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Match Register 3: %08x & %08x\n", ostimer_regs->osmr[3], mem_mask );
			return ostimer_regs->osmr[3];
		case PXA255_OSCR:
			if (0) verboselog(*this, 4, "pxa255_ostimer_r: OS Timer Count Register: %08x & %08x\n", ostimer_regs->oscr, mem_mask );
			// free-running 3.something MHz counter.  this is a complete hack.
			ostimer_regs->oscr += 0x300;
			return ostimer_regs->oscr;
		case PXA255_OSSR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Status Register: %08x & %08x\n", ostimer_regs->ossr, mem_mask );
			return ostimer_regs->ossr;
		case PXA255_OWER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", ostimer_regs->ower, mem_mask );
			return ostimer_regs->ower;
		case PXA255_OIER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", ostimer_regs->oier, mem_mask );
			return ostimer_regs->oier;
		default:
			if (0) verboselog(*this, 0, "pxa255_ostimer_r: Unknown address: %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_ostimer_w)
{
	PXA255_OSTMR_Regs *ostimer_regs = &m_ostimer_regs;

	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 0: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[0] = data;
			if(ostimer_regs->oier & PXA255_OIER_E0)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[0] - ostimer_regs->oscr);

				//printf( "Adjusting one-shot timer to 200MHz * %08x\n", ostimer_regs->osmr[0]);
				ostimer_regs->timer[0]->adjust(period);
			}
			break;
		case PXA255_OSMR1:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 1: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[1] = data;
			if(ostimer_regs->oier & PXA255_OIER_E1)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[1] - ostimer_regs->oscr);

				ostimer_regs->timer[1]->adjust(period, 1);
			}
			break;
		case PXA255_OSMR2:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 2: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[2] = data;
			if(ostimer_regs->oier & PXA255_OIER_E2)
			{
				attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[2] - ostimer_regs->oscr);

				ostimer_regs->timer[2]->adjust(period, 2);
			}
			break;
		case PXA255_OSMR3:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Match Register 3: %08x & %08x\n", data, mem_mask );
			ostimer_regs->osmr[3] = data;
			if(ostimer_regs->oier & PXA255_OIER_E3)
			{
				//attotime period = attotime::from_hz(3846400) * (ostimer_regs->osmr[3] - ostimer_regs->oscr);

				//ostimer_regs->timer[3]->adjust(period, 3);
			}
			break;
		case PXA255_OSCR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Count Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oscr = data;
			break;
		case PXA255_OSSR:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Status Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ossr &= ~data;
			pxa255_ostimer_irq_check();
			break;
		case PXA255_OWER:
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Watchdog Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->ower = data & 0x00000001;
			break;
		case PXA255_OIER:
		{
			int index = 0;
			if (0) verboselog(*this, 3, "pxa255_ostimer_w: OS Timer Interrupt Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs->oier = data & 0x0000000f;
			for(index = 0; index < 4; index++)
			{
				if(ostimer_regs->oier & (1 << index))
				{
					//attotime period = attotime::from_hz(200000000) * ostimer_regs->osmr[index];

					//ostimer_regs->timer[index]->adjust(period, index);
				}
			}

			break;
		}
		default:
			verboselog(*this, 0, "pxa255_ostimer_w: Unknown address: %08x = %08x & %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 Interrupt registers

  pg. 124 to 132, PXA255 Processor Developers Manual [278693-002].pdf

*/

void _39in1_state::pxa255_update_interrupts()
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icfp = (intc_regs->icpr & intc_regs->icmr) & intc_regs->iclr;
	intc_regs->icip = (intc_regs->icpr & intc_regs->icmr) & (~intc_regs->iclr);
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, intc_regs->icfp ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(ARM7_IRQ_LINE,  intc_regs->icip ? ASSERT_LINE : CLEAR_LINE);
}

void _39in1_state::pxa255_set_irq_line(UINT32 line, int irq_state)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	intc_regs->icpr &= ~line;
	intc_regs->icpr |= irq_state ? line : 0;
	//printf( "Setting IRQ line %08x to %d\n", line, irq_state );
	pxa255_update_interrupts();
}

READ32_MEMBER(_39in1_state::pxa255_intc_r)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", intc_regs->icip, mem_mask );
			return intc_regs->icip;
		case PXA255_ICMR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Mask Register: %08x & %08x\n", intc_regs->icmr, mem_mask );
			return intc_regs->icmr;
		case PXA255_ICLR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Level Register: %08x & %08x\n", intc_regs->iclr, mem_mask );
			return intc_regs->iclr;
		case PXA255_ICFP:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", intc_regs->icfp, mem_mask );
			return intc_regs->icfp;
		case PXA255_ICPR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Pending Register: %08x & %08x\n", intc_regs->icpr, mem_mask );
			return intc_regs->icpr;
		case PXA255_ICCR:
			if (0) verboselog(*this, 3, "pxa255_intc_r: Interrupt Controller Control Register: %08x & %08x\n", intc_regs->iccr, mem_mask );
			return intc_regs->iccr;
		default:
			verboselog(*this, 0, "pxa255_intc_r: Unknown address: %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_intc_w)
{
	PXA255_INTC_Regs *intc_regs = &m_intc_regs;

	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICMR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask );
			intc_regs->icmr = data & 0xfffe7f00;
			break;
		case PXA255_ICLR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iclr = data & 0xfffe7f00;
			break;
		case PXA255_ICFP:
			if (0) verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICPR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICCR:
			if (0) verboselog(*this, 3, "pxa255_intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask );
			intc_regs->iccr = data & 0x00000001;
			break;
		default:
			verboselog(*this, 0, "pxa255_intc_w: Unknown address: %08x = %08x & %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 General-Purpose I/O registers

  pg. 105 to 124, PXA255 Processor Developers Manual [278693-002].pdf

*/

READ32_MEMBER(_39in1_state::pxa255_gpio_r)
{
	PXA255_GPIO_Regs *gpio_regs = &m_gpio_regs;

	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Pin-Level Register 0: %08x & %08x\n", gpio_regs->gplr0 | (1 << 1), mem_mask );
			return gpio_regs->gplr0 | (1 << 1) | (m_eeprom->do_read() << 5); // Must be on.  Probably a DIP switch.
		case PXA255_GPLR1:
			verboselog(*this, 3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 1: %08x & %08x\n", gpio_regs->gplr1, mem_mask );
			return 0xff9fffff;
		/*
		    0x200000 = flip screen
		*/
		case PXA255_GPLR2:
			verboselog(*this, 3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 2: %08x & %08x\n", gpio_regs->gplr2, mem_mask );
			return gpio_regs->gplr2;
		case PXA255_GPDR0:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Pin Direction Register 0: %08x & %08x\n", gpio_regs->gpdr0, mem_mask );
			return gpio_regs->gpdr0;
		case PXA255_GPDR1:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Pin Direction Register 1: %08x & %08x\n", gpio_regs->gpdr1, mem_mask );
			return gpio_regs->gpdr1;
		case PXA255_GPDR2:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Pin Direction Register 2: %08x & %08x\n", gpio_regs->gpdr2, mem_mask );
			return gpio_regs->gpdr2;
		case PXA255_GPSR0:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 0: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GPSR1:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 1: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GPSR2:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 2: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GPCR0:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 0: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GPCR1:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 1: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GPCR2:
			verboselog(*this, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 2: %08x & %08x\n", machine().rand(), mem_mask );
			return machine().rand();
		case PXA255_GRER0:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", gpio_regs->grer0, mem_mask );
			return gpio_regs->grer0;
		case PXA255_GRER1:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", gpio_regs->grer1, mem_mask );
			return gpio_regs->grer1;
		case PXA255_GRER2:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", gpio_regs->grer2, mem_mask );
			return gpio_regs->grer2;
		case PXA255_GFER0:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", gpio_regs->gfer0, mem_mask );
			return gpio_regs->gfer0;
		case PXA255_GFER1:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", gpio_regs->gfer1, mem_mask );
			return gpio_regs->gfer1;
		case PXA255_GFER2:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", gpio_regs->gfer2, mem_mask );
			return gpio_regs->gfer2;
		case PXA255_GEDR0:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 0: %08x & %08x\n", gpio_regs->gedr0, mem_mask );
			return gpio_regs->gedr0;
		case PXA255_GEDR1:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 1: %08x & %08x\n", gpio_regs->gedr1, mem_mask );
			return gpio_regs->gedr1;
		case PXA255_GEDR2:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 2: %08x & %08x\n", gpio_regs->gedr2, mem_mask );
			return gpio_regs->gedr2;
		case PXA255_GAFR0_L:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", gpio_regs->gafr0l, mem_mask );
			return gpio_regs->gafr0l;
		case PXA255_GAFR0_U:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", gpio_regs->gafr0u, mem_mask );
			return gpio_regs->gafr0u;
		case PXA255_GAFR1_L:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", gpio_regs->gafr1l, mem_mask );
			return gpio_regs->gafr1l;
		case PXA255_GAFR1_U:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", gpio_regs->gafr1u, mem_mask );
			return gpio_regs->gafr1u;
		case PXA255_GAFR2_L:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", gpio_regs->gafr2l, mem_mask );
			return gpio_regs->gafr2l;
		case PXA255_GAFR2_U:
			verboselog(*this, 3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", gpio_regs->gafr2u, mem_mask );
			return gpio_regs->gafr2u;
		default:
			verboselog(*this, 0, "pxa255_gpio_r: Unknown address: %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_gpio_w)
{
	PXA255_GPIO_Regs *gpio_regs = &m_gpio_regs;

	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
			verboselog(*this, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPLR1:
			verboselog(*this, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPLR2:
			verboselog(*this, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 2: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPDR0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Direction Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpdr0 = data;
			break;
		case PXA255_GPDR1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Direction Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpdr1 = data;
			break;
		case PXA255_GPDR2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Direction Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpdr2 = data;
			break;
		case PXA255_GPSR0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr0 |= data & gpio_regs->gpdr0;
			if(data & 0x00000004)
			{
				m_eeprom->cs_write(ASSERT_LINE);
			}
			if(data & 0x00000008)
			{
				m_eeprom->clk_write(ASSERT_LINE);
			}
			if(data & 0x00000010)
			{
				m_eeprom->di_write(1);
			}
			break;
		case PXA255_GPSR1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr1 |= data & gpio_regs->gpdr1;
			break;
		case PXA255_GPSR2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr2 |= data & gpio_regs->gpdr2;
			break;
		case PXA255_GPCR0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr0 &= ~(data & gpio_regs->gpdr0);
			if(data & 0x00000004)
			{
				m_eeprom->cs_write(ASSERT_LINE);
			}
			if(data & 0x00000008)
			{
				m_eeprom->clk_write(CLEAR_LINE);
			}
			if(data & 0x00000010)
			{
				m_eeprom->di_write(0);
			}
			break;
		case PXA255_GPCR1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr1 &= ~(data & gpio_regs->gpdr1);
			break;
		case PXA255_GPCR2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs->gpsr2 &= ~(data & gpio_regs->gpdr2);
			break;
		case PXA255_GRER0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs->grer0 = data;
			break;
		case PXA255_GRER1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs->grer1 = data;
			break;
		case PXA255_GRER2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs->grer2 = data;
			break;
		case PXA255_GFER0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs->gfer0 = data;
			break;
		case PXA255_GFER1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs->gfer1 = data;
			break;
		case PXA255_GFER2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs->gfer2 = data;
			break;
		case PXA255_GEDR0:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 0: %08x & %08x\n", gpio_regs->gedr0, mem_mask );
			gpio_regs->gedr0 &= ~data;
			break;
		case PXA255_GEDR1:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 1: %08x & %08x\n", gpio_regs->gedr1, mem_mask );
			gpio_regs->gedr1 &= ~data;
			break;
		case PXA255_GEDR2:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 2: %08x & %08x\n", gpio_regs->gedr2, mem_mask );
			gpio_regs->gedr2 &= ~data;
			break;
		case PXA255_GAFR0_L:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", gpio_regs->gafr0l, mem_mask );
			gpio_regs->gafr0l = data;
			break;
		case PXA255_GAFR0_U:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", gpio_regs->gafr0u, mem_mask );
			gpio_regs->gafr0u = data;
			break;
		case PXA255_GAFR1_L:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", gpio_regs->gafr1l, mem_mask );
			gpio_regs->gafr1l = data;
			break;
		case PXA255_GAFR1_U:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", gpio_regs->gafr1u, mem_mask );
			gpio_regs->gafr1u = data;
			break;
		case PXA255_GAFR2_L:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", gpio_regs->gafr2l, mem_mask );
			gpio_regs->gafr2l = data;
			break;
		case PXA255_GAFR2_U:
			verboselog(*this, 3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", gpio_regs->gafr2u, mem_mask );
			gpio_regs->gafr2u = data;
			break;
		default:
			verboselog(*this, 0, "pxa255_gpio_w: Unknown address: %08x = %08x & %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 LCD Controller

  pg. 265 to 310, PXA255 Processor Developers Manual [278693-002].pdf

*/

void _39in1_state::pxa255_lcd_load_dma_descriptor(address_space & space, UINT32 address, int channel)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	lcd_regs->dma[channel].fdadr = space.read_dword(address);
	lcd_regs->dma[channel].fsadr = space.read_dword(address + 0x04);
	lcd_regs->dma[channel].fidr  = space.read_dword(address + 0x08);
	lcd_regs->dma[channel].ldcmd = space.read_dword(address + 0x0c);
	verboselog(*this, 4, "pxa255_lcd_load_dma_descriptor, address = %08x, channel = %d\n", address, channel);
	verboselog(*this, 4, "    DMA Frame Descriptor: %08x\n", lcd_regs->dma[channel].fdadr );
	verboselog(*this, 4, "    DMA Frame Source Address: %08x\n", lcd_regs->dma[channel].fsadr );
	verboselog(*this, 4, "    DMA Frame ID: %08x\n", lcd_regs->dma[channel].fidr );
	verboselog(*this, 4, "    DMA Command: %08x\n", lcd_regs->dma[channel].ldcmd );
}

void _39in1_state::pxa255_lcd_irq_check()
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	if(((lcd_regs->lcsr & PXA255_LCSR_BS)  != 0 && (lcd_regs->lccr0 & PXA255_LCCR0_BM)  == 0) ||
		((lcd_regs->lcsr & PXA255_LCSR_EOF) != 0 && (lcd_regs->lccr0 & PXA255_LCCR0_EFM) == 0) ||
		((lcd_regs->lcsr & PXA255_LCSR_SOF) != 0 && (lcd_regs->lccr0 & PXA255_LCCR0_SFM) == 0))
	{
		pxa255_set_irq_line(PXA255_INT_LCD, 1);
	}
	else
	{
		pxa255_set_irq_line(PXA255_INT_LCD, 0);
	}
}

void _39in1_state::pxa255_lcd_dma_kickoff(int channel)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	if(lcd_regs->dma[channel].fdadr != 0)
	{
		attotime period = attotime::from_hz(20000000) * (lcd_regs->dma[channel].ldcmd & 0x000fffff);

		lcd_regs->dma[channel].eof->adjust(period, channel);

		if(lcd_regs->dma[channel].ldcmd & PXA255_LDCMD_SOFINT)
		{
			lcd_regs->liidr = lcd_regs->dma[channel].fidr;
			lcd_regs->lcsr |= PXA255_LCSR_SOF;
			pxa255_lcd_irq_check();
		}

		if(lcd_regs->dma[channel].ldcmd & PXA255_LDCMD_PAL)
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = lcd_regs->dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index += 2)
			{
				UINT16 color = space.read_word((lcd_regs->dma[channel].fsadr &~ 1) + index);
				m_pxa255_lcd_palette[index >> 1] = (((((color >> 11) & 0x1f) << 3) | (color >> 13)) << 16) | (((((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3)) << 8) | (((color & 0x1f) << 3) | ((color >> 2) & 0x7));
				m_palette->set_pen_color(index >> 1, (((color >> 11) & 0x1f) << 3) | (color >> 13), (((color >> 5) & 0x3f) << 2) | ((color >> 9) & 0x3), ((color & 0x1f) << 3) | ((color >> 2) & 0x7));
			}
		}
		else
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);
			int length = lcd_regs->dma[channel].ldcmd & 0x000fffff;
			int index = 0;
			for(index = 0; index < length; index++)
			{
				m_pxa255_lcd_framebuffer[index] = space.read_byte(lcd_regs->dma[channel].fsadr + index);
			}
		}
	}
}

void _39in1_state::pxa255_lcd_check_load_next_branch(int channel)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	if(lcd_regs->fbr[channel] & 1)
	{
		verboselog(*this, 4, "pxa255_lcd_check_load_next_branch: Taking branch\n" );
		lcd_regs->fbr[channel] &= ~1;
		address_space &space = m_maincpu->space(AS_PROGRAM);
		//lcd_regs->fbr[channel] = (space.read_dword(lcd_regs->fbr[channel] & 0xfffffff0) & 0xfffffff0) | (lcd_regs->fbr[channel] & 0x00000003);
		//printf( "%08x\n", lcd_regs->fbr[channel] );
		pxa255_lcd_load_dma_descriptor(space, lcd_regs->fbr[channel] & 0xfffffff0, 0);
		lcd_regs->fbr[channel] = (space.read_dword(lcd_regs->fbr[channel] & 0xfffffff0) & 0xfffffff0) | (lcd_regs->fbr[channel] & 0x00000003);
		pxa255_lcd_dma_kickoff(0);
		if(lcd_regs->fbr[channel] & 2)
		{
			lcd_regs->fbr[channel] &= ~2;
			if(!(lcd_regs->lccr0 & PXA255_LCCR0_BM))
			{
				lcd_regs->lcsr |= PXA255_LCSR_BS;
			}
		}
	}
	else
	{
		if (0) verboselog(*this, 3, "pxa255_lcd_check_load_next_branch: Not taking branch\n" );
	}
}

TIMER_CALLBACK_MEMBER(_39in1_state::pxa255_lcd_dma_eof)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	if (0) verboselog(*this, 3, "End of frame callback\n" );
	if(lcd_regs->dma[param].ldcmd & PXA255_LDCMD_EOFINT)
	{
		lcd_regs->liidr = lcd_regs->dma[param].fidr;
		lcd_regs->lcsr |= PXA255_LCSR_EOF;
	}
	pxa255_lcd_check_load_next_branch(param);
	pxa255_lcd_irq_check();
}

READ32_MEMBER(_39in1_state::pxa255_lcd_r)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:      // 0x44000000
			verboselog(*this, 3, "pxa255_lcd_r: LCD Control 0: %08x & %08x\n", lcd_regs->lccr0, mem_mask );
			return lcd_regs->lccr0;
		case PXA255_LCCR1:      // 0x44000004
			verboselog(*this, 3, "pxa255_lcd_r: LCD Control 1: %08x & %08x\n", lcd_regs->lccr1, mem_mask );
			return lcd_regs->lccr1;
		case PXA255_LCCR2:      // 0x44000008
			verboselog(*this, 3, "pxa255_lcd_r: LCD Control 2: %08x & %08x\n", lcd_regs->lccr2, mem_mask );
			return lcd_regs->lccr2;
		case PXA255_LCCR3:      // 0x4400000c
			verboselog(*this, 3, "pxa255_lcd_r: LCD Control 3: %08x & %08x\n", lcd_regs->lccr3, mem_mask );
			return lcd_regs->lccr3;
		case PXA255_FBR0:       // 0x44000020
			verboselog(*this, 4, "pxa255_lcd_r: LCD Frame Branch Register 0: %08x & %08x\n", lcd_regs->fbr[0], mem_mask );
			return lcd_regs->fbr[0];
		case PXA255_FBR1:       // 0x44000024
			verboselog(*this, 3, "pxa255_lcd_r: LCD Frame Branch Register 1: %08x & %08x\n", lcd_regs->fbr[1], mem_mask );
			return lcd_regs->fbr[1];
		case PXA255_LCSR:       // 0x44000038
			verboselog(*this, 4, "pxa255_lcd_r: LCD Status Register: %08x & %08x\n", lcd_regs->lcsr, mem_mask );
			return lcd_regs->lcsr;
		case PXA255_LIIDR:      // 0x4400003c
			verboselog(*this, 3, "pxa255_lcd_r: LCD Interrupt ID Register: %08x & %08x\n", lcd_regs->liidr, mem_mask );
			return lcd_regs->liidr;
		case PXA255_TRGBR:      // 0x44000040
			verboselog(*this, 3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", lcd_regs->trgbr, mem_mask );
			return lcd_regs->trgbr;
		case PXA255_TCR:        // 0x44000044
			verboselog(*this, 3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", lcd_regs->tcr, mem_mask );
			return lcd_regs->tcr;
		case PXA255_FDADR0:     // 0x44000200
			if (0) verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", lcd_regs->dma[0].fdadr, mem_mask );
			return lcd_regs->dma[0].fdadr;
		case PXA255_FSADR0:     // 0x44000204
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 0: %08x & %08x\n", lcd_regs->dma[0].fsadr, mem_mask );
			return lcd_regs->dma[0].fsadr;
		case PXA255_FIDR0:      // 0x44000208
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame ID Register 0: %08x & %08x\n", lcd_regs->dma[0].fidr, mem_mask );
			return lcd_regs->dma[0].fidr;
		case PXA255_LDCMD0:     // 0x4400020c
			if (0) verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Command Register 0: %08x & %08x\n", lcd_regs->dma[0].ldcmd & 0xfff00000, mem_mask );
			return lcd_regs->dma[0].ldcmd & 0xfff00000;
		case PXA255_FDADR1:     // 0x44000210
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", lcd_regs->dma[1].fdadr, mem_mask );
			return lcd_regs->dma[1].fdadr;
		case PXA255_FSADR1:     // 0x44000214
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 1: %08x & %08x\n", lcd_regs->dma[1].fsadr, mem_mask );
			return lcd_regs->dma[1].fsadr;
		case PXA255_FIDR1:      // 0x44000218
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Frame ID Register 1: %08x & %08x\n", lcd_regs->dma[1].fidr, mem_mask );
			return lcd_regs->dma[1].fidr;
		case PXA255_LDCMD1:     // 0x4400021c
			verboselog(*this, 3, "pxa255_lcd_r: LCD DMA Command Register 1: %08x & %08x\n", lcd_regs->dma[1].ldcmd & 0xfff00000, mem_mask );
			return lcd_regs->dma[1].ldcmd & 0xfff00000;
		default:
			verboselog(*this, 0, "pxa255_lcd_r: Unknown address: %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

WRITE32_MEMBER(_39in1_state::pxa255_lcd_w)
{
	PXA255_LCD_Regs *lcd_regs = &m_lcd_regs;

	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:      // 0x44000000
			verboselog(*this, 3, "pxa255_lcd_w: LCD Control 0: %08x & %08x\n", data, mem_mask );
			lcd_regs->lccr0 = data & 0x00fffeff;
			break;
		case PXA255_LCCR1:      // 0x44000004
			verboselog(*this, 3, "pxa255_lcd_w: LCD Control 1: %08x & %08x\n", data, mem_mask );
			lcd_regs->lccr1 = data;
			break;
		case PXA255_LCCR2:      // 0x44000008
			verboselog(*this, 3, "pxa255_lcd_w: LCD Control 2: %08x & %08x\n", data, mem_mask );
			lcd_regs->lccr2 = data;
			break;
		case PXA255_LCCR3:      // 0x4400000c
			verboselog(*this, 3, "pxa255_lcd_w: LCD Control 3: %08x & %08x\n", data, mem_mask );
			lcd_regs->lccr3 = data;
			break;
		case PXA255_FBR0:       // 0x44000020
			verboselog(*this, 4l, "pxa255_lcd_w: LCD Frame Branch Register 0: %08x & %08x\n", data, mem_mask );
			lcd_regs->fbr[0] = data & 0xfffffff3;
			if(!lcd_regs->dma[0].eof->enabled())
			{
				if (0) verboselog(*this, 3, "ch0 EOF timer is not enabled, taking branch now\n" );
				pxa255_lcd_check_load_next_branch(0);
				pxa255_lcd_irq_check();
			}
			break;
		case PXA255_FBR1:       // 0x44000024
			verboselog(*this, 3, "pxa255_lcd_w: LCD Frame Branch Register 1: %08x & %08x\n", data, mem_mask );
			lcd_regs->fbr[1] = data & 0xfffffff3;
			if(!lcd_regs->dma[1].eof->enabled())
			{
				verboselog(*this, 3, "ch1 EOF timer is not enabled, taking branch now\n" );
				pxa255_lcd_check_load_next_branch(1);
				pxa255_lcd_irq_check();
			}
			break;
		case PXA255_LCSR:       // 0x44000038
			verboselog(*this, 4, "pxa255_lcd_w: LCD Controller Status Register: %08x & %08x\n", data, mem_mask );
			lcd_regs->lcsr &= ~data;
			pxa255_lcd_irq_check();
			break;
		case PXA255_LIIDR:      // 0x4400003c
			verboselog(*this, 3, "pxa255_lcd_w: LCD Controller Interrupt ID Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_TRGBR:      // 0x44000040
			verboselog(*this, 3, "pxa255_lcd_w: TMED RGB Seed Register: %08x & %08x\n", data, mem_mask );
			lcd_regs->trgbr = data & 0x00ffffff;
			break;
		case PXA255_TCR:        // 0x44000044
			verboselog(*this, 3, "pxa255_lcd_w: TMED Control Register: %08x & %08x\n", data, mem_mask );
			lcd_regs->tcr = data & 0x00004fff;
			break;
		case PXA255_FDADR0:     // 0x44000200
			verboselog(*this, 4, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", data, mem_mask );
			if(!lcd_regs->dma[0].eof->enabled())
			{
				pxa255_lcd_load_dma_descriptor(space, data & 0xfffffff0, 0);
			}
			else
			{
				lcd_regs->fbr[0] &= 0x00000003;
				lcd_regs->fbr[0] |= data & 0xfffffff0;
			}
			break;
		case PXA255_FSADR0:     // 0x44000204
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FIDR0:      // 0x44000208
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_LDCMD0:     // 0x4400020c
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FDADR1:     // 0x44000210
			verboselog(*this, 4, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", data, mem_mask );
			if(!lcd_regs->dma[1].eof->enabled())
			{
				pxa255_lcd_load_dma_descriptor(space, data & 0xfffffff0, 1);
			}
			else
			{
				lcd_regs->fbr[1] &= 0x00000003;
				lcd_regs->fbr[1] |= data & 0xfffffff0;
			}
			break;
		case PXA255_FSADR1:     // 0x44000214
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FIDR1:      // 0x44000218
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_LDCMD1:     // 0x4400021c
			verboselog(*this, 4, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 1: %08x & %08x\n", data, mem_mask );
			break;
		default:
			verboselog(*this, 0, "pxa255_lcd_w: Unknown address: %08x = %08x & %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

INTERRUPT_GEN_MEMBER(_39in1_state::pxa255_vblank_start)
{
}

#ifdef UNUSED_FUNCTION
READ32_MEMBER(_39in1_state::return_zero)
{
	return 0;
}

READ32_MEMBER(_39in1_state::unknown_r)
{
	return 0x00008000;
}
#endif

READ32_MEMBER(_39in1_state::cpld_r)
{
	//if (space.device().safe_pc() != 0xe3af4) printf("CPLD read @ %x (PC %x state %d)\n", offset, space.device().safe_pc(), state);

	if (space.device().safe_pc() == 0x3f04)
	{
		return 0xf0;      // any non-zero value works here
	}
	else if (space.device().safe_pc() == 0xe3af4)
	{
		return ioport("MCUIPT")->read();
	}
	else
	{
		if (m_state == 0)
		{
			return 0;
		}
		else if (m_state == 1)
		{
			switch (offset & ~1)
			{
				case 0x40010: return 0x55;
				case 0x40012: return 0x93;
				case 0x40014: return 0x89;
				case 0x40016: return 0xa2;
				case 0x40018: return 0x31;
				case 0x4001a: return 0x75;
				case 0x4001c: return 0x97;
				case 0x4001e: return 0xb1;
				default: printf("State 1 unknown offset %x\n", offset); break;
			}
		}
		else if (m_state == 2)                      // 29c0: 53 ac 0c 2b a2 07 e6 be 31
		{
			UINT32 seed = m_seed;
			UINT32 magic = m_magic;

			magic = ( (((~(seed >> 16))       ^ (magic >> 1))        & 0x01) |
				(((~((seed >> 19) << 1))        ^ ((magic >> 5) << 1)) & 0x02) |
				(((~((seed >> 20) << 2))        ^ ((magic >> 3) << 2)) & 0x04) |
				(((~((seed >> 22) << 3))        ^ ((magic >> 6) << 3)) & 0x08) |
				(((~((seed >> 23) << 4))        ^   magic)             & 0x10) |
				(((~(((seed >> 16) >> 2) << 5)) ^ ((magic >> 2) << 5)) & 0x20) |
				(((~(((seed >> 16) >> 1) << 6)) ^ ((magic >> 7) << 6)) & 0x40) |
				(((~(((seed >> 16) >> 5) << 7)) ^  (magic << 7))       & 0x80));

			m_magic = magic;
			return magic;
		}
	}

	return 0;
}

WRITE32_MEMBER(_39in1_state::cpld_w)
{
	if (mem_mask == 0xffff)
	{
		m_seed = data<<16;
	}

	if (space.device().safe_pc() == 0x280c)
	{
		m_state = 1;
	}
	if (space.device().safe_pc() == 0x2874)
	{
		m_state = 2;
		m_magic = space.read_byte(0xa02d4ff0);
	}
	else if (offset == 0xa)
	{
	}
#if 0
	else
	{
		printf("%08x: CPLD_W: %08x = %08x & %08x\n", space.device().safe_pc(), offset, data, mem_mask);
	}
#endif
}

READ32_MEMBER(_39in1_state::prot_cheater_r)
{
	return 0x37;
}

DRIVER_INIT_MEMBER(_39in1_state,39in1)
{
	m_dmadac[0] = machine().device<dmadac_sound_device>("dac1");
	m_dmadac[1] = machine().device<dmadac_sound_device>("dac2");

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_read_handler (0xa0151648, 0xa015164b, read32_delegate(FUNC(_39in1_state::prot_cheater_r), this));
}



static ADDRESS_MAP_START( 39in1_map, AS_PROGRAM, 32, _39in1_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
	AM_RANGE(0x00400000, 0x005fffff) AM_ROM AM_REGION("data", 0)
	AM_RANGE(0x04000000, 0x047fffff) AM_READWRITE(cpld_r, cpld_w )
	AM_RANGE(0x40000000, 0x400002ff) AM_READWRITE(pxa255_dma_r, pxa255_dma_w )
	AM_RANGE(0x40400000, 0x40400083) AM_READWRITE(pxa255_i2s_r, pxa255_i2s_w )
	AM_RANGE(0x40a00000, 0x40a0001f) AM_READWRITE(pxa255_ostimer_r, pxa255_ostimer_w )
	AM_RANGE(0x40d00000, 0x40d00017) AM_READWRITE(pxa255_intc_r, pxa255_intc_w )
	AM_RANGE(0x40e00000, 0x40e0006b) AM_READWRITE(pxa255_gpio_r, pxa255_gpio_w )
	AM_RANGE(0x44000000, 0x4400021f) AM_READWRITE(pxa255_lcd_r,  pxa255_lcd_w )
	AM_RANGE(0xa0000000, 0xa07fffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static INPUT_PORTS_START( 39in1 )
	PORT_START("MCUIPT")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x80000000, IP_ACTIVE_LOW )
INPUT_PORTS_END

UINT32 _39in1_state::screen_update_39in1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x = 0;
	int y = 0;

	for(y = 0; y <= (m_lcd_regs.lccr2 & PXA255_LCCR2_LPP); y++)
	{
		UINT32 *d = &bitmap.pix32(y);
		for(x = 0; x <= (m_lcd_regs.lccr1 & PXA255_LCCR1_PPL); x++)
		{
			d[x] = m_pxa255_lcd_palette[m_pxa255_lcd_framebuffer[y*((m_lcd_regs.lccr1 & PXA255_LCCR1_PPL) + 1) + x]];
		}
	}
	return 0;
}

/* To be moved to device start of pxa255 upon completion */
void _39in1_state::pxa255_start()
{
	int index = 0;

	for(index = 0; index < 16; index++)
	{
		m_dma_regs.dcsr[index] = 0x00000008;
		m_dma_regs.timer[index] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(_39in1_state::pxa255_dma_dma_end),this));
	}

	memset(&m_ostimer_regs, 0, sizeof(m_ostimer_regs));
	for(index = 0; index < 4; index++)
	{
		m_ostimer_regs.osmr[index] = 0;
		m_ostimer_regs.timer[index] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(_39in1_state::pxa255_ostimer_match),this));
	}

	memset(&m_intc_regs, 0, sizeof(m_intc_regs));

	memset(&m_lcd_regs, 0, sizeof(m_lcd_regs));
	m_lcd_regs.dma[0].eof = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(_39in1_state::pxa255_lcd_dma_eof),this));
	m_lcd_regs.dma[1].eof = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(_39in1_state::pxa255_lcd_dma_eof),this));
	m_lcd_regs.trgbr = 0x00aa5500;
	m_lcd_regs.tcr = 0x0000754f;
}

void _39in1_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	for (i = 0; i < 0x80000; i += 2)
	{
		ROM[i] = BITSWAP8(ROM[i],7,2,5,6,0,3,1,4) ^ BITSWAP8((i>>3)&0xf, 3,2,4,1,4,4,0,4) ^ 0x90;
	}

	pxa255_start();
}

MACHINE_START_MEMBER(_39in1_state,60in1)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	for (i = 0; i < 0x80000; i += 2)
	{
		if ((i%2)==0)
		{
			ROM[i] = BITSWAP8(ROM[i],5,1,4,2,0,7,6,3)^BITSWAP8(i, 6,0,4,13,0,5,3,11);
		}
	}

	pxa255_start();
}

static MACHINE_CONFIG_START( 39in1, _39in1_state )

	MCFG_CPU_ADD("maincpu", PXA255, 200000000)
	MCFG_CPU_PROGRAM_MAP(39in1_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _39in1_state,  pxa255_vblank_start)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024, 1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 295, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(_39in1_state, screen_update_39in1)

	MCFG_PALETTE_ADD("palette", 256)

	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( 60in1, 39in1 )
	MCFG_MACHINE_START_OVERRIDE(_39in1_state,60in1)
MACHINE_CONFIG_END

ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66_eeprom.bin", 0x000, 0x200, CRC(a423a969) SHA1(4c68654c81e70367209b9f6c712564aae89a3122) )
ROM_END

ROM_START( 48in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 48in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver309",   0x000000, 0x080000, CRC(27023186) SHA1(a2b3770c4b03d6026c6a0ff2e62ab17c3b359b12) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "48_flash.u19", 0x000000, 0x400000, CRC(a975db44) SHA1(5be6520b2ba7728e9e2de3c62ae7c3b88b25172a) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48_93c66.u32", 0x000, 0x200, CRC(cec06912) SHA1(2bc2e45602c5b1e8a3e031dd384e9f16be4e2ddb) )
ROM_END


ROM_START( 48in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "ver302.u2",    0x000000, 0x080000, CRC(5ea25870) SHA1(66edc59a3d355bc3462e98d2062ada721c371af6) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "48in1_93c66_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END


ROM_START( 60in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "hph_ver300.u8",   0x000000, 0x080000, CRC(6fba84c4) SHA1(28881e51227e94a80c8449d9c00a1a675f008d64) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x400000, "data", 0 )
	ROM_LOAD( "flash.u19", 0x000000, 0x400000, CRC(0cfed2a0) SHA1(9aac23f5267af56255e6f8aefade9f00bc106325) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "60in1_eeprom.u32", 0x000, 0x200, CRC(54af5973) SHA1(30aca7790458f4be906f7fa7c74206e16d9fc36f) )
ROM_END

ROM_START( 4in1a )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "plz-v014_ver300.bin", 0x000000, 0x080000, CRC(775f101d) SHA1(8a299a67b487518ba2e2cb5334347b93f8640190) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // confirmed same flash rom as 39 in 1

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "4in1_eeprom.bin", 0x000, 0x200, CRC(df1724f7) SHA1(07814aee3622f4bb8bada938f2a93fae791d6e31) )
ROM_END

ROM_START( 4in1b )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "pzv001-4.bin", 0x000000, 0x080000, CRC(7679a95f) SHA1(56c20fa7d086560b76477b42208cb43d42adba41) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "93c66-4.bin", 0x000, 0x200, CRC(84d1c26a) SHA1(de823adddf949bf77d8478762720fe0b56fba8ea) )
ROM_END

// 19-in-1 is visibly different hardware, extent of differences unknown due to lack of quality pictures/scans
// also, there is a bootleg of the 19-in-1 which may have less or different protection
ROM_START( 19in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "19in1.u8",    0x000000, 0x080000, CRC(87b0506c) SHA1(c43ae4b403864a28e56370685572fa02e7572e66) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
	ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) ) // assuming same flash rom

	// EEPROM - contains security data
	ROM_REGION16_BE( 0x200, "eeprom", 0 )
	ROM_LOAD16_WORD_SWAP( "19in1_eeprom.bin", 0x000, 0x200, NO_DUMP )
ROM_END

GAME(2004, 4in1a,  39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "4 in 1 MAME bootleg (set 1, ver 3.00)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 4in1b,  39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "4 in 1 MAME bootleg (set 2)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 19in1,  39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "19 in 1 MAME bootleg", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 39in1,  0,     39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "39 in 1 MAME bootleg", MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1,  39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "48 in 1 MAME bootleg (set 1, ver 3.09)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1b, 39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "48 in 1 MAME bootleg (set 2, ver 3.09, alt flash)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 48in1a, 39in1, 39in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "48 in 1 MAME bootleg (set 3, ver 3.02)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
GAME(2004, 60in1,  39in1, 60in1, 39in1, _39in1_state, 39in1, ROT270, "bootleg", "60 in 1 MAME bootleg (ver 3.00)", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_SOUND)
