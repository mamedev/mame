// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, David Haywood

/*

TC0030CMD "C-chip" pinout: (from operation wolf schematics)
http://www.arcade-museum.com/manuals-videogames/O/Operation%20Wolf%20Schematics.pdf
http://www.jammarcade.net/partial-rainbow-islands-schematic/

Chip contains 4 dies:
(starting from pin 1 edge going toward pin 32 edge)
1 - uPD78C11 MCU w/ 4kx8 mask rom (assumed to be the same between games)
2 - uPD27C64 8kx8 EPROM (assumed to be different between games)
3 - uPD4464 8kx8 SRAM
4 - ASIC (NEC ULA)


                         +---------\_/---------+
                VCC -- - |  1               64 | - -- VCC
                PA0 <> m |  2               63 | m <- AN7 (tied to GND externally)
                PA1 <> m |  3               62 | m <- AN6
                PA2 <> m |  4               61 | m <- AN5
                PA3 <> m |  5               60 | m <- AN4
                PA4 <> m |  6               59 | m <- AN3
                PA5 <> m |  7               58 | m <- AN2 (tied to VCC externally)
                PA6 <> m |  8     T         57 | m <- AN1
                PA7 <> m |  9             | 56 | m <- AN0
                PB0 <> m | 10     C    ---| 55 | m <- MODE1 (MODE0 and AVSS are internally tied to GND) **
                PB1 <> m | 11             | 54 | m <- /INT1
                PB2 <> m | 12     0    --\  53 | m <- /NMI, used on rainbow islands
                PB3 <> m | 13           | > 52 | m <> PC7
                PB4 <> m | 14     0    --/  51 | m <> PC6
                PB5 <> m | 15               50 | m <> PC5
                PB6 <> m | 16     3    ---- 49 | m <> PC4
                PB7 <> m | 17               48 | m <> PC3
                PC0 <> m | 18     0       | 47 | m <> PC2
  (tied to VCC) VPP -> e | 19          ---| 46 | m <> PC1
                CLK -> a | 20     C       | 45 | a <- A10
      *ASIC_MODESEL ?> a | 21          /--\ 44 | a <- A9
             /RESET -> a | 22     M    |  | 43 | a <- A8
                 D0 <> a | 23          \--/ 42 | a <- A7
                 D1 <> a | 24     D         41 | a <- A6
                 D2 <> a | 25               40 | a <- A5
                 D3 <> a | 26               39 | a <- A4
                 D4 <> a | 27               38 | a <- A3
                 D5 <> a | 28               37 | a <- A2
                 D6 <> a | 29               36 | a <- A1
                 D7 <> a | 30               35 | a <- A0
                /CS -> a | 31               34 | a -> DTACK out to 68k (/CDTA)
                GND -- - | 32               33 | a <- R/W
                         +---------------------+
a = to asic
e = to eprom
m = to mcu
- = power pins, common

CLK is supplied with a 12MHZ oscillator on operation wolf

*Pin 21 goes to the ASIC and may be a mode pin. (try holding it to gnd?) Normally tied to 5v thru a 1k resistor.

** MODE1 is normally tied high externally via a 330ohm resistor, but can be in either state.
   MODE0 on the c-chip is internally tied low.

(move below MODE notes to upd7811.cpp?)

The four Mode0/Mode1 combinations are:
    LOW/LOW     78c11 mem map is 4k external rom (i.e. the eprom inside the c-chip) from 0x0000-0x0FFF;
                the low 4 bits of port F are used, to provide the high 4 address bits.
                speculation: likely the eprom can be banked so the low or high half is visible here,
                or possibly one fixed window and 3 variable windows, managed by the asic?
    LOW/HIGH    78c11 mem map boots to internal rom (mask rom inside the 78c11 inside the c-chip) from
                0x0000-0x0fff but the memory map is under full mcu control and can select any of the
                four modes (internal only, 4k external, 16k external, 64k external)
The following two modes are unusable on the c-chip:
    HIGH/LOW    78c11 mem map is 16k external rom from 0x0000-0x3FFF;
                the low 6 bits of port F are used, to provide the high 6 address bits.
    HIGH/HIGH   78c11 mem map is 64k external rom from 0x0000-0xFFFF;
                all 8 bits of port F are used to provide the high 8 address bits.
VPP is only used for programming the 27c64, do not tie it to 18v or you will probably overwrite the 27c64 with garbage.

(see http://www.cpcwiki.eu/index.php/UPD7810/uPD7811 )


C-chip EXTERNAL memory map (it acts as a device mapped to ram; dtack is asserted on /cs for a 68k):
0x000-0x3FF = RW ram window, a 1k window into the 8k byte sram inside the c-chip; the 'bank' visible is selected by 0x600 below
0x400-0x403 = "ASIC RAM", 4 bytes of ram on the asic
0x400 = unknown, no idea if /DTACK is asserted for R or W here
0x401 = RW 'test command/status register', writing a 0x02 here starts test mode, will return 0x01 set if ok/ready for
    command and 0x04 set if error; this register is very likely handled by the internal rom in the upd78c11 itself rather
    than the eprom, and probably tests the sram and the 78c11 internal ram, among other things.
    * Current guess: 0x401 is actually attached to the high 2 bits of the PF register; bit 0 is pf6 out, bit 1 is pf6 in
      (attached to pf6 thru a resistor?), bit 2 is pf7 out, bit 3 is pf7 in (attached to pf7 through a resistor).
      The 78c11 (I'm guessing) reads pf6 and pf7 once per int; if pf6-in is set it reruns the startup selftest,
      clears pf6-out, then re-sets it. if there is an error, it also sets pf7.
    * Alternate guess: pf4 selects between rom and ram but pf5,6,7 are all mapped to 0x401. a memory mapped register in
      upd78c11 space selects low vs high half of rom/ram access
0x402-0x5ff = unknown (may be mirror of 0x400 and 0x401?) no idea if /DTACK is asserted for R or W here
0x600 = ?W ram window bank select, selects one of 8 1k banks to be accessible at 0x000-0x3ff , only low 3 bits are valid on this register. not sure if readable.
0x601-0x7ff = unknown, no idea if /DTACK is asserted for R or W here

This chip *ALWAYS* has a bypass capacitor (ceramic, 104, 0.10 uF) soldered on top of the chip between pins 1 and 32 OR between 64 and 32.

*/

