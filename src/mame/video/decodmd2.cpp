// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *   Data East Pinball Dot Matrix Display
 *
 *    Type 2: 128x32
 *    68B09E @ 8MHz
 *    68B45 CRTC
 */

#include "decodmd2.h"
#include "rendlay.h"

const device_type DECODMD2 = &device_creator<decodmd_type2_device>;

WRITE8_MEMBER( decodmd_type2_device::bank_w )
{
	m_rombank1->set_entry(data & 0x1f);
}

WRITE8_MEMBER( decodmd_type2_device::crtc_address_w )
{
	m_mc6845->address_w(space,offset,data);
	m_crtc_index = data;
}

READ8_MEMBER( decodmd_type2_device::crtc_status_r )
{
	return m_mc6845->register_r(space,offset);
}

WRITE8_MEMBER( decodmd_type2_device::crtc_register_w )
{
	m_mc6845->register_w(space,offset,data);
	m_crtc_reg[m_crtc_index] = data;
}

READ8_MEMBER( decodmd_type2_device::latch_r )
{
	// clear IRQ?
	m_cpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
	m_busy = false;
	return m_command;
}

WRITE8_MEMBER( decodmd_type2_device::data_w )
{
	// set IRQ?
	m_latch = data;
}

READ8_MEMBER( decodmd_type2_device::busy_r )
{
	UINT8 ret = 0x00;

	ret = (m_status & 0x0f) << 3;

	if(m_busy)
		return 0x80 | ret;
	else
		return 0x00 | ret;
}


WRITE8_MEMBER( decodmd_type2_device::ctrl_w )
{
	if(!(m_ctrl & 0x01) && (data & 0x01))
	{
		m_cpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
		m_busy = true;
		m_command = m_latch;
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
		m_rombank1->set_entry(0);
		logerror("DMD2: Reset\n");
	}
	m_ctrl = data;
}

READ8_MEMBER( decodmd_type2_device::ctrl_r )
{
	return m_ctrl;
}

READ8_MEMBER( decodmd_type2_device::status_r )
{
	return m_status;
}

WRITE8_MEMBER( decodmd_type2_device::status_w )
{
	m_status = data & 0x0f;
}

TIMER_DEVICE_CALLBACK_MEMBER(decodmd_type2_device::dmd_firq)
{
	m_cpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
}

MC6845_UPDATE_ROW( decodmd_type2_device::crtc_update_row )
{
	UINT8 *RAM = m_ram->pointer();
	UINT8 intensity;
	UINT16 addr = (ma & 0xfc00) + ((ma & 0x100)<<2) + (ra << 4);

	for (int x = 0; x < 128; x += 8)
	{
		for (int dot = 0; dot < 8; dot++)
		{
			intensity = ((RAM[addr] >> (7-dot) & 0x01) << 1) | (RAM[addr + 0x200] >> (7-dot) & 0x01);
			bitmap.pix32(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		addr++;
	}
}

static ADDRESS_MAP_START( decodmd2_map, AS_PROGRAM, 8, decodmd_type2_device )
	AM_RANGE(0x0000, 0x2fff) AM_RAMBANK("dmdram")
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(crtc_status_r,crtc_address_w)
	AM_RANGE(0x3001, 0x3001) AM_WRITE(crtc_register_w)
	AM_RANGE(0x3002, 0x3002) AM_WRITE(bank_w)
	AM_RANGE(0x3003, 0x3003) AM_READ(latch_r)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("dmdbank1") AM_WRITE(status_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("dmdbank2") // last 32k of ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( decodmd2 )
	/* basic machine hardware */
	MCFG_CPU_ADD("dmdcpu", M6809E, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(decodmd2_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("firq_timer",decodmd_type2_device,dmd_firq,attotime::from_hz(80))

	MCFG_MC6845_ADD("dmd6845", MC6845, "", XTAL_8MHz / 8)  // TODO: confirm clock speed
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(decodmd_type2_device, crtc_update_row)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_ADD("dmd",RASTER)
	MCFG_SCREEN_SIZE(128, 32)
	MCFG_SCREEN_VISIBLE_AREA(0, 128-1, 0, 32-1)
	MCFG_SCREEN_UPDATE_DEVICE("dmd6845",mc6845_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("12K")

MACHINE_CONFIG_END

machine_config_constructor decodmd_type2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( decodmd2 );
}

decodmd_type2_device::decodmd_type2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECODMD2, "Data East Pinball Dot Matrix Display Type 2", tag, owner, clock, "decodmd2", __FILE__),
		m_cpu(*this,"dmdcpu"),
		m_mc6845(*this,"dmd6845"),
		m_rombank1(*this,"dmdbank1"),
		m_rombank2(*this,"dmdbank2"),
		m_rambank(*this,"dmdram"),
		m_ram(*this,RAM_TAG)
{}

void decodmd_type2_device::device_start()
{
}

void decodmd_type2_device::device_reset()
{
	UINT8* ROM;
	UINT8* RAM = m_ram->pointer();
	m_rom = memregion(m_gfxtag);

	memset(RAM,0,0x3000);

	ROM = m_rom->base();
	m_rombank1->configure_entries(0, 32, &ROM[0x0000], 0x4000);
	m_rombank2->configure_entry(0, &ROM[0x78000]);
	m_rambank->configure_entry(0, &RAM[0]);
	m_rombank1->set_entry(0);
	m_rombank2->set_entry(0);
	m_rambank->set_entry(0);
	m_busy = false;
}

void decodmd_type2_device::static_set_gfxregion(device_t &device, std::string tag)
{
	decodmd_type2_device &cpuboard = downcast<decodmd_type2_device &>(device);
	cpuboard.m_gfxtag = tag;
}
