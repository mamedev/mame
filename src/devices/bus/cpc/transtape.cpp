// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * transtape.c  --  Hard Micro SA Transtape
 *
 * Spanish hacking device
 *
 * Further info at - http://cpcwiki.eu/index.php/Transtape
 */

#include "transtape.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_TRANSTAPE = &device_creator<cpc_transtape_device>;

ROM_START( cpc_transtape )
	ROM_REGION( 0x4000, "tt_rom", 0 )
	ROM_LOAD( "tta.rom",   0x0000, 0x4000, CRC(c568da76) SHA1(cc509d21216bf11d40f9a3e0791ef7f4ada03790) )
ROM_END

const rom_entry *cpc_transtape_device::device_rom_region() const
{
	return ROM_NAME( cpc_transtape );
}

static INPUT_PORTS_START(cpc_transtape)
	PORT_START("transtape")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Red Button") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF,cpc_transtape_device,button_red_w,nullptr)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Black Button") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF,cpc_transtape_device,button_black_w,nullptr)
INPUT_PORTS_END

ioport_constructor cpc_transtape_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cpc_transtape );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_transtape_device::cpc_transtape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_TRANSTAPE, "HM Transtape", tag, owner, clock, "cpc_transtape", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr), m_cpu(nullptr), m_space(nullptr), m_ram(nullptr),
	m_rom_active(false),
	m_romen(true),
	m_output(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_transtape_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());
	m_cpu = static_cast<cpu_device*>(machine().device("maincpu"));
	m_space = &m_cpu->space(AS_IO);

	m_ram = make_unique_clear<UINT8[]>(0x2000);

	m_space->install_write_handler(0xfbf0,0xfbf0,0,0,write8_delegate(FUNC(cpc_transtape_device::output_w),this));
	m_space->install_read_handler(0xfbff,0xfbff,0,0,read8_delegate(FUNC(cpc_transtape_device::input_r),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_transtape_device::device_reset()
{
	// TODO
	m_rom_active = false;
	m_output = 0;
}

void cpc_transtape_device::map_enable()
{
	UINT8* ROM = memregion("tt_rom")->base();
	if(m_output & 0x02)  // ROM enable
	{
		membank(":bank1")->set_base(ROM);
		membank(":bank2")->set_base(ROM+0x2000);
	}
	if(m_output & 0x01)  // RAM enable
	{
		membank(":bank7")->set_base(m_ram.get());
		membank(":bank15")->set_base(m_ram.get());
		membank(":bank8")->set_base(m_ram.get());  // repeats in second 8kB
		membank(":bank16")->set_base(m_ram.get());
	}
}

INPUT_CHANGED_MEMBER(cpc_transtape_device::button_red_w)
{
	// enables device ROM at 0x0000, RAM at 0xc000, generates NMI
	if(newval & 0x01)
	{
		m_output |= 0x1f;
		map_enable();
		m_cpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
	}
}

INPUT_CHANGED_MEMBER(cpc_transtape_device::button_black_w)
{
	// enables device ROM at 0x0000, RAM at 0xc000(?), force execution to start at 0x0000
	if(newval & 0x01)
	{
		m_output |= 0x1f;
		map_enable();
		m_cpu->set_pc(0);
	}
}

READ8_MEMBER(cpc_transtape_device::input_r)
{
	// TODO
	return 0x80;
}

WRITE8_MEMBER(cpc_transtape_device::output_w)
{
	// TODO
	m_output = data;
	m_slot->rom_select(space,0,get_rom_bank());  // trigger rethink
}

void cpc_transtape_device::set_mapping(UINT8 type)
{
	if(type != MAP_OTHER)
		return;
	map_enable();
}
