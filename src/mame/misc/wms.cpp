// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**************************************************************************************************

  WMS 360 / 550 (also 3601?) CPU 1.5 Plus board
  ---------------------------------------------

  Both systems (360 and 550) were originally composed by two boards:

  CPU board:    WMS CPU 1.5 (Plus) MPU/CPU board. Copyright 1999.
  I/O board:    WMS PCBA Video I/O board.

***************************************************************************************************

  Hardware Specs...

  WMS CPU 1.5 (Plus) MPU/CPU board:

  1x AMD N80C188-20 (CMOS High-Integration 16-bit Microprocessor), @20Mhz (40/2) [XU12]
  1X Analog Devices ADSP-2105 KP-55 (Single-Chip 16-bit Microcomputers: 20 MIPS, 5v, 1 Serial Port) [U28]
  1x Cirrus Logic CL-GD5429-86QC-C (VGA Graphics Controller) [U32]
  1x QuickLogic QL2003-XPF144C FPGA [U21]

  4x (1-2-4/8 Mb selectable through jumper) program ROM sockets. [XU02, XU03, XU04, XU05]
  4x (1-2-4/8 Mb selectable through jumper) sound ROM sockets. [XU17, XU18, XU30, XU31]
  1x unknown serial EEPROM (DIP8) (currently missing) [XU27]

  4x ISSI IS41C16257-60K (256K x 16, 4-MBit Dynamic RAM with fast page mode) [U14, U15, U22, U23]
  2x Cypress Semiconductor CY62256LL-70SNC (32K x 8 Static RAM) (near program ROMs) [U6, U9]
  3x Cypress Semiconductor CY7C128A-35VC (CMOS 2048 x 8 Static RAM) (between ADSP-2105 and sound ROMs) [U25, U29, U33]

  1x EXAR ST 16C1550CJ (UART Interface IC W/16-byte FIFO) [U19A]
  1x Intersil Corporation CS82C59A (CMOS Priority Interrupt Controller) [U20]
  1x MAXIM MAX232CWE (Integrated RS-232 Interface IC) (near ADSP-2105) [U24]
  1x MAXIM MAX691ACWE (Microprocessor Supervisory Circuit) [U7]
  1x EPSON RTC72423 (4-bit Real-Time Clock) [U10]

  1x Pletronics MP49 (1.8432 MHz) Crystal (between the edge connectors) [X1]
  1x Pletronics MP49 (14.31818 MHz) Crystal (near the Cirrus Logic VGA device) [X2]
  1x Pletronics 40.000 MHz. Crystal (near the AMD N80C188) [Y1]

  1x 3-pin jumper (for program ROM size) [JP2]
  1x 3-pin jumper (for sound ROM size) [JP3]
  1x 2-pin jumper (near the 40 MHz crystal) [JP300]

  1x VGA D-SUB connector [J1]
  2x 96-Pins (male) edge connectors.
  1x push button [SW1]
  1x 7-segments LED [DS1]
  1x 3.6V lithium battery [B1]


***************************************************************************************************

  Notes...

  The WMS CPU 1.5 Plus board has an upgrade: The WMS 360 / 550 Bluebird CPU NXT board,
  based on a Pentium processor.

  About the dates: There is an internal string containing the game date, but wms seems to be quite
  purposefully subtracted 1900 from the year and stored the result.

***************************************************************************************************

  Settings...

  JP2 Software:
  1-2: 4M or less EPROMS.
  2-3: 8M EPROMS.

  JP3 Sound:
  1-2: 4M or less EPROMS.
  2-3: 8M EPROMS.


***************************************************************************************************/


#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/adsp2100/adsp2100.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define MAIN_CLOCK        XTAL(40'000'000)          // Pletronics 40.000 MHz. Crystal. Used for CPU clock.
#define VIDEO_CLOCK       XTAL(14'318'181)    // Pletronics MP49 14.31818 MHz. Crystal. Used in common VGA ISA cards.

#define UART_CLOCK        XTAL(1'843'200)      // Seems UART clock, since allows integer division to common baud rates.
												// (16 * 115200 baud, 192 * 9600 baud, 1536 * 1200 baud, etc...)


class wms_state : public driver_device
{
public:
	wms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void wms(machine_config &config);

	void init_wms();

private:
	uint8_t test_r();
	uint32_t screen_update_wms(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;
	void wms_io(address_map &map) ATTR_COLD;
	void wms_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
};

uint32_t wms_state::screen_update_wms(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/*********************************************
*           Memory Map Information           *
*********************************************/

void wms_state::wms_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x60000, 0xfffff).rom().region("maincpu", 0x60000); // TODO: fix me
}

