// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

    Games supported:
        * Speed Up
        * Surf Planet
        * Radikal Bikers
        * Football Power

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
      Some versions are known to use REF. 970514 board (looks identical)

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
#include "gaelco3d.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"

#include "emupal.h"
#include "speaker.h"

#include "speedup.lh"


#define LOG_EEPROM          (1U << 1)
#define LOG_SOUND           (1U << 2)
#define LOG_TMS             (1U << 3)
#define LOG_ADSP            (1U << 4)

#define VERBOSE 0

#include "logmacro.h"

#define LOGEEPROM(...)      LOGMASKED(LOG_EEPROM, __VA_ARGS__)
#define LOGSOUND(...)       LOGMASKED(LOG_SOUND, __VA_ARGS__)
#define LOGTMS(...)         LOGMASKED(LOG_TMS, __VA_ARGS__)
#define LOGADSP(...)        LOGMASKED(LOG_ADSP, __VA_ARGS__)


void gaelco3d_state::ser_irq(int state)
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
	m_adsp_bank->configure_entries(0, 256, memregion("adsprom")->base(), 0x4000);

	// Save state support
	save_item(NAME(m_sound_status));
	save_item(NAME(m_analog_ports));
	save_item(NAME(m_adsp_ireg));
	save_item(NAME(m_adsp_ireg_base));
	save_item(NAME(m_adsp_incs));
	save_item(NAME(m_adsp_size));
	save_item(NAME(m_fp_clock));
	save_item(NAME(m_fp_state));
	save_item(NAME(m_fp_analog_ports));
	save_item(NAME(m_fp_length));
}


