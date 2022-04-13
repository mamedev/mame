// license:BSD-3-Clause
// copyright-holders:David Haywood


#include "emu.h"
#include "cedar_magnet_plane.h"


DEFINE_DEVICE_TYPE(CEDAR_MAGNET_PLANE, cedar_magnet_plane_device, "cedmag_plane", "Cedar Plane")


cedar_magnet_plane_device::cedar_magnet_plane_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CEDAR_MAGNET_PLANE, tag, owner, clock)
	, cedar_magnet_board_interface(mconfig, *this, "planecpu", "ram")
{
}


void cedar_magnet_plane_device::cedar_magnet_plane_map(address_map &map)
{
	map(0x0000, 0xffff).ram().share("ram");
}

void cedar_magnet_plane_device::cedar_magnet_plane_io(address_map &map)
{
	map.global_mask(0xff);

	map(0xc0, 0xc3).rw("z80pio0", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xc4, 0xc7).rw("z80pio1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));

	map(0xcc, 0xcc).w(FUNC(cedar_magnet_plane_device::plane_portcc_w));
	map(0xcd, 0xcd).w(FUNC(cedar_magnet_plane_device::plane_portcd_w));
	map(0xce, 0xce).w(FUNC(cedar_magnet_plane_device::plane_portce_w));
	map(0xcf, 0xcf).w(FUNC(cedar_magnet_plane_device::plane_portcf_w));

}


void cedar_magnet_plane_device::plane_portcc_w(u8 data)
{
	m_framebuffer[((m_curline & 0xff) * 0x100) + (m_lineoffset & 0xff)] = data;

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

void cedar_magnet_plane_device::plane_portcd_w(u8 data)
{
	m_lineoffset = data;
}

void cedar_magnet_plane_device::plane_portce_w(u8 data)
{
	m_curline = data;
}

void cedar_magnet_plane_device::plane_portcf_w(u8 data)
{
	// does it have a meaning or is it just some kind of watchdog?
	m_cf_data = data;
}

void cedar_magnet_plane_device::device_add_mconfig(machine_config &config)
{
	z80_device &planecpu(Z80(config, "planecpu", 4000000));
	planecpu.set_addrmap(AS_PROGRAM, &cedar_magnet_plane_device::cedar_magnet_plane_map);
	planecpu.set_addrmap(AS_IO, &cedar_magnet_plane_device::cedar_magnet_plane_io);

	z80pio_device& pio0(Z80PIO(config, "z80pio0", 4000000/2));
//  pio0.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);
	pio0.in_pa_callback().set(FUNC(cedar_magnet_plane_device::pio0_pa_r));
	pio0.out_pa_callback().set(FUNC(cedar_magnet_plane_device::pio0_pa_w));
//  pio0.in_pb_callback().set(FUNC(cedar_magnet_plane_device::pio0_pb_r));
	pio0.out_pb_callback().set(FUNC(cedar_magnet_plane_device::pio0_pb_w));

	z80pio_device& pio1(Z80PIO(config, "z80pio1", 4000000/2));
//  pio1.out_int_callback().set_inputline("maincpu", INPUT_LINE_IRQ0);
//  pio1.in_pa_callback().set(FUNC(cedar_magnet_plane_device::pio1_pa_r));
	pio1.out_pa_callback().set(FUNC(cedar_magnet_plane_device::pio1_pa_w));
//  pio1.in_pb_callback().set(FUNC(cedar_magnet_plane_device::pio1_pb_r));
	pio1.out_pb_callback().set(FUNC(cedar_magnet_plane_device::pio1_pb_w));
}


u8 cedar_magnet_plane_device::pio0_pa_r()
{
// this is read
//  logerror("%s: pio0_pa_r\n", machine().describe_context());
	return 0x00;
}


void cedar_magnet_plane_device::pio0_pa_w(u8 data)
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

void cedar_magnet_plane_device::pio0_pb_w(u8 data)
{
	m_pio0_pb_data = data;
}

void cedar_magnet_plane_device::pio1_pa_w(u8 data)
{
	m_scrollx = data;
}

void cedar_magnet_plane_device::pio1_pb_w(u8 data)
{
	m_scrolly = data;
}

void cedar_magnet_plane_device::device_start()
{
	m_framebuffer = make_unique_clear<u8[]>(0x10000);
	save_pointer(NAME(m_framebuffer), 0x10000);
}

u32 cedar_magnet_plane_device::draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int palbase)
{
	int count = 0;

	if (!(m_pio0_pa_data & 0x40))
		return 0;

	for (int y = 0; y < 256;y++)
	{
		u16 *const dst = &bitmap.pix((y - m_scrolly) & 0xff);

		for (int x = 0; x < 256;x++)
		{
			u8 pix = m_framebuffer[count];
			count++;

			if (pix) dst[(x - m_scrollx) & 0xff] = pix + palbase * 0x100;
		}
	}

	return 0;
}
