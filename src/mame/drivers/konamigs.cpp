// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

	Konami GSAN1 hardware
	(c) 2000

	CPU: Hitachi HD6417709 SH-3
	GPU: Hitachi HD64413AF 'Q2SD' Quick 2D Graphics Renderer with Synchronous DRAM Interface
	SPU: Yamaha YMZ280B-F
	Misc:
	     Altera Max EPM3256ATC144-10
		 Altera Max EPM3064ATC100-10
		 Epson RTC-4553

	Known games (preliminary, some of listed below might not belong to this hardware):
	 Dance Dance Revolution Kids
	 Muscle Ranking Football Masters
	 Muscle Ranking Kick Target
	*Muscle Ranking Spray Hitter
	 Muscle Ranking Struck Out
	 Neratte Don Don
	 Pikkari Chance
	 Run Run Puppy
	 Soreike! Hanapuu

	* denotes these games are archived

	TODO:
	 - currently implemented very basic set of Q2SD GPU features, required/used by Spray Hitter, should be improved if more games will be found.
	 - hook IRQs from GPU and SPU (not used by Spray Hitter)

	Notes:
	 - hold Test + Service while booting to initialise RTC NVRAM

**************************************************************************/

#include "emu.h"
#include "cpu/sh/sh3comn.h"
#include "cpu/sh/sh4.h"
#include "bus/ata/ataintf.h"
#include "machine/ataflash.h"
#include "machine/s3520cf.h"
#include "machine/ticket.h"
#include "sound/ymz280b.h"
#include "speaker.h"
#include "screen.h"

class gsan_state : public driver_device
{
public:
	gsan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ata(*this, "ata")
		, m_hopper(*this, "hopper")
		, m_rtc_r(*this, "RTCR")
		, m_rtc_w(*this, "RTCW")
		, m_dipsw_r(*this, "DSW")
		, m_vram(*this, "vram", 0)
		, m_gpuregs(*this, "gpu_regs", 0)
	{ }

	void gsan(machine_config &config);
	void init_gsan();

protected:
	required_device<sh34_base_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_device<hopper_device> m_hopper;
	required_ioport m_rtc_r;
	required_ioport m_rtc_w;
	required_ioport m_dipsw_r;
	required_shared_ptr<u16> m_vram;
	required_shared_ptr<u16> m_gpuregs;

	void main_map(address_map &map);
	void main_port(address_map &map);
	void ymz280b_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(cf_regs_r);
	DECLARE_WRITE16_MEMBER(cf_regs_w);
	DECLARE_READ16_MEMBER(cf_data_r);
	DECLARE_WRITE16_MEMBER(cf_data_w);
	DECLARE_READ8_MEMBER(rtc_r);
	DECLARE_WRITE8_MEMBER(rtc_w);
	DECLARE_READ64_MEMBER(portc_r);
	DECLARE_WRITE64_MEMBER(portc_w);
	DECLARE_READ64_MEMBER(porte_r);
	DECLARE_WRITE64_MEMBER(porte_w);
	DECLARE_READ16_MEMBER(dipsw_r);
	u8 m_portc_data = 0xff;
	u8 m_porte_data = 0xff;

	// Q2SD GPU
	DECLARE_READ16_MEMBER(gpu_r);
	DECLARE_WRITE16_MEMBER(gpu_w);
	DECLARE_READ16_MEMBER(vram_r);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE_LINE_MEMBER(vblank);
	void draw_quad(u16 cmd, u16 *data);
	void fill_quad(u16 cmd, u16 *data);
	void draw_line(u16 cmd, u16 *data);
	u32 pix_address(u16 x, u16 y) { return  (x & 0xf) | ((y & 0xf) << 4) | ((x & ~0xf) << 4) | ((y & ~0xf) << 9); };
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



// CF interface, looks like standard memory-mapped ATA layout, probably should be devicified
READ16_MEMBER(gsan_state::cf_regs_r)
{
	offset *= 2;
	u16 data = 0;
	if (ACCESSING_BITS_0_7)
		data |= m_ata->read_cs0(offset, 0xff) & 0xff;
	if (ACCESSING_BITS_8_15)
		data |= (m_ata->read_cs0(offset + 1, 0xff) << 8);
	return data;
}

WRITE16_MEMBER(gsan_state::cf_regs_w)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		m_ata->write_cs0(offset, data & 0xff, 0xff);
	if (ACCESSING_BITS_8_15)
		m_ata->write_cs0(offset + 1, data >> 8, 0xff);
}

