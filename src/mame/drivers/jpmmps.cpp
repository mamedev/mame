// license:BSD-3-Clause
// copyright-holders:David Haywood
/* JPM MPS1/2 Hardware

  TMS9995 CPU

  +

  4x 8255 - General I/O

  TMS9902A - CPU -> Reel MCU communication
  TMS9902A - Printer / Black Box comms

  TMS7041 - Reel MCU (4kb Internal ROM, Not Dumped)

  SN76489 - Sound

*/

/*
  Notes:
  MPS1- The hardware still uses TMS CPUs, but now with matrix driven lamps amongst other things. Later games used AY8910 as a sound chip instead of the SN76489.

  MPS2- Basically MPS1 manufactured on a more compact board, replacing discrete logic with 'custom cells' - which are rebadged 8255 PPIs. This is the better
  target to emulate, but some BwBs may struggle with the timing.

  this should be pretty easy to get going, my only concern is patches other drivers have to work around
  what appear to be i8255 issues?
*/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/sn76496.h"
#include "machine/i8255.h"
#include "machine/tms9902.h"
#include "machine/meters.h"
#include "jpmmps.lh"

class jpmmps_state : public driver_device
{
public:
	jpmmps_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_psg(*this, "sn"),
			m_meters(*this, "meters")
	{ }
	UINT8 m_sound_buffer;
	UINT8 m_psg_latch;
	virtual void machine_reset() override;

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<sn76489_device> m_psg;
	required_device<meters_device> m_meters;
public:
	DECLARE_WRITE8_MEMBER(jpmmps_meters_w);
	DECLARE_WRITE8_MEMBER(jpmmps_psg_buf_w);
	DECLARE_WRITE8_MEMBER(jpmmps_ic22_portc_w);
};

