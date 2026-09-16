// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1526/MPS-802/4023 Printer emulation

	MPS-802 is the European model number, CBM 4023 is the IEEE-488 version

**********************************************************************/

#include "emu.h"
#include "c1526.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6504_TAG "u7d"
#define M6532_U4D_TAG "u4d"
#define M6532_U5D_TAG "u5d"
#define M6522_U6D_TAG "u6d"
#define BITMAP_PRINTER_TAG "bitmap"

// 640 dot columns max (80 chars * 8), 6/9 lines per inch -- see manual spec page
#define C1526_PAPER_WIDTH  680
#define C1526_PAPER_HEIGHT 792
#define C1526_HDPI 80
#define C1526_VDPI 72

// nominal spacing between carriage position sensor pulses at printing speed,
static constexpr attotime C1526_CR_STEP_PERIOD = attotime::from_usec(1500);

// The head's home flag sits this many dot columns to the right of the first
// printable column: the firmware seeks left until the home sensor asserts and
// then backs off before it starts a line, so without this the first character
// of every line is struck at a negative x and clipped away.
#define C1526_LEFT_MARGIN 4

// 6 lines per inch in character mode, i.e. 12 rows at 72 vdpi per line feed
#define C1526_PF_RATIO0 1
#define C1526_PF_RATIO1 6



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1526, c1526_device, "c1526", "Commodore 1526/MPS-802 Dot Matrix Printer")
DEFINE_DEVICE_TYPE(GPIB_C4023, c4023_device, "c4023", "Commodore 4023 Printer")


//-------------------------------------------------
//  ROM( c1526 )
//-------------------------------------------------