READ16_MEMBER(gsan_state::cf_data_r)
{
	u16 data = m_ata->read_cs0(0, 0xffff);
	return data;
}

WRITE16_MEMBER(gsan_state::cf_data_w)
{
	m_ata->write_cs0(0, data, 0xffff);
}

// misc I/O
READ16_MEMBER(gsan_state::dipsw_r)
{
	return m_dipsw_r->read();
}
READ8_MEMBER(gsan_state::rtc_r)
{
	return m_rtc_r->read();
}
WRITE8_MEMBER(gsan_state::rtc_w)
{
	m_rtc_w->write(data);
}

// SH-3 GPIO output ports
READ64_MEMBER(gsan_state::portc_r)
{
	return m_portc_data;
}
WRITE64_MEMBER(gsan_state::portc_w)
{
	m_portc_data = data;
	m_hopper->motor_w(data & 0x80);
	machine().bookkeeping().coin_counter_w(0, data & 4);
	machine().bookkeeping().coin_counter_w(1, data & 1);
}
READ64_MEMBER(gsan_state::porte_r)
{
	return m_porte_data;
}
WRITE64_MEMBER(gsan_state::porte_w)
{
	// lamps
#if 0
	u8 mask = m_porte_data ^ data;
	if (mask)
		logerror("PORT_E mask %02X val %02X\n", mask, data & mask);
#endif
	m_porte_data = data;
}



// Q2SD GPU
READ16_MEMBER(gsan_state::vram_r)
{
	return m_vram[offset];
}

WRITE16_MEMBER(gsan_state::vram_w)
{
	COMBINE_DATA(&m_vram[offset]);
}

READ16_MEMBER(gsan_state::gpu_r)
{
	return m_gpuregs[offset];
}

WRITE16_MEMBER(gsan_state::gpu_w)
{
	u16 prevval = m_gpuregs[offset];
	COMBINE_DATA(&m_gpuregs[offset]);
	data = m_gpuregs[offset];
#if 0
	if (prevval != data)
		logerror("Q2SD reg %02X %04X\n", offset*2, data);
#endif

	switch (offset)
	{
	case 0x000: // System control
		if (data & 0x8000) // reset
		{
			m_gpuregs[0x000] &= ~0x0400;
			m_gpuregs[0x001] &= ~0x1680;
		}
		if (data & 0x4000) // display reset
		{
			m_gpuregs[0x001] &= ~0x0800;
		}
		if (data & 0x0100) // start render
		{
			m_gpuregs[0x000] &= ~0x0100;

			u32 listoffs = ((m_gpuregs[0x018 / 2] << 16) | m_gpuregs[0x01a / 2]) / 2;
			bool end_of_list = false;
			do
			{
				u16 cmd = m_vram[listoffs++];
				switch (cmd >> 11)
				{
				case 0x00: // POLYGON4A
					draw_quad(cmd, &m_vram[listoffs]);
					listoffs += 12;
					break;
//				case 0x01: // POLYGON4B
//					listoffs += 14;
//					break;
				case 0x02: // POLYGON4C
					fill_quad(cmd, &m_vram[listoffs]);
					listoffs += 9;
					break;
				case 0x0c: // LINE
					draw_line(cmd, &m_vram[listoffs]);
					listoffs += m_vram[listoffs + 1] * 2 + 2;
					break;
				case 0x12: // LCOFS
					m_gpuregs[0x84 / 2] = m_vram[listoffs++];
					m_gpuregs[0x86 / 2] = m_vram[listoffs++];
					break;
				case 0x16: // WPR
					gpu_w(space, m_vram[listoffs] & 0x3ff, m_vram[listoffs + 1]);
					listoffs += 2;
					break;
				case 0x17: // SCLIP
					m_gpuregs[0x90 / 2] = m_vram[listoffs++];
					m_gpuregs[0x92 / 2] = m_vram[listoffs++];
					break;
				case 0x1a: // VBKEM
					// TODO somehow wait vblank
					listoffs += 2;
					break;
				case 0x1e: // NOP3
					listoffs += 2;
					break;
				case 0x1f: // TRAP
					m_gpuregs[0x001] |= 0x0400;
					end_of_list = true;
					break;
				default:
					logerror("Q2SD not implemented command %04X addr %08X\n", cmd, (listoffs - 1) * 2);
					break;
				}
			} while (!end_of_list);
		}
		break;
	case 0x001: // Status (read only)
		m_gpuregs[offset] = prevval;
		break;
	case 0x002: // Status clear
		m_gpuregs[0x001] &= ~(data & 0xff80);
		break;
	case 0x003: // Interrupt mask, always set to 0, no interrupts ? so lame
		if (data)
			logerror("Q2SD enable IRQ %04X !\n", data);
		break;
	}
}

