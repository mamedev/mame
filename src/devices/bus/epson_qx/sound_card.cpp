// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * YM2149 based sound card with header for an external MPU401
 *
 * Board Design: https://github.com/brijohn/qx10/tree/master/kicad/ym2149_sound_card
 *
 *******************************************************************/

#include "emu.h"
#include "sound_card.h"

#include "speaker.h"

//**************************************************************************
//  YM2149 SOUND CARD DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_YM2149, bus::epson_qx::ym2149_sound_card_device, "epson_qx_option_sound_card", "Epson QX-10 YM2149 Sound Card")

namespace bus::epson_qx {

static INPUT_PORTS_START( ym2149_sound_card )
	PORT_START("IOBASE")
	PORT_CONFNAME(0xf0, 0xc0, "IO Base Address Selection")
	PORT_CONFSETTING(0x80, "&80")
	PORT_CONFSETTING(0x90, "&90")
	PORT_CONFSETTING(0xa0, "&A0")
	PORT_CONFSETTING(0xb0, "&B0")
	PORT_CONFSETTING(0xc0, "&C0")
	PORT_CONFSETTING(0xd0, "&D0")
	PORT_CONFSETTING(0xe0, "&E0")
	PORT_CONFSETTING(0xf0, "&F0")
	PORT_START("IRQ")
	PORT_CONFNAME(0x03, 0x03, "MPU IRQ")
	PORT_CONFSETTING(0x00, "NONE")
	PORT_CONFSETTING(0x01, "INTH1")
	PORT_CONFSETTING(0x02, "INTH2")
	PORT_CONFSETTING(0x03, "INTL")
INPUT_PORTS_END

//-------------------------------------------------
//  ym2149_sound_card_device - constructor
//-------------------------------------------------
ym2149_sound_card_device::ym2149_sound_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_YM2149, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_mpu401(*this, "mpu401"),
	m_ssg(*this, "ym2149"),
	m_iobase(*this, "IOBASE"),
	m_irq(*this, "IRQ"),
	m_installed(false)
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------
ioport_constructor ym2149_sound_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ym2149_sound_card );
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void ym2149_sound_card_device::device_add_mconfig(machine_config &config)
{
	MPU401(config, m_mpu401).irq_cb().set(FUNC(ym2149_sound_card_device::mpu_irq_out)).invert();
	SPEAKER(config, "speaker").front_center();
	YM2149(config, m_ssg, clock()).add_route(ALL_OUTPUTS, "speaker", 1.0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ym2149_sound_card_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_irqline));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void ym2149_sound_card_device::device_reset()
{
	m_ssg->set_pin26_low_w();

	m_irqline = m_irq->read() & 0x03;

	if (!m_installed) {
		address_space &space = m_bus->iospace();
		offs_t iobase = m_iobase->read() & 0xf0;
		space.install_device(iobase, iobase+0x03, *this, &ym2149_sound_card_device::map);
		m_installed = true;
	}
}

void ym2149_sound_card_device::map(address_map &map)
{
	map(0x01, 0x01).r(m_ssg, FUNC(ym2149_device::data_r));
	map(0x00, 0x01).w(m_ssg, FUNC(ym2149_device::address_data_w));
	map(0x02, 0x03).rw(m_mpu401, FUNC(mpu401_device::mpu_r), FUNC(mpu401_device::mpu_w));
}

WRITE_LINE_MEMBER(ym2149_sound_card_device::mpu_irq_out)
{
	switch(m_irqline) {
	case 1:
		get_slot()->inth1_w(state);
		break;
	case 2:
		get_slot()->inth2_w(state);
		break;
	case 3:
		get_slot()->intl_w(state);
		break;
	}
}

} // namespace bus::epson_qx
