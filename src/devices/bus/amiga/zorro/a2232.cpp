// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A2232

    Zorro-II Serial Card

***************************************************************************/

#include "emu.h"
#include "a2232.h"

#define LOG_DATA (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_DATA)

#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_A2232, bus::amiga::zorro::a2232_device, "amiga_a2232", "Commodore A2232 Serial Card")


namespace bus::amiga::zorro {

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
	device_t(mconfig, AMIGA_A2232, tag, owner, clock),
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

void a2232_device::int2_w(uint8_t data)
{
	LOG("%s: int2_w %04x\n", shortname(), data);

	m_zorro->int2_w(1);
}

void a2232_device::irq_ack8_w(uint8_t data)
{
	LOG("%s: irq_ack_w %04x\n", shortname(), data);

	m_ioirq->in_w<8>(CLEAR_LINE);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void a2232_device::autoconfig_base_address(offs_t address)
{
	LOG("%s: autoconfig_base_address received: 0x%06x\n", shortname(), address);
	LOG("-> installing a2232\n");

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	m_zorro->space().install_readwrite_handler(address, address + 0x3fff,
		read16s_delegate(*this, FUNC(a2232_device::shared_ram_r)),
		write16s_delegate(*this, FUNC(a2232_device::shared_ram_w)), 0xffff);

	m_zorro->space().install_readwrite_handler(address + 0x4000, address + 0x4001,
		read16smo_delegate(*this, FUNC(a2232_device::irq_ack_r)),
		write16smo_delegate(*this, FUNC(a2232_device::irq_ack_w)), 0xffff);

	m_zorro->space().install_readwrite_handler(address + 0x8000, address + 0x8001,
		read16smo_delegate(*this, FUNC(a2232_device::reset_low_r)),
		write16smo_delegate(*this, FUNC(a2232_device::reset_low_w)), 0xffff);

	m_zorro->space().install_readwrite_handler(address + 0xa000, address + 0xa001,
		read16smo_delegate(*this, FUNC(a2232_device::irq_r)),
		write16smo_delegate(*this, FUNC(a2232_device::irq_w)), 0xffff);

	m_zorro->space().install_readwrite_handler(address + 0xc000, address + 0xc001,
		read16s_delegate(*this, FUNC(a2232_device::reset_high_r)),
		write16s_delegate(*this, FUNC(a2232_device::reset_high_w)), 0xffff);

	// we're done
	m_zorro->cfgout_w(0);
}

void a2232_device::cfgin_w(int state)
{
	LOG("%s: cfgin_w (%d)\n", shortname(), state);

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
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}


//**************************************************************************
//  ZORRO
//**************************************************************************

uint16_t a2232_device::shared_ram_r(offs_t offset, uint16_t mem_mask)
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

	LOGMASKED(LOG_DATA, "%s: shared_ram_r(%04x) %04x [mask = %04x]\n", shortname(), offset << 1, data, mem_mask);

	return data;
}

void a2232_device::shared_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DATA, "%s: shared_ram_w(%04x) %04x [mask = %04x]\n", shortname(), offset << 1, data, mem_mask);

	if (ACCESSING_BITS_0_7)
		m_shared_ram[(offset << 1) + 1] = data & 0xff;

	if (ACCESSING_BITS_8_15)
		m_shared_ram[offset << 1] = (data & 0xff00) >> 8;
}

uint16_t a2232_device::irq_ack_r()
{
	m_zorro->int2_w(0);

	return 0xffff;
}

void a2232_device::irq_ack_w(uint16_t data)
{
	m_zorro->int2_w(0);
}

uint16_t a2232_device::reset_low_r()
{
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	return 0xffff;
}

void a2232_device::reset_low_w(uint16_t data)
{
	m_iocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint16_t a2232_device::irq_r()
{
	if (!machine().side_effects_disabled())
		m_ioirq->in_w<8>(ASSERT_LINE);

	return 0xffff;
}

void a2232_device::irq_w(uint16_t data)
{
	m_ioirq->in_w<8>(ASSERT_LINE);
}

uint16_t a2232_device::reset_high_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	LOG("%s: reset_high_r %04x [mask = %04x]\n", shortname(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return data;
}

void a2232_device::reset_high_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s: reset_high_w %04x [mask = %04x]\n", shortname(), data, mem_mask);

	m_iocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}


//**************************************************************************
//  ACIA
//**************************************************************************

template<int N>
uint8_t a2232_device::acia_r(offs_t offset)
{
	return m_acia[N]->read(offset >> 1);
}

template<int N>
void a2232_device::acia_w(offs_t offset, uint8_t data)
{
	m_acia[N]->write(offset >> 1, data);
}


//**************************************************************************
//  CIA
//**************************************************************************

uint8_t a2232_device::cia_r(offs_t offset)
{
	return m_cia->read(offset >> 1);
}

void a2232_device::cia_w(offs_t offset, uint8_t data)
{
	m_cia->write(offset >> 1, data);
}

uint8_t a2232_device::cia_port_a_r()
{
	return m_cia_port_a;
}

uint8_t a2232_device::cia_port_b_r()
{
	return m_cia_port_b;
}

void a2232_device::cia_port_b_w(uint8_t data)
{
	// tod clock connected to pb7
	m_cia->tod_w(BIT(data, 7));
}


//**************************************************************************
//  RS232
//**************************************************************************

void a2232_device::rs232_1_rxd_w(int state)
{
	m_acia[0]->write_rxd(state);
	m_cia->sp_w(state);
}

void a2232_device::rs232_1_dcd_w(int state)
{
	m_cia_port_a &= ~0x01;
	m_cia_port_a |= state << 0;
}

void a2232_device::rs232_1_cts_w(int state)
{
	m_cia_port_b &= ~0x01;
	m_cia_port_b |= state << 0;

	m_cia->cnt_w(state);
}

void a2232_device::rs232_2_dcd_w(int state)
{
	m_cia_port_a &= ~0x02;
	m_cia_port_a |= state << 1;
}

void a2232_device::rs232_2_cts_w(int state)
{
	m_cia_port_b &= ~0x02;
	m_cia_port_b |= state << 1;
}

void a2232_device::rs232_3_dcd_w(int state)
{
	m_cia_port_a &= ~0x04;
	m_cia_port_a |= state << 2;
}

void a2232_device::rs232_3_cts_w(int state)
{
	m_cia_port_b &= ~0x04;
	m_cia_port_b |= state << 2;
}

void a2232_device::rs232_4_dcd_w(int state)
{
	m_cia_port_a &= ~0x08;
	m_cia_port_a |= state << 3;
}

void a2232_device::rs232_4_cts_w(int state)
{
	m_cia_port_b &= ~0x08;
	m_cia_port_b |= state << 3;
}

void a2232_device::rs232_5_dcd_w(int state)
{
	m_cia_port_a &= ~0x10;
	m_cia_port_a |= state << 4;
}

void a2232_device::rs232_5_cts_w(int state)
{
	m_cia_port_b &= ~0x10;
	m_cia_port_b |= state << 4;
}

void a2232_device::rs232_6_dcd_w(int state)
{
	m_cia_port_a &= ~0x20;
	m_cia_port_a |= state << 5;
}

void a2232_device::rs232_6_cts_w(int state)
{
	m_cia_port_b &= ~0x20;
	m_cia_port_b |= state << 5;
}

void a2232_device::rs232_7_dcd_w(int state)
{
	m_cia_port_a &= ~0x40;
	m_cia_port_a |= state << 6;
}

void a2232_device::rs232_7_cts_w(int state)
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

} // namespace bus::amiga::zorro
