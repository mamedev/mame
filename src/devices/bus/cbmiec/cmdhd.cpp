// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD HD hard drive emulation

**********************************************************************/

/*

	LOAD "$",12

	https://mikenaberezny.com/hardware/c64-128/cmd-hd-series/

*/

#include "emu.h"
#include "cmdhd.h"

#include "cmdhd.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u12"
#define M6522_1_TAG     "u9"
#define M6522_2_TAG     "u10"
#define I8255A_TAG      "u11"
#define RTC72421A_TAG   "u19"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CMD_HD, cmd_hd_device, "cmdhd", "CMD HD")


//-------------------------------------------------
//  ROM( cmd_hd )
//-------------------------------------------------

ROM_START( cmd_hd )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "cmd_hd_bootrom_v280.u17", 0x0000, 0x8000, CRC(da68435d) SHA1(defd8bc04a52904b8a3560f11c82126619513a10) )

	/*
	ROM_REGION( 0x30c, "plds", 0 )
	ROM_LOAD( "pal16l8.u24", 0x000, 0x104, NO_DUMP ) // Shirley
	ROM_LOAD( "pal16l8.u27", 0x104, 0x104, NO_DUMP ) // Julie
	ROM_LOAD( "pal16l8.u13", 0x208, 0x104, NO_DUMP )
	*/
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cmd_hd_device::device_rom_region() const
{
	return ROM_NAME( cmd_hd );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cmd_hd_device::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(NAME([this](offs_t offset) { return m_ram[offset]; })).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[offset] = data; }));
	map(0x4000, 0x7fff).view(m_ram_view);
	m_ram_view[0](0x4000, 0x7fff).lr8(NAME([this](offs_t offset) { return m_ram[0xc000 | offset]; })).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[0xc000 + offset] = data; }));
	m_ram_view[1](0x4000, 0x7fff).lr8(NAME([this](offs_t offset) { return m_ram[0x4000 | offset]; })).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[0x4000 + offset] = data; }));
	map(0x8000, 0xffff).lr8(NAME([this](offs_t offset) { return m_ram[0x8000 + offset]; })).lw8(NAME([this](offs_t offset, uint8_t data) { if (m_wpram) m_ram[0x8000 + offset] = data; }));
	map(0x8000, 0x800f).mirror(0x1f0).m(m_via0, FUNC(via6522_device::map));
	map(0x8400, 0x840f).mirror(0x1f0).m(m_via1, FUNC(via6522_device::map));
	map(0x8800, 0x8803).mirror(0x1fc).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8c00, 0x8c0f).mirror(0x1f0).rw(m_rtc, FUNC(rtc72421_device::read), FUNC(rtc72421_device::write));
	map(0x8e00, 0x8eff).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[0x8e00 + offset] = data; }));
	map(0x8f00, 0x8fff).lw8(NAME([this](offs_t offset, uint8_t data) { ttl_w(data); m_ram[0x8f00 + offset] = data; }));
	map(0xc000, 0xffff).lr8(NAME([this](offs_t offset) { return m_romos ? m_rom[0x4000 + offset] : m_ram[0xc000 + offset]; }));
}

uint8_t cmd_hd_device::via0_pa_r()
{
	/*

	    bit     description

	    0		AUX DATA IN
	    1		
	    2		AUX CLK IN
	    3
	    4
	    5		
	    6		SB RESET IN
	    7		AUX ATN

	*/

	u8 data = 0;

	data |= m_bus->reset_r() << 6;

	return data;
}

void cmd_hd_device::via0_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0		
	    1		AUX DATA OUT
	    2		
	    3		AUX CLK OUT
	    4
	    5		SB RESET ENA
	    6		
	    7

	*/

	m_sb_reset_ena = BIT(data, 5);
}

uint8_t cmd_hd_device::via0_pb_r()
{
	/*

	    bit     description

	    0		DATA IN
	    1		
	    2		CLK IN
	    3
	    4
	    5		
	    6		
	    7		ATN IN

	*/

	u8 data = 0;

	data |= !m_bus->data_r();
	data |= !m_bus->clk_r() << 2;
	data |= !m_bus->atn_r() << 7;

	return data;
}

void cmd_hd_device::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0		
	    1		DATA OUT
	    2
	    3		CLK OUT
	    4		_ATN ACK
	    5		FST DIR
	    6		_ATN OUT
	    7		

	*/

	m_iec_data = BIT(data, 1);
	m_iec_clk = BIT(data, 3);
	m_atn_ack = BIT(data, 4);
	m_fst_dir = BIT(data, 5);
	m_iec_atn = BIT(data, 6);

	m_iec_sync_timer->adjust(attotime::zero);
}

void cmd_hd_device::via0_cb1_w(int state)
{
	m_fst_clk = state;

	m_iec_sync_timer->adjust(attotime::zero);
}

