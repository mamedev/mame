// license:BSD-3-Clause
// copyright-holders:

/*
Subsino H8-based kiddie ride hardware

CS186P190 0608 PCB with CS186P409 0648 riser PCB

Main components of the main PCB:
SUBSINO SS9689 6433044A22F CPU
32.000 MHz XTAL
MX27C2000PC-70 program ROM (bad, reads all 0xff)
4x W24257AK-15 SRAM
2x Subsino SG001 S0550 JPAX3 customs (GFX?)
4x 27C801 or equivalent GFX ROMs
4x 27C401 or equivalent GFX ROMs
Subsino SS9802-6 0616 custom (I/O)
Subsino SS9804 0420 custom (sound)
MX29F1610MC-12 sound ROM (bad, won't read)
MX29F1610MC-10 sound ROM (bad, won't read)
S-1 (some kind of audio DAC, according to other driver)

bank of 8 switches (DS1)
push-button (DS1)

The riser board has a Subsino SG003 custom (RAMDAC, according to other driver)
and a good number of capacitors and resistors.
*/


#include "emu.h"

#include "subsino_io.h"

#include "cpu/h8/h83048.h"
#include "machine/ds2430a.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define LOG_BLIT  (1U << 1) // tailored for stdout

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


class subsino_sg001_device : public device_t
{
public:
	subsino_sg001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	// TODO: convert to address_map once we have a better grasp of this
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_blit_data[0x20];
	void blitter_exec();
};


DEFINE_DEVICE_TYPE(SUBSINO_SG001, subsino_sg001_device, "subsino_sg001", "Subsino SG001 custom video chip")

subsino_sg001_device::subsino_sg001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SUBSINO_SG001, tag, owner, clock)
{
}

void subsino_sg001_device::device_start()
{
	save_item(NAME(m_blit_data));
}

void subsino_sg001_device::device_reset()
{
}

u8 subsino_sg001_device::read(offs_t offset)
{
	// bit 7: blitter busy
	if (offset == 0x10)
		return 0;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_BLIT, "Warning: blitter read at %06x\n", offset + 0xc0);

	return m_blit_data[offset];
}

void subsino_sg001_device::write(offs_t offset, u8 data)
{
	const u8 prev = m_blit_data[offset];
	m_blit_data[offset] = data;
	// -x-- ---- 0 -> 1 blitter trigger
	// ---- ---x bpp or GFX bank select? $a000xx = 1 on first execs then 0, $4000xx = 1
	if (offset == 0x1d && !BIT(prev, 6) && BIT(data, 6))
	{
		LOGMASKED(LOG_BLIT, "%s blitter exec: %02x\n", this->tag(), data);
		blitter_exec();
	}
}

// writes to $dc then $c0-$c8, $d0-$d4/$d9 (*) finally to trigger at $dd
// (*) range depends on trigger bit 0
void subsino_sg001_device::blitter_exec()
{
	int i;
	LOGMASKED(LOG_BLIT, "\t$dc: %02x\n", m_blit_data[0x1c]);
	LOGMASKED(LOG_BLIT, "\t$c0: ");
	for (i = 0; i < 9; i++)
		LOGMASKED(LOG_BLIT, "%02x ", m_blit_data[0x00 + i]);
	LOGMASKED(LOG_BLIT, "\n\t$d0: ");
	for (i = 0; i < 0xa; i++)
		LOGMASKED(LOG_BLIT, "%02x ", m_blit_data[0x10 + i]);
	LOGMASKED(LOG_BLIT, "\n");
}


namespace {

class subsino_kr_h8_state : public driver_device
{
public:
	subsino_kr_h8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sg001(*this, "sg001_%u", 0U),
		m_eeprom(*this, "eeprom")
	{ }

