// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    prophet600.cpp - Sequential Circuits Prophet-600, designed by Dave Smith
 
	This was the first commercial synthesizer with built-in MIDI.
 
    Skeleton driver by R. Belmont

    Hardware:
        CPU: Z80 CPU + 8253 PIT + 6850 UART

    Memory map:
    0x0000-0x1fff: ROM
    0x2000-0x27ff: RAM 1
    0x3000-0x37ff: RAM 2
    0x4000-0x4001: DAC for CV/gate drive
    0x6000-0x6001: 6850 writes
    0xe000-0xe001: 6850 reads
 
    I/O map:
    00-07:	8253 PIT
    08: set row to scan for keyboard/panel input and LED / lamp output
    09: read: analog comparitor, some value vs. the DAC.  write: output for LEDs/lamps
    	comparitor bits:
    	bit 1: FFFStatus
    	bit 2: output of 8253 channel 2
    	bit 3: ADCCompare: returns sign of if current value is > or < the DAC value
	 
	0a: read: keyboard/panel input scan bits, write: select a pot on the panel for the comparitor
    0b: write: update all gate signals
    0c: ???
    0d: write: select which CV signal will be updated with the current DAC value
    0e: write: mask, masks
    	bit 0: FFFP -FF P
    	bit 1: mask gate from 8253
    	bit 3: FFFD FF D
    	bit 4: FFFCL -FF CL
 
 
    Info:
    - Prophet 600 Technical Manual
 
    - https://github.com/gligli/p600fw - GPLv3 licensed complete replacement firmware
    for the Prophet 600, but written in C and runs on an AVR that replaces the original
    Z80.
 
***************************************************************************/

#include "emu.h"
#include "machine/clock.h"
#include "cpu/z80/z80.h"
#include "bus/midi/midi.h"
#include "machine/6850acia.h"
#include "machine/pit8253.h"

#include "prophet600.lh"

#define MAINCPU_TAG	"z80"
#define PIT_TAG		"pit"
#define UART_TAG	"uart"

enum
{
	CV_CV_Osc1A = 0, CV_Osc2A, CV_Osc3A, CV_Osc4A, CV_Osc5A, CV_Osc6A,           
	CV_Osc1B, CV_Osc2B, CV_Osc3B, CV_Osc4B, CV_Osc5B, CV_Osc6B,           
	CV_Flt1, CV_Flt2, CV_Flt3, CV_Flt4, CV_Flt5, CV_Flt6,                
	CV_Amp1, CV_Amp2, CV_Amp3, CV_Amp4, CV_Amp5, CV_Amp6,                
	CV_PModOscB, CV_VolA, CV_VolB, CV_MasterVol, CV_APW, CV_ExtFilter, CV_Resonance, CV_BPW,

	CV_MAX
};

