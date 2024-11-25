// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for AT&T Model 3B2 computer.

****************************************************************************/

#include "emu.h"
#include "cpu/we32000/we32100.h"
#include "machine/am9517a.h"
#include "machine/mc68681.h"
#include "machine/pit8253.h"
//#include "machine/upd7261.h"
#include "machine/wd_fdc.h"


namespace {

class att3b2_state : public driver_device
{
public:
	att3b2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void att3b2v2(machine_config &config);
	void att3b2v3(machine_config &config);

private:
	void mem_map_300(address_map &map) ATTR_COLD;
	void mem_map_600(address_map &map) ATTR_COLD;

	required_device<we32100_device> m_maincpu;
};


void att3b2_state::mem_map_300(address_map &map)
{
	map.global_mask(0x07ffffff); // 27-bit physical addresses
	map(0x00000000, 0x00007fff).rom().region("bootstrap", 0);
	map(0x00042000, 0x0004200f).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0x000000ff);
	//map(0x00042013, 0x00042013).r(FUNC(att3b2_state::clear_csr6_r));
	//map(0x00043000, 0x00043fff).rw(FUNC(att3b2_state::nvram_nibble_r), FUNC(att3b2_state::nvram_nibble_w)).umask32(0x00ff00ff);
	//map(0x00044000, 0x0004403f).w(FUNC(att3b2_state::csr_w)).umask32(0x000000ff);
	//map(0x00044002, 0x00044003).r(FUNC(att3b2_state::csr_r));
	//map(0x00045003, 0x00045003).w(FUNC(att3b2_state::hard_disk_page_w));
	map(0x00048000, 0x0004800f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x00049000, 0x0004900f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	//map(0x0004a000, 0x0004a001).rw("hdc", FUNC(upd7261_device::read), FUNC(upd7261_device::write));
	//map(0x0004c003, 0x0004c003).r(FUNC(att3b2_state::dpdram_size_r));
	map(0x0004d000, 0x0004d003).rw("fdc", FUNC(wd2797_device::read), FUNC(wd2797_device::write));
	map(0x02000000, 0x0203ffff).ram();
}

void att3b2_state::mem_map_600(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().region("bootstrap", 0);
	//map(0x00040003, 0x00040003).w(FUNC(att3b2_state::floppy_control_w));
	map(0x00041000, 0x0004100f).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0x000000ff);
	//map(0x00042000, 0x00043fff).rw(FUNC(att3b2_state::nvram_byte_r), FUNC(att3b2_state::nvram_byte_w)).umask32(0x00ff00ff);
	//map(0x00045002, 0x00045003).w(FUNC(att3b2_state::floppy_page_12bit_w));
	map(0x00048000, 0x0004800f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x00049000, 0x0004900f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x0004a000, 0x0004a003).rw("fdc", FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0x02000000, 0x0203ffff).mirror(0x1000000).ram();
}


static INPUT_PORTS_START(att3b2)
INPUT_PORTS_END

void att3b2_state::att3b2v2(machine_config &config)
{
	WE32100(config, m_maincpu, 10_MHz_XTAL); // special WE32102 XTAL runs at 1x or 2x speed
	m_maincpu->set_addrmap(AS_PROGRAM, &att3b2_state::mem_map_300);

	PIT8253(config, "pit"); // D8253C-5; unknown clocks

	AM9517A(config, "dmac", 5'000'000); // AM9517A-5DC; unknown clock

	SCN2681(config, "duart", 3'686'400); // MC2681P

	// TODO: hard disk controller (NEC D7261AD)

	WD2797(config, "fdc", 1'000'000); // TMS2797NL

	// TODO: RTC (MM58174AN)
}

void att3b2_state::att3b2v3(machine_config &config)
{
	att3b2v2(config);
	m_maincpu->set_clock(18'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &att3b2_state::mem_map_600);

	PIT8254(config.replace(), "pit"); // Intel 82C54

	FD1793(config.replace(), "fdc", 1'000'000); // FD 1793-02
}

ROM_START(3b2_300)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("3b2300-c.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("3b2300-d.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("3b2300-e.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("3b2300-f.bin", 0x0003, 0x2000, CRC(b8e138c4) SHA1(d2da4a7150150d0f9294814edb7ed357f9341858))
ROM_END

ROM_START(3b2_310)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("aayyc.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("aayyd.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("aayye.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("aayyf.bin", 0x0003, 0x2000, CRC(39dcfd8c) SHA1(2e662b3811f78ab689bc2687b73e3158f33f7f89))
ROM_END

ROM_START(3b2_400)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("3b2-aayyc.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("3b2-aayyd.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("3b2-aayye.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("3b2-aayyf-4.bin", 0x0003, 0x2000, CRC(85b8c5d3) SHA1(85bdf3f889f6c14cbf33ce81421f1f1d02328223))
ROM_END

ROM_START(3b2_600)
	ROM_REGION32_BE(0x20000, "bootstrap", 0)
	ROM_LOAD32_BYTE("abtry.bin", 0x0000, 0x8000, CRC(fa8d488b) SHA1(2e169f4171bd30aba1a9cd550a418c943eb78ceb))
	ROM_LOAD32_BYTE("abtrw.bin", 0x0001, 0x8000, CRC(8705cb68) SHA1(f41365cd0b4f90d8ad0335655b0ac04a7d14d6c6))
	ROM_LOAD32_BYTE("abtru.bin", 0x0002, 0x8000, CRC(ea9e127b) SHA1(6618998ead5a5e07ead8c572619a6bcf71d84497))
	ROM_LOAD32_BYTE("abtrt.bin", 0x0003, 0x8000, CRC(0f075161) SHA1(b67c9c4549dc789df33b5a38e4b35fe26fdfbea6))
ROM_END

} // anonymous namespace


COMP(1984, 3b2_300, 0,       0, att3b2v2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/300", MACHINE_IS_SKELETON)
COMP(1985, 3b2_310, 3b2_300, 0, att3b2v2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/310", MACHINE_IS_SKELETON)
COMP(1985, 3b2_400, 3b2_300, 0, att3b2v2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/400", MACHINE_IS_SKELETON)
COMP(1987, 3b2_600, 0,       0, att3b2v3, att3b2, att3b2_state, empty_init, "AT&T", "3B2/600", MACHINE_IS_SKELETON)
