// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PDP-11

        Unibus models
        ==================
        PDP-11/20 and PDP-11/15 - The original, non-microprogrammed processor;
                designed by Jim O'Loughlin. Floating point was supported by
                peripheral options using various data formats.
        PDP-11/35 and PDP-11/40 - A microprogrammed successor to the PDP-11/20;
                the design team was led by Jim O'Loughlin.
        PDP-11/45, PDP-11/50, and PDP-11/55 ? A much faster microprogrammed processor
                that could use up to 256 kB of semiconductor memory instead of or in
                addition to core memory. First model to support an optional FP11
                floating-point coprocessor, which established the format used in
                later models.
        PDP-11/70 - The 11/45 architecture expanded to allow 4 MB of physical memory
                segregated onto a private memory bus, 2 kB of cache memory, and much
                faster I/O devices connected via the Massbus.[9]
        PDP-11/05 and PDP-11/10 - A cost-reduced successor to the PDP-11/20.
        PDP-11/34 and PDP-11/04 - Cost-reduced follow-on products to the 11/35
                and 11/05; the PDP-11/34 concept was created by Bob Armstrong.
                The 11/34 supported up to 256 kB of Unibus memory. The PDP-11/34a
                supported a fast floating-point option, and the 11/34c supported a
                cache memory option.
        PDP-11/60 - A PDP-11 with user-writable microcontrol store; this was
                designed by another team led by Jim O'Loughlin.
        PDP-11/44 - Replacement for the 11/45 and 11/70 that supported optional cache
                memory and floating-point processor, and included a sophisticated serial
                console interface and support for 4 MB of physical memory. The design
                team was managed by John Sofio.
        PDP-11/24 - First VLSI PDP-11 for Unibus, using the "Fonz-11" (F11) chip set
                with a Unibus adapter.
        PDP-11/84 - Using the VLSI "Jaws-11" (J11) chip set with a Unibus adapter.
        PDP-11/94 - J11-based, faster than 11/84.

        Q-bus models
        ==============
        PDP-11/03 (also known as the LSI-11/03) - The first LSI PDP-11, this system
                used a chipset from Western Digital and supported 60 kB of memory.
        PDP-11/23 - Second generation of LSI (F-11). Early units supported
                only 248 kB of memory.
        PDP-11/23+/MicroPDP-11/23 - Improved 11/23 with more functions on the
                (larger) processor card.
        MicroPDP-11/73 - The third generation LSI-11, this system used the
                faster "Jaws-11" (J-11) chip set and supported up to 4 MB of memory.
        MicroPDP-11/53 - Slower 11/73 with on-board memory.
        MicroPDP-11/83 - Faster 11/73 with PMI (private memory interconnect).
        MicroPDP-11/93 - Faster 11/83; final DEC Q-Bus PDP-11 model.
        KXJ11 - QBUS card (M7616) with PDP-11 based peripheral processor and
                DMA controller. Based on a J11 CPU equipped with 512 kB of RAM,
                64 kB of ROM, and parallel and serial interfaces.
        Mentec M100 - Mentec redesign of the 11/93, with J-11 chipset at 19.66 MHz,
                four on-board serial ports, 1-4 MB of on-board memory, and optional FPU.
        Mentec M11 - Processor upgrade board; microcode implementation of PDP-11
                instruction set by Mentec, using the TI 8832 ALU and TI 8818
                microsequencer from Texas Instruments.
        Mentec M1 - Processor upgrade board; microcode implementation of
                PDP-11 instruction set by Mentec, using Atmel 0.35 ?m ASIC.[10]
        Quickware QED-993 - High performance PDP-11/93 processor upgrade board.
        DECserver 500 and 550 LAT terminal servers DSRVS-BA using the KDJ11-SB chipset

        All PDP-11's execept the first one (11/15 and 11/20) are microprogrammed.

        23/02/2009 Skeleton driver.

        Memory Map (converted from the annoying octal format from the manuals):
        0x0000 - 0x00ff: irq vectors
        0xe000: ROM
        0xff68: "high speed reader and punch device status and buffer register"
        0xff70 - 0xff7e: "teletype keyboard and punch device status and buffer register"
        PDP-11 internal registers:
        0xff80 - 0xffbf: "reserved for expansion of processor registers"
        0xffc0: R0
        0xffc2: R1
        0xffc4: R2
        0xffc6: R3
        0xffc8: R4
        0xffca: R5
        0xffcc: R6 / SP
        0xffce: R7 / PC
        0xfffe: PSW

        SMS-1000:
        Claims to be 100% compatible with DEC PDP-11. Added as a skeleton.

