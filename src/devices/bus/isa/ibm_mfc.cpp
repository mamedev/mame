// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

  ISA 8 bit IBM PC Music Feature Card

  TODO:
   - YM-2164
   - MIDI
   - IRQ/base address selection

  Notes:
   - Some software does not function correctly at higher CPU speeds
  (e.g. the Sierra games and Yamaha Compose/PlayRec)

***************************************************************************/


#include "emu.h"
#include "ibm_mfc.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/pit8253.h"
#include "speaker.h"


//-------------------------------------------------
//  Constants
//-------------------------------------------------

#define TCR_TAC                 0x01
#define TCR_TBC                 0x02
#define TCR_TAE                 0x04
#define TCR_TBE                 0x08
#define TCR_EXT8                0x10
#define TCR_TMSK                0x40
#define TCR_IBE                 0x80

#define TSR_TAS                 0x01
#define TSR_TBS                 0x02
#define TSR_TCS                 0x80

enum
{
	PC_IRQ_TIMERA,
	PC_IRQ_TIMERB,
	PC_IRQ_RXRDY,
	PC_IRQ_TXRDY
};

enum
{
	Z80_IRQ_YM,
	Z80_IRQ_RXRDY,
	Z80_IRQ_TXRDY,
	Z80_IRQ_MIDI_RXRDY,
	Z80_IRQ_MIDI_TXRDY
};


//-------------------------------------------------
//  Globals
//-------------------------------------------------

DEFINE_DEVICE_TYPE(ISA8_IBM_MFC, isa8_ibm_mfc_device, "ibm_mfc", "IBM PC Music Feature Card")


//-------------------------------------------------
//  Interrupt handling
//-------------------------------------------------

void isa8_ibm_mfc_device::set_pc_interrupt(int src, int state)
{
	if (state)
		m_pc_irq_state |= 1 << src;
	else
		m_pc_irq_state &= ~(1 << src);

	update_pc_interrupts();
}