class prophet600_state : public driver_device
{
public:
	prophet600_state(const machine_config &mconfig, device_type type, std::string tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_acia(*this, UART_TAG),
		m_dac(0),
		m_scanrow(0),
		m_comparitor(0),
		m_nmi_gate(false)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;

	DECLARE_DRIVER_INIT(prophet600);

	DECLARE_WRITE_LINE_MEMBER( pit_ch0_tick_w );
	DECLARE_WRITE_LINE_MEMBER( pit_ch2_tick_w );
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( acia_clock_w );

	DECLARE_WRITE8_MEMBER(dac_w);
	DECLARE_WRITE8_MEMBER(scanrow_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(scan_r);
	DECLARE_WRITE8_MEMBER(potmux_w);
	DECLARE_READ8_MEMBER(comparitor_r);
	DECLARE_WRITE8_MEMBER(mask_w);
	DECLARE_WRITE8_MEMBER(cv_w);
	DECLARE_WRITE8_MEMBER(gate_w);

private:
	UINT16 m_dac;
	UINT8 m_scanrow;
	UINT8 m_comparitor;

	bool m_nmi_gate;

	// gates
	bool m_ASaw, m_ATri, m_Sync, m_BSaw, m_BTri, m_PModFA, m_PModFil;

	// control voltages
	UINT16 m_CVs[CV_MAX];
};

WRITE_LINE_MEMBER( prophet600_state::pit_ch0_tick_w )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

WRITE_LINE_MEMBER( prophet600_state::pit_ch2_tick_w )
{
	m_comparitor &= ~0x04;
	m_comparitor |= (state == ASSERT_LINE) ? 0x04 : 0x00;
}

WRITE_LINE_MEMBER( prophet600_state::acia_irq_w )
{
	if (!m_nmi_gate)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
	}
}

WRITE_LINE_MEMBER( prophet600_state::acia_clock_w )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

WRITE8_MEMBER( prophet600_state::dac_w )
{
	if (offset)
	{
		m_dac &= 0xff;
		m_dac |= (data<<8);
		return;
	}

	m_dac &= 0xff00;
	m_dac |= data;
}

// also selects LEDs and keyboard keys to check
WRITE8_MEMBER(prophet600_state::scanrow_w)
{
	m_scanrow = data;
}

// scanrow 0x10 = LEDs, 0x20 = 7-segment #1, 0x40 = 7-segment #2
WRITE8_MEMBER(prophet600_state::led_w)
{
	if (m_scanrow & 0x10)
	{
		// LEDs in row 0x10 are Seq1=0, Seq2=1, ArpUD=2, ArpAssign=3, Preset=4, Record=5, ToTape=6, FromTape=7
	}
	else if (m_scanrow & 0x20)
	{
		output().set_digit_value(0, data);
	}
	else if (m_scanrow & 0x40)
	{
		output().set_digit_value(1, data);
	}
}

READ8_MEMBER(prophet600_state::scan_r)
{
	return 0;
}

WRITE8_MEMBER(prophet600_state::mask_w)
{
	m_nmi_gate = (data & 0x02) ? true : false;
	if (m_nmi_gate)	// gate is set, comparitor line is pulled up to Vcc
	{
		m_comparitor |= 0x04;
	}
	else
	{
		m_comparitor &= ~0x04;
	}

//	printf("8253 gate = %x\n", data & 0x02);
}

WRITE8_MEMBER(prophet600_state::cv_w)
{
	int cvnum = data & 7;

	if (!(cvnum & 0x08)) m_CVs[cvnum] = m_dac;
	if (!(cvnum & 0x10)) m_CVs[cvnum+8] = m_dac;
	if (!(cvnum & 0x20)) m_CVs[cvnum+16] = m_dac;
	if (!(cvnum & 0x40)) m_CVs[cvnum+24] = m_dac;
}

WRITE8_MEMBER(prophet600_state::gate_w)
{
	m_ASaw = (data & 0x01);
	m_ATri = (data & 0x02);
	m_Sync = (data & 0x04);
	m_BSaw = (data & 0x08);
	m_BTri = (data & 0x10);
	m_PModFA = (data & 0x20);
	m_PModFil = (data & 0x40);
}

/* Pots: Mixer=0,Cutoff=1,Resonance=2,FilEnvAmt=3,FilRel=4,FilSus=5,
	FilDec=6,FilAtt=7,AmpRel=8,AmpSus=9,AmpDec=10,AmpAtt=11,
	Glide=12,BPW=13,MVol=14,MTune=15,PitchWheel=16,ModWheel=22,
    Speed,APW,PModFilEnv,LFOFreq,PModOscB,LFOAmt,FreqB,FreqA,FreqBFine
*/
WRITE8_MEMBER(prophet600_state::potmux_w)
{
}

READ8_MEMBER(prophet600_state::comparitor_r)
{
//	m_comparitor ^= 0x04;
	return m_comparitor;
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 8, prophet600_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION(MAINCPU_TAG, 0)
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x3000, 0x37ff) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_WRITE(dac_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE(UART_TAG, acia6850_device, control_w)
	AM_RANGE(0x6001, 0x6001) AM_DEVWRITE(UART_TAG, acia6850_device, data_w)
	AM_RANGE(0xe000, 0xe000) AM_DEVREAD(UART_TAG, acia6850_device, status_r)
	AM_RANGE(0xe001, 0xe001) AM_DEVREAD(UART_TAG, acia6850_device, data_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, prophet600_state )
	AM_RANGE(0x00, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE(PIT_TAG, pit8253_device, read, write)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff00) AM_WRITE(scanrow_w)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0xff00) AM_READWRITE(comparitor_r, led_w)
	AM_RANGE(0x0a, 0x0a) AM_MIRROR(0xff00) AM_READWRITE(scan_r, potmux_w)
	AM_RANGE(0x0b, 0x0b) AM_MIRROR(0xff00) AM_WRITE(gate_w)
	AM_RANGE(0x0d, 0x0d) AM_MIRROR(0xff00) AM_WRITE(cv_w)
	AM_RANGE(0x0e, 0x0e) AM_MIRROR(0xff00) AM_WRITE(mask_w)
ADDRESS_MAP_END

DRIVER_INIT_MEMBER(prophet600_state, prophet600)
{
}

// master crystal is 8 MHz, all clocks derived from there
static MACHINE_CONFIG_START( prophet600, prophet600_state )
	MCFG_CPU_ADD(MAINCPU_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEFAULT_LAYOUT( layout_prophet600 )

	MCFG_DEVICE_ADD(PIT_TAG, PIT8253, XTAL_8MHz/4)
	MCFG_PIT8253_CLK0(XTAL_8MHz/4)
	MCFG_PIT8253_CLK1(XTAL_8MHz/4)
	MCFG_PIT8253_CLK2(XTAL_8MHz/4)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(prophet600_state, pit_ch0_tick_w))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(prophet600_state, pit_ch2_tick_w)) 

	MCFG_DEVICE_ADD(UART_TAG, ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("mdout", midi_port_device, write_txd))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(prophet600_state, acia_irq_w))

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE(UART_TAG, acia6850_device, write_rxd))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_DEVICE_ADD("acia_clock", CLOCK, XTAL_8MHz/16)	// 500kHz = 16 times the MIDI rate
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(prophet600_state, acia_clock_w))

MACHINE_CONFIG_END

static INPUT_PORTS_START( prophet600 )
INPUT_PORTS_END

ROM_START( prpht600 )
	ROM_REGION(0x2000, MAINCPU_TAG, 0)
	ROM_LOAD( "p600.bin",     0x000000, 0x002000, CRC(78e3f048) SHA1(61548b6de3d9b5c0ae76f8e751ece0b57de17118) )
ROM_END

CONS( 1983, prpht600, 0, 0, prophet600, prophet600, prophet600_state, prophet600, "Sequential Circuits", "Prophet-600", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