****************************************************************************/

#include "emu.h"
#include "bus/qbus/qbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "cpu/i86/i186.h"
#include "machine/dl11.h"
#include "rx01.h"


namespace {

class pdp11_state : public driver_device
{
public:
	pdp11_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dl11(*this, "dl11")
		, m_rs232(*this, "rs232")
		, m_qbus(*this, "qbus")
	{ }

	void pdp11ub2(machine_config &config);
	void pdp11(machine_config &config);
	void pdp11qb(machine_config &config);
	void sms1000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<t11_device> m_maincpu;
	required_device<dl11_device> m_dl11;
	required_device<rs232_port_device> m_rs232;
	required_device<qbus_device> m_qbus;
	DECLARE_MACHINE_RESET(pdp11ub2);
	DECLARE_MACHINE_RESET(pdp11qb);
	void load9312prom(uint8_t *desc, uint8_t *src, int size);
	void pdp11_mem(address_map &map) ATTR_COLD;
	void pdp11qb_mem(address_map &map) ATTR_COLD;
	void sms1000_mem_188(address_map &map) ATTR_COLD;
};

void pdp11_state::pdp11_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xdfff).ram();  // RAM
	map(0xea00, 0xfeff).rom();
	map(0xff70, 0xff77).rw(m_dl11, FUNC(dl11_device::read), FUNC(dl11_device::write));

	map(0xfe78, 0xfe7b).w("rx01", FUNC(rx01_device::write));
}

void pdp11_state::pdp11qb_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xe9ff).ram();  // RAM
	map(0xea00, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void pdp11_state::sms1000_mem_188(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x027ff).ram();
	map(0x03000, 0x037ff).rom().region("prom", 0);
	map(0x40000, 0x47fff).rom().region("subcpu", 0);
	map(0x50000, 0x5ffff).nopr();
	map(0xf8000, 0xfffff).rom().region("subcpu", 0);
}

