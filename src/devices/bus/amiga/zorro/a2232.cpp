// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2232

    Zorro-II Serial Card

***************************************************************************/

#include "emu.h"
#include "a2232.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 0
#define VERBOSE_DATA 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(A2232, a2232_device, "a2232", "CBM A2232 Serial Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2232_device::device_add_mconfig(machine_config &config)
{
	// main cpu
	M65CE02(config, m_iocpu, 28.37516_MHz_XTAL / 8); // should run at Amiga clock 7M / 2
	m_iocpu->set_addrmap(AS_PROGRAM, &a2232_device::iocpu_map);

	INPUT_MERGER_ANY_HIGH(config, m_ioirq);
	m_ioirq->output_handler().set_inputline(m_iocpu, m65ce02_device::IRQ_LINE);

	// acia
	for (auto &acia : m_acia)
		MOS6551(config, acia, 28.37516_MHz_XTAL / 8).set_xtal(1.8432_MHz_XTAL);
	m_acia[0]->txd_handler().set("rs232_1", FUNC(rs232_port_device::write_txd));
	m_acia[0]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<0>));
	m_acia[1]->txd_handler().set("rs232_2", FUNC(rs232_port_device::write_txd));
	m_acia[1]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<1>));
	m_acia[2]->txd_handler().set("rs232_3", FUNC(rs232_port_device::write_txd));
	m_acia[2]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<2>));
	m_acia[3]->txd_handler().set("rs232_4", FUNC(rs232_port_device::write_txd));
	m_acia[3]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<3>));
	m_acia[4]->txd_handler().set("rs232_5", FUNC(rs232_port_device::write_txd));
	m_acia[4]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<4>));
	m_acia[5]->txd_handler().set("rs232_6", FUNC(rs232_port_device::write_txd));
	m_acia[5]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<5>));
	m_acia[6]->txd_handler().set("rs232_7", FUNC(rs232_port_device::write_txd));
	m_acia[6]->irq_handler().set(m_ioirq, FUNC(input_merger_device::in_w<6>));

	// cia
	MOS8520(config, m_cia, 1.8432_MHz_XTAL);
	m_cia->irq_wr_callback().set(m_ioirq, FUNC(input_merger_device::in_w<7>));
	m_cia->pa_rd_callback().set(FUNC(a2232_device::cia_port_a_r));
	m_cia->pb_rd_callback().set(FUNC(a2232_device::cia_port_b_r));
	m_cia->pb_wr_callback().set(FUNC(a2232_device::cia_port_b_w));

	// rs232 ports
	rs232_port_device &rs232_1(RS232_PORT(config, "rs232_1", default_rs232_devices, nullptr));
	rs232_1.rxd_handler().set(FUNC(a2232_device::rs232_1_rxd_w));
	rs232_1.dcd_handler().set(FUNC(a2232_device::rs232_1_dcd_w));
	rs232_1.dsr_handler().set(m_acia[0], FUNC(mos6551_device::write_dsr));
	rs232_1.cts_handler().set(FUNC(a2232_device::rs232_1_cts_w));

	rs232_port_device &rs232_2(RS232_PORT(config, "rs232_2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set(m_acia[1], FUNC(mos6551_device::write_rxd));
	rs232_2.dcd_handler().set(FUNC(a2232_device::rs232_2_dcd_w));
	rs232_2.dsr_handler().set(m_acia[1], FUNC(mos6551_device::write_dsr));
	rs232_2.cts_handler().set(FUNC(a2232_device::rs232_2_cts_w));

	rs232_port_device &rs232_3(RS232_PORT(config, "rs232_3", default_rs232_devices, nullptr));
	rs232_3.rxd_handler().set(m_acia[2], FUNC(mos6551_device::write_rxd));
	rs232_3.dcd_handler().set(FUNC(a2232_device::rs232_3_dcd_w));
	rs232_3.dsr_handler().set(m_acia[2], FUNC(mos6551_device::write_dsr));
	rs232_3.cts_handler().set(FUNC(a2232_device::rs232_3_cts_w));

	rs232_port_device &rs232_4(RS232_PORT(config, "rs232_4", default_rs232_devices, nullptr));
	rs232_4.rxd_handler().set(m_acia[3], FUNC(mos6551_device::write_rxd));
	rs232_4.dcd_handler().set(FUNC(a2232_device::rs232_4_dcd_w));
	rs232_4.dsr_handler().set(m_acia[3], FUNC(mos6551_device::write_dsr));
	rs232_4.cts_handler().set(FUNC(a2232_device::rs232_4_cts_w));

	rs232_port_device &rs232_5(RS232_PORT(config, "rs232_5", default_rs232_devices, nullptr));
	rs232_5.rxd_handler().set(m_acia[4], FUNC(mos6551_device::write_rxd));
	rs232_5.dcd_handler().set(FUNC(a2232_device::rs232_5_dcd_w));
	rs232_5.dsr_handler().set(m_acia[4], FUNC(mos6551_device::write_dsr));
	rs232_5.cts_handler().set(FUNC(a2232_device::rs232_5_cts_w));

	rs232_port_device &rs232_6(RS232_PORT(config, "rs232_6", default_rs232_devices, nullptr));
	rs232_6.rxd_handler().set(m_acia[5], FUNC(mos6551_device::write_rxd));
	rs232_6.dcd_handler().set(FUNC(a2232_device::rs232_6_dcd_w));
	rs232_6.dsr_handler().set(m_acia[5], FUNC(mos6551_device::write_dsr));
	rs232_6.cts_handler().set(FUNC(a2232_device::rs232_6_cts_w));

	rs232_port_device &rs232_7(RS232_PORT(config, "rs232_7", default_rs232_devices, nullptr));
	rs232_7.rxd_handler().set(m_acia[6], FUNC(mos6551_device::write_rxd));
	rs232_7.dcd_handler().set(FUNC(a2232_device::rs232_7_dcd_w));
	rs232_7.dsr_handler().set(m_acia[6], FUNC(mos6551_device::write_dsr));
	rs232_7.cts_handler().set(FUNC(a2232_device::rs232_7_cts_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2232_device - constructor
//-------------------------------------------------

a2232_device::a2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2232, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_iocpu(*this, "iocpu"),
	m_ioirq(*this, "ioirq"),
	m_acia(*this, "acia_%u", 0U),
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
}

//-------------------------------------------------
//  device_reset_after_children - reset after child devices
//-------------------------------------------------

void a2232_device::device_reset_after_children()
{
	// reset is kept high at reset, to allow the amiga time to upload its code
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// cts connected to gnd
	for (auto &acia : m_acia)
		acia->write_cts(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE8_MEMBER( a2232_device::int2_w )
{
	if (VERBOSE)
		logerror("%s('%s'): int2_w %04x\n", shortname(), basetag(), data);

	m_slot->int2_w(1);
}

WRITE8_MEMBER( a2232_device::irq_ack8_w )
{
	if (VERBOSE)
		logerror("%s('%s'): irq_ack_w %04x\n", shortname(), basetag(), data);

	m_ioirq->in_w<8>(CLEAR_LINE);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void a2232_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname(), basetag(), address);

	if (VERBOSE)
		logerror("-> installing a2232\n");

	// stop responding to default autoconfig
	m_slot->space().unmap_readwrite(0xe80000, 0xe8007f);

	m_slot->space().install_readwrite_handler(address, address + 0x3fff,
			read16_delegate(*this, FUNC(a2232_device::shared_ram_r)),
			write16_delegate(*this, FUNC(a2232_device::shared_ram_w)), 0xffff);

	m_slot->space().install_readwrite_handler(address + 0x4000, address + 0x4001,
			read16_delegate(*this, FUNC(a2232_device::irq_ack_r)),
			write16_delegate(*this, FUNC(a2232_device::irq_ack_w)), 0xffff);

	m_slot->space().install_readwrite_handler(address + 0x8000, address + 0x8001,
			read16_delegate(*this, FUNC(a2232_device::reset_low_r)),
			write16_delegate(*this, FUNC(a2232_device::reset_low_w)), 0xffff);

	m_slot->space().install_readwrite_handler(address + 0xa000, address + 0xa001,
			read16_delegate(*this, FUNC(a2232_device::irq_r)),
			write16_delegate(*this, FUNC(a2232_device::irq_w)), 0xffff);

	m_slot->space().install_readwrite_handler(address + 0xc000, address + 0xc001,
			read16_delegate(*this, FUNC(a2232_device::reset_high_r)),
			write16_delegate(*this, FUNC(a2232_device::reset_high_w)), 0xffff);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( a2232_device::cfgin_w )
{
	if (VERBOSE)
		logerror("%s('%s'): configin_w (%d)\n", shortname(), basetag(), state);

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
		m_slot->space().install_readwrite_handler(0xe80000, 0xe8007f,
				read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
				write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}


//**************************************************************************
//  ZORRO
//**************************************************************************

READ16_MEMBER( a2232_device::shared_ram_r )
{
	uint16_t data = 0;

	if (ACCESSING_BITS_0_7)
		data |= m_shared_ram[(offset << 1) + 1];
	else
		data |= 0x00ff;

	if (ACCESSING_BITS_8_15)
		data |= m_shared_ram[offset << 1] << 8;
	else
		data |= 0xff00;

	if (VERBOSE_DATA)
		logerror("%s('%s'): shared_ram_r(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset << 1, data, mem_mask);

	return data;
}

WRITE16_MEMBER( a2232_device::shared_ram_w )
{
	if (VERBOSE_DATA)
		logerror("%s('%s'): shared_ram_w(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset << 1, data, mem_mask);

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
	if (!machine().side_effects_disabled())
		m_ioirq->in_w<8>(ASSERT_LINE);

	return 0xffff;
}

WRITE16_MEMBER( a2232_device::irq_w )
{
	m_ioirq->in_w<8>(ASSERT_LINE);
}

READ16_MEMBER( a2232_device::reset_high_r )
{
	uint16_t data = 0xffff;

	if (VERBOSE)
		logerror("%s('%s'): reset_high_r %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return data;
}

WRITE16_MEMBER( a2232_device::reset_high_w )
{
	if (VERBOSE)
		logerror("%s('%s'): reset_high_w %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//**************************************************************************
//  ACIA
//**************************************************************************

template<int N>
READ8_MEMBER( a2232_device::acia_r )
{
	return m_acia[N]->read(offset >> 1);
}

template<int N>
WRITE8_MEMBER( a2232_device::acia_w )
{
	m_acia[N]->write(offset >> 1, data);
}


//**************************************************************************
//  CIA
//**************************************************************************

READ8_MEMBER( a2232_device::cia_r )
{
	return m_cia->read(offset >> 1);
}

WRITE8_MEMBER( a2232_device::cia_w )
{
	m_cia->write(offset >> 1, data);
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
	m_acia[0]->write_rxd(state);
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



void a2232_device::iocpu_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("shared");
	map(0x4000, 0x47ff).rw(FUNC(a2232_device::acia_r<0>), FUNC(a2232_device::acia_w<0>));
	map(0x4800, 0x4fff).rw(FUNC(a2232_device::acia_r<1>), FUNC(a2232_device::acia_w<1>));
	map(0x5000, 0x57ff).rw(FUNC(a2232_device::acia_r<2>), FUNC(a2232_device::acia_w<2>));
	map(0x5800, 0x5fff).rw(FUNC(a2232_device::acia_r<3>), FUNC(a2232_device::acia_w<3>));
	map(0x6000, 0x67ff).rw(FUNC(a2232_device::acia_r<4>), FUNC(a2232_device::acia_w<4>));
	map(0x6800, 0x6fff).rw(FUNC(a2232_device::acia_r<5>), FUNC(a2232_device::acia_w<5>));
	map(0x7000, 0x73ff).w(FUNC(a2232_device::int2_w));
	map(0x7400, 0x77ff).rw(FUNC(a2232_device::acia_r<6>), FUNC(a2232_device::acia_w<6>));
	map(0x7800, 0x7fff).rw(FUNC(a2232_device::cia_r), FUNC(a2232_device::cia_w));
	map(0x8000, 0x8000).w(FUNC(a2232_device::irq_ack8_w));
	map(0xc000, 0xffff).ram().share("shared");
}
