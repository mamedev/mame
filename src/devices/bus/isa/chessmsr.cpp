// license:BSD-3-Clause
// copyright-holders:hap
/*

The ChessMachine SR by Tasc

8-bit ISA card, successor of The Final ChessCard.

I/O is similar to The Final ChessCard, with two 74374 latches, but no ROM.
There's a 74590 counter chip for writing the initial bootstrap. The rest
of the program is sent to RAM via the latches.

VLSI VY86C010-12QC (ARM2), seen with 30MHz XTAL, but XTAL label usually scratched off.
128KB, 512KB, or 1MB RAM. 512KB version probably the most common.
It looks like Gideon 2.1 only sees up to 512KB RAM, The King up to 2MB RAM.
Also seen with VY86C061PSTC (ARM6) @ 32MHz, very rare, aka "Madrid" version.

*/

#include "emu.h"
#include "chessmsr.h"


DEFINE_DEVICE_TYPE(ISA8_CHESSMSR, isa8_chessmsr_device, "isa_chessmsr", "The ChessMachine SR")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

isa8_chessmsr_device::isa8_chessmsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_CHESSMSR, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_mainlatch(*this, "mainlatch"),
	m_sublatch(*this, "sublatch"),
	m_ram(*this, "ram")
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_chessmsr_device::device_start()
{
	set_isa_device();
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_suspended));
	save_item(NAME(m_ram_offset));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_chessmsr_device::device_reset()
{
	if (!m_installed)
	{
		// MAME doesn't allow reading ioport at device_start
		u16 port = ioport("DSW")->read() * 0x40 + 0x10;
		m_isa->install_device(port, port+1, read8_delegate(*this, FUNC(isa8_chessmsr_device::chessmsr_r)), write8_delegate(*this, FUNC(isa8_chessmsr_device::chessmsr_w)));

		m_maincpu->set_unscaled_clock(ioport("CPU")->read() ? (32_MHz_XTAL) : (30_MHz_XTAL/2));

		// install RAM
		u32 ramsize = 1 << ioport("RAM")->read();
		m_ram.allocate(ramsize / 4);
		m_maincpu->space(AS_PROGRAM).install_ram(0, ramsize - 1, m_ram);

		m_installed = true;
	}
}

void isa8_chessmsr_device::device_reset_after_children()
{
	// hold ARM CPU in reset state
	chessmsr_w(machine().dummy_space(), 1, 0);
}



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( chessmsr )
	PORT_START("DSW") // DIP switch on the ISA card PCB, installer shows range 0x110-0x3D0
	PORT_DIPNAME( 0x0f, 0x08, "I/O Port Address" ) PORT_DIPLOCATION("CMSR_SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "0x010 (Invalid)" )
	PORT_DIPSETTING(    0x01, "0x050 (Invalid)" )
	PORT_DIPSETTING(    0x02, "0x090 (Invalid)" )
	PORT_DIPSETTING(    0x03, "0x0D0 (Invalid)" )
	PORT_DIPSETTING(    0x04, "0x110" )
	PORT_DIPSETTING(    0x05, "0x150" )
	PORT_DIPSETTING(    0x06, "0x190" )
	PORT_DIPSETTING(    0x07, "0x1D0" )
	PORT_DIPSETTING(    0x08, "0x210" )
	PORT_DIPSETTING(    0x09, "0x250" )
	PORT_DIPSETTING(    0x0a, "0x290" )
	PORT_DIPSETTING(    0x0b, "0x2D0" )
	PORT_DIPSETTING(    0x0c, "0x310" )
	PORT_DIPSETTING(    0x0d, "0x350" )
	PORT_DIPSETTING(    0x0e, "0x390" )
	PORT_DIPSETTING(    0x0f, "0x3D0" )

	PORT_START("CPU")
	PORT_CONFNAME( 0x01, 0x00, "CPU Type" )
	PORT_CONFSETTING(    0x00, "ARM2 @ 15MHz" )
	PORT_CONFSETTING(    0x01, "ARM6 @ 32MHz" )

	PORT_START("RAM") // setting in 2^x
	PORT_CONFNAME( 0xff, 19, "RAM Size" )
	PORT_CONFSETTING(    17, "128KB" )
	PORT_CONFSETTING(    19, "512KB" )
	PORT_CONFSETTING(    20, "1MB" )
	PORT_CONFSETTING(    21, "2MB" ) // unofficial
INPUT_PORTS_END

ioport_constructor isa8_chessmsr_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(chessmsr);
}



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_chessmsr_device::device_add_mconfig(machine_config &config)
{
	ARM(config, m_maincpu, 30_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &isa8_chessmsr_device::chessmsr_mem);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	GENERIC_LATCH_8(config, m_mainlatch);
	GENERIC_LATCH_8(config, m_sublatch);
	m_sublatch->data_pending_callback().set_inputline(m_maincpu, ARM_FIRQ_LINE);
}



/******************************************************************************
    I/O
******************************************************************************/

// External handlers

READ8_MEMBER(isa8_chessmsr_device::chessmsr_r)
{
	if (offset == 0)
		return m_mainlatch->read();
	else
		return m_mainlatch->pending_r() ? 0 : 2;
}

WRITE8_MEMBER(isa8_chessmsr_device::chessmsr_w)
{
	if (offset == 0)
	{
		if (m_suspended)
			m_maincpu->space(AS_PROGRAM).write_byte(m_ram_offset++, data);
		else
			m_sublatch->write(data);
	}
	else
	{
		// disable CPU, PC side can write to first 256-byte block of RAM when in this state
		m_suspended = bool(~data & 1);
		m_maincpu->set_input_line(INPUT_LINE_RESET, m_suspended ? ASSERT_LINE : CLEAR_LINE);
		m_sublatch->read(); // clear IRQ
		m_ram_offset = 0xff;
	}
}


// Internal (on-card CPU)

void isa8_chessmsr_device::chessmsr_mem(address_map &map)
{
	map(0x00380000, 0x00380000).mirror(0x00000008).r(m_sublatch, FUNC(generic_latch_8_device::read)).w(m_mainlatch, FUNC(generic_latch_8_device::write));
}
