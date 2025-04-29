// license:BSD-3-Clause
// copyright-holders:R. Belmont, Miodrag Milanovic
/***************************************************************************

  ISA 8/16 bit Creative Labs Sound Blaster Sound Card

  TODO:
  - DSP type is a MCS-51 family, it has an internal ROM that needs decapping;
  - implement jumpers DIP-SWs;

***************************************************************************/

#include "emu.h"
#include "sblaster.h"

#include "machine/pic8259.h"
#include "sound/spkrdev.h"
#include "sound/ymopl.h"

#include "speaker.h"

#define SIXTEENBIT  0x01
#define STEREO      0x02
#define SIGNED      0x04
#define ADPCM2      0x08
#define ADPCM3      0x10
#define ADPCM4      0x20

#define IRQ_DMA8    0x01
#define IRQ_DMA16   0x02
#define IRQ_MPU     0x04
#define IRQ_ALL     0xff


/*
  adlib (YM3812/OPL2 chip), part of many many soundcards (soundblaster)
  soundblaster: YM3812 also accessible at 0x228/9 (address jumperable)
  soundblaster pro version 1: 2 YM3812 chips
   at 0x388 both accessed,
   at 0x220/1 left?, 0x222/3 right? (jumperable)
  soundblaster pro version 2: 1 OPL3 chip

  pro audio spectrum +: 2 OPL2
  pro audio spectrum 16: 1 OPL3

  2 x saa1099 chips
    inherited from game blaster
    also on sound blaster 1.0
    option on sound blaster 1.5

  jumperable? normally 0x220
*/
#define ym3812_StdClock XTAL(3'579'545)
#define ymf262_StdClock XTAL(14'318'181)

static const int m_cmd_fifo_length[256] =
{
/*   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F        */
	-1, -1, -1, -1,  1,  3, -1, -1, -1, -1, -1, -1, -1, -1,  2,  1, /* 0x */
		2, -1, -1, -1,  3, -1,  3,  3, -1, -1, -1,  -1,  1, -1, -1,  1, /* 1x */
	-1, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 2x */
		1,  1, -1, -1,  1,  1,  1,  1,  1,  -1, -1, -1, -1, -1, -1, -1, /* 3x */
		2,  3,  3, -1, -1, -1, -1, -1,  3, -1, -1,  -1, -1, -1, -1, -1, /* 4x */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 5x */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 6x */
	-1, -1, -1, -1,  3,  3,  3,  3, -1, -1, -1, -1, -1,  1, -1,  1, /* 7x */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 8x */
	1,  1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 9x */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* Ax */
		4, -1,  4, -1,  4, -1,  4, -1,  4, -1, -1,  -1, -1, -1,  4, -1, /* Bx */
		4, -1,  4, -1,  4, -1,  4, -1,  4, -1, -1,  -1, -1, -1,  4, -1, /* Cx */
		1,  1, -1,  1,  1,  1,  1, -1,  1,  1,  1,  -1, -1, -1, -1, -1, /* Dx */
		2,  1,  2,  1,  2, -1, -1, -1,  1, -1, -1,  -1, -1, -1, -1, -1, /* Ex */
	-1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1  /* Fx */
};

static const int protection_magic[4] = { 0x96, 0xa5, 0x69, 0x5a };

uint8_t sb8_device::ym3812_16_r(offs_t offset)
{
	uint8_t retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = m_ym3812->status_r(); break;
	}
	return retVal;
}

void sb8_device::ym3812_16_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0 : m_ym3812->address_w(data); break;
		case 1 : m_ym3812->data_w(data); break;
	}
}

uint8_t isa8_sblaster1_0_device::saa1099_16_r(offs_t offset)
{
	return 0xff;
}

void isa8_sblaster1_0_device::saa1099_1_16_w(offs_t offset, uint8_t data)
{
	m_saa1099_1->write(offset, data);
}

void isa8_sblaster1_0_device::saa1099_2_16_w(offs_t offset, uint8_t data)
{
	m_saa1099_2->write(offset, data);
}

void sb_device::queue(uint8_t data)
{
	if (m_dsp.fifo_ptr < 15)
	{
		m_dsp.fifo[m_dsp.fifo_ptr] = data;

		m_dsp.fifo_ptr++;
	}
	else
	{
		// FIFO gets to la-la-land
		//logerror("FIFO?\n");
	}
}

void sb_device::queue_r(uint8_t data)
{
	m_dsp.rbuf_status |= 0x80;

	if (m_dsp.fifo_r_ptr < 52)
	{
		m_dsp.fifo_r[m_dsp.fifo_r_ptr] = data;

		m_dsp.fifo_r_ptr++;
	}
	else
	{
		// FIFO gets to la-la-land
		//logerror("FIFO?\n");
	}
}

uint8_t sb_device::dequeue_r()
{
	uint8_t data = m_dsp.fifo_r[0];

	if (m_dsp.fifo_r_ptr > 0)
	{
		for (int i = 0; i < 51; i++)
			m_dsp.fifo_r[i] = m_dsp.fifo_r[i + 1];

		m_dsp.fifo_r[51] = 0;

		m_dsp.fifo_r_ptr--;
	}

	if(m_dsp.fifo_r_ptr == 0)
		m_dsp.rbuf_status &= ~0x80;

	return data;
}


uint8_t sb_device::dsp_reset_r(offs_t offset)
{
//    printf("read DSP reset @ %x\n", offset);
	if(offset)
		return 0xff;
	logerror("Soundblaster DSP Reset port undocumented read\n");
	return 0xff;
}

void sb_device::dsp_reset_w(offs_t offset, uint8_t data)
{
//    printf("%02x to DSP reset @ %x\n", data, offset);
	if(offset)
		return;

	// a reset while in UART MIDI mode simply restores the previous
	// operating state (page 5-3 of the Creative manual).
	if (!m_uart_midi)
	{
		if(data == 0 && m_dsp.reset_latch == 1)
		{
			// reset routine
			m_dsp.fifo_ptr = 0;
			m_dsp.fifo_r_ptr = 0;
			for(int i=0;i < 15; i++)
			{
				m_dsp.fifo[i] = 0;
				m_dsp.fifo_r[i] = 0;
			}
			queue_r(0xaa); // reset OK ID
		}

		m_dsp.reset_latch = data;
		drq_w(0);
		m_dsp.dma_autoinit = 0;
		irq_w(0, IRQ_ALL);
		m_timer->adjust(attotime::never, 0);
		m_dsp.d_rptr = 0;
		m_dsp.d_wptr = 0;
		m_dsp.dma_throttled = false;
		m_dsp.dma_timer_started = false;
	}

	m_onebyte_midi = false;
	m_uart_midi = false;
	m_uart_irq = false;
	m_mpu_midi = false;
	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
	m_recv_read = m_recv_write = 0;
	m_rx_waiting = m_tx_waiting = 0;

	//printf("%02x\n",data);
}

uint8_t sb_device::dsp_data_r(offs_t offset)
{
//    printf("read DSP data @ %x\n", offset);
	if(offset)
		return 0xff;

	if (m_uart_midi)
	{
		uint8_t rv = m_recvring[m_recv_read++];
		if (m_recv_read >= MIDI_RING_SIZE)
		{
			m_recv_read = 0;
		}

		if (m_rx_waiting)
		{
			m_rx_waiting--;
		}

		return rv;
	}

	return dequeue_r();
}

void sb_device::dsp_data_w(offs_t offset, uint8_t data)
{
//    printf("%02x to DSP data @ %x\n", data, offset);
	if(offset)
		return;
	logerror("Soundblaster DSP data port undocumented write\n");
}

