// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    decstation.cpp: MIPS-based DECstation family

    WANTED: boot ROM dumps for KN02CA/KN04CA (MAXine) systems.

    NOTE: after all the spew of failing tests, press 'q' at the MORE prompt and
    wait a few seconds for the PROM monitor to appear.
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
            Cheaper PMAX, 12.5 MHz R2000, othersame as PMAX

        Personal DECstation 5000/xx (MAXine/KN02CA for R3000, KN04CA? for R4000)
            20, 25, or 33 MHz R3000 or 100 MHz R4000
            40 MiB max RAM
            Serial: DEC "DZ" quad-UART for keyboard/mouse, SCC8530 for modem/printer
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller
            Audio: AMD AM79C30
            Color 1024x768 8bpp video on-board
            2 TURBOchannel slots

        DECstation 5000/1xx: (3MIN/KN02BA, KN04BA? for R4000):
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

        DECstation 5000/240 (3MAX+/KN03AA), 5000/260 (3MAX+/KN05)
            40 MHz R3400, or 120 MHz R4400.
            480 MiB max RAM
            Serial: 2x SCC8530
            SCSI: NCR53C94
            Ethernet: AMD7990 "LANCE" controller

****************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "cpu/mips/mips3.h"
#include "machine/decioga.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "machine/dec_lk201.h"
#include "machine/am79c90.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "video/bt459.h"

class decstation_state : public driver_device
{
public:
	decstation_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lk201(*this, "lk201"),
		m_ioga(*this, "ioga"),
		m_rtc(*this, "rtc"),
		m_scc0(*this, "scc0"),
		m_scc1(*this, "scc1"),
		m_asc(*this, "scsibus:7:asc"),
		m_vrom(*this, "gfx"),
		m_bt459(*this, "bt459"),
		m_lance(*this, "am79c90")
		{ }

	void kn02ba(machine_config &config);

	void init_decstation();

protected:
	DECLARE_READ_LINE_MEMBER(brcond0_r) { return ASSERT_LINE; }
	DECLARE_WRITE_LINE_MEMBER(ioga_irq_w);

	DECLARE_READ32_MEMBER(cfb_r);
	DECLARE_WRITE32_MEMBER(cfb_w);

	void ncr5394(device_t *device);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<r3000a_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<lk201_device> m_lk201;
	required_device<dec_ioga_device> m_ioga;
	required_device<mc146818_device> m_rtc;
	required_device<z80scc_device> m_scc0, m_scc1;
	required_device<ncr53c94_device> m_asc;
	required_memory_region m_vrom;
	required_device<bt459_device> m_bt459;
	required_device<am79c90_device> m_lance;

	void threemin_map(address_map &map);

	uint8_t *m_vrom_ptr;
	uint32_t m_vram[0x200000/4];
	uint32_t m_sfb[0x80];
	int m_copy_src;
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

void decstation_state::video_start()
{
}

uint32_t decstation_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bt459->screen_update(screen, bitmap, cliprect, (uint8_t *)&m_vram[0]);
	return 0;
}

/*
    0x100000 copy register 0
    0x100004 copy register 1
    0x100008 copy register 2
    0x10000C copy register 3
    0x100010 copy register 4
    0x100014 copy register 5
    0x100018 copy register 6
    0x10001C copy register 7
    0x100020 foreground register
    0x100024 background register
    0x100028 plane mask
    0x10002C pixel mask
    0x100030 cxt mode
    0x100034 boolean operation
    0x100038 pixel shift
    0x10003C line address
    0x100040 bresh 1
    0x100044 bresh 2
    0x100048 bresh 3
    0x10004C bresh continue
    0x100050 deep register
    0x100054 start register
    0x100058 Clear Interrupt
    0x10005C reserved 2
    0x100060 refresh count
    0x100064 video horiz
    0x100068 video vertical
    0x10006C refresh base
    0x100070 video valid
    0x100074 Interrupt Enable
*/

#define MODE_SIMPLE     0
#define MODE_OPAQUESTIPPLE  1
#define MODE_OPAQUELINE     2
#define MODE_TRANSPARENTSTIPPLE 5
#define MODE_TRANSPARENTLINE    6
#define MODE_COPY       7

READ32_MEMBER(decstation_state::cfb_r)
{
	uint32_t addr = offset << 2;

//  logerror("cfb_r: reading at %x\n", addr);

	if (addr < 0x800000)
	{
		return m_vrom_ptr[addr>>2] & 0xff;
	}

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
		return m_sfb[offset-(0x100000/4)];
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
		return m_vram[offset-(0x200000/4)];
	}

	return 0xffffffff;
}