void cmd_hd_device::via0_cb2_w(int state)
{
	m_fst_data = state;

	m_iec_sync_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(cmd_hd_device::iec_sync_tick)
{
	bool const atn = m_bus->atn_r();
	m_via0->write_ca1(atn);

	m_via0->write_cb1(m_fst_dir || m_bus->srq_r());
	m_via0->write_cb2(m_fst_dir || m_bus->data_r());

	m_bus->atn_w(this, m_iec_atn);
	m_bus->clk_w(this, !m_iec_clk);

	bool data_out = !m_iec_data && !(m_atn_ack && !atn);
	if (m_fst_dir)
		data_out &= m_fst_data;
	m_bus->data_w(this, data_out);

	bool srq_out = 1;
	if (m_fst_dir)
		srq_out &= m_fst_clk;
	m_bus->srq_w(this, srq_out);
}

uint8_t cmd_hd_device::via1_pa_r()
{
	if (m_bdirin)
		return m_scsi_bus->data_r() ^ 0xff;
	else
		return 0xff;
}

void cmd_hd_device::via1_pa_w(uint8_t data)
{
	m_sasi_out = data ^ 0xff;

	if (!m_bdirin)
		m_scsi_bus->data_w(m_scsi_refid, m_sasi_out);
}

uint8_t cmd_hd_device::via1_pb_r()
{
	/*

	    bit     description

	    0		C/_D
	    1		MSG
	    2		I/_O
	    3
	    4
	    5		BSY
	    6		ACK IN
	    7		REQ

	*/

	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 data = 0;

	if (ctrl & S_CTL) data |= 1 << 0;
	if (ctrl & S_MSG) data |= 1 << 1;
	if (ctrl & S_INP) data |= 1 << 2;
	if (ctrl & S_BSY) data |= 1 << 5;
	if (ctrl & S_REQ) data |= 1 << 7;

	data |= m_ack_ff << 6;

	return data;
}

void cmd_hd_device::via1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0		
	    1		
	    2
	    3		BDIRIN
	    4		SEL OUT
	    5
	    6
	    7		

	*/

	m_bdirin = BIT(data, 3);

	if (!m_bdirin)
		m_scsi_bus->data_w(m_scsi_refid, m_sasi_out);

	m_scsi_bus->ctrl_w(m_scsi_refid, (BIT(data, 4) ? S_SEL : 0), S_SEL);
}

void cmd_hd_device::via1_ca2_w(int state)
{
	if (!state)
	{
		m_ack_ff = 1;

		m_scsi_bus->ctrl_w(m_scsi_refid, S_ACK, S_ACK);
	}
}

void cmd_hd_device::via1_cb2_w(int state)
{
	// FORCE
}

uint8_t cmd_hd_device::ppi_pa_r()
{
	// RamLink parallel data PD0-7
	return 0xff;
}

void cmd_hd_device::ppi_pa_w(uint8_t data)
{
	// RamLink parallel data PD0-7
}

uint8_t cmd_hd_device::ppi_pb_r()
{
	/*
	    bit     description

	    0		PRDY IN
	    1		_PBSW8
	    2		_PBSW9
	    3		_PBWRP
	    4		SCSI SEL IN
	    5		PEXT IN
	    6		PCLK IN
	    7		PATN

	*/

	u32 const ctrl = m_scsi_bus->ctrl_r();
	u8 data = 0;

	data |= (m_pb->read() & 0x07) << 1;

	if (ctrl & S_SEL) data |= 1 << 4;

	return data;
}

void cmd_hd_device::ppi_pc_w(uint8_t data)
{
	/*
	    bit     description

	    0		ROMOS
	    1		_ALTMAP
	    2		SCSI ATN OUT
	    3		SCSI RST OUT
	    4		SCSI BSY OUT
	    5		_PEXT OUT
	    6		_PCLK OUT
	    7		_PRDY OUT

	*/

	m_romos = BIT(data, 0);

	m_ram_view.select(BIT(data, 1));

	m_scsi_bus->ctrl_w(m_scsi_refid, (BIT(data, 2) ? S_ATN : 0) | (BIT(data, 3) ? S_RST : 0) | (BIT(data, 4) ? S_BSY : 0), S_ATN|S_RST|S_BSY);
}

void cmd_hd_device::ttl_w(uint8_t data)
{
	/*

	    bit     description

	    0		_ACTLED
	    1		_ERRLED
	    2		_SW8LED
	    3		_SW9LED
	    4		PARITY
	    5		_WPRAM
	    6		_GEOLED
	    7		_WRPLED

	*/

	m_wpram = BIT(data, 5);

	m_leds[LED_ACT] = !BIT(data, 0);
	m_leds[LED_ERR] = !BIT(data, 1);
	m_leds[LED_SW8] = !BIT(data, 2);
	m_leds[LED_SW9] = !BIT(data, 3);
	m_leds[LED_GEO] = !BIT(data, 6);
	m_leds[LED_WRP] = !BIT(data, 7);

	m_via1->write_ca2(!BIT(data, 4));
}

