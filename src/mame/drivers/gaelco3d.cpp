// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

    Games supported:
        * Speed Up
        * Surf Planet
        * Radikal Bikers

    Known bugs:
        * EEPROM interface not right

***************************************************************************

PCB Layout -- Radikal Bikers
----------

Top board

REF. 980311
|----------------------------------------------------------------|
|  RAB.6  RAB.14                          PAL            RAB.48* |
|     RAB.12  RAB.19                                             |
|                         RAB.23*   ADSP-2115  16MHz     RAB.45* |
|                                                                |
|                                                                |
|                             RAB.24  RAB.32           TDA1543   |
|                                                         TL074  |
| RAB.8*    RAB.15*           RAB.25  RAB.33                     |
| RAB.9*    RAB.16*                                              |
|                             RAB.26  RAB.34                     |
|                                                                |
| RAB.10*   RAB.17*           RAB.27  RAB.35           TDA1543   |
| RAB.11*   RAB.18*                                       TL074  |
|                                                                |
|                                                                |
|                                                                |
|----------------------------------------------------------------|
Notes:
      Contents of RAB.24 to RAB.27 = RAB.32 to RAB.35
      * - These ROMs are surface mounted


Bottom board

REF. 980512
|----------------------------------------------------------------|
|       ALTERA                             TMS320C31      50MHz  |
|50MHz  EPM7064   68EC020  CY7C199         (QFP132)              |
|                          CY7C199                               |
|                          CY7C199         KM4216C256            |
|93C66                     CY7C199                               |
|                          CY7C199        |--------|             |
|                          CY7C199        |3D-3G   |             |
|75LBC176                                 |(QFP206)|             |
|75LBC176                                 |        |             |
|                   |------------|        |--------|             |
|                   |CHIP EXPRESS|                               |
|                   |RASTER      |         45MHz                 |
|                   |M1178-01    |                               |
|                   |M032541-4   |                               |
|     CY7C199       |------------|                               |
|     CY7C199                                                    |
|     CY7C199                                                    |
|     CY7C199                                                    |
|                  KM4216C256   KM4216C256                       |
|  |------------|                                                |
|  |CHIP EXPRESS|  KM4216C256   KM4216C256    PAL                |
|  |CHK1        |                                                |
|  |M1105-01    |               KM4216C256                       |
|  |M048494-22  |                                                |
|  |------------|                                                |
|----------------------------------------------------------------|


PCB Layout -- Surf Planet
----------

Top board

REF. 971223
|----------------------------------------------------------------|
|  PLS.5  PLS.11                          PAL            PLS.37* |
|     PLS.8  PLS.13                                              |
|                         PLS.18*   ADSP-2115  16MHz     PLS.40* |
|                                                                |
|                                                                |
|                             PLS.19  PLS.27           TDA1543   |
|                                                         TL074  |
|                             PLS.20  PLS.28                     |
|                                                                |
|                             PLS.21  PLS.29                     |
|                                                                |
| PLS.7*  PLS.12*             PLS.22  PLS.30           TDA1543   |
|    PLS.9*   PLS.15*                                     TL074  |
|                                                                |
| TLC549                                                         |
| LM358                                                          |
|----------------------------------------------------------------|
Notes:
      Contents of PLS.19 to PLS.22 = PLS.27 to PLS.30
      * - These ROMs are surface mounted


Bottom board

REF. 970429
|----------------------------------------------------------------|
|       PAL                 60MHz          TMS320C31             |
|          68HC000         CY7C199         (QFP132)              |
|                          CY7C199                               |
|                          CY7C199         KM4216C256            |
|93C66                     CY7C199                               |
|                          CY7C199        |--------|             |
|                          CY7C199        |3D-3G   |             |
|75LBC176                                 |(QFP206)|             |
|75LBC176                                 |        |             |
|                   |------------|        |--------|             |
|                   |CHIP EXPRESS|                               |
|                   |RASTER      |         45MHz                 |
|                   |M027851-1   |                               |
|                   |9706 CI USA |                               |
|     CY7C199       |------------|                               |
|     CY7C199                                                    |
|     CY7C199                                                    |
|     CY7C199                                                    |
|                  KM4216C256   KM4216C256                       |
|  |------------|                                                |
|  |CHIP EXPRESS|  KM4216C256   KM4216C256    PAL                |
|  |SU3DCOL     |                                                |
|  |M026402-3   |               KM4216C256                       |
|  |9647 CI USA |                                                |
|  |------------|                                                |
|----------------------------------------------------------------|


6 * KM4216C256G = 6 * 256k x 16
10 * CY7C199 =   10 * 32k x 8


**************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/gaelco3d.h"
#include "cpu/tms32031/tms32031.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/eepromser.h"

#define LOG             0





WRITE_LINE_MEMBER(gaelco3d_state::ser_irq)
{
	if (state)
		m_maincpu->set_input_line(6, ASSERT_LINE);
	else
		m_maincpu->set_input_line(6, CLEAR_LINE);
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

void gaelco3d_state::machine_start()
{
	/* Save state support */
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_status));
	save_item(NAME(m_analog_ports));
	save_item(NAME(m_framenum));
	save_item(NAME(m_adsp_ireg));
	save_item(NAME(m_adsp_ireg_base));
	save_item(NAME(m_adsp_incs));
	save_item(NAME(m_adsp_size));
}


