// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Intel iSBC series

        09/12/2009 Skeleton driver.

Notes:

isbc86 commands: BYTE WORD REAL EREAL ROMTEST. ROMTEST works, the others hang.

Press capital-U to drop into the monitor on the isbc 86/05 and 86/30
The 86/05 can boot floppies with the b command but appears to mostly be
able to deal with 256byte sectors so fails to load the irmx 512byte sector images.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "machine/74259.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/z80sio.h"
#include "bus/centronics/ctronics.h"
#include "bus/isbx/isbx.h"
#include "machine/isbc_215g.h"
#include "machine/isbc_208.h"

class isbc_state : public driver_device
{
public:
	isbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart8251(*this, "uart8251")
		, m_uart8274(*this, "uart8274")
		, m_pic_0(*this, "pic_0")
		, m_pic_1(*this, "pic_1")
		, m_centronics(*this, "centronics")
		, m_cent_status_in(*this, "cent_status_in")
		, m_statuslatch(*this, "statuslatch")
		, m_bios(*this, "bios")
		, m_biosram(*this, "biosram")
		, m_sbx(*this, "sbx%u", 1U)
	{ }

	void isbc2861(machine_config &config);
	void isbc86(machine_config &config);
	void rpc86(machine_config &config);
	void isbc8605(machine_config &config);
	void isbc286(machine_config &config);
	void isbc8630(machine_config &config);
	void sm1810(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);

	DECLARE_WRITE_LINE_MEMBER(isbc86_tmr2_w);
	DECLARE_WRITE_LINE_MEMBER(isbc286_tmr2_w);
//  DECLARE_WRITE_LINE_MEMBER(isbc_uart8274_irq);
	uint8_t get_slave_ack(offs_t offset);
	void ppi_c_w(uint8_t data);
	void upperen_w(uint8_t data);
	uint16_t bioslo_r(offs_t offset);
	void bioslo_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void edge_intr_clear_w(uint8_t data);
	void status_register_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE_LINE_MEMBER(bus_intr_out1_w);
	DECLARE_WRITE_LINE_MEMBER(bus_intr_out2_w);
	void isbc2861_mem(address_map &map);
	void isbc286_io(address_map &map);
	void isbc286_mem(address_map &map);
	void isbc8605_io(address_map &map);
	void isbc8630_io(address_map &map);
	void isbc86_mem(address_map &map);
	void isbc_io(address_map &map);
	void rpc86_io(address_map &map);
	void rpc86_mem(address_map &map);
	void sm1810_io(address_map &map);

	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	optional_device<i8251_device> m_uart8251;
//  optional_device<i8274_device> m_uart8274;
	optional_device<i8274_device> m_uart8274;
	required_device<pic8259_device> m_pic_0;
	optional_device<pic8259_device> m_pic_1;
	optional_device<centronics_device> m_centronics;
	optional_device<input_buffer_device> m_cent_status_in;
	optional_device<ls259_device> m_statuslatch;
	optional_memory_region m_bios;
	optional_shared_ptr<u16> m_biosram;
	optional_device_array<isbx_slot_device, 2> m_sbx;

	bool m_upperen;
	offs_t m_megabyte_page;
	bool m_nmi_enable;
	bool m_override;
	bool m_megabyte_enable;
};

void isbc_state::machine_reset()
{
	if(m_centronics)
	{
		m_centronics->write_busy(0);  // centronics_device sets busy to 1 at reset causing spurious irqs
		m_pic_1->ir7_w(0);
	}
	if(m_uart8251)
		m_uart8251->write_cts(0);
	m_upperen = false;
	m_megabyte_page = 0;
}

void isbc_state::rpc86_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xcffff).ram();
	map(0xf8000, 0xfffff).rom().region("bios", 0);
}

