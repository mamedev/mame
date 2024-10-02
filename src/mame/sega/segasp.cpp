// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Sega System SP (Spider)
skeleton driver

this is another 'Naomi-derived' system

as for SP... it must be done having few things in mind:
differences with Naomi in SH4 memory map only:
0x00200000 - 0x00207fff - no SRAM here
0x01000000 - 0x0100ffff - banked access to ROM board address space, see later
0x01010000 - 0x010101ff - I/O registers, IRQ control, reg 0 - upper 16bits of rombd bank address

as for ROM board - unlike regular Naomi M4 it have not only ROM itself,
but the other hardware too*:
0x00000000 - 0x1fffffff - FlashROM, like in regular Naomi M4 cart
0x39xxxxxx - SRAM(128KB)**
0x3axxxxxx - CF IDE registers (0 - data, 4 - error, 8 - sector count, etc)
0x3bxxxxxx - CF IDE AltStatus/Device Ctrl register
0x3dxxxxxx - Network aka Media board shared buffer/RAM
0x3fxxxxxx - Network board present flag (0x01)**

*note: M4-decryption works for all areas if address bit 30 is 1,
for example BIOS can read CF IDE data with on-fly data decryption

**note: this is 8bit device on 16bit bus, each even byte access actual
data, each odd byte is 0xFF.

so, unlike regular Naomi this "rom board" can be accessed via BOTH regular
G1 bus PIO or DMA, or directly via banked  area 0x0100xxxx in SH4 address space.



todo: make this actually readable, we don't support unicode source files

 Title                                       PCB ID     REV CFID    Dumped Region  PIC             MAIN BD Serial
100&Medal Kazaaan!!                         834-15052                 ROM  JP     253-5508-0626J  AAFE-xxxxxxxxxxx
100&Medal Gekikazaaan!!                     834-15148      MDA-C0098D CF   JP     253-5508-0637J  AAFE-xxxxxxxxxxx
Aminosan (satellite)                        837-15041    F*           ROM  JP     253-5508-0613J  AAFG-01A40195003, Medal
Arabian Jewel (main)                        834-15011                 ROM  JP     253-5508-616J   AAFE-xxxxxxxxxxx
Arabian Jewel (satellite)                   837-15019                 ROM  JP     not used        AAFE-xxxxxxxxxxx
Battle Police                               ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Battle Racer                                834-14882NRU              ROM  JP     253-5508-0579J  AAFE-xxxxxxxxxxx, GAME BD SP BTR NRU
Beetle DASH!!                               ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Bingo Galaxy (main)                         834-14788    C            ROM  JP     253-5508-0513J  AAFE-01A37754716, AAFE-01E10924916, AAFE-01D67304905, Medal
Bingo Galaxy (main)                         834-14788    C MDA-C0039A CF   JP     253-5508-0513J  AAFE-xxxxxxxxxxx
Bingo Galaxy (satellite)                    837-14481    C            ROM  JP     not used        AAFE-01A36474716, Medal
Bingo Galaxy (satellite)                    837-14789    F*           ROM  JP     not used        AAFE-xxxxxxxxxxx, game is same as above
Bingo Parade                                ???-?????      MDA-C0008E CF   JP     253-5508-0407J  AAFE-xxxxxxxxxxx, Medal
Brick People / Block People                 834-14881                 ROM  ANY    253-5508-0558   AAFE-01F67905202, AAFE-01F68275202
Dinosaur King                               834-14493                 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Dinosaur King                               834-14493-01 D            ROM  US     253-5508-0408   AAFE-01D1132xxxx, AAFE-01D15924816
Dinosaur King (tournament mode update)      834-14622                 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Dinosaur King - Operation: Dinosaur Rescue  837-14434-91              ROM  US/EXP 253-5508-0408   AAFE-01A30164715, AAFE-01B92094811
- // -                                      834-14662-01    MDA-C0021 CF                          AAFE-01B87574811
Dinosaur King 2                             ???-?????                 no          253-5508-0408   AAFE-xxxxxxxxxxx
Dinosaur King 2 Ver 2.5                     834-14739       MDA-C0030 no   JP     253-5508-0408   AAFE-xxxxxxxxxxx
Dinosaur King 2 Ver 2.5                     834-14792-02 F  MDA-C0047 CF   EXP    253-5508-0408   AAFE-01D73384904
Dinosaur King 2 Ver 2.501 China             ???-?????       MDA-C0081 CF   EXP    253-5508-0408   AAFE-xxxxxxxxxxx
Dinosaur King Ver 4.000                     ???-?????       MDA-C0061 CF   JP     253-5508-0408   AAFE-xxxxxxxxxxx
Disney: Magical Dream Dance on Stage        ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Future Police Patrol Chase                  ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Galileo Factory (main)                      834-14773    F*           ROM  JP     ???-????-????   AAFE-01F9856xxxx
Heat Up Hockey Image                        834000201  TSB0-SEGA00001 CF   JP     253-5508-0606J  AAFE-xxxxxxxxxxx also seen on PCB# 8340002
Heat Up Hockey Image                        8340002         MDA-C0099 no   JP     253-5508-0606J  AAFE-xxxxxxxxxxx
Heat Up Hockey Image                        8340002         MDA-C0100 no   JP     253-5508-0606J  AAFE-xxxxxxxxxxx
Issyouni Turbo Drive                        ???-?????                 no          ???-????-????   AAFE-01E91305101
Isshoni Wanwan Waiwai Puppy 2008            834-14747                 ROM         253-5508-0496J  AAFE-01E34655005
Isshoni Wanwan Waiwai Puppy (INW PUPPY)     834-14826       MDA-C0052 no          ???-????-????   AAFE-xxxxxxxxxxx
King of Beetle: Battle Terminal             ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Kouchu ouja Mushiking Popo no boukenhen     834-14583                 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Love & Berry 2K5 Spring/Summer              834-14535       MDA-C0003 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Love & Berry 2K5 Autumn/Winter              834-14556                 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Love & Berry Ver 1.003                      834-14661-02              ROM  EXP    253-5508-0446   AAFE-01D84934906
Love & Berry Ver 2.000                      834-14661-02              ROM  EXP    253-5508-0446   AAFE-01D8493xxxx
Love & Berry 3 EXP Ver 1.002                834-14661-01    MDA-C0042 CF   US/EXP 253-5508-0446   AAFE-01D64704904
Love & Berry 3 CHN                          ???-?????       MDA-C0071 CF   EXP    253-5508-0446   AAFE-01G15765216
Magical Poppins (Medalink)                  837-14700    F*           ROM  JP     253-5508-0474J  AAFE-xxxxxxxxxxx, Satellite Medal
Manpuku Suizokukan                          8340004      F            ROM  JP     253-5508-0640JN AAFE-01E32815002
Marine & Marine 2K7 1ST                     834-14749                 no          ???-????-????   AAFE-xxxxxxxxxxx
Mirage World (SP MRW SATL)                  834-14713                 ROM  ANY    ???-????-????   AAFG-01A3xxxxxxx, Medal
Monopoly: The Medal                         ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx, Medal
Monopoly: The Medal 2nd Edition (main)      ???-?????       MDA-C0056A     JP     253-5508-0528J  AAFE-xxxxxxxxxxx, Medal
Monopoly: The Medal 2nd Edition (satellite) 837-14528                 ROM  JP     not used        AAFE-xxxxxxxxxxx, Medal
Mushiking 2K6 2ND                           ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
Mushiking 2K7 1ST                           834-14734       MDA-C0028 no   JP     ???-????-????   AAFE-xxxxxxxxxxx
Ocha-Ken Hot Medal (Medalink)               837-14790    G            ROM  JP     unknown         AAFE-01G03115212, Satellite Medal
Puyo Puyo! The Medal Edition (Medalink)     837-14875    F            ROM  JP     253-5508-0568J  AAFE-01E78365014
Tetris Giant / Tetris Dekaris               834-14970    G  MDA-C0076 CF   ANY    253-5508-0604   AAFE-01G03025212
Tetris Giant / Tetris Dekaris Ver.2.000     834-14970    G            ROM  ANY    253-5508-0604   AAFE-xxxxxxxxxxx
Thomas: The Tank Engine                     ???-?????                 no          ???-????-????   AAFE-xxxxxxxxxxx
UNO the Medal (Medalink)                    837-14804    F*           ROM  JP     253-5508-0526J  AAFE-01G00225212, Satellite Medal
Western Dream Gold (Medalink)               837-14699    F*           ROM  JP     253-5508-0473J  AAFE-xxxxxxxxxxx, Satellite Medal
Yataimura Kingyosukui (1-player, Japan)     8340003      D            ROM  JP     253-5509-5151J  AAFE-01C68774814
Yataimura Kingyosukui (4-player, China)     837-14875                 CF   EXP    253-5508-0563J  AAFE-xxxxxxxxxxx
Yataimura Shateki (1-player, Japan)         834000301    D            ROM  JP     253-5508-0628J  AAFE-01C37464814
Unknown                                     834-14865                      JAP