MACHINE_RESET_MEMBER(gaelco3d_state,common)
{
	// Boot the ADSP chip
	uint16_t const *const src = (uint16_t *)memregion("adsprom")->base();
	for (int i = 0; i < (src[3] & 0xff) * 8; i++)
	{
		uint32_t const opcode = ((src[i * 4 + 0] & 0xff) << 16) | ((src[i * 4 + 1] & 0xff) << 8) | (src[i * 4 + 2] & 0xff);
		m_adsp_ram_base[i] = opcode;
	}

	m_adsp_bank->set_entry(0);

	// Keep the TMS32031 halted until the code is ready to go
	m_tms->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


void gaelco3d_state::machine_reset()
{
	MACHINE_RESET_CALL_MEMBER(common);
	m_soundlatch->acknowledge_w();
}


MACHINE_RESET_MEMBER(gaelco3d_state,gaelco3d2)
{
	MACHINE_RESET_CALL_MEMBER(common);
	m_fp_clock = 27;
	m_fp_state = 0;
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


void gaelco3d_state::irq_ack_w(uint16_t data)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}


/*************************************
 *
 *  EEPROM (93C66B)
 *  Serial Interface
 *
 *************************************/

uint16_t gaelco3d_state::eeprom_data_r(offs_t offset, uint16_t mem_mask)
{
	uint32_t result = 0xffff;

	if (ACCESSING_BITS_0_7)
	{
		// bit 0 is clock
		// bit 1 active
		result &= ~uint32_t(gaelco_serial_device::EXT_STATUS_MASK);
		result |= m_serial->status_r();
	}

	if (m_eeprom->do_read())
		result ^= 0x0004;
	if (!machine().side_effects_disabled())
		LOGEEPROM("eeprom_data_r(%02X)\n", result);
	return result;
}



/*************************************
 *
 *  Sound CPU comms
 *
 *************************************/

uint16_t gaelco3d_state::sound_status_r(offs_t offset, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOGSOUND("%s:sound_status_r(%02X) = %02X\n", machine().describe_context(), offset, m_sound_status);
	if (ACCESSING_BITS_0_7)
		return m_sound_status;
	return 0xffff;
}


void gaelco3d_state::sound_status_w(uint16_t data)
{
	LOGSOUND("sound_status_w(%02X)\n", m_sound_status);
	m_sound_status = data;
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

template <int N>
int gaelco3d_state::analog_bit_r()
{
	return BIT(m_analog_ports[N], 7);
}


void gaelco3d_state::analog_port_clock_w(int state)
{
	// A zero/one combo is written here to clock the next analog port bit
	if (!state)
	{
		m_analog_ports[0] <<= 1;
		m_analog_ports[1] <<= 1;
		m_analog_ports[2] <<= 1;
		m_analog_ports[3] <<= 1;
	}
}


void gaelco3d_state::analog_port_latch_w(int state)
{
	// A zero is written here to read the analog ports, and a one is written when finished
	if (!state)
	{
		m_analog_ports[0] = m_analog[0].read_safe(0);
		m_analog_ports[1] = m_analog[1].read_safe(0);
		m_analog_ports[2] = m_analog[2].read_safe(0);
		m_analog_ports[3] = m_analog[3].read_safe(0);
	}
}

template <int N>
int gaelco3d_state::fp_analog_bit_r()
{
	return BIT(m_fp_analog_ports[N], m_fp_clock);
}

void gaelco3d_state::fp_analog_clock_w(int state)
{
	if (state != m_fp_state)
	{
		m_fp_state = state;
		m_fp_clock++;
		if (m_fp_clock == 28)
		{
			m_fp_clock = 0;
			for (int i = 0; i < 2; i++)
			{
				u32 const ay = m_analog[i * 2].read_safe(0);
				u32 const ax = m_analog[i * 2 + 1].read_safe(0);
				m_fp_analog_ports[i] = (ax << 18) | ((ax ^ 0xff) << 10) | (ay << 2) | 1;
				s32 const aay = ay - 0x80;
				s32 const aax = ax - 0x80;
				u32 const len = aay * aay + aax * aax;
				if (len <= m_fp_length[i])
					m_fp_analog_ports[i] |= 2;
				m_fp_length[i] = len;
			}
		}
	}
}


/*************************************
 *
 *  TMS32031 interface
 *
 *************************************/

uint32_t gaelco3d_state::tms_m68k_ram_r(offs_t offset)
{
	//if (!machine().side_effects_disabled())
		//LOGTMS("%s:tms_m68k_ram_r(%04X) = %08X\n", machine().describe_context(), offset, !(offset & 1) ? ((int32_t)m_m68k_ram_base[offset/2] >> 16) : (int)(int16_t)m_m68k_ram_base[offset/2]);
	if (m_m68k_ram_base16)
		return (int32_t)(int16_t)m_m68k_ram_base16[offset];
	else if (offset & 1)
		return (int32_t)(int16_t)m_m68k_ram_base32[offset >> 1];
	else
		return (int32_t)(int16_t)(m_m68k_ram_base32[offset >> 1] >> 16);
}


void gaelco3d_state::tms_m68k_ram_w(offs_t offset, uint32_t data)
{
	if (m_m68k_ram_base16)
		m_m68k_ram_base16[offset] = data;
	else if (offset & 1)
		m_m68k_ram_base32[offset >> 1] = (m_m68k_ram_base32[offset >> 1] & 0xffff0000) | (data & 0xffff);
	else
		m_m68k_ram_base32[offset >> 1] = (m_m68k_ram_base32[offset >> 1] & 0xffff) | (data << 16);
}


void gaelco3d_state::tms_iack_w(offs_t offset, uint8_t data)
{
	LOGTMS("iack_w(%d) - %06X\n", data, offset);
	m_tms->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  TMS32031 control
 *
 *************************************/

void gaelco3d_state::tms_reset_w(int state)
{
	/* this is set to 0 while data is uploaded, then set to $ffff after it is done.
	   It does not ever appear to be touched after that */
	LOGTMS("%06X:tms_reset_w = %d\n", m_maincpu->pc(), state);
	m_tms->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
}


void gaelco3d_state::tms_irq_w(int state)
{
	/* This is written twice, 0,1, in quick succession.
	   Done after uploading, and after modifying the comm area */
	LOGTMS("%06X:tms_irq_w = %d\n", m_maincpu->pc(), state);
	m_tms->set_input_line(0, state ? CLEAR_LINE : ASSERT_LINE);
}


void gaelco3d_state::tms_control3_w(int state)
{
	LOGTMS("%06X:tms_control3_w = %d\n", m_maincpu->pc(), state);
}

/*************************************
 *
 *  ADSP control registers
 *
 *************************************/

// These are some of the control registers. We don't use them all
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

void gaelco3d_state::adsp_control_w(offs_t offset, uint16_t data)
{
	LOGADSP("ADSP control %04X W = %04X\n", 0x3fe0 + offset, data);

	m_adsp_control_regs[offset] = data;
	switch (offset)
	{
		case SYSCONTROL_REG:
			// See if SPORT1 got disabled
			if (BIT(~data, 11))
			{
				for (uint8_t i = 0; i < SOUND_CHANNELS; i++)
					m_dmadac[i]->enable(0);

				m_adsp_autobuffer_timer->reset();
			}
			break;

		case S1_AUTOBUF_REG:
			// Autobuffer off: nuke the timer, and disable the DAC
			if (BIT(~data, 1))
			{
				for (uint8_t i = 0; i < SOUND_CHANNELS; i++)
					m_dmadac[i]->enable(0);

				m_adsp_autobuffer_timer->reset();
			}
			break;

		case S1_CONTROL_REG:
			if (((data >> 4) & 3) == 2)
				LOGADSP("Oh no!, the data is compressed with u-law encoding\n");
			if (((data >> 4) & 3) == 3)
				LOGADSP("Oh no!, the data is compressed with A-law encoding\n");
			break;
	}
}


void gaelco3d_state::adsp_rombank_w(offs_t offset, uint16_t data)
{
	LOGADSP("adsp_rombank_w(%d) = %04X\n", offset, data);
	m_adsp_bank->set_entry((offset & 1) * 0x80 + (data & 0x7f));
}



/*************************************
 *
 *  ADSP sound generation
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(gaelco3d_state::adsp_autobuffer_irq)
{
	// Get the index register
	int reg = m_adsp->state_int(ADSP2100_I0 + m_adsp_ireg);

	// Copy the current data into the buffer
// LOGADSP("ADSP buffer: I%d=%04X incs=%04X size=%04X\n", m_adsp_ireg, reg, m_adsp_incs, m_adsp_size);
	if (m_adsp_incs)
	{
		for (uint8_t i = 0; i < SOUND_CHANNELS; i++)
			m_dmadac[i]->transfer(i, m_adsp_incs, SOUND_CHANNELS * m_adsp_incs, m_adsp_size / (SOUND_CHANNELS * m_adsp_incs), (int16_t *)&m_adsp_fastram_base[(reg - 0x3800) / 2]);
	}

	// Increment it
	reg += m_adsp_size;

	// Check for wrapping
	if (reg >= m_adsp_ireg_base + m_adsp_size)
	{
		// Reset the base pointer
		reg = m_adsp_ireg_base;

		// Generate the (internal, thats why the pulse) IRQ
		m_adsp->pulse_input_line(ADSP2105_IRQ1, m_adsp->minimum_quantum_time());
	}

	// Store it
	m_adsp->set_state_int(ADSP2100_I0 + m_adsp_ireg, reg);
}

void gaelco3d_state::adsp_tx_callback(offs_t offset, uint32_t data)
{
	// Check if it's for SPORT1
	if (offset != 1)
		return;

	// Check if SPORT1 is enabled
	if (BIT(m_adsp_control_regs[SYSCONTROL_REG], 11)) // bit 11
	{
		// We only support autobuffer here (which is what this thing uses), bail if not enabled
		if (BIT(m_adsp_control_regs[S1_AUTOBUF_REG], 1)) // bit 1
		{
			// Get the autobuffer registers
			m_adsp_ireg = (m_adsp_control_regs[S1_AUTOBUF_REG] >> 9) & 7;
			int mreg = (m_adsp_control_regs[S1_AUTOBUF_REG] >> 7) & 3;
			mreg |= m_adsp_ireg & 0x04; // msb comes from ireg
			int const lreg = m_adsp_ireg;

			/* Now get the register contents in a more legible format.
			   We depend on register indexes to be continuous (which is the case in our core) */
			uint16_t source = m_adsp->state_int(ADSP2100_I0 + m_adsp_ireg);
			m_adsp_incs = m_adsp->state_int(ADSP2100_M0 + mreg);
			m_adsp_size = m_adsp->state_int(ADSP2100_L0 + lreg);

			// Get the base value, since we need to keep it around for wrapping
			source -= m_adsp_incs;

			// Make it go back one so we don't lose the first sample
			m_adsp->set_state_int(ADSP2100_I0 + m_adsp_ireg, source);

			// Save it as it is now
			m_adsp_ireg_base = source;

			// Calculate how long until we generate an interrupt

			// Period per each bit sent
			attotime sample_period = attotime::from_hz(m_adsp->clock()) * (2 * (m_adsp_control_regs[S1_SCLKDIV_REG] + 1));

			// Now put it down to samples, so we know what the channel frequency has to be
			sample_period *= 16 * SOUND_CHANNELS;

			for (uint8_t i = 0; i < SOUND_CHANNELS; i++)
			{
				m_dmadac[i]->set_frequency(sample_period.as_hz());
				m_dmadac[i]->enable(1);
			}

			// Fire off a timer which will hit every half-buffer
			sample_period = (sample_period * m_adsp_size) / (SOUND_CHANNELS * m_adsp_incs);

			m_adsp_autobuffer_timer->adjust(sample_period, 0, sample_period);

			return;
		}
		else
			LOGADSP( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	// If we get there, something went wrong. Disable playing
	for (uint8_t i = 0; i < SOUND_CHANNELS; i++)
		m_dmadac[i]->enable(0);

	// Remove timer
	m_adsp_autobuffer_timer->reset();
}



/*************************************
 *
 *  Unknown accesses
 *
 *************************************/

void gaelco3d_state::unknown_137_w(int state)
{
	// Only written $00 or $ff
	LOGADSP("%06X:unknown_137_w = %d\n", m_maincpu->pc(), state);
}

void gaelco3d_state::unknown_13a_w(int state)
{
	// Only written $0000 or $0001
	LOGADSP("%06X:unknown_13a_w = %04X\n", m_maincpu->pc(), state);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void gaelco3d_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).rom();
	map(0x400000, 0x40ffff).ram().w(FUNC(gaelco3d_state::paletteram_w)).share(m_paletteram16);
	map(0x51000c, 0x51000d).portr("IN0");
	map(0x51001c, 0x51001d).portr("IN1");
	map(0x51002c, 0x51002d).portr("IN2");
	map(0x51003c, 0x51003d).portr("IN3");
	map(0x510041, 0x510041).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x510042, 0x510043).r(FUNC(gaelco3d_state::sound_status_r));
	map(0x510044, 0x510045).nopw(); // unknown (cabinet control?)
	map(0x510100, 0x510101).rw(FUNC(gaelco3d_state::eeprom_data_r), FUNC(gaelco3d_state::irq_ack_w));
	map(0x510103, 0x510103).r(m_serial, FUNC(gaelco_serial_device::data_r));
	map(0x510103, 0x510103).select(0x000038).lw8(NAME([this] (offs_t offset, u8 data) { m_mainlatch->write_d0(offset >> 3, data); }));
	map(0x510105, 0x510105).w(m_serial, FUNC(gaelco_serial_device::data_w));
	map(0x510106, 0x510107).mirror(0x000070).nopr(); // clr.b instructions do dummy reads
	map(0x510107, 0x510107).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0xfe0000, 0xfeffff).ram().share(m_m68k_ram_base16);
}


