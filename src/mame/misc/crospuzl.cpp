// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Cross Puzzle

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - The RTC is not hooked up, so its test screen shows nothing useful.  It is
      not on I2C, which is what an earlier pcf8583 attempt assumed and why the
      traffic looked like an unrecognized slave address 0x30.  It is a three
      wire part on the same PIO: CE on bit 22, SCLK on bit 20 and a
      bidirectional data line on bit 19, the direction register picking who
      drives it.  The game bit bangs it 4 bits at a time, least significant
      first (__rtcSendNibble at 0x02018e58, __rtcReadNibble at 0x02018f6c), and
      a read - 0x02018fb2 - raises CE, sends a command nibble 0xc and a start
      address nibble 0, then clocks twelve nibbles back and pairs them into six
      BCD bytes, masking the tens of seconds and minutes to 0x70.  Twelve
      consecutive 4-bit registers from zero is the MSM6242 / RTC-62421 layout -
      S1 S10 MI1 MI10 H1 H10 D1 D10 MO1 MO10 Y1 Y10 - so the register model is
      that of the 4-bit family, but the interface plainly is not: those parts
      are parallel, and this one is clocked a nibble at a time over three
      wires.  Whatever it is, it is a serial part carrying that register map.
    - Lamps / counters.
    - Are there really DIPs on PCB or is it an assumption?
    - Is there a payout button or is it game-driven? The hopper test works.

    Notes:
    - Game enables UART1 receive irq, if that irq is enable it just prints
      "___sysUART1_ISR<LF>___sysUART1_ISR_END<LF>"

=============================================================================

 This PCB uses ADC 'Amazon-LF' SoC, EISC CPU core - However PCBs have been
 seen with a standard VRenderZERO+ MagicEyes EISC chip

=============================================================================

 The boot ROM is a two stage affair.  Only 0x430-0xed1c is used; at 0x2000
 sits a u32 little endian decompressed length (0x126ee) followed by an LZSS
 stream, which the routine at PC=0xc8a expands to 0x02400000 before jumping
 there.  The format is textbook LZSS: 0x1000-byte ring buffer (kept at
 0x02700010, write index starting at 0), flag byte MSB first, set bit means
 literal, clear bit means a big endian 16-bit token holding a 12-bit absolute
 ring position and a 4-bit length-3.  Everything the loader does with the NAND
 lives in that second stage, which is why no NAND code is visible in the ROM
 itself.  Its string table identifies the platform as "Titan2".  The files it
 loads out of the flash use the same LZSS.

 The file system on the NAND starts at byte 0x20000 (page 0x40) with a
 "MGS File system." header; +0x14 holds the payload size and +0x18 the number
 of entries.  The 0x20-byte directory entries start at 0x20100:

     +0x00  u32   flags (bit 0 compressed, bit 1 name stored complemented)
     +0x04  u32   uncompressed size
     +0x08  u32   stored size
     +0x0c  u32   offset of the payload within the file system
     +0x10  char  name[16]

 They are encrypted with the board's 1-Wire serial number:

     idx      = (fs_offset + i) & 0xff            i = 0 .. 0x1f
     plain[i] = cipher[i] ^ tableB[idx] ^ serial[tableA[idx]]

 with tableA at 0x0240fac1 and tableB at 0x0240f9c1 in the decompressed stage
 two, and serial[0..5] the six unique bytes of the DS2401 ROM id (family code
 dropped, CRC unused).  __nfsGetSerial (0x024054cc, via 0x02405414) bit bangs a
 1-Wire READ ROM on PIO bit 25 - 480us reset, 6/60us time slots, command 0x33,
 eight bytes back checked with the Dallas CRC-8 at 0x024053d8.  If that fails,
 0x02404446 gives up and fills every directory entry with 0xff, so no file is
 ever found and the POST stalls right after "System memory test : O.K".

 The part itself was never dumped.  Since tableA only indexes 0..5 the six key
 bytes are independent, and they were recovered by known plaintext from the
 three names the loader looks for - "_autorun.bin", "_t2boot.bin" and
 "_sdata.bin" - remembering that names are stored complemented and flipped back
 by 0x024045a0.  Those three pin all six bytes with no conflict, and the other
 six entries then decrypt to 8x8.pix, mdk_font.pix, t_f_01.pix, t_f_02.pix,
 g_f_01.pix and g_f_02.pix: nine files, matching the count in the header.  The
 nine offsets chain exactly (offset + stored size == next offset) and the last
 ends at 0x445e2c, which is the figure a real board prints in its
 "Flash:[Cross_Puzzle.Res, 640, 4480556/134000656]" boot line.

 The bus is bit banged by 0x02405232: bit 2 of its argument set means release,
 which clears bits 8-9 of the pin mux at 0x0180001c and lets the pull up win;
 clear means drive low, which sets mux bit 8, clears bit 25 of the direction
 register at 0x01802000 and clears bit 25 of the latch at 0x01802004.  Sampling
 reads bit 25 of 0x01802008.  Note the line is never released through the
 direction register, so hooking only the PIO leaves it stuck low.

 __nfpCheckBusy at 0x02401208 waits for the NAND ready/busy line to fall
 before waiting for it to rise again, so nand_device has to hold is_busy()
 for the duration of an operation rather than strobe its callback and stay
 ready: with a part that never goes busy the first wait times out and the
 loader gives up before reading a single page.