void isbc_state::rpc86_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0080, 0x008f).rw("sbx1", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0x00ff);
	map(0x0090, 0x009f).rw("sbx1", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0x00ff);
	map(0x00a0, 0x00af).rw("sbx2", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0x00ff);
	map(0x00b0, 0x00bf).rw("sbx2", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0x00ff);
	map(0x00c0, 0x00c3).rw(m_pic_0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c4, 0x00c7).rw(m_pic_0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00d0, 0x00d7).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x00d8, 0x00db).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x00dc, 0x00df).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
}

void isbc_state::isbc8605_io(address_map &map)
{
	rpc86_io(map);
	map(0x0000, 0x002f).m("isbc_208", FUNC(isbc_208_device::map));
}

void isbc_state::isbc8630_io(address_map &map)
{
	rpc86_io(map);
	map(0x00c0, 0x00c7).w(FUNC(isbc_state::edge_intr_clear_w)).umask16(0xff00);
	map(0x00c8, 0x00df).w(FUNC(isbc_state::status_register_w)).umask16(0xff00);
	map(0x0100, 0x0100).w("isbc_215g", FUNC(isbc_215g_device::write));
}

void isbc_state::sm1810_io(address_map &map)
{
	isbc8630_io(map);
	map(0x00ca, 0x00cb).lr8(NAME([]() { return 0x40; })).umask16(0x00ff); // it reads this without configuring the ppi
}

void isbc_state::isbc86_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xfbfff).ram();
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void isbc_state::isbc_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00c0, 0x00c3).rw(m_pic_0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c4, 0x00c7).rw(m_pic_0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00d0, 0x00d7).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x00d8, 0x00db).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x00dc, 0x00df).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
}

void isbc_state::isbc286_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0080, 0x008f).rw("sbx1", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0x00ff);
	map(0x0080, 0x008f).rw("sbx1", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0xff00);
	map(0x0090, 0x009f).rw("sbx1", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0x00ff);
	map(0x0090, 0x009f).rw("sbx1", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0xff00);
	map(0x00a0, 0x00af).rw("sbx2", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0x00ff);
	map(0x00a0, 0x00af).rw("sbx2", FUNC(isbx_slot_device::mcs0_r), FUNC(isbx_slot_device::mcs0_w)).umask16(0xff00);
	map(0x00b0, 0x00bf).rw("sbx2", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0x00ff);
	map(0x00b0, 0x00bf).rw("sbx2", FUNC(isbx_slot_device::mcs1_r), FUNC(isbx_slot_device::mcs1_w)).umask16(0xff00);
	map(0x00c0, 0x00c3).rw(m_pic_0, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c4, 0x00c7).rw(m_pic_1, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).w(FUNC(isbc_state::upperen_w)).umask16(0xff00);
	map(0x00d0, 0x00d7).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0x00d8, 0x00df).rw(m_uart8274, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w)).umask16(0x00ff);
	map(0x0100, 0x0100).w("isbc_215g", FUNC(isbc_215g_device::write));
}

void isbc_state::isbc286_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xdffff).ram();
	map(0xe0000, 0xfffff).rom().region("bios", 0);
	map(0xfe0000, 0xffffff).rom().region("bios", 0);
}

void isbc_state::isbc2861_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xdffff).ram();
	map(0xe0000, 0xfffff).rw(FUNC(isbc_state::bioslo_r), FUNC(isbc_state::bioslo_w)).share("biosram");
//  map(0x100000, 0x1fffff).ram(); // FIXME: XENIX doesn't like this, IRMX is okay with it
	map(0xff0000, 0xffffff).rom().region("bios", 0);
}