ROM_START( c1526 )
	ROM_REGION( 0x4000, M6504_TAG, 0 )
	ROM_DEFAULT_BIOS("r07c")
	ROM_SYSTEM_BIOS( 0, "r05", "Revision 5" )
	ROMX_LOAD( "325341-05.u8d", 0x0000, 0x2000, CRC(3ef63c59) SHA1(a71be83a476d2777d33dddb0103c036a047975ba), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "mps802_07b", "MPS802 Revision 7b" )
	ROMX_LOAD( "mps802-341-07b.bin", 0x0000, 0x2000, CRC(e6daa984) SHA1(d24c03e66353337352129451c0c0db8e4bda64e2), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r07b", "Revision 7b (Swe/Fin)" )
	ROMX_LOAD( "cbm 1526 vers. 1.0 skand.gen.u8d", 0x0000, 0x2000, CRC(21051f69) SHA1(7e622fc39985ebe9333d2b546b3c85fd6ab17a53), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "mps802_07b_swe", "MPS802 Revision 7b (Swedish)" )
	ROMX_LOAD( "mps802_rev_7b_swedish.bin", 0x0000, 0x2000, CRC(b35dadef) SHA1(363fb3fc8b25204856f29058b3e3827b1bf9ba60), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "r07c", "Revision 7c" )
	ROMX_LOAD( "325341-08.u8d", 0x0000, 0x2000, CRC(38f85b4a) SHA1(25880091979b21fdaf713b53ef2f1cb8063a3505), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "grafik", "MPS802 GrafikROM II v60.12" )
	ROMX_LOAD( "mps802 grafikrom ii v60.12.u8d", 0x0000, 0x2000, CRC(9f5e6b18) SHA1(8b7f620a8f85e250b142d72b812a67fd0e292d68), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "switchgfx", "MPS802 Switchable GraphicsROM" )
	ROMX_LOAD( "mps802_switchable_gfx.bin", 0x0000, 0x4000, CRC(def777f3) SHA1(5865fadab6caccf315888d03bf0ffb2fb415c059), ROM_BIOS(6) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1526_device::device_rom_region() const
{
	return ROM_NAME( c1526 );
}


//-------------------------------------------------
//  ROM( c4023 )
//-------------------------------------------------

ROM_START( c4023 )
	ROM_REGION( 0x2000, M6504_TAG, 0 )
	ROM_LOAD( "325360-03.u8d", 0x0000, 0x2000, CRC(c6bb0977) SHA1(7a8c43d2e205f58d83709c04bc7795602a892ddd) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c4023_device::device_rom_region() const
{
	return ROM_NAME( c4023 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1526_mem )
//-------------------------------------------------

void c1526_device_base::c1526_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0100).m(m_riot0, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x0100).m(m_riot1, FUNC(mos6532_device::ram_map));
	map(0x0200, 0x021f).mirror(0x0020).m(m_riot0, FUNC(mos6532_device::io_map));
	map(0x0240, 0x024f).mirror(0x0030).m(m_via0, FUNC(via6522_device::map));
	map(0x0280, 0x029f).mirror(0x0020).m(m_riot1, FUNC(mos6532_device::io_map));
	map(0x0400, 0x1fff).rom().region(M6504_TAG, 0x0400);
}


//-------------------------------------------------
//  add_common_mconfig - RIOT/VIA/mechanism wiring
//  shared by both the serial (1526/MPS-802) and
//  IEEE-488 (4023) variants
//-------------------------------------------------

void c1526_device_base::add_common_mconfig(machine_config &config)
{
	M6504(config, m_maincpu, XTAL(4'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1526_device_base::c1526_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MOS6532(config, m_riot0, XTAL(4'000'000)/4);
	m_riot0->irq_wr_callback().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	MOS6532(config, m_riot1, XTAL(4'000'000)/4);
	m_riot1->irq_wr_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_riot1->pa_rd_callback().set(FUNC(c1526_device_base::riot1_pa_r));
	m_riot1->pa_wr_callback().set(FUNC(c1526_device_base::riot1_pa_w));
	m_riot1->pb_wr_callback().set(FUNC(c1526_device_base::riot1_pb_w));

	MOS6522(config, m_via0, XTAL(4'000'000)/4);
	m_via0->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	m_via0->writepa_handler().set(FUNC(c1526_device_base::via0_pa_w));
	m_via0->writepb_handler().set(FUNC(c1526_device_base::via0_pb_w));

	BITMAP_PRINTER(config, m_bitmap_printer, C1526_PAPER_WIDTH, C1526_PAPER_HEIGHT, C1526_HDPI, C1526_VDPI);
	m_bitmap_printer->set_cr_stepper_ratio(1, 2);
	m_bitmap_printer->set_pf_stepper_ratio(C1526_PF_RATIO0, C1526_PF_RATIO1);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1526_device::device_add_mconfig(machine_config &config)
{
	add_common_mconfig(config);

	m_riot0->pa_rd_callback().set(FUNC(c1526_device::riot0_pa_r));
	m_riot0->pa_wr_callback().set(FUNC(c1526_device::riot0_pa_w));
	m_riot0->pb_rd_callback().set(FUNC(c1526_device::riot0_pb_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c4023_device::device_add_mconfig(machine_config &config)
{
	add_common_mconfig(config);

	m_riot0->pa_rd_callback().set(FUNC(c4023_device::riot0_pa_r));
	m_riot0->pa_wr_callback().set(FUNC(c4023_device::riot0_pa_w));
	m_riot0->pb_rd_callback().set(FUNC(c4023_device::riot0_pb_dio_r));
	m_riot0->pb_wr_callback().set(FUNC(c4023_device::riot0_pb_dio_w));
}


//-------------------------------------------------
//  INPUT_PORTS( c1526 )
//-------------------------------------------------

static INPUT_PORTS_START( c1526 )
	PORT_START("PAPERADV")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Paper Advance") PORT_CODE(KEYCODE_INSERT)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1526_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1526 );
}


//-------------------------------------------------
//  INPUT_PORTS( c4023 )
//-------------------------------------------------

static INPUT_PORTS_START( c4023 )
	PORT_START("PAPERADV")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Paper Advance") PORT_CODE(KEYCODE_INSERT)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c4023_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c4023 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1526_device_base - constructor
//-------------------------------------------------

c1526_device_base::c1526_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_maincpu(*this, M6504_TAG),
	m_riot0(*this, M6532_U4D_TAG),
	m_riot1(*this, M6532_U5D_TAG),
	m_via0(*this, M6522_U6D_TAG),
	m_irqs(*this, "irqs"),
	m_bitmap_printer(*this, BITMAP_PRINTER_TAG),
	m_pf_stepper(*this, BITMAP_PRINTER_TAG":pf_stepper"),
	m_paper_adv(*this, "PAPERADV")
{
}


//-------------------------------------------------
//  c1526_device - constructor
//-------------------------------------------------

c1526_device::c1526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1526_device_base(mconfig, C1526, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  c4023_device - constructor
//-------------------------------------------------

c4023_device::c4023_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1526_device_base(mconfig, GPIB_C4023, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1526_device_base::device_start()
{
	m_cr_sensor_timer = timer_alloc(FUNC(c1526_device_base::cr_sensor_tick), this);

	save_item(NAME(m_u5d_pa_out));
	save_item(NAME(m_u5d_pb_out));
	save_item(NAME(m_u6d_pa_out));
	save_item(NAME(m_u6d_pb_out));
	save_item(NAME(m_t_signal));
	save_item(NAME(m_home));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1526_device_base::device_reset()
{
	m_cr_sensor_timer->reset();
	m_last_cr_step = attotime::zero;

	m_t_signal = 1;
	m_via0->write_ca1(m_t_signal);

	// the carriage sits at the home position out of reset
	m_home = 0;
	m_via0->write_ca2(m_home);
}


//-------------------------------------------------
//  device_reset_after_children -
//-------------------------------------------------

void c1526_device_base::device_reset_after_children()
{
	m_pf_stepper->set_absolute_position(m_bitmap_printer->get_top_margin() * C1526_PF_RATIO1 / C1526_PF_RATIO0);
}


//-------------------------------------------------
//  riot1_pa_r -
//-------------------------------------------------

uint8_t c1526_device_base::riot1_pa_r()
{
	/*

	    bit     description

	    0
	    1
	    2       PAPER OUT SENSE
	    3       PAPER ADVANCE BUTTON
	    4
	    5
	    6       CARRIAGE POSITION SENSOR
	    7       HOME POSITION SENSOR

	*/

	uint8_t data = 0;

	data |= 1 << 2;
	data |= !BIT(m_paper_adv->read(), 0) << 3;
	data |= m_t_signal << 6;
	data |= m_home << 7;

	return data;
}


//-------------------------------------------------
//  riot1_pa_w -
//-------------------------------------------------

void c1526_device_base::riot1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       PF (paper feed) motor hold
	    1       CA (carriage) motor hold
	    2
	    3
	    4
	    5       ERROR/PAPER-OUT LED
	    6
	    7

	*/

	m_u5d_pa_out = data;

	m_bitmap_printer->set_led_state(bitmap_printer_device::LED_ERROR, BIT(data, 5));
}


//-------------------------------------------------
//  riot1_pb_w -
//-------------------------------------------------

void c1526_device_base::riot1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       ND0
	    1       ND1
	    2       ND2
	    3       ND3
	    4       ND4
	    5       ND5
	    6       ND6
	    7       ND7

	*/

	m_u5d_pb_out = data;
}


//-------------------------------------------------
//  via0_pa_w -
//-------------------------------------------------

void c1526_device_base::via0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0       HM0 -- carriage stepper phase
	    1       HM1
	    2       HM2
	    3       HM3
	    4       PM0 -- paper-feed stepper phase
	    5       PM1
	    6       PM2
	    7       PM3

	*/

	uint8_t const last = m_u6d_pa_out;
	m_u6d_pa_out = data;

	if ((data & 0x0f) != (last & 0x0f))
	{
		int const last_xpos = m_bitmap_printer->m_xpos;

		m_bitmap_printer->update_cr_stepper(bitswap<4>(data, 0, 1, 2, 3));

		update_cr_sensors(m_bitmap_printer->m_xpos != last_xpos);
	}

	if ((data & 0xf0) != (last & 0xf0))
		m_bitmap_printer->update_pf_stepper((data >> 4) & 0x0f);
}


//-------------------------------------------------
//  via0_pb_w -
//-------------------------------------------------

void c1526_device_base::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       needle strobe -- triggers the U8B one-shot
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	if (BIT(m_u6d_pb_out, 0) && !BIT(data, 0))
		fire_needles();

	m_u6d_pb_out = data;
}


//-------------------------------------------------
//  fire_needles - stamp one dot column
//-------------------------------------------------

void c1526_device_base::fire_needles()
{
	int const x = m_bitmap_printer->m_xpos + C1526_LEFT_MARGIN
			+ ((m_bitmap_printer->m_cr_direction < 0) ? 1 : 0);
	int const y = m_bitmap_printer->m_ypos;

	if (x < 0 || y < 0)
		return;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_u5d_pb_out, i))
			m_bitmap_printer->draw_pixel(x, y + 7 - i, 0x000000);
	}
}


//-------------------------------------------------
//  update_cr_sensors -
//-------------------------------------------------

void c1526_device_base::update_cr_sensors(bool stepped)
{
	// home position sensor: idle high, pulled low while the carriage sits at
	// the far left home position
	int const home = (m_bitmap_printer->m_xpos > 0) ? 1 : 0;

	if (home != m_home)
	{
		m_home = home;
		m_via0->write_ca2(m_home);
	}

	if (!stepped)
		return;

	attotime const now = machine().time();
	attotime const interval = std::min(now - m_last_cr_step, C1526_CR_STEP_PERIOD);

	m_last_cr_step = now;

	set_t_signal(0);
	m_cr_sensor_timer->adjust(interval / 2);
}


//-------------------------------------------------
//  set_t_signal -
//-------------------------------------------------

void c1526_device_base::set_t_signal(int state)
{
	if (state == m_t_signal)
		return;

	m_t_signal = state;
	m_via0->write_ca1(m_t_signal);
}


//-------------------------------------------------
//  cr_sensor_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(c1526_device_base::cr_sensor_tick)
{
	set_t_signal(1);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1526_device::device_start()
{
	c1526_device_base::device_start();

	save_item(NAME(m_attn_ack));
	save_item(NAME(m_nrfd));
}


//-------------------------------------------------
//  riot0_pa_r -
//-------------------------------------------------

uint8_t c1526_device::riot0_pa_r()
{
	/*

	    bit     description

	    0       ATTN IN
	    1       CLK IN
	    2
	    3
	    4
	    5
	    6
	    7       DATA IN

	*/

	uint8_t data = 0;

	data |= !m_bus->atn_r();
	data |= !m_bus->clk_r() << 1;
	data |= !m_bus->data_r() << 7;

	return data;
}


//-------------------------------------------------
//  riot0_pa_w -
//-------------------------------------------------

void c1526_device::riot0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       ATTN ACK
	    6       NRFD
	    7

	*/

	m_attn_ack = BIT(data, 5);
	m_nrfd = BIT(data, 6);

	update_iec_data();
}


//-------------------------------------------------
//  riot0_pb_r -
//-------------------------------------------------

uint8_t c1526_device::riot0_pb_r()
{
	/*

	    bit     description

	    0       device address jumper bit 0
	    1       device address jumper bit 1
	    2       device address jumper bit 2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0xf8 | ((m_slot->get_address() - 4) & 0x07);
}


//-------------------------------------------------
//  update_iec_data -
//-------------------------------------------------

void c1526_device::update_iec_data()
{
	int const attn_ack = !m_bus->atn_r() && !m_attn_ack;

	m_bus->data_w(this, (m_nrfd || attn_ack) ? 0 : 1);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1526_device::cbm_iec_atn(int state)
{
	update_iec_data();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1526_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1526_device::cbm_iec_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, !state);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c4023_device::device_start()
{
	c1526_device_base::device_start();

	save_item(NAME(m_rfd_flag));
	save_item(NAME(m_dac_flag));
}


//-------------------------------------------------
//  riot0_pa_r -
//-------------------------------------------------

uint8_t c4023_device::riot0_pa_r()
{
	/*

	    bit     description

	    0       EOI IN (direct)
	    1
	    2
	    3       device address jumper bit 0
	    4       device address jumper bit 1
	    5       device address jumper bit 2
	    6       DAV IN (direct)
	    7       ATN IN (direct)

	*/

	int const addr = m_slot->get_address() - 4;

	uint8_t data = 0;

	data |= m_bus->eoi_r();
	data |= BIT(addr, 0) << 3;
	data |= BIT(addr, 1) << 4;
	data |= BIT(addr, 2) << 5;
	data |= m_bus->dav_r() << 6;
	data |= m_bus->atn_r() << 7;

	return data;
}


//-------------------------------------------------
//  riot0_pa_w -
//-------------------------------------------------

void c4023_device::riot0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       RFD control
	    2       DAC control
	    3
	    4
	    5
	    6
	    7

	*/

	m_rfd_flag = BIT(data, 1);
	m_dac_flag = BIT(data, 2);

	update_ieee();
}


//-------------------------------------------------
//  update_ieee -
//-------------------------------------------------

void c4023_device::update_ieee()
{
	bool const dac_asserted = bool(m_dac_flag);
	bool const rfd_asserted = !dac_asserted && !m_rfd_flag;

	m_bus->ndac_w(this, dac_asserted ? 0 : 1);
	m_bus->nrfd_w(this, rfd_asserted ? 0 : 1);
}


//-------------------------------------------------
//  riot0_pb_dio_r/w -
//-------------------------------------------------

uint8_t c4023_device::riot0_pb_dio_r()
{
	return m_bus->dio_r();
}

void c4023_device::riot0_pb_dio_w(uint8_t data)
{
	m_bus->dio_w(this, data);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c4023_device::ieee488_atn(int state)
{
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c4023_device::ieee488_ifc(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, !state);
}
