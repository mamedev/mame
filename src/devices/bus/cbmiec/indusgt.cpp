// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Indus GT disk drive emulation

**********************************************************************/

/*

Load ROM disk directory:

LOAD"$1",8

Load file from ROM disk:

LOAD"1:FC",8,1

*/

#include "emu.h"
#include "indusgt.h"

#include "indusgt.lh"

DEFINE_DEVICE_TYPE(INDUS_GT, indus_gt_device, "indusgt", "Indus GT Disk Drive")

//-------------------------------------------------
//  ROM( indusgt )
//-------------------------------------------------

ROM_START( indusgt )
	ROM_REGION( 0x4000, "ucd5", 0 )
	ROM_LOAD( "u18 v1.1.u18", 0x0000, 0x2000, CRC(e401ce56) SHA1(9878053bdff7a036f57285c2c4974459df2602d8) )
	ROM_LOAD( "u17 v1.1.u17", 0x2000, 0x2000, CRC(575ad906) SHA1(f48837b024add84f888acd83a9cf9eb7d2379172) )

	ROM_REGION( 0x2000, "romdisk", 0 )
	ROM_LOAD( "u19 v1.1.u19", 0x0000, 0x2000, CRC(8f83e7a5) SHA1(5bceaad520dac9d0527723b3b454e8ec99748e5b) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *indus_gt_device::device_rom_region() const
{
	return ROM_NAME( indusgt );
}


static INPUT_PORTS_START( indusgt )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )

	PORT_START("PANEL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Error")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Drive Type")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Track")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Protect") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(indus_gt_device::protect_changed), 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( indus_gt_device::protect_changed )
{
	if (newval)
	{
		m_protect = !m_protect;
		m_protect_led = m_protect;
		wpt_callback(m_floppy, m_floppy->wpt_r() || m_protect);
	}
}

indus_gt_device::indus_gt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, INDUS_GT, tag, owner, clock)
	, m_panel(*this, "PANEL")
	, m_digits(*this, "digit%u", 0U)
	, m_power_led(*this, "power_led")
	, m_protect_led(*this, "protect_led")
{
}

void indus_gt_device::device_start()
{
	c1541_device_base::device_start();

	m_floppy->setup_wpt_cb(floppy_image_device::wpt_cb(&indus_gt_device::floppy_wpt, this));

	m_protect = false;
	m_power_led = 1;

	save_item(NAME(m_protect));
}

void indus_gt_device::device_post_load()
{
	m_protect_led = m_protect;
}

void indus_gt_device::indusgt_mem(address_map &map)
{
	c1541_mem(map);

	map(0x1201, 0x1201).r(FUNC(indus_gt_device::sensor_r));
	map(0x1400, 0x1400).w(FUNC(indus_gt_device::digit_w<0>));
	map(0x1600, 0x1600).w(FUNC(indus_gt_device::digit_w<1>));
	map(0xa000, 0xbfff).rom().region("romdisk", 0);
}

u8 indus_gt_device::sensor_r()
{
	/*

	    bit     description

	    0       ERROR
	    1       DRIVE TYPE
	    2       TRACK
	    3       parallel cable (0 = serial fast I/O)
	    4       ROM disk enable (0 = enabled)
	    5
	    6       WPT
	    7       TRACK0

	*/

	return (m_panel->read() & 0x07) | (m_floppy->wpt_r() ? 0x00 : 0x40) | (m_floppy->trk00_r() << 7);
}

void indus_gt_device::floppy_wpt(floppy_image_device *floppy, int state)
{
	wpt_callback(floppy, state || m_protect);
}

void indus_gt_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &indus_gt_device::indusgt_mem);

	config.set_default_layout(layout_indusgt);
}

ioport_constructor indus_gt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(indusgt);
}