uint8_t sb_device::dsp_rbuf_status_r(offs_t offset)
{
//    printf("read Rbufstat @ %x\n", offset);

	if(offset)
	{
		if(m_dsp.version > 0x0400)
			irq_w(0, IRQ_DMA16);
		return 0xff;
	}

//    printf("Clear IRQ5\n");
	irq_w(0, IRQ_DMA8);   // reading this port ACKs the card's IRQ, 8-bit dma only?

	// in either SB-MIDI mode, bit 7 indicates if a character is available
	// to read.
	if (m_uart_midi || m_onebyte_midi)
	{
		if (m_rx_waiting)
		{
			return 0x80;
		}

		return 0x00;
	}

	return m_dsp.rbuf_status;
}

uint8_t sb_device::dsp_wbuf_status_r(offs_t offset)
{
//    printf("read Wbufstat @ %x\n", offset);

	if(offset)
		return 0xff;

	// in either SB-MIDI mode, bit 7 indicates if there's space to write.
	// set = buffer full
	if (m_uart_midi || m_onebyte_midi)
	{
		if (m_tx_waiting >= MIDI_RING_SIZE)
		{
			return 0x80;
		}

		return 0x00;
	}

	return m_dsp.wbuf_status;
}

void sb_device::dsp_rbuf_status_w(offs_t offset, uint8_t data)
{
//    printf("%02x to Rbufstat @ %x\n", data, offset);
	if(offset)
		return;

	logerror("Soundblaster DSP Read Buffer status undocumented write\n");
}

void sb_device::process_fifo(uint8_t cmd)
{
	if (m_cmd_fifo_length[cmd] == -1)
	{
		logerror("SB: unemulated or undefined fifo command %02x\n",cmd);
		m_dsp.fifo_ptr = 0;
	}
	else if(m_dsp.fifo_ptr == m_cmd_fifo_length[cmd])
	{
		/* get FIFO params */
//        printf("SB FIFO command: %02x\n", cmd);
		switch(cmd)
		{
			case 0x10:  // Direct DAC
				m_ldac->write(m_dsp.fifo[1] << 8);
				m_rdac->write(m_dsp.fifo[1] << 8);
				break;

			case 0x14:  // 8-bit DMA, no autoinit
			case 0x91:  // 8-bit DMA, no autoinit, high speed. XXX only on DSP 3.xx
				m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
//                printf("Start DMA (not autoinit, size = %x)\n", m_dsp.dma_length);
				m_dsp.dma_transferred = 0;
				m_dsp.dma_autoinit = 0;
				m_dsp.dma_timer_started = false;
				m_dsp.dma_throttled = false;
				drq_w(1);
				m_dsp.flags = 0;
				break;

			case 0x17:  // 2-bit ADPCM w/new reference
				m_dsp.adpcm_new_ref = true;
				m_dsp.adpcm_step = 0;
				[[fallthrough]];
			case 0x16:  // 2-bit ADPCM
				m_dsp.adpcm_count = 0;
				m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
				m_dsp.dma_transferred = 0;
				m_dsp.dma_autoinit = 0;
				m_dsp.dma_timer_started = false;
				m_dsp.dma_throttled = false;
				drq_w(1);
				m_dsp.flags = ADPCM2;
				break;

			case 0x1c:  // 8-bit DMA with autoinit
			case 0x90:  // 8-bit DMA with autoinit, high speed. XXX only on DSP 3.xx
				//              printf("Start DMA (autoinit, size = %x)\n", m_dsp.dma_length);
				m_dsp.dma_transferred = 0;
				m_dsp.dma_autoinit = 1;
				m_dsp.dma_timer_started = false;
				m_dsp.dma_throttled = false;
				drq_w(1);
				m_dsp.flags = 0;
				break;

			case 0x24: // 8-bit ADC DMA
				m_dsp.adc_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
				//                printf("Start DMA (not autoinit, size = %x)\n", m_dsp.adc_length);
				m_dsp.adc_transferred = 0;
				m_dsp.dma_autoinit = 0;
				drq_w(1);
				logerror("SB: ADC capture unimplemented\n");
				break;

			case 0x34:
				m_uart_midi = true;
				m_uart_irq = false;
				break;

			case 0x35:
				m_uart_midi = true;
				m_uart_irq = true;
				break;

			case 0x36:
			case 0x37:  // Enter UART mode
				printf("timestamp MIDI mode not supported, contact MESSDEV!\n");
				break;

			case 0x38:  // single-byte MIDI send
				m_onebyte_midi = true;
				break;

			case 0x40:  // set time constant
				m_dsp.frequency = (1000000 / (256 - m_dsp.fifo[1]));
				//printf("Set time constant: %02x -> %d\n", m_dsp.fifo[1], m_dsp.frequency);
				break;

			case 0x48:  // set DMA block size (for auto-init)
				m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
				break;

			case 0x75:  // 4-bit ADPCM w/new reference
				m_dsp.adpcm_new_ref = true;
				m_dsp.adpcm_step = 0;
				[[fallthrough]];
			case 0x74:  // 4-bit ADPCM
				m_dsp.adpcm_count = 0;
				m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
				m_dsp.dma_transferred = 0;
				m_dsp.dma_autoinit = 0;
				m_dsp.dma_timer_started = false;
				m_dsp.dma_throttled = false;
				drq_w(1);
				m_dsp.flags = ADPCM4;
				break;

			case 0x77:  // 2.6-bit ADPCM w/new reference
				m_dsp.adpcm_new_ref = true;
				m_dsp.adpcm_step = 0;
				[[fallthrough]];
			case 0x76:  // 2.6-bit ADPCM
				m_dsp.adpcm_count = 0;
				m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
				m_dsp.dma_transferred = 0;
				m_dsp.dma_autoinit = 0;
				m_dsp.dma_timer_started = false;
				m_dsp.dma_throttled = false;
				drq_w(1);
				m_dsp.flags = ADPCM3;
				break;

			case 0xd0:  // halt 8-bit DMA
				m_timer->adjust(attotime::never, 0);
				drq_w(0);   // drop DRQ
				m_dsp.dma_throttled = false;
				m_dsp.dma_timer_started = false;
				break;

			case 0xd1: // speaker on
				// ...
				m_dsp.speaker_on = 1;
				break;

			case 0xd3: // speaker off
				// ...
				m_dsp.speaker_on = 0;
				break;

			case 0xd8: // speaker status
				queue_r(m_dsp.speaker_on ? 0xff : 0x00);
				break;

			case 0xe0: // get DSP identification
				queue_r(m_dsp.fifo[1] ^ 0xff);
				break;

			case 0xe1: // get DSP version
				queue_r(m_dsp.version >> 8);
				queue_r(m_dsp.version & 0xff);
				break;

			case 0xe2: // DSP protection
				m_dsp.prot_value += protection_magic[m_dsp.prot_count++] ^ m_dsp.fifo[1];
				m_dsp.prot_count &= 3;
				m_dsp.adc_transferred = 0;
				m_dsp.adc_length = 1;
				m_dsp.wbuf_status = 0x80;
				m_dsp.dma_no_irq = true;
				m_dack_out = (uint8_t)(m_dsp.prot_value & 0xff);
				drq_w(1);
				break;

			case 0xe4: // write test register
				m_dsp.test_reg = m_dsp.fifo[1];
				break;

			case 0xe8: // read test register
				queue_r(m_dsp.test_reg);
				break;

			case 0xf2: // send PIC irq
				irq_w(1, IRQ_DMA8);
				break;

			case 0xf8: // ???
				logerror("SB: Unknown command write 0xf8\n");
				queue_r(0);
				break;
			default:
				if(m_dsp.version >= 0x0201) // SB 2.0
				{
					switch(cmd)
					{
						case 0x1f:  // 2-bit autoinit ADPCM w/new reference
							m_dsp.adpcm_new_ref = true;
							m_dsp.adpcm_step = 0;
							m_dsp.adpcm_count = 0;
							m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
							m_dsp.dma_transferred = 0;
							m_dsp.dma_autoinit = 1;
							m_dsp.dma_timer_started = false;
							m_dsp.dma_throttled = false;
							drq_w(1);
							m_dsp.flags = ADPCM2;
							break;
						case 0x7d:  // 4-bit autoinit ADPCM w/new reference
							m_dsp.adpcm_new_ref = true;
							m_dsp.adpcm_step = 0;
							m_dsp.adpcm_count = 0;
							m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
							m_dsp.dma_transferred = 0;
							m_dsp.dma_autoinit = 1;
							m_dsp.dma_timer_started = false;
							m_dsp.dma_throttled = false;
							drq_w(1);
							m_dsp.flags = ADPCM4;
							break;
						case 0x7f:  // 2.6-bit autoinit ADPCM w/new reference
							m_dsp.adpcm_new_ref = true;
							m_dsp.adpcm_step = 0;
							m_dsp.adpcm_count = 0;
							m_dsp.dma_length = (m_dsp.fifo[1] + (m_dsp.fifo[2]<<8)) + 1;
							m_dsp.dma_transferred = 0;
							m_dsp.dma_autoinit = 1;
							m_dsp.dma_timer_started = false;
							m_dsp.dma_throttled = false;
							drq_w(1);
							m_dsp.flags = ADPCM3;
							break;
						case 0xda: // stop 8-bit autoinit
							m_dsp.dma_autoinit = 0;
							break;
					}
				}
				if(m_dsp.version >= 0x0301) // SB Pro 2
				{
					switch(cmd)
					{
						case 0xe3: // copyright notice, check if in pro 2
							const char* copyright = "NOT COPYRIGHT (C) CREATIVE TECHNOLOGY LTD, 1992.";
							int j = strlen(copyright);
							for(int k = 4; k <= j; k++)
								queue_r(copyright[k]);
							break;
					}
				}
				if(m_dsp.version >= 0x0400) // SB16
				{
					int mode;
					switch(cmd)
					{
						case 0x0f:  // read asp reg
							queue_r(0);
							[[fallthrough]];
						case 0x0e:  // write asp reg
						case 0x02:  // get asp version
						case 0x04:  // set asp mode register
						case 0x05:  // set asp codec param
							logerror("SB16: unimplemented ASP command\n");
							break;
						case 0x41: // set output sample rate
							m_dsp.frequency = m_dsp.fifo[2] + (m_dsp.fifo[1] << 8);
							break;
						case 0x42: // set input sample rate
							m_dsp.adc_freq = m_dsp.fifo[2] + (m_dsp.fifo[1] << 8);
							break;
						case 0xd4: // continue 8-bit dma
							drq_w(1);
							break;
						case 0xd5: // pause 16-bit dma
							m_timer->adjust(attotime::never, 0);
							drq16_w(0);   // drop DRQ
							m_dsp.dma_throttled = false;
							m_dsp.dma_timer_started = false;
							break;
						case 0xd6: // continue 16-bit dma
							logerror("SB: 16-bit dma resume\n");
							break;
						case 0xd9: // stop 16-bit autoinit
							m_dsp.dma_autoinit = 0;
							break;
						case 0xb0:
						case 0xb2:
						case 0xb4:
						case 0xb6:
						case 0xc0:
						case 0xc2:
						case 0xc4:
						case 0xc6:
							mode = m_dsp.fifo[1];
							m_dsp.flags = 0;
							m_dsp.dma_length = (m_dsp.fifo[2] + (m_dsp.fifo[3]<<8)) + 1;
							if(cmd & 0x04)
								m_dsp.dma_autoinit = 1;
							if(mode & 0x10)
								m_dsp.flags |= SIGNED;
							if(mode & 0x20)
								m_dsp.flags |= STEREO;
							if((cmd & 0xf0) == 0xb0)
							{
								m_dsp.flags |= SIXTEENBIT;
								m_dsp.dma_length <<= 1;
								drq16_w(1);
							}
							else
								drq_w(1);
							m_dsp.dma_transferred = 0;
							m_dsp.dma_timer_started = false;
							m_dsp.dma_throttled = false;
							break;
						case 0xb8:
						case 0xbe:
						case 0xc8:
						case 0xce:
							mode = m_dsp.fifo[1];
							m_dsp.adc_length = (m_dsp.fifo[2] + (m_dsp.fifo[3]<<8)) + 1;
							m_dsp.adc_transferred = 0;
							if(cmd & 0x04)
								m_dsp.dma_autoinit = 1;
							if(mode & 0x20)
								m_dsp.adc_length <<= 1;
							if((cmd & 0xf0) == 0xb0)
							{
								m_dsp.adc_length <<= 1;
								drq16_w(1);
							}
							else
								drq_w(1);
							logerror("SB: ADC capture unimplemented\n");
							break;
						case 0xf3: // send PIC irq
							irq_w(1, IRQ_DMA16);
							break;
						case 0xfc:
							queue_r((((m_dsp.flags & SIXTEENBIT) && m_dsp.dma_autoinit) << 4) | ((!(m_dsp.flags & SIXTEENBIT) && m_dsp.dma_autoinit) << 2));
							break;
					}
				}
		}
		m_dsp.fifo_ptr = 0;
	}
}