MACHINE_RESET_MEMBER(gaelco3d_state,common)
{
	UINT16 *src;
	int i;

	m_framenum = 0;

	/* boot the ADSP chip */
	src = (UINT16 *)memregion("user1")->base();
	for (i = 0; i < (src[3] & 0xff) * 8; i++)
	{
		UINT32 opcode = ((src[i*4+0] & 0xff) << 16) | ((src[i*4+1] & 0xff) << 8) | (src[i*4+2] & 0xff);
		m_adsp_ram_base[i] = opcode;
	}

	/* allocate a timer for feeding the autobuffer */
	m_adsp_autobuffer_timer = machine().device<timer_device>("adsp_timer");

	membank("bank1")->configure_entries(0, 256, memregion("user1")->base(), 0x4000);
	membank("bank1")->set_entry(0);

	/* keep the TMS32031 halted until the code is ready to go */
	m_tms->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	for (i = 0; i < SOUND_CHANNELS; i++)
	{
		char buffer[10];
		sprintf(buffer, "dac%d", i + 1);
		m_dmadac[i] = machine().device<dmadac_sound_device>(buffer);
	}
}


void gaelco3d_state::machine_reset()
{
	MACHINE_RESET_CALL_MEMBER( common );
	m_tms_offset_xor = 0;
}


MACHINE_RESET_MEMBER(gaelco3d_state,gaelco3d2)
{
	MACHINE_RESET_CALL_MEMBER( common );
	m_tms_offset_xor = BYTE_XOR_BE(0);
}



/*************************************
 *
 *  IRQ handling
 *
 *************************************/

INTERRUPT_GEN_MEMBER(gaelco3d_state::vblank_gen)
{
	gaelco3d_render(*m_screen);
	device.execute().set_input_line(2, ASSERT_LINE);
}


WRITE16_MEMBER(gaelco3d_state::irq_ack_w)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

WRITE32_MEMBER(gaelco3d_state::irq_ack32_w)
{
	if (mem_mask == 0xffff0000)
		irq_ack_w(space, offset, data, mem_mask >> 16);
	else if (ACCESSING_BITS_0_7)
		m_serial->tr_w(space, 0, data & 0x01);
	else
		logerror("%06X:irq_ack_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}


/*************************************
 *
 *  EEPROM (93C66B)
 *  Serial Interface
 *
 *************************************/

READ16_MEMBER(gaelco3d_state::eeprom_data_r)
{
	UINT32 result = 0xffff;

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 is clock */
		/* bit 1 active */
		result &= ~GAELCOSER_EXT_STATUS_MASK;
		result |= m_serial->status_r(space, 0);
	}

	if (m_eeprom->do_read())
		result ^= 0x0004;
	if (LOG)
		logerror("eeprom_data_r(%02X)\n", result);
	return result;
}

READ32_MEMBER(gaelco3d_state::eeprom_data32_r)
{
	if (ACCESSING_BITS_16_31)
		return (eeprom_data_r(space, 0, mem_mask >> 16) << 16) | 0xffff;
	else if (ACCESSING_BITS_0_7)
	{
		UINT8 data = m_serial->data_r(space,0);
		if (LOG)
			logerror("%06X:read(%02X) = %08X & %08X\n", m_maincpu->pc(), offset, data, mem_mask);
		return  data | 0xffffff00;
	}
	else
		logerror("%06X:read(%02X) = mask %08X\n", m_maincpu->pc(), offset, mem_mask);

	return 0xffffffff;
}


WRITE16_MEMBER(gaelco3d_state::eeprom_data_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->di_write(data & 0x01);
	}
	else if (mem_mask != 0xffff)
		logerror("write mask: %08x data %08x\n", mem_mask, data);
}


WRITE16_MEMBER(gaelco3d_state::eeprom_clock_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->clk_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE16_MEMBER(gaelco3d_state::eeprom_cs_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	}
}



/*************************************
 *
 *  Sound CPU comms
 *
 *************************************/

TIMER_CALLBACK_MEMBER(gaelco3d_state::delayed_sound_w)
{
	if (LOG)
		logerror("delayed_sound_w(%02X)\n", param);
	m_sound_data = param;
	m_adsp->set_input_line(ADSP2115_IRQ2, ASSERT_LINE);
}


WRITE16_MEMBER(gaelco3d_state::sound_data_w)
{
	if (LOG)
		logerror("%06X:sound_data_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
	if (ACCESSING_BITS_0_7)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(gaelco3d_state::delayed_sound_w),this), data & 0xff);
}


READ16_MEMBER(gaelco3d_state::sound_data_r)
{
	if (LOG)
		logerror("sound_data_r(%02X)\n", m_sound_data);
	m_adsp->set_input_line(ADSP2115_IRQ2, CLEAR_LINE);
	return m_sound_data;
}


READ16_MEMBER(gaelco3d_state::sound_status_r)
{
	if (LOG)
		logerror("%06X:sound_status_r(%02X) = %02X\n", space.device().safe_pc(), offset, m_sound_status);
	if (ACCESSING_BITS_0_7)
		return m_sound_status;
	return 0xffff;
}