#define M9312_PORT_CONFSETTING \
PORT_CONFSETTING ( 0x00, "'DL' BOOT prom for RL11 controller") \
PORT_CONFSETTING ( 0x01, "'DM' BOOT prom for RK06/07 controller") \
PORT_CONFSETTING ( 0x02, "'DX' BOOT prom for RX01 compatible controller") \
PORT_CONFSETTING ( 0x03, "'DP/DB' BOOT prom for RP02/03,RP04/5/6 RM02/3 controller") \
PORT_CONFSETTING ( 0x04, "'DK/DT' BOOT prom for RK03/05,TU55/56 controllers") \
PORT_CONFSETTING ( 0x05, "'MM' BOOT prom for TU16/E16 TM02/3 controllers") \
PORT_CONFSETTING ( 0x06, "'MT' BOOT prom for TU10/TS03 controller") \
PORT_CONFSETTING ( 0x07, "'DS' BOOT prom for RS03/RS04 controller") \
PORT_CONFSETTING ( 0x08, "'PR/TT' BOOT prom for PC05,LO SPD RDR controllers") \
PORT_CONFSETTING ( 0x09, "'CT' BOOT prom for TA11/TU60 controller") \
PORT_CONFSETTING ( 0x0a, "'RS' BOOT prom for RS11, RS64 controller") \
PORT_CONFSETTING ( 0x0b, "'CR' BOOT prom for CR11 card reader") \
PORT_CONFSETTING ( 0x0c, "'MS' BOOT prom for TS11/TS04/TU80 compatible controller") \
PORT_CONFSETTING ( 0x0d, "'DD' BOOT prom for TU58 DECtapeII serial tape controller") \
PORT_CONFSETTING ( 0x0e, "'DU' BOOT prom for MSCP compatible controller") \
PORT_CONFSETTING ( 0x0f, "'XX' Unknown 1/3") \
PORT_CONFSETTING ( 0x10, "'XX' Unknown 2/3") \
PORT_CONFSETTING ( 0x11, "'XX' Unknown 3/3") \
PORT_CONFSETTING ( 0x12, "'DY' BOOT prom for RX02 compatible controller") \
PORT_CONFSETTING ( 0x13, "'XM' DECNET 1/3 (DECnet DDCMP DMC11/DMR11)") \
PORT_CONFSETTING ( 0x14, "'XM' DECNET 2/3 (DECnet DDCMP DMC11/DMR11)") \
PORT_CONFSETTING ( 0x15, "'XM' DECNET 3/3 (DECnet DDCMP DMC11/DMR11)") \
PORT_CONFSETTING ( 0x16, "'XU' DECNET 1/3 (DECnet DDCMP DU11)") \
PORT_CONFSETTING ( 0x17, "'XU' DECNET 2/3 (DECnet DDCMP DU11)") \
PORT_CONFSETTING ( 0x18, "'XU' DECNET 3/3 (DECnet DDCMP DU11)") \
PORT_CONFSETTING ( 0x19, "'XW' DECNET 1/3 (DECnet DDCMP DUP11)") \
PORT_CONFSETTING ( 0x1a, "'XW' DECNET 2/3 (DECnet DDCMP DUP11)") \
PORT_CONFSETTING ( 0x1b, "'XW' DECNET 3/3 (DECnet DDCMP DUP11)") \
PORT_CONFSETTING ( 0x1c, "'XL' DECNET 1/3 (DECnet DDCMP DL11-E)") \
PORT_CONFSETTING ( 0x1d, "'XL' DECNET 2/3 (DECnet DDCMP DL11-E)") \
PORT_CONFSETTING ( 0x1e, "'XL' DECNET 3/3 (DECnet DDCMP DL11-E)") \
PORT_CONFSETTING ( 0x1f, "'XE' DEUNA DECnet Ethernet") \
PORT_CONFSETTING ( 0x20, "'MU' TMSCP tapes, including TK50, TU81")
/* Input ports */
static INPUT_PORTS_START( pdp11 )
	PORT_START("S1")
	PORT_DIPNAME( 0x01, 0x01, "S1-1" )
	PORT_DIPSETTING(    0x00, "Direct boot" )
	PORT_DIPSETTING(    0x01, "Console mode" )
	PORT_DIPNAME( 0x02, 0x02, "S1-2 Boot")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_START("S1_2")
	PORT_DIPNAME( 0x80, 0x00, "S1-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "S1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "S1-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "S1-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "S1-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "S1-8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "S1-9" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x00, "S1-10" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_START( "CONSPROM" )
	PORT_CONFNAME ( 0x01, 0, "Console PROM" )
	PORT_CONFSETTING ( 0x00, "11/04/05/34/35/40/45/50/55" )
	PORT_CONFSETTING ( 0x01, "11/60-70" )
	PORT_START( "DEVPROM1" )
	PORT_CONFNAME ( 0x2f, 0x02, "Device 1 PROM" )
	M9312_PORT_CONFSETTING
	PORT_START( "DEVPROM2" )
	PORT_CONFNAME ( 0x2f, 0x00, "Device 2 PROM" )
	M9312_PORT_CONFSETTING
	PORT_START( "DEVPROM3" )
	PORT_CONFNAME ( 0x2f, 0x0d, "Device 3 PROM" )
	M9312_PORT_CONFSETTING
	PORT_START( "DEVPROM4" )
	PORT_CONFNAME ( 0x2f, 0x04, "Device 4 PROM" )
	M9312_PORT_CONFSETTING
INPUT_PORTS_END

void pdp11_state::machine_start()
{
}

void pdp11_state::machine_reset()
{
	// Load M9301-YA
	uint8_t* user1 = memregion("user1")->base();
	uint8_t* maincpu = memregion("maincpu")->base();
	int i;

	for(i=0;i<0x100;i++) {
		uint8_t nib1 = user1[i+0x000] ^ 0x00;
		uint8_t nib2 = user1[i+0x200] ^ 0x01;
		uint8_t nib3 = user1[i+0x400] ^ 0x0f;
		uint8_t nib4 = user1[i+0x600] ^ 0x0e;

		maincpu[0xea00 + i*2 + 1] = (nib1 << 4) + nib2;
		maincpu[0xea00 + i*2 + 0] = (nib3 << 4) + nib4;
	}
	for(i=0x100;i<0x200;i++) {
		uint8_t nib1 = user1[i+0x000] ^ 0x00;
		uint8_t nib2 = user1[i+0x200] ^ 0x01;
		uint8_t nib3 = user1[i+0x400] ^ 0x0f;
		uint8_t nib4 = user1[i+0x600] ^ 0x0e;

		maincpu[0xf600 + (i-0x100)*2 + 1] = (nib1 << 4) + nib2;
		maincpu[0xf600 + (i-0x100)*2 + 0] = (nib3 << 4) + nib4;
	}
}

void pdp11_state::load9312prom(uint8_t *desc, uint8_t *src, int size)
{
	//   3   2   1   8
	//   7   6   5   4
	// ~11 ~10   9   0
	//  15  14  13 ~12
	for(int i=0;i<size;i++) {
		uint8_t nib1 = src[i*4+0];
		uint8_t nib2 = src[i*4+1];
		uint8_t nib3 = src[i*4+2];
		uint8_t nib4 = src[i*4+3];

		desc[i*2 + 0] = (nib2 << 4) + ((nib1 & 0x0e) | (nib3 & 1));
		desc[i*2 + 1] = ((nib4 ^ 0x01)<<4) + ((nib1 & 0x01) | ((nib3 ^ 0x0c) & 0x0e));
	}
}

MACHINE_RESET_MEMBER(pdp11_state,pdp11ub2)
{
	// Load M9312
	uint8_t* user1 = memregion("consproms")->base() + ioport("CONSPROM")->read() * 0x0400;
	uint8_t* maincpu = memregion("maincpu")->base();

	//0165000
	load9312prom(maincpu + 0165000,user1,0x100);

	uint8_t s1 = ioport("S1")->read();

	if (s1 & 0x02) { // if boot enabled
		uint16_t addr = 0173000;
		if (s1 & 1) {
			addr = 0165000;
		}
		addr += ioport("S1_2")->read() * 2;
		m_maincpu->set_state_int(T11_PC, addr);
	}

	//0173000
	load9312prom(maincpu + 0173000,memregion("devproms")->base() + ioport("DEVPROM1")->read() * 0x0200,0x080);
	//0173200
	load9312prom(maincpu + 0173200,memregion("devproms")->base() + ioport("DEVPROM2")->read() * 0x0200,0x080);
	//0173400
	load9312prom(maincpu + 0173400,memregion("devproms")->base() + ioport("DEVPROM3")->read() * 0x0200,0x080);
	//0173600
	load9312prom(maincpu + 0173600,memregion("devproms")->base() + ioport("DEVPROM4")->read() * 0x0200,0x080);

}

MACHINE_RESET_MEMBER(pdp11_state,pdp11qb)
{
	m_maincpu->set_state_int(T11_PC, 0xea00);
}


void pdp11_state::pdp11(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, 4'000'000); // Need proper CPU here
	m_maincpu->set_initial_mode(6 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &pdp11_state::pdp11_mem);

	DL11(config, m_dl11, XTAL(4'608'000));
	m_dl11->set_rxc(9600);
	m_dl11->set_txc(9600);
	m_dl11->set_rxvec(060);
	m_dl11->set_txvec(064);
	m_dl11->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
// future
//  m_dl11->txrdy_wr_callback().set_inputline(m_maincpu, t11_device::CP0_LINE);
//  m_dl11->rxrdy_wr_callback().set_inputline(m_maincpu, t11_device::CP0_LINE);

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_dl11, FUNC(dl11_device::rx_w));

	RX01(config, "rx01", 0);
	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::CP0_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, "pc11");
	QBUS_SLOT(config, "qbus" ":2", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":3", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":4", qbus_cards, nullptr);
}