void isa8_ibm_mfc_device::update_pc_interrupts(void)
{
	// IRQs enabled?
	if (m_tcr & TCR_IBE)
	{
		// IRQs unmasked?
		if (m_tcr & TCR_TMSK)
		{
			m_isa->irq3_w(m_pc_irq_state ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

void isa8_ibm_mfc_device::set_z80_interrupt(int src, int state)
{
	if (state)
		m_z80_irq_state |= 1 << src;
	else
		m_z80_irq_state &= ~(1 << src);

	update_z80_interrupts();
}

void isa8_ibm_mfc_device::update_z80_interrupts(void)
{
	m_cpu->set_input_line(0, m_z80_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  Z80 memory map
//-------------------------------------------------

void isa8_ibm_mfc_device::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8000).ram(); // Unknown - tested on startup
	map(0xbfff, 0xbfff).ram(); // Unknown - tested on startup
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xffff).ram();
}

void isa8_ibm_mfc_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x10, 0x10).rw("d71051", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x11, 0x11).rw("d71051", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x20, 0x23).rw("d71055c_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


//-------------------------------------------------
//  Jumpers and DIP switches
//-------------------------------------------------

static INPUT_PORTS_START( ibm_mfc )
	PORT_START("J1")
	PORT_DIPNAME( 0x07, 0x03, "IBM MFC J1: IRQ" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, "IBM MFC SW1: Base Address" )
	PORT_DIPSETTING(    0x00, "2A00" )
	PORT_DIPSETTING(    0x01, "2A10" )
	PORT_DIPSETTING(    0x02, "2A20" )
	PORT_DIPSETTING(    0x03, "2A30" )
INPUT_PORTS_END

//-------------------------------------------------
//  D71055C PPI (PC)
//-------------------------------------------------

READ8_MEMBER( isa8_ibm_mfc_device::ppi0_i_a )
{
	// Read data from the Z80 PIU
	return m_d71055c_1->pa_r();
}

WRITE8_MEMBER( isa8_ibm_mfc_device::ppi0_o_b )
{
	// Write data to the Z80 PIU - no action required
}

WRITE8_MEMBER( isa8_ibm_mfc_device::ppi0_o_c )
{
	// PC Port B /OBF (C1) -> Z80 Port B /STB (C2)
	m_d71055c_1->pc2_w(BIT(data, 1));

	// PC Port A IBF (C5) -> Z80 Port A /ACK (C6)
#if 0 // TODO
	m_d71055c_1->pc6_w(!BIT(data, 5));
#else
	if (!BIT(data, 5) && BIT(m_pc_ppi_c, 5))
		m_d71055c_1->pc6_w(0);
#endif

	// Bit 0 (INTRB) is TxRDY
	set_pc_interrupt(PC_IRQ_TXRDY, BIT(data, 0));

	// Bit 3 (INTRA) is RxRDY
	set_pc_interrupt(PC_IRQ_RXRDY, BIT(data, 3));

	m_pc_ppi_c = data;
}

READ8_MEMBER( isa8_ibm_mfc_device::ppi0_i_c )
{
	// Receive data bit 8
	return BIT(m_z80_ppi_c, 5) << 7;
}

//-------------------------------------------------
//  D71055C PPI (Z80)
//-------------------------------------------------

WRITE8_MEMBER( isa8_ibm_mfc_device::ppi1_o_a )
{
	// Write data to the PC PIU - no action required
}

READ8_MEMBER( isa8_ibm_mfc_device::ppi1_i_b )
{
	// Read data from the PC PIU
	return m_d71055c_0->pb_r();
}

WRITE8_MEMBER( isa8_ibm_mfc_device::ppi1_o_c )
{
	// PortA /OBF (C7) -> PortA /STB (C2)
	m_d71055c_0->pc4_w(BIT(data, 7));

	// PortB IBF (C1) -> PortB /ACK (C2)
#if 0 // TODO
	m_d71055c_0->pc2_w(!BIT(data, 1));
#else
	if (!BIT(data, 1) && BIT(m_z80_ppi_c, 1))
		m_d71055c_0->pc2_w(0);
#endif

	set_z80_interrupt(Z80_IRQ_TXRDY, BIT(data, 3));
	set_z80_interrupt(Z80_IRQ_RXRDY, BIT(data, 0));

	m_z80_ppi_c = data;
}

//-------------------------------------------------
//  D8253 PIT
//-------------------------------------------------

WRITE_LINE_MEMBER( isa8_ibm_mfc_device::d8253_out0 )
{
	if (m_tcr & TCR_TAE)
		set_pc_interrupt(PC_IRQ_TIMERA, 1);
}

WRITE_LINE_MEMBER( isa8_ibm_mfc_device::d8253_out1 )
{
	if (m_tcr & TCR_TBE)
		set_pc_interrupt(PC_IRQ_TIMERB, 1);
}


//-------------------------------------------------
//  uPD71051 USART
//-------------------------------------------------

WRITE_LINE_MEMBER( isa8_ibm_mfc_device::write_usart_clock )
{
	m_d71051->write_txc(state);
	m_d71051->write_rxc(state);
}

//-------------------------------------------------
//  YM-2164
//-------------------------------------------------


WRITE_LINE_MEMBER(isa8_ibm_mfc_device::ibm_mfc_ym_irq)
{
	set_z80_interrupt(Z80_IRQ_YM, state);
}


//-------------------------------------------------
//  ISA interface
//-------------------------------------------------

READ8_MEMBER( isa8_ibm_mfc_device::ibm_mfc_r )
{
	uint8_t val;

	switch (offset)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			val = m_d71055c_0->read(offset);
			break;
		}

		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
		{
			val = (m_pc_irq_state ? 0x80 : 0) | (m_pc_irq_state & 3);
			break;
		}

		default:
		{
			fatalerror("Unhandled IBM MFC read from %d\n", offset);
		}
	}

	return val;
}

WRITE8_MEMBER( isa8_ibm_mfc_device::ibm_mfc_w )
{
	switch (offset)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(1000));
			m_d71055c_0->write(offset, data);
			break;
		}

		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		{
			m_d8253->write(offset & 3, data);
			break;
		}

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
		{
			m_tcr = data;

			if (~m_tcr & TCR_TAC)
				set_pc_interrupt(PC_IRQ_TIMERA, 0);

			if (~m_tcr & TCR_TBC)
				set_pc_interrupt(PC_IRQ_TIMERB, 0);

			m_d71051->write_dsr((m_tcr & TCR_EXT8) ? 1 : 0);

			break;
		}

		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
		{
			// TSR is read-only but Yamaha software attempts to write to it
			break;
		}
	}
}


