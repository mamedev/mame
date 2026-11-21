// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Onmyou Taisenki's two-contact dock receives battle data from a separate
    Drive toy. Replay a captured packet, including its checksum, without
    substituting character data or bypassing the console's receiver.

    Pulse widths are inferred from ban_omt's receive routine. The handheld
    firmware and physical link timings remain unverified.

***************************************************************************/

#include "emu.h"
#include "xavix_io_omt_drive.h"

DEFINE_DEVICE_TYPE(XAVIX_OMT_DRIVE, xavix_omt_drive_device, "xavix_omt_drive", "Onmyou Taisenki Drive Link")

static INPUT_PORTS_START(omt_drive)
	PORT_START("SEND")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Send Docked Drive Data") PORT_CODE(KEYCODE_B)
		PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_omt_drive_device::send_changed), 0)
INPUT_PORTS_END

xavix_omt_drive_device::xavix_omt_drive_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_OMT_DRIVE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_timer(nullptr)
	, m_loaded(false)
{
}

ioport_constructor xavix_omt_drive_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(omt_drive);
}

void xavix_omt_drive_device::device_start()
{
	m_timer = timer_alloc(FUNC(xavix_omt_drive_device::pulse), this);
	m_image.fill(0);
	m_packet.fill(0);
	m_saved_image.fill(0);
	m_saved_present = false;
	stop();
	save_item(NAME(m_packet));
	save_item(NAME(m_saved_image));
	save_item(NAME(m_saved_present));
	save_item(NAME(m_phase));
	save_item(NAME(m_bit));
	save_item(NAME(m_data));
}

void xavix_omt_drive_device::device_reset()
{
	stop();
}

void xavix_omt_drive_device::device_pre_save()
{
	m_saved_present = m_loaded;
	m_saved_image = m_image;
}

void xavix_omt_drive_device::device_post_load()
{
	if (m_saved_present != m_loaded || (m_loaded && m_saved_image != m_image))
	{
		stop();
		popmessage("Drive data differs from the save state. The transfer has been cancelled.");
	}
}

std::pair<std::error_condition, std::string> xavix_omt_drive_device::call_load()
{
	if (length() != PACKET_BYTES)
		return { image_error::INVALIDLENGTH, "Expected one 25-byte Drive packet, including checksum" };
	if (fread(m_image.data(), m_image.size()) != m_image.size())
		return { image_error::UNSPECIFIED, "Unable to read Drive packet" };
	m_loaded = true;
	stop();
	return {};
}

void xavix_omt_drive_device::call_unload()
{
	m_loaded = false;
	stop();
}

void xavix_omt_drive_device::stop()
{
	if (m_timer)
		m_timer->adjust(attotime::never);
	m_phase = IDLE;
	m_bit = 0;
	m_data = 0;
}

INPUT_CHANGED_MEMBER(xavix_omt_drive_device::send_changed)
{
	if (newval && m_loaded && m_phase == IDLE)
	{
		m_packet = m_image;
		m_bit = 0;
		m_data = 1;
		m_phase = HEADER_HIGH;
		m_timer->adjust(attotime::from_msec(16));
	}
}

TIMER_CALLBACK_MEMBER(xavix_omt_drive_device::pulse)
{
	switch (m_phase)
	{
	case HEADER_HIGH:
		m_data = 0;
		m_phase = HEADER_LOW;
		m_timer->adjust(attotime::from_msec(16));
		break;
	case HEADER_LOW:
		m_data = 1;
		m_phase = DATA_HIGH;
		m_timer->adjust(attotime::from_msec(2));
		break;
	case DATA_HIGH:
		m_data = 0;
		m_phase = DATA_LOW;
		m_timer->adjust(attotime::from_msec(BIT(m_packet[m_bit / 8], 7 - (m_bit & 7)) ? 6 : 2));
		break;
	case DATA_LOW:
		m_bit++;
		m_data = 1;
		m_phase = m_bit == PACKET_BYTES * 8 ? TRAILER : DATA_HIGH;
		m_timer->adjust(attotime::from_msec(2));
		break;
	case TRAILER:
		stop();
		break;
	}
}