REV PCB       IC6s      Flash       AU1500
C  171-8278C  315-6370  8x 128Mbit  AMD
D  171-8278D  315-6370  8x 128Mbit  AMD
F  171-8278F  315-6416  8x 512Mbit  AMD
F* 171-8278F  315-6416  2x 512Mbit  RMI
G  171-8278G  315-6416  2x 512Mbit  RMI

*/

#include "emu.h"

#include "naomi.h"
#include "naomim4.h"

#include "cpu/sh/sh4.h"

#include "debugger.h"


namespace {

class segasp_state : public naomi_state
{
public:
	segasp_state(const machine_config &mconfig, device_type type, const char *tag) :
		naomi_state(mconfig, type, tag),
		m_sp_eeprom(*this, "sp_eeprom")
	{
	}

	void segasp(machine_config &config);

	void init_segasp();

private:
	required_device<eeprom_serial_93cxx_device> m_sp_eeprom;

	uint64_t sp_eeprom_r(offs_t offset, uint64_t mem_mask = ~0);
	void sp_eeprom_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t sp_rombdflg_r();
	uint64_t sp_io_r(offs_t offset, uint64_t mem_mask = ~0);
	uint64_t sn_93c46a_r();
	void sn_93c46a_w(uint64_t data);
	uint64_t sp_bank_r(offs_t offset, uint64_t mem_mask = ~0);
	void sp_bank_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint16_t m_sp_bank = 0;

	void onchip_port(address_map &map) ATTR_COLD;
	void segasp_map(address_map &map) ATTR_COLD;
};

uint64_t segasp_state::sp_bank_r(offs_t offset, uint64_t mem_mask)
{
	if (ACCESSING_BITS_32_63)
		return -1;
	return m_sp_bank;
}

void segasp_state::sp_bank_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_32_63)
		return;
	uint16_t bank = data & 0xffff;
	if (bank != m_sp_bank)
		logerror("SystemSP: G2 Bank set to %08X%s\n", (bank & 0x3fff) << 16, (bank & 0x4000) ? " decrypt ON" :"" );
	m_sp_bank = bank;
}

uint64_t segasp_state::sn_93c46a_r()
{
	int res;

	/* bit 3 is EEPROM data */
	res = m_eeprom->do_read() << 4;
	res |= (ioport("DSW")->read() << 4) & 0xC0; // note: only old REV D PCB have DSW 3-4 here, newer does not, always 0 readed
	return res;
}

void segasp_state::sn_93c46a_w(uint64_t data)
{
	/* bit 4 is data */
	/* bit 2 is clock */
	/* bit 5 is cs */
	m_eeprom->di_write((data & 0x8) >> 3);
	m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write((data & 0x4) ? ASSERT_LINE : CLEAR_LINE);
}

uint64_t segasp_state::sp_eeprom_r(offs_t offset, uint64_t mem_mask)
{
	if (ACCESSING_BITS_32_63)
		return -1;
	return m_sp_eeprom->do_read() << 4;
}

void segasp_state::sp_eeprom_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_32_63)
		return;
	m_sp_eeprom->di_write(data & 1);
	m_sp_eeprom->cs_write((data & 2) ? ASSERT_LINE : CLEAR_LINE);
	m_sp_eeprom->clk_write((data & 4) ? ASSERT_LINE : CLEAR_LINE);
}

uint64_t segasp_state::sp_rombdflg_r()
{
	// bit 0 - romboard type, 1 = M4
	// bit 1 - debug mode (enable easter eggs in BIOS, can boot game without proper eeproms/settings)
	return ioport("CFG")->read();
}