void sb_device::dsp_cmd_w(offs_t offset, uint8_t data)
{
//  printf("%02x to DSP command @ %x\n", data, offset);

	if(offset)
		return;

	if (m_uart_midi || m_onebyte_midi)
	{
		xmit_char(data);
		m_onebyte_midi = false; // clear onebyte (if this is uart, that's harmless)
		return;
	}

	queue(data);

	process_fifo(m_dsp.fifo[0]);
}

void sb_device::adpcm_decode(uint8_t sample, int size)
{
	const uint8_t adpcm_2_table[] =  {0, 1, 1, 3, 2, 6, 4, 12, 8, 24, 16, 48};
	const uint8_t step_2_table[] =   {0, 2, 0, 4, 2, 6, 4,  8, 6, 10,  8, 10};

	const uint8_t adpcm_3_table[] =  {0,  1,  2,  3,  1,  3,  5,  7,
									2,  6, 10, 14,  4, 12, 20, 28,
									8, 24, 40, 56};
	const uint8_t step_3_table[] =   {0,  0,  0,  4,  0,  4,  4,  8,
									4,  8,  8, 12,  8, 12, 12, 16,
									12, 16, 16, 16};

	const uint8_t adpcm_4_table[] =  {0,  1,  2,  3,  4,  5,  6,  7,
									1,  3,  5,  7,  9, 11, 13, 15,
									2,  6, 10, 14, 18, 22, 26, 30,
									4, 12, 20, 28, 36, 44, 52, 60};
	const uint8_t step_4_table[]  =  {0,  0,  0,  0,  0,  8,  8,  8,
									0,  8,  8,  8,  8, 16, 16, 16,
									8, 16, 16, 16, 16, 24, 24, 24,
									16, 24, 24, 24, 24, 24, 24, 24};

	int16_t dec_sample = m_dsp.adpcm_ref;
	uint8_t index;
	switch(size)
	{
		case 2:
			index = (sample & 1) | m_dsp.adpcm_step;
			dec_sample += ((sample & 2)?-1:1) * adpcm_2_table[index];
			m_dsp.adpcm_step = step_2_table[index];
			break;
		case 3:
			index = (sample & 3) | m_dsp.adpcm_step;
			dec_sample += ((sample & 4)?-1:1) * adpcm_3_table[index];
			m_dsp.adpcm_step = step_3_table[index];
			break;
		case 4:
			index = (sample & 7) | m_dsp.adpcm_step;
			dec_sample += ((sample & 8)?-1:1) * adpcm_4_table[index];
			m_dsp.adpcm_step = step_4_table[index];
			break;
	}

	if(dec_sample > 255)
		dec_sample = 255;
	else if(dec_sample < 0)
		dec_sample = 0;
	m_dsp.adpcm_ref = dec_sample;
	m_ldac->write(m_dsp.adpcm_ref << 8);
	m_rdac->write(m_dsp.adpcm_ref << 8);
}