WRITE_LINE_MEMBER(gsan_state::vblank)
{
	if (state)
	{
		m_gpuregs[0x001] |= 0x0800; // VBLANK
		m_gpuregs[0x000] &= ~0x0200;
	}
}

void gsan_state::draw_quad(u16 cmd, u16 *data)
{
	//	logerror("Q2SD draw %04X src %d:%d sz %d:%d dst %d:%d %d:%d %d:%d %d:%d\n", cmd, data[0], data[1], data[2], data[3], (s16)data[4], (s16)data[5], (s16)data[6], (s16)data[7], (s16)data[8], (s16)data[9], (s16)data[10], (s16)data[11]);
	if (cmd & 0x5ff)
		logerror("Q2SD unhandled draw mode %04X\n", cmd);
	u32 ssx = data[0];
	u32 ssy = data[1];
	s16 sdx = data[4];
	s16 sdy = data[5];
	s16 edx = data[8];
	s16 edy = data[9];
	s16 sclipx = m_gpuregs[0x90 / 2];
	s16 sclipy = m_gpuregs[0x92 / 2];

	s16 ddx = (edx >= sdx) ? 1 : -1;
	s16 ddy = (edy >= sdy) ? 1 : -1;
	edx += ddx;
	edy += ddy;

	u32 fg_offs = (m_gpuregs[0x014 / 2] & 0x7f) << 15;
	u32 src_offs = (((m_gpuregs[0x01c / 2] & 0x7f) << 16) | (m_gpuregs[0x01c / 2] & 0xe000)) / 2;

	bool opaq = !(cmd & 0x0200);

	for (int y = sdy, sy = 0; y != edy; y += ddy, sy++)
		for (int x = sdx, sx = 0; x != edx; x += ddx, sx++)
			if ((x >= 0 && x <= sclipx) && (y >= 0 && y <= sclipy))
			{
				u16 pix = m_vram[src_offs + pix_address(ssx + sx, ssy + sy)];
				if (pix || opaq)
					m_vram[fg_offs + pix_address(x, y)] = pix;
			}
}

void gsan_state::fill_quad(u16 cmd, u16 *data)
{
	//	logerror("Q2SD fill dst %d:%d %d:%d %d:%d %d:%d col %04X\n", (s16)data[0], (s16)data[1], (s16)data[2], (s16)data[3], (s16)data[4], (s16)data[5], (s16)data[6], (s16)data[7], data[8]);
	if (cmd & 0x7ff)
		logerror("Q2SD unhandled draw mode %04X\n", cmd);
	s16 sdx = data[0];
	s16 sdy = data[1];
	s16 edx = data[4];
	s16 edy = data[5];
	u16 color = data[8];
	s16 sclipx = m_gpuregs[0x90 / 2];
	s16 sclipy = m_gpuregs[0x92 / 2];

	s16 ddx = (edx >= sdx) ? 1 : -1;
	s16 ddy = (edy >= sdy) ? 1 : -1;
	edx += ddx;
	edy += ddy;

	u32 fg_offs = (m_gpuregs[0x014 / 2] & 0x7f) << 15;

	for (int y = sdy; y != edy; y += ddy)
		for (int x = sdx; x != edx; x += ddx)
		{
			if ((x >= 0 && x <= sclipx) && (y >= 0 && y <= sclipy))
				m_vram[fg_offs + pix_address(x, y)] = color;
		}
}