uint64_t segasp_state::sp_io_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t retval;

	int reg = offset * 2;
	int shift = 0;

	if (ACCESSING_BITS_32_63)
	{
		reg++;
		shift = 32;
	}

	switch (reg)
	{
	case 0x00/4:        // CN9 17-24 IN_PORT 0 (IN)
		retval = ioport("IN_PORT0")->read();
		break;
	case 0x04/4:        // CN9 41-48 IN_PORT 1 (IN)
		retval = ioport("IN_PORT1")->read();
		break;
	case 0x08/4:        // CN9 25-32 (I/O)
		retval = ioport("IN_PORT3")->read();
		break;
	case 0x18/4:        // IN_PORT 2
		// bit 0:3 - DIPSW 1-4
		// bit 4:5 - TEST/SERVICE SW, CN9 5-6 (old rev PCB only)
		retval = ioport("DSW")->read();
		retval |= ioport("IN_PORT2")->read();
		break;
	default:
		retval = -1;
	}

	return retval << shift;
}

// todo, base DC / Naomi stuff should be in it's own map, differences only here, same for Naomi 2 etc.
void segasp_state::segasp_map(address_map &map)
{
	/* Area 0 */
	map(0x00000000, 0x001fffff).mirror(0xa2000000).rom().region("maincpu", 0); // BIOS

	map(0x005f6800, 0x005f69ff).mirror(0x02000000).rw(FUNC(segasp_state::dc_sysctrl_r), FUNC(segasp_state::dc_sysctrl_w));
	map(0x005f6c00, 0x005f6cff).mirror(0x02000000).m(m_maple, FUNC(maple_dc_device::amap));
	map(0x005f7000, 0x005f70ff).mirror(0x02000000).m(m_naomig1, FUNC(naomi_g1_device::submap)).umask64(0x0000ffff0000ffff);
	map(0x005f7400, 0x005f74ff).mirror(0x02000000).m(m_naomig1, FUNC(naomi_g1_device::amap));
	map(0x005f7800, 0x005f78ff).mirror(0x02000000).m(m_g2if, FUNC(dc_g2if_device::amap));
	map(0x005f7c00, 0x005f7cff).mirror(0x02000000).m(m_powervr2, FUNC(powervr2_device::pd_dma_map));
	map(0x005f8000, 0x005f9fff).mirror(0x02000000).m(m_powervr2, FUNC(powervr2_device::ta_map));
	map(0x00600000, 0x006007ff).mirror(0x02000000).rw(FUNC(segasp_state::dc_modem_r), FUNC(segasp_state::dc_modem_w));
	map(0x00700000, 0x00707fff).mirror(0x02000000).rw(FUNC(segasp_state::dc_aica_reg_r), FUNC(segasp_state::dc_aica_reg_w));
	map(0x00710000, 0x0071000f).mirror(0x02000000).rw("aicartc", FUNC(aicartc_device::read), FUNC(aicartc_device::write)).umask64(0x0000ffff0000ffff);

	map(0x00800000, 0x00ffffff).mirror(0x02000000).rw(FUNC(segasp_state::soundram_r), FUNC(segasp_state::soundram_w));           // sound RAM (8 MB)

	/* External Device */
	map(0x01000000, 0x0100ffff).ram(); // banked access to ROM/NET board address space, mainly backup SRAM and ATA
	map(0x01010000, 0x01010007).rw(FUNC(segasp_state::sp_bank_r), FUNC(segasp_state::sp_bank_w));
//  map(0x01010080, 0x01010087) IRQ pending/reset, ATA control
	map(0x01010100, 0x01010127).r(FUNC(segasp_state::sp_io_r));
	map(0x01010128, 0x0101012f).rw(FUNC(segasp_state::sp_eeprom_r), FUNC(segasp_state::sp_eeprom_w));
	map(0x01010150, 0x01010157).r(FUNC(segasp_state::sp_rombdflg_r));
//  map(0x01010180, 0x010101af) custom UART 1
//  map(0x010101c0, 0x010101ef) custom UART 2

	/* Area 1 */
	map(0x04000000, 0x04ffffff).mirror(0x02000000).ram().share("dc_texture_ram");      // texture memory 64 bit access
	map(0x05000000, 0x05ffffff).mirror(0x02000000).ram().share("frameram"); // apparently this actually accesses the same memory as the 64-bit texture memory access, but in a different format, keep it apart for now

	/* Area 2*/
	map(0x08000000, 0x09ffffff).mirror(0x02000000).noprw(); // 'Unassigned'

	/* Area 3 */
	map(0x0c000000, 0x0dffffff).mirror(0xa2000000).ram().share("dc_ram");

	/* Area 4 */
	map(0x10000000, 0x107fffff).mirror(0x02000000).w(m_powervr2, FUNC(powervr2_device::ta_fifo_poly_w));
	map(0x10800000, 0x10ffffff).w(m_powervr2, FUNC(powervr2_device::ta_fifo_yuv_w));
	map(0x11000000, 0x11ffffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath0_w)); // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE0 register - cannot be written directly, only through dma / store queue)
	/*       0x12000000 -0x13ffffff Mirror area of  0x10000000 -0x11ffffff */
	map(0x13000000, 0x13ffffff).w(m_powervr2, FUNC(powervr2_device::ta_texture_directpath1_w)); // access to texture / framebuffer memory (either 32-bit or 64-bit area depending on SB_LMMODE1 register - cannot be written directly, only through dma / store queue)

	/* Area 5 */
	//map(0x14000000, 0x17ffffff).noprw(); // MPX Ext.

	/* Area 6 */
	//map(0x18000000, 0x1bffffff).noprw(); // Unassigned

	/* Area 7 */
	//map(0x1c000000, 0x1fffffff).noprw(); // SH4 Internal
}

void segasp_state::onchip_port(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(segasp_state::sn_93c46a_r), FUNC(segasp_state::sn_93c46a_w));
}