uint8_t sb16_device::mpu401_r(offs_t offset)
{
	uint8_t res;

	irq_w(0, IRQ_MPU);
	if(offset == 0) // data
	{
		res = m_recvring[m_recv_read++];
		if (m_recv_read >= MIDI_RING_SIZE)
		{
			m_recv_read = 0;
		}

		if (m_rx_waiting)
		{
			m_rx_waiting--;
		}
	}
	else // status
	{
		res = 0;
		if (m_tx_waiting >= MIDI_RING_SIZE)
		{
			res |= 0x40;   // tx full
		}
		if (m_rx_waiting == 0)
		{
			res |= 0x80;    // rx empty
		}
	}

	return res;
}

void sb16_device::mpu401_w(offs_t offset, uint8_t data)
{
	if(offset == 0) // data
	{
		logerror("SB MPU401:%02x %02x\n",offset,data);
		if (m_mpu_midi)
		{
			xmit_char(data);
		}
	}
	else // command
	{
		logerror("SB MPU401:%02x %02x\n",offset,data);

		switch(data)
		{
			case 0x3f: // enter MPU-401 UART mode
				irq_w(1, IRQ_MPU);
				m_recv_read = m_recv_write = 0;
				m_xmit_read = m_xmit_write = m_tx_waiting = 0;
				m_recvring[m_recv_write++] = 0xfe;
				m_rx_waiting = 1;
				m_mpu_midi = true;
				break;

			case 0xff: // reset
				irq_w(1, IRQ_MPU);
				m_recv_read = m_recv_write = 0;
				m_recvring[m_recv_write++] = 0xfe;
				m_rx_waiting = 1;
				m_mpu_midi = false;
				break;
		}
	}

}

void sb16_device::mixer_set()
{
	ymf262_device *ymf = subdevice<ymf262_device>("ymf262");
	float lmain = m_mixer.main_vol[0]/248.0;
	float rmain = m_mixer.main_vol[1]/248.0;
	m_ldac->set_output_gain(ALL_OUTPUTS, lmain*(m_mixer.dac_vol[0]/248.0f));
	m_rdac->set_output_gain(ALL_OUTPUTS, rmain*(m_mixer.dac_vol[1]/248.0f));
	ymf->set_output_gain(0, lmain*(m_mixer.fm_vol[0]/248.0f));
	ymf->set_output_gain(1, rmain*(m_mixer.fm_vol[1]/248.0f));
	ymf->set_output_gain(2, lmain*(m_mixer.fm_vol[0]/248.0f));
	ymf->set_output_gain(3, rmain*(m_mixer.fm_vol[1]/248.0f));
}

void sb16_device::mixer_reset()
{
	m_mixer.status = 0x80;
	m_mixer.main_vol[0] = 0xc0;
	m_mixer.main_vol[1] = 0xc0;
	m_mixer.dac_vol[0] = 0xc0;
	m_mixer.dac_vol[1] = 0xc0;
	m_mixer.fm_vol[0] = 0xc0;
	m_mixer.fm_vol[1] = 0xc0;
	m_mixer.cd_vol[0] = 0x00;
	m_mixer.cd_vol[1] = 0x00;
	m_mixer.line_vol[0] = 0x00;
	m_mixer.line_vol[1] = 0x00;
	m_mixer.mic_vol = 0x00;
	m_mixer.pc_speaker_vol = 0x00;
	m_mixer.output_ctl = 0x1f;
	m_mixer.input_ctl[0] = 0x15;
	m_mixer.input_ctl[1] = 0x0b;
	m_mixer.input_gain[0] = 0x00;
	m_mixer.input_gain[1] = 0x00;
	m_mixer.output_gain[0] = 0x00;
	m_mixer.output_gain[1] = 0x00;
	m_mixer.agc = 0x00;
	m_mixer.treble[0] = 0x80;
	m_mixer.treble[1] = 0x80;
	m_mixer.bass[0] = 0x80;
	m_mixer.bass[1] = 0x80;
	mixer_set();
}

uint8_t sb16_device::mixer_r(offs_t offset)
{
	if(offset == 0)
		return m_mixer.status;
	return m_mixer.data;
}

void sb16_device::mixer_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		switch(data)
		{
			case 0x00:
				mixer_reset();
				return;
			case 0x01:
				m_mixer.data = m_mixer.status;
				break;
			case 0x04:
				m_mixer.data = (m_mixer.dac_vol[0] & 0xf0) | (m_mixer.dac_vol[1] >> 4);
				break;
			case 0x0a:
				m_mixer.data = m_mixer.mic_vol >> 5;
				break;
			case 0x22:
				m_mixer.data = (m_mixer.main_vol[0] & 0xf0) | (m_mixer.main_vol[1] >> 4);
				break;
			case 0x26:
				m_mixer.data = (m_mixer.fm_vol[0] & 0xf0) | (m_mixer.fm_vol[1] >> 4);
				break;
			case 0x28:
				m_mixer.data = (m_mixer.cd_vol[0] & 0xf0) | (m_mixer.cd_vol[1] >> 4);
				break;
			case 0x2e:
				m_mixer.data = (m_mixer.line_vol[0] & 0xf0) | (m_mixer.line_vol[1] >> 4);
				break;
			case 0x30:
			case 0x31:
				m_mixer.data = m_mixer.main_vol[data & 1];
				break;
			case 0x32:
			case 0x33:
				m_mixer.data = m_mixer.dac_vol[data & 1];
				break;
			case 0x34:
			case 0x35:
				m_mixer.data = m_mixer.fm_vol[data & 1];
				break;
			case 0x36:
			case 0x37:
				m_mixer.data = m_mixer.cd_vol[data & 1];
				break;
			case 0x38:
			case 0x39:
				m_mixer.data = m_mixer.line_vol[data & 1];
				break;
			case 0x3a:
				m_mixer.data = m_mixer.mic_vol;
				break;
			case 0x3b:
				m_mixer.data = m_mixer.pc_speaker_vol;
				break;
			case 0x3c:
				m_mixer.data = m_mixer.output_ctl;
				break;
			case 0x3d:
			case 0x3e:
				m_mixer.data = m_mixer.input_ctl[(data + 1) & 1];
				break;
			case 0x3f:
			case 0x40:
				m_mixer.data = m_mixer.input_gain[(data + 1) & 1];
				break;
			case 0x41:
			case 0x42:
				m_mixer.data = m_mixer.output_gain[(data + 1) & 1];
				break;
			case 0x43:
				m_mixer.data = m_mixer.agc;
				break;
			case 0x44:
			case 0x45:
				m_mixer.data = m_mixer.treble[data & 1];
				break;
			case 0x46:
			case 0x47:
				m_mixer.data = m_mixer.bass[data & 1];
				break;
			case 0x80:
				m_mixer.data = 0x12; // irq5
				break;
			case 0x81:
				m_mixer.data = 0x22;  // dma1&5
				break;
			case 0x82:
				m_mixer.data = m_dsp.irq_active | 0x20;
				break;
			default:
				logerror("SB: Unimplemented mixer index %02x\n", data);
				m_mixer.status = data | 0x80;
				m_mixer.data = 0x0a;
				return;
		}
		m_mixer.status = data;
		return;
	}
	switch(m_mixer.status)
	{
		case 0x04:
			m_mixer.dac_vol[0] = (data & 0xf0) | 8;
			m_mixer.dac_vol[1] = (data << 4) | 8;
			break;
		case 0x0a:
			m_mixer.mic_vol = (data << 5) | 0x18;
			break;
		case 0x22:
			m_mixer.main_vol[0] = (data & 0xf0) | 8;
			m_mixer.main_vol[1] = (data << 4) | 8;
			break;
		case 0x26:
			m_mixer.fm_vol[0] = (data & 0xf0) | 8;
			m_mixer.fm_vol[1] = (data << 4) | 8;
			break;
		case 0x28:
			m_mixer.cd_vol[0] = (data & 0xf0) | 8;
			m_mixer.cd_vol[1] = (data << 4) | 8;
			break;
		case 0x2e:
			m_mixer.line_vol[0] = (data & 0xf0) | 8;
			m_mixer.line_vol[1] = (data << 4) | 8;
			break;
		case 0x30:
		case 0x31:
			m_mixer.main_vol[m_mixer.status & 1] = data & 0xf8;
			break;
		case 0x32:
		case 0x33:
			m_mixer.dac_vol[m_mixer.status & 1] = data & 0xf8;
			break;
		case 0x34:
		case 0x35:
			m_mixer.fm_vol[m_mixer.status & 1] = data & 0xf8;
			break;
		case 0x36:
		case 0x37:
			m_mixer.cd_vol[m_mixer.status & 1] = data & 0xf8;
			break;
		case 0x38:
		case 0x39:
			m_mixer.line_vol[m_mixer.status & 1] = data & 0xf8;
			break;
		case 0x3a:
			m_mixer.mic_vol = data & 0xf8;
			break;
		case 0x3b:
			m_mixer.pc_speaker_vol = data & 0xc0;
			break;
		case 0x3c:
			m_mixer.output_ctl = data & 0x1f;
			break;
		case 0x3d:
		case 0x3e:
			m_mixer.input_ctl[(m_mixer.status + 1) & 1] = data & 0x7f;
			break;
		case 0x3f:
		case 0x40:
			m_mixer.input_gain[(m_mixer.status + 1) & 1] = data & 0xc0;
			break;
		case 0x41:
		case 0x42:
			m_mixer.output_gain[(m_mixer.status + 1) & 1] = data & 0xc0;
			break;
		case 0x43:
			m_mixer.agc = data & 1;
			break;
		case 0x44:
		case 0x45:
			m_mixer.treble[m_mixer.status & 1] = data & 0xf0;
			break;
		case 0x46:
		case 0x47:
			m_mixer.bass[m_mixer.status & 1] = data & 0xf0;
			break;
		case 0x80:
		case 0x81:
			// don't support these yet
			break;
		default:
			return;
	}
	m_mixer.data = data;
	mixer_set();
	return;
}