void gsan_state::draw_line(u16 cmd, u16 *data)
{
	if (cmd & 0x7ff)
		logerror("Q2SD unhandled line mode %04X\n", cmd);

	u16 color = *data++;
	u16 count = *data++;
	while (count > 1)
	{
		s16 x0 = *data++;
		s16 y0 = *data++;
		s16 x1 = data[0];
		s16 y1 = data[1];
		--count;

		int dx = abs(x1 - x0);
		int dy = -abs(y1 - y0);
		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;

		s16 sclipx = m_gpuregs[0x90 / 2];
		s16 sclipy = m_gpuregs[0x92 / 2];
		u32 fg_offs = (m_gpuregs[0x014 / 2] & 0x7f) << 15;

		while (true)
		{
			if ((x0 >= 0 && x0 <= sclipx) && (y0 >= 0 && y0 <= sclipy))
				m_vram[fg_offs + pix_address(x0, y0)] = color;

			if (x0 == x1 && y0 == y1)
				break;

			int e2 = 2 * err;
			if (e2 >= dy)
			{
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx)
			{
				err += dx;
				y0 += sy;
			}
		}
	}
}

u32 gsan_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_gpuregs[0x000 / 2] & 0x2000)
	{
		u32 fg_offs = (m_gpuregs[0x014 / 2] & 0x7f) << 15;
		bool fg_en = !(m_gpuregs[0x056 / 2] & 8);
		bool bg_en = m_gpuregs[0x00a / 2] & 0x0400;

		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				u16 pix = 0;
				u32 idx = pix_address(x, y);
				if (fg_en)
					pix = m_vram[fg_offs + idx];
				if (bg_en && pix == 0)
					pix = m_vram[idx];
				bitmap.pix32(y, x) = pal565(pix, 11, 5, 0);
			}
	}
	else
		bitmap.fill(rgb_t::black(), cliprect);
	return 0;
}



void gsan_state::main_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("maincpu", 0);
	map(0x08000000, 0x083fffff).rw(FUNC(gsan_state::vram_r), FUNC(gsan_state::vram_w)).share("vram");
	map(0x0c000000, 0x0c3fffff).ram().share("main_ram");
	map(0x10000000, 0x100007ff).rw(FUNC(gsan_state::gpu_r), FUNC(gsan_state::gpu_w)).share("gpu_regs");
	// misc I/O
	map(0x14000800, 0x14000807).rw(FUNC(gsan_state::cf_regs_r), FUNC(gsan_state::cf_regs_w));
	map(0x14000c00, 0x14000c03).rw(FUNC(gsan_state::cf_data_r), FUNC(gsan_state::cf_data_w));
	map(0x14001000, 0x14001001).r(FUNC(gsan_state::dipsw_r));
	map(0x14001019, 0x14001019).w(FUNC(gsan_state::rtc_w));
	map(0x14001039, 0x14001039).r(FUNC(gsan_state::rtc_r));

	map(0x18000000, 0x18000001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0x1f000000, 0x1f000fff).ram(); // cache RAM-mode (SH3 internal), actually should be 7Fxxxxxx, but current SH3 core doesn't like 7Fxxxxxx
	map(0xa0000000, 0xa000ffff).rom().region("maincpu", 0); // uncached mirror, otherwise no disassembly can bee seen in debugger (bug?)
}

void gsan_state::main_port(address_map &map)
{
	map(SH3_PORT_C, SH3_PORT_C + 7).rw(FUNC(gsan_state::portc_r), FUNC(gsan_state::portc_w));
	map(SH3_PORT_E, SH3_PORT_E + 7).rw(FUNC(gsan_state::porte_r), FUNC(gsan_state::porte_w));
	map(SH3_PORT_F, SH3_PORT_F + 7).portr("PORT_F");
	map(SH3_PORT_L, SH3_PORT_L + 7).portr("PORT_L");
}

void gsan_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x3fffff).ram();
}


static INPUT_PORTS_START( gsan )
	PORT_START("PORT_L")
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("PORT_F")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Medal")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // looks like not regular coin in to play, but coins for medals exchange
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin Slot 1" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0003, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0000, "5 Coins/2 Credits" )
	PORT_DIPNAME( 0x0078, 0x0078, "Coin Slot 2" ) PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(      0x0078, "2 Medals" )