void gaelco3d_state::main020_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x400000, 0x40ffff).ram().w(FUNC(gaelco3d_state::paletteram_020_w)).share(m_paletteram32);
	map(0x51000c, 0x51000f).portr("IN0");
	map(0x51001c, 0x51001f).portr("IN1");
	map(0x51002c, 0x51002f).portr("IN2");
	map(0x51003c, 0x51003f).portr("IN3");
	map(0x510041, 0x510041).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x510042, 0x510043).r(FUNC(gaelco3d_state::sound_status_r));
	map(0x510100, 0x510101).rw(FUNC(gaelco3d_state::eeprom_data_r), FUNC(gaelco3d_state::irq_ack_w));
	map(0x510103, 0x510103).r(m_serial, FUNC(gaelco_serial_device::data_r));
	map(0x510103, 0x510103).select(0x000038).lw8(NAME([this] (offs_t offset, u8 data) { m_mainlatch->write_d0(offset >> 3, data); }));
	map(0x510105, 0x510105).w(m_serial, FUNC(gaelco_serial_device::data_w));
	map(0x510107, 0x510107).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0xfe0000, 0xfeffff).ram().share(m_m68k_ram_base32);
}

void gaelco3d_state::tms_map(address_map &map)
{
	map(0x000000, 0x007fff).rw(FUNC(gaelco3d_state::tms_m68k_ram_r), FUNC(gaelco3d_state::tms_m68k_ram_w));
	map(0x400000, 0x7fffff).rom().region("tmsrom", 0);
	map(0xc00000, 0xc00007).w(FUNC(gaelco3d_state::render_w));
}


void gaelco3d_state::adsp_program_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share(m_adsp_ram_base);   // 1k words internal RAM
	map(0x37ff, 0x37ff).nopr();                         // speedup hammers this for no apparent reason
}

void gaelco3d_state::adsp_data_map(address_map &map)
{
	map(0x0000, 0x0001).w(FUNC(gaelco3d_state::adsp_rombank_w));
	map(0x0000, 0x1fff).bankr(m_adsp_bank);
	map(0x2000, 0x2000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).umask16(0x00ff);
	map(0x2000, 0x2000).w(FUNC(gaelco3d_state::sound_status_w));
	map(0x3800, 0x39ff).ram().share(m_adsp_fastram_base);    // 512 words internal RAM
	map(0x3fe0, 0x3fff).w(FUNC(gaelco3d_state::adsp_control_w)).share(m_adsp_control_regs);
}



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
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<0>))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<1>))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<2>))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<3>))

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
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<0>))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<1>))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<2>))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<3>))

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
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<0>))
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<1>))
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<2>))
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::analog_bit_r<3>))

	PORT_START("IN3")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
INPUT_PORTS_END

