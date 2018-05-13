// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    krz2000.cpp - Kurzweil K2000 series

    Skeleton driver by R. Belmont

    Hardware in brief:
        TMP68301 CPU @ 16 MHz
        uPD72064 FDC
        85C30 SCSI
        M37450 on I/O board to handle panel/display/keyboard scanning
        HD6303 on I/O board to manage reverb DSP program loading

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "machine/upd765.h"
#include "machine/ncr5380n.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class k2000_state : public driver_device
{
public:
	k2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void k2000(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void k2000_map(address_map &map);

private:
	required_device<cpu_device> m_maincpu;
};

void k2000_state::machine_start()
{
}

void k2000_state::machine_reset()
{
}

void k2000_state::k2000_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x100000, 0x11ffff).ram();
	map(0xfffc00, 0xffffff).rw("tmp68301", FUNC(tmp68301_device::regs_r), FUNC(tmp68301_device::regs_w));  // TMP68301 Registers
}

MACHINE_CONFIG_START(k2000_state::k2000)
	MCFG_DEVICE_ADD("maincpu", M68301, XTAL(12'000'000))
	MCFG_DEVICE_PROGRAM_MAP(k2000_map)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("tmp68301",tmp68301_device,irq_callback)

	MCFG_DEVICE_ADD("tmp68301", TMP68301, 0)
	MCFG_TMP68301_CPU("maincpu")

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
MACHINE_CONFIG_END

static INPUT_PORTS_START( k2000 )
INPUT_PORTS_END

ROM_START( k2000 )
	ROM_REGION(0x140000, "maincpu", 0)
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_hi__v2.0j_3b69__=c=_1993_yca.tms27c040.u6", 0x000000, 0x080000, CRC(35c17fc3) SHA1(b91deec0127669b46af05a2acaa212e29e49abfb) )
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_lo__v2.0j_0db0__=c=_1993_yca.tms27c040.u3", 0x000001, 0x080000, CRC(11c7f436) SHA1(c2afe84b58d71932f223097ea01812eb513bd740) )
	ROM_LOAD16_BYTE( "k2j-k2rj_su_hi__v12ts_5e89__=c=_1993_yca.m27c1001.u5", 0x100000, 0x020000, CRC(16e0bdb7) SHA1(962fa10896f6a95210d752be28f02640869893a4) )
	ROM_LOAD16_BYTE( "k2j-k2rj_su_lo__v12ts_2f52__=c=_1993_yca.m27c1001.u2", 0x100001, 0x020000, CRC(cb11e837) SHA1(bcdf3d5abe8c53727a142008acb2755ed0ecc6ea) )

	ROM_REGION(0x800000, "pcm", 0)
	ROM_LOAD16_BYTE( "k2m1h_25da__830106-01__=c=1991_yca__japan_9322_d.ide6ed.u38", 0x000001, 0x100000, CRC(f110b0e7) SHA1(d8731b74b1ca6761f8fd3f6360bfe2f1cc3077bc) )
	ROM_LOAD16_BYTE( "k2m1l_20fe__830107-01__=c=1991_yca__japan_9324_d.idb9a4.u43", 0x000000, 0x100000, CRC(00715fbe) SHA1(99f661096031b794de216c74ce9b780e9889d344) )
	ROM_LOAD16_BYTE( "k2m2h_de34__830108-01__=c=1991_yca__japan_9324_d.id0000.u39", 0x200001, 0x100000, CRC(99aae00e) SHA1(7045fb6b19b046f3f068a3581b6498ee62603fb4) )
	ROM_LOAD16_BYTE( "k2m2l_1bdf__830109-01__=c=1991_yca__japan_9324_d.id836c.u44", 0x200000, 0x100000, CRC(b2acd497) SHA1(24d3e84016fa08a990ce4c39294ad47fb0cab3d0) )
	ROM_LOAD16_BYTE( "k2m3h_0e87__830110-01__=c=1991_yca__japan_9324_d.iddbfa.u40", 0x400001, 0x100000, CRC(f448694f) SHA1(484593d072c43fe442cd8cc6cc40cd24677b35cc) )
	ROM_LOAD16_BYTE( "k2m3l_3cde__830111-01__=c=1991_yca__japan_9329_d.ide8b5.u46", 0x400000, 0x100000, CRC(be8408f9) SHA1(fbeab2d690532d055d424be52d937e2729b3daac) )
	ROM_LOAD16_BYTE( "k2m4h_3f2d__830112-01__=c=1991_yca__japan_9324_d.iddbdb.u42", 0x600001, 0x100000, CRC(da8666f5) SHA1(4d0f306cad9a3a96cf1232b9b8df12fae044a1d6) )
	ROM_LOAD16_BYTE( "k2m4l_2e6d__830113-01__=c=1991_yca__japan_9327_d.id3199.u47", 0x600000, 0x100000, CRC(6eb73185) SHA1(fe48fe44be90a856251974750b1eac7f5291e1e6) )

	ROM_REGION(0x3000, "pals", 0)
	ROM_LOAD( "pseudo_v4d.u11.gal16v8b.jed", 0x000000, 0x000bd0, CRC(43561132) SHA1(a0c567c81022bc7fb83023d89556ccd5aa1ab36d) )
	ROM_LOAD( "sndram_v1.u50.gal16v8b.jed", 0x001000, 0x000bd0, CRC(cabc9335) SHA1(968fa5baa43c7589c901f09b12085437834aeb37) )
	ROM_LOAD( "godot_v5.u10.gal20v8a.jed", 0x002000, 0x00066f, CRC(c6517456) SHA1(b82530d46afdca5f6460e77ac11710cad55a6b89) )
ROM_END

CONS( 1990, k2000, 0, 0, k2000, k2000, k2000_state, empty_init, "Kurzweil Music Systems", "K2000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )

