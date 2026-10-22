// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Interface Module emulation

**********************************************************************/

/*

    TODO:

    - CTC interrupts
    - DAC output to the sound summer
    - user port and centronics interrupts
    - multiprocessing mode

*/

#include "emu.h"
#include "eim.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MC6850_TAG      "459"
#define ADC0809_TAG     "427"
#define Z80CTC_TAG      "458"
#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NEWBRAIN_EIM, newbrain_eim_device, "newbrain_eim", "NewBrain EIM")


//-------------------------------------------------
//  ROM( newbrain_eim )
//-------------------------------------------------

ROM_START( newbrain_eim )
	ROM_REGION( 0x6000, "eim", 0 )
	ROM_DEFAULT_BIOS( "aci3" )
	ROM_SYSTEM_BIOS( 0, "aci2", "ACI issue 2" )
	ROMX_LOAD( "e415-2.rom", 0x0000, 0x2000, CRC(5b0e390c) SHA1(0f99cae57af2e64f3f6b02e5325138d6ba015e72), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "aci3", "ACI issue 3 (18/8/83)" )
	ROMX_LOAD( "e415-3.rom", 0x0000, 0x2000, CRC(2f88bae5) SHA1(04e03f230f4b368027442a7c2084dae877f53713), ROM_BIOS(1) )
	ROM_LOAD( "e416-3.rom", 0x2000, 0x2000, CRC(8b5099d8) SHA1(19b0cfce4c8b220eb1648b467f94113bafcb14e0) ) // 10/8/83.mtv
	ROM_LOAD( "e417-2.rom", 0x4000, 0x2000, CRC(6a7afa20) SHA1(f90db4f8318777313a862b3d5bab83c2fd260010) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *newbrain_eim_device::device_rom_region() const
{
	return ROM_NAME( newbrain_eim );
}


//-------------------------------------------------
//  INPUT_PORTS( newbrain_eim )
//-------------------------------------------------

static INPUT_PORTS_START( newbrain_eim )
	PORT_START("SW")
	PORT_CONFNAME( 0x04, 0x04, "Video on Power Up" )
	PORT_CONFSETTING(    0x00, "Reversed" )
	PORT_CONFSETTING(    0x04, "Normal" )
	PORT_CONFNAME( 0x08, 0x08, "Power Supply" )
	PORT_CONFSETTING(    0x00, "Battery" )
	PORT_CONFSETTING(    0x08, "Mains" )
	PORT_CONFNAME( 0x10, 0x10, "Columns on Power Up" )
	PORT_CONFSETTING(    0x00, "80" )
	PORT_CONFSETTING(    0x10, "40" )
	PORT_CONFNAME( 0x40, 0x40, "Video Display Required" )
	PORT_CONFSETTING(    0x00, DEF_STR( No ) )
	PORT_CONFSETTING(    0x40, DEF_STR( Yes ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor newbrain_eim_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( newbrain_eim );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void newbrain_eim_device::device_add_mconfig(machine_config &config)
{
	// devices
	Z80CTC(config, m_ctc, XTAL(16'000'000)/8);
	m_ctc->zc_callback<0>().set(m_acia, FUNC(acia6850_device::write_rxc));
	m_ctc->zc_callback<1>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_ctc->zc_callback<2>().set(FUNC(newbrain_eim_device::ctc_z2_w));

	ADC0809(config, m_adc, 500000);
	m_adc->eoc_callback().set(FUNC(newbrain_eim_device::adc_eoc_w));
	m_adc->in_callback<0>().set_constant(0);
	m_adc->in_callback<1>().set_constant(0);
	m_adc->in_callback<2>().set_constant(0);
	m_adc->in_callback<3>().set_constant(0);
	m_adc->in_callback<4>().set_constant(0);
	m_adc->in_callback<5>().set_constant(0);
	m_adc->in_callback<6>().set_constant(0);
	m_adc->in_callback<7>().set_constant(0);

	ACIA6850(config, m_acia);
	m_acia->irq_handler().set(FUNC(newbrain_eim_device::acia_interrupt));
	m_acia->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	NEWBRAIN_EXPANSION_SLOT(config, m_exp, XTAL(16'000'000)/8, newbrain_expansion_cards, "fdc");
	m_exp->int_callback().set(FUNC(newbrain_eim_device::exp_int_w));
	m_exp->host_mreq_r_callback().set([this] (offs_t offset) { return m_slot->host_mreq_r(offset); });
	m_exp->host_mreq_w_callback().set([this] (offs_t offset, uint8_t data) { m_slot->host_mreq_w(offset, data); });
	m_exp->host_main_r_callback().set([this] (offs_t offset) { return m_slot->host_main_r(offset); });
	m_exp->host_main_w_callback().set([this] (offs_t offset, uint8_t data) { m_slot->host_main_w(offset, data); });
	m_exp->host_paged_r_callback().set(FUNC(newbrain_eim_device::paged_r));
	m_exp->host_paged_w_callback().set(FUNC(newbrain_eim_device::paged_w));

	// internal ram
	RAM(config, m_ram).set_default_size("64K");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_eim_device - constructor
//-------------------------------------------------

newbrain_eim_device::newbrain_eim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEWBRAIN_EIM, tag, owner, clock),
	device_newbrain_expansion_slot_interface(mconfig, *this),
	m_ctc(*this, Z80CTC_TAG),
	m_acia(*this, MC6850_TAG),
	m_adc(*this, ADC0809_TAG),
	m_exp(*this, "exp"),
	m_rom(*this, "eim"),
	m_ram(*this, RAM_TAG),
	m_sw(*this, "SW"),
	m_psr(0),
	m_enrg2(0),
	m_user(0),
	m_dac(0),
	m_aciaint(0),
	m_anint(0),
	m_expint(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newbrain_eim_device::device_start()
{
	std::fill(std::begin(m_pr), std::end(m_pr), 0xff);

	m_c2_timer = timer_alloc(FUNC(newbrain_eim_device::ctc_c2_tick), this);

	// state saving
	save_item(NAME(m_pr));
	save_item(NAME(m_psr));
	save_item(NAME(m_enrg2));
	save_item(NAME(m_user));
	save_item(NAME(m_dac));
	save_item(NAME(m_aciaint));
	save_item(NAME(m_anint));
	save_item(NAME(m_expint));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newbrain_eim_device::device_reset()
{
	m_psr = 0;
	m_enrg2 = 0;

	// page register lookup in the address path stretches every opcode fetch
	m_slot->m1_wait_w(1);

	update_ctc_clock();

	update_interrupt();
}


//-------------------------------------------------
//  page - complemented page number of the slot
//-------------------------------------------------

int newbrain_eim_device::page(offs_t offset)
{
	int slot = (offset >> 13) & 7;

	if (!BIT(m_psr, 0))
		return (slot < 4) ? (0x14 | slot) : slot;

	return m_pr[(BIT(m_psr, 2) << 3) | slot];
}


//-------------------------------------------------
//  decode - map a CPU address through the paging
//-------------------------------------------------

uint8_t *newbrain_eim_device::decode(int p, offs_t offset, bool &romov, int &exrm, bool &raminh, bool &rom)
{
	offs_t addr = offset & 0x1fff;

	p &= 0x7f;
	rom = false;
	romov = false;

	if ((p & 0xec) == 0x04)
	{
		// computer memory
		exrm = (BIT(p, 4) ? 0 : 4) | (p & 3);

		if (exrm == 4)
		{
			raminh = true;
			rom = true;
			return m_rom->base() + 0x4000 + addr;
		}

		return nullptr;
	}

	exrm = 4;
	raminh = true;

	if ((p & 0xf8) == 0x18)
		return m_ram->pointer() + (((p & 7) << 13) | addr);

	if ((p & 0xfe) == 0x02)
	{
		rom = true;
		return m_rom->base() + ((p & 1) << 13) + addr;
	}

	return nullptr;
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

uint8_t newbrain_eim_device::mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	data = m_exp->mreq_r(offset, data, romov, exrm, raminh);

	bool rom;
	int p = page(offset);
	uint8_t *ptr = decode(p, offset, romov, exrm, raminh, rom);

	if (ptr)
		data = *ptr;
	else if (raminh)
		data = m_exp->page_r(p & 0x7f, offset & 0x1fff, data);

	return data;
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_eim_device::mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	m_exp->mreq_w(offset, data, romov, exrm, raminh);

	bool rom;
	int p = page(offset);
	uint8_t *ptr = decode(p, offset, romov, exrm, raminh, rom);

	if (ptr && !rom)
		*ptr = data;
	else if (!ptr && raminh)
		m_exp->page_w(p & 0x7f, offset & 0x1fff, data);
}


//-------------------------------------------------
//  paged_r - paged access by a downstream bus master
//-------------------------------------------------

uint8_t newbrain_eim_device::paged_r(offs_t offset)
{
	bool romov = true, raminh = false, rom;
	int exrm = 0;
	int p = m_pr[(BIT(offset, 16) << 3) | ((offset >> 13) & 7)];
	uint8_t *ptr = decode(p, offset, romov, exrm, raminh, rom);

	if (ptr)
		return *ptr;

	if (!raminh)
		return m_slot->host_main_r((exrm << 13) | (offset & 0x1fff));

	return m_exp->page_r(p & 0x7f, offset & 0x1fff, 0xff);
}


//-------------------------------------------------
//  paged_w - paged access by a downstream bus master
//-------------------------------------------------

void newbrain_eim_device::paged_w(offs_t offset, uint8_t data)
{
	bool romov = true, raminh = false, rom;
	int exrm = 0;
	int p = m_pr[(BIT(offset, 16) << 3) | ((offset >> 13) & 7)];
	uint8_t *ptr = decode(p, offset, romov, exrm, raminh, rom);

	if (ptr)
	{
		if (!rom)
			*ptr = data;
	}
	else if (!raminh)
		m_slot->host_main_w((exrm << 13) | (offset & 0x1fff), data);
	else
		m_exp->page_w(p & 0x7f, offset & 0x1fff, data);
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

uint8_t newbrain_eim_device::iorq_r(offs_t offset, uint8_t data, bool &prtov)
{
	data = m_exp->iorq_r(offset, data, prtov);

	int port = offset & 0xff;

	if (port >= 0x20)
	{
		prtov = true;
		return data;
	}

	switch (port)
	{
	case 0x00:
	case 0x02:
		prtov = true;
		break;

	case 0x01:
		prtov = true;
		if (!machine().side_effects_disabled())
		{
			m_enrg2 = 0xff;
			update_ctc_clock();
			update_interrupt();
		}
		break;

	case 0x03:
		prtov = true;
		if (!machine().side_effects_disabled())
			m_user = 0xff;
		break;

	case 0x05:
		prtov = true;
		if (!machine().side_effects_disabled())
			m_dac = 0xff;
		break;

	case 0x10: case 0x11: case 0x12: case 0x13:
		prtov = true;
		data = m_adc->data_r();
		break;

	case 0x14:
		data &= ~0x40;
		data |= (m_anint && BIT(m_enrg2, 1)) << 2;
		data |= !m_aciaint << 6;
		break;

	case 0x15:
		prtov = true;
		data = m_sw->read() | 0xa3;
		break;

	case 0x17:
		prtov = true;
		break;

	case 0x18: case 0x19:
		prtov = true;
		data = m_acia->read(port & 1);
		break;

	case 0x1a: case 0x1b:
		prtov = true;
		break;

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		prtov = true;
		data = m_ctc->read(port & 3);
		break;
	}

	return data;
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_eim_device::iorq_w(offs_t offset, uint8_t data, bool &prtov)
{
	m_exp->iorq_w(offset, data, prtov);

	int port = offset & 0xff;

	if (port == 0xff && BIT(offset, 8))
	{
		m_psr = data;
	}

	if (port >= 0x20)
	{
		prtov = true;
		return;
	}

	switch (port)
	{
	case 0x00:
		prtov = true;
		break;

	case 0x01:
		prtov = true;
		m_enrg2 = data;
		update_ctc_clock();
		update_interrupt();
		break;

	case 0x02:
		prtov = true;
		m_pr[(BIT(offset, 12) << 3) | (offset >> 13)] = ~((BIT(offset, 11) << 7) | (data & 0x7f));
		break;

	case 0x03:
		prtov = true;
		m_user = data;
		break;

	case 0x05:
		prtov = true;
		m_dac = data;
		break;

	case 0x10: case 0x11: case 0x12: case 0x13:
		prtov = true;
		m_adc->address_w((BIT(m_enrg2, 5) << 2) | (port & 3));
		m_adc->start_w(1);
		m_adc->start_w(0);
		break;

	case 0x15:
	case 0x17:
	case 0x1a: case 0x1b:
		prtov = true;
		break;

	case 0x18: case 0x19:
		prtov = true;
		m_acia->write(port & 1, data);
		break;

	case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		prtov = true;
		m_ctc->write(port & 3, data);
		break;
	}
}


//-------------------------------------------------
//  update_interrupt -
//-------------------------------------------------

void newbrain_eim_device::update_interrupt()
{
	m_slot->int_w(m_aciaint || (m_anint && BIT(m_enrg2, 1)) || m_expint);
}


//-------------------------------------------------
//  adc_eoc_w -
//-------------------------------------------------

void newbrain_eim_device::adc_eoc_w(int state)
{
	m_anint = state;
	update_interrupt();
}


//-------------------------------------------------
//  acia_interrupt -
//-------------------------------------------------

void newbrain_eim_device::acia_interrupt(int state)
{
	m_aciaint = state;
	update_interrupt();
}


//-------------------------------------------------
//  exp_int_w -
//-------------------------------------------------

void newbrain_eim_device::exp_int_w(int state)
{
	m_expint = state;
	update_interrupt();
}


//-------------------------------------------------
//  ctc_z2_w -
//-------------------------------------------------

void newbrain_eim_device::ctc_z2_w(int state)
{
	// connected to CTC channel 0/1 clock inputs
	m_ctc->trg0(state);
	m_ctc->trg1(state);
}


//-------------------------------------------------
//  update_ctc_clock -
//-------------------------------------------------

void newbrain_eim_device::update_ctc_clock()
{
	attotime period = attotime::from_hz(BIT(m_enrg2, 3) ? (XTAL(16'000'000)/4/5) : (XTAL(16'000'000)/4/13));

	m_c2_timer->adjust(period, 0, period);
}


//-------------------------------------------------
//  ctc_c2_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(newbrain_eim_device::ctc_c2_tick)
{
	m_ctc->trg2(1);
	m_ctc->trg2(0);
}