void sb8_device::drq_w(int state)
{
	switch(m_config->read() & 0x0c)
	{
		case 0:
		default:
			m_isa->drq1_w(state);
			break;
		case 4:
			m_isa->drq3_w(state);
			break;
	}
}

void sb8_device::irq_w(int state, int source)
{
	switch(m_config->read() & 0x03)
	{
		case 0:
		default:
			m_isa->irq5_w(state);
			break;
		case 1:
			m_isa->irq7_w(state);
			break;
	}
}

// TODO: sb16 irq/drq is software set
void sb16_device::drq_w(int state)
{
	switch(m_config->read() & 0x0c)
	{
		case 0:
		default:
			m_isa->drq1_w(state);
			break;
		case 4:
			m_isa->drq3_w(state);
			break;
	}
}

void sb16_device::irq_w(int state, int source)
{
	if(state)
		m_dsp.irq_active |= source;
	else
		m_dsp.irq_active &= ~source;

	switch(m_config->read() & 0x03)
	{
		case 0:
		default:
			m_isa->irq5_w(m_dsp.irq_active != 0);
			break;
		case 1:
			m_isa->irq7_w(m_dsp.irq_active != 0);
			break;
	}
}

void sb16_device::drq16_w(int state)
{
	switch(m_config->read() & 0xc0)
	{
		case 0:
		default:
			m_isa->drq5_w(state);
			break;
		case 0x40:
			m_isa->drq6_w(state);
			break;
		case 0x80:
			m_isa->drq7_w(state);
			break;
	}
}

static INPUT_PORTS_START( sb8 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x03, 0x00, "Default IRQ")
	PORT_CONFSETTING( 0x00, "IRQ5")
	PORT_CONFSETTING( 0x01, "IRQ7")
	PORT_CONFNAME(0x0c, 0x00, "Default DMA")
	PORT_CONFSETTING( 0x00, "DMA1")
	PORT_CONFSETTING( 0x04, "DMA3")
	//PORT_CONFNAME(0x30, 0x00, "Default Port Base")
	//PORT_CONFSETTING( 0x00, "220")
	//PORT_CONFSETTING( 0x10, "240")
	//PORT_CONFSETTING( 0x20, "260")
INPUT_PORTS_END

static INPUT_PORTS_START( sb16 )
	PORT_INCLUDE(sb8)
	PORT_MODIFY("CONFIG")
	PORT_CONFNAME(0xc0, 0x00, "Default DMA16")
	PORT_CONFSETTING( 0x00, "DMA5")
	PORT_CONFSETTING( 0x40, "DMA6")
	PORT_CONFSETTING( 0x80, "DMA7")
INPUT_PORTS_END

ioport_constructor sb8_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sb8);
}