//-------------------------------------------------
//  ROM definition
//-------------------------------------------------

ROM_START( ibm_mfc )
	ROM_REGION( 0x8000, "ibm_mfc", 0 )
	ROM_LOAD( "xc215 c 0.bin", 0x0000, 0x8000, CRC(28c58a4f) SHA1(e7edf28d20e6c146e3144526c89cd6beea64663b) )
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_ibm_mfc_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, XTAL(11'800'000) / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &isa8_ibm_mfc_device::prg_map);
	m_cpu->set_addrmap(AS_IO, &isa8_ibm_mfc_device::io_map);

	I8255(config, m_d71055c_0);
	m_d71055c_0->in_pa_callback().set(FUNC(isa8_ibm_mfc_device::ppi0_i_a));
	m_d71055c_0->out_pb_callback().set(FUNC(isa8_ibm_mfc_device::ppi0_o_b));
	m_d71055c_0->in_pc_callback().set(FUNC(isa8_ibm_mfc_device::ppi0_i_c));
	m_d71055c_0->out_pc_callback().set(FUNC(isa8_ibm_mfc_device::ppi0_o_c));

	I8255(config, m_d71055c_1);
	m_d71055c_1->out_pa_callback().set(FUNC(isa8_ibm_mfc_device::ppi1_o_a));
	m_d71055c_1->in_pb_callback().set(FUNC(isa8_ibm_mfc_device::ppi1_i_b));
	m_d71055c_1->out_pc_callback().set(FUNC(isa8_ibm_mfc_device::ppi1_o_c));

	I8251(config, m_d71051, 0);

	clock_device &usart_clock(CLOCK(config, "usart_clock", XTAL(4'000'000) / 8)); // 500KHz
	usart_clock.signal_handler().set(FUNC(isa8_ibm_mfc_device::write_usart_clock));

	PIT8253(config, m_d8253, 0);
	m_d8253->set_clk<0>(XTAL(4'000'000) / 8);
	m_d8253->out_handler<0>().set(FUNC(isa8_ibm_mfc_device::d8253_out0));
	m_d8253->set_clk<1>(0);
	m_d8253->out_handler<1>().set(FUNC(isa8_ibm_mfc_device::d8253_out1));
	m_d8253->set_clk<2>(XTAL(4'000'000) / 2);
	m_d8253->out_handler<2>().set(m_d8253, FUNC(pit8253_device::write_clk1));

	SPEAKER(config, "ymleft").front_left();
	SPEAKER(config, "ymright").front_right();
	YM2151(config, m_ym2151, XTAL(4'000'000));
	m_ym2151->irq_handler().set(FUNC(isa8_ibm_mfc_device::ibm_mfc_ym_irq));
	m_ym2151->add_route(0, "ymleft", 1.00);
	m_ym2151->add_route(1, "ymright", 1.00);
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa8_ibm_mfc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm_mfc );
}


//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_ibm_mfc_device::device_rom_region() const
{
	return ROM_NAME( ibm_mfc );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_ibm_mfc_device - constructor
//-------------------------------------------------

isa8_ibm_mfc_device::isa8_ibm_mfc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_IBM_MFC, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_tcr(0), m_pc_ppi_c(0), m_z80_ppi_c(0), m_pc_irq_state(0), m_z80_irq_state(0),
	m_cpu(*this, "ibm_mfc"),
	m_ym2151(*this, "ym2151"),
	m_d8253(*this, "d8253"),
	m_d71051(*this, "d71051"),
	m_d71055c_0(*this, "d71055c_0"),
	m_d71055c_1(*this, "d71055c_1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ibm_mfc_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x2a20, 0x2a20 + 15, read8_delegate(FUNC(isa8_ibm_mfc_device::ibm_mfc_r), this), write8_delegate(FUNC(isa8_ibm_mfc_device::ibm_mfc_w), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_ibm_mfc_device::device_reset()
{
	m_tcr = 0;
	m_d71051->write_dsr(0);
	m_pc_irq_state = 0;
	m_z80_irq_state = 0;
}
