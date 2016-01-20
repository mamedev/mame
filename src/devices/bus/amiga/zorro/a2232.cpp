// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2232

    Zorro-II Serial Card

***************************************************************************/

#include "a2232.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 0
#define VERBOSE_DATA 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type A2232 = &device_creator<a2232_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static ADDRESS_MAP_START( iocpu_map, AS_PROGRAM, 8, a2232_device)
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("shared")
	AM_RANGE(0x4000, 0x47ff) AM_READWRITE(acia_0_r, acia_0_w)
	AM_RANGE(0x4800, 0x4fff) AM_READWRITE(acia_1_r, acia_1_w)
	AM_RANGE(0x5000, 0x57ff) AM_READWRITE(acia_2_r, acia_2_w)
	AM_RANGE(0x5800, 0x5fff) AM_READWRITE(acia_3_r, acia_3_w)
	AM_RANGE(0x6000, 0x67ff) AM_READWRITE(acia_4_r, acia_4_w)
	AM_RANGE(0x6800, 0x6fff) AM_READWRITE(acia_5_r, acia_5_w)
	AM_RANGE(0x7000, 0x73ff) AM_WRITE(int2_w)
	AM_RANGE(0x7400, 0x77ff) AM_READWRITE(acia_6_r, acia_6_w)
	AM_RANGE(0x7800, 0x7fff) AM_READWRITE(cia_r, cia_w)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(irq_ack_w)
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_SHARE("shared")
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a2232 )
	// main cpu
	MCFG_CPU_ADD("iocpu", M65CE02, XTAL_28_37516MHz / 8) // should run at Amiga clock 7M / 2
	MCFG_CPU_PROGRAM_MAP(iocpu_map)

	// acia
	MCFG_DEVICE_ADD("acia_0", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_1", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_0_irq_w))

	MCFG_DEVICE_ADD("acia_1", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_2", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_1_irq_w))

	MCFG_DEVICE_ADD("acia_2", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_3", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_2_irq_w))

	MCFG_DEVICE_ADD("acia_3", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_4", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_3_irq_w))

	MCFG_DEVICE_ADD("acia_4", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_5", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_4_irq_w))

	MCFG_DEVICE_ADD("acia_5", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_6", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_5_irq_w))

	MCFG_DEVICE_ADD("acia_6", MOS6551, XTAL_28_37516MHz / 8)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
	MCFG_MOS6551_TXD_HANDLER(DEVWRITELINE("rs232_7", rs232_port_device, write_txd))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(a2232_device, acia_6_irq_w))

	// cia
	MCFG_DEVICE_ADD("cia", MOS8520, XTAL_1_8432MHz)
	MCFG_MOS6526_IRQ_CALLBACK(WRITELINE(a2232_device, cia_irq_w))
	MCFG_MOS6526_PA_INPUT_CALLBACK(READ8(a2232_device, cia_port_a_r))
	MCFG_MOS6526_PB_INPUT_CALLBACK(READ8(a2232_device, cia_port_b_r))
	MCFG_MOS6526_PB_OUTPUT_CALLBACK(WRITE8(a2232_device, cia_port_b_w))

	// rs232 ports
	MCFG_RS232_PORT_ADD("rs232_1", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(a2232_device, rs232_1_rxd_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_1_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_0", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_1_cts_w))

	MCFG_RS232_PORT_ADD("rs232_2", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_1", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_2_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_1", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_2_cts_w))

	MCFG_RS232_PORT_ADD("rs232_3", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_2", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_3_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_2", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_3_cts_w))

	MCFG_RS232_PORT_ADD("rs232_4", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_3", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_4_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_3", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_4_cts_w))

	MCFG_RS232_PORT_ADD("rs232_5", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_4", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_5_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_4", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_5_cts_w))

	MCFG_RS232_PORT_ADD("rs232_6", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_5", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_6_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_5", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_6_cts_w))

	MCFG_RS232_PORT_ADD("rs232_7", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia_6", mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(a2232_device, rs232_7_dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("acia_6", mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(a2232_device, rs232_7_cts_w))
MACHINE_CONFIG_END

machine_config_constructor a2232_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2232 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2232_device - constructor
//-------------------------------------------------

a2232_device::a2232_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2232, "CBM A2232 Serial Card", tag, owner, clock, "a2232", __FILE__),
	device_zorro2_card_interface(mconfig, *this),
	m_iocpu(*this, "iocpu"),
	m_acia_0(*this, "acia_0"),
	m_acia_1(*this, "acia_1"),
	m_acia_2(*this, "acia_2"),
	m_acia_3(*this, "acia_3"),
	m_acia_4(*this, "acia_4"),
	m_acia_5(*this, "acia_5"),
	m_acia_6(*this, "acia_6"),
	m_cia(*this, "cia"),
	m_shared_ram(*this, "shared"),
	m_cia_port_a(0xff),
	m_cia_port_b(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2232_device::device_start()
{
	set_zorro_device();
	memset(m_irqs, 0, sizeof(m_irqs));
}

//-------------------------------------------------
//  device_reset_after_children - reset after child devices
//-------------------------------------------------

void a2232_device::device_reset_after_children()
{
	// reset is kept high at reset, to allow the amiga time to upload its code
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// cts connected to gnd
	m_acia_0->write_cts(0);
	m_acia_1->write_cts(0);
	m_acia_2->write_cts(0);
	m_acia_3->write_cts(0);
	m_acia_4->write_cts(0);
	m_acia_5->write_cts(0);
	m_acia_6->write_cts(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void a2232_device::update_irqs()
{
	// look for any active irq
	for (auto & elem : m_irqs)
	{
		if (elem)
		{
			m_iocpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
			return;
		}
	}

	// if we get here no irqs are pending
	m_iocpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

WRITE8_MEMBER( a2232_device::int2_w )
{
	if (VERBOSE)
		logerror("%s('%s'): int2_w %04x\n", shortname().c_str(), basetag().c_str(), data);

	m_slot->int2_w(1);
}

WRITE8_MEMBER( a2232_device::irq_ack_w )
{
	if (VERBOSE)
		logerror("%s('%s'): irq_ack_w %04x\n", shortname().c_str(), basetag().c_str(), data);

	m_irqs[IRQ_AMIGA] = CLEAR_LINE;
	update_irqs();
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void a2232_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname().c_str(), basetag().c_str(), address);

	if (VERBOSE)
		logerror("-> installing a2232\n");

	// stop responding to default autoconfig
	m_slot->m_space->unmap_readwrite(0xe80000, 0xe8007f);

	m_slot->m_space->install_readwrite_handler(address, address + 0x3fff,
		read16_delegate(FUNC(a2232_device::shared_ram_r), this),
		write16_delegate(FUNC(a2232_device::shared_ram_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0x4000, address + 0x4001,
		read16_delegate(FUNC(a2232_device::irq_ack_r), this),
		write16_delegate(FUNC(a2232_device::irq_ack_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0x8000, address + 0x8001,
		read16_delegate(FUNC(a2232_device::reset_low_r), this),
		write16_delegate(FUNC(a2232_device::reset_low_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0xa000, address + 0xa001,
		read16_delegate(FUNC(a2232_device::irq_r), this),
		write16_delegate(FUNC(a2232_device::irq_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0xc000, address + 0xc001,
		read16_delegate(FUNC(a2232_device::reset_high_r), this),
		write16_delegate(FUNC(a2232_device::reset_high_w), this), 0xffff);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( a2232_device::cfgin_w )
{
	if (VERBOSE)
		logerror("%s('%s'): configin_w (%d)\n", shortname().c_str(), basetag().c_str(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);

		autoconfig_product(0x46);
		autoconfig_manufacturer(0x0202);
		autoconfig_serial(0x00000000);

		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true); // ?

		// install autoconfig handler
		m_slot->m_space->install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
			write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);
	}
}


//**************************************************************************
//  ZORRO
//**************************************************************************

READ16_MEMBER( a2232_device::shared_ram_r )
{
	UINT16 data = 0;

	if (ACCESSING_BITS_0_7)
		data |= m_shared_ram[(offset << 1) + 1];
	else
		data |= 0x00ff;

	if (ACCESSING_BITS_8_15)
		data |= m_shared_ram[offset << 1] << 8;
	else
		data |= 0xff00;

	if (VERBOSE_DATA)
		logerror("%s('%s'): shared_ram_r(%04x) %04x [mask = %04x]\n", shortname().c_str(), basetag().c_str(), offset << 1, data, mem_mask);

	return data;
}

WRITE16_MEMBER( a2232_device::shared_ram_w )
{
	if (VERBOSE_DATA)
		logerror("%s('%s'): shared_ram_w(%04x) %04x [mask = %04x]\n", shortname().c_str(), basetag().c_str(), offset << 1, data, mem_mask);

	if (ACCESSING_BITS_0_7)
		m_shared_ram[(offset << 1) + 1] = data & 0xff;

	if (ACCESSING_BITS_8_15)
		m_shared_ram[offset << 1] = (data & 0xff00) >> 8;
}

READ16_MEMBER( a2232_device::irq_ack_r )
{
	m_slot->int2_w(0);

	return 0xffff;
}

WRITE16_MEMBER( a2232_device::irq_ack_w )
{
	m_slot->int2_w(0);
}

READ16_MEMBER( a2232_device::reset_low_r )
{
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	return 0xffff;
}

WRITE16_MEMBER( a2232_device::reset_low_w )
{
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

READ16_MEMBER( a2232_device::irq_r )
{
	m_irqs[IRQ_AMIGA] = ASSERT_LINE;
	update_irqs();

	return 0xffff;
}

WRITE16_MEMBER( a2232_device::irq_w )
{
	m_irqs[IRQ_AMIGA] = ASSERT_LINE;
	update_irqs();
}

READ16_MEMBER( a2232_device::reset_high_r )
{
	UINT16 data = 0xffff;

	if (VERBOSE)
		logerror("%s('%s'): reset_high_r %04x [mask = %04x]\n", shortname().c_str(), basetag().c_str(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return data;
}

WRITE16_MEMBER( a2232_device::reset_high_w )
{
	if (VERBOSE)
		logerror("%s('%s'): reset_high_w %04x [mask = %04x]\n", shortname().c_str(), basetag().c_str(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//**************************************************************************
//  ACIA
//**************************************************************************

READ8_MEMBER( a2232_device::acia_0_r )
{
	return m_acia_0->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_0_w )
{
	m_acia_0->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_0_irq_w )
{
	m_irqs[IRQ_ACIA_0] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_1_r )
{
	return m_acia_1->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_1_w )
{
	m_acia_1->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_1_irq_w )
{
	m_irqs[IRQ_ACIA_1] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_2_r )
{
	return m_acia_2->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_2_w )
{
	m_acia_2->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_2_irq_w )
{
	m_irqs[IRQ_ACIA_2] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_3_r )
{
	return m_acia_3->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_3_w )
{
	m_acia_3->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_3_irq_w )
{
	m_irqs[IRQ_ACIA_3] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_4_r )
{
	return m_acia_4->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_4_w )
{
	m_acia_4->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_4_irq_w )
{
	m_irqs[IRQ_ACIA_4] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_5_r )
{
	return m_acia_5->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_5_w )
{
	m_acia_5->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_5_irq_w )
{
	m_irqs[IRQ_ACIA_5] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::acia_6_r )
{
	return m_acia_6->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::acia_6_w )
{
	m_acia_6->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::acia_6_irq_w )
{
	m_irqs[IRQ_ACIA_6] = state;
	update_irqs();
}


//**************************************************************************
//  CIA
//**************************************************************************

READ8_MEMBER( a2232_device::cia_r )
{
	return m_cia->read(space, offset >> 1);
}

WRITE8_MEMBER( a2232_device::cia_w )
{
	m_cia->write(space, offset >> 1, data);
}

WRITE_LINE_MEMBER( a2232_device::cia_irq_w )
{
	m_irqs[IRQ_CIA] = state;
	update_irqs();
}

READ8_MEMBER( a2232_device::cia_port_a_r )
{
	return m_cia_port_a;
}

READ8_MEMBER( a2232_device::cia_port_b_r )
{
	return m_cia_port_b;
}

WRITE8_MEMBER( a2232_device::cia_port_b_w )
{
	// tod clock connected to pb7
	m_cia->tod_w(BIT(data, 7));
}


//**************************************************************************
//  RS232
//**************************************************************************

WRITE_LINE_MEMBER( a2232_device::rs232_1_rxd_w )
{
	m_acia_0->write_rxd(state);
	m_cia->sp_w(state);
}

WRITE_LINE_MEMBER( a2232_device::rs232_1_dcd_w )
{
	m_cia_port_a &= ~0x01;
	m_cia_port_a |= state << 0;
}

WRITE_LINE_MEMBER( a2232_device::rs232_1_cts_w )
{
	m_cia_port_b &= ~0x01;
	m_cia_port_b |= state << 0;

	m_cia->cnt_w(state);
}

WRITE_LINE_MEMBER( a2232_device::rs232_2_dcd_w )
{
	m_cia_port_a &= ~0x02;
	m_cia_port_a |= state << 1;
}

WRITE_LINE_MEMBER( a2232_device::rs232_2_cts_w )
{
	m_cia_port_b &= ~0x02;
	m_cia_port_b |= state << 1;
}

WRITE_LINE_MEMBER( a2232_device::rs232_3_dcd_w )
{
	m_cia_port_a &= ~0x04;
	m_cia_port_a |= state << 2;
}

WRITE_LINE_MEMBER( a2232_device::rs232_3_cts_w )
{
	m_cia_port_b &= ~0x04;
	m_cia_port_b |= state << 2;
}

WRITE_LINE_MEMBER( a2232_device::rs232_4_dcd_w )
{
	m_cia_port_a &= ~0x08;
	m_cia_port_a |= state << 3;
}

WRITE_LINE_MEMBER( a2232_device::rs232_4_cts_w )
{
	m_cia_port_b &= ~0x08;
	m_cia_port_b |= state << 3;
}

WRITE_LINE_MEMBER( a2232_device::rs232_5_dcd_w )
{
	m_cia_port_a &= ~0x10;
	m_cia_port_a |= state << 4;
}

WRITE_LINE_MEMBER( a2232_device::rs232_5_cts_w )
{
	m_cia_port_b &= ~0x10;
	m_cia_port_b |= state << 4;
}

WRITE_LINE_MEMBER( a2232_device::rs232_6_dcd_w )
{
	m_cia_port_a &= ~0x20;
	m_cia_port_a |= state << 5;
}

WRITE_LINE_MEMBER( a2232_device::rs232_6_cts_w )
{
	m_cia_port_b &= ~0x20;
	m_cia_port_b |= state << 5;
}

WRITE_LINE_MEMBER( a2232_device::rs232_7_dcd_w )
{
	m_cia_port_a &= ~0x40;
	m_cia_port_a |= state << 6;
}

WRITE_LINE_MEMBER( a2232_device::rs232_7_cts_w )
{
	m_cia_port_b &= ~0x40;
	m_cia_port_b |= state << 6;
}