void cmd_hd_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(16'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &cmd_hd_device::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MOS6522(config, m_via0, XTAL(16'000'000)/8);
	m_via0->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_via0->readpa_handler().set(FUNC(cmd_hd_device::via0_pa_r));
	m_via0->writepa_handler().set(FUNC(cmd_hd_device::via0_pa_w));
	m_via0->readpb_handler().set(FUNC(cmd_hd_device::via0_pb_r));
	m_via0->writepb_handler().set(FUNC(cmd_hd_device::via0_pb_w));
	m_via0->cb1_handler().set(FUNC(cmd_hd_device::via0_cb1_w));
	m_via0->cb2_handler().set(FUNC(cmd_hd_device::via0_cb2_w));

	MOS6522(config, m_via1, XTAL(16'000'000)/8);
	m_via1->irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_via1->readpa_handler().set(FUNC(cmd_hd_device::via1_pa_r));
	m_via1->writepa_handler().set(FUNC(cmd_hd_device::via1_pa_w));
	m_via1->readpb_handler().set(FUNC(cmd_hd_device::via1_pb_r));
	m_via1->writepb_handler().set(FUNC(cmd_hd_device::via1_pb_w));
	m_via1->ca2_handler().set(FUNC(cmd_hd_device::via1_ca2_w));
	m_via1->cb2_handler().set(FUNC(cmd_hd_device::via1_cb2_w));
	
	I8255A(config, m_ppi, 0);
	m_ppi->in_pa_callback().set(FUNC(cmd_hd_device::ppi_pa_r));
	m_ppi->out_pa_callback().set(FUNC(cmd_hd_device::ppi_pa_w));
	m_ppi->in_pb_callback().set(FUNC(cmd_hd_device::ppi_pb_r));
	m_ppi->out_pc_callback().set(FUNC(cmd_hd_device::ppi_pc_w));
	
	RTC72421(config, m_rtc, XTAL(32'768));

	auto &sasi(NSCSI_BUS(config, "sasi"));
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "sasi:1", default_scsi_devices, nullptr);
	sasi.set_external_device(7, *this);

	config.set_default_layout(layout_cmdhd);
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_CHANGED_MEMBER( cmd_hd_device::pbres_changed )
{
	if (!newval)
	{
		reset();
	}
}

static INPUT_PORTS_START( cmd_hd )
	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Swap 8")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Swap 9")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Write Protect")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cmd_hd_device::pbres_changed), 0)
INPUT_PORTS_END

ioport_constructor cmd_hd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cmd_hd );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cmd_hd_device - constructor
//-------------------------------------------------

cmd_hd_device::cmd_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CMD_HD, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	nscsi_device_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_via0(*this, M6522_1_TAG),
	m_via1(*this, M6522_2_TAG),
	m_ppi(*this, I8255A_TAG),
	m_rtc(*this, RTC72421A_TAG),
	m_leds(*this, "led%u", 0U),
	m_pb(*this, "PB"),
	m_ram_view(*this, "ram"),
	m_rom(*this, M6502_TAG),
	m_ram(*this, "ram", 0x10000, ENDIANNESS_BIG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cmd_hd_device::device_start()
{
	m_iec_sync_timer = timer_alloc(FUNC(cmd_hd_device::iec_sync_tick), this);
	m_ack_clear_timer = timer_alloc(FUNC(cmd_hd_device::clear_ack_tick), this);
	
	m_leds[LED_PWR] = 1;

	// state saving
	save_item(NAME(m_fst_dir));
	save_item(NAME(m_fst_clk));
	save_item(NAME(m_fst_data));
	save_item(NAME(m_atn_ack));
	save_item(NAME(m_iec_atn));
	save_item(NAME(m_iec_clk));
	save_item(NAME(m_iec_data));
	save_item(NAME(m_sb_reset_ena));
	save_item(NAME(m_bdirin));
	save_item(NAME(m_sasi_out));
	save_item(NAME(m_ack_ff));
	save_item(NAME(m_romos));
	save_item(NAME(m_wpram));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cmd_hd_device::device_reset()
{
	m_scsi_bus->ctrl_wait(m_scsi_refid, S_BSY|S_RST|S_REQ, S_ALL);

	m_ram_view.select(0);
	
	m_romos = 1;
	m_wpram = 1;
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_srq(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_atn(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_data(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void cmd_hd_device::cbm_iec_reset(int state)
{
	if (m_sb_reset_ena && !state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  scsi_ctrl_changed -
//-------------------------------------------------

void cmd_hd_device::scsi_ctrl_changed()
{
	u32 const ctrl = m_scsi_bus->ctrl_r();

	bool const bsy = ctrl & S_BSY;
	m_leds[LED_BSY] = bsy;
	m_via0->write_cb1(!bsy);

	bool const rst = ctrl & S_RST;
	m_via1->write_ca1(!rst);

	bool const req = ctrl & S_REQ;
	if (!req)
	{
		m_ack_ff = 0;
		m_ack_clear_timer->adjust(attotime::zero);
	}
}
	
TIMER_CALLBACK_MEMBER(cmd_hd_device::clear_ack_tick)
{
	m_scsi_bus->ctrl_w(m_scsi_refid, 0, S_ACK);
}