uint8_t wms_state::test_r()
{
	return 1;
}

void wms_state::wms_io(address_map &map)
{
	map(0x1207, 0x1207).r(FUNC(wms_state::test_r));
}


void wms_state::adsp_program_map(address_map &map)
{
}

void wms_state::adsp_data_map(address_map &map)
{
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( wms )
INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*********************************************/

static const gfx_layout gfxlayout =
{
	8,8,
	0x100000/(8),
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( gfx_wms )
	GFXDECODE_ENTRY( "maincpu", 0x00000, gfxlayout,   0, 1 )
GFXDECODE_END


void wms_state::wms(machine_config &config)
{
	/* basic machine hardware */
	I80188(config, m_maincpu, MAIN_CLOCK);    // AMD N80C188-20, ( 40 MHz. internally divided by 2)
	m_maincpu->set_addrmap(AS_PROGRAM, &wms_state::wms_map);
	m_maincpu->set_addrmap(AS_IO, &wms_state::wms_io);

	adsp2105_device &adsp(ADSP2105(config, "adsp", MAIN_CLOCK / 2));  // ADSP-2105 could run either at 13.824 or 20 MHz...
	adsp.set_disable();
	adsp.set_addrmap(AS_PROGRAM, &wms_state::adsp_program_map);
	adsp.set_addrmap(AS_DATA, &wms_state::adsp_data_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(wms_state::screen_update_wms));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_wms);
	PALETTE(config, "palette").set_entries(0x100);
}


/*********************************************
*                 ROMs Load                  *
*********************************************/

// --------------- SetUp/Clear Chips ---------------

ROM_START( wms )    // RamClear Rev 3.03
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "10.bin",      0x0e0000, 0x020000, CRC(cf901f7d) SHA1(009a28fede06d2ff7f476ff643bf27cddd2adbab) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END

ROM_START( wmsa )    // RamClear Rev 3.03
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "10cver4.010", 0x0e0000, 0x020000, CRC(fd310b97) SHA1(5745549258a1cefec4b3dddbe9d9a0d6281278e9) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END

ROM_START( wmsb )    // RamClear Rev 3.03
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "50cver4.010", 0x0e0000, 0x020000, CRC(eeeeab29) SHA1(898c05c0674a9978caaad4a0fe3650a9d9a56715) )
	ROM_REGION(0x100000, "rom", ROMREGION_ERASE00)
ROM_END


// ----------------- Regular Sets ------------------


ROM_START( btippers )    // Big Tippers (Russian) / U5 03/09/01
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(96e362e3) SHA1(a0c35e9aa6bcbc5ffbf8750fa728294ef1e21b02) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(5468a57c) SHA1(3cb87c288bef1782b086a9d6d17f5c3a04aca3c8) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(460ce5b6) SHA1(a4e22fff508db1e36e30ce0ec2c4fefaee67dcfc) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(442ed657) SHA1(e4d33c85c22c44908a016521af53fc234a836b63) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(2d7a9a0e) SHA1(0ab5752ca3bf360180caec219b7bfd478bb09cf4) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(5d767b66) SHA1(fb0866408657db540b85641ad5624885d7ef58ef) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(d4f533a9) SHA1(5ec53fed535fe6409481f99561c13e1fb98385ed) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(c845e18a) SHA1(3e20fbf6ac127a780a7a1517347e3cf7e951e5eb) )
ROM_END


ROM_START( wmsboom )    // Boom (Russian) / U5 02/12/98
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(5008854d) SHA1(8d9d11775b6cbdef1c71683c4d92e64af26e8939) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(b1fc3e98) SHA1(372aab282905f1fe5781a87f2791d34c93aa0492) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x080000, CRC(ba1b2ab6) SHA1(087a360c1260484ad3bc0b2601003da9581a92b5) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x080000, CRC(3ba6c20d) SHA1(2324a7bc83f695541bbf1a66c0559fea30f3d007) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x080000, CRC(948e8104) SHA1(04bffd1bb2dc9b96550424e8be64a75907b4cbe4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x080000, CRC(68187707) SHA1(9a85fe5737ae372999f2bfaf50263c00fc9b22f4) )
ROM_END


