// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller Turbo Trans cartridge emulation

**********************************************************************/

#include "emu.h"
#include "turbotrans.h"

#include <algorithm>
#include <iterator>



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_TURBOTRANS, c64_turbotrans_device, "c64_turbotrans", "C64 Rossmöller TurboTrans cartridge")


//-------------------------------------------------
//  ROM( c64_turbotrans )
//-------------------------------------------------

ROM_START( c64_turbotrans )
	ROM_REGION( 0x2000, "ttk", 0 )
	ROM_DEFAULT_BIOS("v30b")
	ROM_SYSTEM_BIOS( 0, "v30a", "v3.0 (1)" )
	ROMX_LOAD( "ttk301", 0x0000, 0x2000, CRC(b3304dcf) SHA1(4d47a265ef65e4823f862cfc3d514c2a71473580), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v30b", "v3.0 (2)" )
	ROMX_LOAD( "ttk302", 0x0000, 0x2000, CRC(9e696a7b) SHA1(5afae75d66d539f4bb4af763f029f0ef6523a4eb), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_turbotrans_device::device_rom_region() const
{
	return ROM_NAME( c64_turbotrans );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_turbotrans_device::device_add_mconfig(machine_config &config)
{
	MOS6522(config, m_via, DERIVED_CLOCK(1, 1));
	m_via->writepa_handler().set(FUNC(c64_turbotrans_device::drive_data_w<0>));
	m_via->writepb_handler().set(FUNC(c64_turbotrans_device::drive_data_w<1>));
	m_via->readpa_handler().set([this] () -> u8 { return m_drive_data[0]; });
	m_via->readpb_handler().set([this] () -> u8 { return m_drive_data[1]; });
	m_via->ca2_handler().set(FUNC(c64_turbotrans_device::drive_strobe_w<0>));
	m_via->cb2_handler().set(FUNC(c64_turbotrans_device::drive_strobe_w<1>));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}


//-------------------------------------------------
//  INPUT_PORTS( c64_turbotrans )
//-------------------------------------------------

static INPUT_PORTS_START( c64_turbotrans )
	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F11) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF_OWNER, FUNC(c64_expansion_slot_device::reset_w))

	PORT_START("SWITCH")
	PORT_CONFNAME( 0x01, 0x01, "Kernal" )
	PORT_CONFSETTING(    0x00, "Original" )
	PORT_CONFSETTING(    0x01, "TurboTrans" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_turbotrans_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_turbotrans );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_turbotrans_device - constructor
//-------------------------------------------------

c64_turbotrans_device::c64_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_TURBOTRANS, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_via(*this, "via"),
	m_exp(*this, "exp"),
	m_kernal(*this, "ttk"),
	m_switch(*this, "SWITCH"),
	m_drive{ { mconfig, *this, 0 }, { mconfig, *this, 1 } },
	m_kernal_enable(true)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_turbotrans_device::device_start()
{
	std::fill(std::begin(m_drive_data), std::end(m_drive_data), 0xff);

	int drive = 0;
	for (device_t &device : device_enumerator(machine().root_device()))
	{
		if (drive < 2 && device.interface(m_drive[drive].m_other))
		{
			m_drive[drive].m_other->m_other = &m_drive[drive];
			drive++;
		}
	}

	save_item(NAME(m_kernal_enable));
	save_item(NAME(m_drive_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_turbotrans_device::device_reset()
{
	m_kernal_enable = BIT(m_switch->read(), 0);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_turbotrans_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!romh)
	{
		data = m_kernal->base()[offset & 0x1fff];
	}

	if (!io2 && BIT(offset, 7))
	{
		data = m_via->read(offset & 0x0f);
	}

	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_turbotrans_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2 && BIT(offset, 7))
	{
		m_via->write(offset & 0x0f, data);
	}

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1, io2);
}


//-------------------------------------------------
//  c64_game_r - GAME read
//-------------------------------------------------

int c64_turbotrans_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	int e_game = m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
	int game = !(sphi2 && ba && rw && m_slot->hiram() && m_kernal_enable && ((offset & 0xe000) == 0xe000));

	return e_game && game;
}


//-------------------------------------------------
//  c64_exrom_r - EXROM read
//-------------------------------------------------

int c64_turbotrans_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  c64_ba_w - BA write
//-------------------------------------------------

void c64_turbotrans_device::c64_ba_w(int state)
{
	m_exp->ba_w(state);
}


//-------------------------------------------------
//  c64_phi2_w - phi2 write
//-------------------------------------------------

void c64_turbotrans_device::c64_phi2_w(int state)
{
	m_exp->phi2_w(state);
}


//-------------------------------------------------
//  c64_irq_w - IRQ write
//-------------------------------------------------

void c64_turbotrans_device::c64_irq_w(int state)
{
	m_exp->host_irq_w(state);
}


//-------------------------------------------------
//  c64_nmi_w - NMI write
//-------------------------------------------------

void c64_turbotrans_device::c64_nmi_w(int state)
{
	m_exp->host_nmi_w(state);
}


//-------------------------------------------------
//  drive_data_w -
//-------------------------------------------------

template <int Drive> void c64_turbotrans_device::drive_data_w(uint8_t data)
{
	if (m_drive[Drive].m_other)
		m_drive[Drive].m_other->parallel_data_w(data);
}


//-------------------------------------------------
//  drive_strobe_w -
//-------------------------------------------------

template <int Drive> void c64_turbotrans_device::drive_strobe_w(int state)
{
	if (m_drive[Drive].m_other)
		m_drive[Drive].m_other->parallel_strobe_w(state);
}


//-------------------------------------------------
//  drive_port::parallel_data_w -
//-------------------------------------------------

void c64_turbotrans_device::drive_port::parallel_data_w(uint8_t data)
{
	m_host.machine().scheduler().synchronize(timer_expired_delegate(FUNC(c64_turbotrans_device::drive_data_sync), &m_host), (m_index << 8) | data);
}


//-------------------------------------------------
//  drive_data_sync -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(c64_turbotrans_device::drive_data_sync)
{
	m_drive_data[BIT(param, 8)] = u8(param);
}


//-------------------------------------------------
//  drive_port::parallel_strobe_w -
//-------------------------------------------------

void c64_turbotrans_device::drive_port::parallel_strobe_w(int state)
{
	if (m_index)
		m_host.m_via->write_cb1(state);
	else
		m_host.m_via->write_ca1(state);
}
