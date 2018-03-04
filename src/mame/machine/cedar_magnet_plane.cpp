// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"
#include "cedar_magnet_plane.h"


DEFINE_DEVICE_TYPE(CEDAR_MAGNET_PLANE, cedar_magnet_plane_device, "cedmag_plane", "Cedar Plane")


cedar_magnet_plane_device::cedar_magnet_plane_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CEDAR_MAGNET_PLANE, tag, owner, clock)
	, cedar_magnet_board_interface(mconfig, *this, "planecpu", "ram")
{
}



ADDRESS_MAP_START(cedar_magnet_plane_device::cedar_magnet_plane_map)
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(cedar_magnet_plane_device::cedar_magnet_plane_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE(0xc0, 0xc3) AM_DEVREADWRITE("z80pio0", z80pio_device, read_alt, write_alt)
	AM_RANGE(0xc4, 0xc7) AM_DEVREADWRITE("z80pio1", z80pio_device, read_alt, write_alt)

	AM_RANGE(0xcc, 0xcc) AM_WRITE(plane_portcc_w)
	AM_RANGE(0xcd, 0xcd) AM_WRITE(plane_portcd_w)
	AM_RANGE(0xce, 0xce) AM_WRITE(plane_portce_w)
	AM_RANGE(0xcf, 0xcf) AM_WRITE(plane_portcf_w)

ADDRESS_MAP_END




WRITE8_MEMBER(cedar_magnet_plane_device::plane_portcc_w)
{
	m_framebuffer[((m_curline&0xff)*0x100)+(m_lineoffset&0xff)] = data;

	// counters simply wrap when they reach the maximum, don't move onto next row/colummn (confirmed by xain)
	if (m_pio0_pa_data&0x01)
	{
		m_lineoffset++;
	}
	else
	{
		m_curline++;
	}
}

WRITE8_MEMBER(cedar_magnet_plane_device::plane_portcd_w)
{
	m_lineoffset = data;
}

WRITE8_MEMBER(cedar_magnet_plane_device::plane_portce_w)
{
	m_curline = data;

}

WRITE8_MEMBER(cedar_magnet_plane_device::plane_portcf_w)
{
	// does it have a meaning or is it just some kind of watchdog?
	m_cf_data = data;
}

MACHINE_CONFIG_START(cedar_magnet_plane_device::device_add_mconfig)
	MCFG_CPU_ADD("planecpu", Z80,4000000)
	MCFG_CPU_PROGRAM_MAP(cedar_magnet_plane_map)
	MCFG_CPU_IO_MAP(cedar_magnet_plane_io)

	MCFG_DEVICE_ADD("z80pio0", Z80PIO, 4000000/2)
//  MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(cedar_magnet_plane_device, pio0_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(cedar_magnet_plane_device, pio0_pa_w))
//  MCFG_Z80PIO_IN_PB_CB(READ8(cedar_magnet_plane_device, pio0_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(cedar_magnet_plane_device, pio0_pb_w))

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, 4000000/2)
//  MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
//  MCFG_Z80PIO_IN_PA_CB(READ8(cedar_magnet_plane_device, pio1_pa_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(cedar_magnet_plane_device, pio1_pa_w))
//  MCFG_Z80PIO_IN_PB_CB(READ8(cedar_magnet_plane_device, pio1_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(cedar_magnet_plane_device, pio1_pb_w))
MACHINE_CONFIG_END


READ8_MEMBER(cedar_magnet_plane_device::pio0_pa_r)
{
// this is read
//  logerror("%s: pio0_pa_r\n", machine().describe_context());
	return 0x00;
}


WRITE8_MEMBER(cedar_magnet_plane_device::pio0_pa_w)
{
	m_pio0_pa_data = data;

	// 7ex- 321d
	//
	// e = video enable
	// d = draw direction
	// x = done? gets set at end of each frame at least, but unlike video enable, also when video shouldn't be enabled
	// 7 = always set?
	// 321 = always set after startup?
}

WRITE8_MEMBER(cedar_magnet_plane_device::pio0_pb_w)
{
	m_pio0_pb_data = data;
}

WRITE8_MEMBER(cedar_magnet_plane_device::pio1_pa_w)
{
	m_scrollx = data;
}

WRITE8_MEMBER(cedar_magnet_plane_device::pio1_pb_w)
{
	m_scrolly = data;
}

void cedar_magnet_plane_device::device_start()
{
	save_item(NAME(m_framebuffer));
}

uint32_t cedar_magnet_plane_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase)
{
	int count = 0;

	if (!(m_pio0_pa_data & 0x40))
		return 0;

	for (int y = 0;y < 256;y++)
	{
		uint16_t *dst = &bitmap.pix16((y-m_scrolly)&0xff);

		for (int x = 0; x < 256;x++)
		{
			uint8_t pix = m_framebuffer[count];
			count++;

			if (pix) dst[(x-m_scrollx)&0xff] = pix + palbase*0x100;
		}
	}

	return 0;
}