ROM_START( cashcrop )    // Cash Crop (Russian) / U5 09/05/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(e1918f25) SHA1(55c33c1f604a44caef65e712e69d21792161dfbc) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(22eb1718) SHA1(cc99997d446efd9df99a3e4f04589d1a47d2b638) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(4ce5b630) SHA1(84495ea66d7636a956324f7e8d334fd6aa74724f) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(3073a171) SHA1(7758500fcf8a8c0c43c464644de885a95b8ae152) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(7090eefd) SHA1(f0bc2ab2da956ab3921774c839cab065a59e1daa) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(a8b01689) SHA1(15916b7f98a49dac848a6010f96d9ccac4a9e8f2) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(5fc10445) SHA1(722bf476d0efc6fc77395f2d2cc3913f61a137ac) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(17cdabce) SHA1(d54e6d1697ccd424922c712f4a1f75e8d5f44288) )
ROM_END


ROM_START( filthyr )    // Filthy Rich (English) / U4 09/03/97
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "filthy_rich.xu3", 0x0000, 0x100000, CRC(0673102d) SHA1(1d69e0f7b4d5faa37ecb8edba5a1eb63210d7032) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "filthy_rich.xu2", 0x0000, 0x100000, CRC(095c5a44) SHA1(37bf83160bac1c4ab3b64abd207fe111da6b7a1b) )
	// sound
	ROM_LOAD( "filthy_rich.xu30", 0x0000, 0x100000, CRC(28d97696) SHA1(1bee36610ed5123ac3cb63fef7069e7975fffe6f) )
	ROM_LOAD( "filthy_rich.xu31", 0x0000, 0x100000, CRC(29ec4d17) SHA1(fbdb7775d4e1abf2241eb29ee042aeb9deb5bbeb) )
	ROM_LOAD( "filthy_rich.xu17", 0x0000, 0x100000, CRC(6ef1ca21) SHA1(c0e98c2430b85623a9481262c9d06103e22c4fdd) )
	ROM_LOAD( "filthy_rich.xu18", 0x0000, 0x100000, CRC(f74c48c7) SHA1(e62d2691ad579291c8cbb6f2c61399dbe52515c3) )
ROM_END


ROM_START( filthyrr )    // Filthy Rich (Russian) / U4 09/03/97
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(4b57aa15) SHA1(076ef6c1782a0ec34c15bffd5e93644ba179c1f7) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(12dfb80c) SHA1(b7cff3b6dd76102ae3b7cea51da97af9578088ef) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x080000, CRC(86b7c3b4) SHA1(d2c555aa88d141fbd17a5473631adec258b38fe2) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x080000, CRC(12d4e1f2) SHA1(f6317e39a0dd0ce7248f4e1c6a0d69f2d01eb613) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x080000, CRC(d4ce89cb) SHA1(63ddfc5a7ab4f271ebe6ef650832c3b582004acf) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x080000, CRC(fb0de283) SHA1(e1df32787a6fcf597041923d977eacbe5744b770) )
ROM_END


ROM_START( hottop )    // Hot Toppings (English) / U5 04/22/103
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "hot_topping_9fbah.xu03", 0x0000, 0x100000, CRC(9da8d2d7) SHA1(722809ec9df83c866ec7cbcc83b619d65134b523) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "hot_topping_8aceh.xu02", 0x0000, 0x100000, CRC(15ebe9a4) SHA1(3852bf9bd0976abe46102b2c237a513f6447bea6) )
	ROM_LOAD( "hot_topping_2935h.xu04", 0x0000, 0x100000, CRC(8cab6576) SHA1(be1dc406cbf4fd4035f4e123c4095a0a07350035) )
	// sound
	ROM_LOAD( "hot_topping_1a00h.xu30", 0x0000, 0x100000, CRC(18b938ff) SHA1(4c75bfc23cb3754aa1457ed4183664c8d579b46f) )
	ROM_LOAD( "hot_topping_8f00h.xu31", 0x0000, 0x100000, CRC(944df9f0) SHA1(5cc1f40aefcc82098d12d00fef603d86cd0c145f) )
	ROM_LOAD( "hot_topping_6100h.xu17", 0x0000, 0x100000, CRC(9345c066) SHA1(ae04f59e4791ec1cc8751fb1fe194e4f690e442b) )
	ROM_LOAD( "hot_topping_1600h.xu18", 0x0000, 0x100000, CRC(2ad1b2e2) SHA1(47832422be66acace22fe411062010303b9ce899) )
ROM_END

ROM_START( hottopa )    // Hot Toppings (Russian?) / U5 06/26/101
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, NO_DUMP ) // missing

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(15ebe9a4) SHA1(3852bf9bd0976abe46102b2c237a513f6447bea6) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(8cab6576) SHA1(be1dc406cbf4fd4035f4e123c4095a0a07350035) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6ac44a87) SHA1(37d2c07845804b5ac472a6f443d82f3c13d02d04) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c52b0eba) SHA1(2d69b69a403619f92c35e05e90ce5a76e1cc50a1) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(371f075f) SHA1(ffebaf9d36a0b3b2b726e39b51c456cabeb6e2c4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(bee1eae3) SHA1(e18b10e7c6dafd9df8b34ee19ce74a82b8adcec4) )
ROM_END