void pdp11_state::pdp11ub2(machine_config &config)
{
	pdp11(config);
	MCFG_MACHINE_RESET_OVERRIDE(pdp11_state,pdp11ub2)
}

void pdp11_state::pdp11qb(machine_config &config)
{
	pdp11(config); // FIXME: MXV11-B requires a F-11 or J-11 CPU
	MCFG_MACHINE_RESET_OVERRIDE(pdp11_state,pdp11qb)

	m_maincpu->set_initial_mode(0 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &pdp11_state::pdp11qb_mem);
}

void pdp11_state::sms1000(machine_config &config)
{
	pdp11qb(config);

	i80188_cpu_device &subcpu(I80188(config, "subcpu", 12.288_MHz_XTAL));
	subcpu.set_addrmap(AS_PROGRAM, &pdp11_state::sms1000_mem_188);
}

/* ROM definition */
ROM_START( pdp11ub )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "23-034a9.bin", 0x0000, 0x0200, CRC(01c5d78d) SHA1(b447c67bfd5134c142240a919f23a949e1953fb2))
	ROM_LOAD( "23-035a9.bin", 0x0200, 0x0200, CRC(c456df6c) SHA1(188c8ece6a2d67911016f55dd22b698a40aff515))
	ROM_LOAD( "23-036a9.bin", 0x0400, 0x0200, CRC(208ff511) SHA1(27198a1110319b70674a72fd03a798dfa2c2109a))
	ROM_LOAD( "23-037a9.bin", 0x0600, 0x0200, CRC(d248b282) SHA1(ea638de6bde8342654d3e62b7810aa041e111913))
