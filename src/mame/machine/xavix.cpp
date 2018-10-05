// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "includes/xavix.h"

// general DMA fro ROM, not Video DMA
WRITE8_MEMBER(xavix_state::rom_dmatrg_w)
{
	if (data & 0x01) // namcons2 writes 0x81, most of the time things write 0x01
	{
		logerror("%s: rom_dmatrg_w (do DMA?) %02x\n", machine().describe_context(), data);

		uint32_t source = (m_rom_dmasrc_hi_data << 16) | (m_rom_dmasrc_md_data << 8) | m_rom_dmasrc_lo_data;
		uint16_t dest = (m_rom_dmadst_hi_data << 8) | m_rom_dmadst_lo_data;
		uint16_t len = (m_rom_dmalen_hi_data << 8) | m_rom_dmalen_lo_data;

		source &= m_rgnlen - 1;
		logerror("  (possible DMA op SRC %08x DST %04x LEN %04x)\n", source, dest, len);

		address_space& destspace = m_maincpu->space(AS_PROGRAM);

		for (int i = 0; i < len; i++)
		{
			uint8_t dat = m_rgn[(source + i) & (m_rgnlen - 1)];
			destspace.write_byte(dest + i, dat);
		}
	}
	else // the interrupt routine writes 0x80 to the trigger, maybe 'clear IRQ?'
	{
		logerror("%s: rom_dmatrg_w (unknown) %02x\n", machine().describe_context(), data);
	}
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_lo_w)
{
	logerror("%s: rom_dmasrc_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_md_w)
{
	logerror("%s: rom_dmasrc_md_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_md_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_hi_w)
{
	logerror("%s: rom_dmasrc_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_hi_data = data;
	// this would mean Taito Nostalgia relies on mirroring tho, as it has the high bits set... so could just be wrong
	logerror("  (DMA ROM source of %02x%02x%02x)\n", m_rom_dmasrc_hi_data, m_rom_dmasrc_md_data, m_rom_dmasrc_lo_data);
}

WRITE8_MEMBER(xavix_state::rom_dmadst_lo_w)
{
	logerror("%s: rom_dmadst_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmadst_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmadst_hi_w)
{
	logerror("%s: rom_dmadst_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmadst_hi_data = data;

	logerror("  (DMA dest of %02x%02x)\n", m_rom_dmadst_hi_data, m_rom_dmadst_lo_data);
}

WRITE8_MEMBER(xavix_state::rom_dmalen_lo_w)
{
	logerror("%s: rom_dmalen_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmalen_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmalen_hi_w)
{
	logerror("%s: rom_dmalen_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmalen_hi_data = data;

	logerror("  (DMA len of %02x%02x)\n", m_rom_dmalen_hi_data, m_rom_dmalen_lo_data);
}

READ8_MEMBER(xavix_state::rom_dmatrg_r)
{
	logerror("%s: rom_dmatrg_r (operation status?)\n", machine().describe_context());
	return 0x00;
}



WRITE8_MEMBER(xavix_state::vector_enable_w)
{
	logerror("%s: vector_enable_w %02x\n", machine().describe_context(), data);
	m_vectorenable = data;
}

WRITE8_MEMBER(xavix_state::irq_vector0_lo_w)
{
	logerror("%s: irq_vector0_lo_w %02x\n", machine().describe_context(), data);
	m_irq_vector0_lo_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector0_hi_w)
{
	logerror("%s: irq_vector0_hi_w %02x\n", machine().describe_context(), data);
	m_irq_vector0_hi_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector1_lo_w)
{
	logerror("%s: irq_vector1_lo_w %02x\n", machine().describe_context(), data);
	m_irq_vector1_lo_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector1_hi_w)
{
	logerror("%s: irq_vector1_hi_w %02x\n", machine().describe_context(), data);
	m_irq_vector1_hi_data = data;
}


WRITE8_MEMBER(xavix_state::extintrf_7900_w)
{
	logerror("%s: extintrf_7900_w %02x (---FIRST WRITE ON STARTUP---)\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::extintrf_7901_w)
{
	logerror("%s: extintrf_7901_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::extintrf_7902_w)
{
	logerror("%s: extintrf_7902_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_7a80_w)
{
	logerror("%s: xavix_7a80_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::adc_7b00_w)
{
	logerror("%s: adc_7b00_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::adc_7b80_r)
{
	logerror("%s: adc_7b80_r\n", machine().describe_context());
	return 0xff;
}

WRITE8_MEMBER(xavix_state::adc_7b80_w)
{
	logerror("%s: adc_7b80_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::adc_7b81_w)
{
	logerror("%s: adc_7b81_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::slotreg_7810_w)
{
	logerror("%s: slotreg_7810_w %02x\n", machine().describe_context(), data);
}


TIMER_DEVICE_CALLBACK_MEMBER(xavix_state::scanline_cb)
{

}

INTERRUPT_GEN_MEMBER(xavix_state::interrupt)
{
	if (m_6ff8 & 0x20)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}



READ8_MEMBER(xavix_state::colmix_6ff0_r)
{
	//logerror("%s: colmix_6ff0_r\n", machine().describe_context());
	return m_6ff0;
}

WRITE8_MEMBER(xavix_state::colmix_6ff0_w)
{
	// expected to return data written
	m_6ff0 = data;
	//logerror("%s: colmix_6ff0_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::colmix_6ff1_w)
{
	logerror("%s: colmix_6ff1_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::colmix_6ff2_w)
{
	logerror("%s: colmix_6ff2_w %02x\n", machine().describe_context(), data);
}


READ8_MEMBER(xavix_state::dispctrl_6ff8_r)
{
	//logerror("%s: dispctrl_6ff8_r\n", machine().describe_context());
	return m_6ff8;
}

WRITE8_MEMBER(xavix_state::dispctrl_6ff8_w)
{
	// I think this is something to do with IRQ ack / enable
	m_6ff8 = data;
	logerror("%s: dispctrl_6ff8_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::dispctrl_6ffa_w)
{
	logerror("%s: dispctrl_6ffa_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::dispctrl_6ffb_w)
{
	logerror("%s: dispctrl_6ffb_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_75f0_r)
{
	logerror("%s: sound_75f0_r\n", machine().describe_context());
	return m_75fx[0];
}

READ8_MEMBER(xavix_state::sound_75f1_r)
{
	logerror("%s: sound_75f1_r\n", machine().describe_context());
	return m_75fx[1];
}

READ8_MEMBER(xavix_state::sound_75f6_r)
{
	logerror("%s: sound_75f6_r\n", machine().describe_context());
	return m_75fx[6];
}

READ8_MEMBER(xavix_state::sound_75f8_r)
{
	logerror("%s: sound_75f8_r\n", machine().describe_context());
	return m_75fx[8];
}

READ8_MEMBER(xavix_state::sound_75f9_r)
{
	logerror("%s: sound_75f9_r\n", machine().describe_context());
	return 0x00;
}

READ8_MEMBER(xavix_state::sound_75fa_r)
{
	logerror("%s: sound_75fa_r\n", machine().describe_context());
	return m_75fx[10];
}

READ8_MEMBER(xavix_state::sound_75fb_r)
{
	logerror("%s: sound_75fb_r\n", machine().describe_context());
	return m_75fx[11];
}

READ8_MEMBER(xavix_state::sound_75fc_r)
{
	logerror("%s: sound_75fc_r\n", machine().describe_context());
	return m_75fx[12];
}

READ8_MEMBER(xavix_state::sound_75fd_r)
{
	logerror("%s: sound_75fd_r\n", machine().describe_context());
	return m_75fx[13];
}




WRITE8_MEMBER(xavix_state::sound_75f0_w)
{
	// expected to return data written
	m_75fx[0] = data;
	logerror("%s: sound_75f0_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f1_w)
{
	// expected to return data written
	m_75fx[1] = data;
	logerror("%s: sound_75f1_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f6_w)
{
	// expected to return data written
	m_75fx[6] = data;
	logerror("%s: sound_75f6_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75f7_w)
{
	m_75fx[7] = data;
	logerror("%s: sound_75f7_w %02x\n", machine().describe_context(), data);
}


WRITE8_MEMBER(xavix_state::sound_75f8_w)
{
	// expected to return data written
	m_75fx[8] = data;
	logerror("%s: sound_75f8_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75f9_w)
{
	m_75fx[9] = data;
	logerror("%s: sound_75f9_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fa_w)
{
	// expected to return data written
	m_75fx[10] = data;
	logerror("%s: sound_75fa_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fb_w)
{
	// expected to return data written
	m_75fx[11] = data;
	logerror("%s: sound_75fb_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fc_w)
{
	// expected to return data written
	m_75fx[12] = data;
	logerror("%s: sound_75fc_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fd_w)
{
	// expected to return data written
	m_75fx[13] = data;
	logerror("%s: sound_75fd_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75fe_w)
{
	m_75fx[14] = data;
	logerror("%s: sound_75fe_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::sound_75ff_w)
{
	m_75fx[15] = data;
	logerror("%s: sound_75ff_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::io_0_r)
{
	return m_in0->read();
}

READ8_MEMBER(xavix_state::io_1_r)
{
	/*
	int pc = m_maincpu->state_int(M6502_PC);

	if (pc == 0x3acc) return 0x08;
	if (pc == 0x3ae0) return 0x08;
	if (pc == 0xfcb0) return 0xff;

	logerror("%04x: in1 read\n", pc);
	*/
	return m_in1->read();
}

READ8_MEMBER(xavix_state::io_2_r)
{
	return 0xff;
}

READ8_MEMBER(xavix_state::io_3_r)
{
	return 0xff;
}


WRITE8_MEMBER(xavix_state::io_0_w)
{
	logerror("%s: io_0_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::io_1_w)
{
	logerror("%s: io_1_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::io_2_w)
{
	logerror("%s: io_2_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::io_3_w)
{
	logerror("%s: io_3_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::arena_6fe8_r)
{
	logerror("%s: arena_6fe8_r\n", machine().describe_context());
	return m_6fe8;
}

WRITE8_MEMBER(xavix_state::arena_6fe8_w)
{
	// expected to return data written
	m_6fe8 = data;
	logerror("%s: arena_6fe8_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::arena_6fe9_r)
{
	logerror("%s: arena_6fe9_r\n", machine().describe_context());
	return m_6fe9;
}

WRITE8_MEMBER(xavix_state::arena_6fe9_w)
{
	// expected to return data written
	m_6fe9 = data;
	logerror("%s: arena_6fe9_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::arena_6fea_w)
{
	logerror("%s: arena_6fea_w %02x\n", machine().describe_context(), data);
}


READ8_MEMBER(xavix_state::timer_7c01_r)
{
	logerror("%s: timer_7c01_r\n", machine().describe_context());
	return m_7c01;
}

WRITE8_MEMBER(xavix_state::timer_7c00_w)
{
	logerror("%s: timer_7c00_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::timer_7c01_w)
{
	// expected to return data written
	m_7c01 = data;
	logerror("%s: timer_7c01_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::timer_7c02_w)
{
	logerror("%s: timer_7c02_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::sound_75f4_r)
{
	// used with 75f0
	return 0xff;
}

READ8_MEMBER(xavix_state::sound_75f5_r)
{
	// used with 75f1
	return 0xff;
}

READ8_MEMBER(xavix_state::mult_r)
{
	return m_multresults[offset];
}

WRITE8_MEMBER(xavix_state::mult_w)
{
	// rad_fb writes here, why would you write to the results registers?
	logerror("%s: mult_w (write to multiply RESULT registers, why?) reg: %d %02x\n", machine().describe_context(), offset, data);
	m_multresults[offset] = data;
}

WRITE8_MEMBER(xavix_state::mult_param_w)
{
	COMBINE_DATA(&m_multparams[offset]);
	// there are NOPs after one of the writes, so presumably the operation is write triggerd and not intstant
	// see test code at 0184a4 in monster truck

	if (offset == 2)
	{
		// assume 0 is upper bits, might be 'mode' instead, check
		uint16_t param1 = (m_multparams[0]<<8) | (m_multparams[1]);
		uint8_t param2 = (m_multparams[2]);

		uint16_t result =  param1*param2;

		m_multresults[1] = (result>>8)&0xff;
		m_multresults[0] = result&0xff;
	}
}


READ8_MEMBER(xavix_state::irq_source_r)
{
	/* the 2nd IRQ routine (regular IRQ, not NMI?) reads here before deciding what to do

	 the following bits have been seen to be checked (active low?)

	  0x40 - Monster Truck - stuff with 6ffb 6fd6 and 6ff8
	  0x20 - most games (but not Monster Truck) - DMA related?
	  0x10 - card night + monster truck - 7c00 related? (increases 16-bit counter in ram stores 0xc1 at 7c00)
	  0x08 - several games - Input related (ADC? - used for analog control on Monster Truck) (uses 7a80 top bit to determine direction, and 7a81 0x08 as an output, presumably to clock)
	  0x04 - Monster Truck - loads/stores 7b81
	*/
	logerror("%s: irq_source_r\n", machine().describe_context());
	return 0xff;
}

WRITE8_MEMBER(xavix_state::irq_source_w)
{
	logerror("%s: irq_source_w %02x\n", machine().describe_context(), data);
	// cleared on startup in monster truck, no purpose?
}


void xavix_state::machine_start()
{

}

void xavix_state::machine_reset()
{
	m_rom_dmasrc_lo_data = 0;
	m_rom_dmasrc_md_data = 0;
	m_rom_dmasrc_hi_data = 0;

	m_rom_dmadst_lo_data = 0;
	m_rom_dmadst_hi_data = 0;

	m_rom_dmalen_lo_data = 0;
	m_rom_dmalen_hi_data = 0;

	m_vectorenable = 0;
	m_irq_vector0_lo_data = 0;
	m_irq_vector0_hi_data = 0;
	m_irq_vector1_lo_data = 0;
	m_irq_vector1_hi_data = 0;

	m_6ff0 = 0;
	m_6ff8 = 0;

	m_75fx[0] = 0;
	m_75fx[1] = 0;

	m_75fx[6] = 0;
	m_75fx[8] = 0;
	m_75fx[10] = 0;
	m_75fx[11] = 0;
	m_75fx[12] = 0;
	m_75fx[13] = 0;

	for (int i = 0; i < 3; i++)
		m_multparams[i] = 0;

	for (int i = 0; i < 2; i++)
		m_multresults[i] = 0;

	for (int i = 0; i < 2; i++)
	{
		m_vid_dma_param1[i] = 0;
		m_vid_dma_param2[i] = 0;
	}

	for (int i = 0; i < 8; i++)
	{
		m_tmap1_regs[i] = 0;
		m_tmap2_regs[i] = 0;
	}

	m_lowbus->set_bank(0);
}

typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

int16_t xavix_state::get_vectors(int which, int half)
{
//  logerror("get_vectors %d %d\n", which, half);
	if (m_vectorenable == 0)
		return -1;

	if (which == 0) // irq?
	{
		if (half == 0)
			return m_irq_vector0_hi_data;
		else
			return m_irq_vector0_lo_data;
	}
	else
	{
		if (half == 0)
			return m_irq_vector1_hi_data;
		else
			return m_irq_vector1_lo_data;
	}
}