/* Input ports */
static INPUT_PORTS_START( isbc )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( isbc86_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_300 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( isbc286_terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

WRITE_LINE_MEMBER( isbc_state::isbc86_tmr2_w )
{
	m_uart8251->write_rxc(state);
	m_uart8251->write_txc(state);
}

uint8_t isbc_state::get_slave_ack(offs_t offset)
{
	if (offset == 7)
		return m_pic_1->acknowledge();

	return 0x00;
}

WRITE_LINE_MEMBER( isbc_state::isbc286_tmr2_w )
{
	m_uart8274->rxca_w(state);
	m_uart8274->txca_w(state);
}

WRITE_LINE_MEMBER( isbc_state::write_centronics_ack )
{
	m_cent_status_in->write_bit4(state);

	if(state)
		m_pic_1->ir7_w(1);
}

void isbc_state::ppi_c_w(uint8_t data)
{
	m_centronics->write_strobe(data & 1);

	if(data & 0x80)
		m_pic_1->ir7_w(0);
}

void isbc_state::upperen_w(uint8_t data)
{
	m_upperen = true;
}

uint16_t isbc_state::bioslo_r(offs_t offset)
{
	if(m_upperen)
		return m_biosram[offset];
	else if(offset >= 0x8000)
		return m_bios->as_u16(offset - 0x8000);
	return 0xffff;
}

void isbc_state::bioslo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(m_upperen)
		COMBINE_DATA(&m_biosram[offset]);
}

#if 0
WRITE_LINE_MEMBER(isbc_state::isbc_uart8274_irq)
{
	m_uart8274->m1_r(); // always set
	m_pic_0->ir6_w(state);
}
#endif

void isbc_state::edge_intr_clear_w(uint8_t data)
{
	// reset U32 flipflop
}

void isbc_state::status_register_w(uint8_t data)
{
	m_megabyte_page = (data & 0xf0) << 16;
	m_statuslatch->write_bit(data & 0x07, BIT(data, 3));
}

WRITE_LINE_MEMBER(isbc_state::nmi_mask_w)
{
	// combined with NMI input by 74LS08 AND gate at U12
	m_nmi_enable = state;
}

WRITE_LINE_MEMBER(isbc_state::bus_intr_out1_w)
{
	// Multibus interrupt request (active high)
}

WRITE_LINE_MEMBER(isbc_state::bus_intr_out2_w)
{
	// Multibus interrupt request (active high)
}

void isbc_state::isbc86(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(5'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &isbc_state::isbc86_mem);
	m_maincpu->set_addrmap(AS_IO, &isbc_state::isbc_io);
	m_maincpu->set_irq_acknowledge_callback("pic_0", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic_0, 0);
	m_pic_0->out_int_callback().set_inputline(m_maincpu, 0);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(XTAL(22'118'400)/18);
	pit.out_handler<0>().set(m_pic_0, FUNC(pic8259_device::ir0_w));
	pit.set_clk<1>(XTAL(22'118'400)/18);
	pit.set_clk<2>(XTAL(22'118'400)/18);
	pit.out_handler<2>().set(FUNC(isbc_state::isbc86_tmr2_w));

	I8255A(config, "ppi");

	I8251(config, m_uart8251, 0);
	m_uart8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart8251->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart8251->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart8251->rxrdy_handler().set("pic_0", FUNC(pic8259_device::ir6_w));

	/* video hardware */
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_uart8251, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_uart8251, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_uart8251, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(isbc86_terminal));
}

void isbc_state::rpc86(machine_config &config)
{
	/* basic machine hardware */
	I8086(config, m_maincpu, XTAL(5'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &isbc_state::rpc86_mem);
	m_maincpu->set_addrmap(AS_IO, &isbc_state::rpc86_io);
	m_maincpu->set_irq_acknowledge_callback("pic_0", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic_0, 0);
	m_pic_0->out_int_callback().set_inputline(m_maincpu, 0);

	pit8253_device &pit(PIT8253(config, "pit", 0));
	pit.set_clk<0>(XTAL(22'118'400)/18);
	pit.out_handler<0>().set(m_pic_0, FUNC(pic8259_device::ir2_w));
	pit.set_clk<1>(XTAL(22'118'400)/144);
	pit.set_clk<2>(XTAL(22'118'400)/18);
	pit.out_handler<2>().set(FUNC(isbc_state::isbc86_tmr2_w));

	I8255A(config, "ppi");

	I8251(config, m_uart8251, 0);
	m_uart8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart8251->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart8251->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart8251->rxrdy_handler().set("pic_0", FUNC(pic8259_device::ir6_w));
	m_uart8251->txrdy_handler().set("pic_0", FUNC(pic8259_device::ir7_w));

	/* video hardware */
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_uart8251, FUNC(i8251_device::write_rxd));
	//rs232.cts_handler().set(m_uart8251, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_uart8251, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(isbc286_terminal));

	ISBX_SLOT(config, m_sbx[0], 0, isbx_cards, nullptr);
	//m_sbx[0]->mintr0().set("pic_0", FUNC(pic8259_device::ir3_w));
	//m_sbx[0]->mintr1().set("pic_0", FUNC(pic8259_device::ir4_w));
	ISBX_SLOT(config, m_sbx[1], 0, isbx_cards, nullptr);
	//m_sbx[1]->mintr0().set("pic_0", FUNC(pic8259_device::ir5_w));
	//m_sbx[1]->mintr1().set("pic_0", FUNC(pic8259_device::ir6_w));
}

void isbc_state::isbc8605(machine_config &config)
{
	rpc86(config);

	m_maincpu->set_addrmap(AS_IO, &isbc_state::isbc8605_io);

	ISBC_208(config, "isbc_208", 0, m_maincpu).irq_callback().set(m_pic_0, FUNC(pic8259_device::ir5_w));
}

void isbc_state::isbc8630(machine_config &config)
{
	rpc86(config);

	m_maincpu->set_addrmap(AS_IO, &isbc_state::isbc8630_io);

	ISBC_215G(config, "isbc_215g", 0, 0x100, m_maincpu).irq_callback().set(m_pic_0, FUNC(pic8259_device::ir5_w));

	LS259(config, m_statuslatch); // U14
//  m_statuslatch->q_out_cb<0>().set("pit", FUNC(pit8253_device::write_gate0));
//  m_statuslatch->q_out_cb<1>().set("pit", FUNC(pit8253_device::write_gate1));
	m_statuslatch->q_out_cb<2>().set(FUNC(isbc_state::nmi_mask_w));
	m_statuslatch->q_out_cb<3>().set([this] (int state) { m_override = state; }); // 1 = access onboard dual-port RAM
	m_statuslatch->q_out_cb<4>().set(FUNC(isbc_state::bus_intr_out1_w));
	m_statuslatch->q_out_cb<5>().set(FUNC(isbc_state::bus_intr_out2_w));
	m_statuslatch->q_out_cb<5>().append_output("led0").invert(); // ds1
	m_statuslatch->q_out_cb<6>().set_output("led1").invert(); // ds3
	m_statuslatch->q_out_cb<7>().set([this] (int state) { m_megabyte_enable = !state; });
}

void isbc_state::isbc286(machine_config &config)
{
	/* basic machine hardware */
	I80286(config, m_maincpu, XTAL(16'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &isbc_state::isbc286_mem);
	m_maincpu->set_addrmap(AS_IO, &isbc_state::isbc286_io);
	m_maincpu->set_irq_acknowledge_callback("pic_0", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic_0, 0);
	m_pic_0->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic_0->in_sp_callback().set_constant(1);
	m_pic_0->read_slave_ack_callback().set(FUNC(isbc_state::get_slave_ack));

	PIC8259(config, m_pic_1, 0);
	m_pic_1->out_int_callback().set(m_pic_0, FUNC(pic8259_device::ir7_w));
	m_pic_1->in_sp_callback().set_constant(0);

	pit8254_device &pit(PIT8254(config, "pit", 0));
	pit.set_clk<0>(XTAL(22'118'400)/18);
	pit.out_handler<0>().set(m_pic_0, FUNC(pic8259_device::ir0_w));
	pit.set_clk<1>(XTAL(22'118'400)/18);
	pit.out_handler<1>().set(m_uart8274, FUNC(i8274_device::rxtxcb_w));
	pit.set_clk<2>(XTAL(22'118'400)/18);
	pit.out_handler<2>().set(FUNC(isbc_state::isbc286_tmr2_w));

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set("cent_data_out", FUNC(output_latch_device::write));
	ppi.in_pb_callback().set(m_cent_status_in, FUNC(input_buffer_device::read));
	ppi.out_pc_callback().set(FUNC(isbc_state::ppi_c_w));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(isbc_state::write_centronics_ack));
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit6));

	INPUT_BUFFER(config, m_cent_status_in, 0);

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

#if 0
	I8274(config, m_uart8274, XTAL(16'000'000)/4);
	m_uart8274->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_uart8274->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	m_uart8274->out_int_callback().set(FUNC(isbc_state::isbc_uart8274_irq));
#else
	I8274(config, m_uart8274, XTAL(16'000'000)/4);
	m_uart8274->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	m_uart8274->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_uart8274->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_uart8274->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
//  m_uart8274->out_int_callback().set(FUNC(isbc_state::isbc_uart8274_irq));
	m_uart8274->out_int_callback().set(m_pic_0, FUNC(pic8259_device::ir6_w));
#endif

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_uart8274, FUNC(i8274_device::rxa_w));
	rs232a.dcd_handler().set(m_uart8274, FUNC(i8274_device::dcda_w));
	rs232a.cts_handler().set(m_uart8274, FUNC(i8274_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set(m_uart8274, FUNC(i8274_device::rxb_w));
	rs232b.dcd_handler().set(m_uart8274, FUNC(i8274_device::dcdb_w));
	rs232b.cts_handler().set(m_uart8274, FUNC(i8274_device::ctsb_w));
	rs232b.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(isbc286_terminal));

	ISBX_SLOT(config, m_sbx[0], 0, isbx_cards, nullptr);
	m_sbx[0]->mintr0().set("pic_1", FUNC(pic8259_device::ir3_w));
	m_sbx[0]->mintr1().set("pic_1", FUNC(pic8259_device::ir4_w));
	ISBX_SLOT(config, m_sbx[1], 0, isbx_cards, nullptr);
	m_sbx[1]->mintr0().set("pic_1", FUNC(pic8259_device::ir5_w));
	m_sbx[1]->mintr1().set("pic_1", FUNC(pic8259_device::ir6_w));

	ISBC_215G(config, "isbc_215g", 0, 0x100, m_maincpu).irq_callback().set(m_pic_0, FUNC(pic8259_device::ir5_w));
}

void isbc_state::isbc2861(machine_config &config)
{
	isbc286(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &isbc_state::isbc2861_mem);
}

void isbc_state::sm1810(machine_config &config)
{
	isbc8630(config);
	m_maincpu->set_clock(XTAL(4'000'000)); // calibrated clock to pass self test
	m_maincpu->set_addrmap(AS_IO, &isbc_state::sm1810_io);

	m_uart8251->dtr_handler().set(m_uart8251, FUNC(i8251_device::write_dsr));
}

/* ROM definition */
ROM_START( isbc86 )
	ROM_REGION16_LE( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "8612_2u.bin", 0x0001, 0x1000, CRC(84fa14cf) SHA1(783e1459ab121201fd49368d4bf769c1bab6447a))
	ROM_LOAD16_BYTE( "8612_2l.bin", 0x0000, 0x1000, CRC(922bda5f) SHA1(15743e69f3aba56425fa004d19b82ec20532fd72))
	ROM_LOAD16_BYTE( "8612_3u.bin", 0x2001, 0x1000, CRC(68d47c3e) SHA1(16c17f26b33daffa84d065ff7aefb581544176bd))
	ROM_LOAD16_BYTE( "8612_3l.bin", 0x2000, 0x1000, CRC(17f27ad2) SHA1(c3f379ac7d67dc4a0a7a611a0bc6323b8a3d4840))
ROM_END

ROM_START( isbc8605 )
	ROM_REGION16_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "i8605mon.bin", 0x4000, 0x4000, CRC(e16acb6e) SHA1(eb9a3fd21f7609d44f8052b6a0603ecbb52dc3f3))