ROM_START( inwinner )    // Instant Winner (Russian) / U5 11/25/98
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(f5c26579) SHA1(4ef22c0115417320ef17f0c3fc7550db24bdcb8c) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(93082de5) SHA1(5469cd5dfc2a54707cf08feae1e09d808efabf6c) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(d6d3e635) SHA1(90ac66c26fe807992117eab42e12006d9c491ffb) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(6e0c0bc9) SHA1(3f518c507a75bd3d48a3387c80936ff713a4a05d) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(0fff871b) SHA1(703cbdcbe20988d1422c89cde7c63f387af6de3c) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(8a42ac41) SHA1(3e56e369938d4789d4677f8ad0a7c0e1eb38942d) )
ROM_END

ROM_START( inwinners )    // Instant Winner (Spanish)
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "instant_winner_xu3_spanish.bin", 0x0000, 0x100000, CRC(8c3f81bb) SHA1(ae1722763f5b9d2a1b72f3889752979af150b8b0) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "instant_winner_xu2_spanish.bin", 0x0000, 0x100000, CRC(54895461) SHA1(6b2cb92fc82ddbe96924758c130e271848bf5b0b) )
	// sound
	ROM_LOAD( "instant_winner_xu30_spanish.bin", 0x0000, 0x100000, CRC(c2e6f321) SHA1(e4251b0c42fd03932cd94d18edca8d33a42d9670) )
	ROM_LOAD( "instant_winner_xu31_spanish.bin", 0x0000, 0x100000, CRC(9dcd7705) SHA1(8c543b4be6e51edfc512c2449086e418f8d32310) )
	ROM_LOAD( "instant_winner_xu17_spanish.bin", 0x0000, 0x100000, CRC(8cbf7b46) SHA1(b95fbd1c11bbbb2e97a06e541442bd5c396b500f) )
	ROM_LOAD( "instant_winner_xu18_spanish.bin", 0x0000, 0x100000, CRC(906451f6) SHA1(9da2ff04d28632fd68ab08cb3786aa19b904fdc9) )
ROM_END


ROM_START( jptparty )    // Jackpot Party (Russian) / U5 04/01/98
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(750bfd36) SHA1(161c98455cb2ceb1d7d1a35941b5bb0784129b25) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(78c8bfcd) SHA1(a6ce68a88bcd3c74bffcc852d6f91b6f0f077a91) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(969ac077) SHA1(30207da5f9c3e1fc2a1f4d1bc66fcd2bfc760a8e) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c76a100e) SHA1(e29b65a92fd8254f577852acfefbec40840e4962) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(53001015) SHA1(ce08db51f812a71db93026bb9d9c33e705020860) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(4f34c9be) SHA1(ef1ad53002c5232fd64715add8f5646c7c6499d6) )
ROM_END


ROM_START( leprgld )    // Leprechaun's Gold (Russian) / U5 08/20/101
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(0985d6b6) SHA1(291c124a9ae813942b14620789370b4a7b9cf85e) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(e29c5127) SHA1(4afe22cd59fa8104b2920b5b3aca9d23ae55c83a) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(8e046831) SHA1(b16b707962c70ef71c11b9c181bdc2335fdf1f51) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(1dbcfdb8) SHA1(652e2939a87e4a68bdb9af405274cb49a7dbed00) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6a41056f) SHA1(54168a0115dc8d232100fbbd440063dc75fdbaaf) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(0485e432) SHA1(c57a47fd27a14a2ea01400515f61be58be5fa391) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(10bfcdcf) SHA1(34dc619f33c81014120d9b6a2f8b2373451af2e4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(a3f82467) SHA1(0971f32bff0d4bda43cd98e975e49710d73f9286) )
ROM_END