#include "emu.h"
#include "taitocchip.h"

#include "cpu/upd7810/upd7810.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(TAITO_CCHIP, taito_cchip_device, "cchip", "Taito TC0030CMD (C-Chip)")

taito_cchip_device::taito_cchip_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TAITO_CCHIP, tag, owner, clock),
	m_upd7811(*this, "upd7811"),
	m_upd4464_bank(*this, "upd4464_bank"),
	m_upd4464_bank68(*this, "upd4464_bank68"),
	m_sharedram(*this, "upd4464", 0x2000, ENDIANNESS_LITTLE),
	m_in_pa_cb(*this, 0),
	m_in_pb_cb(*this, 0),
	m_in_pc_cb(*this, 0),
	m_in_ad_cb(*this, 0),
	m_out_pa_cb(*this),
	m_out_pb_cb(*this),
	m_out_pc_cb(*this)
{
}

void taito_cchip_device::ext_interrupt(int state)
{
	m_upd7811->set_input_line(UPD7810_INTF1, state);
}

ROM_START( taito_cchip )
	ROM_REGION( 0x1000, "upd7811", 0 )
	// optically extracted, the internal checksum passes, although that doesn't rule out the possibility of error
	ROM_LOAD( "cchip_upd78c11.bin", 0x0000, 0x1000, CRC(43021521) SHA1(73bc4b46cd2d6805ec926f39f22af00e38a3f822) )
ROM_END


u8 taito_cchip_device::asic_r(offs_t offset)
{
	if ((offset != 0x001) && (!machine().side_effects_disabled())) // prevent logerror spam for now
		logerror("%s: asic_r %04x\n", machine().describe_context(), offset);
	if (offset < 0x200) // 400-5ff is asic 'ram'
		return m_asic_ram[offset&3];
	return 0x00; // 600-7ff is write-only(?) asic banking reg, may read as open bus or never assert /DTACK on read?
}

