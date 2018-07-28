// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    decstation.cpp: MIPS-based DECstation family

    WANTED: all boot ROM dumps except 5000/133, all TURBOchannel card ROM dumps

    NOTE: after all the spew of failing tests (it really wants a VT102 terminal),
    press 'q' at the MORE prompt and wait a few seconds for the PROM monitor to appear.
    Type 'ls' for a list of commands (this is a very UNIX-flavored PROM monitor).

    Machine types:
        DECstation 3100 (PMAX/KN01):
            16.67 MHz R2000 with FPU and MMU
            24 MiB max RAM
            Serial: DEC "DZ" quad-UART (DC7085 gate array)
            SCSI: DEC "SII" SCSI interface (DC7061 gate array)
            Ethernet: AMD7990 "LANCE" controller
            Monochrome or color video on-board
        PMIN/KN01:
            Cheaper PMAX, 12.5 MHz R2000, same as PMAX

        Personal DECstation 5000/xx (MAXine/KN02BA):
            20, 25, or 33 MHz R3000 or 100 MHz R4000
            40 MiB max RAM
            Serial: DEC "DZ" quad-UART for keyboard/mouse, SCC8530 for modem/printer
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller
            Audio: AMD AM79C30
            Color 1024x768 8bpp video on-board
            2 TURBOchannel slots

        DECstation 5000/1xx: (3MIN/KN02DA):
            20, 25, or 33 MHz R3000 or 100 MHz R4000
            128 MiB max RAM
            Serial: 2x SCC8530
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller
            No on-board video
            3 TURBOchannel slots

        DECstation 5000/200: (3MAX/KN02):
            25 MHz R3000
            480 MiB max RAM
            Serial: DEC "DZ" quad-UART
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controllor

        DECstation 5000/240, 5000/261 (3MAX+/KN03)
            40 MHz R3400, or 120 MHz R4400.
            480 MiB max RAM
            Serial: 2x SCC8530
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller

****************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "machine/decioga.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "bus/rs232/rs232.h"

class decstation_state : public driver_device
{
public:
	decstation_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ioga(*this, "ioga"),
		m_rtc(*this, "rtc"),
		m_scc0(*this, "scc0"),
		m_scc1(*this, "scc1"),
		m_asc(*this, "scsibus:7:asc")
		{ }

	void kn02da(machine_config &config);

	void init_decstation();

protected:
	DECLARE_READ_LINE_MEMBER(brcond0_r) { return ASSERT_LINE; }

	DECLARE_READ32_MEMBER(cfb_r);
	DECLARE_WRITE32_MEMBER(cfb_w);

	void ncr5394(device_t *device);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<dec_ioga_device> m_ioga;
	required_device<mc146818_device> m_rtc;
	required_device<z80scc_device> m_scc0, m_scc1;
	required_device<ncr53c94_device> m_asc;