ROM_START( leprglds )    // Leprechaun's Gold (Spanish) / U5 08/20/101
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "leprechaun_s_gold.xu03", 0x0000, 0x100000, CRC(a750e5b9) SHA1(891bbfded9ece4d81aa8415cbb5a2728c118300e) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "leprechaun_s_gold.xu02", 0x0000, 0x100000, CRC(884d4871) SHA1(2c7a452dd466532df5fc14b209302a5ad42bb356) )
	ROM_LOAD( "leprechaun_s_gold.xu04", 0x0000, 0x100000, CRC(02642dd2) SHA1(4ac00b9053f9859865910888952e09d2fda5db06) )
	ROM_LOAD( "leprechaun_s_gold.xu05", 0x0000, 0x100000, CRC(eedd0dd2) SHA1(2b1dceb541a660357404f24069392f7892c87cfb) )
	// sound
	ROM_LOAD( "leprechaun_s_gold.xu30", 0x0000, 0x100000, CRC(6a41056f) SHA1(54168a0115dc8d232100fbbd440063dc75fdbaaf) )
	ROM_LOAD( "leprechaun_s_gold.xu31", 0x0000, 0x100000, CRC(0485e432) SHA1(c57a47fd27a14a2ea01400515f61be58be5fa391) )
	ROM_LOAD( "leprechaun_s_gold.xu17", 0x0000, 0x100000, CRC(10bfcdcf) SHA1(34dc619f33c81014120d9b6a2f8b2373451af2e4) )
	ROM_LOAD( "leprechaun_s_gold.xu18", 0x0000, 0x100000, CRC(a3f82467) SHA1(0971f32bff0d4bda43cd98e975e49710d73f9286) )
ROM_END


ROM_START( lol )    // Life of Luxury (Russian) / U3 08/30/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(28af87a0) SHA1(c94b98b6922b5f5551eff239226bfe007535fc21) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(58bb1a7c) SHA1(9b93b8d51c0d2895a0926cef6875ca9e985f3193) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(1782c8f4) SHA1(35930b3a1a3a00e7c4e2bfafeb482425a875aa18) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(defafc56) SHA1(4faa41cdf2af52a2dfe8342f0238e13608676cc5) )
ROM_END


ROM_START( lovewin )    // Love To Win (Russian) / U5 04/18/101
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(b1b2c7da) SHA1(96430bfa062467bba1daaf2e6180068b3211a739) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(9cdff164) SHA1(58d8692476e73db12674ed986f8ce17183c687e9) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(e0c44410) SHA1(2a95a3ee68cf9c19d6427379d3754506d8a89e88) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(75fd9131) SHA1(db6e6099ccc1c5bca3953a2322eaf154d61dea2b) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(fda225f2) SHA1(757d9118657fa6e033e1449ed5cae25079eb2616) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(c923d180) SHA1(358383458f04950c346a1af07b0c88a3dcd90bf4) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(a2b19335) SHA1(f078582d0be2c79e973b2f1472f1288ac1e73ae1) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(e003b31c) SHA1(41e7fb83041a86a1f6b5d73eb5310739584aeaeb) )
ROM_END


ROM_START( mtburn )    // Money To Burn (English) / U5 08/02/101
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "money_to_burn.xu3", 0x0000, 0x100000, CRC(92d94269) SHA1(8f7e342837067b6c2180c3a161bc34671c0fab72) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "money_to_burn.xu2", 0x0000, 0x100000, CRC(cf496bf5) SHA1(8cc8b9106316fef3bd1436b08488ae8552411f9d) )
	// sound
	ROM_LOAD( "money_to_burn.xu30", 0x0000, 0x100000, CRC(86255bc7) SHA1(75b6876d72c9f4c576f807fe970d49e8c208b477) )
	ROM_LOAD( "money_to_burn.xu31", 0x0000, 0x100000, CRC(f5c80417) SHA1(097d9fc73f44674cff3b24be917a0db7e4117553) )
	ROM_LOAD( "money_to_burn.xu17", 0x0000, 0x100000, CRC(0ec9579a) SHA1(8f78282f4691426a0ca49387ab58f63a578b96a2) )
	ROM_LOAD( "money_to_burn.xu18", 0x0000, 0x100000, CRC(69fe5e04) SHA1(532d9a6b1326543227a8f64c595b1f8bfc0dbf4a) )
ROM_END


ROM_START( mtburnr )    // Money To Burn (Russian) / U5 02/11/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a329aff0) SHA1(2f631783bd15b8a695d92d0861e167130f914046) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a0f63408) SHA1(5740ecf6413208c468cdfb84e36887a7ca01262f) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(dc238bf3) SHA1(f649d20fa7b6ce9ee3c9ff392e78ddd66dff8638) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(36b773ac) SHA1(1ce30518da56989ceae3389988b671d9e319c66e) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(20c28e90) SHA1(54ffb735bea5b68513eb6831a749cd3dc94af15f) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(336e2b23) SHA1(ee47d41c95e418f7c2c647b5f6e47e7bb203c50e) )
ROM_END