	void modcart(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<subsino_sg001_device, 2> m_sg001;
	required_device<ds2430a_device> m_eeprom;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void out_a_w(u8 data);
	void out_b_w(u8 data);
	void out_c_w(u8 data);
	void out_d_w(u8 data);

	void program_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void subsino_kr_h8_state::video_start()
{
}

uint32_t subsino_kr_h8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	return 0;
}


void subsino_kr_h8_state::out_a_w(u8 data)
{
	logerror("%s output A write: %02x\n", machine().describe_context(), data);
}

void subsino_kr_h8_state::out_b_w(u8 data)
{
	logerror("%s output B write: %02x\n", machine().describe_context(), data);
}

void subsino_kr_h8_state::out_c_w(u8 data)
{
	logerror("%s output C write: %02x\n", machine().describe_context(), data);
}

void subsino_kr_h8_state::out_d_w(u8 data)
{
	// TODO: bit 3 = unknown output
	m_eeprom->data_w(!BIT(data, 6));
}


void subsino_kr_h8_state::program_map(address_map &map)
{
	map(0x000000, 0x007fff).rom();
	map(0x080000, 0x0bffff).rom();
	map(0x200000, 0x207fff).ram();
//	map(0x400000, 0x40007f).ram(); // SG001 commands?
	map(0x400094, 0x400094).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x400095, 0x400095).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x400096, 0x400096).w("ramdac", FUNC(ramdac_device::mask_w));
	// I/O? vblank? (res & 0xe0) == 0xc0 before setting palette
	map(0x40009e, 0x40009e).lr8(NAME([] () { return 0xc0; }));
	map(0x4000c0, 0x4000df).rw(m_sg001[0], FUNC(subsino_sg001_device::read), FUNC(subsino_sg001_device::write));
	map(0x600000, 0x60001f).rw("io", FUNC(ss9802_device::read), FUNC(ss9802_device::write));
	// map(0x600060, 0x600061);
	// map(0x800000, 0x800001);
//	map(0xa00000, 0xa000ff).ram(); // second SG001 commands?
	map(0xa000c0, 0xa000df).rw(m_sg001[1], FUNC(subsino_sg001_device::read), FUNC(subsino_sg001_device::write));
}

void subsino_kr_h8_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

static INPUT_PORTS_START( modcart )
	// bits 0, 4, 5, 7 used during current state loop
	PORT_START("IN-A")
	PORT_DIPNAME( 0x01, 0x01, "IN-A0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN-A1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN-A2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN-A3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN-A4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN-A5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN-A6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN-A7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-B")
	PORT_DIPNAME( 0x01, 0x01, "IN-B0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN-B1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN-B2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN-B3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN-B4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN-B5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN-B6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN-B7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-C")
	PORT_DIPNAME( 0x01, 0x01, "IN-C0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN-C1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN-C2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN-C3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN-C4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN-C5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN-C6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN-C7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN-D")
	PORT_DIPNAME( 0x01, 0x01, "IN-D0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN-D1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN-D2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // output
	PORT_DIPNAME( 0x10, 0x10, "IN-D4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN-D5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // output
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(ds2430a_device::data_r))

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// Variable tile sizes like subsino2.cpp
// End portions of both banks have tables + other undecoded stuff, more bpp select?
static GFXDECODE_START( gfx_modcart )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x8_raw, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x8_raw, 0, 1 )
GFXDECODE_END


void subsino_kr_h8_state::modcart(machine_config &config)
{
	H83044(config, m_maincpu, 32_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &subsino_kr_h8_state::program_map);

	ss9802_device &io(SS9802(config, "io"));
	io.in_port_callback<0>().set([this] () { logerror("%s io port 0 read\n", machine().describe_context()); return 0xff; });
	io.out_port_callback<0>().set([this] (uint8_t data) { logerror("%s io port 0 write: %02x\n", machine().describe_context(), data); });
	io.in_port_callback<1>().set_ioport("DSW"); // maybe
	io.out_port_callback<2>().set([this] (uint8_t data) { logerror("%s io port 1 write: %02x\n", machine().describe_context(), data); });
	io.in_port_callback<3>().set_ioport("IN-C"); // maybe
	io.in_port_callback<4>().set_ioport("IN-B"); // maybe
	io.in_port_callback<5>().set_ioport("IN-A"); // maybe
	io.in_port_callback<6>().set_ioport("IN-D"); // maybe
	io.out_port_callback<6>().set(FUNC(subsino_kr_h8_state::out_d_w));
	io.out_port_callback<7>().set(FUNC(subsino_kr_h8_state::out_c_w));
	io.out_port_callback<8>().set(FUNC(subsino_kr_h8_state::out_b_w));
	io.out_port_callback<9>().set(FUNC(subsino_kr_h8_state::out_a_w));

	DS2430A(config, m_eeprom).set_timing_scale(0.32);

	SUBSINO_SG001(config, m_sg001[0], 0);
	SUBSINO_SG001(config, m_sg001[1], 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify once it works
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-16-1);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(subsino_kr_h8_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, 0);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_modcart);
	PALETTE(config, "palette").set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", "palette"));
	ramdac.set_addrmap(0, &subsino_kr_h8_state::ramdac_map);

	SPEAKER(config, "mono").front_center();

	// SS9804
}