void taito_cchip_device::asic_w(offs_t offset, u8 data)
{
	LOG("%s: asic_w %04x %02x\n", machine().describe_context(), offset, data);
	if (offset == 0x200)
	{
		LOG("cchip set bank to %02x\n", data & 0x7);
		m_upd4464_bank->set_entry(data & 0x7);
	}
	else
	{
		m_asic_ram[offset & 3] = data;
	}
}

void taito_cchip_device::asic68_w(offs_t offset, u8 data)
{
	LOG("%s: asic68_w %04x %02x\n", machine().describe_context(), offset, data);
	if (offset == 0x200)
	{
		LOG("cchip (68k side) set bank to %02x\n", data & 0x7);
		m_upd4464_bank68->set_entry(data & 0x7);
	}
	else
	{
		m_asic_ram[offset & 3] = data;
	}
}

void taito_cchip_device::cchip_map(address_map &map)
{
	//map(0x0000, 0x0fff).rom(); // internal ROM of uPD7811
	map(0x1000, 0x13ff).bankrw(m_upd4464_bank);
	map(0x1400, 0x17ff).rw(FUNC(taito_cchip_device::asic_r), FUNC(taito_cchip_device::asic_w));
	map(0x2000, 0x3fff).rom().region("cchip_eprom", 0);
}



void taito_cchip_device::device_add_mconfig(machine_config &config)
{
	upd78c11_device &upd(UPD78C11(config, m_upd7811, DERIVED_CLOCK(1, 1)));
	upd.set_addrmap(AS_PROGRAM, &taito_cchip_device::cchip_map);
	upd.pa_in_cb().set([this] { return m_in_pa_cb(); });
	upd.pb_in_cb().set([this] { return m_in_pb_cb(); });
	upd.pc_in_cb().set([this] { return m_in_pc_cb(); });
	upd.pa_out_cb().set([this] (u8 data) { m_out_pa_cb(data); });
	upd.pb_out_cb().set([this] (u8 data) { m_out_pb_cb(data); });
	upd.pc_out_cb().set([this] (u8 data) { m_out_pc_cb(data); });
	upd.pf_out_cb().set([this] (u8 data) { logerror("%s port F written %.2x\n", machine().describe_context(), data); }); // internal? related to locking out the 68k?
	upd.an0_func().set([this] { return BIT(m_in_ad_cb(), 0) ? 0xff : 0; });
	upd.an1_func().set([this] { return BIT(m_in_ad_cb(), 1) ? 0xff : 0; });
	upd.an2_func().set([this] { return BIT(m_in_ad_cb(), 2) ? 0xff : 0; });
	upd.an3_func().set([this] { return BIT(m_in_ad_cb(), 3) ? 0xff : 0; });
	upd.an4_func().set([this] { return BIT(m_in_ad_cb(), 4) ? 0xff : 0; });
	upd.an5_func().set([this] { return BIT(m_in_ad_cb(), 5) ? 0xff : 0; });
	upd.an6_func().set([this] { return BIT(m_in_ad_cb(), 6) ? 0xff : 0; });
	upd.an7_func().set([this] { return BIT(m_in_ad_cb(), 7) ? 0xff : 0; });
}

void taito_cchip_device::device_start()
{
	m_upd4464_bank->configure_entries(0, m_sharedram.length() / 0x400, &m_sharedram[0], 0x400);
	m_upd4464_bank->set_entry(0);

	// the 68k has a different view into the banked memory?
	m_upd4464_bank68->configure_entries(0, m_sharedram.length() / 0x400, &m_sharedram[0], 0x400);
	m_upd4464_bank68->set_entry(0);

	m_asic_ram[0] = m_asic_ram[1] = m_asic_ram[2] = m_asic_ram[3] = 0;
	save_item(NAME(m_asic_ram));
}

const tiny_rom_entry *taito_cchip_device::device_rom_region() const
{
	return ROM_NAME(taito_cchip);
}