ROM_END

ROM_START( pdp11ub2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x800, "consproms", ROMREGION_ERASEFF )
	ROM_LOAD( "23-248f1.bin", 0x0000, 0x0400, CRC(ecda1a6d) SHA1(b2bf770dda349fdd469235871564280baf06301d)) // M9312 11/04/05/34/35/40/45/50/55  Console/Diagnostic PROM
	ROM_LOAD( "23-616f1.bin", 0x0400, 0x0400, CRC(a3dfb5aa) SHA1(7f06c624ae3fbb49535258b8722b5a3c548da3ba)) // M9312 11/60-70 Diagnostic/Console ROM
	ROM_REGION( 0x4200, "devproms", ROMREGION_ERASEFF )
	ROM_LOAD( "23-751a9.bin", 0x0000, 0x0200, CRC(15bebc6a) SHA1(a621c5b1cebebbb110ee646a8c36ee4c606e269b)) // M9312 'DL' BOOT prom for RL11 controller
	ROM_LOAD( "23-752a9.bin", 0x0200, 0x0200, CRC(6cf1f859) SHA1(7c876eda2f0d74d6f5d876256c28dbd56c405ca7)) // M9312 'DM' BOOT prom for RK06/07 controller
	ROM_LOAD( "23-753a9.bin", 0x0400, 0x0200, CRC(f4c4b40c) SHA1(a0bdb922c722d439f35ba8149a8f657ffcc8fb54)) // M9312 'DX' BOOT prom for RX01 compatible controller
	ROM_LOAD( "23-755a9.bin", 0x0600, 0x0200, CRC(ed06b35c) SHA1(d972c6a743d73ce9244d2bcfdd40eea2bb22e717)) // M9312 'DP/DB' BOOT prom for RP02/03,RP04/5/6 RM02/3 controller
	ROM_LOAD( "23-756a9.bin", 0x0800, 0x0200, CRC(12271ab2) SHA1(f0ff42a8fd839dd75d6c1a25cc82d0933fd09dbc)) // M9312 'DK/DT' BOOT prom for RK03/05,TU55/56 controllers
	ROM_LOAD( "23-757a9.bin", 0x0a00, 0x0200, CRC(af251aab) SHA1(4d760ec3f6ff5f4e2cafcb44b275183872b69cb6)) // M9312 'MM' BOOT prom for TU16/E16 TM02/3 controllers
	ROM_LOAD( "23-758a9.bin", 0x0c00, 0x0200, CRC(b71e8878) SHA1(f45c47c702c94a70c36732c12173ce60d0be1a11)) // M9312 'MT' BOOT prom for TU10/TS03 controller
	ROM_LOAD( "23-759a9.bin", 0x0e00, 0x0200, CRC(29a93448) SHA1(0b549170c6a3f49c1587adb6cc691786111c0dd3)) // M9312 'DS' BOOT prom for RS03/RS04 controller
	ROM_LOAD( "23-760a9.bin", 0x1000, 0x0200, CRC(ea093648) SHA1(3875a0147c43db1a5a381bbe85937a5628e6220c)) // M9312 'PR/TT' BOOT prom for PC05,LO SPD RDR controllers
	ROM_LOAD( "23-761a9.bin", 0x1200, 0x0200, CRC(4310ebe8) SHA1(a3144f96819ea57acfac5de5e19961294e7d4ad9)) // M9312 'CT' BOOT prom for TA11/TU60 controller
	ROM_LOAD( "23-762a9.bin", 0x1400, 0x0200, NO_DUMP)                                                      // M9312 'RS' BOOT prom for RS11, RS64 controller
	ROM_LOAD( "23-763a9.bin", 0x1600, 0x0200, NO_DUMP)                                                      // M9312 'CR' BOOT prom for CR11 card reader
	ROM_LOAD( "23-764a9.bin", 0x1800, 0x0200, CRC(7c8b7ed4) SHA1(ba0c9f03027eb3dafcc0936e877637d3c9947f94)) // M9312 'MS' BOOT prom for TS11/TS04/TU80 compatible controller
	ROM_LOAD( "23-765a9.bin", 0x1a00, 0x0200, CRC(702dfeb2) SHA1(0d37bdd3846de4b104b8968a0e83ed81abd7f9ae)) // M9312 'DD' BOOT prom for TU58 DECtapeII serial tape controller
	ROM_LOAD( "23-767a9.bin", 0x1c00, 0x0200, CRC(4b94e3fa) SHA1(3cf92c2f64f95e8cc3abb8af2526cc65ce53ca8a)) // M9312 'DU' BOOT prom for MSCP compatible controller (UDA50/RA50/RC25/RAxx)
	ROM_LOAD( "23-786a9.bin", 0x1e00, 0x0200, CRC(a5326664) SHA1(238f97fc5b2b540948ea1e27a4cd1dcf18255b21)) // M9312 'XX' Unknown 1/3
	ROM_LOAD( "23-787a9.bin", 0x2000, 0x0200, CRC(025debf9) SHA1(8ea2faf2e2d78be0ad2f77e61bae0dfb9c3b4b01)) // M9312 'XX' Unknown 2/3
	ROM_LOAD( "23-788a9.bin", 0x2200, 0x0200, CRC(3c7ed364) SHA1(519ffac2e4878490128e754a0473502c767a94e2)) // M9312 'XX' Unknown 3/3
	ROM_LOAD( "23-811a9.bin", 0x2400, 0x0200, CRC(9aa8499a) SHA1(11b040e0908d7492dcc450cbb72d76633dd687ca)) // M9312 'DY' BOOT prom for RX02 compatible controller
	ROM_LOAD( "23-862a9.bin", 0x2600, 0x0200, CRC(38dbd994) SHA1(c5db671e6b70f3b4d345a02b46e0ea7566160d04)) // M9312 'XM' DECNET 1/3 (DECnet DDCMP DMC11/DMR11)
	ROM_LOAD( "23-863a9.bin", 0x2800, 0x0200, CRC(bbef2f41) SHA1(f472b7a8bd4c0a49dc3ec38f886755910f73fe66)) // M9312 'XM' DECNET 2/3 (DECnet DDCMP DMC11/DMR11)
	ROM_LOAD( "23-864a9.bin", 0x2a00, 0x0200, CRC(85cc17dc) SHA1(371dbd3c672fe4b1819762c3082c4217a7597547)) // M9312 'XM' DECNET 3/3 (DECnet DDCMP DMC11/DMR11)
	ROM_LOAD( "23-865a9.bin", 0x2c00, 0x0200, NO_DUMP)                                                      // M9312 'XU' DECNET 1/3 (DECnet DDCMP DU11)
	ROM_LOAD( "23-866a9.bin", 0x2e00, 0x0200, NO_DUMP)                                                      // M9312 'XU' DECNET 2/3 (DECnet DDCMP DU11)
	ROM_LOAD( "23-867a9.bin", 0x3000, 0x0200, NO_DUMP)                                                      // M9312 'XU' DECNET 3/3 (DECnet DDCMP DU11)
	ROM_LOAD( "23-868a9.bin", 0x3200, 0x0200, NO_DUMP)                                                      // M9312 'XW' DECNET 1/3 (DECnet DDCMP DUP11)
	ROM_LOAD( "23-869a9.bin", 0x3400, 0x0200, NO_DUMP)                                                      // M9312 'XW' DECNET 2/3 (DECnet DDCMP DUP11)
	ROM_LOAD( "23-870a9.bin", 0x3600, 0x0200, NO_DUMP)                                                      // M9312 'XW' DECNET 3/3 (DECnet DDCMP DUP11)
	ROM_LOAD( "23-926a9.bin", 0x3800, 0x0200, NO_DUMP)                                                      // M9312 'XL' DECNET 1/3 (DECnet DDCMP DL11-E)
	ROM_LOAD( "23-927a9.bin", 0x3a00, 0x0200, NO_DUMP)                                                      // M9312 'XL' DECNET 2/3 (DECnet DDCMP DL11-E)
	ROM_LOAD( "23-928a9.bin", 0x3c00, 0x0200, NO_DUMP)                                                      // M9312 'XL' DECNET 3/3 (DECnet DDCMP DL11-E)
	ROM_LOAD( "23-e22a9.bin", 0x3e00, 0x0200, NO_DUMP)                                                      // M9312 'XE' DEUNA DECnet Ethernet
	ROM_LOAD( "23-e39a9.bin", 0x4000, 0x0200, CRC(4b94e3fa) SHA1(3cf92c2f64f95e8cc3abb8af2526cc65ce53ca8a)) // M9312 'MU' TMSCP tapes, including TK50, TU81