static INPUT_PORTS_START( footbpow )
	PORT_START("IN0")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "P1 Dribbling" )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "P1 Defense" )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "P2 Dribbling" )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME( "P2 Defense" )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )        // coin
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08000000, IP_ACTIVE_LOW )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::fp_analog_bit_r<1>))
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(gaelco3d_state::fp_analog_bit_r<0>))
	PORT_BIT( 0xc0000000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_REVERSE PORT_SENSITIVITY(25) PORT_KEYDELTA(50)

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(50)

	PORT_START("ANALOG2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2) PORT_REVERSE PORT_SENSITIVITY(25) PORT_KEYDELTA(50)

	PORT_START("ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2) PORT_SENSITIVITY(25) PORT_KEYDELTA(50)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void gaelco3d_state::gaelco3d(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, 15000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco3d_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco3d_state::vblank_gen));

	TMS32031(config, m_tms, 60000000);
	m_tms->set_addrmap(AS_PROGRAM, &gaelco3d_state::tms_map);
	m_tms->set_mcbl_mode(true);
	m_tms->iack().set(FUNC(gaelco3d_state::tms_iack_w));

	ADSP2115(config, m_adsp, 16000000);
	m_adsp->sport_tx().set(FUNC(gaelco3d_state::adsp_tx_callback));
	m_adsp->set_addrmap(AS_PROGRAM, &gaelco3d_state::adsp_program_map);
	m_adsp->set_addrmap(AS_DATA, &gaelco3d_state::adsp_data_map);

	EEPROM_93C66_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	config.set_maximum_quantum(attotime::from_hz(6000));

	TIMER(config, m_adsp_autobuffer_timer).configure_generic(FUNC(gaelco3d_state::adsp_autobuffer_irq));

	GAELCO_SERIAL(config, m_serial, 0);
	m_serial->irq_handler().set(FUNC(gaelco3d_state::ser_irq));

	LS259(config, m_mainlatch); // IC5 on bottom board next to EEPROM
	m_mainlatch->q_out_cb<0>().set(m_serial, FUNC(gaelco_serial_device::tr_w));
	m_mainlatch->q_out_cb<1>().set(m_serial, FUNC(gaelco_serial_device::rts_w));
	m_mainlatch->q_out_cb<2>().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write));
	m_mainlatch->q_out_cb<3>().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write));
	m_mainlatch->q_out_cb<4>().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write));
	m_mainlatch->q_out_cb<5>().set(FUNC(gaelco3d_state::tms_reset_w));
	m_mainlatch->q_out_cb<6>().set(FUNC(gaelco3d_state::tms_irq_w));
	m_mainlatch->q_out_cb<7>().set(FUNC(gaelco3d_state::unknown_13a_w));

	LS259(config, m_outlatch); // IC2 on top board near edge connector
	m_outlatch->q_out_cb<1>().set(FUNC(gaelco3d_state::tms_control3_w));
	m_outlatch->q_out_cb<2>().set_output("Start_lamp"); // START LAMP
	m_outlatch->q_out_cb<3>().set(FUNC(gaelco3d_state::unknown_137_w));
	m_outlatch->q_out_cb<4>().set(m_serial, FUNC(gaelco_serial_device::irq_enable));
	m_outlatch->q_out_cb<5>().set(FUNC(gaelco3d_state::analog_port_clock_w));
	m_outlatch->q_out_cb<6>().set(FUNC(gaelco3d_state::analog_port_latch_w));
	m_outlatch->q_out_cb<7>().set(m_serial, FUNC(gaelco_serial_device::unknown_w));

	GENERIC_LATCH_8(config, m_soundlatch).data_pending_callback().set_inputline(m_adsp, ADSP2115_IRQ2);

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	m_screen->set_size(576, 432);
	m_screen->set_visarea(0, 575, 0, 431);
	m_screen->set_screen_update(FUNC(gaelco3d_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_555);

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	DMADAC(config, m_dmadac[0]).add_route(ALL_OUTPUTS, "mono", 0.45);  // speedup: front mono
	DMADAC(config, m_dmadac[1]).add_route(ALL_OUTPUTS, "mono", 0.45);  // speedup: left rear
	DMADAC(config, m_dmadac[2]).add_route(ALL_OUTPUTS, "mono", 0.45);  // speedup: right rear
	DMADAC(config, m_dmadac[3]).add_route(ALL_OUTPUTS, "mono", 0.45);  // speedup: seat speaker
}


void gaelco3d_state::gaelco3d2(machine_config &config)
{
	gaelco3d(config);

	// Basic machine hardware
	M68EC020(config.replace(), m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco3d_state::main020_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco3d_state::vblank_gen));

	m_tms->set_clock(50000000);

	MCFG_MACHINE_RESET_OVERRIDE(gaelco3d_state,gaelco3d2)
}

void gaelco3d_state::footbpow(machine_config &config)
{
	gaelco3d2(config);
	m_outlatch->q_out_cb<5>().set(FUNC(gaelco3d_state::fp_analog_clock_w));
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
Gaelco was notorious for using the same ROM labels even when ROM data changed. This was common for earlier games like Biomechanical Toy and
 the same applies for the various versions of the games listed below. The only way to know / verify the game version is by the boot up screen.
 Occasionally PCBs have been observed with a handwritten version on the labels, but it's unknown if it's a factory or operator addition.
*/

/*
Speed Up uses a small external PCB with a PIC for credit distribution:
  ________________________
 |         ____  ____    |
 |         ····  ····    |
 |  ______   _________ ·||
 | LM340T5  |PIC16C54| ·||
 |                 ___ ·||
 |     _________  |  | ·||
 |    |ULN2003A1| |  |   |
 | ___            |  <-74HCT245N
 ||  |  _________ |__|   |
 ||  | |_DIPSx8_|      ·||
 ||  <-CD74HCT174E     ·||
 ||__|  _________      ·||
 |     |74HCT245N      ·||
 |_______________________|

Manuals with dip switches setup and schematics can be downloaded from https://www.recreativas.org/manuales/videojuegos
*/

ROM_START( speedup ) // Version 2.20 - REF. 960717 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "sup_2.2_10.ic10", 0x000000, 0x80000, CRC(ee781e64) SHA1(d90fa9319982fa389c2032e13d59850971078006) ) // 2.2 is handwritten between SUP and 10
	ROM_LOAD16_BYTE( "sup_2.2_15.ic15", 0x000001, 0x80000, CRC(1b8ff9d2) SHA1(4939b45844de962d2b93be058b44c09e366cf8db) ) // 2.2 is handwritten between SUP and 15

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "sup_ic25.ic25", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) ) // designation silkscreend on mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "sup_ic32.ic32", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) ) // designation silkscreend on mask ROM
	ROM_LOAD32_WORD( "sup_ic33.ic33", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "sup_ic12.ic12", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic14.ic14", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic11.ic11", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic13.ic13", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label
	// These 2 are copies of the previous 2 at different IC locations
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label

	ROM_REGION( 0x2000, "coin", 0 ) // Credit distribution PCB
	ROM_LOAD( "2x1c_pic16c54.u1", 0x0000, 0x2000, NO_DUMP )
ROM_END

ROM_START( speedup12 ) // Version 1.20 - REF. 960717 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "sup_10.ic10", 0x000000, 0x80000, CRC(07e70bae) SHA1(17013d859ec075e12518b094040a056d850b3271) )
	ROM_LOAD16_BYTE( "sup_15.ic15", 0x000001, 0x80000, CRC(7947c28d) SHA1(46efb56d0f7fe2e92d0d04dcd2f130aef3be436d) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "sup_ic25.ic25", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) ) // designation silkscreend on mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "sup_ic32.ic32", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) ) // designation silkscreend on mask ROM
	ROM_LOAD32_WORD( "sup_ic33.ic33", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "sup_ic12.ic12", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic14.ic14", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic11.ic11", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic13.ic13", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label
	// These 2 are copies of the previous 2 at different IC locations
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label

	ROM_REGION( 0x2000, "coin", 0 ) // Credit distribution PCB
	ROM_LOAD( "2x1c_pic16c54.u1", 0x0000, 0x2000, NO_DUMP )
ROM_END

ROM_START( speedup10 ) // Version 1.00 - REF. 960717 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "ic10_1.00.ic10", 0x000000, 0x80000, CRC(24ed8f48) SHA1(59d59e2a0b2fb7aed5320167960129819adedd9a) ) // handwritten labels IC10 1.00
	ROM_LOAD16_BYTE( "ic15_1.00.ic15", 0x000001, 0x80000, CRC(b3fda7f1) SHA1(e77ef3cb46be0767476f65dcc8d4fc12550be4a3) ) // handwritten labels IC15 1.00

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "sup_ic25.ic25", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) ) // designation silkscreend on mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "sup_ic32.ic32", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) ) // designation silkscreend on mask ROM
	ROM_LOAD32_WORD( "sup_ic33.ic33", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "sup_ic12.ic12", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic14.ic14", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic11.ic11", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) ) // designation silkscreend on mask ROM
	ROM_LOAD( "sup_ic13.ic13", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) ) // designation silkscreend on mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label
	// These 2 are copies of the previous 2 at different IC locations
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) ) // nondescript green dot label
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) ) // nondescript white dot label

	ROM_REGION( 0x2000, "coin", 0 ) // Credit distribution PCB
	ROM_LOAD( "2x1c_pic16c54.u1", 0x0000, 0x2000, NO_DUMP )