static ADDRESS_MAP_START( jpmmps_map, AS_PROGRAM, 8, jpmmps_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_ic26", i8255_device, read, write)
	AM_RANGE(0xc004, 0xc007) AM_DEVREADWRITE("ppi8255_ic21", i8255_device, read, write)
	AM_RANGE(0xc008, 0xc00b) AM_DEVREADWRITE("ppi8255_ic22", i8255_device, read, write)
	AM_RANGE(0xc00c, 0xc00f) AM_DEVREADWRITE("ppi8255_ic25", i8255_device, read, write)

	AM_RANGE(0xe800, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( jpmmps_io_map, AS_IO, 8, jpmmps_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE("tms9902_ic5", tms9902_device, cruread, cruwrite)

//  AM_RANGE(0x0020, 0x0020) // power fail
//  AM_RANGE(0x0021, 0x0021) // wd timeout
//  AM_RANGE(0x0022, 0x0022) // invalid access
//  AM_RANGE(0x0023, 0x0023) // clear down

//  AM_RANGE(0x0026, 0x0026) // uart4 int
//  AM_RANGE(0x0027, 0x0027) // uart2 int

	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE("tms9902_ic10", tms9902_device, cruread, cruwrite)

//  AM_RANGE(0x0060, 0x0060) // watchdog
//  AM_RANGE(0x0061, 0x0061) // ram en
//  AM_RANGE(0x0062, 0x0062) // alarm
//  AM_RANGE(0x0063, 0x0063) // nmi en
//  AM_RANGE(0x0064, 0x0064) // reel en
//  AM_RANGE(0x0065, 0x0065) // io en
//  AM_RANGE(0x0066, 0x0066) // bb
//  AM_RANGE(0x0067, 0x0067) // diagnostic led
ADDRESS_MAP_END


static INPUT_PORTS_START( jpmmps )
INPUT_PORTS_END

WRITE8_MEMBER(jpmmps_state::jpmmps_meters_w)
{
	for (int meter = 0; meter < 8; meter ++)
	{
		m_meters->update(meter, (data & (1 << meter)));
	}
}


WRITE8_MEMBER(jpmmps_state::jpmmps_psg_buf_w)
{
	m_sound_buffer = data;
}

WRITE8_MEMBER(jpmmps_state::jpmmps_ic22_portc_w)
{
	//Handle PSG

	if (m_psg_latch != (data & 0x04))
	{
		if (!m_psg_latch)//falling edge
		{
			m_psg->write(m_sound_buffer);
		}
	}
	m_psg_latch = (data & 0x04);

	m_meters->update(8, (data & 0x08));

}


// these are wrong
#define MAIN_CLOCK 2000000
#define SOUND_CLOCK 2000000
#define DUART_CLOCK 2000000


void jpmmps_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	static_cast<tms9995_device*>(machine().device("maincpu"))->ready_line(ASSERT_LINE);
}

static MACHINE_CONFIG_START( jpmmps, jpmmps_state )

	// CPU TMS9995, standard variant; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9995, MAIN_CLOCK, jpmmps_map, jpmmps_io_map)

	MCFG_DEVICE_ADD("ppi8255_ic26", I8255, 0)
	// Port B 0 is coin lockout
	MCFG_I8255_OUT_PORTC_CB(WRITE8(jpmmps_state, jpmmps_meters_w))

	MCFG_DEVICE_ADD("ppi8255_ic21", I8255, 0)

	MCFG_DEVICE_ADD("ppi8255_ic22", I8255, 0)
	MCFG_I8255_OUT_PORTB_CB(WRITE8(jpmmps_state, jpmmps_psg_buf_w)) // SN chip data
	MCFG_I8255_OUT_PORTC_CB(WRITE8(jpmmps_state, jpmmps_ic22_portc_w))  // C3 is last meter, C2 latches in data

	MCFG_DEVICE_ADD("ppi8255_ic25", I8255, 0)

	MCFG_DEVICE_ADD("tms9902_ic10", TMS9902, DUART_CLOCK) // Communication with Reel MCU
	MCFG_DEVICE_ADD("tms9902_ic5", TMS9902, DUART_CLOCK) // Communication with Security / Printer

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn", SN76489, SOUND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(9) // TODO: meters.cpp sets a max of 8

	MCFG_DEFAULT_LAYOUT(layout_jpmmps)
MACHINE_CONFIG_END



ROM_START( j2adnote )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "adanote1.bin", 0x0000, 0x8000, CRC(a64352e7) SHA1(6b78abab21420b0e5c5f6e41c9d798d30d5942c5) )
	ROM_LOAD( "adanote2.bin", 0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnotea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "add_a_note_1_1.bin", 0x0000, 0x8000, CRC(0e0377ba) SHA1(30e280769975a69d988f34df863b4ebad192f538) )
	ROM_LOAD( "add_a_note_1_2.bin", 0x8000, 0x8000, CRC(922e6c8e) SHA1(8b0de3306e0c43e34df627ab1b28e1c84fe2eafc) )
ROM_END

ROM_START( j2adnoteb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 20p 1.1.bin", 0x0000, 0x8000, CRC(3f7365af) SHA1(734ed13c5943ff9b38ae061e3fb1b25e1cc9134c) )
	ROM_LOAD( "addanote 20p 1.2.bin", 0x8000, 0x8000, CRC(ecfac3f4) SHA1(21f410e05cd30d3d936bba72dee6fabd709b32d1) )
ROM_END

ROM_START( j2adnotec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1.bin", 0x0000, 0x8000, CRC(67ff2728) SHA1(08bde11dcf20eaecac9b83072769b04dab465117) )
	ROM_LOAD( "addanote 1.2.bin", 0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnoted )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1 a.bin", 0x0000, 0x8000, CRC(cf3a31ee) SHA1(0455a3e885a92a7a740a682c02c667dba69c8890) )
	ROM_LOAD( "addanote 1.2.bin",   0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnotee )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1 p.bin", 0x8000, 0x8000, CRC(54215e09) SHA1(0c2e7f80a7f67e014ea51e0b80397ee023b94288) )
	ROM_LOAD( "addanote 1.2.bin",   0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnotef )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1 s.bin", 0x8000, 0x8000, CRC(227de861) SHA1(b0a06193becd78fc24ff6175177a5f1a7d2a59c3) )
	ROM_LOAD( "addanote 1.2.bin",   0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnoteg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1 x.bin", 0x0000, 0x8000, CRC(0043d56a) SHA1(47ea7c4cf37f48f5753c62394529c45243a26859) )
	ROM_LOAD( "addanote 1.2.bin",   0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnoteh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 1.1 y.bin", 0x0000, 0x8000, CRC(a886c3ac) SHA1(60c5ecdbf036b32bdafcc0f0bc4e599d72d9187f) )
	ROM_LOAD( "addanote 1.2.bin",   0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END

ROM_START( j2adnotei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "addanote 74% fixed 1b1.bin", 0x0000, 0x8000, CRC(ecfab9ba) SHA1(ea273d82e11a62c847cbaaef58a7d8b24a834bca) )
	ROM_LOAD( "addanote 1.2.bin",           0x8000, 0x8000, CRC(e421d220) SHA1(566ddf237cdb36bca9d8b6b67596e1602850971d) )
ROM_END


ROM_START( j2bankch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bankchase_1_1.bin", 0x0000, 0x8000, CRC(5ccb3adc) SHA1(d0a0129474b0ba05ea979840b212f3854785b4b6) )//99035
	ROM_LOAD( "bankchase_1_2.bin", 0x8000, 0x8000, CRC(c6602f9c) SHA1(d529bc07754bdd1cd0761bc42d8bde167a08236d) )//98677

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bank_chase_2_1.bin", 0x0000, 0x8000, CRC(152719d6) SHA1(5fe974f824caf7e1c38810a9f914225a5f4de8ce) )
	ROM_LOAD( "bank_chase_2_2.bin", 0x8000, 0x8000, CRC(1ccb194a) SHA1(0a22a5195033e8d66026e264852bf3976c8dae3d) )

	ROM_LOAD( "bankchase 1.1 10p 4.80 ebd9.bin", 0x0000, 0x8000, CRC(84c23924) SHA1(cf901ac19b4e980495e757e1570918690f43c607) )
	ROM_LOAD( "bankchase 1.2 10p 4.80 csa7.bin", 0x8000, 0x8000, CRC(304e8979) SHA1(24459a4224f601ebb35d1b92e03245bd7293f23e) )

	ROM_LOAD( "bankchase 1.1 20p.bin", 0x0000, 0x8000, CRC(33aa388b) SHA1(1ba80be7523a9177a16dd5d0fb80a8dd0ea16b97) )
	ROM_LOAD( "bankchase 1.2 20p.bin", 0x8000, 0x8000, CRC(6414cea1) SHA1(e6c147ecb7197c66e47f4d2435e65a64d35867e2) )

	ROM_LOAD( "bankchase 1.1 a.bin", 0x0000, 0x8000, CRC(f40e2c1a) SHA1(be651adc06a03c63bb2c063da8a26947c21da5b9) )
	ROM_LOAD( "bankchase 1.1 p.bin", 0x0000, 0x8000, CRC(6f1543fd) SHA1(9a1abc0b3f0072214de86fc76ad61bb0615695a4) )
	ROM_LOAD( "bankchase 1.1 s.bin", 0x0000, 0x8000, CRC(1949f595) SHA1(f813a82ac85c1e8e8072ac2aeb18228e9baaa62e) )
	ROM_LOAD( "bankchase 1.1 x.bin", 0x0000, 0x8000, CRC(3b77c89e) SHA1(82f27e131cb4e24ab6b57c0be2110e936325c82f) )
	ROM_LOAD( "bankchase 1.1 y.bin", 0x0000, 0x8000, CRC(93b2de58) SHA1(19dd2cf5340893c6a4ceeee6a87cdee14453641f) )

	ROM_LOAD( "bankchase10p2.1", 0x0000, 0x8000, CRC(e2f0b079) SHA1(8476acdc9169d40c81774d6bf5e1e7d82ca236db) )
ROM_END

ROM_START( j2bankrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "braid_5p.p1", 0x0000, 0x2000, CRC(1a291b5f) SHA1(7d6ec4582a25ee0fa83d77ce474c09d6063c996a) )
	ROM_LOAD( "braid_5p.p2", 0x2000, 0x2000, CRC(ed99a442) SHA1(aa63bf07456154ebec1d25e82bf57531be982cfa) )
	ROM_LOAD( "braid_5p.p3", 0x4000, 0x2000, CRC(0fd26516) SHA1(8cd2c8865bf8c186bc5277a3a6245e9085ac5c68) )
ROM_END

ROM_START( j2bigbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb3-1.bin", 0x0000, 0x4000, CRC(ff994a86) SHA1(baf79a39c2315ccc107b21a889ec7a90c1e1380e) )//12394
	ROM_LOAD( "bb3-2.bin", 0x4000, 0x4000, CRC(17488088) SHA1(560ca909115fa16d196027b4517b4c32f963abdf) )//11554

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bigbankerb11548 3.1.bin", 0x0000, 0x4000, CRC(c28a0f9a) SHA1(33740a31d2688d39ed79deead9f26cc3b8f17253) )

	ROM_LOAD( "bigbankerb11593 3p1.bin", 0x0000, 0x4000, CRC(c8f291e0) SHA1(0f152c3e3797c04d50db417d4927c0671b6ae46c) )
	ROM_LOAD( "bigbankerb12330 3y1.bin", 0x0000, 0x4000, CRC(eb687672) SHA1(d50b5153787f6cbd6c4dfe2bcfccde9aba57a9bf) )
	ROM_LOAD( "bigbankerb12366 3x1.bin", 0x0000, 0x4000, CRC(d67b336e) SHA1(ddc97bab97f444cc914af1a0c997c17ad7635891) )
ROM_END



ROM_START( j2bigbox )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "big_box_2p_1_1.bin", 0x0000, 0x4000, CRC(8b8cfe1f) SHA1(e1469370a27e037d51a6e94e4194b939d736591b) )
	ROM_LOAD( "big_box_2p_1_2.bin", 0x4000, 0x4000, CRC(8a79d1c5) SHA1(ad83c47958b1cd51f24f08bd667df81a6935341f) )
ROM_END


ROM_START( j2bigbuk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bigbucks3.1.bin", 0x0000, 0x4000, CRC(72104a23) SHA1(3fe3dba8dc47ded1d18e1f2ceb7e55dd530aa56d) )
	ROM_LOAD( "bigbucks3.2.bin", 0x4000, 0x4000, CRC(850db95d) SHA1(1d1b5c37a275b68662f42dca07dedf844a66c059) )
	ROM_LOAD( "bigbucks3.3.bin", 0xc000, 0x4000, CRC(a6cac228) SHA1(a03788fa9f75ecdf8b13dbab0ba2a91931d7f250) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bigbucks3c1.bin", 0x0000, 0x4000, CRC(929cba2d) SHA1(9512d0937c8eccd4ef65f8dcb2ae3789779b398d) )
ROM_END



ROM_START( j2bigdl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bdl.bin", 0x0000, 0xc000, CRC(f28b8489) SHA1(33d3bddb43c78e70b250380f8e69c3017cd997d1) ) /* split */
ROM_END



ROM_START( j2blkchy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "blckch1.bin", 0x0000, 0x4000, CRC(78200ccf) SHA1(9ed71b591c0e8ad4e2f67d48df6dc39b303a795b) )
	ROM_LOAD( "blckch2.bin", 0x4000, 0x4000, CRC(d20f3b51) SHA1(c6d2ec6d2dc7b1d314fdfdad12de32596c30d2aa) )
ROM_END


ROM_START( j2cashbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_bonus_2_1.bin", 0x0000, 0x8000, CRC(6f2bb994) SHA1(b7b05d8c6a74012d85fa8b4a479b91ad04dffba1) )
	ROM_LOAD( "cash_bonus_2_2.bin", 0x8000, 0x8000, CRC(0d7ba071) SHA1(11ca1d995f73999bcc7d154f4d1870be7373aa85) )
ROM_END


ROM_START( j2cashfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashfalls b10602 3.1.bin", 0x0000, 0x4000, CRC(ec3c3123) SHA1(79eca1f8d5d99ba3532296681409057ffe9da244) )
	ROM_LOAD( "cashfalls b10689 3.2.bin", 0x4000, 0x4000, CRC(2d1d1864) SHA1(45d24822e64d10f069fb470fdc20dbc17e7a2e17) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashfalls b10759 3p1.bin", 0x0000, 0x4000, CRC(e644af59) SHA1(778bd16a4af27afd4620534af76658b352faa55e) ) //also b10764
	ROM_LOAD( "cashfalls b11043 3y1.bin", 0x0000, 0x4000, CRC(c5de48cb) SHA1(f4536521a87f1adce19db4d74a85c14605fbe263) )
	ROM_LOAD( "cashfalls b113-- 3a1.bin", 0x0000, 0x4000, CRC(d12f743f) SHA1(6d30dbf29258a446ec756da4e0a0d145807da8d6) )
	ROM_LOAD( "cashfalls deluxe b12977 3x1.bin", 0x0000, 0x4000, CRC(f8cd0dd7) SHA1(3202f12019b4a511b92382a09621799fcb7659ec) )
ROM_END


ROM_START( j2cashrl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashreel a79069 1.1.bin", 0x0000, 0x4000, CRC(6a62ddd1) SHA1(3d28fe186fcf2ba7a0fd178372591cbb32cda2a3) )
	ROM_LOAD( "cashreel a79679 1.2.bin", 0x4000, 0x4000, CRC(7bd6b009) SHA1(62fa6a110ca4d54c7ed0fc0e8e0fa1ac920f09b6) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashreel b16361 1y1.bin", 0x0000, 0x4000, CRC(4380a439) SHA1(c18e2262f7f267fb1c256086d9febc684d929623) )
	ROM_LOAD( "cashreel b16365 1x1.bin", 0x0000, 0x4000, CRC(7e93e125) SHA1(2e22c0a4040a4309522f03559f6eafc161f3944f) )
	ROM_LOAD( "cashreel b16372 1a1.bin", 0x0000, 0x4000, CRC(577198cd) SHA1(05d2bb38d0225df5cd734bdf76673407317ab853) )
ROM_END



ROM_START( j2cashrv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cr1", 0x0000, 0x2000, CRC(66064d25) SHA1(516104d2ec0cfda956a1f4499647f5f1a233046c) )
	ROM_LOAD( "cr2", 0x2000, 0x2000, CRC(9f333cf6) SHA1(083a2f9a188f28e9414cd385c3f5abe517523100) )
	ROM_LOAD( "cr3", 0x4000, 0x2000, CRC(2e606b0d) SHA1(144ca5fff497fe59823744138d44f264ef20fd0f) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashreserve2-1.bin", 0x0000, 0x2000, CRC(1c16901d) SHA1(bf3062d3432d77228dc6d9f6aa0d6929d9020547) )
ROM_END


ROM_START( j2cashro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_rolls_1_1.bin", 0x0000, 0x4000, CRC(574c1cb2) SHA1(2ea5f14f52563df6b1451f5b2a52facceb078c72) )
	ROM_LOAD( "cash_rolls_1_2.bin", 0x4000, 0x4000, CRC(60a402e8) SHA1(bf46c578c1b3d5bdfdf40449b455e09f3891d7b9) )
ROM_END


ROM_START( j2cashtk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashtrack b16090 1x1.bin", 0x0000, 0x4000, CRC(166434ac) SHA1(38e61816c2c0f793c25228dc321d1922782ef047) )
	/* missing rom 2? */

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashtrack b16122 1a1.bin", 0x0000, 0x4000, CRC(3f864d44) SHA1(c0a52444849ee03570ee056f54ef61da0466b2f8) )
	ROM_LOAD( "cashtrack b16123 1y1.bin", 0x0000, 0x4000, CRC(2b7771b0) SHA1(268891bfb5a7454bb4fdc21965ef0565caab7480) )
ROM_END



ROM_START( j2cashtd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashtrack deluxe a 1.1.bin",      0x0000, 0x4000, CRC(02950858) SHA1(84809b06bb7d9d470b407df655e3d45342a542e2) )
	ROM_LOAD( "cashtrack deluxe b17560 1b2.bin", 0x4000, 0x4000, CRC(fffd95d1) SHA1(c80d131b4d93bdbbd7226859ddd356c432c92c7d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashtrack deluxe protocol 1p1.bin", 0x0000, 0x4000, CRC(08ed9622) SHA1(686701ad4db873ee90a042c2b39a4bc10cb9fcfd) )
ROM_END



ROM_START( j2coppot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "copperpot1_1.bin", 0x0000, 0x4000, CRC(15fcedd0) SHA1(12b2d02b8dcc1721150bbeb630d348fc7c56df1b) )
	ROM_LOAD( "copperpot1_2.bin", 0x4000, 0x4000, CRC(e4454282) SHA1(46914dbba6aec7e1a38c0013d8fe69e62992f42f) )
ROM_END



ROM_START( j2coprun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "copperrun2.1", 0x0000, 0x2000, CRC(49868bf2) SHA1(a0dc29be55b0dc2f1f43c962dc805590ea3ca248) )
	ROM_LOAD( "copperrun2.2", 0x2000, 0x2000, CRC(7a082c5f) SHA1(b1f421604f8e80d29f92301de9708a48091f3ab8) )
	ROM_LOAD( "copperrun2.3", 0x4000, 0x2000, CRC(d15e2276) SHA1(504b792f4ac508e3f9ef8add1adbb09d4c2d9afe) )
ROM_END

ROM_START( j2cprndx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crundeluxe3.1", 0x0000, 0x2000, CRC(379e296a) SHA1(3f9b7adcb10bad611a0bfa144ee270166688c957) )
	ROM_LOAD( "crundeluxe3.2", 0x2000, 0x2000, CRC(62eb4fa7) SHA1(0d8dc650acbf651f80124b3d847e4c11940e235b) )
	ROM_LOAD( "crundeluxe3.3", 0x4000, 0x2000, CRC(1c7446f1) SHA1(0065e1fa5018d615d736e40d221ccd2d6e980147) )
ROM_END

ROM_START( j2crkbnk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11nf1.bin", 0x0000, 0x4000, CRC(3b215ba7) SHA1(5b0d6a23dfdcfcc2bb0f769305ee468714fdf40f) )
	ROM_LOAD( "11nf2.bin", 0x4000, 0x4000, CRC(c7bda190) SHA1(d9c019aac107a564c67ef6a9d5ddb0dd7e0f0748) )
ROM_END



ROM_START( j2droplt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drplot71.bin", 0x0000, 0x2000, CRC(75cc1f17) SHA1(1e642d3b20cc680e25b789f8123223ca2b97bc2a) )
	ROM_LOAD( "drplot72.bin", 0x2000, 0x2000, CRC(97d42bb2) SHA1(4821a6ed60d7f5f62355eed7ee55d6a41789f1c8) )
	ROM_LOAD( "drplot73.bin", 0x4000, 0x2000, CRC(5a6fc22d) SHA1(c7803db19548f541d0b53d7c1e78a493f0cc0a4b) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "dropthelot1_3.bin", 0x0000, 0x2000, CRC(05ef5eb5) SHA1(a5f8efd3fe708cc7d3f030adcbcd863654f9b13e) )
ROM_END



ROM_START( j2dropld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dropthelotdeluxe1.1.bin", 0x0000, 0x2000, CRC(07f2d01d) SHA1(c6769dbfed2d7134819a8e0f08fbcf27d228ad88) )//c07116
	ROM_LOAD( "dropthelotdeluxe1.2.bin", 0x2000, 0x2000, CRC(cb6c9a46) SHA1(711cfd358fade847edb51ad686c6969eebda4109) )//c07284
	ROM_LOAD( "dropthelotdeluxe1.3.bin", 0x4000, 0x2000, CRC(3e2ce507) SHA1(d59c7dbbd469caf4976a6e8b7b7f3e2e9d35535a) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "dropthelotdeluxe b14526 1b1.bin", 0x0000, 0x2000, CRC(b3d72289) SHA1(bfc9dafb05577673f62a349ac3a10659b60fcc30) )
	ROM_LOAD( "dropthelotdeluxe b14542 1bp3.bin", 0x0000, 0x2000, CRC(b900b1d0) SHA1(e318224d5df62ec896624c1fb1c2353131bdcd69) )
	ROM_LOAD( "dropthelotdeluxe b14554 1x1.bin", 0x0000, 0x2000, CRC(09f5438a) SHA1(fa164034b780104cb13701e2c15208036820d88e) )
	ROM_LOAD( "dropthelotdeluxe b14561 1y1.bin", 0x0000, 0x2000, CRC(43198090) SHA1(7e058633901b7c3614a9b8ac29876ff6f31422f5) )
	ROM_LOAD( "dropthelotdeluxe b17547 1a1.bin", 0x0000, 0x2000, CRC(47e73da7) SHA1(5dc0ee91099f8dae3defd945fcb0262c2c467128) )
	ROM_LOAD( "dropthelotdeluxe b17557 1a2.bin", 0x0000, 0x2000, CRC(051ff3cd) SHA1(341abf5b9015089b632576ca8cfe388d81fe3ee7) )
ROM_END



ROM_START( j2ewn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn_5p_2_1.bin", 0x0000, 0x4000, CRC(05598dda) SHA1(2473dc3e909fc96225d5e263d1d541d6135fce8c) )
	ROM_LOAD( "ewn_5p_2_2.bin", 0x0000, 0x4000, CRC(228de93c) SHA1(fa5517914a3e5fbe45dba71548ad25a19f7783d3) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "ewn5p1.1", 0x0000, 0x2000, CRC(457835d8) SHA1(00029231017d9e2de2a3b08c7eccce67e941c02b) )
	ROM_LOAD( "ewn5p1.2", 0x0000, 0x2000, CRC(8d5a0823) SHA1(589e1e646b481c3cab340e022ced3c70adf76186) )
	ROM_LOAD( "ewn5p1.3", 0x0000, 0x2000, CRC(407297cb) SHA1(390b3f0cb0a3f235199ec269190ae64bd4816b9f) )
	ROM_LOAD( "ewn_alt_2p_2_1.bin", 0x0000, 0x2000, CRC(d1186cf0) SHA1(7e8078263d66286b1be7ff6135f088be04c53c3b) )
	ROM_LOAD( "ewn_alt_2p_2_2.bin", 0x0000, 0x2000, CRC(91ec9342) SHA1(8e0daefbb83c84dff3111f62b752fb6eb3b6511d) )
	ROM_LOAD( "ewn_alt_2p_2_3.bin", 0x0000, 0x2000, CRC(8f4033f6) SHA1(8f33c185b619e635f6b2070b32bb1d477cf5fa43) )
	ROM_LOAD( "ewn_2p_2_1.bin", 0x0000, 0x4000, CRC(1b6dd32f) SHA1(a78985ecd3858c98071228b0643f1290e056142d) )
	ROM_LOAD( "ewn_2p_2_2.bin", 0x0000, 0x4000, CRC(6b08d41f) SHA1(fc582458cd104450191ceea5085d44e1f8705391) )
	ROM_LOAD( "e_w_nudger_3_1.bin", 0x0000, 0x8000, CRC(b1fd3114) SHA1(e3906b3d93330b8e44cb21cbc76737edf5046aa7) )
	ROM_LOAD( "e_w_nudger_3_2.bin", 0x8000, 0x8000, CRC(ae617ae1) SHA1(093ddb44bea91b1d67821c756f6ac07789995d95) )
	ROM_LOAD( "eachwaynudgermk32.1.bin", 0x0000, 0x8000, CRC(8c7fcf54) SHA1(e1b92cc1860c101580a75405bdb302fd39a03c29) )
	ROM_LOAD( "eachwaynudgermk32.2.bin", 0x0000, 0x8000, CRC(8678d7ec) SHA1(f30ff44fe0cb5cc9fcd3cc2a9394ee16d830e1c4) )
	ROM_LOAD( "each_way_nudge_4_1.bin", 0x0000, 0x8000, CRC(a945ca5d) SHA1(6250a601f9ef237e6708df15b2cdd0eb81cc5c7e) )//88
	ROM_LOAD( "each_way_nudge_4_2.bin", 0x0000, 0x8000, CRC(3a83c908) SHA1(967a9bf830ea9601cc6418d5eca5e0c6f0e89faf) )//88
	ROM_LOAD( "ewn-1.bin", 0x0000, 0x2000, CRC(dc6dc4e4) SHA1(5f6bc674b8e7f9ef42ed8f0f5305130d3e779294) )
	ROM_LOAD( "ewn-2.bin", 0x0000, 0x2000, CRC(98ec4b3a) SHA1(db6c554336113254cbcf0122ff1a425e0fe970e2) )
ROM_END



ROM_START( j2ews )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews5p1.1", 0x0000, 0x4000, CRC(ad2f928f) SHA1(287c583e666bab4a7471179e431c6c536a4fd052) )
	ROM_LOAD( "ews5p1.2", 0x0000, 0x4000, CRC(bc098815) SHA1(4cf941ac8b23cdc2011e021f8e0eb4a7dcb73bf8) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "ews_5p_2_1.bin", 0x0000, 0x4000, CRC(84cba46e) SHA1(07d13002cd1c915f58f04e4eb989fd007856f2a7) )
	ROM_LOAD( "ews_5p_2_2.bin", 0x0000, 0x4000, CRC(057fb19b) SHA1(c5010d0bf7aba6dd6778a536b3cdcedd44d15eff) )
	ROM_LOAD( "ews_s1.bin", 0x0000, 0x4000, CRC(d435d674) SHA1(e7d9ea13906b86df15fbba408763f4169bb8ff80) )
	ROM_LOAD( "ews_s2.bin", 0x0000, 0x4000, CRC(965a3c43) SHA1(7f19681fad06fe935effc0d00055e26c15b58d97) )
ROM_END


ROM_START( j2exec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubexecutive2.1", 0x0000, 0x4000, CRC(bfb27cc9) SHA1(3a04e64d50dd04d11e5c66673ac4adf426aa195e) )//10p
	ROM_LOAD( "clubexecutive2.2", 0x4000, 0x4000, CRC(1f44cbc4) SHA1(fd029357ebac7fea618d35d8f8a98f7f3abee8c9) )//10p
	ROM_LOAD( "clubexecutive2.3", 0x8000, 0x4000, CRC(cc286353) SHA1(13f19396972e6a65c88245ff291ff3618f1b7fa7) )//10p

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "150 2.1.bin", 0x0000, 0x4000, CRC(a1cd6f9f) SHA1(6137c7410b29d3ddf3d52a0e64b316d85f1b3e08) )
	ROM_LOAD( "150 2.3.bin", 0x0000, 0x4000, CRC(8d9dc4da) SHA1(a6651d1dd50a28b6367330ec8aa311ef68f04356) )
	ROM_LOAD( "20p 2.2.bin", 0x0000, 0x4000, CRC(c53bde72) SHA1(369257c57fded297e5625a857d3f4b0eace64763) )
	ROM_LOAD( "5p 2.1.bin", 0x0000, 0x4000, CRC(8620217a) SHA1(25ed75529812c106fb32b1f7b93c8b26083d8fd8) )
	ROM_LOAD( "5p 2.2.bin", 0x0000, 0x4000, CRC(dc09b228) SHA1(6f392025f6d9e8b5d03f8ad82a5ab072b272679b) )
	ROM_LOAD( "5p 2.3.bin", 0x0000, 0x4000, CRC(b2be1986) SHA1(68e571187bf87bed8a2027e746d7ba8c6ed19282) )
ROM_END



ROM_START( j2fasttk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fast_trak_3_1.bin", 0x0000, 0x8000, CRC(67b9ff41) SHA1(42f42aa49b6ca3a944d12ec8d351eee28f10725d) )
	ROM_LOAD( "fast_trak_3_2.bin", 0x0000, 0x8000, CRC(cc05816c) SHA1(09504aca557e9c112ffc39b814d2a043bca36ea0) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "fsttrk31.bin", 0x0000, 0x8000, CRC(59763417) SHA1(e59ff3f2594596d9523a3875ae51f00da9da5dde) )
	ROM_LOAD( "fsttrk32.bin", 0x0000, 0x8000, CRC(157e2afe) SHA1(56ec9947214db23b13848d4dfcd606be0d0b424c) )
ROM_END


ROM_START( j2fqueen )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "findthequeen.bin", 0x0000, 0x8000, CRC(9bd37775) SHA1(9e4587124648008d144d1eac46a542056e55e270) )
ROM_END



ROM_START( j2fiveal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fivealive1_1.bin", 0x0000, 0x8000, CRC(87b46057) SHA1(fc8c29a66813bf102eeed5859f63a31d72b103a2) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "fivealive1b_1.bin", 0x0000, 0x8000, CRC(7600bfb8) SHA1(4e7184a8dbd5c03c914e3b7860f1f63e54006697) )
ROM_END



ROM_START( j2fiveln )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "five_liner_1_1.bin", 0x0000, 0x8000, CRC(dc5f04e8) SHA1(b579a2b7d074d52e7bb2e903bcc4c0f69083dd03) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "fiveliner10p3.1.bin", 0x0000, 0x8000, CRC(8158cf7c) SHA1(50456bca9121a3c39106ad30f14d352e1a600b17) )
	ROM_LOAD( "fiveliner20p5.1.bin", 0x0000, 0x8000, CRC(e3f0ef82) SHA1(f4ebb22a057f793652f7786a4f01c5b6e6a1a245) )
ROM_END



ROM_START( j2fws )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5wayshuffle2_1.bin", 0x0000, 0x8000, CRC(34d71a37) SHA1(8f720787f9669ea4ff05350e5cc3b0213fff8637) )
	ROM_LOAD( "5wayshuffle2_2.bin", 0x8000, 0x8000, CRC(37f83030) SHA1(2c76a96268aa87a992df339d22a8c3cae2b469b1) )
ROM_END

ROM_START( j2fwsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5wayshuffle3_1.p1", 0x0000, 0x8000, CRC(81c3a09d) SHA1(93a74956ba6f59a659a6782725ef2d6f4773253e) )
	ROM_LOAD( "5wayshuffle3_1.p2", 0x8000, 0x8000, CRC(cc5a6c31) SHA1(c343c6b20964f28676769be1300968672987b527) )
ROM_END

ROM_START( j2frmtch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fmach1.bin", 0x0000, 0x4000, CRC(5adda171) SHA1(3bf4c9e3f416869c13d061770e24cc43f6e4b302) )
	ROM_LOAD( "fmach2.bin", 0x4000, 0x4000, CRC(8514ef24) SHA1(2abf0395ba6ce9f045b2d0d0844fd5ae351a5061) )
ROM_END



ROM_START( j2fullhs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fullhouse_3_1.bin", 0x0000, 0x8000, CRC(9e272562) SHA1(4aa009add7ce763af166ed672e1e83bbc52d0375) )
	ROM_LOAD( "fullhouse_3_2.bin", 0x8000, 0x8000, CRC(acc9f0ba) SHA1(219345ddd9470e6f857218f86a3a7342ec584bdc) )
ROM_END



ROM_START( j2ghostb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ghostbuster2.1.bin", 0x0000, 0x2000, CRC(59a50c84) SHA1(8273ec2d103b8017b0a0cfea81393123d0f80d2d) )
	ROM_LOAD( "ghostbuster2.2.bin", 0x2000, 0x2000, CRC(4b53de33) SHA1(77beb73ebf35f0f0eff7109b5fd5d699931fa04d) )
ROM_END



ROM_START( j2goldrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldrun.bin", 0x0000, 0x6000, CRC(9e8cc91d) SHA1(d37f9bfd9af3a141ea55cf9b92453d2ca2f35f03) ) /* split */
ROM_END



ROM_START( j2goldbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpmgoldenbars2.1.bin", 0x0000, 0x8000, CRC(301a9321) SHA1(861854d000a3102265cc8254996a3ee8c3639855) )
	ROM_LOAD( "jpmgoldenbars2.2.bin", 0x8000, 0x8000, CRC(d94d1c30) SHA1(f26c4eb158c8291521b23c091bf45475b70cc3ae) )

	// this is a 68k rom
	//  ROM_LOAD( "jpmgoldenbars1.1.bin", 0x0000, 0x8000, CRC(45f91660) SHA1(1c6bc864e56c8c6ea61ebb5e181ad736aeab06cf) )
	//  ROM_LOAD( "jpmgoldenbars1.2.bin", 0x0000, 0x8000, CRC(eb6595f0) SHA1(2b0aabb50a1d1f88249b733faf02194c0181f999) )
	//  ROM_LOAD( "jpmgoldenbars1.3.bin", 0x0000, 0x8000, CRC(01c7dcfb) SHA1(9f00a14df5b2ea13d2bd4f3ff1ab5ee65d464709) )
	//  ROM_LOAD( "jpmgoldenbars1.4.bin", 0x0000, 0x8000, CRC(88bf0d26) SHA1(ecbfa69ffde42dc4464f39fc641c98a8485e0218) )
ROM_END



ROM_START( j2gldchy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldencherry1.1.bin", 0x0000, 0x4000, CRC(9b6232b1) SHA1(9f102cd7b5e0ae1ca4e9d5b6e5ab9012b0f736b8) )
	ROM_LOAD( "goldencherry1.2.bin", 0x4000, 0x4000, CRC(af08dc61) SHA1(7d47d510ecd0b285c58e9492a4ea325b6f74815e) )
ROM_END



ROM_START( j2gldwin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldenwin7.1.bin", 0x0000, 0x4000, CRC(8ba51701) SHA1(8dfd78ebddab2dc71f8cfea2bdc7698e5ba94869) )
	ROM_LOAD( "goldenwin7.2.bin", 0x4000, 0x4000, CRC(64c004f5) SHA1(cc5ff5f94e939fc67844d928ca06b0ee0c5dc60b) )
ROM_END



ROM_START( j2hinote )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hi_note_2_1.bin", 0x0000, 0x8000, CRC(f8ccdb6a) SHA1(3f6fa27295bfb64f3176dfc7f8f163015df7fdd7) )
	ROM_LOAD( "hi_note_2_2.bin", 0x8000, 0x8000, CRC(57baf38f) SHA1(a3906486aca7e76f1faacc5df14ec6793bf0842e) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "hinote21.bin", 0x0000, 0x8000, CRC(bf4cbae5) SHA1(5504ddafb12e553a9a19b4e845fdf2d16cda0702) )
	ROM_LOAD( "hinote22.bin", 0x8000, 0x8000, CRC(eb6b4d87) SHA1(f86d7f2b8ff18918dfc9e4471a0c76c8cae8c236) )
ROM_END



ROM_START( j2hiroll )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hi_roller_1_1.bin", 0x0000, 0x8000, CRC(da7d05b4) SHA1(e594b6d46a525453be7eb93e38e81a689be30c66) )
	ROM_LOAD( "hi_roller_1_2.bin", 0x8000, 0x8000, CRC(cfde426f) SHA1(6c8d4044f783dee48c96a1b172836ead5039e0bf) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "hi_roller_1c1.bin", 0x0000, 0x8000, CRC(176e8fe0) SHA1(296da0cc61fc97b4545a58d799e31b282d107946) )
	ROM_LOAD( "hi_roller_1c2.bin", 0x0000, 0x8000, CRC(99cc5b33) SHA1(028f1638a354d399ede0e0096895a10bafdbc323) )
	ROM_LOAD( "hiroller2_1.bin", 0x0000, 0x8000, CRC(719bd3df) SHA1(66d73223b5752c337258d5a710697d58dad0724f) )
	ROM_LOAD( "hiroller2_2.bin", 0x0000, 0x8000, CRC(99cc5b33) SHA1(028f1638a354d399ede0e0096895a10bafdbc323) )
	ROM_LOAD( "hiroller_1_1.bin", 0x0000, 0x8000, CRC(bdc1f7f6) SHA1(86617a7d40178cae26748987208324de760fa9c5) )//Protocol?
ROM_END


ROM_START( j2hotpot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hpv7-1.p1", 0x0000, 0x2000, CRC(bb43d2fa) SHA1(23a3fd7cf49fb4540467dc695973f943c6d659c7) )//3gbp token, v7.1
	ROM_LOAD( "hpv7-2.p2", 0x2000, 0x2000, CRC(a3e59d08) SHA1(1f9872cfdc80216972fe071603bafb7caaf81bc8) )
	ROM_LOAD( "hpv7-3.p3", 0x4000, 0x2000, CRC(1413c369) SHA1(e6d72ab545c74ee3cf8645f8719a798caa4d0f88) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "hotpot1.bin", 0x0000, 0x2000, CRC(92f551c7) SHA1(ddb59a25167784591daa06408a81406a75e53415) )
	ROM_LOAD( "hotpot1a.bin", 0x0000, 0x2000, CRC(381f3a97) SHA1(dc7b0c53dc5e953d9cce5ea3502f1940eafc1565) )
	ROM_LOAD( "hotpot2.bin", 0x0000, 0x2000, CRC(fa7ead32) SHA1(2c79fbbb93e20e2dc09996e94000618c18162018) )
	ROM_LOAD( "hotpot2a.bin", 0x0000, 0x2000, CRC(0c7f9975) SHA1(e53a24270cf26f6f484ddee414472ac36891febd) )
	ROM_LOAD( "hotpot3.bin", 0x0000, 0x2000, CRC(1b0a934c) SHA1(e65f0f7050c8683954cd34389a4303cfbd35a53e) )
ROM_END


ROM_START( j2hotptd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hotptdl1.bin", 0x0000, 0x2000, CRC(f4852b74) SHA1(4e9b517df02afd4cefffa9b971eb0933142e64d0) )
	ROM_LOAD( "hotptdl2.bin", 0x2000, 0x2000, CRC(c912e5a3) SHA1(744a4acb93d80588b31280164b94f5248e6a7736) )
	ROM_LOAD( "hotptdl3.bin", 0x4000, 0x2000, CRC(e2793969) SHA1(dfcd2caae3d5dc8f9a37bd4b18002296fafe4eb6) )
ROM_END



ROM_START( j2hotsht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hot_shot_5p_1_1.bin", 0x0000, 0x4000, CRC(a59ff613) SHA1(853614765dc1c511315e0679d5d63a740edaa9dd) )
	ROM_LOAD( "hot_shot_5p_1_2.bin", 0x4000, 0x4000, CRC(59d52547) SHA1(1cf1e054bec55d2b10a4d7a253bb31008e62461b) )
ROM_END



ROM_START( j2hypnot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hypnot41.bin", 0x0000, 0x4000, CRC(c63de222) SHA1(668375e3e2ffccad04523debc208227874f9469c) )
	ROM_LOAD( "hypnot42.bin", 0x4000, 0x4000, CRC(4ae94c6c) SHA1(478f3c656240e3eac6624612ec1a6944b4ca6e54) )
	ROM_LOAD( "hypnot43.bin", 0x8000, 0x4000, CRC(5b1e61c0) SHA1(5fcc076412f1c715fa1ff0c38253a95deadbecf9) )
ROM_END



ROM_START( j2jackbr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpbar121.bin", 0x0000, 0x4000, CRC(410cdf8f) SHA1(0112e6422f10317f7d2bacb10b79e1813a53cf24) )
	ROM_LOAD( "jpbar122.bin", 0x4000, 0x4000, CRC(d8b1500a) SHA1(c2528fe2e56a5aed2b9f4132f315322f2e9b7803) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "jpbar231.bin", 0x0000, 0x4000, CRC(734d898a) SHA1(0baa5722865d85744918c028f00e85c64cf6eba6) )
	ROM_LOAD( "jpbar232.bin", 0x4000, 0x4000, CRC(08af1772) SHA1(5ea8db5c9da5fcfa295a6b98b6a76d11fc76f580) )
ROM_END



ROM_START( j2jackdc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jackpot.bin", 0x0000, 0x010000, CRC(0895de20) SHA1(cb34730feb70997e7c3f8b99bec857114a902fed) ) /* split? */
ROM_END



ROM_START( j2jokers )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jokers1.bin", 0x0000, 0x4000, CRC(a876d542) SHA1(884f7451d1c27c85afd63247be433b0e33e116bf) )
	ROM_LOAD( "jokers2.bin", 0x4000, 0x4000, CRC(e3fefaa6) SHA1(c8d3375cb58f6ece09768df18c0018cc99839ab5) )
ROM_END



ROM_START( j2kingcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "koc1v4.bin", 0x0000, 0x8000, CRC(5acecdbf) SHA1(f58f1ce3ca51038f41d2b893f3c15637e9d28ee0) )
	ROM_LOAD( "koc2v4.bin", 0x8000, 0x8000, CRC(6f9b132f) SHA1(14154898fcd2c75e15eeec282b2452c81234b00e) )

	// these look like bellfruit (encrypted) roms
	//  ROM_LOAD( "king of clubs 39340002.bin", 0x0000, 0x8000, CRC(028708bf) SHA1(9e6942f6a25b260faa4c14c4d61a373be1518f40) )
	//  ROM_LOAD( "king of clubs 39340026.bin", 0x0000, 0x8000, CRC(d3b0746e) SHA1(2847cec108a99747a7e3e31a0f7bcf766cdc1546) )
ROM_END



ROM_START( j2litean )//MPS2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lite_a_nudge_mk3_3_1.bin", 0x0000, 0x8000, CRC(2e4f176a) SHA1(c369ab233b4892a65fa885906bb52585c178aa1b) )
	ROM_LOAD( "lite_a_nudge_mk3_3_2.bin", 0x8000, 0x8000, CRC(f427b39d) SHA1(24907b2c7936f178db183796543e338b7206c107) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "lan5p1.1", 0x0000, 0x4000, CRC(70d37765) SHA1(1903f765247b7eb7fd681feb2f0d55119c1b6d67) )
	ROM_LOAD( "lan5p1.2", 0x0000, 0x4000, CRC(ca092928) SHA1(1b5e4825bc13b2f4eacae42439ca4175699a0bc2) )
	ROM_LOAD( "lan_10p_1_1.bin", 0x0000, 0x4000, CRC(ce467343) SHA1(cd3ecb760e96a577ae86554fcd15c2d984656b10) )
	ROM_LOAD( "lan_10p_1_2.bin", 0x0000, 0x4000, CRC(1c256a65) SHA1(770f709ed0247ae3c3746456339718a536e12a05) )
	ROM_LOAD( "lan_2p_1_1.bin", 0x0000, 0x4000, CRC(a86a03c6) SHA1(94db532b8e95ebad7682d2edbffadf898f6b15f6) )
	ROM_LOAD( "lan_2p_1_2.bin", 0x0000, 0x4000, CRC(ff4f3fcd) SHA1(fb56604bec7420f37f837469de5c0d66596de0ce) )
	ROM_LOAD( "lan_4_1.bin", 0x0000, 0x8000, CRC(05a988d7) SHA1(1da0333362270b0cde8a20dd195121a6f9e0e582) )//88
	ROM_LOAD( "lan_4_2.bin", 0x0000, 0x8000, CRC(19f91447) SHA1(36a374156164566f76e69fa499bbbe950783ae98) )//88
	ROM_LOAD( "lanudgemk32.1.bin", 0x0000, 0x8000, CRC(399e85d6) SHA1(08c9dd585fedcfacefe73614fa319650374b007f) )
	ROM_LOAD( "lanudgemk32.2.bin", 0x0000, 0x8000, CRC(fc47568b) SHA1(5fe87ba8be1d956f8a15d57562049a57786e3ddd) )
ROM_END


ROM_START( j2lovsht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "looshoo1.bin", 0x0000, 0x2000, CRC(be017717) SHA1(f1e14c39d781265da92f18daf89b70a8d64ac973) )
	ROM_LOAD( "looshoo2.bin", 0x2000, 0x2000, CRC(8d75bce8) SHA1(63d0e36c02874dfa87da0449206b8902a5f8616a) )
	ROM_LOAD( "looshoo3.bin", 0x4000, 0x2000, CRC(8f37689f) SHA1(4bfcfc4a8369a7223523112e8b56032f2f5a0f12) )
ROM_END


ROM_START( j2lovshd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lootshootdx1-1.p1", 0x0000, 0x2000, CRC(e468ce58) SHA1(a588c85a66ea68172c0f3a5317beee04902d7ac8) )
	ROM_LOAD( "lootshootdx1-2.p2", 0x2000, 0x2000, CRC(30424219) SHA1(10e635e48f3808bbdcc349aecb3f09dcb03a752d) )
	ROM_LOAD( "lootshootdx1-3.p3", 0x4000, 0x2000, CRC(48134998) SHA1(72b40153638eeed080e01fccdf96c513d3b1cfd6) )
ROM_END

ROM_START( j2luckar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la_2_1_10p.p1", 0x0000, 0x8000, CRC(027beb54) SHA1(4522ed4e77c77a3909eed71f66f45d916b824b79) )
ROM_END


ROM_START( j2lucky2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lucky2s10p3.1.bin", 0x0000, 0x8000, CRC(54bc5a4e) SHA1(eb59e47fc4fd2971ab170249e56c8a751b2e45ac) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "lucky2s2p3.1.bin", 0x0000, 0x8000, CRC(5ea016b4) SHA1(9c10a66cd0c29dd5b787ba5bc9dff6bad65e38e7) )
	ROM_LOAD( "lucky2s5p3.1.bin", 0x0000, 0x8000, CRC(034407f8) SHA1(3f223586ad8af4e7dea6d4f3958233bac6485566) )
ROM_END


ROM_START( j2monblt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mb2-1.bin", 0x0000, 0x4000, CRC(e3f3bbdc) SHA1(84be0d32025f73d2ef1b0e378a7826c46a00f414) )
	ROM_LOAD( "mb2-2.bin", 0x4000, 0x4000, CRC(c0078fa9) SHA1(c84bf3e16730990ef62ec757d0d05471712c8731) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "monbelt1.bin", 0x0000, 0x4000, CRC(f7028728) SHA1(bbce0924aa1802a1ba0f3d37f7c8c2581f5b1104) )
	ROM_LOAD( "monbelt2.bin", 0x0000, 0x4000, CRC(8da3dff8) SHA1(3b50478a0616e8b4b82f19bafcab149560c7a4d1) )
ROM_END



ROM_START( j2mongam )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mongam61.bin", 0x0000, 0x2000, CRC(1ac86006) SHA1(e59caa18597abfcf42cb60241337e023fb33b723) )
	ROM_LOAD( "mongam62.bin", 0x2000, 0x2000, CRC(88bcce8a) SHA1(4d5c71b4807be72b7ecf99fbddf79d734e101832) )
	ROM_LOAD( "mongam63.bin", 0x4000, 0x4000, CRC(4a8d11a0) SHA1(fe7e00cfb10d83d9f521cf6dcd27d51a561af916) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	// something else? TTX dumps this as the MPS set
	ROM_LOAD( "moneygame3.1.bin", 0x0000, 0x2000, CRC(14734a7e) SHA1(84bcfaaa3f59d8ef2acefe399b48f258e507f20a) )
	ROM_LOAD( "moneygame3.2.bin", 0x0000, 0x2000, CRC(6c7ce60c) SHA1(c775925efbb44963d57aea5719d9a3aba181ab71) )
	ROM_LOAD( "moneygame3.3.bin", 0x0000, 0x4000, CRC(1303e942) SHA1(3e9436408375c7ca0f636f9eb92bec48e3d1d05c) )
ROM_END



ROM_START( j2mongmd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mongmdl1.bin", 0x0000, 0x2000, CRC(4efbdd5e) SHA1(976b069ef4c5cf08730c1c5cca08a24c4c878af0) )
	ROM_LOAD( "mongmdl2.bin", 0x2000, 0x2000, CRC(4303210e) SHA1(f114343dad760d0cd35e01be4eef32c5e9aae08c) )
	ROM_LOAD( "mongmdl3.bin", 0x4000, 0x4000, CRC(905f0b06) SHA1(a889f8405458b272f62012c3d794939976c43e6e) )
ROM_END



ROM_START( j2multwn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mw-p1.bin", 0x0000, 0x8000, CRC(3b602e9b) SHA1(7ad13fa78b473ce6cdef769afd9bc149e40c72f1) )
	ROM_LOAD( "mw-p2.bin", 0x8000, 0x8000, CRC(0258f0e9) SHA1(65a14324b85a949c8c659fc249b7622eaed3915d) )
ROM_END



ROM_START( j2notexc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "note_exch_4_1.bin", 0x0000, 0x8000, CRC(927b61db) SHA1(30da9707e811e90fa3d4e09d6ad7b7c30b53b177) )
	ROM_LOAD( "note_exch_4_2.bin", 0x8000, 0x8000, CRC(6853d3ef) SHA1(9f5a0c9d09177ca71b11f35e0c8c1c828099172f) )
ROM_END

ROM_START( j2notexca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "notexc41.bin", 0x0000, 0x8000, CRC(3e7e707a) SHA1(4652ab41e8e8ba5bcd7df0638f55d63d656768ce) )
	ROM_LOAD( "notexc42.bin", 0x8000, 0x8000, CRC(33f5c3cc) SHA1(ed8cc3df7662e2843816f1cb62f92f0778933cc5) )
ROM_END

ROM_START( j2notexcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "note_exchange_2_1.bin", 0x0000, 0x8000, CRC(74b00405) SHA1(1672db0d6304c756472395c672cf0a49c7850c11) )
	ROM_LOAD( "note_exchange_2_2.bin", 0x8000, 0x8000, CRC(06386b08) SHA1(deb2622a3a3c2a442ca1407fdc196d6a06e5aa80) )
ROM_END

ROM_START( j2notesh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "note_shoot_1_1.bin", 0x0000, 0x8000, CRC(42a36aa6) SHA1(ee375f878dc7bd6e0b7c64076c89da7997d66e80) )
	ROM_LOAD( "note_shoot_1_2.bin", 0x8000, 0x8000, CRC(9e610213) SHA1(023f4da0ff04a411a6023c6bca888cb6f7308edf) )
ROM_END



ROM_START( j2nudbnz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudgebonanza1.bin", 0x0000, 0x2000, CRC(df542949) SHA1(0021f987036e0089a7f6d9a879844f596150d7ac) )
	ROM_LOAD( "nudgebonanza2.bin", 0x2000, 0x2000, CRC(0245d2ab) SHA1(b8592ae6b63f3eb193752f43c203ee99a2a1f91f) )
	ROM_LOAD( "nudgebonanza3.bin", 0x4000, 0x4000, CRC(2ed7e15d) SHA1(28ced6bf5945eb20a34e0cd24f55ed9e5c616146) )
ROM_END

ROM_START( j2nudbnza )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nb_1.p1", 0x0000, 0x2000, CRC(16344441) SHA1(e16a72111a1f1bcee461749cdb9f0f0ec25a4d56) )
	ROM_LOAD( "nb_1.p2", 0x2000, 0x2000, CRC(af66679c) SHA1(170e83cbe3b50da8b59dcb5a64ff08cc20cb1b28) )
	ROM_LOAD( "nb_1.p3", 0x4000, 0x2000, CRC(79f779a4) SHA1(86fd00ef4aef0bb3074a8a541bdc5cddc9376414) )
ROM_END


ROM_START( j2nuddup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndudp188", 0x0000, 0x8000, CRC(13c743dd) SHA1(f818f97e4183d24bf565ed4874e46c217d6cb94d) )//5307, deluxe 2.1
	ROM_LOAD( "ndudp288", 0x8000, 0x8000, CRC(2168fbeb) SHA1(5e3475af6d1778ecf536fc6496ca1dc82b151b9d) )//5308, deluxe 2.2

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "5310.bin", 0x0000, 0x8000, CRC(90cda2cb) SHA1(e2d8cfa5d6e475dc0cdda53530332019dce9d9df) )
ROM_END


ROM_START( j2nuddud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudgedoubleupdelux1.1", 0x0000, 0x8000, CRC(638321d3) SHA1(d1e5a7debfb957ad4d85e7501ec78a37fe56921d) )
	ROM_LOAD( "nudgedoubleupdelux1.2", 0x8000, 0x8000, CRC(398eafaa) SHA1(84eb5729cf0a7a387693eda0a7214e0c23ae9bab) )
ROM_END



ROM_START( j2nudup3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu_p1.bin", 0x0000, 0x2000, CRC(cd1a885d) SHA1(f516ecf9933b674854955cda90a0fdfdbc17ec12) )
	ROM_LOAD( "ndu_p2.bin", 0x2000, 0x2000, CRC(92702d92) SHA1(0abf307ff0dca7f9ad55bad6d28a82d0f2396dc6) )
	ROM_LOAD( "ndu_p3.bin", 0x4000, 0x2000, CRC(3b5cf755) SHA1(a7460d4a1350f2136ce83631a79c88a9991bada9) )
ROM_END



ROM_START( j2nudshf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nshuffler10p2.1.128k.bin", 0x0000, 0x4000, CRC(070f68a3) SHA1(389a401365a993f00fef27905473e76ec5dd8829) )
	ROM_LOAD( "nshuffler10p2.2.128k.bin", 0x4000, 0x4000, CRC(a28174be) SHA1(dba251e394e9e8714e7d74b0c7da05969d84056d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "nshuffler2p2.1.128k.bin", 0x0000, 0x4000, CRC(aa54a73b) SHA1(e4f02a1d913f292864586488080cdd2939e0773b) )
	ROM_LOAD( "nshuffler2p2.2.128k.bin", 0x0000, 0x4000, CRC(89f3e3d5) SHA1(c84b0052d7d45781d46a067193f825228904044c) )
	ROM_LOAD( "nshuffler5p3.1.128k.bin", 0x0000, 0x4000, CRC(f1d835dd) SHA1(2ce68da041cbff4dc7352683e76c587160cf3ba2) )
	ROM_LOAD( "nshuffler5p3.2.128k.bin", 0x0000, 0x4000, CRC(adf6a580) SHA1(0ed51229fd80a28bea655c35ca8886cf950d3e3f) )
ROM_END



ROM_START( j2plsmon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plsmon31.bin", 0x0000, 0x4000, CRC(0d5b9c23) SHA1(f449aea9f4754cc2425ea2ae13a776b65ef37c74) )
	ROM_LOAD( "plsmon32.bin", 0x4000, 0x4000, CRC(24b6ef68) SHA1(aef382faec1c55abad7ff3b2dc9e46ce8afe0c00) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "pmd11.bin", 0x0000, 0x4000, CRC(c436038d) SHA1(b2ffbbe8abc7cfc7f2547d5ae1ce61657fe8c00d) )
	ROM_LOAD( "pmd12.bin", 0x0000, 0x4000, CRC(73ebb1a1) SHA1(5dacc96a6a01343947ed0ae25d25353cfa38a31e) )
	ROM_LOAD( "pmd1a1.bin", 0x0000, 0x4000, CRC(f9254691) SHA1(96d7dd37d04181648e7cbb471f095449e12a8d71) )
	ROM_LOAD( "pmd1p1.bin", 0x0000, 0x4000, CRC(ce4e9df7) SHA1(b86432e1665b70fc392256da4837f97e04231e4d) )
	ROM_LOAD( "pmd1x1.bin", 0x0000, 0x4000, CRC(d0c73f79) SHA1(b2fa4f2b300f11f2c6ed5c550906af0e391163e0) )
	ROM_LOAD( "pmd1y1.bin", 0x0000, 0x4000, CRC(edd47a65) SHA1(57e9131d64807d8fe7f4bd37b546b7bd40b892c6) )
ROM_END



ROM_START( j2plsmnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pmd11.bin", 0x0000, 0x4000, CRC(c436038d) SHA1(b2ffbbe8abc7cfc7f2547d5ae1ce61657fe8c00d) )
	ROM_LOAD( "pmd12.bin", 0x4000, 0x4000, CRC(73ebb1a1) SHA1(5dacc96a6a01343947ed0ae25d25353cfa38a31e) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "pmd1a1.bin", 0x0000, 0x4000, CRC(f9254691) SHA1(96d7dd37d04181648e7cbb471f095449e12a8d71) )
	ROM_LOAD( "pmd1p1.bin", 0x0000, 0x4000, CRC(ce4e9df7) SHA1(b86432e1665b70fc392256da4837f97e04231e4d) )
	ROM_LOAD( "pmd1x1.bin", 0x0000, 0x4000, CRC(d0c73f79) SHA1(b2fa4f2b300f11f2c6ed5c550906af0e391163e0) )
	ROM_LOAD( "pmd1y1.bin", 0x0000, 0x4000, CRC(edd47a65) SHA1(57e9131d64807d8fe7f4bd37b546b7bd40b892c6) )
ROM_END



ROM_START( j2plsnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plusnudge10p2.1.bin", 0x0000, 0x8000, CRC(68a3418c) SHA1(bfa50ac6c4f7e5c2367e16c6cde72e0591900820) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "plusnudge2p2.1.bin", 0x0000, 0x8000, CRC(c506cd38) SHA1(a76177068a9c48dbf5155a6af548039560252ced) )
	ROM_LOAD( "plusnudge5p2.404.802.1.bin", 0x0000, 0x8000, CRC(5d51671d) SHA1(5d0ee84ffebc3a1f2323a9358fce9a93193d9d2a) )
	ROM_LOAD( "plusnudge5p241.1.bin", 0x0000, 0x8000, CRC(d5f8efb2) SHA1(481c3ce4bff55f121b44285bdc2cb1cf46db52b3) )
ROM_END



ROM_START( j2potlck )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "potlucka.bin", 0x0000, 0x2000, CRC(3b93d8b4) SHA1(15760db52030aad4e26a5912b2404f9bc7d0daa3) )
	ROM_LOAD( "potluckb.bin", 0x2000, 0x2000, CRC(0ad9837b) SHA1(caa8762ef01beb5bcb1c874dec0cc5ccf6601e55) )
	ROM_LOAD( "potluckc.bin", 0x4000, 0x2000, CRC(ae1434e3) SHA1(de26c05ab7dd6b686d5425bdd1fd7fcf6c682859) )
ROM_END



ROM_START( j2pndrsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pndrsh31.bin", 0x0000, 0x8000, CRC(6cfa9a9e) SHA1(d19b477854592e8ecb034cf2c4081ab98b8aa5a3) )
	ROM_LOAD( "pndrsh32.bin", 0x8000, 0x8000, CRC(665c7cff) SHA1(70e9ea72f690fb86178a75d63f4dc090d076b0bb) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "poundrush3.1", 0x0000, 0x8000, CRC(4e29243c) SHA1(072bf69c399951105615d900274c6b5f6003cf06) )
	ROM_LOAD( "poundrush3.2", 0x0000, 0x8000, CRC(861088d3) SHA1(471d46fddfe08b46abbe49fef6e175f9cbcdd05a) )
ROM_END


ROM_START( j2pyramd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pyramid4.1.bin", 0x0000, 0x2000, CRC(9b06811e) SHA1(e921aaa42bc4c9ece48a8dba492006b5186c559e) )
	ROM_LOAD( "pyramid4.2.bin", 0x2000, 0x2000, CRC(2cc5359d) SHA1(022dd422fa8bf6951303bca17b5fe65164a402e3) )
	ROM_LOAD( "pyramid4.3.bin", 0x4000, 0x4000, CRC(fd977a84) SHA1(3f7c1bf6de88ed5352f1fd65a1d6abc5d2b2ee9a) )
ROM_END



ROM_START( j2reelbn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rlbingo1.bin", 0x0000, 0x4000, CRC(0978c56a) SHA1(ba2c53181a80c6f7bc04a5a139ec3327ddcec03c) )
	ROM_LOAD( "rlbingo2.bin", 0x4000, 0x4000, CRC(41c3de8f) SHA1(9a43c49961a1878044292ad4f3a185185e1044f1) )
ROM_END

ROM_START( j2reelbna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rlbingo1.bin", 0x0000, 0x4000, CRC(0978c56a) SHA1(ba2c53181a80c6f7bc04a5a139ec3327ddcec03c) )
	ROM_LOAD( "rlbngo42.bin", 0x4000, 0x4000, CRC(039750b6) SHA1(d3cc77938e5eac264746659e5af26e04c3a2e12c) )
ROM_END

ROM_START( j2reelbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelbonus2.1", 0x0000, 0x8000, CRC(7f1d72e1) SHA1(47419282f42a84c980ff93031988b6b10272123a) )
	ROM_LOAD( "reelbonus2.2", 0x8000, 0x8000, CRC(a22045e5) SHA1(477b276ce2f3bd04b8d3e69471a23677bc3f24fd) )
ROM_END





ROM_START( j2reelmg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rmjcp1v1", 0x0000, 0x8000, CRC(668179dc) SHA1(cc51c55483a34e8764411bbfa9cbbe7e4957ca9e) )
	ROM_LOAD( "rmjcp1v2", 0x8000, 0x8000, CRC(95fe6c56) SHA1(af99358b5df03a67e685b1dd70e25314997d7e71) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "rmjcp3v1", 0x0000, 0x8000, CRC(2b0fab59) SHA1(6ed389c8cfdfdf8c45f69359a852ba20536654a1) )
	ROM_LOAD( "rmjcp3v2", 0x8000, 0x8000, CRC(8626395f) SHA1(ba08b9e8a47a2b16889e97ed10fb3db7fb0332fb) )
ROM_END

ROM_START( j2reelmgd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelm.bin", 0x0000, 0x010000, CRC(d0351966) SHA1(8d771eb470a7581196398229b9033f37ed91564c) ) /* split? */
ROM_END



ROM_START( j2reelmc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelmagicclub2.1", 0x0000, 0x8000, CRC(86249072) SHA1(4dab692e082e10db42c5afbc86dd9b8dd3cc8563) )
	ROM_LOAD( "reelmagicclub2.2", 0x8000, 0x8000, CRC(9fff1110) SHA1(09a52d3f0efa175e89ccf031947443872e5ad05a) )
ROM_END


ROM_START( j2reelmo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reelmon1.bin", 0x0000, 0x4000, CRC(80c18a15) SHA1(0803ffab57ea067dc0be1096650cdc91a3e0520a) )
	ROM_LOAD( "reelmon2.bin", 0x4000, 0x4000, CRC(8eaf21fc) SHA1(3511b5c6700d70a2da8f088b41c4da196b502c91) )
ROM_END



ROM_START( j2rotnot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rotnot21.bin", 0x0000, 0x8000, CRC(eeb22e36) SHA1(fbb2d8fd23d67af323d6af2499744cadcc394c6d) )
	ROM_LOAD( "rotnot22.bin", 0x8000, 0x8000, CRC(145447b9) SHA1(d95ab24c2ff3aa67ee00dbe603df1f7384dc90fc) )
ROM_END

ROM_START( j2roulcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "clubroulette1.1", 0x0000, 0x4000, CRC(4b7ed007) SHA1(e910486c88ac226df8ed904d0bf18791bf832595) )
	ROM_LOAD( "clubroulette1.2", 0x4000, 0x4000, CRC(74ade689) SHA1(0ec892c274ff40224d895c44725d99c06d0a6b0a) )
ROM_END


ROM_START( j2rdclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "royaldeal5p2.1", 0x0000, 0x4000, CRC(aaedeb89) SHA1(ae4a9ab549bf7c5e70781a4db709a1bad411d766) )
	ROM_LOAD( "royaldeal5p2.2", 0x4000, 0x4000, CRC(3d8e7847) SHA1(18a57a29754708b217eaa475a2dbe2ca30bcf0f3) )
ROM_END


ROM_START( j2slvrgh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b16436 delux 3.1.bin", 0x0000, 0x2000, CRC(9f3486f7) SHA1(7dd066444ce5475b069d4c664ac505856a0a5e42) )//dil85
	ROM_LOAD( "b16453 delux 3.2.bin", 0x2000, 0x2000, CRC(030168be) SHA1(b202f73ed6e9704ee3a26a56eb02c79695b4d54d) )//dil85

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "delux 1.1.bin", 0x0000, 0x2000, CRC(a0ba5a80) SHA1(4238f8a06b32a3f9a5523050b536696d5905d92e) )
	ROM_LOAD( "delux 1.2.bin", 0x2000, 0x2000, CRC(e5a192e6) SHA1(daafcb34bf88cd9e79856e52e57635e1703fb931) )
ROM_END

ROM_START( j2sldgld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "solidgold_5p__2.p1", 0x0000, 0x2000, CRC(0f7791a7) SHA1(50b672dbe693227ce3203e213d40bf524657238e) )//2 Pounds
	ROM_LOAD( "solidgold_5p__2.p2", 0x2000, 0x2000, CRC(e31b5902) SHA1(f50ac325fa43e26244c61d125a538f191ae9a234) )
	ROM_LOAD( "solidgold_5p__2.p3", 0x4000, 0x2000, CRC(972d6db7) SHA1(6a0ef0e458c58229d15801482f1bd77cbea5aad7) )
ROM_END

ROM_START( j2spcrsv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sprsvdl1.bin", 0x0000, 0x2000, CRC(150da174) SHA1(ba1eb762c227950c2de6a97444cfc42da26feceb) )
	ROM_LOAD( "sprsvdl2.bin", 0x2000, 0x2000, CRC(304a3f88) SHA1(f3e68dd3f2182bea9695b56562fa0fefa1d0edc1) )
	ROM_LOAD( "sprsvdl3.bin", 0x4000, 0x2000, CRC(9780a97f) SHA1(9cb146319b42a1ce1c1167051bfecfba51d943c3) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "sr31.bin", 0x0000, 0x2000, CRC(333cda7c) SHA1(9f2b4793c0121572d74374b15d0856b848213113) )
	ROM_LOAD( "sr32.bin", 0x2000, 0x2000, CRC(43c96908) SHA1(2f5cea9041236440dbd64a2575665035d71ecedb) )
	ROM_LOAD( "sr33.bin", 0x4000, 0x2000, CRC(bfad6479) SHA1(c1c640f9ec68a0c2f61441fd3c4ebdce9ec26c89) )
ROM_END



ROM_START( j2stahed )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "streets2.1", 0x0000, 0x8000, CRC(a5cc97a2) SHA1(dc6f4184b6eff4a85439348e0db132ac693c0837) )
	ROM_LOAD( "streets2.2", 0x8000, 0x8000, CRC(94b5ee13) SHA1(1a84200db11570793765cd3db2762bdceaba0b58) )
ROM_END



ROM_START( j2supfrt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supafruit 14.1.bin", 0x0000, 0x2000, CRC(a565a3bf) SHA1(a80a9b58d20627e64def7787012298beb36ebc70) )
	ROM_LOAD( "supafruit 14.2.bin", 0x2000, 0x2000, CRC(7131b034) SHA1(8c7ef185dcbedb890e94d32db88c1a59f2b032b7) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "supafruit 11.1 1m.bin",0x0000, 0x2000, CRC(b123ea7f) SHA1(0a5bbb9df5fee287cde8433c62b1c62d699b3d3a) )
	ROM_LOAD( "supafruit 11.3.bin",   0x0000, 0x2000, CRC(b0b4c97b) SHA1(0d6e8f3cb2d5d68d1e15899d5bfda3911409d113) )
	ROM_LOAD( "supafruit 11a2 m.bin", 0x0000, 0x2000, CRC(d5d27ece) SHA1(417b4e81ee6f2356dbeba5c23dee4f5e43157274) )
ROM_END


ROM_START( j2supfrc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supafruitclubfruit 057529 6.1.bin", 0x0000, 0x2000, CRC(05f34d40) SHA1(bf665549d3826c0ca70726dd31b253885d7f41e7) )
	ROM_LOAD( "supafruitclubfruit 057654 6.2.bin", 0x2000, 0x2000, CRC(70743781) SHA1(4b08aca7b40d183207de87d1df16b2965299d9e1) )
	ROM_LOAD( "supafruitclubfruit 057741 6.3.bin", 0x4000, 0x2000, CRC(88f775a4) SHA1(2c74a106a80ed929e3b27671e50d4d4a8b69de84) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "supafruitclubfruit 067290 10.1.bin", 0x0000, 0x2000, CRC(b6375d28) SHA1(7f8af10f8216ff10636ed4c954bae6c91128b42f) )
	ROM_LOAD( "supafruitclubfruit 067339 10.2.bin", 0x2000, 0x2000, CRC(dcf75abb) SHA1(519640d99d72312a521c578d3b9ad1f66bbbcc58) )
	ROM_LOAD( "supafruitclubfruit 067385 10.3.bin", 0x4000, 0x2000, CRC(b0fc5b2a) SHA1(a96d11f941133fb52041860093e92ec99a110275) )
	ROM_LOAD( "supafruitclubfruit 113506 11.2.bin", 0x0000, 0x2000, CRC(32f8b492) SHA1(ef1a52dce32693fe5641c5e44f369c77a5563864) )
	ROM_LOAD( "supafruitclubfruit 113585 14a2.bin", 0x0000, 0x2000, CRC(7131b034) SHA1(8c7ef185dcbedb890e94d32db88c1a59f2b032b7) )
	ROM_LOAD( "supafruitclubfruit 113600 14.3.bin", 0x0000, 0x2000, CRC(6682e8b0) SHA1(9c0127d0b31b77df4a18dd969e1f450f61737179) )
	ROM_LOAD( "supafruitclubfruit 15.1.bin", 0x0000, 0x2000, CRC(82ea80d2) SHA1(92f99b7588f0c4ca18af547df1f69e013e3b6887) )
	ROM_LOAD( "supafruitclubfruit 15.2.bin", 0x2000, 0x2000, CRC(648ed61a) SHA1(8f034de605401e7ee9c37deef00f39554faf857e) )
	ROM_LOAD( "supafruitclubfruit 15.3.bin", 0x4000, 0x2000, CRC(203908e7) SHA1(ace67b49c89847c37ff17bcbb6d2406028cb3c4d) )
ROM_END


ROM_START( j2supsft )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supshft1.bin", 0x0000, 0x4000, CRC(af33026c) SHA1(a937cfb0a443d028117ad51dd22c5b46859f68a2) )
	ROM_LOAD( "supshft2.bin", 0x4000, 0x4000, CRC(2176a98b) SHA1(3c0fd08a7a993ddbb75b82b54b52c0a610da8b17) )
	ROM_LOAD( "supshft3.bin", 0x8000, 0x4000, CRC(6181bdc6) SHA1(99863c8130d0d82fd3bdd9462ac1fd90794e31aa) )
ROM_END


ROM_START( j2supstp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supa_steppa_2_1.bin", 0x0000, 0x4000, CRC(e59e0c23) SHA1(ecd57b0c7ca5130946d2ccaf95ec611bc8701a5c) )//10p
	ROM_LOAD( "supa_steppa_2_2.bin", 0x4000, 0x4000, CRC(1cf2d94b) SHA1(a57908c14b4214b4d84d94065359baaef641df6f) )//10p

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "supa_steppa_5p_2_1.bin", 0x0000, 0x4000, CRC(ab1fbb54) SHA1(fc1799504893ea2c33dbbc5141b597cce09e7875) )
	ROM_LOAD( "supa_steppa_5p_2_2.bin", 0x4000, 0x4000, CRC(c7974fdb) SHA1(2011a16022e516ead025973854c59e81d4a600f5) )

// something else?
//  ROM_LOAD( "supasteppa2-1.p1", 0x0000, 0x1000, CRC(aac5b165) SHA1(5bf4acb85be227e1f4979fea4552fa5f64e9b7b2) )
//  ROM_LOAD( "supasteppa2-2.p2", 0x0000, 0x1000, CRC(3a93ea9e) SHA1(24e711a398d7f071fb904993ff0a974b4ac8b1d6) )

ROM_END



ROM_START( j2suptrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strack1.bin", 0x0000, 0x4000, CRC(cc111fc7) SHA1(2ffcf00b77861b856b0e4d668ff335961f6a465e) )
	ROM_LOAD( "strack2.bin", 0x4000, 0x4000, CRC(f7b4a85f) SHA1(c3427381316b6ee9f68b34a407fed4a3b1c36157) )
ROM_END



ROM_START( j2suprft )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superfruit.bin", 0x0000, 0x8000, CRC(91fcd2e4) SHA1(920a36ee0fecbf93f1ec1e784b53347c64229ae0) )
ROM_END



ROM_START( j2supln )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supaline8-1.p1", 0x0000, 0x2000, CRC(1c333a06) SHA1(3d45c46c604b21eacef03ede1b75946395718f87) )
	ROM_LOAD( "supaline8-2.p2", 0x2000, 0x2000, CRC(fa9fceba) SHA1(390e302e57accbfaeb1960e2781960e459cb3c42) )
	ROM_LOAD( "supaline8-3.p3", 0x4000, 0x2000, CRC(23794c20) SHA1(0bc6e33a1499d92a8b9518be29a0e8fe533a2ddf) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "supline1.bin", 0x0000, 0x2000, CRC(3cae87c0) SHA1(585488268f4b02f207d9135c8bbbe050d788fb28) )
	ROM_LOAD( "supline2.bin", 0x2000, 0x2000, CRC(ed7c5fdc) SHA1(15037b1521ca795b4664c740fe70bf9becfcc97d) )
	ROM_LOAD( "supline3.bin", 0x4000, 0x2000, CRC(23794c20) SHA1(0bc6e33a1499d92a8b9518be29a0e8fe533a2ddf) )
ROM_END



ROM_START( j2suppot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp-p1.bin", 0x0000, 0x2000, CRC(049fa419) SHA1(0093184470b06ff509b30dfc8cebb76622309c79) )
	ROM_LOAD( "sp-p2.bin", 0x2000, 0x2000, CRC(e7bfe97c) SHA1(f42c9bf89a7c69ab8607949b4b75dca24caa37d9) )
	ROM_LOAD( "sp-p3.bin", 0x4000, 0x2000, CRC(6321fa32) SHA1(8b626fd98ac3567a533a7960926846a07469a958) )
ROM_END


ROM_START( j2suprl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supreel.2-1", 0x0000, 0x2000, CRC(ce4c0d0e) SHA1(87bcfefb916f803a17f68d73159b29dca8521e3b) )
	ROM_LOAD( "supreel.2-2", 0x2000, 0x2000, CRC(7152dcce) SHA1(b0d377b1920a35a55cfcd8885e3810e5bf6ea81c) )
	ROM_LOAD( "supreel.3-3", 0x4000, 0x2000, CRC(c2d2de55) SHA1(f350e4f2efcc3a1c2f7f16b9d7b698966fb2b278) )
ROM_END



ROM_START( j2suprsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ss31.bin", 0x0000, 0x2000, CRC(1ca141f1) SHA1(9540a278a7a1183cb10dc444303c36c29b0fc3b9) )
	ROM_LOAD( "ss32.bin", 0x2000, 0x2000, CRC(f176cfd0) SHA1(7b938e93f267b4e54575597cc429aeb8a7c38156) )
	ROM_LOAD( "ss33.bin", 0x4000, 0x2000, CRC(fac4add0) SHA1(db2f72afc2cd199a881065e089e60dff4ca41a88) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "supshot1.bin", 0x0000, 0x2000, CRC(0f6aa191) SHA1(4e565d692cc0363d54ffe338c1a86a692dfab0fb) )
	ROM_LOAD( "supshot2.bin", 0x2000, 0x2000, CRC(1d1ebaf5) SHA1(fd28965bcf617378a82503788d2c4ef78971b240) )
	ROM_LOAD( "supshot3.bin", 0x4000, 0x2000, CRC(68a1c826) SHA1(872097408059a85348af312fb9273299143832fc) )
	ROM_LOAD( "sushtdl1.bin", 0x0000, 0x2000, CRC(6af8fe4a) SHA1(0c37f23afb29086aee6121a25bb7704a13933d3d) )
	ROM_LOAD( "sushtdl2.bin", 0x2000, 0x2000, CRC(972ea092) SHA1(771940505ca6ee28365f915dfa4985e89fe4e325) )
	ROM_LOAD( "sushtdl3.bin", 0x4000, 0x2000, CRC(8c5a54a6) SHA1(3fcbd70333e3050ab89a0aa30f79b73efb5980f6) )
ROM_END



ROM_START( j2supstr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supersstars1.bin", 0x0000, 0x8000, CRC(c6a924df) SHA1(fd4cac2b1c723850257a76ea5e1cdacfbbca77b6) )
	ROM_LOAD( "supersstars2.bin", 0x8000, 0x8000, CRC(25b81e75) SHA1(ec46f5ebbf156daf2c2305ce16fe24656171f0e0) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "sus6a1.bin", 0x0000, 0x8000, CRC(9824294f) SHA1(551416985b698c3bf84df23e0122c69b690631cf) )
ROM_END



ROM_START( j2swbank )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "switch_back_3_1.bin", 0x0000, 0x8000, CRC(1df40714) SHA1(8456012e968b1fb971a886eb8894c2d63c7e6890) )
	ROM_LOAD( "switch_back_3_2.bin", 0x8000, 0x8000, CRC(556e68bd) SHA1(735361def32e02f7dd50f3515bd7272bb3f2c596) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "swback31.bin", 0x0000, 0x8000, CRC(f22e831f) SHA1(211926c72af03f5757e349560dde9d8230a13db7) )
	ROM_LOAD( "switchbackp1.bin", 0x0000, 0x8000, CRC(9592715d) SHA1(ebf64f1fa645338f3d9eb11850a29f244f30234c) )
	ROM_LOAD( "switchbackp2.bin", 0x0000, 0x8000, CRC(e1829918) SHA1(7b1d0e4de1035d7ec77c8390a652dd3d174cbb1b) )
ROM_END



ROM_START( j2take2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "t21.bin", 0x0000, 0x2000, CRC(55d85e88) SHA1(62e01903b98eca51211f67408d30bb1b3a998b2a) )
	ROM_LOAD( "t22.bin", 0x2000, 0x2000, CRC(2e7a8662) SHA1(eeda34a57a7ef9a1f5ae0a9f7aead7a0c8323a4c) )
	ROM_LOAD( "t23.bin", 0x4000, 0x2000, CRC(2e7c41b9) SHA1(c748f98ef4326d961f7b9e0ed3ef8d993c2dcb91) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "take_two_2_1.bin", 0x0000, 0x2000, CRC(f8dced6d) SHA1(df4e702ad4deeeb242c2285a6149a72550888b24) )
	ROM_LOAD( "take_two_2_2.bin", 0x2000, 0x2000, CRC(f86857bb) SHA1(10cb6592f74cbf107b7277ee91a1a70fe3817d9d) )
	ROM_LOAD( "take_two_2_3.bin", 0x4000, 0x2000, CRC(219dd63f) SHA1(421704401a7f6a3c1dbcf6fb33b10869f3a53192) )
ROM_END


ROM_START( j2topsht )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "topshot.bin", 0x0000, 0x6000, CRC(8e27a0d0) SHA1(9c93b675f52d3d8ab191c36275b55f3f6d946f39) ) /* split */
ROM_END


ROM_START( j2westrn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "western.bin", 0x0000, 0x6000, CRC(091b7133) SHA1(a7731997b84aa34db456879bc117576d981583fb) ) /* split */
ROM_END

/* 3rd Party */



ROM_START( j2always )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a82p1.bin", 0x0000, 0x8000, CRC(c4599731) SHA1(4d30fb23009adc85a6ef3f81b7d17d61a2f49286) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "a82p2.bin", 0x0000, 0x8000, CRC(ac96493f) SHA1(bac4f123c5bd453b54370d769d88e8e94a54243e) )
ROM_END



ROM_START( j2blustr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs1.bin", 0x0000, 0x2000, CRC(d6c13e1b) SHA1(d41a63887cd3dc61886aad6fe607874ffa48789d) )
	ROM_LOAD( "bs2.bin", 0x2000, 0x2000, CRC(10ce9682) SHA1(fc5e2f31f1ce7c0b2cdbb05771aecfd8852f4bb5) )
	ROM_LOAD( "bs3.bin", 0x4000, 0x2000, CRC(8a48f588) SHA1(b15bdcf5f221107ba1a44f23c70e875099eace45) )
ROM_END



ROM_START( j2bonanz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bonanza.bin", 0x0000, 0x6000, CRC(0968c8dc) SHA1(34590800016d91e93f2ce0a2e544ee38780cec6e) ) /* split */
ROM_END



ROM_START( j2cshalm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashalarm1.bin", 0x0000, 0x2000, CRC(43086157) SHA1(5708f8228ab0a9e5ec3a2cb2028b6ab9d2aaf1dd) )
	ROM_LOAD( "cashalarm2.bin", 0x2000, 0x2000, CRC(6d038d86) SHA1(1986f37b7ace12f4ba4099a50c2997d23ad6a887) )
	ROM_LOAD( "cashalarm3.bin", 0x4000, 0x2000, CRC(60454c43) SHA1(22107b3e7b69e44ba2abc7ed95ade5861515d8ea) )
ROM_END



ROM_START( j2cshcrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashcards1.bin", 0x0000, 0x2000, CRC(4fb12327) SHA1(5a946d368311a14cc34c7b0b80eeed077856ebe2) )
	ROM_LOAD( "cashcards2.bin", 0x2000, 0x2000, CRC(b8627f75) SHA1(a2a1c93448ea1654d67ef9efcd7cdfb430077122) )
	ROM_LOAD( "cashcards3.bin", 0x4000, 0x2000, CRC(e4f1161c) SHA1(c54975a8c4ab48adbb8098adbab92d87d9338ca0) )
ROM_END



ROM_START( j2cshfil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashfilla10p2.40 23.1.89.bin", 0x0000, 0x8000, CRC(6dc7225d) SHA1(6aea39871fdf92b520e86a8ba71df41d2ef72689) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashfilla10p4.80 23.1.89.bin", 0x0000, 0x8000, CRC(c007fcb8) SHA1(c704c32f141b94019afea8c33dd78374fbb1e808) )
	ROM_LOAD( "cashfilla2.bin", 0x0000, 0x8000, CRC(d52a2c5d) SHA1(b97c099ef145ebf3ba6aded0b4efa910dfd83568) )
	ROM_LOAD( "cashfilla2p2.40 23.1.89.bin", 0x0000, 0x8000, CRC(d52a2c5d) SHA1(b97c099ef145ebf3ba6aded0b4efa910dfd83568) )
	ROM_LOAD( "cashfilla5p2.40 23.1.89.bin", 0x0000, 0x8000, CRC(2ec712b7) SHA1(24b85b46dad08b1cd4512f141523cdbfa7f3bd10) )
	ROM_LOAD( "cf.bin", 0x0000, 0x8000, CRC(e097baa6) SHA1(60d8cd01c879d24ffbd97070f5431e8e1ca20f3f) )
ROM_END



ROM_START( j2cshnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cash_nudger_2_1.bin", 0x0000, 0x8000, CRC(1b57aa3f) SHA1(8cf10b2f76a921ae7fb67a9bdb7c9b2631c07d0d) )
ROM_END



ROM_START( j2cshsmh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashsmashp1.bin", 0x0000, 0x4000, CRC(db1c0492) SHA1(a9b66d82afcc97ca19284f8a38c35b280d5cc8ff) )
	ROM_LOAD( "cashsmashp2.bin", 0x4000, 0x4000, CRC(95e4f70e) SHA1(273392d6bb98fb955139c54bf8ca9bbb8745ac76) )
ROM_END



ROM_START( j2coinsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwb_c_shoot_2p_1c1.bin", 0x0000, 0x2000, CRC(520bae1a) SHA1(68b224fd66f9aa3529bb206f525789a9c0c8b696) )
	ROM_LOAD( "bwb_c_shoot_2p_1c2.bin", 0x2000, 0x2000, CRC(efa7eae7) SHA1(83311c9888750d54a9f027aaf8d7602968494237) )
ROM_END



ROM_START( j2criscr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ccjpot 20p.bin", 0x0000, 0x8000, CRC(8b00ab7d) SHA1(1986cb623c91272c7177402a68df1fdd0a3b1447) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "criss cross jackpot 10p 2.40.bin", 0x0000, 0x8000, CRC(fa12be2b) SHA1(3edd68b80e3694542c12fdd031bc2dc9dbe9f925) )
	ROM_LOAD( "criss cross jackpot 10p 4.80.bin", 0x0000, 0x8000, CRC(dfe8b1fa) SHA1(a16d1e9b28d902f8bd4ef7dca08c9e79ca9385ea) )
	ROM_LOAD( "criss cross jackpot 10p all cash 2.40.bin", 0x0000, 0x8000, CRC(fa12be2b) SHA1(3edd68b80e3694542c12fdd031bc2dc9dbe9f925) )
	ROM_LOAD( "criss cross jackpot 20p 4.80 p1 15.2.91.bin", 0x0000, 0x8000, CRC(85ac71fe) SHA1(816d09b9fe7eec4bc4bda0c809100f2df29efe2d) )
	ROM_LOAD( "criss cross jackpot 20p all cash 4.80.bin", 0x0000, 0x8000, CRC(44494281) SHA1(23721853cf279376743e7dd95ae9956e4fbd80a4) )
	ROM_LOAD( "crisscrossjackpot.bin", 0x0000, 0x8000, CRC(3936b6a7) SHA1(c5ccb519acc8022ffb663ce6ddee71856b84f23a) )
ROM_END



ROM_START( j2fivepn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5pnup1", 0x0000, 0x8000, CRC(a41261c5) SHA1(01c13d0340fb64d442f10c16de09d80472339979) )
	ROM_LOAD( "5pnup2", 0x8000, 0x8000, CRC(acee5823) SHA1(954af36b8e2c3ededbe26aaeb7bd5a7338d84966) )
ROM_END



ROM_START( j2frucnx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fruit connexion 2p 2.00.bin", 0x0000, 0x8000, CRC(9d02ff13) SHA1(d905406da87fb66b906e892a9fabd9eb02fd84ce) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "fruit connexion 10p 4.80 7.11.89.bin", 0x0000, 0x8000, CRC(a0b34ba1) SHA1(7af4e9fe4c49eb4a129709485fed5cb685b5b487) )
	ROM_LOAD( "fruit connexion 2p 2.40 7.11.89.bin", 0x0000, 0x8000, CRC(8666298a) SHA1(ac41d4c66e344a8aafdc94f7d9afb67342516f56) )
	ROM_LOAD( "fruit connexion 5p 2.00.bin", 0x0000, 0x8000, CRC(0d05a2b1) SHA1(5f80f6b032078e6ad595b5494b150540dc65cf7b) )
	ROM_LOAD( "fruit connexion 5p 2.40 7.11.89.bin", 0x0000, 0x8000, CRC(e907092a) SHA1(25cf4d399e37b4c1bc394f5d1a3d2dcd2dce1d92) )
ROM_END



ROM_START( j2hilocl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hiloclimber9_1.bin", 0x0000, 0x8000, CRC(e6d25474) SHA1(2904d8c1593b033f1808a57cfca33ae0e01399b4) )
ROM_END



ROM_START( j2hitmon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hit_money_2p_p1.bin", 0x0000, 0x2000, CRC(737a4bd6) SHA1(4269bacd8921a689a6340b8614bf559ad00e7b4d) )
	ROM_LOAD( "hit_money_2p_p2.bin", 0x2000, 0x2000, CRC(10b0c8ef) SHA1(85a108f2c47f1fdc32397a1b686a90e9b00f1125) )//50p as well
	ROM_LOAD( "hit_money_2p_p3.bin", 0x4000, 0x2000, CRC(4e6cadb6) SHA1(564fe7eb169f7b76dc387a4bd845c2be806ab6a2) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "hitmoney_2p2_std_p1_1payout.bin", 0x0000, 0x2000, CRC(80ec414d) SHA1(a464eb5c9a7853ea830e2c553918924f37a73efe) )
	ROM_LOAD( "hitmoney_2p2_std_p2.bin", 0x0000, 0x2000, CRC(ca1558f3) SHA1(ab9a5500fd71478039b113111d3685053385a043) )
	ROM_LOAD( "hitmoney_2p2_std_p3.bin", 0x0000, 0x2000, CRC(4e6cadb6) SHA1(564fe7eb169f7b76dc387a4bd845c2be806ab6a2) )
	ROM_LOAD( "hitmoney_p1_50p.bin", 0x0000, 0x2000, CRC(4523ecaf) SHA1(a4f04e6b59b3c91c902e0e13ab10ed01df286bec) )
	ROM_LOAD( "hitmoney_p2_50p.bin", 0x0000, 0x2000, CRC(10b0c8ef) SHA1(85a108f2c47f1fdc32397a1b686a90e9b00f1125) )
	ROM_LOAD( "hitmoney_p3_50p.bin", 0x0000, 0x2000, CRC(f1428c4e) SHA1(05bf2d341fc832e541a6d9f03ae59691d3f37b56) )
ROM_END



ROM_START( j2penny )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcp-pennypound2p-p1.bin", 0x0000, 0x4000, CRC(0fcf5bb7) SHA1(769a9f62d557651a40d6a243be1b21cbacdb0742) )
	ROM_LOAD( "pcp-pennypound2p-p2.bin", 0x4000, 0x4000, CRC(af71c444) SHA1(b08983ef5dbd3fcb0a140538e0197804746ee772) )
ROM_END



ROM_START( j2litnot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "liteanoteclub1.bin", 0x0000, 0x4000, CRC(c15703ae) SHA1(11fb1da82d945010033592732bcf2508d7b1f084) )
	ROM_LOAD( "liteanoteclub2.bin", 0x4000, 0x4000, CRC(84c950e0) SHA1(8116455001ddc7caed56669a2de300ee1fa96d46) )
ROM_END



ROM_START( j2maxima )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "maximap1.bin", 0x0000, 0x2000, CRC(6db4256b) SHA1(788859ae4e07da971e27a89aa18884e26aff3a96) )
	ROM_LOAD( "maximap2.bin", 0x2000, 0x2000, CRC(2863f339) SHA1(5820af1114c923f81d26ef332d6b6652a5507a77) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "pcpmaxp1.bin", 0x0000, 0x2000, CRC(630c9038) SHA1(264a09f520b3bb743db470bffa9534671713a8e8) )
	ROM_LOAD( "pcpmaxp2.bin", 0x2000, 0x2000, CRC(409cc405) SHA1(df6cdc98ed994fc5514794cf5effdba2aafefbaf) )
ROM_END



ROM_START( j2missis )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mississippigambler2.1", 0x0000, 0x8000, CRC(40695e9c) SHA1(4f71004f8a1550dd1245902dc06b0c4d91b14d71) )
ROM_END



ROM_START( j2montrp )//PCP conversion of Bank Raid cabinet
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtrap_p1.bin", 0x0000, 0x2000, CRC(8ae769ee) SHA1(ce738d817c040e3d2563c1a0af0f416aef12d465) )
	ROM_LOAD( "mtrap_p2.bin", 0x2000, 0x2000, CRC(97861334) SHA1(05b63738884357ce91fb5dc9872ef3bc105b255c) )
	ROM_LOAD( "mtrap_p3.bin", 0x4000, 0x2000, CRC(febb605c) SHA1(855e20f886b59d1884307b6230a12ae4dd06f8b7) )
ROM_END



ROM_START( j2nolimt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nln4_0a.bin", 0x0000, 0x8000, CRC(0c35c938) SHA1(73b2f5a0e5d1fbb539e03bd85ccada160b09b29c) )
	ROM_LOAD( "nln4_0b.bin", 0x8000, 0x8000, CRC(1f53f64d) SHA1(c5b4b1e364bedd6174727fdee2f7608125a3f042) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	// check if this is the same set.
	ROM_LOAD( "nlnu58p1", 0x0000, 0x010000, CRC(12f9c194) SHA1(14fbee0eff87000aca68b0e3655d11da62337b64) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "nlnu58p2", 0x0000, 0x010000, CRC(ddf52210) SHA1(8783807698e067e1c6fe09c6ab925ed61706f504) ) // 1ST AND 2ND HALF IDENTICAL
ROM_END



ROM_START( j2nudfev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwb_nudge_fever_2p_p1.bin", 0x0000, 0x4000, CRC(11c09d8a) SHA1(8f92be53623048abb1865f514f547039ac6e9c2b) )
	ROM_LOAD( "bwb_nudge_fever_2p_p2.bin", 0x4000, 0x4000, CRC(35e7f8e9) SHA1(51fc321f869bc48592a7f75e11425cd7ffa84c23) )
ROM_END



ROM_START( j2nudmon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudge money 2p-2 15.12.99 p1.bin", 0x0000, 0x4000, CRC(0afd836b) SHA1(3656d0ff3c60392f3819ebed05ffc9732553500f) )
	ROM_LOAD( "nudge money 2p-2 15.12.99 p2.bin", 0x2000, 0x4000, CRC(e7e08048) SHA1(59580adcdb8bd92c29ed5e9b1f37f631b048f15e) )
	ROM_LOAD( "nudge money 2p-2 15.12.99 p3.bin", 0x4000, 0x4000, CRC(010cc7b8) SHA1(0082f2f46b7668f8ee5b24e719ff65079074fbec) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "pcp_nudge_money_2p_p1.bin", 0x0000, 0x2000, CRC(ed681bbb) SHA1(55b32e5814afcf0e1e6016d43a33078d2370c27f) )
	ROM_LOAD( "pcp_nudge_money_2p_p2.bin", 0x0000, 0x2000, CRC(95de0be8) SHA1(317bb17191256d8cabb5e214fd52ee4925df849e) )
	ROM_LOAD( "pcp_nudge_money_2p_p3.bin", 0x0000, 0x2000, CRC(bd842821) SHA1(cc4c7de3d0e0bb399a27f1862a69cfab1f6f1e5c) )
ROM_END



ROM_START( j2paypkt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "paypack1.bin", 0x0000, 0x2000, CRC(b7f678c7) SHA1(0ff618a899c597fc20e25aba93a01f7cae589ddd) )
	ROM_LOAD( "paypack2.bin", 0x2000, 0x2000, CRC(31678673) SHA1(8739c63dc56569a5e0fd2e2945605d3ad822c098) )
	ROM_LOAD( "paypack3.bin", 0x4000, 0x2000, CRC(3df391e4) SHA1(e5905ce9035c4cfa7e4b4e0daf9fcbc35e06c4d3) )
ROM_END






ROM_START( j2silvcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pcp_s_classic_p1.bin", 0x0000, 0x2000, CRC(adc83da7) SHA1(af9dd67d3f285fd4aa265e71f06e2a1ff8613779) )
	ROM_LOAD( "pcp_s_classic_p2.bin", 0x2000, 0x2000, CRC(edfc50a9) SHA1(55a348d66ae70ee246496523ebaeeb6dcb3c8c83) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "silver_classic_rom1.bin", 0x0000, 0x2000, CRC(d5a28bd8) SHA1(b364fdebe94fd10ee245f61350587eb7aa91e6c5) )
	ROM_LOAD( "silver_classic_rom2.bin", 0x2000, 0x2000, CRC(82530d61) SHA1(2c918406aba59d3dc5d946bb04d5ff9e9678317d) )
ROM_END



ROM_START( j2silvsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "silver_shot_rom1.bin", 0x0000, 0x2000, CRC(4e02769b) SHA1(8a99d7786fd8e38a857d72ce10ebd8247351fe53) )
	ROM_LOAD( "silver_shot_rom2.bin", 0x2000, 0x2000, CRC(4181e0cf) SHA1(3d28771114a1fbb7eaa3304c5b38b94e9a4f6642) )
ROM_END



ROM_START( j2strk10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strkten1.bin", 0x0000, 0x4000, CRC(63f6d62b) SHA1(815683b253e5e8c3f4f974b3fea9290e021ba0d8) )
	ROM_LOAD( "strkten2.bin", 0x4000, 0x4000, CRC(48aa4307) SHA1(36dd635459e0aef51a42a6b9d8312ab36fcef878) )
	ROM_LOAD( "strkten3.bin", 0x8000, 0x4000, CRC(bef62dc7) SHA1(e4927998793b3b1ef86b8aec7f77e958a7ce40a8) )
ROM_END



ROM_START( j2sstrea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supa_streak_5p_p1.bin", 0x0000, 0x2000, CRC(255b7348) SHA1(84f48aa420a079bf8e892b961d8f77864a77b72e) )
	ROM_LOAD( "supa_streak_5p_p2.bin", 0x2000, 0x2000, CRC(892be10e) SHA1(56391bba261e52498f1af8456f0e878dbd14c9a6) )
	ROM_LOAD( "supa_streak_5p_p3.bin", 0x4000, 0x2000, CRC(9e8c94fd) SHA1(076ab984cc6b2c63b38f58ca4986b704b5f64973) )
ROM_END



ROM_START( j2supchy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supcherry1.bin", 0x0000, 0x4000, CRC(955bbdfb) SHA1(4f0870e1fb07301bdf6620842931aa3f5d2dac16) )
	ROM_LOAD( "supcherry2.bin", 0x4000, 0x2000, CRC(d61d1d3d) SHA1(3308254dd5cfca154aa5877c89541dc983185775) )
ROM_END



ROM_START( j2tstplt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "testpilot2p.1", 0x0000, 0x4000, CRC(22292c10) SHA1(95db5303567880afe23ce37c009c11a245ea66b0) )
	ROM_LOAD( "testpilot2p.2", 0x4000, 0x4000, CRC(f02ff88e) SHA1(5b89fae391b05059b88a8fd85c28178cfd71a7f2) )
ROM_END

ROM_START( j2tstplta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "testpilot2p.1", 0x0000, 0x4000, CRC(22292c10) SHA1(95db5303567880afe23ce37c009c11a245ea66b0) )
	ROM_LOAD( "testpilotp2game.bin", 0x4000, 0x4000, CRC(33d4d441) SHA1(713501a0b3e5f712d315cd3405c4ea4918213704) )
ROM_END


ROM_START( j2trail )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwb_trailblazer.bin", 0x0000, 0x8000, CRC(a53fbb6c) SHA1(d47ec7d7df4443915022113349d4eae56df22892) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bwb_trailblazer_2p_p1.bin", 0x0000, 0x8000, CRC(366f0305) SHA1(553062feacdf6cc1d08defaae132c6b39897ec4e) )
ROM_END



ROM_START( j2bkroll )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bank roll f28165 6.1.bin", 0x0000, 0x004000, CRC(0e016ab1) SHA1(7a5a15aee9970c0f33c4052238dbb2a4f28e9ab0) )
	ROM_LOAD( "bank roll f28180 6.2.bin", 0x4000, 0x004000, CRC(87482480) SHA1(87d592428f32fc7119c3f22fc82c76ab16215502) )
	ROM_LOAD( "bank roll f28196 6.3.bin", 0x8000, 0x004000, CRC(6db9539c) SHA1(4deaa4d4b7cce1c35372cabc77d463142c0dca83) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bank roll 6.a1.bin", 0x0000, 0x004000, CRC(33122fad) SHA1(99d65166058f6d15de3e0f0a3bf5c8f06139a708) )
	ROM_LOAD( "bank roll 6.p1.bin", 0x0000, 0x004000, CRC(0479f4cb) SHA1(d5e8015b9611fb1da7d49f05336ee894b0da1017) )
	ROM_LOAD( "bank roll 6.x1.bin", 0x0000, 0x004000, CRC(1af05645) SHA1(307cd8b7f6a06e8b0d12b95638d2113e456507d7) )
	ROM_LOAD( "bank roll 6.y1.bin", 0x0000, 0x004000, CRC(90ae3e66) SHA1(f07602dfb9b4f15639ec06f54d7a0ca916543268) )
	ROM_LOAD( "bnkrol61.bin", 0x0000, 0x004000, CRC(2298cbcc) SHA1(6ff074665cd1c2da3c17b88b987c7e6e165dd504) )
	ROM_LOAD( "bnkrol62.bin", 0x0000, 0x004000, CRC(374872aa) SHA1(ce49edd745cd7d4eacc07ad02522db02d6f4b315) )
	ROM_LOAD( "bnkrol63.bin", 0x0000, 0x004000, CRC(0f722140) SHA1(ca1b6821f31981d17ca3d6aca70decadbf2a7dc9) )
ROM_END




ROM_START( j2bodym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bodymatch6.1.bin", 0x0000, 0x004000, CRC(6c133082) SHA1(4294ec6dccc7864eb6b957f820d91b5426414e42) )
ROM_END



ROM_START( j2cvault )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashvault b15236 1.1 x.bin", 0x0000, 0x004000, CRC(ce5abe6f) SHA1(6682f8fc94ba1128cc5dce95ac476820c0cdf2df) )
	ROM_LOAD( "cashvault b15244 1.2.bin", 0x4000, 0x004000, CRC(b72ab676) SHA1(9b0c51e9adf374f3ac72d1abee1f3b48599bfe17) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "cashvault 1.1.bin", 0x0000, 0x004000, CRC(daab829b) SHA1(59dd52c375b5870c7355630db25ee01291328741) )
	ROM_LOAD( "cashvault b11416 1a1.bin", 0x0000, 0x004000, CRC(e7b8c787) SHA1(ae71302c5a69a2bc982c1915e1a8ded86d04eb52) )
	ROM_LOAD( "cashvault b11438 1p1.bin", 0x0000, 0x004000, CRC(d0d31ce1) SHA1(c3858e6b2f6bdfe55cf762d131484e393b79a2a0) )
	ROM_LOAD( "cashvault b15232 1.1 y.bin", 0x0000, 0x004000, CRC(f349fb73) SHA1(b2ccdfefaa05075791579d9af711991f8c44a93b) )
ROM_END



ROM_START( j2cashab ) // check
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10p_cashablanca_v0_1.bin", 0x0000, 0x008000, CRC(ca645972) SHA1(3785e37d96d30112135146d17d3f8332aa120b71) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "2p_cashablanca.bin", 0x0000, 0x008000, CRC(a736a0e0) SHA1(2e295ee5501322eed1f00027c477bb4ea57b5c67) )
	ROM_LOAD( "2p_cashablanca_v01.bin", 0x0000, 0x008000, CRC(53f06670) SHA1(4f60f9715d648525796a55c587d6ee1d54a1cbde) )
	ROM_LOAD( "5p_cashablanca.bin", 0x0000, 0x008000, CRC(126d558a) SHA1(09c866dfe6c23e328252ab0e3f94788a0cfce3f7) )
	ROM_LOAD( "casha blanca 20p.bin", 0x0000, 0x008000, CRC(81d85698) SHA1(343f5f1d251220287167ae4e600e200450236f39) )
	ROM_LOAD( "casha blanca 2p.bin", 0x0000, 0x008000, CRC(53f06670) SHA1(4f60f9715d648525796a55c587d6ee1d54a1cbde) )
	ROM_LOAD( "cashablanca_5p_v0_1.bin", 0x0000, 0x008000, CRC(95d25467) SHA1(b77bb86e1ecab264bcae99bc02bb0370a4529081) )
ROM_END



ROM_START( j2cashry ) // check
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cashino_2p_p1.bin", 0x0000, 0x002000, CRC(f7747303) SHA1(eb3a422c91e53dd9e5c155b5972de04b4b2c87e7) )
	ROM_LOAD( "cashino_2p_p2.bin", 0x2000, 0x002000, CRC(b41fbfaa) SHA1(c69d83f456457e6713ca6e494e16d028e51b0094) )
	ROM_LOAD( "cashino_2p_p3.bin", 0x4000, 0x002000, CRC(7c8ac23d) SHA1(66309774cc53151602f6c804fed1d090b5841e36) )
ROM_END



ROM_START( j2casino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "casinoclassic10p100p1.bin", 0x0000, 0x008000, CRC(1a100dd8) SHA1(52bf7e5e1ad5e3ef8688a2ea9388917e4eee1663) )
	ROM_LOAD( "casinoclassic5p60.bin", 0x0000, 0x008000, CRC(e9a04179) SHA1(94e541e09ebe48c73816bda2601c1270ace78131) )
	ROM_LOAD( "casinoclassic5p80.bin", 0x0000, 0x008000, CRC(1b4e3d6f) SHA1(c080def1ceab53c0be72b76c23d69aa7faf34d2b) )
ROM_END


ROM_START( j2coinct )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpmcoincount1.1.bin", 0x0000, 0x002000, CRC(1ab03e2f) SHA1(d96222bedcd8c4823dae3901ed08b3ca2dbd30b7) )
	ROM_LOAD( "jpmcoincount1.2.bin", 0x2000, 0x002000, CRC(1a459843) SHA1(0a9d51aa3878a54adc3ce417d7448a33d5f96983) )
	ROM_LOAD( "jpmcoincount1.3.bin", 0x4000, 0x004000, CRC(8d838f4e) SHA1(93425bdb8d96ee564b9fe75ebc6fc02ae1a4b01d) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "jpmcoincount1.bin", 0x0000, 0x004000, CRC(15a4a2ae) SHA1(23259bada9595080e2d7e16c25d5e4b4e83ec47f) )
	ROM_LOAD( "jpmcoincount2.bin", 0x0000, 0x004000, CRC(0e59ad8a) SHA1(30ff26855be6af1bbd97c6d7debd2ed4b545bcfc) )
ROM_END







ROM_START( j2loots )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "loots1.1", 0x0000, 0x002000, CRC(9a9a152e) SHA1(e0e49d4fcd057341b12b3af6441a1f078b6e074e) )
	ROM_LOAD( "loots1.2", 0x2000, 0x002000, CRC(d3e86e5f) SHA1(c405766c9076fb76a76136537990c48a81260ebb) )
	ROM_LOAD( "loots1.3", 0x4000, 0x002000, CRC(759ed0f4) SHA1(2869860705d30514883220f016d5a554b997ecda) )
ROM_END



ROM_START( j2match )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "match21.bin", 0x0000, 0x008000, CRC(ffa67832) SHA1(2129b2e19d1f028ad0654ecb2b0ce47ffe658780) )
ROM_END



ROM_START( j2monbnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moneybands10p1.1.128k.bin", 0x0000, 0x004000, CRC(7147e491) SHA1(e7161644453c79a0a35daac577844107235e54fd) )
	ROM_LOAD( "moneybands10p1.2.128k.bin", 0x4000, 0x004000, CRC(1034a3bd) SHA1(60196b5b825d7a979e5d981369d50e836e023d34) )
ROM_END



ROM_START( j2monmtx ) // check
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mmatrix1.rom", 0x0000, 0x002000, CRC(1ea0872c) SHA1(9a1ab3570baf71204542242427721d091ebeab56) )
	ROM_LOAD( "mmatrix2.rom", 0x2000, 0x002000, CRC(1cbffdba) SHA1(dc07e305e99949c670a1946308337c8fbf487dc5) )
	ROM_LOAD( "mmatrix3.rom", 0x4000, 0x002000, CRC(77a3ecd2) SHA1(890f52b17e227bd422dced159ad79a947ef689b9) )
ROM_END



ROM_START( j2tst )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mps1tb.bin", 0x0000, 0x002000, CRC(7d110b7e) SHA1(774d7f4343e8ce7cd99a98e237d293d6028ec9c6) )
ROM_END




ROM_START( j2nudnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudge_nudge2_mps.bin", 0x0000, 0x004000, CRC(839addc8) SHA1(4faf5535a57accb95de699737fe7b9290d98df73) ) /* data only at 0x2000-0x3fff */
	ROM_LOAD( "nudge_nudge1_mps.bin", 0x0000, 0x002000, CRC(c6f03c97) SHA1(e1cd16c9b2773bb01e6681e2300fb08b5727151a) )
ROM_END


ROM_START( j2pinac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pinnacle4.1", 0x0000, 0x008000, CRC(1c24a9e9) SHA1(da9e7555f405d2506e8cb2fad7595c5e6c08f659) )
	ROM_LOAD( "pinnacle4.2", 0x8000, 0x008000, CRC(ca86ee0c) SHA1(8857961f14ec7822985938a7758037c39f6540bd) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "pinnacle_3_1.bin", 0x0000, 0x008000, CRC(e41c6510) SHA1(ae2ae4c1c614e31f30269c85e4bef529a44f0529) )
	ROM_LOAD( "pinnacle_3_2.bin", 0x0000, 0x008000, CRC(2dcc17d4) SHA1(9193b91d64cbcf44da7df0bf3ddb80c25408ac84) )
ROM_END


ROM_START( j2pinclb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pinn_21.bin", 0x0000, 0x008000, CRC(421986d1) SHA1(944a5f4c8da50cc58cc0d822685619502299962a) )
	ROM_LOAD( "pinn_22.bin", 0x8000, 0x008000, CRC(4c3b8a34) SHA1(f3ef7b90078fcbb7a5ef9e65b997c52cddabf6b7) )
ROM_END






ROM_START( j2reelcz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpm_reel_crazy_5p_2_p21.bin", 0x0000, 0x008000, CRC(48b6f8e1) SHA1(837ddac056a88b6edb8479289b5180525a4ff5ff) )
	ROM_LOAD( "jpm_reel_crazy_5p_2_p22.bin", 0x8000, 0x008000, CRC(9dbb6d3f) SHA1(6f4b0b20e51fde445648419aeca659fb05aedf1a) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "rc71.bin", 0x0000, 0x002000, CRC(567c31f3) SHA1(2ed35da0d673e372ac515527d51be731be341394) )
	ROM_LOAD( "rc72.bin", 0x0000, 0x002000, CRC(cf23edea) SHA1(833db58e2ca2dd759df796ee22ff5890a1531d7b) )
	ROM_LOAD( "rc73.bin", 0x0000, 0x002000, CRC(044f8143) SHA1(e0c34981e5df2536d2598d93ab7d41418883b88b) )
	ROM_LOAD( "reel_crazy_2p_1_1.bin", 0x0000, 0x008000, CRC(6925c78e) SHA1(db95bd4a0363f81d44643ec8f5e916529bb3a0ec) )
	ROM_LOAD( "reel_crazy_2p_1_2.bin", 0x0000, 0x008000, CRC(9a09ccef) SHA1(f0e5cdbafcf99d8666cfb5cefe5fe3ab0b9fae51) )
	ROM_LOAD( "reelcrazy10p2.1.bin", 0x0000, 0x008000, CRC(3d0e1f38) SHA1(ae80d0297a6ce107c8ca4b6a99b9e2f87fe9d332) )
	ROM_LOAD( "reelcrazy10p2.2.bin", 0x0000, 0x008000, CRC(01b6855b) SHA1(c70719de2e8ef1b080b77df52dcc720f7a8c8f66) )
	ROM_LOAD( "reelcrazy20p2.1.bin", 0x0000, 0x008000, CRC(2d5ece47) SHA1(919ec40b2bdf0ec4f45eac6b6d324e55c24c849c) )
	ROM_LOAD( "reelcrazy20p2.2.bin", 0x0000, 0x008000, CRC(e5e20e4d) SHA1(d5ba7f6cdb748afa7089da7313fa518c40677372) )
	ROM_LOAD( "reelcrazy2p_rom1.bin", 0x0000, 0x008000, CRC(855246f5) SHA1(d36893290c4917a5652d4a9282ef3faf5a1ae8be) )
	ROM_LOAD( "reelcrazy5p2.1.bin", 0x0000, 0x008000, CRC(3cfbcd70) SHA1(a3a64d05b73e7351b2bc7b03e79f414b1bbb7820) )
	ROM_LOAD( "reelcrazy5p2.2.bin", 0x0000, 0x008000, CRC(e2fc02b7) SHA1(0c93f63091083c59da5b080950ebe13abbd8fda1) )
	ROM_LOAD( "reelcrz1.bin", 0x0000, 0x002000, CRC(35b90d03) SHA1(00ebbe2e28bfff3acf5a4118dae0963fa8b38d47) )
	ROM_LOAD( "reelcrz2.bin", 0x0000, 0x002000, CRC(e9d85c2a) SHA1(09ff2b33e1d0602e7374c176eae916ab22382008) )
	ROM_LOAD( "reelcrz3.bin", 0x0000, 0x002000, CRC(d7e676cb) SHA1(66ff74a07b2e7f514412a691a66ae080547c47b8) )
	ROM_LOAD( "rlcrzdl1.bin", 0x0000, 0x002000, CRC(98573078) SHA1(473bca55b59709f5df5fb65ca30e6792b98d3567) )
	ROM_LOAD( "rlcrzdl2.bin", 0x0000, 0x002000, CRC(ad78ec14) SHA1(98043892d769034eb4f9918d37f9ca626d770375) )
	ROM_LOAD( "rlcrzdl3.bin", 0x0000, 0x002000, CRC(9dd8f38c) SHA1(5172c8d677be17342b82339c891a9adcebfbfaeb) )
ROM_END



ROM_START( j2reeldc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "reeldeal1-1.bin", 0x0000, 0x008000, CRC(0efce06d) SHA1(94b885ae2b0b8784a5bad32e8b3a3f04c7e3c81a) )
	ROM_LOAD( "reeldeal1-2.bin", 0x8000, 0x008000, CRC(2e02163d) SHA1(00fcf5544d1fc7bd303069e949a12d45987e62d5) )
ROM_END









ROM_START( j2topcd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwb10ptopcardic1.bin", 0x0000, 0x002000, CRC(4fb26539) SHA1(7d31dbedc0174fb2205176bb0ce59ee687264b39) )
	ROM_LOAD( "bwb10ptopcardic2.bin", 0x2000, 0x002000, CRC(2ffdc415) SHA1(969825e14bd372131b8064a425f1b714200036cf) )

	ROM_REGION( 0x10000, "altrevs", 0 ) /* split later */
	ROM_LOAD( "bwb10ptopcardic1alt.bin", 0x0000, 0x002000, CRC(300a7a95) SHA1(7420034f5b65d9ed41b736dfc6872f6193767621) )
ROM_END


ROM_START( j2xxx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triplex_5p2_ic1.bin", 0x0000, 0x002000, CRC(c2d13519) SHA1(560e7d92a0e3c42ee24db5bc654a80c4265e572c) )
	ROM_LOAD( "triplex_5p2_ic2.bin", 0x2000, 0x002000, CRC(6869cae5) SHA1(b72f66a5ba43b2894a9dd5ee121d7e52bd68fc26) )
ROM_END


ROM_START( j2tupnud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tupnud1.bin", 0x0000, 0x004000, CRC(16bb80a7) SHA1(64a32792487648d3d96cad7ff913f417f31d79f9) )
	ROM_LOAD( "tupnud2.bin", 0x4000, 0x004000, CRC(bf2abbed) SHA1(fece694dc04cb5d08e48b34560454242d4b3fdd7) )
ROM_END


ROM_START( j2wrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7wildreelbingo.1", 0x0000, 0x004000, CRC(1af6cc94) SHA1(a3b1f608665cfecaaa3a852d15085767c3a6f616) )
ROM_END




ROM_START( j2ncsp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_bwbncsp0pp_rom1.bin", 0x0000, 0x004000, CRC(5a33bf9f) SHA1(156e7afec7983d7aa7922f522c9b55579b39a35e) )
	ROM_LOAD( "unk_bwbncsp0pp_rom2.bin", 0x4000, 0x004000, CRC(69d740ae) SHA1(a946f9d7de4af0567f333cded4f699f594c1da07) )
ROM_END


ROM_START( j2nrrp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_bwbnprpopp_rom1.bin", 0x0000, 0x004000, CRC(6e15ac48) SHA1(e07246a1559abfee39b9c5ff9d4fc4c188e52d94) )
	ROM_LOAD( "unk_bwbnprpopp_rom2.bin", 0x4000, 0x004000, CRC(fbd4d07f) SHA1(4590157f5b48bcbe7dabbf08cf60ad544dcb6c1a) )
ROM_END


ROM_START( j2nsc15 ) // check
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_pcpnsc15_rom1.bin", 0x0000, 0x004000, CRC(01b46c7e) SHA1(c619921e123ae554caccfc6872498e361fd15a10) )
	ROM_LOAD( "unk_pcpnsc15_rom2.bin", 0x4000, 0x004000, CRC(e7e4d7d2) SHA1(64d5adcc3a99b517793becee267b4c00d22fe3e3) )
ROM_END


ROM_START( j2nsw12 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_pcpnsw12.bin", 0x0000, 0x008000, CRC(113953d0) SHA1(034b2e23af6102feede26fff9616205cd94ab058) )
ROM_END





ROM_START( j2club77 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club77.1", 0x0000, 0x008000, CRC(64366cab) SHA1(5cf60e81c782ae829b7f6f22e1cd0e821a25cc1a) )
ROM_END

ROM_START( j2contnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ml1", 0x0000, 0x008000, CRC(910e210a) SHA1(11c94a7f587aacc87b3d2038d54faade13639f79) )
	ROM_LOAD( "ml2", 0x8000, 0x008000, CRC(74b2197e) SHA1(1f9e3807276bcf9ce4ee871d0e5bf69a7642be21) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "ml.bin", 0x0000, 0x010000, CRC(0db2289a) SHA1(f59d3f358a7fb17990019f57859bebd5e723c7eb) )
	ROM_LOAD( "nl4_0a.bin", 0x0000, 0x008000, CRC(9327b2cf) SHA1(f20af45bc6ce76bd53add40a037b5ac4eb894d71) )
	ROM_LOAD( "nl4_0b.bin", 0x0000, 0x008000, CRC(396cdc04) SHA1(b962de8e73df14820d3483d02e5f0ab5e89f58a5) )
ROM_END

ROM_START( j2crown )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crown_dealer_2_1.bin", 0x0000, 0x008000, CRC(95fdbbb9) SHA1(f26e80dea91a6387d2e3e098b625dae5a6d23ef6) )
ROM_END




ROM_START( j2hirola )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hiroll11.bin", 0x0000, 0x008000, CRC(e1cb4cdf) SHA1(035a858c0dd1f3e3419d26f67c60748dbbf836f2) )
	ROM_LOAD( "hiroll12.bin", 0x8000, 0x008000, CRC(ad778c26) SHA1(d5ed42fc47d9f1926778a142552a5712956e6fbf) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "hirol1c1.bin", 0x0000, 0x008000, CRC(ef1c8ef5) SHA1(600924da8b5808b2cab69ccf654b66efa834a653) )
ROM_END




ROM_START( j2monmin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "moneymine1.bin", 0x0000, 0x002000, CRC(b566ce95) SHA1(ff2e29ddef40a9b8585877bff98b404245dd6e69) )
	ROM_LOAD( "moneymine2.bin", 0x2000, 0x002000, CRC(89cd2d0f) SHA1(1edbb62d36aa9a2d2bda200a17ab3b019bb59156) )
	ROM_LOAD( "moneymine3.bin", 0x4000, 0x002000, CRC(1a334ff8) SHA1(56d5be930a5f61aa469b99754e2be3a5f534cc5d) )
ROM_END

ROM_START( j2notspn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "notespinner1_1.bin", 0x0000, 0x004000, CRC(393b16c5) SHA1(b3eeb341a763c2dd67732a44f4b2b38e0d336fd4) )
	ROM_LOAD( "notespinner1_2.bin", 0x4000, 0x004000, CRC(ace0b721) SHA1(b0d4cce887c605e735aed6aa45cee0659c823b70) )
ROM_END

ROM_START( j2pharo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pharoah3.1", 0x0000, 0x004000, CRC(d2ec2136) SHA1(c47e8e6969683022347818d1bf8d0245b9dc3dc3) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "pharoah3.2", 0x2000, 0x002000, CRC(e072bb1f) SHA1(9836ad9642bbb0754faf2fde513003c883bf9d08) )
	ROM_LOAD( "pharoah3.3", 0x4000, 0x004000, CRC(8cf3e05e) SHA1(808234d124d50a8a7a2e47450b68158846157def) )
ROM_END




ROM_START( j2super7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supersevens3.1", 0x0000, 0x008000, CRC(a4371bed) SHA1(c92087e6f5d14541776c08ca630a93f82d3cec72) )
ROM_END

ROM_START( j2sex )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "superexchange1.1", 0x0000, 0x008000, CRC(694416f8) SHA1(08d57a92b016ca795d85de7de749110b8c574313) )
	ROM_LOAD( "superexchange1.2", 0x8000, 0x008000, CRC(c23c85a4) SHA1(27c459d3b6b8dc7d4bc447a469565ea053b4e9a5) )
ROM_END

ROM_START( j2sng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sngp1.bin", 0x0000, 0x004000, CRC(2a550867) SHA1(b8772d24947b032ca220a2329c992f218c4e516e) )
	ROM_LOAD( "sngp2.bin", 0x4000, 0x004000, CRC(9e6e1259) SHA1(7f01e950b34a4a0a41aa867d674e681297ea4e5c) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "sng5pp1", 0x0000, 0x008000, CRC(28e767ba) SHA1(82d53c50eb3eb35b6edc7b75d0f1e4a9cf1bf2c0) )
	ROM_LOAD( "sng5pp2", 0x0000, 0x008000, CRC(fd77b52a) SHA1(fd2c6c5499fad8ecbefae12db6239274f9e2cead) )
ROM_END

ROM_START( j2tupnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tn1_1.bin", 0x0000, 0x004000, CRC(b2da1dec) SHA1(999ca895d9e95255e2c381f7b17101c15c9899a1) )
	ROM_LOAD( "tn1_2.bin", 0x4000, 0x004000, CRC(9f60c9ec) SHA1(11476db7e43d9dbf1418dcfc23dc6b3eca93cbce) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "tn.bin", 0x0000, 0x008000, CRC(b292165a) SHA1(ee248285203a6a7302faa40b69199ab79b164a35) )
	ROM_LOAD( "tn89-2_1.bin", 0x0000, 0x008000, CRC(3721ce87) SHA1(dee85f9f56a85893f83846b2d366789dfa7fff0a) )
	ROM_LOAD( "tn_1986_2_1.bin", 0x0000, 0x004000, CRC(e8f104f5) SHA1(8584c6c383b0104dcade40de4e54f23fcb07b2c7) )
	ROM_LOAD( "tn_1986_2_2.bin", 0x0000, 0x004000, CRC(a1a7630d) SHA1(e31844a330efdde4f29531551891c62721fbf0db) )
	ROM_LOAD( "tn_a_1.bin", 0x0000, 0x008000, CRC(b292165a) SHA1(ee248285203a6a7302faa40b69199ab79b164a35) )
ROM_END

ROM_START( j2008 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_008_rom1.bin", 0x0000, 0x004000, CRC(819c7988) SHA1(3a903efbc32859eb5c4612314d44a38896c23f18) )
	ROM_LOAD( "unk_008_rom2.bin", 0x4000, 0x004000, CRC(e9912d5d) SHA1(c866f16bbb8cc23d103a48c44efd3549988cfb55) )
ROM_END

ROM_START( j2chsn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_chsnsn05_rom1.bin", 0x0000, 0x004000, CRC(b6314f95) SHA1(1207e29d824584a2298e7acf0d3caf906dede86e) )
	ROM_LOAD( "unk_chsnsn05_rom2.bin", 0x2000, 0x004000, CRC(c477bb66) SHA1(2112edfa81a445d025d4719c20ba5e5417476f1f) )
ROM_END

ROM_START( j2lhs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lhs875.1a", 0x0000, 0x002000, CRC(62c255e9) SHA1(17f07455fc8763537891026d13f20f37cee708d8) )
	ROM_LOAD( "lhs875.2a", 0x2000, 0x002000, CRC(2c81513a) SHA1(69cb4ddde81d89ab642556e391087a2462be62d3) )
ROM_END

ROM_START( j2nn2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_nn_2_1.bin", 0x0000, 0x002000, CRC(c6f03c97) SHA1(e1cd16c9b2773bb01e6681e2300fb08b5727151a) )
	ROM_LOAD( "unk_nn_2_2.bin", 0x2000, 0x002000, CRC(5eb8de2e) SHA1(2b43117e8bba070a16cc96dad91518a84f18705d) )
ROM_END

ROM_START( j2rm941 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unk_rm941_1a.bin", 0x0000, 0x002000, CRC(ace3affd) SHA1(0b1846a48866746a8dc1e092233be18249b4a22e) )
	ROM_LOAD( "unk_rm941_2a.bin", 0x2000, 0x002000, CRC(14a9c702) SHA1(2e46ab458deb539fd50407cae98672c8e8ef0bb0) )
ROM_END


ROM_START( j2nud5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cn_1_1.bin", 0x0000, 0x008000, CRC(e36fb53f) SHA1(f9072a8637440161c1e812b02a8325709e8c2e4c) )
	ROM_LOAD( "cn_1_2.bin", 0x8000, 0x008000, CRC(9f8ed728) SHA1(abca9d0f0c4226187102950ce53719e766044259) )
ROM_END

ROM_START( j2b7 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b7p1", 0x0000, 0x004000, CRC(65b13c64) SHA1(44de337d831c9b0e8fa8807e618ff5af74e501a7) )
	ROM_LOAD( "b7p2", 0x4000, 0x004000, CRC(a3ec9702) SHA1(2b01e8c8e010e9d8d81eaa23b4fa7da446ac598a) )
ROM_END



ROM_START( j2ss )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supastepper5p1.1", 0x0000, 0x004000, CRC(885c8509) SHA1(e727cf64a49d552d81eff15ce65af732e2d08609) )
	ROM_LOAD( "supastepper5p1.2", 0x4000, 0x004000, CRC(debf6812) SHA1(2becbd336652224139365f9273d7b27687a7ac79) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "supastepper2p2.1", 0x0000, 0x004000, CRC(7120e1a8) SHA1(7daf69bf4bae1b07bebd4b9aa8f5096816c57803) )
	ROM_LOAD( "supastepper2p2.2", 0x4000, 0x004000, CRC(60280d22) SHA1(de09d9a4edf75993ef36e309aececf8062702c11) )
ROM_END

ROM_START( j2nbz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nubnzdl1.bin", 0x0000, 0x002000, CRC(bae682d2) SHA1(650082af9210b0af8b08870a4cdf4196035ea8a5) )
	ROM_LOAD( "nubnzdl2.bin", 0x2000, 0x002000, CRC(ff150af7) SHA1(02e25200560e8435ebbf19c3ae9c3e9cf00342c1) )
	ROM_LOAD( "nubnzdl3.bin", 0x4000, 0x004000, CRC(450d7fc9) SHA1(f82acb017e765f7188a874dade6fd1a5d6b2033e) )
ROM_END

ROM_START( j2clbbin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "club bingo 3.1.bin", 0x0000, 0x008000, CRC(ad6d6c23) SHA1(247e2117e5285d1309a8a92be96ee67e46279aa6) )
ROM_END


ROM_START( j2sirich )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strikerich1_1.bin", 0x0000, 0x008000, CRC(8b3295c5) SHA1(2f228575e96e574f28627668cb979cdd58307e12) )
ROM_END

ROM_START( j2siricha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rich2_1.rom", 0x0000, 0x008000, CRC(ee75ebcb) SHA1(968b8a0bff9779681b16456f9399b0d122b1796c) )
ROM_END

ROM_START( j2wag )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bwbnwagn.bin", 0x0000, 0x008000, CRC(1163559a) SHA1(9e13b23eae478fd5b5468599b1163d06e189d446) )

	ROM_REGION( 0x10000, "altrevs", 0 )
	ROM_LOAD( "winagain.bin", 0x0000, 0x004000, CRC(193e743d) SHA1(1c2d0dc8bea80c29c012a6a43d5bcd342b2b9f2a) )
ROM_END

ROM_START( j2hcash ) // was in a 'Project' 'Hot Cash' set..
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hotcash1.bin", 0x0000, 0x008000, CRC(834a9060) SHA1(14e59e0caa85d183907843ccb8d0d6abf00c9b42) )
	ROM_LOAD( "hotcash2.bin", 0x8000, 0x008000, CRC(ee7c082c) SHA1(2f75e20755b24e5e86bfc4d5bf2b064149224d28) )
ROM_END

ROM_START( j2sset )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sunset_strip_v2_0.bin", 0x0000, 0x008000, CRC(cbd2adea) SHA1(5c7d324d690a2ef0b85f3db323a0b4608b9e59b6) )
ROM_END



GAME(198?, j2adnote ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 1)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnotea,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 2)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnoteb,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 3)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnotec,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 4)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnoted,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 5)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnotee,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 6)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnotef,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 7)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnoteg,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 8)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnoteh,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 9)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2adnotei,j2adnote   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Add A Note (JPM) (MPS, set 10)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bankch ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Bank Chase (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bankrd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Bank Raid (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bigbnk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Big Banker (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bigbox ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Big Box (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bigbuk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Big Buck$ (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bigdl  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Big Deal (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2blkchy ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Black Cherry (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashbn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Bonus Club (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashfl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Falls (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashrl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Reels (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashrv ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Reserve (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashro ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Rolls (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashtk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Track (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashtd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Track Deluxe (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2coppot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Copper Pot (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2coprun ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Copper Run (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cprndx ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Copper Run Deluxe (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2crkbnk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Crack The Bank (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2droplt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Drop The Lot (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2dropld ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Drop The Lot Deluxe (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2ewn    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Each Way Nudger (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2ews    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Each Way Shuffle (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2exec   ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Executive Club (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fasttk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Fast Trak (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fqueen ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Find The Queen (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fiveal ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Five Alive (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fiveln ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Five Liner (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fws    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Five Way Shuffle (Set 1) (JPM) (MPS)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fwsa   ,j2fws      ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Five Way Shuffle (Set 2) (JPM) (MPS)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2frmtch ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Fruit Match (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fullhs ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Full House Club (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2ghostb ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Ghostbuster (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2goldrn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Gold Run (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2goldbr ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Golden Bars (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2gldchy ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Golden Cherry (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2gldwin ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Golden Win (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hinote ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hi Note (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hiroll ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hi Roller (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hotpot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hot Pot (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hotptd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hot Pot Deluxe (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hotsht ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hot Shot Club (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hypnot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Hypernote (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2jackbr ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Jackpot Bars (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2jackdc ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Jackpot Dice (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2jokers ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Jokers (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2kingcl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","King Of Clubs (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2litean ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Lite A Nudge (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2lovsht ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Loot Shoot (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2lovshd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Loot Shoot Deluxe (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2luckar ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Lucky Arrows (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2lucky2 ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Lucky 2s (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2monblt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Money Belt (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2mongam ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Money Game (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2mongmd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Money Game Deluxe (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2multwn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Multi Win (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2notexc ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Note Exchange (Set 1) (JPM) (MPS)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2notexca,j2notexc   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Note Exchange (Set 2) (JPM) (MPS)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2notexcb,j2notexc   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Note Exchange (Set 3) (JPM) (MPS)",          MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2notesh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Note Shoot (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudbnz ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Bonanza Deluxe (Set 1) (JPM) (MPS)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudbnza,j2nudbnz   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Bonanza Deluxe (Set 2) (JPM) (MPS)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nuddup ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Double Up (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nuddud ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Double Up Deluxe (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudup3 ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Double Up MkIII (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudshf ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Shuffler (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2plsmon ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Plus Money (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2plsmnd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Plus Money Deluxe (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2plsnud ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Plus Nudge (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2potlck ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Pot Luck (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2pndrsh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Pound Rush (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2pyramd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Pyramid (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelbn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Bingo Club (Set 1) (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelbna,j2reelbn   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Bingo Club (Set 2) (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelbo ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Bonus (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelmg ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Magic (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelmgd,j2reelmg   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Magic (JPM) [Dutch] (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelmc ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Magic Club (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelmo ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Money (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2rotnot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Rota Note (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2roulcl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Roulette Club (JPM) [Mps] (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2rdclb  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Royal Deal Club (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2slvrgh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Silver Ghost (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2sldgld ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Solid Gold (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2spcrsv ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Special Reserve (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2stahed ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Streets Ahead (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supfrt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supa Fruit (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supfrc ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supa Fruit Club (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(1987, j2supsft ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supashifta (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supstp ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supa Steppa (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2suptrk ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supa Track (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2suprft ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Super Fruit (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supln  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Super Line (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2suppot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Super Pots (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2suprl  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Super Reel (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2suprsh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supershot (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supstr ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Superstars (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2swbank ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Switch Back (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2take2  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Take 2 (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2topsht ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Top Shot (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2westrn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Western (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )


GAME(198?, j2blustr ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Blue Streak (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cshalm ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Cash Alarm (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cshcrd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Cash Cards (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cshfil ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Cash-Filla (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cshsmh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Cash Smash (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2criscr ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Criss Cross Jackpot (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2frucnx ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Fruit Connexion (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hitmon ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Hit Money (Pcp) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2penny  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","In For A Penny In For A Pound (Pcp) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2maxima ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Maxima (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2montrp ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Money Trapper (Pcp) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudmon ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Nudge Money (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2paypkt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Pay Packet (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2silvcl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Silver Classic (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2silvsh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Silver Shot (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2sstrea ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Supa Streak (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2tstplt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Test Pilot (Set 1) (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2tstplta,j2tstplt   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Test Pilot (Set 2) (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2bonanz ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Eurocoin","Bonanza (Eurocoin) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2supchy ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Eurocoin","Super Cherry (Eurocoin) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2hilocl ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Crystal","Hi Lo Climber Club (Crystal) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2litnot ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Crystal","Lite A Note Club (Crystal) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2missis ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Crystal","Mississippi Gambler Club (Crystal) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2always ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Always Eight (Bwb) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2coinsh ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Coin Shoot (Bwb) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudfev ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Nudge Fever (Bwb) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2trail  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Trailblazer (Bwb) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2wag    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Win-A-Gain (Bwb) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2cshnud ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Mdm","Cash Nudger (Mdm) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2fivepn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Mdm","Fivepenny Nudger (Mdm) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nolimt ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Mdm","No Limit Nudge (Mdm) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2strk10 ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Ace?","Strike Ten (Ace) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL ) // there was another (68k based) game in this set, which makes me wonder if this one is by Ace at all




GAME(198?, j2bkroll ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Bank Roll (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2bodym  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Body Match (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cvault ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cash Vault (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashab ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Cashablanca (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2cashry ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Cashino Royale (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2casino ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Casino Classic (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2coinct ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Coin Count (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2loots  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","Loot Shoot (Pcp) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2match  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Match It (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2monbnd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Money Bands (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2monmtx ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Money Matrix (Bwb) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2tst    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","MPS 1 Test Rom (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nudnud ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Nudge (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2pinac  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Pinnacle (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2pinclb ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Pinnacle Club (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reelcz ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Crazy (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2reeldc ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Reel Deal Club (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2topcd  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Top Card (Bwb) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2xxx    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","Triple X (Bwb) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2tupnud ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Tuppenny Nudger (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2wrb    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Wild Reel Bingo (JPM) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2ncsp   ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","unknown 'ncsp0pp' (Bwb) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nrrp   ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Bwb","unknown 'nprpopp' (Bwb) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nsc15  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","unknown 'nsc15' (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nsw12  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Pcp","unknown 'nsw12' (Pcp) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2club77 ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Club 77 (Unk) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2contnd ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Mdm","Continuous Nudger (Mdm) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2crown  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Crown Dealer (Unk) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2hirola ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Hi Roll (Unk) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2monmin ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Money Mine (Unk) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2notspn ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Note Spinner (Unk) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2super7 ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Super 7's (Unk) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2sex    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Super Exchanger (Unk) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2sng    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Cotswold Microsystems","Super Nudge Gambler (Cotswold Microsystems) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2tupnd  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Mdm","Tuppenny Nudger (Mdm) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2008    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","unknown '008' (Unk) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2chsn   ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","unknown 'chsnsn05' (Unk) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2lhs    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","unknown 'lhs' (Unk) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nn2    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","unknown 'nn_2' (Unk) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2rm941  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","unknown 'rm941' (Unk) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2pharo  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Pharoah (Unk) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2nud5p  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","5p Nudger (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2b7     ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Bar 7? (JPM) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2ss     ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Supa Stepper (JPM) (MPS)",                       MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2nbz    ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Nudge Bonanza (JPM) (MPS)",                      MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2clbbin ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "Crystal?","Club Bingo (Crystal) (MPS)",                        MACHINE_IS_SKELETON_MECHANICAL )

GAME(198?, j2sirich ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Strike It Rich (JPM) (MPS) (set 1)",                     MACHINE_IS_SKELETON_MECHANICAL ) // I don't think this is MayGay, it was just in a MayGay set with some actual MayGay roms
GAME(198?, j2siricha,j2sirich   ,jpmmps,jpmmps, driver_device, 0,ROT0,   "JPM","Strike It Rich (JPM) (MPS) (set 2)",                     MACHINE_IS_SKELETON_MECHANICAL ) // I don't think this is MayGay, it was just in a MayGay set with some actual MayGay roms

GAME(198?, j2hcash  ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Hot Cash (Unk) (MPS)",                     MACHINE_IS_SKELETON_MECHANICAL )
GAME(198?, j2sset   ,0          ,jpmmps,jpmmps, driver_device, 0,ROT0,   "<unknown>","Sunset Strip (v2.0) (Unk) (MPS?)",                     MACHINE_IS_SKELETON_MECHANICAL )