ROM_END

ROM_START( pdp11qb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "m7195fa.1", 0xc000, 0x2000, CRC(0fa58752) SHA1(4bcd006790a60f2998ee8377ac5e2c18ef330930))
	ROM_LOAD16_BYTE( "m7195fa.2", 0xc001, 0x2000, CRC(15b6f60c) SHA1(80dd4f8ca3c27babb5e75111b04241596a07c53a))
ROM_END

ROM_START( sms1000 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "21251001u",    0x00000, 0x010000, CRC(eec3ccbb) SHA1(69eedb2c3bffe0a2988b1c066df1fea195618087) )

	ROM_REGION( 0x8000, "subcpu", 0 )
	ROM_LOAD( "21251000u",    0x00000, 0x008000, CRC(68db0afc) SHA1(577124bc64f6ddc9771e11b483120a175bfcf8c5) )

	ROM_REGION( 0x800, "prom", 0 )
	ROM_LOAD( "21251002u",    0x00000, 0x000800, CRC(66ca0eaf) SHA1(8141f64f81d9954169bcff6c79fd9f85e91f98e0) )

	ROM_REGION( 0x20000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "2123001",      0x00000, 0x000800, CRC(7eb10e9b) SHA1(521ce8b8a79075c30ad92d810141c725d26fc50e) )
	ROM_LOAD( "2115001.jed",  0x01000, 0x000b19, CRC(02170f78) SHA1(afe50d165b39bff1cadae4290344341376729fda) )
	// no idea how large these undumped proms are
	ROM_LOAD( "2096001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2097002",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "20982000f",    0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "20982001f",    0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2099002b",     0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2108001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2109001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2110001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2111001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2116001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2117001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2118001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2119001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2120001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2121001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2122001",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2124008",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2124009",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2124010",      0x1f000, 0x000100, NO_DUMP )
	ROM_LOAD( "2127001b",     0x1f000, 0x000100, NO_DUMP ) // has 3 of these
ROM_END

} // Anonymous namespace


/* Driver */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY                          FULLNAME                            FLAGS */
COMP( 197?, pdp11ub,  0,       0,      pdp11,    pdp11, pdp11_state, empty_init, "Digital Equipment Corporation", "PDP-11 [Unibus](M9301-YA)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 197?, pdp11ub2, pdp11ub, 0,      pdp11ub2, pdp11, pdp11_state, empty_init, "Digital Equipment Corporation", "PDP-11 [Unibus](M9312)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 198?, pdp11qb,  pdp11ub, 0,      pdp11qb,  pdp11, pdp11_state, empty_init, "Digital Equipment Corporation", "PDP-11 [Q-BUS] (M7195 - MXV11-B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1987, sms1000,  0,       0,      sms1000,  pdp11, pdp11_state, empty_init, "Scientific Micro Systems",      "SMS-1000",                         MACHINE_IS_SKELETON )