WRITE16_MEMBER(gaelco3d_state::sound_status_w)
{
	if (LOG)
		logerror("sound_status_w(%02X)\n", m_sound_data);
	m_sound_status = data;
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

CUSTOM_INPUT_MEMBER(gaelco3d_state::analog_bit_r)
{
	int which = (FPTR)param;
	return (m_analog_ports[which] >> 7) & 0x01;
}


WRITE16_MEMBER(gaelco3d_state::analog_port_clock_w)
{
	/* a zero/one combo is written here to clock the next analog port bit */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			m_analog_ports[0] <<= 1;
			m_analog_ports[1] <<= 1;
			m_analog_ports[2] <<= 1;
			m_analog_ports[3] <<= 1;
		}
	}
	else
	{
		if (LOG)
			logerror("%06X:analog_port_clock_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
	}
}


WRITE16_MEMBER(gaelco3d_state::analog_port_latch_w)
{
	/* a zero is written here to read the analog ports, and a one is written when finished */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			m_analog_ports[0] = read_safe(ioport("ANALOG0"), 0);
			m_analog_ports[1] = read_safe(ioport("ANALOG1"), 0);
			m_analog_ports[2] = read_safe(ioport("ANALOG2"), 0);
			m_analog_ports[3] = read_safe(ioport("ANALOG3"), 0);
		}
	}
	else
	{
		if (LOG)
			logerror("%06X:analog_port_latch_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
	}

}



/*************************************
 *
 *  TMS32031 interface
 *
 *************************************/

READ32_MEMBER(gaelco3d_state::tms_m68k_ram_r)
{
//  logerror("%06X:tms_m68k_ram_r(%04X) = %08X\n", space.device().safe_pc(), offset, !(offset & 1) ? ((INT32)m_m68k_ram_base[offset/2] >> 16) : (int)(INT16)m_m68k_ram_base[offset/2]);
	return (INT32)(INT16)m_m68k_ram_base[offset ^ m_tms_offset_xor];
}


WRITE32_MEMBER(gaelco3d_state::tms_m68k_ram_w)
{
	m_m68k_ram_base[offset ^ m_tms_offset_xor] = data;
}