INPUT_PORTS_START( segasp )
	PORT_START("CFG")
	PORT_DIPNAME( 0x01, 0x01, "ROM Board type" )
	PORT_DIPSETTING(    0x00, "other" )
	PORT_DIPSETTING(    0x01, "M4-type" )
	PORT_DIPNAME( 0x02, 0x00, "BIOS Debug mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Monitor" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, "31 kHz" )
	PORT_DIPSETTING(    0x00, "15 kHz" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW:4")     // Must be ON, with off BIOS bootstrap will deadloop with green screen
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN_PORT0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("IN_PORT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xca, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN_PORT2")
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN_PORT3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END

void segasp_state::segasp(machine_config &config)
{
	naomi_aw_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &segasp_state::segasp_map);
	m_maincpu->set_addrmap(AS_IO, &segasp_state::onchip_port);

	EEPROM_93C46_16BIT(config, "main_eeprom");
	EEPROM_93C46_16BIT(config, "sp_eeprom");

// todo, not exactly NaomiM4 (see notes at top of driver) use custom board type here instead
	X76F100(config, "naomibd_eeprom");  // actually not present
	naomi_m4_board &rom_board(NAOMI_M4_BOARD(config, "rom_board", 0, "naomibd_eeprom", "pic_readout"));
	rom_board.irq_callback().set(FUNC(dc_state::g1_irq));
}

void segasp_state::init_segasp()
{
	set_drc_options();
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_BIOS(bios))

#define SEGASP_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "v101", "BOOT VER 1.01" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "epr-24236a.ic50", 0x000000, 0x200000, CRC(ca7df0de) SHA1(504c74d5fc96c53ef9f7753e9e37fb8b39cb628c) ) \
	ROM_SYSTEM_BIOS( 1, "v200", "BOOT VER 2.00" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "epr-24328.ic50", 0x000000, 0x200000, CRC(25f2ef00) SHA1(e58dec9f171e52b3ded213b3fcd9a0de8a438076) ) \
	ROM_SYSTEM_BIOS( 2, "v201", "BOOT VER 2.01" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "epr-24328a.ic50", 0x000000, 0x200000, CRC(03ec3805) SHA1(a8fbaea826ca257be0b2b86952f247254929e046) ) \
	ROM_REGION16_BE( 0x80, "main_eeprom", 0 ) \
	ROM_LOAD16_WORD( "mb_serial.ic57", 0x0000, 0x0080, CRC(e1e3c009) SHA1(66bc636c527389c3338f631d78c788b4bd4e93be) )

// net_firm_119.ic72  - Network/Media Board firmware VER 1.19(VxWorks), 1st half contain original 1.10 version
// fpr-24208a.ic72    - version 1.23, 1st half - 1.10
// fpr-24329.ic72     - version 1.21, 1st half - 1.20
// fpr-24407_123.ic72 - version 1.23, 1st half - 1.20
// fpr-24407.ic72     - version 1.25, 1st half - 1.20
#define SEGASP_NETFIRM \
	ROM_REGION( 0x200000, "netcpu", 0) \
	ROM_LOAD( "net_eeprom.ic74s",  0x00000000,    0x200, CRC(77cc5a6c) SHA1(cbfba546256b70bce6c6fd0030d7e2e410a25526) ) \
	ROM_LOAD( "net_firm_119.ic72", 0x00000000, 0x200000, CRC(a738ea1c) SHA1(d25187a973a7e166e70334f964363adf2be87257) ) \
	ROM_LOAD( "fpr-24208a.ic72",   0x00000000, 0x200000, CRC(a738ea1c) SHA1(3c32ddfb3c40be66b9fb2ba35fbfd5b534bb3da0) ) \
	ROM_LOAD( "fpr-24329.ic72",    0x00000000, 0x200000, CRC(a738ea1c) SHA1(d0d062a4089a2d3404df45eb015faaf7eee9b8c2) ) \
	ROM_LOAD( "fpr-24407.ic72",    0x00000000, 0x200000, CRC(a738ea1c) SHA1(fbcc3d119b47a6da4d194e3fe4a98126c7049edf) ) \
	ROM_LOAD( "fpr-24407_123.ic72",0x00000000, 0x200000, CRC(a738ea1c) SHA1(3f5a2fb03bbb1bd9af9fe32ad76a224c97aa9b7a) )

// keep M4 board code happy for now
#define SEGASP_MISC \
	ROM_REGION( 0x84, "naomibd_eeprom", ROMREGION_ERASEFF )

#define SEGASPEE_US \
	ROM_LOAD16_WORD( "mb_eeprom_us.ic54s", 0x0000, 0x0080, CRC(4186d7ab) SHA1(fc23da69a511a7643b6a066161bcfc3cdeeacf04) )

#define SEGASPEE_EXP \
	ROM_LOAD16_WORD( "mb_eeprom_exp.ic54s", 0x0000, 0x0080, CRC(947ddfad) SHA1(832a3db097af680d1d0eb9451b4650565f0cf8c7) )

#define SEGASP_US \
	ROM_REGION16_BE(0x80, "sp_eeprom", 0) \
	SEGASPEE_US

#define SEGASP_EXP \
	ROM_REGION16_BE(0x80, "sp_eeprom", 0) \
	SEGASPEE_EXP

#define SEGASP_JP \
	ROM_REGION16_BE(0x80, "sp_eeprom",  ROMREGION_ERASEFF)

ROM_START( segasp )
	SEGASP_BIOS
	SEGASP_NETFIRM
	SEGASP_MISC
	SEGASP_JP
	SEGASPEE_US
	SEGASPEE_EXP

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)
	ROM_PARAMETER( ":rom_board:id", "5502" )
	ROM_REGION( 0x800, "pic_readout", ROMREGION_ERASE00 )
ROM_END

// probably satellite screen-less player I/O and hoppers controller units, while actual game running on main Lindbergh unit
// actual size is 16Mbyte, the rest is garbage leftover from some other game
ROM_START( aminosan )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(3d15e610) SHA1(00b040365eaf23f4d6ffe9927e264836d0cdaba1) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(0db8a64a) SHA1(6b6144d1e9b90cb1beb4ac65801ee0b261339106) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0613-jpn.ic15", 0, 0x800, CRC(cf3071b9) SHA1(27044f8d51ce3d2662847ff3f58dd4302935b9c8) )
ROM_END