ROM_END

ROM_START( isbc8630 )
	ROM_REGION16_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "14378", "14378" )
	ROMX_LOAD( "143780-001_isdm_for_isbc_86-30_socket_u57_i2732a.bin", 0x4000, 0x1000, CRC(db0ef880) SHA1(8ef296066d16881217618e54b410d12157f318ea), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "143782-001_isdm_for_isbc_86-30_socket_u39_i2732a.bin", 0x4001, 0x1000, CRC(ea1ebe78) SHA1(f03b63659e8f5e96f481dbc6c2ddef1d22850ebb), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "143781-001_isdm_for_isbc_86-30_socket_u58_i2732a.bin", 0x6000, 0x1000, CRC(93732612) SHA1(06e751d0f5ab1fe2c52fd79f6f4725ccf3379791), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "143783-001_isdm_for_isbc_86-30_socket_u40_i2732a.bin", 0x6001, 0x1000, CRC(337102d5) SHA1(535f63d24c3948187b208ea594f979bc33579a15), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "14503", "14503" )
	ROMX_LOAD( "145032-001_u57.bin", 0x0000, 0x2000, CRC(09a24dea) SHA1(e21277f1d4d72e0858846f7293ac48417b392e3b), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "145030-001_u39.bin", 0x0001, 0x2000, CRC(c58f3a98) SHA1(76f6d5be8ea6854a98f6555320cfcdb814e5c633), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "145033-001_u58.bin", 0x4000, 0x2000, CRC(496aca5f) SHA1(c09f4d2254ece1eb139ef5fd4ad0ce6a55376da5), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "145031-001_u40.bin", 0x4001, 0x2000, CRC(150fcd90) SHA1(4bca0f46b9b05ef0124bac5dea09ddd952e73af2), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( isbc286 )
	ROM_REGION16_LE( 0x20000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "u79.bin", 0x00001, 0x10000, CRC(144182ea) SHA1(4620ca205a6ac98fe2636183eaead7c4bfaf7a72))
	ROM_LOAD16_BYTE( "u36.bin", 0x00000, 0x10000, CRC(22db075f) SHA1(fd29ea77f5fc0697c8f8b66aca549aad5b9db3ea))
ROM_END

/*
 * :uart8274 A Reg 00 <- 18 - Channel reset command
 * :uart8274 A Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 A Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, <DTR=0
 * :uart8274 A Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enable
 * :uart8274 B Reg 00 <- 18 - Channel reset command
 * :uart8274 B Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 B Reg 05 <- ea - Tx Enabled, Transmitter Bits/Character 8, Send Break 0, RTS=0, DTR=0
 * :uart8274 B Reg 03 <- c1 - Rx 8 bits, No Auto Enables, Rx Enabled,

 * :uart8274 B Reg 00 <- 18 - Channel reset command
 * :uart8274 B Reg 04 <- 4e - x16 clock, 2 stop bit, even parity but parity disabled
 * :uart8274 B Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 B Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 B Reg 07 <- 00 - Hi SYNC bits
 * :uart8274 B Reg 06 <- 00 - Lo SYNC bits
 * :uart8274 A Reg 02 <- 04 - RTSB selected, non vectored mode, 85-1 mode selected, A over B interleaved int prios
 * :uart8274 B Reg 02 <- 26 - interrupt vector 26
 * :uart8274 B Reg 01 <- 00 - Rx INT/DMA int disabled, no vector modification

 * :uart8274 B Reg 00 <- 18 - Channel reset command
 * :uart8274 B Reg 00 <- 18 - Channel reset command
 * :uart8274 B Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 B Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 B Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 B Reg 00 <- 28 - Reset Transmitter Interrupt Pending
 * :uart8274 B Reg 00 <- 28 - Reset Transmitter Interrupt Pending
 * :uart8274 B Reg 00 <- 28 - Reset Transmitter Interrupt Pending
 * :uart8274 B Reg 00 <- 28 - Reset Transmitter Interrupt Pending

 * :uart8274 A Reg 00 <- 18 - Channel reset command
 * :uart8274 A Reg 04 <- 4e - x16 clock, 2 stop bit, even parity but parity disabled
 * :uart8274 A Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 A Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 A Reg 07 <- 00 - Hi SYNC bits
 * :uart8274 A Reg 06 <- 00 - Lo SYNC bits
 * :uart8274 A Reg 02 <- 04 - RTSB selected, non vectored mode, 85-1 mode selected, A over B interleaved int prios
 * :uart8274 B Reg 02 <- 26 - interrupt vector 26
 * :uart8274 A Reg 01 <- 00 - Rx INT/DMA int disabled, no vector modification

 * :uart8274 A Reg 01 -> ?? - Read out Status Register 1 (Errors and All Sent flag)
 * :uart8274 A Reg 05 <- e2 - Tx Disabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 A Reg 03 <- c0 - Rx Disabled, Rx 8 bits, No Auto Enables
 * :uart8274 A Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 A Reg 04 <- 4e - x16 clock, 2 stop bit, even parity but parity disabled
 * :uart8274 A Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
 * :uart8274 A Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 A Reg 07 <- 00 - Hi SYNC bits
 * :uart8274 A Reg 06 <- 00 - Lo SYNC bits
 * :uart8274 A Reg 02 <- 04 - RTSB selected, non vectored mode, 85-1 mode selected, A over B interleaved int prios
 * :uart8274 B Reg 02 <- 26 - interrupt vector 26
 * :uart8274 A Reg 01 <- 00 - Rx INT/DMA int disabled, no vector modification
 * :uart8274 A Reg 02 <- 04 - RTSB selected, non vectored mode, 85-1 mode selected, A over B interleaved int prios

 * :uart8274 B Reg 02 <- a5 - interrupt vector a5
 * :uart8274 B Reg 02 <- 00 - interrupt vector 0

 * :uart8274 B Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 B Reg 01 <- 1e - Wait disabled, Int mode 3, vector modified, Tx int/DMA enabled
 * :uart8274 A Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 A Reg 01 <- 1e - Wait disabled, Int mode 3, vector modified, Tx int/DMA enabled

 * :uart8274 B Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 B Reg 01 <- 1e - Wait disabled, Int mode 3, vector modified, Tx int/DMA enabled
 * :uart8274 B Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 B Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0

 * :uart8274 B Reg 04 <- 44 - x16 clock, 1 stop bit, no parity
 * :uart8274 B Reg 01 <- 1e - Wait disabled, Int mode 3, vector modified, Tx int/DMA enabled
 * :uart8274 B Reg 03 <- c1 - Rx Enabled, Rx 8 bits, No Auto Enables
 * :uart8274 B Reg 05 <- ea - Tx Enabled, Tx 8 bits, Send Break 0, RTS=0, DTR=0
*/
ROM_START( isbc2861 )
	ROM_REGION16_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v11", "iSDM Monitor V1.1" )
	ROMX_LOAD( "174894-001.bin", 0x0000, 0x4000, CRC(79e4f7af) SHA1(911a4595d35e6e82b1149e75bb027927cd1c1658), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "174894-002.bin", 0x0001, 0x4000, CRC(66747d21) SHA1(4094b1f10a8bc7db8d6dd48d7128e14e875776c7), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "174894-003.bin", 0x8000, 0x4000, CRC(c98c7f17) SHA1(6e9a14aedd630824dccc5eb6052867e73b1d7db6), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "174894-004.bin", 0x8001, 0x4000, CRC(61bc1dc9) SHA1(feed5a5f0bb4630c8f6fa0d5cca30654a80b4ee5), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v10", "iSDM Monitor V1.0" )
	ROMX_LOAD( "rmx286-_in_socket_u41_on_isbc_286-10.bin.u41", 0x0000, 0x4000, CRC(00996834) SHA1(a17a0f8909be642d89199660b24574b71a9d0c13), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "rmx286-_in_socket_u76_on_isbc_286-10.bin.u76", 0x0001, 0x4000, CRC(90c9c7e8) SHA1(b5f961ab236976713266fe7a378e8750825fd5dc), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "rmx286-_in_socket_u40_on_isbc_286-10.bin.u40", 0x8000, 0x4000, CRC(35716c9b) SHA1(5b717b4c2f6c59ec140635df7448294a22123a16), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "rmx286-_in_socket_u75_on_isbc_286-10.bin.u75", 0x8001, 0x4000, CRC(68c3eb50) SHA1(3eeef2676e4fb187adb8ab50645f4bd172426c15), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( isbc28612 )
	ROM_REGION16_LE( 0x10000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "176346-001.bin", 0x0000, 0x8000, CRC(f86c8be5) SHA1(e2bb16b0aeb718219e65d61edabd7838ef34c560))
	ROM_LOAD16_BYTE( "176346-002.bin", 0x0001, 0x8000, CRC(b964c6c3) SHA1(c3de8541182e32b3568fde77da8c435eab397498))
ROM_END

ROM_START( rpc86 )
	ROM_REGION16_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "145068-001.bin", 0x4001, 0x1000, CRC(0fa9db83) SHA1(4a44f8683c263c9ef6850cbe05aaa73f4d4d4e06))
	ROM_LOAD16_BYTE( "145069-001.bin", 0x6001, 0x1000, CRC(1692a076) SHA1(0ce3a4a867cb92340871bb8f9c3e91ce2984c77c))
	ROM_LOAD16_BYTE( "145070-001.bin", 0x4000, 0x1000, CRC(8c8303ef) SHA1(60f94daa76ab9dea6e309ac580152eb212b847a0))
	ROM_LOAD16_BYTE( "145071-001.bin", 0x6000, 0x1000, CRC(a49681d8) SHA1(e81f8b092cfa2d1737854b1fa270a4ce07d61a9f))
ROM_END

ROM_START( sm1810 )
	ROM_REGION16_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sm1810.42-1.06-1.bin", 0x0000, 0x2000, CRC(de8b42e7) SHA1(bb93335b4ef79638f88c38adedfb7dd9ed9d1e31))
	ROM_LOAD16_BYTE( "sm1810.42-1.06-0.bin", 0x0001, 0x2000, CRC(352bb060) SHA1(2112fcbf9903ad8af29a5a8d4b57eaeb5cd74739))
	ROM_LOAD16_BYTE( "sm1810.42-1.06-2.bin", 0x4000, 0x2000, CRC(ae015240) SHA1(2c345a9e0832a0f26493bda394b2c4ad7ada7aad))
	ROM_LOAD16_BYTE( "sm1810.42-1.06-3.bin", 0x4001, 0x2000, CRC(9741a51a) SHA1(c9d3a6a5c51fe9814986517b0f4cbeae8200babc))
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT  CLASS       INIT        COMPANY  FULLNAME       FLAGS */
COMP( 19??, rpc86,     0,      0,      rpc86,    isbc,  isbc_state, empty_init, "Intel", "RPC 86",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1978, isbc86,    0,      0,      isbc86,   isbc,  isbc_state, empty_init, "Intel", "iSBC 86/12A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1981, isbc8605,  0,      0,      isbc8605, isbc,  isbc_state, empty_init, "Intel", "iSBC 86/05",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1981, isbc8630,  0,      0,      isbc8630, isbc,  isbc_state, empty_init, "Intel", "iSBC 86/30",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 19??, isbc286,   0,      0,      isbc286,  isbc,  isbc_state, empty_init, "Intel", "iSBC 286",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1983, isbc2861,  0,      0,      isbc2861, isbc,  isbc_state, empty_init, "Intel", "iSBC 286/10", MACHINE_NO_SOUND_HW)
COMP( 1983, isbc28612, 0,      0,      isbc2861, isbc,  isbc_state, empty_init, "Intel", "iSBC 286/12", MACHINE_NO_SOUND_HW)
COMP( 19??, sm1810,    0,      0,      sm1810,   isbc,  isbc_state, empty_init, "<unknown>", "SM1810",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