ROM_START( modcart )
	ROM_REGION( 0x180000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "ss9689_6433044a22f.u31",   0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) ) // wasn't dumped for this set, but part number matches
	ROM_LOAD( "modern_cart_std_v112.u35", 0x080000, 0x100000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "modern_cart_v100.u26", 0x000000, 0x80000, CRC(f2910fbf) SHA1(dd06fc160a09333a15f2d753ac0c4ef267a422ee) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u27", 0x000001, 0x80000, CRC(2aacf7e3) SHA1(6edf3d7e047aa02e6438dd33c5e44b94b29510dd) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u28", 0x000002, 0x80000, CRC(1d24fb35) SHA1(b063e9f2c84d6904bc7633d75e7dfcc881923fac) )
	ROM_LOAD32_BYTE( "modern_cart_v100.u29", 0x000003, 0x80000, CRC(6fa9f574) SHA1(68afac743b72be30b63eda53cf917c8f2568381a) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "u22", 0x000000, 0x100000, CRC(af7cd652) SHA1(f0532fdb75296e6c03b52d0e35331a3b930ca490) )
	ROM_LOAD32_BYTE( "u23", 0x000001, 0x100000, CRC(c69f7f90) SHA1(a594bf299fd94cf3be61b9c7af163d9d9eb15da3) )
	ROM_LOAD32_BYTE( "u24", 0x000002, 0x100000, CRC(174934ec) SHA1(ca55199e5071ffc9282cea1c7fb6cb4b244ddaf9) BAD_DUMP ) // FIXED BITS (xxxxxx1x)
	ROM_LOAD32_BYTE( "u25", 0x000003, 0x100000, CRC(64560184) SHA1(85f4a5b328f19d3630159c82f8a8af0826622afc) BAD_DUMP ) // FIXED BITS (xxx1xxxx)

	ROM_REGION( 0x400000, "ss9804", ROMREGION_ERASE00 )
	ROM_LOAD( "u18", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "u19", 0x200000, 0x200000, NO_DUMP )
ROM_END

ROM_START( coolbi )
	ROM_REGION( 0xc0000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "ss9689_6433044a22f.u31",    0x00000, 0x08000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) ) // wasn't dumped for this set, but part number matches
	ROM_LOAD( "cool_bi std u35 v1.06.u35", 0x80000, 0x40000, CRC(51e66960) SHA1(faa754e1ca7e44b902de4093b47982cb717f89a3) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "cool_bi std u26 v100.u26", 0x000000, 0x80000, CRC(f0fead44) SHA1(9a37762f03bea0f87bf82018b01a4cc7d57be644) )
	ROM_LOAD32_BYTE( "cool_bi std u27 v100.u27", 0x000001, 0x80000, CRC(f334a774) SHA1(d4c82d6163fe9c99a7112dc848b8950422ae6bdf) )
	ROM_LOAD32_BYTE( "cool_bi std u28 v100.u28", 0x000002, 0x80000, CRC(0f70527c) SHA1(df980ebe8913c49e2f1b9978f65333217124c1bf) )
	ROM_LOAD32_BYTE( "cool_bi std u29 v100.u29", 0x000003, 0x80000, CRC(9b974d8d) SHA1(3d4e4ce6d99019a5aff7bd34afd9ea7c2e9ba3f8) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "cool_bi std u22 v100.u22", 0x000000, 0x100000, CRC(61f20b6e) SHA1(9ae69553c8b450ef72825b8955d6f547cb73d65a) )
	ROM_LOAD32_BYTE( "cool_bi std u23 v100.u23", 0x000001, 0x100000, CRC(b25aee87) SHA1(7e86a4cd7957fc2309854a52352d23e9798f9f58) )
	ROM_LOAD32_BYTE( "cool_bi std u24 v100.u24", 0x000002, 0x100000, CRC(07bb7895) SHA1(cc1bbf5a99e48fcf13546fa90384039cbd8fe6af) )
	ROM_LOAD32_BYTE( "cool_bi std u25 v100.u25", 0x000003, 0x100000, CRC(0cd41da8) SHA1(cba678d0be79da7c7079fe9cbfd929a78a5b946b) )

	ROM_REGION( 0x400000, "ss9804", ROMREGION_ERASE00 )
	ROM_LOAD( "u18", 0x000000, 0x200000, NO_DUMP )
	ROM_LOAD( "u19", 0x200000, 0x200000, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 2002, modcart, 0, modcart, modcart, subsino_kr_h8_state, empty_init, ROT0, "Subsino", "Modern Cart",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 200?, coolbi,  0, modcart, modcart, subsino_kr_h8_state, empty_init, ROT0, "Subsino", "Cool-Bi Cool-Bi", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