ROM_START( brickppl )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x10000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(d79afdb6) SHA1(328e535980624d9173164b756ebbdc1ca4cb6f18) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(4f3c0937) SHA1(72d68b66c57ff539b8058f80f1a15ffa44095460) )
	ROM_LOAD( "ic64",  0x08000000, 0x4000000, CRC(383e90d9) SHA1(eeca4b1bd0cd1fed7b85f045d71e0c7258d4350b) )
	ROM_LOAD( "ic65",  0x0c000000, 0x4000000, CRC(4c29b5ac) SHA1(9e6a79ad2d2498eed5b2590c8764222e7d6c0229) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0558-com.ic15", 0, 0x800, BAD_DUMP CRC(7592d004) SHA1(632373d807f54953d68c95a9f874ed3e8011f085) )
ROM_END

ROM_START( bingogal )
	SEGASP_BIOS
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x01000000, CRC(c07d9870) SHA1(5d40c14c398c11908f05ef1fd274aa3818409fc6) )
	ROM_LOAD( "ic63",  0x01000000, 0x01000000, CRC(5d85e6c0) SHA1(c09e843399fa4855ea149564480adbdf02dcc182) )
	ROM_LOAD( "ic64",  0x02000000, 0x01000000, CRC(73134f52) SHA1(63e32fbbd15bb527d9b840dcc92bb9dd86483ae3) )
	ROM_LOAD( "ic65",  0x03000000, 0x01000000, CRC(1e4ae511) SHA1(55b4f9dc86f7da8db9e4875a6ee120228be42591) )
	ROM_LOAD( "ic66s", 0x04000000, 0x01000000, CRC(810d5dfc) SHA1(6998a622d0a4be27ba6d1fcfb2a89586f269b59e) )
	ROM_LOAD( "ic67s", 0x05000000, 0x01000000, CRC(92014e31) SHA1(6a5cf75da4c81dc55386996b6e62bbb4594591e8) )
	ROM_LOAD( "ic68s", 0x06000000, 0x01000000, CRC(0640172c) SHA1(44ccf6919922a1ce8ffd49ad3306c68b15193a71) )
	ROM_LOAD( "ic69s", 0x07000000, 0x01000000, CRC(ca26fbf9) SHA1(fe131e23109d4ff2b79ce53b79c22009b2078c85) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0513-jpn.ic15", 0, 0x800, BAD_DUMP CRC(778dc297) SHA1(a920ab31ea670cc5056c40baea3b832b7868bfe7) )
ROM_END

// This is installer of whole game machine.
// CF card contains Main unit binary, as well as Satellite, which is sent and programmed from main to satellites via network.
// There is also network board firmware update v1.19 and firmware for SH4-based I/O board.
ROM_START( bingogalo )
	SEGASP_BIOS
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)

	// BINGO GALAXY
	// MDA-C0039
	// REV. A
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0039a", 0, SHA1(950c27cc0cd77b2dd4e6d510da83699d8b511907) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0513-jpn.ic15", 0, 0x800, BAD_DUMP CRC(778dc297) SHA1(a920ab31ea670cc5056c40baea3b832b7868bfe7) )
ROM_END

// Also was dumped 837-14789 PCB, which uses 2x 512Mbit Flash ROMs. Game contents is the same as joined IC 62-64 dumps below.
ROM_START( bingogals )
	SEGASP_BIOS
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x01000000, CRC(880eb905) SHA1(7afdb154329d49c85b10316d62aef4934e9a5479) )
	ROM_LOAD( "ic63",  0x01000000, 0x01000000, CRC(41dab407) SHA1(d4582e6d8a0e67e6bfebcb336a4c8392f7cdba39) )
	ROM_LOAD( "ic64",  0x02000000, 0x01000000, CRC(97dfb2ab) SHA1(97f95643145717b199cf79020d5e81bf913a96a7) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", ROMREGION_ERASEFF) // not populated
ROM_END

ROM_START( bingopar )
	SEGASP_BIOS
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)

	// BINGO PARADE MAIN
	// MDA-C0008
	// REV. E
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0008e", 0, SHA1(a825c4d98d35d8f3ebc3df9579ba42b29e53346b) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0407-jpn.ic15", 0, 0x800, CRC(186d9d36) SHA1(21e5576a603eb4d624528012112f9e483b71cbad) )
ROM_END

ROM_START( dinoking )
	SEGASP_BIOS
	SEGASP_US
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x01000000, CRC(8bd18bf7) SHA1(8972ed2bf5bc2f8af9b864118f7a22940c392079) )
	ROM_LOAD( "ic63",  0x01000000, 0x01000000, CRC(8e8c8d1b) SHA1(4ec4a91515e57524d82a0cb98beabe8a286a5cd1) )
	ROM_LOAD( "ic64",  0x02000000, 0x01000000, CRC(01b32ff7) SHA1(27301813ccd895b16a247ebed5edc3d8e3eab334) )
	ROM_LOAD( "ic65",  0x03000000, 0x01000000, CRC(4b60cdb3) SHA1(4ad3e97845d6bdd8b32369aa23e622e066c8ba67) )
	ROM_LOAD( "ic66s", 0x04000000, 0x01000000, CRC(ee3c278e) SHA1(3273a9f1eace78f65ba25bf0f6fcaa77fa421fc4) )
	ROM_LOAD( "ic67s", 0x05000000, 0x01000000, CRC(42441393) SHA1(7ba94bc12ace699ea1159cece3d070fb35789d31) )
	ROM_LOAD( "ic68s", 0x06000000, 0x01000000, CRC(4a787a44) SHA1(4d8f348466187fb67ffff8605be151cea1f77ec6) )
	ROM_LOAD( "ic69s", 0x07000000, 0x01000000, CRC(c78e46c2) SHA1(b8224c68face23010414d13ebb4cc05a2a9dce8a) )


	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0408-com.ic15", 0, 0x800, CRC(f77c49dc) SHA1(e10173bbbd5930ed159cec9a7dba308e2a3f3c43) )
ROM_END

ROM_START( galilfac )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF )
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(37fda864) SHA1(e4cdb7b35a97285d609d445e7411200fd1cd1df4) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(c16707a7) SHA1(0dd9dc670a932b1a008f477f8cd7b68bce451a96) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-unk-jpn.ic15", 0, 0x800, BAD_DUMP CRC(f2233b04) SHA1(c44e717db0185634be79a59c005c0b96f6511962) ) // bruteforced, dumped PCB had no PIC key
ROM_END