ROM_START( otchart )    // Off The Charts (Russian) / U5 07/31/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(b1baee77) SHA1(9f8e87bb10499fa73d961bce4fd75d31c880ea51) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a329aff0) SHA1(2f631783bd15b8a695d92d0861e167130f914046) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(3b2ebe91) SHA1(c2576b303242f75489b96ba90a24d8ad737b5064) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(f0e69a31) SHA1(82552972b8769ec836c3e84110fa6ed2efdcf970) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(d9ab5d90) SHA1(880dab563268ac230c61647bb1ced666606e0f80) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(1f875649) SHA1(4e8e1f860d01f72539f3965c730cc69c3789f898) )
ROM_END


ROM_START( perfect )    // Perfect Game (Russian) / U5 03/17/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(1e90dc52) SHA1(6ac1762ce979e1259e03025d89271034565d8f4a) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(f6f09482) SHA1(55d9395d473ccafbb655bc852a0ea8e9b11957ae) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(9bfc079f) SHA1(99dda328e5796b5e8fd1f43251bbfa5573d8991e) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(f9cf7ee6) SHA1(da74df8fdd1312e1a2dbaa74bb9bc8d97b3fc08e) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(7c04579d) SHA1(6aed1dd61efb58c551db1eccde21935ff5fb517e) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(beb1992e) SHA1(94a6d1179a8a9755cb8c3469f9dff81551e3a845) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(af9270a5) SHA1(e5ab2039d02749a0847f1712d2e0d610a5146c60) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(03fe1d5f) SHA1(87c7b8d77b4654bc938440516035a63704cf108d) )
ROM_END


ROM_START( reelemin )    // Reel 'Em In (English) / U4 01/22/97
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "reel__em_in.xu3", 0x0000, 0x100000, CRC(a3840965) SHA1(1e782bb5eaeabce0d1a07eb895d344028364cd4f) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "reel__em_in.xu2", 0x0000, 0x100000, CRC(04658d90) SHA1(1180b2596175394e079c8498bfc5e956426d0e1c) )
	// sound
	ROM_LOAD( "reel__em_in.xu30", 0x0000, 0x100000, CRC(742590b2) SHA1(65df9d346b1f69f79e6e79c91724813f052c64ee) )
	ROM_LOAD( "reel__em_in.xu31", 0x0000, 0x100000, CRC(bf3aabb8) SHA1(251c1bcf28f117152902b37915c41be1e912dda0) )
	ROM_LOAD( "reel__em_in.xu17", 0x0000, 0x100000, CRC(f99bf909) SHA1(4dac7aac0cf815d0b67f4c8d55d84f529e45f27d) )
	ROM_LOAD( "reel__em_in.xu18", 0x0000, 0x100000, CRC(1e4b64f1) SHA1(f082b2a7b6e029ffded1eec6fbc5f755bdd8d9eb) )
ROM_END


ROM_START( reeleminr )    // Reel 'Em In (Russian) / U4 01/22/97
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a3840965) SHA1(1e782bb5eaeabce0d1a07eb895d344028364cd4f) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(04658d90) SHA1(1180b2596175394e079c8498bfc5e956426d0e1c) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(742590b2) SHA1(65df9d346b1f69f79e6e79c91724813f052c64ee) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(bf3aabb8) SHA1(251c1bcf28f117152902b37915c41be1e912dda0) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(f99bf909) SHA1(4dac7aac0cf815d0b67f4c8d55d84f529e45f27d) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(1e4b64f1) SHA1(f082b2a7b6e029ffded1eec6fbc5f755bdd8d9eb) )
ROM_END


ROM_START( sonoth )    // Something For Nothing (Russian) / ???
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(a5afc1dd) SHA1(5743c630d382d1cc2f7720a2038b105f0dd49790) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(66aff9dd) SHA1(d5eced91421957f9ff5929a3bed10a81d52916b9) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(57e139d7) SHA1(7973ab1cbfcd23b8d812ca843c1e8ebfdd17599b) )
ROM_END


ROM_START( swingin )    // Swingin In The Green (Russian) / U5 03/17/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(f0ee2385) SHA1(8269cf0a765228fd8a2512cefde715e3a7219d7a) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(7857764b) SHA1(07eeb9f134ee52a03bda72f7df037a81b32a70a4) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(fed9c637) SHA1(64dd3f53f844ce3be69f941c7b7a9f6804f4181f) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(fb56775b) SHA1(283e8c5abd3e8d7c45aededa85182b60aed781d8) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(e3dc27f5) SHA1(0ef43b4a87c624fd8e008b4346065de1731b8c9b) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(85da08d9) SHA1(cd60b524d353be2094e403263ea7d3d1d9ded472) )
ROM_END