	void decstation_map(address_map &map);
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

READ32_MEMBER(decstation_state::cfb_r)
{
	static const char fwver[8]    = "V5.3a  ";
	static const char fwvendor[8] = "DEC    ";
	static const char fwmodule[8] = "PMAG-BA";
	static const char fwtype[8]   = "TCF0   ";
	static const char fwinfo[8]   = "CX - D8";
	uint32_t addr = offset << 2;

	logerror("cfb_r: reading at %x\n", addr);

	// attempt to fake the ROM ID bytes of a PMAG-BA
	// color framebuffer card.  doesn't work for unknown reasons.
	if ((addr >= 0x3c0400) && (addr < 0x3c0420))
	{
		return fwver[(addr>>2)&0x7];
	}
	if ((addr >= 0x3c0420) && (addr < 0x3c0440))
	{
		return fwvendor[(addr>>2)&0x7];
	}
	if ((addr >= 0x3c0440) && (addr < 0x3c0460))
	{
		return fwmodule[(addr>>2)&0x7];
	}
	if ((addr >= 0x3c0460) && (addr < 0x3c0480))
	{
		return fwtype[(addr>>2)&0x7];
	}
	if ((addr >= 0x3c0480) && (addr < 0x3c04a0))
	{
		return fwinfo[(addr>>2)&0x7];
	}

	switch (addr)
	{
		case 0x3c03e0: return 1;    // ROM width
		case 0x3c03e4: return 4;    // ROM stride
		case 0x3c03e8: return 1;    // ROM size in 8 KiB units
		case 0x3c03ec: return 1;    // card address space in 4 MiB units
		case 0x3c03f0: return 0x55555555; // TURBOchannel ID bytes
		case 0x3c03f4: return 0x00000000;
		case 0x3c03f8: return 0xaaaaaaaa;
		case 0x3c03fc: return 0xffffffff;
		case 0x3c0470: return 0;    // does card support parity?
	}

	return 0xffffffff;
}

WRITE32_MEMBER(decstation_state::cfb_w)
{
	logerror("cfb: %08x (mask %08x) @ %x\n", data, mem_mask, offset);
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

void decstation_state::machine_start()
{
}

void decstation_state::machine_reset()
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void decstation_state::decstation_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram();  // full 128 MB
	map(0x10000000, 0x103cffff).rw(FUNC(decstation_state::cfb_r), FUNC(decstation_state::cfb_w));
	map(0x1c000000, 0x1c07ffff).m(m_ioga, FUNC(dec_ioga_device::map));
	map(0x1c100000, 0x1c100003).rw(m_scc0, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w)).umask32(0x0000ff00);
	map(0x1c100004, 0x1c100007).rw(m_scc0, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w)).umask32(0x0000ff00);
	map(0x1c100008, 0x1c10000b).rw(m_scc0, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w)).umask32(0x0000ff00);
	map(0x1c10000c, 0x1c10000f).rw(m_scc0, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w)).umask32(0x0000ff00);
	map(0x1c180000, 0x1c180003).rw(m_scc1, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w)).umask32(0x0000ff00);
	map(0x1c180004, 0x1c180007).rw(m_scc1, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w)).umask32(0x0000ff00);
	map(0x1c180008, 0x1c18000b).rw(m_scc1, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w)).umask32(0x0000ff00);
	map(0x1c18000c, 0x1c18000f).rw(m_scc1, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w)).umask32(0x0000ff00);
	map(0x1c200000, 0x1c2000ff).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).umask32(0x000000ff);
	map(0x1c300000, 0x1c30003f).m(m_asc, FUNC(ncr53c94_device::map)).umask32(0x000000ff);
	map(0x1fc00000, 0x1fc3ffff).rom().region("user1", 0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void decstation_state::ncr5394(device_t *device)
{
	devcb_base *devcb;
	(void)devcb;
	MCFG_DEVICE_CLOCK(10000000)
}

static void dec_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("asc", NCR53C94);
}

MACHINE_CONFIG_START(decstation_state::kn02da)
	MCFG_DEVICE_ADD( "maincpu", R3041, 33000000 ) // FIXME: Should be R3000A
	MCFG_R3000_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_R3000_BRCOND0_INPUT(READLINE(*this, decstation_state, brcond0_r))
	MCFG_DEVICE_PROGRAM_MAP( decstation_map )

	MCFG_DEVICE_ADD("ioga", DECSTATION_IOGA, XTAL(12'500'000))

	MCFG_DEVICE_ADD("rtc", MC146818, XTAL(32'768))
	MCFG_MC146818_IRQ_HANDLER(WRITELINE("ioga", dec_ioga_device, rtc_irq_w))
	MCFG_MC146818_BINARY(true)

	MCFG_DEVICE_ADD("scc0", SCC85C30, XTAL(14'745'600)/2)
	//MCFG_Z80SCC_OUT_INT_CB(WRITELINE("ioga", dec_ioga_device, scc0_irq_w))

	MCFG_DEVICE_ADD("scc1", SCC85C30, XTAL(14'745'600)/2)
	//MCFG_Z80SCC_OUT_INT_CB(WRITELINE("ioga", dec_ioga_device, scc1_irq_w))
	MCFG_Z80SCC_OUT_TXDA_CB(WRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(WRITELINE("rs232b", rs232_port_device, write_txd))

	MCFG_DEVICE_ADD("rs232a", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc1", z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc1", z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc1", z80scc_device, ctsa_w))

	MCFG_DEVICE_ADD("rs232b", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("scc1", z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE("scc1", z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("scc1", z80scc_device, ctsb_w))

	MCFG_NSCSI_BUS_ADD("scsibus")
	MCFG_NSCSI_ADD("scsibus:0", dec_scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD("scsibus:1", dec_scsi_devices, "cdrom", false)
	MCFG_NSCSI_ADD("scsibus:2", dec_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:3", dec_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:4", dec_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:5", dec_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:6", dec_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD("scsibus:7", dec_scsi_devices, "asc", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("asc", [this] (device_t *device) { ncr5394(device); })
MACHINE_CONFIG_END

static INPUT_PORTS_START( decstation )
	PORT_START("UNUSED") // unused IN0
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void decstation_state::init_decstation()
{
}

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( ds5k133 )
	ROM_REGION32_LE( 0x40000, "user1", 0 )
	ROM_LOAD( "ds5000-133_005eb.bin", 0x000000, 0x040000, CRC(76a91d29) SHA1(140fcdb4fd2327daf764a35006d05fabfbee8da6) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                 FULLNAME                FLAGS
COMP( 1992, ds5k133, 0,      0,      kn02da, decstation, decstation_state, init_decstation, "Digital Equipment Corporation", "DECstation 5000/133", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