ROM_START( isshoni )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x20000000, "rom_board", ROMREGION_ERASEFF )
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(82a71af8) SHA1(368e197d4f5393506e1a0c6bcd31a1c4a650b6a6) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(d4f65679) SHA1(0921fc06d08c21c00607832c009f5b8f977f2b0f) )
	ROM_LOAD( "ic64",  0x08000000, 0x4000000, CRC(28c764c8) SHA1(4c8c023e2da59f7f98c9d1ad6ff94a0ebfee7167) )
	ROM_LOAD( "ic65",  0x0c000000, 0x4000000, CRC(906e7df8) SHA1(0553658ac3b1130203c993b2afe40359d65fab6f) )
	ROM_LOAD( "ic66s", 0x10000000, 0x4000000, CRC(7815e178) SHA1(490eb5439ad2d2d38dbfdf659178522abc29e8f6) )
	ROM_LOAD( "ic67s", 0x14000000, 0x4000000, CRC(8d2c6567) SHA1(42120e99364f33f856ff1085e19e9600d8ea009d) )
	ROM_LOAD( "ic68s", 0x18000000, 0x4000000, CRC(d8213da3) SHA1(818bdceb326854c4d45bcbea309356e3e2c98051) )
	// IC69S populated, empty

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0496-jpn.ic15", 0, 0x800, CRC(7bca4250) SHA1(241c335deeaccd079028603432bf0f87185cb46d) )
ROM_END

ROM_START( kingyo )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x01000000, CRC(02f8ee37) SHA1(b3c3e28a25d73a5d465e3ac72a393dc8036fe4d4) )
	ROM_LOAD( "ic63",  0x01000000, 0x01000000, CRC(98b1738a) SHA1(f5e300e7179d9df0bd50055b0c7a780934b526ab) )
	ROM_LOAD( "ic64",  0x02000000, 0x01000000, CRC(27998de0) SHA1(12321f42472c96461e970287e3edf6a685577350) )
	ROM_LOAD( "ic65",  0x02000000, 0x01000000, CRC(548b3a81) SHA1(071f3373050b75931027f1ef3e1197acb9f069e4) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-5151-jpn.ic15", 0, 0x800, CRC(d29c911a) SHA1(5e256fd90e6ce039dd065dfc74d3aff999c46de0) )
ROM_END

ROM_START( lovebery )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(1bd80ed0) SHA1(d50307573389ebe71e381a75deb83811fa397b94) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(d3870287) SHA1(efd3630d54068f5a8caf242a48db410bedf48e7a) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0446-com.ic15", 0, 0x800, CRC(9e519dc6) SHA1(69a1efc81c9dc44fcb8fc9c17bd7f879d3259950) )

	// LOVE AND BERRY
	// Dress up and Dance!
	// Type-3
	// 800
	// note: this dump from "empty/dead" Management Chip with no game run count left
	ROM_REGION( 0x80, "rf_tag", 0 )
	ROM_LOAD( "berry_type3.bin", 0, 0x80, CRC(0c58aabd) SHA1(8e5d8c9fd2c84e93b442192682930cf4da3fcf79) )
ROM_END

ROM_START( lovebero )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(0a23cea3) SHA1(1780d935b0d641769859b2022df8e4262e7bafd8) ) // sldh
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(d3870287) SHA1(efd3630d54068f5a8caf242a48db410bedf48e7a) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0446-com.ic15", 0, 0x800, CRC(9e519dc6) SHA1(69a1efc81c9dc44fcb8fc9c17bd7f879d3259950) )
ROM_END

ROM_START( magicpop )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(f52306db) SHA1(4a12d12d0c14cd6363b4ccb384ae5719b43c627c) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(1e5bad78) SHA1(cc254e8bfc874dadf58af9f0562c9b81842fff09) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0474-jpn.ic15", 0, 0x800, BAD_DUMP CRC(711ba0aa) SHA1(b413889e80e7660ac8ac735bbb0f35485ba6cde6) )
ROM_END

ROM_START( manpuku )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x04000000, CRC(a6712d70) SHA1(afbcf4dca754600d632de3ae14f578652be7ba83) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0640-jpn.ic15", 0, 0x800, CRC(2fe1e48c) SHA1(50a684afd3bdcf0eff59df859d6ebb3b8534ec07) )
ROM_END

// Mirage World
// "coin pusher" type medal game machine, consists of "Main", "Center" and several "Satellite" units connected via Ethernet.
// Only Satellite unit was dumped, Main and Center units is missing.
// 834-14713 SP MRW SATL
ROM_START( mirworld )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(beda1ded) SHA1(a493a298d8fbe1fa2c66423da2dee378e691a7f4) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(ec7e58cb) SHA1(8c6a45da31cd87a71b8e2f7454dc31bc2891210b) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", ROMREGION_ERASEFF ) // not populated
ROM_END

ROM_START( ochaken )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(7eb89b69) SHA1(5991c72df7ee68073f6de158f6ddf3f0490444ac) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(e52d7885) SHA1(96485af39b7cbf3c7bfd403f673eb8678077bbe8) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	// no PIC was provided with game board, brute forced key
	ROM_LOAD( "317-unknown.ic15", 0, 0x800, BAD_DUMP CRC(0a6e8627) SHA1(01a0b66bffbf7caca8199b132a6014813f04843f) )
ROM_END

ROM_START( puyomedal )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(4fcfb8c7) SHA1(d1e7dfe7ff95d433c24a27befbb81de2a3527203) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(0db8a64a) SHA1(6b6144d1e9b90cb1beb4ac65801ee0b261339106) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0568-jpn.ic15", 0, 0x800, CRC(313e6987) SHA1(9bec2a2806e4ba018518b9f3cc157c35d08a0490) )
ROM_END

