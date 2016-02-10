// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball / Sega Pinball Dot Matrix Display
 *
 * Type 3: 192x64
 * 68000 @ 12MHz
 * 68B45 CRTC
 */

#include "decodmd3.h"
#include "rendlay.h"

const device_type DECODMD3 = &device_creator<decodmd_type3_device>;

WRITE8_MEMBER( decodmd_type3_device::data_w )
{
	m_latch = data;
}

READ8_MEMBER( decodmd_type3_device::busy_r )
{
	UINT8 ret = 0x00;

	ret = (m_status & 0x0f) << 3;

	if(m_busy)
		return 0x80 | ret;
	else
		return 0x00 | ret;
}

WRITE8_MEMBER( decodmd_type3_device::ctrl_w )
{
	if(!(m_ctrl & 0x01) && (data & 0x01))
	{
		m_cpu->set_input_line(M68K_IRQ_1,ASSERT_LINE);
		m_busy = true;
		m_command = m_latch;
	}
	if((m_ctrl & 0x02) && !(data & 0x02))
	{
		m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
		logerror("DMD3: Reset\n");
	}
	m_ctrl = data;
}

READ16_MEMBER( decodmd_type3_device::status_r )
{
	return m_status;
}

WRITE16_MEMBER( decodmd_type3_device::status_w )
{
	m_status = data & 0x0f;
}

READ16_MEMBER( decodmd_type3_device::latch_r )
{
	// clear IRQ?
	m_cpu->set_input_line(M68K_IRQ_1,CLEAR_LINE);
	m_busy = false;
	return m_command;
}

TIMER_DEVICE_CALLBACK_MEMBER(decodmd_type3_device::dmd_irq)
{
	m_cpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
}

WRITE16_MEMBER( decodmd_type3_device::crtc_address_w )
{
	if(ACCESSING_BITS_8_15)
	{
		m_mc6845->address_w(space,offset,data >> 8);
		m_crtc_index = data >> 8;
	}
}

READ16_MEMBER( decodmd_type3_device::crtc_status_r )
{
	if(ACCESSING_BITS_8_15)
		return m_mc6845->register_r(space,offset);
	else
		return 0xff;
}

WRITE16_MEMBER( decodmd_type3_device::crtc_register_w )
{
	if(ACCESSING_BITS_8_15)
	{
		if(m_crtc_index == 9)  // hack!!
			data -= 0x100;
		m_mc6845->register_w(space,offset,data >> 8);
		m_crtc_reg[m_crtc_index] = data >> 8;
	}
}

MC6845_UPDATE_ROW( decodmd_type3_device::crtc_update_row )
{
	UINT8 *RAM = m_ram->pointer();
	UINT8 intensity;
	UINT16 addr = ((ma & 0x7ff) << 2) | ((ra & 0x02) << 12);
	addr += ((ra & 0x01) * 24);

	for (int x = 0; x < 192; x += 16)
	{
		for (int dot = 0; dot < 8; dot++)
		{
			intensity = ((RAM[addr + 1] >> (7-dot) & 0x01) << 1) | (RAM[addr + 0x801] >> (7-dot) & 0x01);
			bitmap.pix32(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		for (int dot = 8; dot < 16; dot++)
		{
			intensity = ((RAM[addr] >> (15-dot) & 0x01) << 1) | (RAM[addr + 0x800] >> (15-dot) & 0x01);
			bitmap.pix32(y, x + dot) = rgb_t(0x3f * intensity, 0x2a * intensity, 0x00);
		}
		addr += 2;
	}
}

static ADDRESS_MAP_START( decodmd3_map, AS_PROGRAM, 16, decodmd_type3_device )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROMBANK("dmdrom")
	AM_RANGE(0x00800000, 0x0080ffff) AM_RAMBANK("dmdram")
	AM_RANGE(0x00c00010, 0x00c00011) AM_READWRITE(crtc_status_r,crtc_address_w)
	AM_RANGE(0x00c00012, 0x00c00013) AM_WRITE(crtc_register_w)
	AM_RANGE(0x00c00020, 0x00c00021) AM_READWRITE(latch_r,status_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( decodmd3 )
	/* basic machine hardware */
	MCFG_CPU_ADD("dmdcpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(decodmd3_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer",decodmd_type3_device,dmd_irq,attotime::from_hz(150))

	MCFG_MC6845_ADD("dmd6845", MC6845, nullptr, XTAL_12MHz / 4)  // TODO: confirm clock speed
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(16)
	MCFG_MC6845_UPDATE_ROW_CB(decodmd_type3_device, crtc_update_row)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_SCREEN_ADD("dmd",RASTER)
	MCFG_SCREEN_SIZE(192, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 192-1, 0, 64-1)
	MCFG_SCREEN_UPDATE_DEVICE("dmd6845",mc6845_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

machine_config_constructor decodmd_type3_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( decodmd3 );
}

decodmd_type3_device::decodmd_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DECODMD3, "Data East Pinball Dot Matrix Display Type 3", tag, owner, clock, "decodmd3", __FILE__),
		m_cpu(*this,"dmdcpu"),
		m_mc6845(*this,"dmd6845"),
		m_ram(*this,RAM_TAG),
		m_rambank(*this,"dmdram"),
		m_rombank(*this,"dmdrom")
{}

void decodmd_type3_device::device_start()
{
}

void decodmd_type3_device::device_reset()
{
	UINT8* ROM;
	UINT8* RAM = m_ram->pointer();
	m_rom = memregion(m_gfxtag);

	ROM = m_rom->base();
	memset(RAM,0,0x10000);
	m_rambank->configure_entry(0, &RAM[0]);
	m_rambank->set_entry(0);
	m_rombank->configure_entry(0, &ROM[0]);
	m_rombank->set_entry(0);
}

void decodmd_type3_device::static_set_gfxregion(device_t &device, const char *tag)
{
	decodmd_type3_device &cpuboard = downcast<decodmd_type3_device &>(device);
	cpuboard.m_gfxtag = tag;
}