//	PORT_DIPSETTING(      0x0070, "2 Medals" )
	PORT_DIPSETTING(      0x0068, "3 Medals" )
	PORT_DIPSETTING(      0x0060, "4 Medals" )
	PORT_DIPSETTING(      0x0058, "5 Medals" )
	PORT_DIPSETTING(      0x0050, "6 Medals" )
	PORT_DIPSETTING(      0x0048, "7 Medals" )
	PORT_DIPSETTING(      0x0040, "8 Medals" )
	PORT_DIPSETTING(      0x0038, "9 Medals" )
	PORT_DIPSETTING(      0x0030, "10 Medals" )
	PORT_DIPSETTING(      0x0028, "11 Medals" )
	PORT_DIPSETTING(      0x0020, "12 Medals" )
	PORT_DIPSETTING(      0x0018, "13 Medals" )
	PORT_DIPSETTING(      0x0010, "14 Medals" )
	PORT_DIPSETTING(      0x0008, "15 Medals" )
	PORT_DIPSETTING(      0x0000, "16 Medals" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0000, "Standard of Payout" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0f00, "15%" )
	PORT_DIPSETTING(      0x0e00, "20%" )
	PORT_DIPSETTING(      0x0d00, "25%" )
	PORT_DIPSETTING(      0x0c00, "30%" )
	PORT_DIPSETTING(      0x0b00, "35%" )
	PORT_DIPSETTING(      0x0a00, "40%" )
	PORT_DIPSETTING(      0x0900, "45%" )
	PORT_DIPSETTING(      0x0800, "50%" )
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x3000, 0x0000, "Play Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "1" )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x4000, 0x4000, "Backup Memory" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "Keep" )
	PORT_DIPSETTING(      0x0000, "Clear" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("RTCW")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_dir_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_cs_line)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, write_bit)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_clock_line)

	PORT_START("RTCR")
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", rtc4553_device, read_bit)
INPUT_PORTS_END

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void gsan_state::machine_start()
{
	save_item(NAME(m_portc_data));
	save_item(NAME(m_porte_data));
}

void gsan_state::machine_reset()
{
	memset(&m_gpuregs[0], 0, 0x800);
	m_gpuregs[0] = 0xc000;
	m_gpuregs[1] = 0x0044;
}

static void gsan_devices(device_slot_interface &device)
{
	device.option_add("cfcard", ATA_FLASH_PCCARD);
}

void gsan_state::gsan(machine_config &config)
{
	// basic machine hardware
	// SH7709 is earlier version of SH7709S (cv1k), not exact same, have minor differences
	SH3BE(config, m_maincpu, 32_MHz_XTAL * 2); // not verified, TODO check TMU clock, here it should be XTAL*2/3
	m_maincpu->set_md(0, 0);  // none of this is verified
	m_maincpu->set_md(1, 0);  // (the sh3 is different to the sh4 anyway, should be changed)
	m_maincpu->set_md(2, 0);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(32_MHz_XTAL * 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gsan_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gsan_state::main_port);

	// misc
	ATA_INTERFACE(config, m_ata).options(gsan_devices, "cfcard", nullptr, true);
	RTC4553(config, "rtc");
	HOPPER(config, "hopper", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_HIGH);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(gsan_state::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.screen_vblank().set(FUNC(gsan_state::vblank));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16.9344_MHz_XTAL));
	ymz.set_addrmap(0, &gsan_state::ymz280b_map);
	ymz.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gsan_state::init_gsan()
{
	m_maincpu->sh2drc_set_options(SH2DRC_FASTEST_OPTIONS);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0000ffff, 0, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0c3fffff, 1, memshare("main_ram")->ptr());
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( musclhit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsan5-a.u17", 0x00000, 0x10000, CRC(6ae1d1e8) SHA1(3224e4b8198aa38c094088456281cbd62c085407) )

	ROM_REGION( 0x0f, "rtc", 0 )
	ROM_LOAD( "nvram.u9", 0x00, 0x0f, CRC(17614a6a) SHA1(f4714659937e7dd3eedc18bbedc4b3000134df16) )

	DISK_REGION( "ata:0:cfcard:image" )
	DISK_IMAGE( "gsan6_a-213", 0, SHA1(d9e7a350428d1621fc70e81561390c01837a94c0) )
ROM_END



//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2000, musclhit,      0, gsan, gsan, gsan_state, init_gsan, ROT0, "Konami / TBS", "Muscle Ranking Kinniku Banzuke Spray Hitter", MACHINE_IMPERFECT_GRAPHICS|MACHINE_SUPPORTS_SAVE )