ROM_START( wmstopb )    // Top Banana (Russian) / U4 04/30/99
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(dbbabd02) SHA1(6e85bb11222c593a326e282e6edfff3647379c78) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(74212101) SHA1(d8e25de334e451a56e9a7cd0f9eca8d1bea4b6b4) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(44e7a0ae) SHA1(8558ec01b3d3c96a4a47869d4e69cbe66ee1b804) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(7b034ad8) SHA1(55d2a5ee42dcf06557ce7ff40977393c70a00123) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(60f6f2fd) SHA1(5d3dd6f5a2b10b45d3281b49608cd24a110e0541) )
ROM_END


ROM_START( wdun )    // Who Dunnit (Russian) / U5 09/02/99
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(9c5f2a50) SHA1(8c591eb80276d460dd66bdf817778aed11d013ef) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(e272e5c5) SHA1(a51c723d51e0632a192597d2ab8ec09bcf1e54e7) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(9a5c34c7) SHA1(680c504868d6850df1f450c0c4f5345c112f7151) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(fa942bad) SHA1(faac6fd68c3ec7d3da564e9bb19ebb2210ad275f) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(fd98076e) SHA1(7a19e9c66df099ac1315efa62575c4d53a7f4582) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(59aaceb0) SHA1(2f7506b634291cc1f7654115fe6be7d34ba16c8c) )
ROM_END


ROM_START( winbid )    // Winning Bid (English) / U5 10/09/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "winning_bid.xu3", 0x0000, 0x100000, CRC(a60afb19) SHA1(461e6b8ac1a8874ecd4ecd709ed0f0e6d9aab989) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "winning_bid.xu2", 0x0000, 0x100000, CRC(9b8d153e) SHA1(24e5500f90eb0c903f09ac62b3496bea9a6cd640) )
	ROM_LOAD( "winning_bid.xu4", 0x0000, 0x100000, CRC(80079c9a) SHA1(296cea5e7aa2f0f0bfb03af335cc4c65a9cb68c3) )
	// sound
	ROM_LOAD( "winning_bid.xu30", 0x0000, 0x100000, CRC(2ef7bc4d) SHA1(f287098341de34d79f18fcd38f9e5cdd13bdaaeb) )
	ROM_LOAD( "winning_bid.xu31", 0x0000, 0x100000, CRC(068e4cab) SHA1(7ea3804353601c82ebc2d4cb4dd8bf1f8a48132e) )
	ROM_LOAD( "winning_bid.xu17", 0x0000, 0x100000, CRC(6f7bcadf) SHA1(9ddd0a0259dc2d7e69fcb58a16fac4ddc344e41e) )
	ROM_LOAD( "winning_bid.xu18", 0x0000, 0x100000, CRC(f7f8814e) SHA1(274e669df9e6e33eb5867906069f43a9c8898d45) )
ROM_END


ROM_START( winbidr )    // Winning Bid (Russian) / U5 09/07/99
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(873eecfb) SHA1(35960c9a2a4efeac8b8cea94f1eac6ffb175b9a9) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(628d4bc6) SHA1(7f812addacd5e93843e2a4dc852fbc543dbc4cec) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(0882d890) SHA1(23b388477078c6a1b2756eb3cacb4c146109bb90) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(6dd36b10) SHA1(d595b9cf4f26e55f6aa1cdf18482d9808cf923bd) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(711f6064) SHA1(4dfd12df5594dda46754b12cf6e8399da25051b9) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(74d7272a) SHA1(127fd29a0edb86a2c893a50d4da3cd961673c8b4) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(2ad34442) SHA1(a20b56c63d4b7a41ff9bb44aadca9fd297276b58) )
ROM_END


ROM_START( wldstrek )    // Wild Streak (Russian) / U4 04/11/100
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(6200c5e3) SHA1(fcf542875a11972974a9c72e171d4ca0686b0c0e) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(d194924a) SHA1(2880ddf2950bb9a4d325e4f13fdad91b3978b7b7) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(6023bc00) SHA1(b53ac52bc4d323168d081aece520743f03833dd6) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(d2f34426) SHA1(14174a6babba97d17dddd54de14ef5cffead79d1) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(f0865c98) SHA1(c93d256388357bc782caaba2e1cc917e248b640d) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(49917f88) SHA1(e21ba71fbce2e52e5a65104be4f7624e1b38f16e) )
ROM_END


