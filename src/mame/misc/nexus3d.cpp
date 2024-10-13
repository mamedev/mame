// license:BSD-3-Clause
// copyright-holders:Scott Stone
/************************************************************************

    NEXUS 3D Version 1.0 Board from Interpark

    Games on this platform:

    Arcana Heart FULL, Examu Inc, 2006

    MagicEyes VRENDER 3D Soc (200 MHz ARM920T CPU / GFX / Sound)
    Also Has 2x QDSP QS1000 for sound

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/nandflash.h"
#include "emupal.h"
#include "screen.h"
#include "debugger.h"

//#include "machine/i2cmem.h"


namespace {

class nexus3d_state : public driver_device
{
public:
	nexus3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_fbram(*this, "fbram"),
		m_nand(*this, "nand"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void nexus3d(machine_config &config);

	void init_acheart();
	void init_acheartf();

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_fbram;
	required_device<samsung_k9f2g08u0m_device> m_nand;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

//  uint32_t nexus3d_unk2_r();
//  uint32_t nexus3d_unk3_r();
//  void nexus3d_unk2_w(uint32_t data);
//  void nexus3d_unk3_w(uint32_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void nexus3d_map(address_map &map) ATTR_COLD;

	uint32_t m_intpend = 0, m_intmask = 0, m_intlevel = 0;
	uint32_t int_pending_r();
	void int_ack_w(uint32_t data);
	uint32_t int_level_r();
	uint32_t int_mask_r();
	void int_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void IntReq(int level);

	uint32_t vrender3d_status_r();
	void rop_data_w(uint32_t data);
	void rop_register_w(uint16_t data);
	uint16_t rop_status_r();

	uint32_t timer_status_r();
	void timer_status_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t timer_count_r();
	void timer_count_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t m_timer_status = 0;
	uint32_t m_timer_count = 0;
	emu_timer  *m_timer = nullptr;
	TIMER_CALLBACK_MEMBER(timercb);
	bool m_timer_irq = false;
	bool m_timer_result = false;

	uint32_t crtc_vblank_r();
};

void nexus3d_state::video_start()
{
	// ...
}

uint32_t nexus3d_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const fbram = reinterpret_cast<uint16_t *>(m_fbram.target());
	int const width = 640;

	uint16_t const *const visible = fbram + (m_screen->frame_number() & 1) * (0x96000/2);

	uint32_t const dx = cliprect.left();
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		std::copy_n(&visible[(y * width) + dx], width, &bitmap.pix(y, dx));

	return 0;
}

uint32_t nexus3d_state::vrender3d_status_r()
{
	return (0xbf<<16) | m_screen->vpos();
}

void nexus3d_state::IntReq(int level)
{
	if (level != -1)
	{
		m_intlevel = level;
		m_intpend |= 1 << level;
	}
	uint32_t inten = m_intmask ^ 0xffffffff;

	if (m_intpend & inten)
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);
}


uint32_t nexus3d_state::int_mask_r()
{
	return m_intmask;
}

void nexus3d_state::int_mask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intmask);
}

uint32_t nexus3d_state::int_pending_r()
{
	return m_intpend;
}

void nexus3d_state::int_ack_w(uint32_t data)
{
	m_intpend &= ~data;
	IntReq(-1);
}

uint32_t nexus3d_state::int_level_r()
{
	return m_intlevel;
}

void nexus3d_state::rop_register_w(uint16_t data)
{
	//printf("%04x REG\n",data);
}

void nexus3d_state::rop_data_w(uint32_t data)
{
	//printf("%04x DATA\n",data);
}

uint16_t nexus3d_state::rop_status_r()
{
	return machine().rand() & 0x1000;
}

uint32_t nexus3d_state::timer_status_r()
{
	uint32_t res = (m_timer_status & ~0x30) | ((m_timer_irq == true) << 5) | ((m_timer_result == true) << 4);
	return res;
}

void nexus3d_state::timer_status_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_timer_status);
	//printf("%08x %08x\n",m_timer_status, m_timer_count);

	if (m_timer_status & 0x20)
		m_timer_irq = false;

	if (m_timer_status & 8)
	{
		m_timer_result = false;
		// TODO: unknown formula, should be counter / (bits 0-1 and maybe 2)
		attotime period = attotime::from_hz(14318180 * 3);
		m_timer->adjust(period);
	}
}

uint32_t nexus3d_state::timer_count_r()
{
	return m_timer_count;
}

void nexus3d_state::timer_count_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_timer_count);
}

TIMER_CALLBACK_MEMBER(nexus3d_state::timercb)
{
	m_timer_result = true;
	m_timer_status &= ~8;
	#if 0
	if (m_timer_irq == false && m_timer_status & 0x10)
	{
		m_timer_irq = true;
		// lv 10 (the only enabled irq at POST) should be UART
		IntReq(?);
	}
	#endif
}


uint32_t nexus3d_state::crtc_vblank_r()
{
	uint16_t res = (m_screen->vblank()<<1) | (m_screen->hblank()<<0);

	return (res<<16);
}

void nexus3d_state::nexus3d_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("mainram");
	map(0x02000000, 0x023fffff).ram().share("fbram"); // boundary tbd

	map(0x03720000, 0x0373ffff).ram(); // 3d fifo, boundary tbd
	map(0x046c0000, 0x046fffff).ram(); // """

	map(0x60000000, 0x67ffffff).ram(); // color tables?

	// actually USB hubs (prints "USB STRAGE" if 0)
	map(0x8c000000, 0x8c000003).portr("IN0");
	map(0x8c800000, 0x8c800003).portr("IN1");
	map(0x8d000000, 0x8d000003).portr("IN2");

	// flash
	map(0x9C000000, 0x9C000003).r(m_nand, FUNC(nand_device::data_r));
	map(0x9C000010, 0x9C000013).w(m_nand, FUNC(nand_device::command_w));
	map(0x9C000018, 0x9C00001b).w(m_nand, FUNC(nand_device::address_w));

	// read on irq 9 service, unknown purpose
	map(0xc0000200, 0xc00002bf).nopr();

	// on irq, acknowledge happens to both 800 and 810 ports
	map(0xc0000800, 0xc0000803).nopw();
	map(0xc0000808, 0xc000080b).rw(FUNC(nexus3d_state::int_mask_r), FUNC(nexus3d_state::int_mask_w));
	map(0xc0000810, 0xc0000813).rw(FUNC(nexus3d_state::int_pending_r), FUNC(nexus3d_state::int_ack_w));
	map(0xc0000814, 0xc0000817).r(FUNC(nexus3d_state::int_level_r));

	// lots of accesses in this range
	// 0xc00018xx seems CRTC related
	// 0xc000091x loads a "gfx charset"?
	map(0xc0000910, 0xc0000913).w(FUNC(nexus3d_state::rop_data_w));
	map(0xc000091c, 0xc000091f).w(FUNC(nexus3d_state::rop_register_w)).umask32(0xffff0000);

	map(0xc0000d00, 0xc0000d03).rw(FUNC(nexus3d_state::timer_status_r), FUNC(nexus3d_state::timer_status_w));
	map(0xc0000d04, 0xc0000d07).rw(FUNC(nexus3d_state::timer_count_r), FUNC(nexus3d_state::timer_count_w));

	map(0xc0001844, 0xc0001847).r(FUNC(nexus3d_state::crtc_vblank_r));

//  map(0xc0000f40, 0xc0000f4f).ram();
//  map(0xC0000F44, 0xC0000F47) (nexus3d_unk2_r, nexus3d_unk2_w ) // often, status for something.
	map(0xc0000f4c, 0xc0000f4f).r(FUNC(nexus3d_state::rop_status_r)).umask32(0xffff0000);

	map(0xe0000014, 0xe0000017).r(FUNC(nexus3d_state::vrender3d_status_r));

//  map(0xe0000000, 0xe00000ff) General / Control registers
//  map(0xe0000300, 0xe00003ff) GTE constant vector registers

}

static INPUT_PORTS_START( nexus3d )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void nexus3d_state::machine_start()
{
	m_timer = timer_alloc(FUNC(nexus3d_state::timercb), this);

}

void nexus3d_state::machine_reset()
{
	// the first part of the flash ROM automatically gets copied to RAM
	memcpy(m_mainram, memregion("nand")->base(), 4 * 1024);
}

void nexus3d_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		// EXTINT1?
		//IntReq(9);
		IntReq(1);
	}
}

void nexus3d_state::nexus3d(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &nexus3d_state::nexus3d_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw((XTAL(14'318'181)*2), 454*2, 0, 640, 262*2, 0, 480); // not accurate, needs CRTC understanding
	m_screen->set_screen_update(FUNC(nexus3d_state::screen_update));
	m_screen->screen_vblank().set(FUNC(nexus3d_state::screen_vblank));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_565);

	SAMSUNG_K9F2G08U0M(config, m_nand, 0);
}



ROM_START( acheart )
	ROM_REGION( 0x10800898, "nand", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheart.u1",     0x000000, 0x10800898, CRC(109bf439) SHA1(33fd39355923ef384d5eaeec8ae3f296509bde93) )

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//  ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END


ROM_START( acheartf )
	ROM_REGION( 0x10800898, "nand", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "arcanaheartfull.u1",     0x000000, 0x10800898, CRC(54b57a9d) SHA1(dee5a43b3aea854d2b98869dca74c57b66fb06eb))

	ROM_REGION( 0x200000, "user2", 0 ) // QDSP stuff
	ROM_LOAD( "u38.bin",     0x000000, 0x200000, CRC(29ecfba3) SHA1(ab02c7a579a3c05a19b79e42342fd5ed84c7b046) )
	ROM_LOAD( "u39.bin",     0x000000, 0x200000, CRC(eef0b1ee) SHA1(5508e6b2f0ae1555662793313a05e94a87599890) )
	ROM_LOAD( "u44.bin",     0x000000, 0x200000, CRC(b9723bdf) SHA1(769090ada7375ecb3d0bc10e89fe74a8e89129f2) )
	ROM_LOAD( "u45.bin",     0x000000, 0x200000, CRC(1c6a3169) SHA1(34a2ca00a403dc1e3909ed1c55320cf2bbd9d49e) )
	ROM_LOAD( "u46.bin",     0x000000, 0x200000, CRC(1e8a7e73) SHA1(3270bc359b266e57debf8fd4283a46e08d679ae2) )

	ROM_REGION( 0x080000, "wavetable", ROMREGION_ERASEFF ) /* QDSP wavetable rom */
//  ROM_LOAD( "qs1001a",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) ) // missing from this set, but should be the same
ROM_END

void nexus3d_state::init_acheart()
{
	// 0x1230 BL <vector>
	// 0x1234 CMPS, #$0 <- if R0 = 1 then stops at 0x1244
}

void nexus3d_state::init_acheartf()
{
	// 0x1290 BL <vector>
	// 0x1294 CMPS, #$0 <- if R0 = 1 then stops at 0x1244

	// patch additional unknown check after $c0000a00
	// 0x107c serial?
}

} // anonymous namespace


GAME( 2005, acheart,  0, nexus3d, nexus3d, nexus3d_state, init_acheart,  ROT0, "Examu", "Arcana Heart",      MACHINE_IS_SKELETON )
GAME( 2006, acheartf, 0, nexus3d, nexus3d, nexus3d_state, init_acheartf, ROT0, "Examu", "Arcana Heart Full", MACHINE_IS_SKELETON )