ROM_END


ROM_START( surfplnt ) // Version 4.1 - REF. 971223 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "pls_5.ic5",   0x000000, 0x80000, CRC(c96e0a18) SHA1(b313d02d1d1bff8717b3d798e6ae681baefc1061) )
	ROM_LOAD16_BYTE( "pls_11.ic11", 0x000001, 0x80000, CRC(99211d2d) SHA1(dee5b157489ce9c6988c8eec92fa91fff60d521c) )
	ROM_LOAD16_BYTE( "pls_8.ic8",   0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "pls_13.ic13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "pls_ic18.ic18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "pls_ic40.ic40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "pls_ic37.ic37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "pls_ic7.ic7",   0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic9.ic9",   0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic12.ic12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic15.ic15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "pls_19.ic19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "pls_20.ic20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "pls_21.ic21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "pls_22.ic22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "pls_19.ic27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) ) // labeled PLS 19 in IC27 on the PCB
//  ROM_LOAD( "pls_20.ic28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) ) // labeled PLS 20 in IC28 on the PCB
//  ROM_LOAD( "pls_21.ic29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) ) // labeled PLS 21 in IC29 on the PCB
//  ROM_LOAD( "pls_22.ic30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) ) // labeled PLS 22 in IC30 on the PCB
ROM_END

ROM_START( surfplnt40 ) // Version 4.0 - REF. 970514 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "pls_5.ic5",   0x000000, 0x80000, CRC(572e0343) SHA1(badb08a5a495611b5fd2d821d4299348b2c9f308) ) // sldh
	ROM_LOAD16_BYTE( "pls_11.ic11", 0x000001, 0x80000, CRC(6056edaa) SHA1(9bc2df54d1367b9d58272a8f506e523e74110361) ) // sldh
	ROM_LOAD16_BYTE( "pls_8.ic8",   0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "pls_13.ic13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "pls_ic18.ic18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "pls_ic40.ic40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "pls_ic37.ic37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "pls_ic7.ic7",   0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic9.ic9",   0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic12.ic12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic15.ic15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "pls_19.ic19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "pls_20.ic20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "pls_21.ic21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "pls_22.ic22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "pls_19.ic27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) ) // labeled PLS 19 in IC27 on the PCB
//  ROM_LOAD( "pls_20.ic28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) ) // labeled PLS 20 in IC28 on the PCB
//  ROM_LOAD( "pls_21.ic29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) ) // labeled PLS 21 in IC29 on the PCB
//  ROM_LOAD( "pls_22.ic30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) ) // labeled PLS 22 in IC30 on the PCB
ROM_END

ROM_START( surfplnt30 ) // Version 3.0 - REF. 970514 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "pls_5.ic5",   0x000000, 0x80000, CRC(9845d0e9) SHA1(48bbe43aecdf79d095c37d74f84f449e79a2b372) ) // sldh
	ROM_LOAD16_BYTE( "pls_11.ic11", 0x000001, 0x80000, CRC(aa3fe4ba) SHA1(5813044fbb3bb7ebccbe77602f30db65926fe8d3) ) // sldh
	ROM_LOAD16_BYTE( "pls_8.ic8",   0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "pls_13.ic13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "pls_ic18.ic18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "pls_ic40.ic40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "pls_ic37.ic37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "pls_ic7.ic7",   0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic9.ic9",   0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic12.ic12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic15.ic15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "pls_19.ic19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "pls_20.ic20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "pls_21.ic21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "pls_22.ic22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "pls_19.ic27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) ) // labeled PLS 19 in IC27 on the PCB
//  ROM_LOAD( "pls_20.ic28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) ) // labeled PLS 20 in IC28 on the PCB
//  ROM_LOAD( "pls_21.ic29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) ) // labeled PLS 21 in IC29 on the PCB
//  ROM_LOAD( "pls_22.ic30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) ) // labeled PLS 22 in IC30 on the PCB
ROM_END

