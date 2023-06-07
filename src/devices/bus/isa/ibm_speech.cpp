// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/*
    IBM PS/2 Speech Adapter

    The ROM listing calls this card the "TALKER II".

    The ISA version of the card uses a different timing mechanism
    than the PCjr version to account for a 4MHz operating speed
    rather than the 4.77 bus clock. The ISA version also uses
    IRQ7 rather than IRQ1.

    Lots of TTL, all off-the-shelf parts except the mask ROM.

    Y1      - 4MHz oscillator
    ZM5     - MC14529CP
    ZM8     - TMS5220CNL
    ZM9     - 8254 PIT
    ZM10    - 32Kx8 mask ROM
    ZM22    - 8255A PIC
*/

#include "emu.h"
#include "ibm_speech.h"

#define LOG_ROM         (1U << 1)
#define LOG_MUX         (1U << 2)
#define LOG_CVSD        (1U << 3)
#define LOG_PIT         (1U << 4)
#define LOG_IRQ         (1U << 5)
#define LOG_PORTS       (1U << 6)
#define LOG_ALL         (LOG_ROM|LOG_MUX|LOG_CVSD|LOG_PIT|LOG_IRQ|LOG_PORTS)

//#define VERBOSE         (LOG_PORTS)
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(ISA8_IBM_SPEECH, isa8_ibm_speech_device, "isa_ibm_speech", "IBM PS/2 Speech Adapter")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