ioport_constructor sb16_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sb16);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_SOUND_BLASTER_1_0, isa8_sblaster1_0_device, "isa_sblaster1_0", "Sound Blaster 1.0")
DEFINE_DEVICE_TYPE(ISA8_SOUND_BLASTER_1_5, isa8_sblaster1_5_device, "isa_sblaster1_5", "Sound Blaster 1.5")
DEFINE_DEVICE_TYPE(ISA16_SOUND_BLASTER_16, isa16_sblaster16_device, "isa_sblaster_16", "Sound Blaster 16")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sb_device::common(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // unknown DAC
	DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC

	PC_JOY(config, m_joy);

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(sb_device::midi_rx_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

void isa8_sblaster1_0_device::device_add_mconfig(machine_config &config)
{
	common(config);

	YM3812(config, m_ym3812, ym3812_StdClock);
	m_ym3812->add_route(ALL_OUTPUTS, "speaker", 3.0, 0);
	m_ym3812->add_route(ALL_OUTPUTS, "speaker", 3.0, 1);

	SAA1099(config, m_saa1099_1, XTAL(14'318'181) / 2); // or CMS-301, from OSC pin in ISA bus
	m_saa1099_1->add_route(0, "speaker", 0.5, 0);
	m_saa1099_1->add_route(1, "speaker", 0.5, 1);

	SAA1099(config, m_saa1099_2, XTAL(14'318'181) / 2); // or CMS-301, from OSC pin in ISA bus
	m_saa1099_2->add_route(0, "speaker", 0.5, 0);
	m_saa1099_2->add_route(1, "speaker", 0.5, 1);
}

void isa8_sblaster1_5_device::device_add_mconfig(machine_config &config)
{
	common(config);

	YM3812(config, m_ym3812, ym3812_StdClock);
	m_ym3812->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	m_ym3812->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);
	/* no CM/S support (empty sockets) */
}

void isa16_sblaster16_device::device_add_mconfig(machine_config &config)
{
	common(config);

	ymf262_device &ymf262(YMF262(config, "ymf262", ymf262_StdClock));
	ymf262.add_route(0, "speaker", 1.0, 0);
	ymf262.add_route(1, "speaker", 1.0, 1);
	ymf262.add_route(2, "speaker", 1.0, 0);
	ymf262.add_route(3, "speaker", 1.0, 1);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

sb_device::sb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_ldac(*this, "ldac"),
	m_rdac(*this, "rdac"),
	m_joy(*this, "pc_joy"),
	m_mdout(*this, "mdout"),
	m_config(*this, "CONFIG"), m_dack_out(0), m_onebyte_midi(false), m_uart_midi(false), m_uart_irq(false), m_mpu_midi(false), m_rx_waiting(0), m_tx_waiting(0), m_xmit_read(0), m_xmit_write(0), m_recv_read(0), m_recv_write(0), m_tx_busy(false), m_timer(nullptr)
{
}

sb8_device::sb8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	sb_device(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_ym3812(*this, "ym3812")
{
}

sb16_device::sb16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	sb_device(mconfig, type, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  isa8_sblaster_device - constructor
//-------------------------------------------------

isa8_sblaster1_0_device::isa8_sblaster1_0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sb8_device(mconfig, ISA8_SOUND_BLASTER_1_0, tag, owner, clock),
	m_saa1099_1(*this, "saa1099.1"),
	m_saa1099_2(*this, "saa1099.2")
{
}

isa8_sblaster1_5_device::isa8_sblaster1_5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sb8_device(mconfig, ISA8_SOUND_BLASTER_1_5, tag, owner, clock)
{
}

isa16_sblaster16_device::isa16_sblaster16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	sb16_device(mconfig, ISA16_SOUND_BLASTER_16, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sb8_device::device_start()
{
	m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_isa->install_device(0x0226, 0x0227, read8sm_delegate(*this, FUNC(sb_device::dsp_reset_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_reset_w)));
	m_isa->install_device(0x022a, 0x022b, read8sm_delegate(*this, FUNC(sb_device::dsp_data_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_data_w)));
	m_isa->install_device(0x022c, 0x022d, read8sm_delegate(*this, FUNC(sb_device::dsp_wbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_cmd_w)));
	m_isa->install_device(0x022e, 0x022f, read8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_w)));
	if(m_dsp.version >= 0x0301)
	{
		ymf262_device &ymf262 = *subdevice<ymf262_device>("ymf262");

		m_isa->install_device(0x0388, 0x038b, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
		m_isa->install_device(0x0220, 0x0223, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
		m_isa->install_device(0x0228, 0x0229, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	}
	else
	{
		m_isa->install_device(0x0388, 0x0389, read8sm_delegate(*this, FUNC(sb8_device::ym3812_16_r)), write8sm_delegate(*this, FUNC(sb8_device::ym3812_16_w)));
		m_isa->install_device(0x0228, 0x0229, read8sm_delegate(*this, FUNC(sb8_device::ym3812_16_r)), write8sm_delegate(*this, FUNC(sb8_device::ym3812_16_w)));
	}

	sb_device::device_start();
}

void isa8_sblaster1_0_device::device_start()
{
	set_isa_device();
	// 1.0 always has the SAA1099s for CMS back-compatibility
	m_isa->install_device(0x0220, 0x0221, read8sm_delegate(*this, FUNC(isa8_sblaster1_0_device::saa1099_16_r)), write8sm_delegate(*this, FUNC(isa8_sblaster1_0_device::saa1099_1_16_w)));
	m_isa->install_device(0x0222, 0x0223, read8sm_delegate(*this, FUNC(isa8_sblaster1_0_device::saa1099_16_r)), write8sm_delegate(*this, FUNC(isa8_sblaster1_0_device::saa1099_2_16_w)));
	m_isa->set_dma_channel(1, this, false);
	m_dsp.version = 0x0105;
	sb8_device::device_start();
}

void isa8_sblaster1_5_device::device_start()
{
	set_isa_device();
	/* 1.5 makes CM/S support optional (empty sockets, but they work if the user populates them!) */
	m_isa->set_dma_channel(1, this, false);
	m_dsp.version = 0x0200;
	sb8_device::device_start();
}

void sb16_device::device_start()
{
	ymf262_device &ymf262 = *subdevice<ymf262_device>("ymf262");
	m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
	m_isa->install_device(0x0224, 0x0225, read8sm_delegate(*this, FUNC(sb16_device::mixer_r)), write8sm_delegate(*this, FUNC(sb16_device::mixer_w)));
	m_isa->install_device(0x0226, 0x0227, read8sm_delegate(*this, FUNC(sb_device::dsp_reset_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_reset_w)));
	m_isa->install_device(0x022a, 0x022b, read8sm_delegate(*this, FUNC(sb_device::dsp_data_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_data_w)));
	m_isa->install_device(0x022c, 0x022d, read8sm_delegate(*this, FUNC(sb_device::dsp_wbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_cmd_w)));
	m_isa->install_device(0x022e, 0x022f, read8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_w)));
	m_isa->install_device(0x0330, 0x0331, read8sm_delegate(*this, FUNC(sb16_device::mpu401_r)), write8sm_delegate(*this, FUNC(sb16_device::mpu401_w)));
	m_isa->install_device(0x0388, 0x038b, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	m_isa->install_device(0x0220, 0x0223, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	m_isa->install_device(0x0228, 0x0229, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));

	save_item(NAME(m_mixer.data));
	save_item(NAME(m_mixer.status));
	save_item(NAME(m_mixer.main_vol));
	save_item(NAME(m_mixer.dac_vol));
	save_item(NAME(m_mixer.fm_vol));
	save_item(NAME(m_mixer.cd_vol));
	save_item(NAME(m_mixer.line_vol));
	save_item(NAME(m_mixer.mic_vol));
	save_item(NAME(m_mixer.pc_speaker_vol));
	save_item(NAME(m_mixer.output_ctl));
	save_item(NAME(m_mixer.input_ctl));
	save_item(NAME(m_mixer.input_gain));
	save_item(NAME(m_mixer.output_gain));
	save_item(NAME(m_mixer.agc));
	save_item(NAME(m_mixer.treble));
	save_item(NAME(m_mixer.bass));

	sb_device::device_start();
}

void isa16_sblaster16_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(1, this, false);
	m_isa->set_dma_channel(5, this, false);
	m_dsp.version = 0x0405; // diagnose.exe rejects anything lower than 0x0402
	sb16_device::device_start();
}

void sb_device::device_start()
{
	m_timer = timer_alloc(FUNC(sb_device::timer_tick), this);

	save_item(NAME(m_dack_out));
	save_item(NAME(m_onebyte_midi));
	save_item(NAME(m_uart_midi));
	save_item(NAME(m_uart_irq));
	save_item(NAME(m_mpu_midi));
	save_item(NAME(m_rx_waiting));
	save_item(NAME(m_tx_waiting));
	save_item(NAME(m_recvring));
	save_item(NAME(m_xmitring));
	save_item(NAME(m_xmit_read));
	save_item(NAME(m_xmit_write));
	save_item(NAME(m_recv_read));
	save_item(NAME(m_recv_write));
	save_item(NAME(m_tx_busy));

	save_item(NAME(m_dsp.reset_latch));
	save_item(NAME(m_dsp.rbuf_status));
	save_item(NAME(m_dsp.wbuf_status));
	save_item(NAME(m_dsp.fifo));
	save_item(NAME(m_dsp.fifo_ptr));
	save_item(NAME(m_dsp.fifo_r));
	save_item(NAME(m_dsp.fifo_r_ptr));

	save_item(NAME(m_dsp.test_reg));
	save_item(NAME(m_dsp.speaker_on));
	save_item(NAME(m_dsp.dma_no_irq));
	save_item(NAME(m_dsp.prot_count));
	save_item(NAME(m_dsp.prot_value));
	save_item(NAME(m_dsp.frequency));
	save_item(NAME(m_dsp.adc_freq));
	save_item(NAME(m_dsp.dma_length));
	save_item(NAME(m_dsp.dma_transferred));
	save_item(NAME(m_dsp.adc_length));
	save_item(NAME(m_dsp.adc_transferred));
	save_item(NAME(m_dsp.dma_autoinit));
	save_item(NAME(m_dsp.data));
	save_item(NAME(m_dsp.d_wptr));
	save_item(NAME(m_dsp.d_rptr));
	save_item(NAME(m_dsp.dma_timer_started));
	save_item(NAME(m_dsp.dma_throttled));
	save_item(NAME(m_dsp.flags));
	save_item(NAME(m_dsp.irq_active));
	save_item(NAME(m_dsp.adpcm_new_ref));
	save_item(NAME(m_dsp.adpcm_ref));
	save_item(NAME(m_dsp.adpcm_step));
	save_item(NAME(m_dsp.adpcm_count));

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sb_device::device_reset()
{
	m_dsp.prot_value = 0xaa;
	m_dsp.prot_count = 0;
	m_dack_out = 0;
	m_dsp.fifo_ptr = 0;
	m_dsp.fifo_r_ptr = 0;
	m_dsp.wbuf_status = 0;
	m_dsp.rbuf_status = 0;
	m_dsp.frequency = 8000; // per stereo-fx
	m_dsp.flags = 0;
	m_dsp.irq_active = 0;
	m_dsp.dma_no_irq = false;
	mixer_reset();

	m_ldac->write(0x8000);
	m_rdac->write(0x8000);

	m_onebyte_midi = false;
	m_uart_midi = false;
	m_uart_irq = false;
	m_mpu_midi = false;
	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;
	m_recv_read = m_recv_write = 0;
	m_rx_waiting = m_tx_waiting = 0;

	// MIDI is 31250 baud, 8-N-1
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(31250);
}

uint8_t sb_device::dack_r(int line)
{
	m_dsp.adc_transferred++;
	if(m_dsp.adc_transferred >= m_dsp.adc_length)
	{
		drq_w(0);

		if (m_dsp.dma_autoinit)
		{
			m_dsp.adc_transferred = 0;
			drq_w(1);
		}
		else
			m_dsp.wbuf_status = 0;
		if(!m_dsp.dma_no_irq)
			irq_w(1, IRQ_DMA8);
		else
			m_dsp.dma_no_irq = false;
	}
	else
		drq_w(1);
	return m_dack_out;
}

uint16_t sb16_device::dack16_r(int line)
{
	m_dsp.adc_transferred += 2;
	if (m_dsp.adc_transferred >= m_dsp.adc_length)
	{
		drq16_w(0);

		if (m_dsp.dma_autoinit)
		{
			m_dsp.adc_transferred = 0;
			drq16_w(1);
		}
		irq_w(1, IRQ_DMA16);
	}
	else
		drq16_w(1);
	return m_dack_out;
}

void sb16_device::dack16_w(int line, uint16_t data)
{
	// set the transfer timer on the 1st byte
	if (!m_dsp.dma_timer_started)
	{
		m_timer->adjust(attotime::from_hz((double)m_dsp.frequency), 0, attotime::from_hz((double)m_dsp.frequency));
		m_dsp.d_rptr = m_dsp.d_wptr = 0;
		m_dsp.dma_timer_started = true;
	}

	m_dsp.data[m_dsp.d_wptr++] = data & 0xff;
	m_dsp.data[m_dsp.d_wptr++] = data >> 8;
	m_dsp.d_wptr %= 128;

	if (m_dsp.d_wptr == m_dsp.d_rptr)
	{
//        printf("throttling DRQ\n");
		drq16_w(0); // drop DRQ here
		m_dsp.dma_throttled = true;
	}

	m_dsp.dma_transferred += 2;
	if (m_dsp.dma_transferred >= m_dsp.dma_length)
	{
//        printf("DMA fill completed (%d out of %d)\n", m_dsp.dma_transferred, m_dsp.dma_length);

		drq16_w(0); // drop DRQ here

		if (m_dsp.dma_autoinit)
		{
//            printf("autoinit reset\n");
			m_dsp.dma_transferred = 0;
			if (!m_dsp.dma_throttled)   // if we're not throttled, re-raise DRQ right now
			{
				drq16_w(1);   // raise DRQ again (page 3-15 of the Creative manual indicates auto-init will keep going until you stop it)
			}
		}

		irq_w(1, IRQ_DMA16);    // raise IRQ as per the Creative manual
	}
}

/* TODO: this mustn't be instant! */
void sb_device::dack_w(int line, uint8_t data)
{
//    printf("dack_w: line %x data %02x\n", line, data);
//  if(data != 0x80)
//      printf("%02x\n",data);

	// set the transfer timer on the 1st byte
	if (!m_dsp.dma_timer_started)
	{
		m_timer->adjust(attotime::from_hz((double)m_dsp.frequency), 0, attotime::from_hz((double)m_dsp.frequency));
		m_dsp.d_rptr = m_dsp.d_wptr = 0;
		m_dsp.dma_timer_started = true;
	}

	m_dsp.data[m_dsp.d_wptr++] = data;
	m_dsp.d_wptr %= 128;

	if (m_dsp.d_wptr == m_dsp.d_rptr)
	{
//        printf("throttling DRQ\n");
		drq_w(0);   // drop DRQ here
		m_dsp.dma_throttled = true;
	}

	m_dsp.dma_transferred++;
	if (m_dsp.dma_transferred >= m_dsp.dma_length)
	{
//        printf("DMA fill completed (%d out of %d)\n", m_dsp.dma_transferred, m_dsp.dma_length);

		drq_w(0);   // drop DRQ here

		if (m_dsp.dma_autoinit)
		{
//            printf("autoinit reset\n");
			m_dsp.dma_transferred = 0;
			if (!m_dsp.dma_throttled)   // if we're not throttled, re-raise DRQ right now
			{
				drq_w(1);   // raise DRQ again (page 3-15 of the Creative manual indicates auto-init will keep going until you stop it)
			}
		}

		irq_w(1, IRQ_DMA8); // raise IRQ as per the Creative manual
	}
}

TIMER_CALLBACK_MEMBER(sb_device::timer_tick)
{
//    printf("DMA timer expire\n");
	uint16_t lsample, rsample;
	switch (m_dsp.flags) {
		case 0: // 8-bit unsigned mono
			m_ldac->write(m_dsp.data[m_dsp.d_rptr] << 8);
			m_rdac->write(m_dsp.data[m_dsp.d_rptr] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			break;
		case SIGNED: // 8-bit signed mono
			m_ldac->write((m_dsp.data[m_dsp.d_rptr] ^ 0x80) << 8);
			m_rdac->write((m_dsp.data[m_dsp.d_rptr] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			break;
		case STEREO: // 8-bit unsigned stereo
			m_ldac->write(m_dsp.data[m_dsp.d_rptr] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			m_rdac->write(m_dsp.data[m_dsp.d_rptr] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			break;
		case SIGNED | STEREO: // 8-bit signed stereo
			m_ldac->write((m_dsp.data[m_dsp.d_rptr] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_rdac->write((m_dsp.data[m_dsp.d_rptr] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			break;
		case SIXTEENBIT: // 16-bit unsigned mono
			lsample = m_dsp.data[m_dsp.d_rptr] | (m_dsp.data[m_dsp.d_rptr+1] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			m_ldac->write(lsample);
			m_rdac->write(lsample);
			break;
		case SIXTEENBIT | SIGNED: // 16-bit signed mono
			lsample = m_dsp.data[m_dsp.d_rptr] | ((m_dsp.data[m_dsp.d_rptr+1] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_ldac->write(lsample);
			m_rdac->write(lsample);
			break;
		case SIXTEENBIT | STEREO: // 16-bit unsigned stereo
			lsample = m_dsp.data[m_dsp.d_rptr] | (m_dsp.data[m_dsp.d_rptr+1] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			m_dsp.d_rptr %= 128;
			rsample = m_dsp.data[m_dsp.d_rptr] | (m_dsp.data[m_dsp.d_rptr+1] << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x80;
			m_ldac->write(lsample);
			m_rdac->write(rsample);
			break;
		case SIXTEENBIT | SIGNED | STEREO: // 16-bit signed stereo
			lsample = m_dsp.data[m_dsp.d_rptr] | ((m_dsp.data[m_dsp.d_rptr+1] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.d_rptr %= 128;
			rsample = m_dsp.data[m_dsp.d_rptr] | ((m_dsp.data[m_dsp.d_rptr+1] ^ 0x80) << 8);
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_dsp.data[m_dsp.d_rptr++] = 0x00;
			m_ldac->write(lsample);
			m_rdac->write(rsample);
			break;
		case ADPCM2:
			if(m_dsp.adpcm_new_ref)
			{
				m_dsp.adpcm_ref = m_dsp.data[m_dsp.d_rptr++];
				m_dsp.adpcm_new_ref = false;
				m_ldac->write(m_dsp.adpcm_ref << 8);
				m_rdac->write(m_dsp.adpcm_ref << 8);
				break;
			}
			lsample = m_dsp.data[m_dsp.d_rptr];
			switch(m_dsp.adpcm_count++)
			{
				case 0:
					adpcm_decode(lsample >> 6, 2);
					break;
				case 1:
					adpcm_decode((lsample >> 4) & 3, 2);
					break;
				case 2:
					adpcm_decode((lsample >> 2) & 3, 2);
					break;
				case 3:
					adpcm_decode(lsample & 3, 2);
					m_dsp.data[m_dsp.d_rptr++] = 0x80;
					m_dsp.adpcm_count = 0;
					break;
			}
			break;
		case ADPCM3:
			if(m_dsp.adpcm_new_ref)
			{
				m_dsp.adpcm_ref = m_dsp.data[m_dsp.d_rptr++];
				m_dsp.adpcm_new_ref = false;
				m_ldac->write(m_dsp.adpcm_ref << 8);
				m_rdac->write(m_dsp.adpcm_ref << 8);
				break;
			}
			lsample = m_dsp.data[m_dsp.d_rptr];
			switch(m_dsp.adpcm_count++)
			{
				case 0:
					adpcm_decode(lsample >> 5, 3);
					break;
				case 1:
					adpcm_decode((lsample >> 2) & 7, 3);
					break;
				case 2:
					adpcm_decode(((lsample & 2) << 1) | (lsample & 1), 3);
					m_dsp.data[m_dsp.d_rptr++] = 0x80;
					m_dsp.adpcm_count = 0;
					break;
			}
			break;
		case ADPCM4:
			if(m_dsp.adpcm_new_ref)
			{
				m_dsp.adpcm_ref = m_dsp.data[m_dsp.d_rptr++];
				m_dsp.adpcm_new_ref = false;
				m_ldac->write(m_dsp.adpcm_ref << 8);
				m_rdac->write(m_dsp.adpcm_ref << 8);
				break;
			}
			lsample = m_dsp.data[m_dsp.d_rptr];
			switch(m_dsp.adpcm_count++)
			{
				case 0:
					adpcm_decode(lsample >> 4, 4);
					break;
				case 1:
					adpcm_decode(lsample & 15, 4);
					m_dsp.data[m_dsp.d_rptr++] = 0x80;
					m_dsp.adpcm_count = 0;
					break;
			}
			break;
		default:
			logerror("SB: unimplemented sample type %x\n", m_dsp.flags);
	}
	m_dsp.d_rptr %= 128;

	if (m_dsp.dma_throttled)
	{
		if (m_dsp.d_rptr == m_dsp.d_wptr)
		{
//            printf("unthrottling DRQ\n");
			if(m_dsp.flags & SIXTEENBIT) // 16-bit audio through 8-bit dma?
				drq16_w(1);
			else
				drq_w(1);   // raise DRQ
			m_dsp.dma_throttled = false;
		}
	}
}

void sb_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

	// in UART MIDI mode, we set the DMA8 IRQ on receiving a character
	if (m_uart_midi)
	{
		m_recvring[m_recv_write++] = data;
		if (m_recv_write >= MIDI_RING_SIZE)
		{
			m_recv_write = 0;
		}

		if (m_recv_write != m_recv_read)
		{
			m_rx_waiting++;
		}
		if (m_uart_irq)
		{
			irq_w(1, IRQ_DMA8);
		}
	}
}

void sb16_device::rcv_complete()    // Rx completed receiving byte
{
	receive_register_extract();
	uint8_t data = get_received_char();

	// for UART or MPU, add character to the receive queue
	if (m_uart_midi || m_mpu_midi)
	{
		m_recvring[m_recv_write++] = data;
		if (m_recv_write >= MIDI_RING_SIZE)
		{
			m_recv_write = 0;
		}

		if (m_recv_write != m_recv_read)
		{
			m_rx_waiting++;
		}

		if (m_uart_irq)
		{
			irq_w(1, IRQ_DMA8);
		}

		if (m_mpu_midi)
		{
			irq_w(1, IRQ_MPU);
		}
	}
}

void sb_device::tra_complete()    // Tx completed sending byte
{
//  printf("Tx complete\n");
	// is there more waiting to send?
	if (m_tx_waiting)
	{
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= MIDI_RING_SIZE)
		{
			m_xmit_read = 0;
		}
		m_tx_waiting--;
	}
	else
	{
		m_tx_busy = false;
	}
}

void sb_device::tra_callback()    // Tx send bit
{
	int bit = transmit_register_get_data_bit();
	m_mdout->write_txd(bit);
}

void sb_device::xmit_char(uint8_t data)
{
//  printf("SB: xmit %02x\n", data);

	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= MIDI_RING_SIZE)
		{
			m_xmit_write = 0;
		}
		m_tx_waiting++;
	}
}

void isa16_sblaster16_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		ymf262_device &ymf262 = *subdevice<ymf262_device>("ymf262");
		m_isa->install_device(0x0200, 0x0207, read8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_r)), write8smo_delegate(*subdevice<pc_joy_device>("pc_joy"), FUNC(pc_joy_device::joy_port_w)));
		m_isa->install_device(0x0226, 0x0227, read8sm_delegate(*this, FUNC(sb_device::dsp_reset_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_reset_w)));
		m_isa->install_device(0x022a, 0x022b, read8sm_delegate(*this, FUNC(sb_device::dsp_data_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_data_w)));
		m_isa->install_device(0x022c, 0x022d, read8sm_delegate(*this, FUNC(sb_device::dsp_wbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_cmd_w)));
		m_isa->install_device(0x022e, 0x022f, read8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_r)), write8sm_delegate(*this, FUNC(sb_device::dsp_rbuf_status_w)));
		m_isa->install_device(0x0224, 0x0225, read8sm_delegate(*this, FUNC(sb16_device::mixer_r)), write8sm_delegate(*this, FUNC(sb16_device::mixer_w)));
		m_isa->install_device(0x0330, 0x0331, read8sm_delegate(*this, FUNC(sb16_device::mpu401_r)), write8sm_delegate(*this, FUNC(sb16_device::mpu401_w)));
		m_isa->install_device(0x0388, 0x038b, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
		m_isa->install_device(0x0220, 0x0223, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
		m_isa->install_device(0x0228, 0x0229, read8sm_delegate(ymf262, FUNC(ymf262_device::read)), write8sm_delegate(ymf262, FUNC(ymf262_device::write)));
	}
}