WRITE8_MEMBER(gaelco3d_state::tms_iack_w)
{
	if (LOG)
		logerror("iack_w(%d) - %06X\n", data, offset);
	m_tms->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  TMS32031 control
 *
 *************************************/

WRITE16_MEMBER(gaelco3d_state::tms_reset_w)
{
	/* this is set to 0 while data is uploaded, then set to $ffff after it is done */
	/* it does not ever appear to be touched after that */
	if (LOG)
		logerror("%06X:tms_reset_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
		m_tms->set_input_line(INPUT_LINE_RESET, (data == 0xffff) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE16_MEMBER(gaelco3d_state::tms_irq_w)
{
	/* this is written twice, 0,1, in quick succession */
	/* done after uploading, and after modifying the comm area */
	if (LOG)
		logerror("%06X:tms_irq_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
	if (ACCESSING_BITS_0_7)
		m_tms->set_input_line(0, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}


WRITE16_MEMBER(gaelco3d_state::tms_control3_w)
{
	if (LOG)
		logerror("%06X:tms_control3_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}


WRITE16_MEMBER(gaelco3d_state::tms_comm_w)
{
	COMBINE_DATA(&m_tms_comm_base[offset ^ m_tms_offset_xor]);
	if (LOG)
		logerror("%06X:tms_comm_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset*2, data, mem_mask);
}



/*************************************
 *
 *  ADSP control registers
 *
 *************************************/

/* These are some of the control registers. We don't use them all */
enum
{
	S1_AUTOBUF_REG = 15,
	S1_RFSDIV_REG,
	S1_SCLKDIV_REG,
	S1_CONTROL_REG,
	S0_AUTOBUF_REG,
	S0_RFSDIV_REG,
	S0_SCLKDIV_REG,
	S0_CONTROL_REG,
	S0_MCTXLO_REG,
	S0_MCTXHI_REG,
	S0_MCRXLO_REG,
	S0_MCRXHI_REG,
	TIMER_SCALE_REG,
	TIMER_COUNT_REG,
	TIMER_PERIOD_REG,
	WAITSTATES_REG,
	SYSCONTROL_REG
};

/*
ADSP control 3FFF W = 0008  (SYSCONTROL_REG)
ADSP control 3FFE W = 1249  (WAITSTATES_REG)
ADSP control 3FEF W = 0D82  (S1_AUTOBUF_REG)
ADSP control 3FF1 W = 0005  (S1_SCLKDIV_REG)
ADSP control 3FF2 W = 4A0F  (S1_CONTROL_REG)
ADSP control 3FFF W = 0C08  (SYSCONTROL_REG)
*/

WRITE16_MEMBER(gaelco3d_state::adsp_control_w)
{
	if (LOG)
		logerror("ADSP control %04X W = %04X\n", 0x3fe0 + offset, data);

	m_adsp_control_regs[offset] = data;
	switch (offset)
	{
		case SYSCONTROL_REG:
			/* see if SPORT1 got disabled */
			if ((data & 0x0800) == 0)
			{
				dmadac_enable(&m_dmadac[0], SOUND_CHANNELS, 0);
				m_adsp_autobuffer_timer->reset();
			}
			break;

		case S1_AUTOBUF_REG:
			/* autobuffer off: nuke the timer, and disable the DAC */
			if ((data & 0x0002) == 0)
			{
				dmadac_enable(&m_dmadac[0], SOUND_CHANNELS, 0);
				m_adsp_autobuffer_timer->reset();
			}
			break;

		case S1_CONTROL_REG:
			if (((data >> 4) & 3) == 2)
				logerror("Oh no!, the data is compresed with u-law encoding\n");
			if (((data >> 4) & 3) == 3)
				logerror("Oh no!, the data is compresed with A-law encoding\n");
			break;
	}
}


WRITE16_MEMBER(gaelco3d_state::adsp_rombank_w)
{
	if (LOG)
		logerror("adsp_rombank_w(%d) = %04X\n", offset, data);
	membank("bank1")->set_entry((offset & 1) * 0x80 + (data & 0x7f));
}



/*************************************
 *
 *  ADSP sound generation
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(gaelco3d_state::adsp_autobuffer_irq)
{
	/* get the index register */
	int reg = m_adsp->state_int(ADSP2100_I0 + m_adsp_ireg);

	/* copy the current data into the buffer */
// logerror("ADSP buffer: I%d=%04X incs=%04X size=%04X\n", m_adsp_ireg, reg, m_adsp_incs, m_adsp_size);
	if (m_adsp_incs)
		dmadac_transfer(&m_dmadac[0], SOUND_CHANNELS, m_adsp_incs, SOUND_CHANNELS * m_adsp_incs, m_adsp_size / (SOUND_CHANNELS * m_adsp_incs), (INT16 *)&m_adsp_fastram_base[reg - 0x3800]);

	/* increment it */
	reg += m_adsp_size;

	/* check for wrapping */
	if (reg >= m_adsp_ireg_base + m_adsp_size)
	{
		/* reset the base pointer */
		reg = m_adsp_ireg_base;

		/* generate the (internal, thats why the pulse) irq */
		generic_pulse_irq_line(m_adsp, ADSP2105_IRQ1, 1);
	}

	/* store it */
	m_adsp->set_state_int(ADSP2100_I0 + m_adsp_ireg, reg);
}

WRITE32_MEMBER(gaelco3d_state::adsp_tx_callback)
{
	/* check if it's for SPORT1 */
	if (offset != 1)
		return;

	/* check if SPORT1 is enabled */
	if (m_adsp_control_regs[SYSCONTROL_REG] & 0x0800) /* bit 11 */
	{
		/* we only support autobuffer here (wich is what this thing uses), bail if not enabled */
		if (m_adsp_control_regs[S1_AUTOBUF_REG] & 0x0002) /* bit 1 */
		{
			/* get the autobuffer registers */
			int     mreg, lreg;
			UINT16  source;
			attotime sample_period;

			m_adsp_ireg = (m_adsp_control_regs[S1_AUTOBUF_REG] >> 9) & 7;
			mreg = (m_adsp_control_regs[S1_AUTOBUF_REG] >> 7) & 3;
			mreg |= m_adsp_ireg & 0x04; /* msb comes from ireg */
			lreg = m_adsp_ireg;

			/* now get the register contents in a more legible format */
			/* we depend on register indexes to be continuous (wich is the case in our core) */
			source = m_adsp->state_int(ADSP2100_I0 + m_adsp_ireg);
			m_adsp_incs = m_adsp->state_int(ADSP2100_M0 + mreg);
			m_adsp_size = m_adsp->state_int(ADSP2100_L0 + lreg);

			/* get the base value, since we need to keep it around for wrapping */
			source -= m_adsp_incs;

			/* make it go back one so we don't lose the first sample */
			m_adsp->set_state_int(ADSP2100_I0 + m_adsp_ireg, source);

			/* save it as it is now */
			m_adsp_ireg_base = source;

			/* calculate how long until we generate an interrupt */

			/* period per each bit sent */
			sample_period = attotime::from_hz(m_adsp->clock()) * (2 * (m_adsp_control_regs[S1_SCLKDIV_REG] + 1));

			/* now put it down to samples, so we know what the channel frequency has to be */
			sample_period *= 16 * SOUND_CHANNELS;

			dmadac_set_frequency(&m_dmadac[0], SOUND_CHANNELS, ATTOSECONDS_TO_HZ(sample_period.attoseconds()));
			dmadac_enable(&m_dmadac[0], SOUND_CHANNELS, 1);

			/* fire off a timer wich will hit every half-buffer */
			sample_period = (sample_period * m_adsp_size) / (SOUND_CHANNELS * m_adsp_incs);

			m_adsp_autobuffer_timer->adjust(sample_period, 0, sample_period);

			return;
		}
		else
			logerror( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	/* if we get there, something went wrong. Disable playing */
	dmadac_enable(&m_dmadac[0], SOUND_CHANNELS, 0);

	/* remove timer */
	m_adsp_autobuffer_timer->reset();
}



/*************************************
 *
 *  Unknown accesses
 *
 *************************************/


WRITE32_MEMBER(gaelco3d_state::radikalb_lamp_w)
{
	/* arbitrary data written */
	if (ACCESSING_BITS_0_7)
		logerror("%06X:unknown_127_w = %02X\n", space.device().safe_pc(), data & 0xff);
	else
		logerror("%06X:unknown_127_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}

WRITE32_MEMBER(gaelco3d_state::unknown_137_w)
{
	/* only written $00 or $ff */
	if (ACCESSING_BITS_0_7)
		logerror("%06X:unknown_137_w = %02X\n", space.device().safe_pc(), data & 0xff);
	else
		logerror("%06X:unknown_137_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}

WRITE32_MEMBER(gaelco3d_state::unknown_13a_w)
{
	/* only written $0000 or $0001 */
	if (ACCESSING_BITS_0_15)
		logerror("%06X:unknown_13a_w = %04X\n", space.device().safe_pc(), data & 0xffff);
	else
		logerror("%06X:unknown_13a_w(%02X) = %08X & %08X\n", space.device().safe_pc(), offset, data, mem_mask);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, gaelco3d_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(gaelco3d_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x51000c, 0x51000d) AM_READ_PORT("IN0")
	AM_RANGE(0x51001c, 0x51001d) AM_READ_PORT("IN1")
	AM_RANGE(0x51002c, 0x51002d) AM_READ_PORT("IN2")
	AM_RANGE(0x51003c, 0x51003d) AM_READ_PORT("IN3")
	AM_RANGE(0x510040, 0x510041) AM_WRITE(sound_data_w)
	AM_RANGE(0x510042, 0x510043) AM_READ(sound_status_r)
	AM_RANGE(0x510100, 0x510101) AM_READ(eeprom_data_r)
	AM_RANGE(0x510100, 0x510101) AM_WRITE(irq_ack_w)
	AM_RANGE(0x510102, 0x510103) AM_DEVWRITE8("serial", gaelco_serial_device, tr_w, 0x00ff)
	AM_RANGE(0x510102, 0x510103) AM_DEVREAD8("serial", gaelco_serial_device, data_r, 0x00ff)
	AM_RANGE(0x510104, 0x510105) AM_DEVWRITE8("serial", gaelco_serial_device, data_w, 0x00ff)
	AM_RANGE(0x51010a, 0x51010b) AM_DEVWRITE8("serial", gaelco_serial_device, rts_w, 0x00ff)
	AM_RANGE(0x510110, 0x510113) AM_WRITE(eeprom_data_w)
	AM_RANGE(0x510116, 0x510117) AM_WRITE(tms_control3_w)
	AM_RANGE(0x510118, 0x51011b) AM_WRITE(eeprom_clock_w)
	AM_RANGE(0x510120, 0x510123) AM_WRITE(eeprom_cs_w)
	AM_RANGE(0x51012a, 0x51012b) AM_WRITE(tms_reset_w)
	AM_RANGE(0x510132, 0x510133) AM_WRITE(tms_irq_w)
	AM_RANGE(0x510146, 0x510147) AM_DEVWRITE8("serial", gaelco_serial_device, irq_enable, 0x00ff)
	AM_RANGE(0x510156, 0x510157) AM_WRITE(analog_port_clock_w)
	AM_RANGE(0x510166, 0x510167) AM_WRITE(analog_port_latch_w)
	AM_RANGE(0x510176, 0x510177) AM_DEVWRITE8("serial", gaelco_serial_device, unknown_w, 0x00ff)
	AM_RANGE(0xfe7f80, 0xfe7fff) AM_WRITE(tms_comm_w) AM_SHARE("tms_comm_base")
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM AM_SHARE("m68k_ram_base")
ADDRESS_MAP_END


static ADDRESS_MAP_START( main020_map, AS_PROGRAM, 32, gaelco3d_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(gaelco3d_paletteram_020_w) AM_SHARE("paletteram")
	AM_RANGE(0x51000c, 0x51000f) AM_READ_PORT("IN0")
	AM_RANGE(0x51001c, 0x51001f) AM_READ_PORT("IN1")
	AM_RANGE(0x51002c, 0x51002f) AM_READ_PORT("IN2")
	AM_RANGE(0x51003c, 0x51003f) AM_READ_PORT("IN3")
	AM_RANGE(0x510040, 0x510043) AM_READ16(sound_status_r, 0x0000ffff)
	AM_RANGE(0x510040, 0x510043) AM_WRITE16(sound_data_w, 0xffff0000)
	AM_RANGE(0x510100, 0x510103) AM_READ(eeprom_data32_r)
	AM_RANGE(0x510100, 0x510103) AM_WRITE(irq_ack32_w)
	AM_RANGE(0x510104, 0x510107) AM_DEVWRITE8("serial", gaelco_serial_device, data_w, 0x00ff0000)
	AM_RANGE(0x510108, 0x51010b) AM_DEVWRITE8("serial", gaelco_serial_device, rts_w, 0x000000ff)
	AM_RANGE(0x510110, 0x510113) AM_WRITE16(eeprom_data_w, 0x0000ffff)
	AM_RANGE(0x510114, 0x510117) AM_WRITE16(tms_control3_w, 0x0000ffff)
	AM_RANGE(0x510118, 0x51011b) AM_WRITE16(eeprom_clock_w, 0x0000ffff)
	AM_RANGE(0x510120, 0x510123) AM_WRITE16(eeprom_cs_w, 0x0000ffff)
	AM_RANGE(0x510124, 0x510127) AM_WRITE(radikalb_lamp_w)
	AM_RANGE(0x510128, 0x51012b) AM_WRITE16(tms_reset_w, 0x0000ffff)
	AM_RANGE(0x510130, 0x510133) AM_WRITE16(tms_irq_w, 0x0000ffff)
	AM_RANGE(0x510134, 0x510137) AM_WRITE(unknown_137_w)
	AM_RANGE(0x510138, 0x51013b) AM_WRITE(unknown_13a_w)
	AM_RANGE(0x510144, 0x510147) AM_DEVWRITE8("serial", gaelco_serial_device, irq_enable, 0x000000ff)
	AM_RANGE(0x510154, 0x510157) AM_WRITE16(analog_port_clock_w, 0x0000ffff)
	AM_RANGE(0x510164, 0x510167) AM_WRITE16(analog_port_latch_w, 0x0000ffff)
	AM_RANGE(0x510174, 0x510177) AM_DEVWRITE8("serial", gaelco_serial_device, unknown_w, 0x000000ff)
	AM_RANGE(0xfe7f80, 0xfe7fff) AM_WRITE16(tms_comm_w, 0xffffffff) AM_SHARE("tms_comm_base")
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM AM_SHARE("m68k_ram_base")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tms_map, AS_PROGRAM, 32, gaelco3d_state )
	AM_RANGE(0x000000, 0x007fff) AM_READWRITE(tms_m68k_ram_r, tms_m68k_ram_w)
	AM_RANGE(0x400000, 0x5fffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xc00000, 0xc00007) AM_WRITE(gaelco3d_render_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( adsp_program_map, AS_PROGRAM, 32, gaelco3d_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("adsp_ram_base")       /* 1k words internal RAM */
	AM_RANGE(0x37ff, 0x37ff) AM_READNOP                         /* speedup hammers this for no apparent reason */
ADDRESS_MAP_END

static ADDRESS_MAP_START( adsp_data_map, AS_DATA, 16, gaelco3d_state )
	AM_RANGE(0x0000, 0x0001) AM_WRITE(adsp_rombank_w)
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x2000) AM_READWRITE(sound_data_r, sound_status_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM AM_SHARE("adsp_fastram")    /* 512 words internal RAM */
	AM_RANGE(0x3fe0, 0x3fff) AM_WRITE(adsp_control_w) AM_SHARE("adsp_regs")
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( speedup )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LSHIFT)    // view
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_LALT)      // brake
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE // gear (low=1 high=2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )       // start
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // verified
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // checked after reading analog from port 1
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)   // checked after reading analog from port 2
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)   // checked after reading analog from port 3
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)nullptr)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )        // verified
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )     // verified
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( surfplnt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VOLUME_UP )    // low two bits read, compared against 3
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )  // low four bits read, compared against f
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )      // checked
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )       // start
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )        // coin
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)nullptr)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
INPUT_PORTS_END


static INPUT_PORTS_START( radikalb )
	PORT_START("IN0")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )  // handle up
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 )      // view
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 )      // brake
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON1 )      // accel
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )       // start
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )        // coin
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08000000, IP_ACTIVE_LOW )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)nullptr)
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)1)
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)2)
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM_MEMBER(DEVICE_SELF, gaelco3d_state,analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( gaelco3d, gaelco3d_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 15000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco3d_state,  vblank_gen)

	MCFG_CPU_ADD("tms", TMS32031, 60000000)
	MCFG_CPU_PROGRAM_MAP(tms_map)
	MCFG_TMS3203X_MCBL(true)
	MCFG_TMS3203X_IACK_CB(WRITE8(gaelco3d_state, tms_iack_w))

	MCFG_CPU_ADD("adsp", ADSP2115, 16000000)
	MCFG_ADSP21XX_SPORT_TX_CB(WRITE32(gaelco3d_state, adsp_tx_callback))
	MCFG_CPU_PROGRAM_MAP(adsp_program_map)
	MCFG_CPU_DATA_MAP(adsp_data_map)


	MCFG_EEPROM_SERIAL_93C66_ADD("eeprom")

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_TIMER_DRIVER_ADD("adsp_timer", gaelco3d_state, adsp_autobuffer_irq)

	MCFG_DEVICE_ADD("serial", GAELCO_SERIAL, 0)
	MCFG_GAELCO_SERIAL_IRQ_HANDLER(WRITELINE(gaelco3d_state, ser_irq))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(576, 432)
	MCFG_SCREEN_VISIBLE_AREA(0, 575, 0, 431)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco3d_state, screen_update_gaelco3d)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRRGGGGGBBBBB("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac1", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)  /* speedup: front mono */

	MCFG_SOUND_ADD("dac2", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)  /* speedup: left rear */

	MCFG_SOUND_ADD("dac3", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)  /* speedup: right rear */

	MCFG_SOUND_ADD("dac4", DMADAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)  /* speedup: seat speaker */
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gaelco3d2, gaelco3d )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", M68EC020, 25000000)
	MCFG_CPU_PROGRAM_MAP(main020_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco3d_state,  vblank_gen)

	MCFG_CPU_MODIFY("tms")
	MCFG_CPU_CLOCK(50000000)

	MCFG_MACHINE_RESET_OVERRIDE(gaelco3d_state,gaelco3d2)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( speedup )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "sup10.bin", 0x000000, 0x80000, CRC(07e70bae) SHA1(17013d859ec075e12518b094040a056d850b3271) )
	ROM_LOAD16_BYTE( "sup15.bin", 0x000001, 0x80000, CRC(7947c28d) SHA1(46efb56d0f7fe2e92d0d04dcd2f130aef3be436d) )

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "sup25.bin", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "sup32.bin", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) )
	ROM_LOAD32_WORD( "sup33.bin", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "sup12.bin", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) )
	ROM_LOAD( "sup14.bin", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) )
	ROM_LOAD( "sup11.bin", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) )
	ROM_LOAD( "sup13.bin", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
	/* these 2 are copies of the previous 2 */
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
ROM_END


ROM_START( speedup10 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ic10_1.00", 0x000000, 0x80000, CRC(24ed8f48) SHA1(59d59e2a0b2fb7aed5320167960129819adedd9a) ) /* hand written labels IC10 1.00 */
	ROM_LOAD16_BYTE( "ic15_1.00", 0x000001, 0x80000, CRC(b3fda7f1) SHA1(e77ef3cb46be0767476f65dcc8d4fc12550be4a3) ) /* hand written labels IC50 1.00 */

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "sup25.bin", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "sup32.bin", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) )
	ROM_LOAD32_WORD( "sup33.bin", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "sup12.bin", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) )
	ROM_LOAD( "sup14.bin", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) )
	ROM_LOAD( "sup11.bin", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) )
	ROM_LOAD( "sup13.bin", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
	/* these 2 are copies of the previous 2 */
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
ROM_END


ROM_START( surfplnt )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "surfplnt.u5",  0x000000, 0x80000, CRC(c96e0a18) SHA1(b313d02d1d1bff8717b3d798e6ae681baefc1061) )
	ROM_LOAD16_BYTE( "surfplnt.u11", 0x000001, 0x80000, CRC(99211d2d) SHA1(dee5b157489ce9c6988c8eec92fa91fff60d521c) )
	ROM_LOAD16_BYTE( "surfplnt.u8",  0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "surfplnt.u13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "pls.18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "pls.40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) )
	ROM_LOAD32_WORD( "pls.37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "pls.7",  0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) )
	ROM_LOAD( "pls.9",  0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) )
	ROM_LOAD( "pls.12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) )
	ROM_LOAD( "pls.15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "surfplnt.u19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "surfplnt.u20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "surfplnt.u21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "surfplnt.u22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "surfplnt.u27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
//  ROM_LOAD( "surfplnt.u28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
//  ROM_LOAD( "surfplnt.u29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
//  ROM_LOAD( "surfplnt.u30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
ROM_END


ROM_START( surfplnt40 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "surfpl40.u5",  0x000000, 0x80000, CRC(572e0343) SHA1(badb08a5a495611b5fd2d821d4299348b2c9f308) )
	ROM_LOAD16_BYTE( "surfpl40.u11", 0x000001, 0x80000, CRC(6056edaa) SHA1(9bc2df54d1367b9d58272a8f506e523e74110361) )
	ROM_LOAD16_BYTE( "surfplnt.u8",  0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "surfplnt.u13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "pls.18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "pls.40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) )
	ROM_LOAD32_WORD( "pls.37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "pls.7",  0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) )
	ROM_LOAD( "pls.9",  0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) )
	ROM_LOAD( "pls.12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) )
	ROM_LOAD( "pls.15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "surfplnt.u19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "surfplnt.u20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "surfplnt.u21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "surfplnt.u22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "surfplnt.u27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
//  ROM_LOAD( "surfplnt.u28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
//  ROM_LOAD( "surfplnt.u29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
//  ROM_LOAD( "surfplnt.u30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
ROM_END


ROM_START( radikalb )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68020 code */
	ROM_LOAD32_BYTE( "rab.6",  0x000000, 0x80000, CRC(ccac98c5) SHA1(43a30caf9880f48aba79676f9e746fdc6258139d) )
	ROM_LOAD32_BYTE( "rab.12", 0x000001, 0x80000, CRC(26199506) SHA1(1b7b44895aa296eab8061ae85cbb5b0d30119dc7) )
	ROM_LOAD32_BYTE( "rab.14", 0x000002, 0x80000, CRC(4a0ac8cb) SHA1(4883e5eddb833dcd39376be435aa8e8e2ec47ab5) )
	ROM_LOAD32_BYTE( "rab19.ic19", 0x000003, 0x80000, CRC(c2d4fcb2) SHA1(8e389d1479ba084e5363aef9c797c65ca7f355d2) )

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "rab.23", 0x0000000, 0x400000, CRC(dcf52520) SHA1(ab54421c182436660d2a56a334c1aa335424644a) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "rab.48", 0x000000, 0x400000, CRC(9c56a06a) SHA1(54f12d8b55fa14446c47e31684c92074c4157fe1) )
	ROM_LOAD32_WORD( "rab.45", 0x000002, 0x400000, CRC(7e698584) SHA1(a9423835a126396902c499e9f7df3b68c2ab28a8) )

	ROM_REGION( 0x2000000, "gfx1", 0 )
	ROM_LOAD( "rab.8",  0x0000000, 0x400000, CRC(4fbd4737) SHA1(594438d3edbe00682290986cc631615d7bef67f3) )
	ROM_LOAD( "rab.10", 0x0800000, 0x400000, CRC(870b0ce4) SHA1(75910dca87d2eb3a6b4a28f6e9c63a6b6700de84) )
	ROM_LOAD( "rab.15", 0x1000000, 0x400000, CRC(edb9d409) SHA1(1f8df507e990eee197f2779b45bd8f143d1bd439) )
	ROM_LOAD( "rab.17", 0x1800000, 0x400000, CRC(e120236b) SHA1(37d7996530eda3df0c19bca1cbf26e5ba58b0977) )

	ROM_LOAD( "rab.9",  0x0400000, 0x400000, CRC(9e3e038d) SHA1(4a5f0b3c54c508d7f310f270dbd11bffb2bcfa89) )
	ROM_LOAD( "rab.11", 0x0c00000, 0x400000, CRC(75672271) SHA1(ebbdf089b4a4d5ead7d62555bb5c9a921aaa1c22) )
	ROM_LOAD( "rab.16", 0x1400000, 0x400000, CRC(9d595e46) SHA1(b985332974e1fb0b9d20d521da0d7deceea93a8a) )
	ROM_LOAD( "rab.18", 0x1c00000, 0x400000, CRC(3084bc49) SHA1(9da43482293eeb08ceae67455b2fcd97b6ef5109) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "rab.24", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
	ROM_LOAD( "rab.25", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
	ROM_LOAD( "rab.26", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
	ROM_LOAD( "rab.27", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "rab.32", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
//  ROM_LOAD( "rab.33", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
//  ROM_LOAD( "rab.34", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
//  ROM_LOAD( "rab.35", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
ROM_END



ROM_START( radikalba )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 68020 code */
	ROM_LOAD32_BYTE( "rab.6",  0x000000, 0x80000, CRC(ccac98c5) SHA1(43a30caf9880f48aba79676f9e746fdc6258139d) )
	ROM_LOAD32_BYTE( "rab.12", 0x000001, 0x80000, CRC(26199506) SHA1(1b7b44895aa296eab8061ae85cbb5b0d30119dc7) )
	ROM_LOAD32_BYTE( "rab.14", 0x000002, 0x80000, CRC(4a0ac8cb) SHA1(4883e5eddb833dcd39376be435aa8e8e2ec47ab5) )
	ROM_LOAD32_BYTE( "rab.19", 0x000003, 0x80000, CRC(2631bd61) SHA1(57331ad49e7284b82073f696049de109b7683b03) )

	ROM_REGION16_LE( 0x400000, "user1", 0 ) /* ADSP-2115 code & data */
	ROM_LOAD( "rab.23", 0x0000000, 0x400000, CRC(dcf52520) SHA1(ab54421c182436660d2a56a334c1aa335424644a) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "rab.48", 0x000000, 0x400000, CRC(9c56a06a) SHA1(54f12d8b55fa14446c47e31684c92074c4157fe1) )
	ROM_LOAD32_WORD( "rab.45", 0x000002, 0x400000, CRC(7e698584) SHA1(a9423835a126396902c499e9f7df3b68c2ab28a8) )

	ROM_REGION( 0x2000000, "gfx1", 0 )
	ROM_LOAD( "rab.8",  0x0000000, 0x400000, CRC(4fbd4737) SHA1(594438d3edbe00682290986cc631615d7bef67f3) )
	ROM_LOAD( "rab.10", 0x0800000, 0x400000, CRC(870b0ce4) SHA1(75910dca87d2eb3a6b4a28f6e9c63a6b6700de84) )
	ROM_LOAD( "rab.15", 0x1000000, 0x400000, CRC(edb9d409) SHA1(1f8df507e990eee197f2779b45bd8f143d1bd439) )
	ROM_LOAD( "rab.17", 0x1800000, 0x400000, CRC(e120236b) SHA1(37d7996530eda3df0c19bca1cbf26e5ba58b0977) )

	ROM_LOAD( "rab.9",  0x0400000, 0x400000, CRC(9e3e038d) SHA1(4a5f0b3c54c508d7f310f270dbd11bffb2bcfa89) )
	ROM_LOAD( "rab.11", 0x0c00000, 0x400000, CRC(75672271) SHA1(ebbdf089b4a4d5ead7d62555bb5c9a921aaa1c22) )
	ROM_LOAD( "rab.16", 0x1400000, 0x400000, CRC(9d595e46) SHA1(b985332974e1fb0b9d20d521da0d7deceea93a8a) )
	ROM_LOAD( "rab.18", 0x1c00000, 0x400000, CRC(3084bc49) SHA1(9da43482293eeb08ceae67455b2fcd97b6ef5109) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "rab.24", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
	ROM_LOAD( "rab.25", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
	ROM_LOAD( "rab.26", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
	ROM_LOAD( "rab.27", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "rab.32", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
//  ROM_LOAD( "rab.33", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
//  ROM_LOAD( "rab.34", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
//  ROM_LOAD( "rab.35", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
ROM_END


/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(gaelco3d_state,gaelco3d)
{
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1996, speedup,    0,        gaelco3d,  speedup,  gaelco3d_state, gaelco3d, ROT0, "Gaelco", "Speed Up (Version 1.20)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, speedup10,  speedup,  gaelco3d,  speedup,  gaelco3d_state, gaelco3d, ROT0, "Gaelco", "Speed Up (Version 1.00)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

GAME( 1997, surfplnt,   0,        gaelco3d,  surfplnt, gaelco3d_state, gaelco3d, ROT0, "Gaelco", "Surf Planet (Version 4.1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1997, surfplnt40, surfplnt, gaelco3d,  surfplnt, gaelco3d_state, gaelco3d, ROT0, "Gaelco", "Surf Planet (Version 4.0)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

GAME( 1998, radikalb,   0,        gaelco3d2, radikalb, gaelco3d_state, gaelco3d, ROT0, "Gaelco",                 "Radikal Bikers (Version 2.02)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1998, radikalba,  radikalb, gaelco3d2, radikalb, gaelco3d_state, gaelco3d, ROT0, "Gaelco (Atari license)", "Radikal Bikers (Version 2.02, Atari license)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