isa8_ibm_speech_device::isa8_ibm_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_IBM_SPEECH, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_lpc(*this, "lpc"),
	m_cvsd(*this, "cvsd"),
	m_dac1bit(*this, "8254_audio"),
	m_pit(*this, "timers"),
	m_ppi(*this, "ppi"),
	m_speaker(*this, "speaker"),
	m_rom(*this, "option")
{ }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ibm_speech_device::device_start()
{
	set_isa_device();

	// Not configurable.
	m_isa->install_device(0xfb98, 0xfb9b,
		emu::rw_delegate(m_ppi, FUNC(i8255_device::read)),
		emu::rw_delegate(m_ppi, FUNC(i8255_device::write)));
	m_isa->install_device(0xfb9c, 0xfb9f,
		emu::rw_delegate(m_pit, FUNC(pit8254_device::read)),
		emu::rw_delegate(m_pit, FUNC(pit8254_device::write)));
	m_isa->install_device(0xff98, 0xff98,
		emu::rw_delegate(*this, FUNC(isa8_ibm_speech_device::shift_register_r)),
		emu::rw_delegate(*this, FUNC(isa8_ibm_speech_device::shift_register_w)));
	m_isa->install_device(0xff9f, 0xff9f,
		emu::rw_delegate(*this, FUNC(isa8_ibm_speech_device::audio_control_latch_r)),
		emu::rw_delegate(*this, FUNC(isa8_ibm_speech_device::audio_control_latch_w)));

	m_channel_mux = 0;
	m_acl_int_ena = false;
	m_acl_chan_ena = false;
	m_sr_clk = false;
	m_cvsd_ed = false;
	m_beeper_gate = false;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_ibm_speech_device::device_reset()
{
	// Not configurable.
	rom_page_w(0);
	m_cvsd_sr_bits_remaining = 0;
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( ibm_speech )
	ROM_REGION( 0x8000, "option", 0 )
	ROM_LOAD( "68x2838.zm10", 0x0000, 0x8000, CRC(98ddf27f) SHA1(0cb41d490db2ed59989d2eae14a002ad0382e7ad) )
ROM_END

const tiny_rom_entry *isa8_ibm_speech_device::device_rom_region() const
{
	return ROM_NAME( ibm_speech );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_ibm_speech_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL XTAL_Y1 = XTAL(4'000'000);

	// breakout box
	SPEAKER(config, m_speaker);

	PIT8254(config, m_pit, 0);
	// Channel 0: CVSD CLOCK.
	//            Rising edge goes to the MC3418's clock pin.
	//            Rising edge inverted goes to the shift register clock.
	m_pit->set_clk<0>(XTAL_Y1);  // CVSD clock is the 4MHz crystal.
	m_pit->out_handler<0>().set(FUNC(isa8_ibm_speech_device::cvsd_clock_w));
	// Channel 1: CVSD FRAME.
	//            PCjr: CVSD_CLOCK / 8, but seems to be just CVSD_CLOCK here.
	m_pit->set_clk<1>(XTAL_Y1);
	m_pit->out_handler<1>().set(FUNC(isa8_ibm_speech_device::cvsd_frame_w));
	// Channel 2: INT CLOCK.
	//            Channel 2 can be configured either for interrupt mode or audio mode via the Audio Control Latch.
	//            In interrupt mode, IRQ1 is signalled by the 5220.
	//              The gate opens when the 5220 IRQ is signalled.
	//            In audio mode, the timer output is multiplexed into audio output.
	//              The gate is held open.
	m_pit->set_clk<2>(XTAL_Y1);
	m_pit->out_handler<2>().set(FUNC(isa8_ibm_speech_device::int_clock_w));

	// TMS5220 LPC
	// The 5220 has no direct-access speech ROM.
	// All data is transferred from the host PC via the 8255.
	// The preset vocabulary is in the option ROM.
	TMS5220(config, m_lpc, 640000); // TODO: Confirm TMS clock
	m_lpc->add_route(ALL_OUTPUTS, m_speaker, 1.0);
	m_lpc->irq_cb().set(FUNC(isa8_ibm_speech_device::lpc_interrupt_w));

	// MC3418 CVSD
	MC3418(config, m_cvsd, 0);
	m_cvsd->add_route(ALL_OUTPUTS, m_speaker, 1.0);

	// PIT CH2
	SPEAKER_SOUND(config, m_dac1bit);
	m_dac1bit->add_route(ALL_OUTPUTS, m_speaker, 1.0);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(isa8_ibm_speech_device::porta_r));
	m_ppi->in_pb_callback().set(m_lpc, FUNC(tms5220_device::status_r)); // direct hookup
	m_ppi->in_pc_callback().set(FUNC(isa8_ibm_speech_device::portc_r));

	m_ppi->out_pa_callback().set(FUNC(isa8_ibm_speech_device::porta_w));
	m_ppi->out_pb_callback().set(m_lpc, FUNC(tms5220_device::data_w));  // direct hookup
	m_ppi->out_pc_callback().set(FUNC(isa8_ibm_speech_device::portc_w));
}

void isa8_ibm_speech_device::device_reset_after_children()
{
	m_pit->write_gate0(ASSERT_LINE); // pulled high
	m_pit->write_gate1(ASSERT_LINE); // pulled high
}

/******************************************************************************
    ROM bankswitching

    The option ROM is 32Kx8 and divided into four 8K pages at CE00: to CFFF:.
    Bits 2-3 of PPI PORT.A control which page is visible.
    All pages contain the option ROM boot signature.
******************************************************************************/
void isa8_ibm_speech_device::rom_page_w(uint8_t data)
{
	m_isa->install_bank(0xce000, 0xcffff, m_rom + (0x2000*data));
}

/******************************************************************************
    Audio Control Latch

    This is for multiplexing interrupt and audio sources.
******************************************************************************/
uint8_t isa8_ibm_speech_device::audio_control_latch_r()
{
	uint8_t data = 0;

	// The bits are inverted when reading.
	data |= (m_acl_int_ena  ? 0 : 2);   // -INT_ENA
	data |= (m_acl_chan_ena ? 0 : 1);   // -CHAN_ENA

	return data;
}

void isa8_ibm_speech_device::audio_control_latch_w(uint8_t data)
{
	m_acl_int_ena   = BIT(data, 1);    // INT_ENA
	m_acl_chan_ena  = BIT(data, 0);    // CHAN_ENA

	if(m_acl_int_ena)
	{
		// IRQ7 enabled, close PIT CH2 gate.
		// CH2 will be set up for one-shot mode.
		m_pit->write_gate2(CLEAR_LINE);
	}
	else
	{
		// IRQ7 disabled, PIT CH2 is in audio output mode. Gate is held open.
		m_pit->write_gate2(ASSERT_LINE);
	}
}


/******************************************************************************
    PPI Port A

    PA.0: CHANNEL MUX    (O)
    PA.1: CHANNEL MUX    (O)
    PA.2: ROM PAGE       (O)
    PA.3: ROM PAGE       (O)
    PA.4: n/c
    PA.5: n/c
    PA.6: n/c
    PA.7: LPC IN PROGRESS (I/O)
******************************************************************************/
uint8_t isa8_ibm_speech_device::porta_r()
{
	uint8_t data = 0;

	data |= (m_lpc_running << 7);

	return data;
}

void isa8_ibm_speech_device::porta_w(uint8_t data)
{
	channel_mux_w(data & 3);
	rom_page_w((data >> 2) & 3);
	m_lpc_running = BIT(data, 7);
}

/******************************************************************************
    PPI Port C

    PC.0: -LPC READY  (I)
    PC.1: -LPC INT    (I)
    PC.2: CVSD FRAME  (I)
    PC.3: -CVSD CLOCK (I)
    PC.4: LPC RS      (O) (5220 RS#, inverted)
    PC.5: LPC WS      (O) (5220 WS#, inverted)
    PC.6: CVSD E/D    (I) (PC.6 | PIT.OUT1 triggers ZM12 SLOAD#)
    PC.7: n/c
******************************************************************************/
uint8_t isa8_ibm_speech_device::portc_r()
{
	uint8_t data = 0;

	data |= (m_lpc->readyq_r()  << 0);
	data |= (m_lpc->intq_r()    << 1);
	data |= (m_cvsd_frame       << 2);

	// PCjr
	//data |= (!m_cvsd_clock      ? (1 << 3) : 0);

	if(m_cvsd_sr_bits_remaining > 0)
	{
		// ISA: Clock is held until the shift register needs a new frame.
	}
	else
	{
		data |= (!m_cvsd_clock << 3);
	}

	data |= (m_cvsd_ed << 6); // 0: CVSD playback, 1: CVSD record

	return data;
}

void isa8_ibm_speech_device::portc_w(uint8_t data)
{
	m_lpc->rsq_w(!BIT(data, 4));
	m_lpc->wsq_w(!BIT(data, 5));
}

/******************************************************************************
    Channel Mux

    Selects the audio source from the four possible sources. Only one source
    can be used at a time.
******************************************************************************/
void isa8_ibm_speech_device::channel_mux_w(uint8_t data)
{
	switch(data & 3)
	{
		case 0:
			LOGMASKED(LOG_MUX, "Channel MUX: source now LPC\n");
			m_lpc->set_output_gain(0, 1.0);
			m_cvsd->set_output_gain(0, 0.0);
			m_beeper_gate = 0;
			break;
		case 1:
			LOGMASKED(LOG_MUX, "Channel MUX: source now CVSD\n");
			m_lpc->set_output_gain(0, 0.0);
			m_cvsd->set_output_gain(0, 1.0);
			m_beeper_gate = 0;
			break;
		case 2:
			LOGMASKED(LOG_MUX, "Channel MUX: source now 8254\n");
			m_lpc->set_output_gain(0, 0.0);
			m_cvsd->set_output_gain(0, 0.0);
			m_beeper_gate = 1;
			break;
		case 3:
			LOGMASKED(LOG_MUX, "Channel MUX: source now Test Signal\n");
			// TODO: How is this generated?
			break;
	}
}

void isa8_ibm_speech_device::lpc_interrupt_w(int state)
{
	// Active low.

	// Pulse IRQ7 on falling edge.
	if(m_lpc_interrupt && !state)
	{
		LOGMASKED(LOG_IRQ, "pulsing IRQ7\n");
		m_isa->irq7_w(ASSERT_LINE);         // Raise IRQ...
		m_pit->write_gate2(ASSERT_LINE);    // ...CH2 gate open, triggering CH2 one-shot.
	}
	else if(!m_lpc_interrupt && !state)
	{
		LOGMASKED(LOG_IRQ, "IRQ7 already raised, go away\n");
	}
	else if(state)
	{
		LOGMASKED(LOG_IRQ, "LPC INT pin going high, CH2 gate closed\n");
		m_isa->irq7_w(CLEAR_LINE);
		m_pit->write_gate2(CLEAR_LINE);
	}

	m_lpc_interrupt = state;
}

/******************************************************************************
    CVSD Clock - PIT CH0

    Sets the speed of the CVSD bitstream.

    Non-inverted to:
        - MC3418 CLK
    Inverted to:
        - PIT CLK1
        - ZM12 serial clock
        - PPI PORTC.3
******************************************************************************/
void isa8_ibm_speech_device::cvsd_clock_w(int state)
{
	m_cvsd_clock = state;

	LOGMASKED(LOG_CVSD, "cvsd_clock_w %d\n", state);

	// PIT CH0 is the CVSD CLOCK signal.
	m_cvsd->clock_w(state);         // Straight through to the CVSD CLK pin.

	// Through an inverter and to:
	//m_pit->write_clk1(state ? 0 : 1); // PCjr: PIT CLK1
	cvsd_shiftreg_clk_w(state ? 0 : 1); // 74859 clock pin
										// PPI PC.3 (handled over there)
}

/******************************************************************************
    CVSD Frame - PIT CH1

    Defined as (CVSD Clock / 8).
    This signal goes low for one clock every 8 CVSD bit clocks, which specifies
    how often a new byte gets latched into the CVSD shift register.
******************************************************************************/
void isa8_ibm_speech_device::cvsd_frame_w(int state)
{
	// PIT CH1 is the CVSD FRAME signal.
	LOGMASKED(LOG_CVSD, "*** CVSD frame going %s\n", (state ? "high" : "low"));

	if(state && (m_cvsd_sr_bits_remaining == 0))
	{
		m_cvsd_sr_serial = m_cvsd_sr_parallel;
	}

	m_cvsd_frame = state;
}

/******************************************************************************
    INT CLOCK - PIT CH2

    Clocked by the 4MHz crystal.

    When INT_ENA is asserted, pulses IRQ7 when the 5220 raises an IRQ.
    When INT_ENA is cleared, the timer acts as a PC speaker-style beeper.
******************************************************************************/
void isa8_ibm_speech_device::int_clock_w(int state)
{
	if(!m_acl_int_ena)
	{
		// CH2 is now a beeper.
		//LOGMASKED(LOG_IRQ, "%s: INT CLOCK: beeper mode\n", FUNCNAME);
		m_dac1bit->level_w(state);
	}
}

/******************************************************************************
    CVSD Shift Register
******************************************************************************/
void isa8_ibm_speech_device::cvsd_shiftreg_clk_w(int state)
{
	// If rising edge of the inverted signal...
	if(!m_sr_clk && state)
	{
		// Shift one bit out of the shift register and into the CVSD.
		if(m_cvsd_sr_bits_remaining > 0)
		{
			LOGMASKED(LOG_CVSD, "bit going SR -> CVSD, %d left\n", m_cvsd_sr_bits_remaining);
			m_cvsd->digin_w(BIT(m_cvsd_sr_serial, 0));
			m_cvsd_sr_serial >>= 1;

			m_cvsd_sr_bits_remaining--;
		}
	}

	m_sr_clk = state;
}

/******************************************************************************
    CVSD Shift Register read and write logic.
******************************************************************************/

// The host sees the parallel side, the CVSD sees the serial side.
uint8_t isa8_ibm_speech_device::shift_register_r()
{
	return m_cvsd_sr_parallel;
}

void isa8_ibm_speech_device::shift_register_w(uint8_t data)
{
	if(m_cvsd_sr_bits_remaining == 0)
	{
		LOGMASKED(LOG_CVSD, "incoming to CVSD shift register %02X\n", data);
		m_cvsd_sr_parallel = data;
		m_cvsd_sr_bits_remaining = 8;
		cvsd_frame_w(1);
	}
	else
	{
		LOGMASKED(LOG_CVSD, "incoming to CVSD shift register %02X but not out of bits\n", data);
	}
}