ROM_START( shateki )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x01000000, CRC(c463ae32) SHA1(7f4aca80c0a49caa589d1338464b71c4d33f0bd6) )
	ROM_LOAD( "ic63",  0x01000000, 0x01000000, CRC(2acf1456) SHA1(028eae0ff11857e991f26355c1f405e3bcbd33f3) )
	ROM_LOAD( "ic64",  0x02000000, 0x01000000, CRC(fd04ebf8) SHA1(07f6287697deccb1e9c3f1da22c4d8336dfbc41e) )
	ROM_LOAD( "ic65",  0x03000000, 0x01000000, CRC(0c0ed06b) SHA1(1ed0d1624ad304095323aacaab53519fe62b8330) )
	ROM_LOAD( "ic66s", 0x04000000, 0x01000000, CRC(3a9f33cc) SHA1(af4c96ea3b539d631d15b7a66efdf985cd78f87c) ) // 4 below ROMs leftovers junk from some other game
	ROM_LOAD( "ic67s", 0x05000000, 0x01000000, CRC(8e08cf8b) SHA1(51da97b769823773d1b2bc7e7f40a5b51543642d) ) // keep them for now, may worth to remove later
	ROM_LOAD( "ic68s", 0x06000000, 0x01000000, CRC(14734999) SHA1(cd763fcd4bab5a3cfb09b4054c5358b70249f9b5) )
	ROM_LOAD( "ic69s", 0x07000000, 0x01000000, CRC(6c0cd4c5) SHA1(63963a3f8f1f90c8c78f0dc1f215666d3b7b1151) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0628-jpn.ic15", 0, 0x800, CRC(c02f7424) SHA1(901e7f17a8e9e2e265ce4b2ec2cc56649ae57b17) )
ROM_END

ROM_START( tetgiant )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASE)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(31ba1938) SHA1(9b5a05193b3df13cd7617a38913e0b0fbd61da44) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(cb946213) SHA1(6195e33c44a1e8eb464dfc3558dc1c9b4d910ef3) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0604-com.ic15", 0, 0x800, CRC(a46dfd47) SHA1(9e24739ecaaf85ef3b862485064450db6c607189) )
ROM_END

ROM_START( unomedal )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(2af0cfb3) SHA1(88e75e547e525d7752f365256ba22411d2f0c7d9) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(fe375da9) SHA1(b2c6ddf3f15ba801bfa1d0c6ae7ebde0ce6766e9) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0526-jpn.ic15", 0, 0x800, CRC(14232fb7) SHA1(5fb8f832c760081aec6fdaa3be8535e3d641ae13) )
ROM_END

ROM_START( westdrmg )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)
	ROM_LOAD( "ic62",  0x00000000, 0x4000000, CRC(67bde24c) SHA1(a236b69af04d3794092955918f217bf5cfbdde04) )
	ROM_LOAD( "ic63",  0x04000000, 0x4000000, CRC(f5901eab) SHA1(9a5a0ba3f444647d40866aa48f4b3b885c66edf7) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0473-jpn.ic15", 0, 0x800, BAD_DUMP CRC(8ec686b0) SHA1(a9e636df399654aeab5b35e44c9db8321c8cbd5a) )
ROM_END


ROM_START( dinokior )
	SEGASP_BIOS
	SEGASP_US
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// DINOSAUR KING
	// Operation:Dinosaur Rescue
	// MDA-C0021
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0021", 0, SHA1(947c987fb93a32c5acf7839e0186de91b5a9facc) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // actually 8x 128Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0408-com.ic15", 0, 0x800, CRC(f77c49dc) SHA1(e10173bbbd5930ed159cec9a7dba308e2a3f3c43) )
ROM_END

ROM_START( dinoki25 )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v200" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// DINOSAUR KING
	// VER2.5 ENG_ASIA
	// MDA-C0047
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0047", 0, SHA1(0f97291d9c5dbe3e66a5220da05aebdfaa78b35d) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0408-com.ic15", 0, 0x800, CRC(f77c49dc) SHA1(e10173bbbd5930ed159cec9a7dba308e2a3f3c43) )

	// DINOSAUR KING
	// TYPE-3
	// 800
	// note: this dump from "empty/dead" Management Chip with no game run count left
	ROM_REGION( 0x80, "rf_tag", 0 )
	ROM_LOAD( "dino_type3.bin", 0, 0x80, CRC(1b6c9ea7) SHA1(2e56a1969c49c347f7facda187e5bf787c74328c) )
ROM_END

// This game's protection uses contact IC card reader made by Hirocon, with 2 IC card slots, cards probably SLE4428 or SLE5528.
ROM_START( dinokich )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v200" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// DINOSAUR KING
	// MDA-C0081
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0081", 0, SHA1(c9b8449666124f52b175a3d3d795bc0e7ea4d91e) ) // unused space contain leftovers from "Disney Magical Dream Dance" game, encrypted, key is 1ffbc9f0

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0408-com.ic15", 0, 0x800, CRC(f77c49dc) SHA1(e10173bbbd5930ed159cec9a7dba308e2a3f3c43) )
ROM_END

// This version does not use RFID readers and Management chips.
ROM_START( dinoki4 )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v200" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// 古代王者 恐竜キング
	// 目覚めよ！ 新たなる力！！
	// MDA-C0061
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0061", 0, BAD_DUMP SHA1(d9e21aff3d33cc5d3d97decacf963cff23c60ff1) ) // BAD_DUMP note: actual game files is all good and genuine, but image itself was modified and not original.

	ROM_PARAMETER( ":rom_board:id", "5502" )

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0408-com.ic15", 0, 0x800, CRC(f77c49dc) SHA1(e10173bbbd5930ed159cec9a7dba308e2a3f3c43) )
ROM_END