****************************************************************************/

#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/ds2401.h"
#include "machine/nandflash.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/vrender0.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class crospuzl_state : public driver_device
{
public:
	crospuzl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_serial_id(*this, "serial_id"),
		m_hopper(*this, "hopper"),
		m_nand(*this, "nand")
	{ }


	void crospuzl(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<u32> m_workram;

	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<ds2401_device> m_serial_id;
	required_device<hopper_device> m_hopper;
	required_device<nand_device> m_nand;

	u32 m_pio = 0;
	u32 m_ddr = 0;
	u32 m_pinmux = 0;

	// 1-Wire line, on PIO bit 25.  The loader does not release it through the
	// direction register: it detaches the pad from the PIO in the pin mux at
	// 0x0180001c instead (bits 8-9 cleared), leaving the bus pulled up.  To
	// drive it low it re-attaches the pad (bit 8 set), sets the direction to
	// output and clears the latch.  So all three registers take part, and the
	// line only reads low when every one of them says so.
	static constexpr int SERIAL_ID_BIT = 25;

	void update_serial_id()
	{
		const bool attached = BIT(m_pinmux, 8);
		const bool driving = attached && !BIT(m_ddr, SERIAL_ID_BIT);
		m_serial_id->write((driving && !BIT(m_pio, SERIAL_ID_BIT)) ? 0 : 1);
	}

	void hopper_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// PIO
	u32 piolddr_r();
	void piolddr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioldat_r();
	void pioldat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 pioedat_r();

	u32 pinmux_r();
	void pinmux_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void main_map(address_map &map) ATTR_COLD;
};


void crospuzl_state::hopper_w(offs_t offset, u32 data, u32 mem_mask)
{
	m_hopper->motor_w(BIT(data, 4));

	if (data & 0xffffffef)
		logerror("%s hopper_w unknown bits written: %08x\n", machine().describe_context(), data);
}

u32 crospuzl_state::pioedat_r()
{
	return (m_serial_id->read() << SERIAL_ID_BIT)
		| (machine().rand() & 0x04000000); // NAND ready/busy line
	// TODO: correct busy line support in nandflash.cpp, then (m_nand->is_busy() ? 0 : 0x04000000)
}

// Pin function select.  The loader read-modify-writes bits 8-9 here to route
// the 1-Wire pin, so it has to read back what it wrote.
u32 crospuzl_state::pinmux_r()
{
	return m_pinmux;
}

void crospuzl_state::pinmux_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pinmux);
	update_serial_id();
}

u32 crospuzl_state::piolddr_r()
{
	return m_ddr;
}

void crospuzl_state::piolddr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ddr);
	update_serial_id();
}

u32 crospuzl_state::pioldat_r()
{
	return m_pio;
}

// PIO Latched output DATa Register
void crospuzl_state::pioldat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio);
	update_serial_id();
}

