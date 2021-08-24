// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    The Dragon's Claw from Lucidata

    http://archive.worldofdragon.org/index.php?title=The_Dragon%27s_Claw

    The Dragon's Claw is an extension cartridge for the Dragon by Lucidata.
    It features two parallel I/O ports. One with the BBC User Port pinout,
    and the second with a Centronics Printer Port pinout.

***************************************************************************/

#include "emu.h"
#include "dragon_claw.h"
#include "bus/bbc/userport/userport.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(DRAGON_CLAW, device_cococart_interface, dragon_claw_device, "dragon_claw", "The Dragon's Claw")

//-------------------------------------------------
//  INPUT_PORTS( claw )
//-------------------------------------------------

INPUT_PORTS_START(claw)
	PORT_START("LINKS")
	PORT_CONFNAME(0x03, 0x00, "L1 Address Selection")
	PORT_CONFSETTING(0x00, "$FF80")
	PORT_CONFSETTING(0x01, "$FF90")
	PORT_CONFSETTING(0x02, "$FFA0")
	PORT_CONFSETTING(0x03, "$FFB0")
	PORT_CONFNAME(0x04, 0x00, "L2 Interrupt Line")
	PORT_CONFSETTING(0x00, "NMI")
	PORT_CONFSETTING(0x04, "CART")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dragon_claw_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(claw);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_claw_device - constructor
//-------------------------------------------------

dragon_claw_device::dragon_claw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DRAGON_CLAW, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_via(*this, "via")
	, m_slot(*this, "slot")
	, m_links(*this, "LINKS")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_claw_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dragon_claw_device::device_reset()
{
	uint8_t addr = (m_links->read() & 0x03) << 8;

	install_readwrite_handler(0xff80 | addr, 0xff8f | addr, read8sm_delegate(*m_via, FUNC(via6522_device::read)), write8sm_delegate(*m_via, FUNC(via6522_device::write)));
}

static void dragon_cart(device_slot_interface &device)
{
	dragon_cart_add_basic_devices(device);
	dragon_cart_add_fdcs(device);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_claw_device::device_add_mconfig(machine_config &config)
{
	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via->readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	m_via->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	m_via->cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	m_via->cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));
	m_via->irq_handler().set(FUNC(dragon_claw_device::irq_w)); // link L2 selects either NMI or CART line

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, nullptr));
	centronics.ack_handler().set(m_via, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	COCOCART_SLOT(config, m_slot, DERIVED_CLOCK(1, 1), dragon_cart, nullptr);
	m_slot->cart_callback().set([this](int state) { set_line_value(line::CART, state); });
	m_slot->nmi_callback().set([this](int state) { set_line_value(line::NMI, state); });
	m_slot->halt_callback().set([this](int state) { set_line_value(line::HALT, state); });
}

WRITE_LINE_MEMBER(dragon_claw_device::irq_w)
{
	if (m_links->read() & 0x04)
		set_line_value(line::CART, state);
	else
		set_line_value(line::NMI, state);
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void dragon_claw_device::set_sound_enable(bool sound_enable)
{
	m_slot->set_line_value(line::SOUND_ENABLE, sound_enable ? line_value::ASSERT : line_value::CLEAR);
}


//-------------------------------------------------
//  get_cart_base
//-------------------------------------------------

u8 *dragon_claw_device::get_cart_base()
{
	return m_slot->get_cart_base();
}


//-------------------------------------------------
//  get_cart_size
//-------------------------------------------------

u32 dragon_claw_device::get_cart_size()
{
	return m_slot->get_cart_size();
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 dragon_claw_device::cts_read(offs_t offset)
{
	return m_slot->cts_read(offset);
}


//-------------------------------------------------
//  cts_write
//-------------------------------------------------

void dragon_claw_device::cts_write(offs_t offset, u8 data)
{
	m_slot->cts_write(offset, data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 dragon_claw_device::scs_read(offs_t offset)
{
	return m_slot->scs_read(offset);
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void dragon_claw_device::scs_write(offs_t offset, u8 data)
{
	m_slot->scs_write(offset, data);
}


//-------------------------------------------------
//  cartridge_space
//-------------------------------------------------

address_space &dragon_claw_device::cartridge_space()
{
	return device_cococart_interface::cartridge_space();
}