ROM_START( huhimage )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v200" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// TSB0-SEGA00001
	DISK_REGION( "cflash" )
	DISK_IMAGE( "tsb0-sega00001", 0, SHA1(2e5703783e5200ef37845d3ca1f862c3541cc21d) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0606-jpn.ic15", 0, 0x800, CRC(75638f59) SHA1(2bf8cb4547034f5c2a14e485c9a3f9fcfb7c609e) )
ROM_END

ROM_START( kingyoch )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// YATAIMURA
	// KINGYOSUKUI EX
	DISK_REGION( "cflash" )
	DISK_IMAGE( "kingyo", 0, SHA1(f3685a0c6109b78f4812972b71bcec4ef9e9198f) ) // free space contain other game leftovers, presumable Issyouni Wan Wan, encrypted, key is 511248c2

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	// no PIC was provided with CF card, brute forced key
	ROM_LOAD( "317-0563-jpn.ic15", 0, 0x800, BAD_DUMP CRC(8af67833) SHA1(0b79abf9182c249a6d4976d6fd3b90101d66354f) )
ROM_END

ROM_START( loveber3 )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v200" )
	SEGASP_US
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// LOVE AND BERRY III ENG
	// MDA-C0042
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0042", 0, SHA1(9992d90dae8ce7636e4153e02b779c27931b3be6) )

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0446-com.ic15", 0, 0x800, CRC(9e519dc6) SHA1(69a1efc81c9dc44fcb8fc9c17bd7f879d3259950) )
ROM_END

// This game's protection uses contact IC card reader made by Hirocon, with 2 IC card slots, cards probably SLE4428 or SLE5528.
ROM_START( loveber3cn )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_EXP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// LOVE AND BERRY III CHN
	// MDA-C0071
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0071", 0, SHA1(ea65def6d97dc76c08654fae3c89b80a0492575b) ) // unused space contain leftovers from "Disney Magical Dream Dance" game, encrypted, key is 1ffbc9f0

	ROM_PARAMETER( ":rom_board:id", "5508" )  // 8x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0446-com.ic15", 0, 0x800, CRC(9e519dc6) SHA1(69a1efc81c9dc44fcb8fc9c17bd7f879d3259950) )
ROM_END

ROM_START( tetgiano )
	SEGASP_BIOS
	ROM_DEFAULT_BIOS( "v201" )
	SEGASP_JP
	SEGASP_MISC

	ROM_REGION( 0x08000000, "rom_board", ROMREGION_ERASEFF)

	// TETRIS - DEKARIS (romaji)
	// / TETRIS® - GIANT
	// MDA-C0076
	DISK_REGION( "cflash" )
	DISK_IMAGE( "mda-c0076", 0, SHA1(6987c888d2a3ada2d07f6396d47fdba507ca859d) )

	ROM_PARAMETER( ":rom_board:id", "5502" )  // 2x 512Mbit FlashROMs

	ROM_REGION( 0x800, "pic_readout", 0 )
	ROM_LOAD( "317-0604-com.ic15", 0, 0x800, CRC(a46dfd47) SHA1(9e24739ecaaf85ef3b862485064450db6c607189) )
ROM_END

} // anonymous namespace


#define GAME_FLAGS (MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_SOUND)

GAME( 2004, segasp,  0,          segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Sega System SP (Spider) BIOS", GAME_FLAGS | MACHINE_IS_BIOS_ROOT )
// These use ROMs
GAME( 2010, aminosan,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Aminosan (satellite)", GAME_FLAGS )
GAME( 2009, bingogal,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Bingo Galaxy (main)", GAME_FLAGS ) // 28.05.2009
GAME( 2009, bingogals,segasp,    segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Bingo Galaxy (satellite)", GAME_FLAGS ) // 28.05.2009
GAME( 2009, brickppl,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Brick People / Block PeePoo (Ver 1.002)", GAME_FLAGS )
GAME( 2005, dinoking,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Dinosaur King (USA)", GAME_FLAGS )
GAME( 2008, galilfac,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Galileo Factory (main)", GAME_FLAGS )
GAME( 2008, isshoni,segasp,      segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Isshoni Wanwan Waiwai Puppy 2008", GAME_FLAGS ) // いっしょにワンワンわいわいパピー 2008
GAME( 2009, kingyo,segasp,       segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Yataimura Kingyosukui (1-player, Japan, Ver 1.005)", GAME_FLAGS ) // キッズ屋台村 金魚すくい
GAME( 2006, lovebery,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Love And Berry - 1st-2nd Collection (Export, Ver 2.000)", GAME_FLAGS )
GAME( 2006, lovebero,lovebery,   segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Love And Berry - 1st-2nd Collection (Export, Ver 1.003)", GAME_FLAGS )
GAME( 2009, magicpop,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Magical Poppins", GAME_FLAGS )
GAME( 2013, manpuku,segasp,      segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Manpuku Suizokukan", GAME_FLAGS ) // まんぷくすいぞくかん
GAME( 2007, mirworld,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Mirage World (satellite)", GAME_FLAGS )
GAME( 2007, ochaken, segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Ocha-Ken Hot Medal", GAME_FLAGS )
GAME( 2009, puyomedal,segasp,    segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Puyo Puyo! The Medal Edition", GAME_FLAGS )
GAME( 2010, shateki,segasp,      segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Yataimura Shateki (1-player, Japan, Ver 1.000)", GAME_FLAGS )
GAME( 2009, tetgiant,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Tetris Giant / Tetris Dekaris (Ver.2.000)", GAME_FLAGS )
GAME( 2009, unomedal,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "UNO the Medal", GAME_FLAGS )
GAME( 2009, westdrmg,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Western Dream Gold", GAME_FLAGS )
// These use a CF card
GAME( 2007, bingogalo,bingogal,  segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Bingo Galaxy (main) (MDA-C0039A)", GAME_FLAGS ) // 31.10.2007(main)/15.11.2007(satellite)
GAME( 2005, bingopar,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Bingo Parade (main) (MDA-C0008E)", GAME_FLAGS ) // 31.10.2007(main)/15.11.2007(satellite)
GAME( 2006, dinokior,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Dinosaur King - Operation: Dinosaur Rescue (USA, Export) (MDA-C0021)", GAME_FLAGS )
GAME( 2008, dinoki25,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Dinosaur King - D-Team VS. the Alpha Fortress (Export, Ver 2.500) (MDA-C0047)", GAME_FLAGS )
GAME( 2010, dinokich,dinoki25,   segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Konglongwang - D-Kids VS Alpha Yaosai (China, Ver 2.501) (MDA-C0081)", GAME_FLAGS ) // D-Kids VS 亚法要塞
GAME( 2008, dinoki4,segasp,      segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Kodai Ouja Kyouryuu King - Mezame yo! Arata-naru Chikara!! (Japan, Ver 4.000) (MDA-C0061)", GAME_FLAGS ) // Ancient Ruler Dinosaur King - Wake up! New Power!!
GAME( 2019, huhimage,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Heat Up Hockey Image (Ver.1.003R)", GAME_FLAGS )
GAME( 2009, kingyoch,kingyo,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Yataimura Kingyosukui (4-player, China, Ver 1.000)", GAME_FLAGS )
GAME( 2007, loveber3,segasp,     segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Love And Berry - 3rd-5th Collection (USA, Export, Ver 1.002) (MDA-C0042)", GAME_FLAGS )
GAME( 2010, loveber3cn,loveber3, segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Love And Berry - 3rd-5th Collection (China, Ver 1.001) (MDA-C0071)", GAME_FLAGS )
GAME( 2009, tetgiano,tetgiant,   segasp,    segasp, segasp_state, init_segasp, ROT0, "Sega", "Tetris Giant / Tetris Dekaris (MDA-C0076)", GAME_FLAGS )