void crospuzl_state::main_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

	map(0x01500000, 0x01500000).rw(m_nand, FUNC(nand_device::data_r), FUNC(nand_device::data_w));
	map(0x01500100, 0x01500100).w(m_nand, FUNC(nand_device::command_w));
	map(0x01500200, 0x01500200).w(m_nand, FUNC(nand_device::address_w));
	map(0x01510000, 0x01510003).portr("IN0"); // .w ??
	map(0x01511000, 0x01511003).portr("IN1"); // .w ??
	map(0x01512000, 0x01512003).portr("IN2").w(FUNC(crospuzl_state::hopper_w));
	map(0x01513000, 0x01513003).portr("IN3"); // .w ??

	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x0180001c, 0x0180001f).rw(FUNC(crospuzl_state::pinmux_r), FUNC(crospuzl_state::pinmux_w));
	map(0x01802000, 0x01802003).rw(FUNC(crospuzl_state::piolddr_r), FUNC(crospuzl_state::piolddr_w));
	map(0x01802004, 0x01802007).rw(FUNC(crospuzl_state::pioldat_r), FUNC(crospuzl_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(crospuzl_state::pioedat_r));

	map(0x02000000, 0x027fffff).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}

void crospuzl_state::machine_start()
{
	save_item(NAME(m_pio));
	save_item(NAME(m_ddr));
	save_item(NAME(m_pinmux));
}

void crospuzl_state::machine_reset()
{
	m_ddr = 0xffffffff;
	m_pinmux = 0;
	update_serial_id();
}


static INPUT_PORTS_START(crospuzl)
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button 1 / Start")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // this resets coins, but doesn't seem to be a payout (no hopper activation)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) // note?
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Test 2") // duplicate?
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void crospuzl_state::crospuzl(machine_config &config)
{
	// The real part runs somewhere around 80 MHz but averages about five cycles
	// per instruction, while this core retires one per cycle, so it is clocked
	// at a fifth of that to execute at the right rate.  The software timed delay
	// loop at 0x024052e6 is the yardstick: sixteen instructions per turn, called
	// with counts meant to be microseconds - 480 for the 1-Wire reset, 70 for
	// the presence sample, 6 and 10 for the bit slots - so one microsecond per
	// turn works out at exactly 16 MHz.  Clock it any faster and the DS2401
	// never sees a reset long enough to answer.  Revisit once the core counts
	// cycles.
	SE3208(config, m_maincpu, 14318180 * 3); // FIXME: 72 MHz-ish
	m_maincpu->set_clock_scale(0.26);
	m_maincpu->set_addrmap(AS_PROGRAM, &crospuzl_state::main_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 6); // FIXME: 72 MHz-ish
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(14318180 * 2); // Unknown clock, should output ~70 Hz?

	SAMSUNG_K9F1G08U0B(config, m_nand, 0);

	HOPPER(config, m_hopper, attotime::from_msec(50));

	DS2401(config, m_serial_id);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}

ROM_START( crospuzl )
	ROM_REGION( 0x80010, "maincpu", 0 )
	ROM_LOAD("en29lv040a.u5",  0x000000, 0x80010, CRC(d50e8500) SHA1(d681cd18cd0e48854c24291d417d2d6d28fe35c1) )

	// the following was AI-fixed to have the spare bytes at the end of every page instead of all at the end of the file.
	// original dump with following hashes: CRC(7f3c88c3) SHA1(db3169a7b4caab754e9d911998a2ece13c65ce5b)
	ROM_REGION( 0x8400000, "nand", 0 )
	ROM_LOAD("k9f1g08u0b.u1", 0x000000, 0x8400000, BAD_DUMP CRC(c89854f8) SHA1(2767b3ee5a18776ecc5a063065f5ccc885f3c208) )

	ROM_REGION( 0x08, "serial_id", 0 )
	ROM_LOAD( "ds2401.bin", 0x00, 0x08, BAD_DUMP CRC(59aa97e0) SHA1(0bd57a57f7274a088bea051cbf9c4c8a02a9ff83) ) // AI-generated, not dumped (but almost surely correct)

	ROM_REGION( 0x8000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0x0000, 0x8000, CRC(6b3dccbb) SHA1(0b879765409d03a6e6b3083008125e39c9027a17) ) // pre-initialized
ROM_END

} // anonymous namespace


GAME( 200?, crospuzl, 0, crospuzl, crospuzl, crospuzl_state, empty_init, ROT0, "<unknown>", "Cross Puzzle (v. 1.00)", MACHINE_NOT_WORKING )