WRITE32_MEMBER(decstation_state::cfb_w)
{
	uint32_t addr = offset << 2;

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
		//printf("SFB: %08x (mask %08x) @ %x\n", data, mem_mask, offset<<2);
		COMBINE_DATA(&m_sfb[offset-(0x100000/4)]);

		if ((addr == 0x100030) && (data = 7))
		{
			m_copy_src = 1;
		}
		return;
	}

	if ((addr >= 0x1c0000) && (addr < 0x200000))
	{
		//printf("Bt459: %08x (mask %08x) @ %x\n", data, mem_mask, offset<<2);
		return;
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
		//printf("FB: %08x (mask %08x) @ %x\n", data, mem_mask, offset<<2);

		switch (m_sfb[0x30/4])
		{
			case MODE_SIMPLE:   // simple
				COMBINE_DATA(&m_vram[offset-(0x200000/4)]);
				break;

			case MODE_TRANSPARENTSTIPPLE:
				{
					uint8_t *pVRAM = (uint8_t *)&m_vram[offset-(0x200000/4)];
					uint8_t fgs[4];

					fgs[0] = m_sfb[0x20/4] >> 24;
					fgs[1] = (m_sfb[0x20/4] >> 16) & 0xff;
					fgs[2] = (m_sfb[0x20/4] >> 8) & 0xff;
					fgs[3] = m_sfb[0x20/4] & 0xff;
					for (int x = 0; x < 32; x++)
					{
						if (data & (1<<(31-x)))
						{
							pVRAM[x] = fgs[x & 3];
						}
					}
				}
				break;

			case MODE_COPY:
				{
					uint8_t *pVRAM = (uint8_t *)&m_vram[offset-(0x200000/4)];
					uint8_t *pBuffer = (uint8_t *)&m_sfb[0];    // first 8 32-bit regs are the copy buffer

					if (m_copy_src)
					{
						m_copy_src = 0;

						for (int x = 0; x < 32; x++)
						{
							if (data & (1<<(31-x)))
							{
								pBuffer[x] = pVRAM[x];
							}
						}
					}
					else
					{
						m_copy_src = 1;

						for (int x = 0; x < 32; x++)
						{
							if (data & (1<<(31-x)))
							{
								pVRAM[x] = pBuffer[x];
							}
						}
					}
				}
				break;

			default:
				logerror("SFB: Unsupported VRAM write %08x (mask %08x) at %08x in mode %x\n", data, mem_mask, offset<<2, m_sfb[0x30/4]);
				break;
		}
		return;
	}
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

WRITE_LINE_MEMBER(decstation_state::ioga_irq_w)
{
	// not sure this is correct
	m_maincpu->set_input_line(INPUT_LINE_IRQ3, state);
}

void decstation_state::machine_start()
{
	m_vrom_ptr = m_vrom->base();
	save_item(NAME(m_vram));
	save_item(NAME(m_sfb));
	save_item(NAME(m_copy_src));
}

void decstation_state::machine_reset()
{
	m_copy_src = 1;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void decstation_state::threemin_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram();  // full 128 MB
	map(0x10000000, 0x13ffffff).rw(FUNC(decstation_state::cfb_r), FUNC(decstation_state::cfb_w));
	map(0x101c0000, 0x101c000f).m("bt459", FUNC(bt459_device::map)).umask32(0x000000ff);
	map(0x1c000000, 0x1c07ffff).m(m_ioga, FUNC(dec_ioga_device::map));
	map(0x1c0c0000, 0x1c0c0007).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0x0000ffff);
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

MACHINE_CONFIG_START(decstation_state::kn02ba)
	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 65536, 131072);
	m_maincpu->set_endianness(ENDIANNESS_LITTLE);
	m_maincpu->set_fpurev(0x340); // should be R3010A v4.0
	m_maincpu->in_brcond<0>().set(FUNC(decstation_state::brcond0_r));
	m_maincpu->set_addrmap(AS_PROGRAM, &decstation_state::threemin_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(130000000, 1704, 32, (1280+32), 1064, 3, (1024+3));
	m_screen->set_screen_update(FUNC(decstation_state::screen_update));

	BT459(config, m_bt459, 83'020'800);

	AM79C90(config, m_lance, XTAL(12'500'000));
	m_lance->irq_out().set("ioga", FUNC(dec_ioga_device::lance_irq_w));

	DECSTATION_IOGA(config, m_ioga, XTAL(12'500'000));
	m_ioga->irq_out().set(FUNC(decstation_state::ioga_irq_w));

	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set("ioga", FUNC(dec_ioga_device::rtc_irq_w));
	m_rtc->set_binary(true);

	SCC85C30(config, m_scc0, XTAL(14'745'600)/2);
	m_scc0->out_int_callback().set("ioga", FUNC(dec_ioga_device::scc0_irq_w));
	m_scc0->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_scc0->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));

	SCC85C30(config, m_scc1, XTAL(14'745'600)/2);
	m_scc1->out_int_callback().set("ioga", FUNC(dec_ioga_device::scc1_irq_w));
	m_scc1->out_txdb_callback().set("lk201", FUNC(lk201_device::rx_w));

	MCFG_DEVICE_ADD("lk201", LK201, 0)
	MCFG_LK201_TX_HANDLER(WRITELINE("scc1", z80scc_device, rxb_w))

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc0, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc0, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc0, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc0, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc0, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc0, FUNC(z80scc_device::ctsb_w));

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
	// 5.7j                                                                                                                                                                                                                                 sx
	ROM_LOAD( "ds5000-133_005eb.bin", 0x000000, 0x040000, CRC(76a91d29) SHA1(140fcdb4fd2327daf764a35006d05fabfbee8da6) )

	ROM_REGION32_LE( 0x20000, "gfx", 0 )
	ROM_LOAD( "pmagb-ba-rom.img", 0x000000, 0x020000, CRC(91f40ab0) SHA1(a39ce6ed52697a513f0fb2300a1a6cf9e2eabe33) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                 FULLNAME                FLAGS
COMP( 1992, ds5k133, 0,      0,      kn02ba, decstation, decstation_state, init_decstation, "Digital Equipment Corporation", "DECstation 5000/133", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