ROM_START( yukongld )    // Yukon Gold (Russian) / U5 11/06/00
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "xu-3.bin", 0x0000, 0x100000, CRC(7c12f7c7) SHA1(20eb1a5ff85e0fd36e669c7dc53b677f12446af3) )

	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD( "xu-2.bin", 0x0000, 0x100000, CRC(a1330976) SHA1(d1307f2bd2f4d407b38557507ecbf94e3a5b43d1) )
	ROM_LOAD( "xu-4.bin", 0x0000, 0x100000, CRC(e5d722cd) SHA1(eacbd9bac5f1f931ae951f3f746851ac3237442a) )
	ROM_LOAD( "xu-5.bin", 0x0000, 0x100000, CRC(cae8cb45) SHA1(64647f1392e7ac870058461cb9f128ac61182906) )
	// sound
	ROM_LOAD( "xu-30.bin", 0x0000, 0x100000, CRC(0c4fd3a0) SHA1(6371cb9d108be20d717a92c26a87666b6bb676fc) )
	ROM_LOAD( "xu-31.bin", 0x0000, 0x100000, CRC(4d30c67c) SHA1(feda6b0e09bd39e871685c89a6566986012c4099) )
	ROM_LOAD( "xu-17.bin", 0x0000, 0x100000, CRC(56f03f5d) SHA1(93c5a7698dd10a97f1f386c27560a816a779beb1) )
	ROM_LOAD( "xu-18.bin", 0x0000, 0x100000, CRC(5978592d) SHA1(7716070894e2b201e37a290b3a8bcf44ce54ea6c) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void wms_state::init_wms()
{
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME        PARENT    MACHINE  INPUT     STATE      INIT  ROT    COMPANY  FULLNAME                          FLAGS

GAME( 200?, wms,        0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "WMS SetUp/Clear Chips (set 1)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, wmsa,       wms,      wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "WMS SetUp/Clear Chips (set 2)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, wmsb,       wms,      wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "WMS SetUp/Clear Chips (set 3)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2001, btippers,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Big Tippers (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 03/09/01
GAME( 1998, wmsboom,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Boom (Russian)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 02/12/98
GAME( 2000, cashcrop,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Cash Crop (Russian)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 09/05/100
GAME( 1997, filthyr,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Filthy Rich (English)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 09/03/97
GAME( 1997, filthyrr,   filthyr,  wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Filthy Rich (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 09/03/97
GAME( 2003, hottop,     0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Hot Toppings (English)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 04/22/103
GAME( 2001, hottopa,    hottop,   wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Hot Toppings (Russian?)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 06/26/101
GAME( 1998, inwinner,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Instant Winner (Russian)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 11/25/98
GAME( 1998, inwinners,  inwinner, wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Instant Winner (Spanish)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // ???
GAME( 1998, jptparty,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Jackpot Party (Russian)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 04/01/98
GAME( 2001, leprgld,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Leprechaun's Gold (Russian)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 08/20/101
GAME( 2001, leprglds,   leprgld,  wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Leprechaun's Gold (Spanish)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 08/20/101
GAME( 2000, lol,        0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Life of Luxury (Russian)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U3 08/30/100
GAME( 2001, lovewin,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Love To Win (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 04/18/101
GAME( 2001, mtburn,     0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Money To Burn (English)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 08/02/101
GAME( 2000, mtburnr,    mtburn,   wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Money To Burn (Russian)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 02/11/100
GAME( 2000, otchart,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Off The Charts (Russian)",        MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 07/31/100
GAME( 2000, perfect,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Perfect Game (Russian)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 03/17/100
GAME( 1997, reelemin,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Reel 'Em In (English)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 01/22/97
GAME( 1997, reeleminr,  reelemin, wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Reel 'Em In (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 01/22/97
GAME( 200?, sonoth,     0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Something For Nothing (Russian)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // ???
GAME( 2000, swingin,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Swingin In The Green (Russian)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 03/17/100
GAME( 1999, wmstopb,    0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Top Banana (Russian)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 04/30/99
GAME( 1999, wdun,       0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Who Dunnit (Russian)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 09/02/99
GAME( 2000, winbid,     0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Winning Bid (English)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 10/09/100
GAME( 1999, winbidr,    winbid,   wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Winning Bid (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 09/07/99
GAME( 2000, wldstrek,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Wild Streak (Russian)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U4 04/11/100
GAME( 2000, yukongld,   0,        wms,     wms,      wms_state, init_wms,  ROT0, "WMS",   "Yukon Gold (Russian)",            MACHINE_NO_SOUND | MACHINE_NOT_WORKING )  // U5 11/06/00
