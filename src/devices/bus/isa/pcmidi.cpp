// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    PC MIDI Card
    Copyright (C) 1989 Music Quest, Inc.
    FCC ID: IH9MQ9

    This is one of several Z8681-based MIDI cards by Music Quest. Its
    interface is more or less compatible with the Roland MPU401.

****************************************************************************

Music Quest PC MIDI CARD IH9MQ9 component list:

C1  : 2.2uf 50v electrolytic capacitor
C2  : 100nf tantalum capacitor
C3  : 100nf tantalum capacitor
C4  : 100nf tantalum capacitor
C5  : 100nf tantalum capacitor
C6  : 100nf tantalum capacitor
C7  : 100nf tantalum capacitor
C9  : 100nf tantalum capacitor

D1  : diode (IN4148 ?)
Y1  : 12,000 MHz crystal 2 pin

R1  : 15K ohm resistor +-5%
R2  : 390 ohm resistor +-5%

RP1 : 10K ohm Commoned (?) resistor network 2% SIP package (code used A103GA)
RP2 : 220 ohm Commoned (?) resistor network 2% SIP package (code used B221GA)



U1  : 74LS374N
U2  : Zilog Z8 ROMLESS MCU Z0868112PSC
U3  : 74LS139N
U4  : 74LS74AN
U5  : PAL-16L8-25N - read protected , unknown code
U6  : OTP firmware rom chip compatible with 27C512 eproms/eeproms , 64kbyte
U7  : HYUNDAI HY611ALP-10 or compatible 2K x8bits CMOS SRAM IC
U8  : 74LS125AN
U9  : 74LS374N
U10 : 74LS374N
U11 : SHARP PC900V or compatible photocoupler

***************************************************************************/

#include "emu.h"
#include "pcmidi.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_PCMIDI, isa8_pcmidi_device, "isa_pcmidi", "Music Quest PC MIDI Card (IHQMQ9)")


//**************************************************************************
//  DEVICE SETUP
//**************************************************************************

//-------------------------------------------------
//  isa8_pcmidi_device - constructor
//-------------------------------------------------

isa8_pcmidi_device::isa8_pcmidi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ISA8_PCMIDI, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_mpu(*this, "mpu")
	, m_cmdlatch(*this, "cmdlatch")
	, m_statlatch(*this, "statlatch")
	, m_midiout(*this, "midiout")
	, m_config(*this, "CONFIG")
	, m_mpu_p3(0)
	, m_host_irq(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_pcmidi_device::device_start()
{
	set_isa_device();

	save_item(NAME(m_mpu_p3));
	save_item(NAME(m_host_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_pcmidi_device::device_reset()
{
	offs_t ioaddr = BIT(m_config->read(), 0) ? 0x300 : 0x330;
	m_isa->install_device(ioaddr, ioaddr + 1,
			read8sm_delegate(*this, FUNC(isa8_pcmidi_device::host_r)),
			write8sm_delegate(*this, FUNC(isa8_pcmidi_device::host_w)));

	set_host_irq(false);
}


//**************************************************************************
//  LATCHES AND IRQS
//**************************************************************************

//-------------------------------------------------
//  set_host_irq - update the host IRQ level
//-------------------------------------------------

void isa8_pcmidi_device::set_host_irq(bool state)
{
	if (m_host_irq != state)
	{
		m_host_irq = state;

		ioport_value config = m_config->read();
		if (!BIT(config, 1))
			m_isa->irq3_w(state);
		if (!BIT(config, 2))
			m_isa->irq5_w(state);
		if (!BIT(config, 3))
			m_isa->irq7_w(state);
		if (!BIT(config, 4))
			m_isa->irq2_w(state);
	}
}


//-------------------------------------------------
//  host_r - read from data or status port
//-------------------------------------------------

u8 isa8_pcmidi_device::host_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		set_host_irq(false);
		m_mpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	return m_statlatch->read();
}


//-------------------------------------------------
//  host_w - write to data or command port
//-------------------------------------------------

void isa8_pcmidi_device::host_w(offs_t offset, u8 data)
{
	m_cmdlatch->write(data);

	// IRQs are latched and acknowledged internally
	m_mpu->pulse_input_line(offset ? INPUT_LINE_IRQ0 : INPUT_LINE_IRQ2, attotime::zero);
}


//-------------------------------------------------
//  status_w - mailbox write from local processor
//-------------------------------------------------

void isa8_pcmidi_device::status_w(u8 data)
{
	m_statlatch->write(data);
	if (!BIT(m_mpu_p3, 5))
	{
		set_host_irq(true);
		m_mpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  mpu_p3_w - write to output port
//-------------------------------------------------

void isa8_pcmidi_device::mpu_p3_w(u8 data)
{
	m_midiout->write_txd(BIT(data, 7));

	// P36 = error LED output?

	if (BIT(m_mpu_p3, 5) && !BIT(data, 5))
		set_host_irq(false);

	m_mpu_p3 = data;
}


//-------------------------------------------------
//  mpu_map - Z8681 address map
//-------------------------------------------------

void isa8_pcmidi_device::mpu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("microcode", 0);
	map(0x6000, 0x7fff).rom().mirror(0x8000).region("microcode", 0);
	map(0xa000, 0xa7ff).ram();
	map(0xc000, 0xc000).mirror(0xff).r(m_cmdlatch, FUNC(generic_latch_8_device::read)).w(FUNC(isa8_pcmidi_device::status_w));
}


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static INPUT_PORTS_START(isa_pcmidi)
	PORT_START("CONFIG") // dual inline block of jumpers with idiosyncratic numbering
	PORT_DIPNAME(0x01, 0x00, "I/O Address (P)") PORT_DIPLOCATION("J1:1")
	PORT_DIPSETTING(0x01, "300")
	PORT_DIPSETTING(0x00, "330")
	PORT_DIPNAME(0x1e, 0x0e, "Interrupt") PORT_DIPLOCATION("J1:2,3,4,5")
	PORT_DIPSETTING(0x0e, "IRQ2")
	PORT_DIPSETTING(0x1c, "IRQ3")
	PORT_DIPSETTING(0x1a, "IRQ5")
	PORT_DIPSETTING(0x16, "IRQ7")
INPUT_PORTS_END

ioport_constructor isa8_pcmidi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(isa_pcmidi);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_pcmidi_device::device_add_mconfig(machine_config &config)
{
	Z8681(config, m_mpu, 12_MHz_XTAL);
	m_mpu->set_addrmap(AS_PROGRAM, &isa8_pcmidi_device::mpu_map);
	m_mpu->p3_out_cb().set(FUNC(isa8_pcmidi_device::mpu_p3_w));

	GENERIC_LATCH_8(config, m_cmdlatch);
	GENERIC_LATCH_8(config, m_statlatch);

	MIDI_PORT(config, m_midiout, midiout_slot, "midiout");
	MIDI_PORT(config, "midiin", midiin_slot, "midiin").rxd_handler().set_inputline(m_mpu, INPUT_LINE_IRQ3).invert();
}


//**************************************************************************
//  ROM DEFINITION
//**************************************************************************

ROM_START(isa_pcmidi)
	ROM_REGION(0x10000, "microcode", 0)
	ROM_LOAD("ih9mq9_firmware_v010.bin", 0x00000, 0x10000, CRC(d4cab098) SHA1(9446c4939de557f839a74ca6fe7f41df46752e25))
ROM_END

const tiny_rom_entry *isa8_pcmidi_device::device_rom_region() const
{
	return ROM_NAME(isa_pcmidi);
}