ROM_START( surfplnt20 ) // Version 2.0 - REF. 970514 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "pls_5.ic5",   0x000000, 0x80000, CRC(93e0cf29) SHA1(f9b12dd5fb239c6a0f6d67f824355f753b86e13f) ) // sldh
	ROM_LOAD16_BYTE( "pls_11.ic11", 0x000001, 0x80000, CRC(08e74d50) SHA1(8f49307c7aacba3161ac6ed01b4333048e24194f) ) // sldh
	ROM_LOAD16_BYTE( "pls_8.ic8",   0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "pls_13.ic13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "pls_ic18.ic18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "pls_ic40.ic40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "pls_ic37.ic37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x1000000, "texture", 0 )
	ROM_LOAD( "pls_ic7.ic7",   0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic9.ic9",   0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic12.ic12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "pls_ic15.ic15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "pls_19.ic19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "pls_20.ic20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "pls_21.ic21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "pls_22.ic22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "pls_19.ic27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) ) // labeled PLS 19 in IC27 on the PCB
//  ROM_LOAD( "pls_20.ic28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) ) // labeled PLS 20 in IC28 on the PCB
//  ROM_LOAD( "pls_21.ic29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) ) // labeled PLS 21 in IC29 on the PCB
//  ROM_LOAD( "pls_22.ic30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) ) // labeled PLS 22 in IC30 on the PCB
ROM_END


ROM_START( radikalb ) // Version 2.02 - REF. 980311 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68020 code
	ROM_LOAD32_BYTE( "rab_6.ic6",   0x000000, 0x80000, CRC(ccac98c5) SHA1(43a30caf9880f48aba79676f9e746fdc6258139d) )
	ROM_LOAD32_BYTE( "rab_12.ic12", 0x000001, 0x80000, CRC(26199506) SHA1(1b7b44895aa296eab8061ae85cbb5b0d30119dc7) )
	ROM_LOAD32_BYTE( "rab_14.ic14", 0x000002, 0x80000, CRC(4a0ac8cb) SHA1(4883e5eddb833dcd39376be435aa8e8e2ec47ab5) )
	ROM_LOAD32_BYTE( "rab_19.ic19", 0x000003, 0x80000, CRC(c2d4fcb2) SHA1(8e389d1479ba084e5363aef9c797c65ca7f355d2) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "rab_ic23.ic23", 0x0000000, 0x400000, CRC(dcf52520) SHA1(ab54421c182436660d2a56a334c1aa335424644a) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "rab_ic48.ic48", 0x000000, 0x400000, CRC(9c56a06a) SHA1(54f12d8b55fa14446c47e31684c92074c4157fe1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "rab_ic45.ic45", 0x000002, 0x400000, CRC(7e698584) SHA1(a9423835a126396902c499e9f7df3b68c2ab28a8) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x2000000, "texture", 0 )
	ROM_LOAD( "rab_ic8.ic8",   0x0000000, 0x400000, CRC(4fbd4737) SHA1(594438d3edbe00682290986cc631615d7bef67f3) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic10.ic10", 0x0800000, 0x400000, CRC(870b0ce4) SHA1(75910dca87d2eb3a6b4a28f6e9c63a6b6700de84) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic15.ic15", 0x1000000, 0x400000, CRC(edb9d409) SHA1(1f8df507e990eee197f2779b45bd8f143d1bd439) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic17.ic17", 0x1800000, 0x400000, CRC(e120236b) SHA1(37d7996530eda3df0c19bca1cbf26e5ba58b0977) ) // designation silkscreend on SMT mask ROM
	// these 4 ROMs are mounted on the underside of the ROM board
	ROM_LOAD( "rab_ic9.ic9",   0x0400000, 0x400000, CRC(9e3e038d) SHA1(4a5f0b3c54c508d7f310f270dbd11bffb2bcfa89) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic11.ic11", 0x0c00000, 0x400000, CRC(75672271) SHA1(ebbdf089b4a4d5ead7d62555bb5c9a921aaa1c22) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic16.ic16", 0x1400000, 0x400000, CRC(9d595e46) SHA1(b985332974e1fb0b9d20d521da0d7deceea93a8a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic18.ic18", 0x1c00000, 0x400000, CRC(3084bc49) SHA1(9da43482293eeb08ceae67455b2fcd97b6ef5109) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "rab_24.ic24", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
	ROM_LOAD( "rab_25.ic25", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
	ROM_LOAD( "rab_26.ic26", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
	ROM_LOAD( "rab_27.ic27", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "rab_24.ic32", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) ) // labeled RAB 24 in IC32 on the PCB
//  ROM_LOAD( "rab_25.ic33", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) ) // labeled RAB 25 in IC33 on the PCB
//  ROM_LOAD( "rab_26.ic34", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) ) // labeled RAB 26 in IC34 on the PCB
//  ROM_LOAD( "rab_27.ic35", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) ) // labeled RAB 27 in IC35 on the PCB
ROM_END

ROM_START( radikalba ) // Version 2.02, Atari license - REF. 980311 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68020 code
	ROM_LOAD32_BYTE( "rab_6.ic6",   0x000000, 0x80000, CRC(ccac98c5) SHA1(43a30caf9880f48aba79676f9e746fdc6258139d) )
	ROM_LOAD32_BYTE( "rab_12.ic12", 0x000001, 0x80000, CRC(26199506) SHA1(1b7b44895aa296eab8061ae85cbb5b0d30119dc7) )
	ROM_LOAD32_BYTE( "rab_14.ic14", 0x000002, 0x80000, CRC(4a0ac8cb) SHA1(4883e5eddb833dcd39376be435aa8e8e2ec47ab5) )
	ROM_LOAD32_BYTE( "rab_19.ic19", 0x000003, 0x80000, CRC(2631bd61) SHA1(57331ad49e7284b82073f696049de109b7683b03) ) // sldh

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "rab_ic23.ic23", 0x0000000, 0x400000, CRC(dcf52520) SHA1(ab54421c182436660d2a56a334c1aa335424644a) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "rab_ic48.ic48", 0x000000, 0x400000, CRC(9c56a06a) SHA1(54f12d8b55fa14446c47e31684c92074c4157fe1) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "rab_ic45.ic45", 0x000002, 0x400000, CRC(7e698584) SHA1(a9423835a126396902c499e9f7df3b68c2ab28a8) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x2000000, "texture", 0 )
	ROM_LOAD( "rab_ic8.ic8",   0x0000000, 0x400000, CRC(4fbd4737) SHA1(594438d3edbe00682290986cc631615d7bef67f3) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic10.ic10", 0x0800000, 0x400000, CRC(870b0ce4) SHA1(75910dca87d2eb3a6b4a28f6e9c63a6b6700de84) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic15.ic15", 0x1000000, 0x400000, CRC(edb9d409) SHA1(1f8df507e990eee197f2779b45bd8f143d1bd439) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic17.ic17", 0x1800000, 0x400000, CRC(e120236b) SHA1(37d7996530eda3df0c19bca1cbf26e5ba58b0977) ) // designation silkscreend on SMT mask ROM
	// these 4 ROMs are mounted on the underside of the ROM board
	ROM_LOAD( "rab_ic9.ic9",   0x0400000, 0x400000, CRC(9e3e038d) SHA1(4a5f0b3c54c508d7f310f270dbd11bffb2bcfa89) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic11.ic11", 0x0c00000, 0x400000, CRC(75672271) SHA1(ebbdf089b4a4d5ead7d62555bb5c9a921aaa1c22) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic16.ic16", 0x1400000, 0x400000, CRC(9d595e46) SHA1(b985332974e1fb0b9d20d521da0d7deceea93a8a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "rab_ic18.ic18", 0x1c00000, 0x400000, CRC(3084bc49) SHA1(9da43482293eeb08ceae67455b2fcd97b6ef5109) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "rab_24.ic24", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
	ROM_LOAD( "rab_25.ic25", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
	ROM_LOAD( "rab_26.ic26", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
	ROM_LOAD( "rab_27.ic27", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "rab_24.ic32", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) ) // labeled RAB 24 in IC32 on the PCB
//  ROM_LOAD( "rab_25.ic33", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) ) // labeled RAB 25 in IC33 on the PCB
//  ROM_LOAD( "rab_26.ic34", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) ) // labeled RAB 26 in IC34 on the PCB
//  ROM_LOAD( "rab_27.ic35", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) ) // labeled RAB 27 in IC35 on the PCB
ROM_END

/*
Football Power has a small PCB inside the ball controller, very similar to the one found on
Gaelco Football, with two accelerometers and a PIC.
   _________________
  |      ______    |
 _|     /     /    |
 \|    /  <-ADXL250
  |   /_____/      |
  |  ____________  |
  |  \  \    <-ADXL150
  |   \  \_____\  \|
  |    \___________\
  |   Osc          |
  |  16 MHz        |
  |  __________    |
  | |PIC16C710|    |
  | |_________|    |
  |                |
  |________________|
*/
ROM_START( footbpow ) // Version 1.2 - REF. 000208 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68020 code
	ROM_LOAD32_BYTE( "fop_7.ic7",   0x000000, 0x80000, CRC(a2d7ec69) SHA1(27e4f3d27882152244c0f9d5a984e0f1bd7b7d3f) )
	ROM_LOAD32_BYTE( "fop_12.ic12", 0x000001, 0x80000, CRC(443caf77) SHA1(2b0c6dccee28fb3caa0b2493f59ddbd29897aed9) )
	ROM_LOAD32_BYTE( "fop_13.ic13", 0x000002, 0x80000, CRC(57723eda) SHA1(09972b09444b6704dcc966033bfab61ea57d0cd0) )
	ROM_LOAD32_BYTE( "fop_19.ic19", 0x000003, 0x80000, CRC(aa59cd2d) SHA1(7cc6edfd0896e4d2c881b16d5ad07361bdeff11d) )

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "fop_ic23.ic23", 0x0000000, 0x400000, CRC(3c02f7c6) SHA1(2325f2a1b260ac60929c82640ced481ad67bb2e0) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "fop_ic48.ic48", 0x000000, 0x800000, CRC(efddf5c1) SHA1(1014b0193d17de05ebcc733fc5d26089b932385b) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "fop_ic42.ic42", 0x000002, 0x800000, CRC(8772e536) SHA1(530dfb4e27466bd97582c4fd50af01f14716ed2b) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x2000000, "texture", 0 )
	ROM_LOAD( "fop_ic8.ic8",   0x0000000, 0x400000, CRC(eaff30ec) SHA1(63f5d33b98194a206c558f9e02c432e7e05aa0e6) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic10.ic10", 0x0800000, 0x400000, CRC(536c822b) SHA1(235e96af470785f6cca010782560a4071f285901) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic15.ic15", 0x1000000, 0x400000, CRC(c8903051) SHA1(b5927a0bbba017d42b98e7850df966cfa9eeb64a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic17.ic17", 0x1800000, 0x400000, CRC(559a38ae) SHA1(e36d596ad90d0f3657d677e3afa984be30c1fa3b) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "fop_24.ic24", 0x0000000, 0x020000, CRC(3214ae1b) SHA1(3ae2fa28ef603b34b3c72313c513f200e2750b85) )
	ROM_LOAD( "fop_25.ic25", 0x0020000, 0x020000, CRC(69a8734c) SHA1(835db85371d8fbf0c1a2bc0c6109286f12c95794) )
	ROM_LOAD( "fop_26.ic26", 0x0040000, 0x020000, CRC(b5877b68) SHA1(6f6f00da84d6d84895691266c2022fd4cd92f228) )
	ROM_LOAD( "fop_27.ic27", 0x0060000, 0x020000, CRC(58309912) SHA1(eb62ccfd75fc168338d30bc30214e6f9f62e5e70) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "fop_24.ic34", 0x0000000, 0x020000, CRC(3214ae1b) SHA1(3ae2fa28ef603b34b3c72313c513f200e2750b85) ) // labeled FOP 24 in IC34 on the PCB
//  ROM_LOAD( "fop_25.ic35", 0x0020000, 0x020000, CRC(69a8734c) SHA1(835db85371d8fbf0c1a2bc0c6109286f12c95794) ) // labeled FOP 25 in IC35 on the PCB
//  ROM_LOAD( "fop_26.ic36", 0x0040000, 0x020000, CRC(b5877b68) SHA1(6f6f00da84d6d84895691266c2022fd4cd92f228) ) // labeled FOP 26 in IC36 on the PCB
//  ROM_LOAD( "fop_27.ic37", 0x0060000, 0x020000, CRC(58309912) SHA1(eb62ccfd75fc168338d30bc30214e6f9f62e5e70) ) // labeled FOP 27 in IC37 on the PCB

	ROM_REGION( 0x2000, "io", ROMREGION_ERASEFF)
	ROM_LOAD("ball_pic16c710.u1", 0x0000, 0x2000, NO_DUMP ) // I/O for the ball controller
ROM_END

ROM_START( footbpow11 ) // Version 1.1 - REF. 000208 ROM board
	ROM_REGION( 0x200000, "maincpu", 0 )    // 68020 code
	ROM_LOAD32_BYTE( "fop_7_..ic7",  0x000000, 0x80000, CRC(1314e1d6) SHA1(9d049a0659d07af7b874bc541e4abfd249a430c8) ) // labeled FOP 7 .
	ROM_LOAD32_BYTE( "fop_12..ic12", 0x000001, 0x80000, CRC(d0dea4db) SHA1(688fa53a682cb0a76f547364600e6a61a85a1a1d) ) // labeled FOP 12.
	ROM_LOAD32_BYTE( "fop_13..ic13", 0x000002, 0x80000, CRC(c58afb45) SHA1(0b0f95b31532bcdf599d160d1a9fd46fe9d24b55) ) // labeled FOP 13.
	ROM_LOAD32_BYTE( "fop_19..ic19", 0x000003, 0x80000, CRC(c6380cf1) SHA1(845d9db0e8f4a762f5e527da45f6751e5583cb71) ) // labeled FOP 19.

	ROM_REGION16_LE( 0x400000, "adsprom", 0 ) // ADSP-2115 code & data
	ROM_LOAD( "fop_ic23.ic23", 0x0000000, 0x400000, CRC(3c02f7c6) SHA1(2325f2a1b260ac60929c82640ced481ad67bb2e0) ) // designation silkscreend on SMT mask ROM

	ROM_REGION32_LE( 0x1000000, "tmsrom", 0 )
	ROM_LOAD32_WORD( "fop_ic48.ic48", 0x000000, 0x800000, CRC(efddf5c1) SHA1(1014b0193d17de05ebcc733fc5d26089b932385b) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD32_WORD( "fop_ic42.ic42", 0x000002, 0x800000, CRC(8772e536) SHA1(530dfb4e27466bd97582c4fd50af01f14716ed2b) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x2000000, "texture", 0 )
	ROM_LOAD( "fop_ic8.ic8",   0x0000000, 0x400000, CRC(eaff30ec) SHA1(63f5d33b98194a206c558f9e02c432e7e05aa0e6) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic10.ic10", 0x0800000, 0x400000, CRC(536c822b) SHA1(235e96af470785f6cca010782560a4071f285901) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic15.ic15", 0x1000000, 0x400000, CRC(c8903051) SHA1(b5927a0bbba017d42b98e7850df966cfa9eeb64a) ) // designation silkscreend on SMT mask ROM
	ROM_LOAD( "fop_ic17.ic17", 0x1800000, 0x400000, CRC(559a38ae) SHA1(e36d596ad90d0f3657d677e3afa984be30c1fa3b) ) // designation silkscreend on SMT mask ROM

	ROM_REGION( 0x0080000, "texmask", 0 )
	ROM_LOAD( "fop_24.ic24", 0x0000000, 0x020000, CRC(3214ae1b) SHA1(3ae2fa28ef603b34b3c72313c513f200e2750b85) )
	ROM_LOAD( "fop_25.ic25", 0x0020000, 0x020000, CRC(69a8734c) SHA1(835db85371d8fbf0c1a2bc0c6109286f12c95794) )
	ROM_LOAD( "fop_26.ic26", 0x0040000, 0x020000, CRC(b5877b68) SHA1(6f6f00da84d6d84895691266c2022fd4cd92f228) )
	ROM_LOAD( "fop_27.ic27", 0x0060000, 0x020000, CRC(58309912) SHA1(eb62ccfd75fc168338d30bc30214e6f9f62e5e70) )
	// These 4 are copies of the previous 4 at different IC locations
//  ROM_LOAD( "fop_24.ic34", 0x0000000, 0x020000, CRC(3214ae1b) SHA1(3ae2fa28ef603b34b3c72313c513f200e2750b85) ) // labeled FOP 24 in IC34 on the PCB
//  ROM_LOAD( "fop_25.ic35", 0x0020000, 0x020000, CRC(69a8734c) SHA1(835db85371d8fbf0c1a2bc0c6109286f12c95794) ) // labeled FOP 25 in IC35 on the PCB
//  ROM_LOAD( "fop_26.ic36", 0x0040000, 0x020000, CRC(b5877b68) SHA1(6f6f00da84d6d84895691266c2022fd4cd92f228) ) // labeled FOP 26 in IC36 on the PCB
//  ROM_LOAD( "fop_27.ic37", 0x0060000, 0x020000, CRC(58309912) SHA1(eb62ccfd75fc168338d30bc30214e6f9f62e5e70) ) // labeled FOP 27 in IC37 on the PCB

	ROM_REGION( 0x2000, "io", ROMREGION_ERASEFF)
	ROM_LOAD("ball_pic16c710.u1", 0x0000, 0x2000, NO_DUMP ) // I/O for the ball controller
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1996, speedup,   0,        gaelco3d,  speedup,  gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Speed Up (version 2.20)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_speedup )
GAMEL( 1996, speedup12, speedup,  gaelco3d,  speedup,  gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Speed Up (version 1.20)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_speedup )
GAMEL( 1996, speedup10, speedup,  gaelco3d,  speedup,  gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Speed Up (version 1.00)",                      MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_speedup )

GAME( 1997, surfplnt,   0,        gaelco3d,  surfplnt, gaelco3d_state, empty_init, ROT0, "Gaelco (Atari license)", "Surf Planet (version 4.1)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1997, surfplnt40, surfplnt, gaelco3d,  surfplnt, gaelco3d_state, empty_init, ROT0, "Gaelco (Atari license)", "Surf Planet (version 4.0)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1997, surfplnt30, surfplnt, gaelco3d,  surfplnt, gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Surf Planet (version 3.0)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1997, surfplnt20, surfplnt, gaelco3d,  surfplnt, gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Surf Planet (version 2.0)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

GAME( 1998, radikalb,   0,        gaelco3d2, radikalb, gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Radikal Bikers (version 2.02)",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
GAME( 1998, radikalba,  radikalb, gaelco3d2, radikalb, gaelco3d_state, empty_init, ROT0, "Gaelco (Atari license)", "Radikal Bikers (version 2.02, Atari license)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

GAME( 1999, footbpow,   0,        footbpow,  footbpow, gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Football Power (version 1.2)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )
GAME( 1999, footbpow11, footbpow, footbpow,  footbpow, gaelco3d_state, empty_init, ROT0, "Gaelco",                 "Football Power (version 1.1)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS )
