// license:BSD-3-Clause
// copyright-holders:R. Belmont, Philip Bennett
/***************************************************************************

    Magic the Gathering: Armageddon
    Driver by R. Belmont and Phil Bennett


PCB Information (needs tidying:)


TOP Board
.20u    27c4001 stickered   U20
                #1537 V1.0  a 1 was handwritten over the 0

.u7     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON
                SND3 P/N 1605
                9806 D


.u8     stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1534 SND0
                9752 D

.u13        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1536 SND2
                9752 D

.u14        stamped     (c) 1997 11/25/97
                ACCLAIM COINOP
                ARMAGEDDON
                1535 SND1
                9752 D

Analog devices  ADSP 2181
Xilinx      XC5202
dt71256 x2
Analog Devices  AD1866R

Bottom board
.u32    27c801      stickered   4
.u33    27c801      stickered   3
.u34    27c801      stickered   2
.u35    27c801      stickered   1
.u58    AT17C128    stickered   CC3E
.u66    GAL16V8D

Xilinx  XC4005E
Xilinx  XC95108 stickered   ACCLAIM COIN-OP
                CSC2_10.JED
                B710/0B84
                U40 p/N 1611

3dFX    500-0003-03     x2
    BF1684.1
TI  TVP3409
    V53C16258HK40       x24
    V53C511816500K60    x4

U38 and U97 on main board   3DFX
                            500-0004-02
                            BF2733.1 TMU
                            9748 20001
                            TAIWAN 1001

U4 on daughter board        Zoran ZR36050PQC
                            -29.5
                            85 GF7B9726E

U11 on main board           Removed heatsink, Couldn't see anything...
                            On a second PCB: IDT 79RV5000-200BS272 YA9802C

U71 on main board           Galileo
                            GT-64010A-B-0
                            BB8018.1
                            TAIWAN
14.31818 Oscillator by the TI part
50.0000 Oscillator by EPROMS
33.0000 Oscillator by the V53C511816500K60
KM622560LG-7 by Battery

Bottom daughter board
All read as 29F800B
.u9     stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S0
                1514 11/25/97
                9803 D


.u10        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S1
                1515 11/25/97
                9803 D

.u11        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S3
                1517 11/25/97
                9803 D

.u12        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON S2
                1516 11/25/97
                9803 D

.u20        stamped     (c) 1997
                ACCLAIM COINOP
                ARMAGEDDON K0
                1543 11/25/97
                9752 D

Xilinx  XC4010E
Zoran   ZR36120PQC
Zoran   ZR36016PQC
Xilinx  XC3120A
    DT72811
    DT71256 x2
    DT72271
29.500000 oscillator by ZR36120PQC
Medium size chip with heat sink on it

***************************************************************************/

#include "emu.h"

#include "magictg_pinkeye.h"

#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "machine/gt64xxx.h"
#include "machine/pci.h"
#include "sound/dac.h"
#include "video/voodoo_pci.h"

#include "screen.h"
#include "speaker.h"

#include <string>

#define LOG_SERIAL (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGSERIAL(...) LOGMASKED(LOG_SERIAL, __VA_ARGS__)


namespace {

constexpr char PCI_ID_GALILEO[] = "pci:00.0";
constexpr char PCI_ID_VIDEO[]   = "pci:07.0";
constexpr char PCI_ID_PINKEYE[] = "pci:09.0";

class magictg_state : public driver_device
{
public:
	magictg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mips(*this, "mips")
		, m_adsp(*this, "adsp")
		, m_galileo(*this, PCI_ID_GALILEO)
		, m_voodoo(*this, PCI_ID_VIDEO)
		, m_adsp_rom(*this, "adsp_rom")
		, m_adsp_data(*this, "adsp_data")
		, m_adsp_dm_view(*this, "adsp_dm_view")
		, m_adsp_ext_ram(*this, "adsp_ext_ram", 0x8000 * 2, ENDIANNESS_LITTLE)
		, m_dac(*this, "dac%u", 0U)
		, m_track_x(*this, "TRACK%u_X", 0U)
		, m_track_y(*this, "TRACK%u_Y", 0U)
		, m_track_last{}
		, m_sram{}
		, m_cpld_irq_status(0)
		, m_cpld_irq_enable(0)
		, m_adsp_ram_bank(0)
		, m_adsp_rom_bank(0)
		, m_host_cmd(0)
		, m_host_cmd_full(false)
		, m_adsp_reply(0)
		, m_adsp_reply_full(false)
	{
	}

	void magictg(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<mips3_device> m_mips;
	required_device<adsp2181_device> m_adsp;
	required_device<gt64010_device> m_galileo;
	required_device<voodoo_1_pci_device> m_voodoo;
	required_region_ptr<uint8_t> m_adsp_rom;
	required_region_ptr<uint8_t> m_adsp_data;
	memory_view m_adsp_dm_view;
	memory_share_creator<uint16_t> m_adsp_ext_ram;
	required_device_array<dac_16bit_r2r_twos_complement_device, 2> m_dac;
	required_ioport_array<2> m_track_x;
	required_ioport_array<2> m_track_y;

	uint8_t m_track_last[2][2];
	uint32_t m_sram[0x3000 / 4];
	uint8_t m_cpld_irq_status, m_cpld_irq_enable;

	uint8_t m_adsp_ram_bank, m_adsp_rom_bank;
	uint16_t m_host_cmd;
	bool m_host_cmd_full;
	uint16_t m_adsp_reply;
	bool m_adsp_reply_full;

	std::string m_serial_line;

	void pinkeye_irq_w(int state);
	void update_irq();
	uint32_t cpld_irq_r();
	void cpld_irq_enable_w(uint8_t data);
	void reset_w(uint8_t data);

	uint32_t unk_r();
	uint32_t unk2_r();
	void serial_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t adsp_host_r(offs_t offset, uint32_t mem_mask = ~0);
	void adsp_host_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t adsp_status_r();
	uint8_t adsp_bdma_r(offs_t offset);
	uint8_t adsp_pf_r();
	void adsp_dmovlay_w(uint32_t data);
	uint16_t adsp_ext_ram_r(offs_t offset);
	void adsp_ext_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t adsp_latch_r();
	void adsp_latch_w(uint16_t data);
	template <int Player> uint16_t trackball_r();

	void adsp_data_map(address_map &map) ATTR_COLD;
	void adsp_io_map(address_map &map) ATTR_COLD;
	void adsp_program_map(address_map &map) ATTR_COLD;
	void cs0_map(address_map &map) ATTR_COLD;
	void cs1_map(address_map &map) ATTR_COLD;
	void cs2_map(address_map &map) ATTR_COLD;
	void cs3_map(address_map &map) ATTR_COLD;
};


/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

void magictg_state::machine_start()
{
	save_item(NAME(m_track_last));
	save_item(NAME(m_sram));
	save_item(NAME(m_cpld_irq_status));
	save_item(NAME(m_cpld_irq_enable));
	save_item(NAME(m_adsp_ram_bank));
	save_item(NAME(m_adsp_rom_bank));
	save_item(NAME(m_host_cmd));
	save_item(NAME(m_host_cmd_full));
	save_item(NAME(m_adsp_reply));
	save_item(NAME(m_adsp_reply_full));
}

void magictg_state::machine_reset()
{
	m_adsp_dm_view.select(0);
	m_adsp_ram_bank = 0;
	m_adsp_rom_bank = 0;
	m_host_cmd_full = false;
	m_adsp_reply_full = false;
	m_adsp->set_input_line(ADSP2181_IRQL0, CLEAR_LINE);

	// HACK: the host reads the switches while the DSP is still checksumming its ROMs, before
	// the DSP has written them; real hardware boots much more slowly, so start them released
	for (offs_t addr = 0x3a4e; addr <= 0x3a56; addr++)
	{
		m_adsp->space(AS_DATA).write_word(addr, 0xffff);
	}
}


/***************************************************************************
    CPLD
***************************************************************************/

void magictg_state::pinkeye_irq_w(int state)
{
	m_cpld_irq_status = (m_cpld_irq_status & ~0x80) | (state ? 0x80 : 0x00);
	update_irq();
}

void magictg_state::update_irq()
{
	m_mips->set_input_line(MIPS3_IRQ2, (m_cpld_irq_status & m_cpld_irq_enable) ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t magictg_state::cpld_irq_r()
{
	return m_cpld_irq_status;
}

void magictg_state::cpld_irq_enable_w(uint8_t data)
{
	m_cpld_irq_enable = data;
	update_irq();
}

void magictg_state::reset_w(uint8_t data)
{
	// bit 0 resets the board, which the game does to enter service mode once it's flagged
	// that in NVRAM; bit 1 holds the ADSP-2181 in reset
	if (BIT(data, 0))
	{
		machine().schedule_soft_reset();
		return;
	}
	m_adsp->set_input_line(INPUT_LINE_RESET, BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t magictg_state::unk_r()
{
	// will not boot otherwise
	return 0x6000;
}

uint32_t magictg_state::unk2_r()
{
	return 0xffffffff;
}

void magictg_state::serial_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// the game's debug console
	if ((offset == 0) && ACCESSING_BITS_24_31)
	{
		const char c = char(data >> 24);
		if ((c == '\n') || (m_serial_line.length() >= 256))
		{
			LOGSERIAL("%s\n", m_serial_line);
			m_serial_line.clear();
		}
		if ((c != '\n') && (c != '\r'))
		{
			m_serial_line += c;
		}
	}
}


/***************************************************************************
    MAD COW HOST INTERFACE
***************************************************************************/

// halfwords: 0x100 is the command/reply latch, 0x102 IDMA data, 0x104 the IDMA address
uint32_t magictg_state::adsp_host_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	if (offset == 0)
	{
		if (ACCESSING_BITS_0_15)
		{
			result |= m_adsp->idma_data_r();
		}
		if (ACCESSING_BITS_16_31)
		{
			result |= uint32_t(m_adsp_reply) << 16;
			if (!machine().side_effects_disabled())
			{
				m_adsp_reply_full = false;
			}
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		result |= uint32_t(m_adsp->idma_addr_r()) << 16;
	}
	return result;
}

void magictg_state::adsp_host_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == 0)
	{
		if (ACCESSING_BITS_0_15)
		{
			m_adsp->idma_data_w(data & 0xffff);
		}
		if (ACCESSING_BITS_16_31)
		{
			m_host_cmd = data >> 16;
			m_host_cmd_full = true;
			m_adsp->set_input_line(ADSP2181_IRQL0, ASSERT_LINE);
		}
	}
	else if (ACCESSING_BITS_16_31)
	{
		m_adsp->idma_addr_w(data >> 16);
	}
}

uint32_t magictg_state::adsp_status_r()
{
	// bit 2 is the ADSP /IACK pin; IDMA accesses complete at once here
	return (m_host_cmd_full ? 0 : 1) | (m_adsp_reply_full ? 2 : 0);
}

uint8_t magictg_state::adsp_bdma_r(offs_t offset)
{
	// IO port 1 selects the boot ROM or one of the four sound ROMs
	if (m_adsp_rom_bank == 0)
	{
		return m_adsp_rom[offset & 0x7ffff];
	}
	else if (m_adsp_rom_bank <= 4)
	{
		return m_adsp_data[((m_adsp_rom_bank - 1) << 22) + offset];
	}
	return 0xff;
}

uint8_t magictg_state::adsp_pf_r()
{
	// PF0 is set once the host has taken the last reply
	return m_adsp_reply_full ? 0xfe : 0xff;
}

void magictg_state::adsp_dmovlay_w(uint32_t data)
{
	m_adsp_dm_view.select(data ? 1 : 0);
}

uint16_t magictg_state::adsp_ext_ram_r(offs_t offset)
{
	return m_adsp_ext_ram[(m_adsp_ram_bank << 13) + offset];
}

void magictg_state::adsp_ext_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_adsp_ext_ram[(m_adsp_ram_bank << 13) + offset]);
}

uint16_t magictg_state::adsp_latch_r()
{
	if (!machine().side_effects_disabled())
	{
		m_host_cmd_full = false;
		m_adsp->set_input_line(ADSP2181_IRQL0, CLEAR_LINE);
	}
	return m_host_cmd;
}

void magictg_state::adsp_latch_w(uint16_t data)
{
	m_adsp_reply = data;
	m_adsp_reply_full = true;
}

template <int Player>
uint16_t magictg_state::trackball_r()
{
	// counts since the last read, X in the high byte and Y in the low
	const uint8_t x = m_track_x[Player]->read();
	const uint8_t y = m_track_y[Player]->read();
	const uint8_t dx = x - m_track_last[Player][0];
	const uint8_t dy = y - m_track_last[Player][1];
	if (!machine().side_effects_disabled())
	{
		m_track_last[Player][0] = x;
		m_track_last[Player][1] = y;
	}
	return (dx << 8) | dy;
}

// the GT-64010's device chip selects, as the boot ROM sets them up
void magictg_state::cs0_map(address_map &map)
{
	map(0x000100, 0x000107).rw(FUNC(magictg_state::adsp_host_r), FUNC(magictg_state::adsp_host_w));
}

void magictg_state::cs1_map(address_map &map)
{
}

void magictg_state::cs2_map(address_map &map)
{
	map(0x1001004, 0x1001007).umask32(0x0000'00ff).w(FUNC(magictg_state::cpld_irq_enable_w));
	map(0x100100c, 0x100100f).r(FUNC(magictg_state::cpld_irq_r));
	map(0x1001018, 0x100101b).umask32(0x0000'00ff).w(FUNC(magictg_state::reset_w));
	map(0x1001024, 0x1001027).r(FUNC(magictg_state::adsp_status_r));
	map(0x1001108, 0x100110b).r(FUNC(magictg_state::unk_r));
}

void magictg_state::cs3_map(address_map &map)
{
	map(0x000000, 0x002fff).lrw32(
			NAME([this] (offs_t offset) -> uint32_t { return m_sram[offset]; }),
			NAME([this] (offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_sram[offset]); }));
	map(0x800000, 0x800007).rw(FUNC(magictg_state::unk2_r), FUNC(magictg_state::serial_w));
}

void magictg_state::adsp_program_map(address_map &map)
{
	map(0x0000, 0x3fff).ram();
}

void magictg_state::adsp_data_map(address_map &map)
{
	map(0x0000, 0x1fff).view(m_adsp_dm_view);
	m_adsp_dm_view[0](0x0000, 0x1fff).ram();
	m_adsp_dm_view[1](0x0000, 0x1fff).rw(FUNC(magictg_state::adsp_ext_ram_r), FUNC(magictg_state::adsp_ext_ram_w));
	map(0x2000, 0x3fdf).ram();
}

void magictg_state::adsp_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x000).lw16(NAME([this] (uint16_t data) { m_adsp_ram_bank = data & 3; }));
	map(0x001, 0x001).lw16(NAME([this] (uint16_t data) { m_adsp_rom_bank = data & 7; }));
	map(0x003, 0x003).rw(FUNC(magictg_state::adsp_latch_r), FUNC(magictg_state::adsp_latch_w));
	map(0x004, 0x004).r(FUNC(magictg_state::trackball_r<1>));
	map(0x005, 0x005).portr("SW0");
	map(0x006, 0x006).portr("SW1");
	map(0x007, 0x007).portr("SW2");
	map(0x008, 0x008).nopw(); // lamps or coin counters?
	map(0x00f, 0x00f).r(FUNC(magictg_state::trackball_r<0>));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( magictg )
	PORT_START("SW0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x03f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0400, IP_ACTIVE_LOW )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("%p Blast")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("%p Creature")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("%p Teleport")
	PORT_BIT( 0x0f80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("%p Blast")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("%p Creature")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("%p Teleport")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void magictg_state::magictg(machine_config &config)
{
	R5000BE(config, m_mips, 200'000'000); // exact model 79RV5000-200BS272 rated for 200 MHz, clock not measured

	PCI_ROOT(config, "pci");

	GT64010(config, m_galileo, 50'000'000, m_mips, -1);
	m_galileo->set_be(1);
	m_galileo->set_map(0, address_map_constructor(&magictg_state::cs0_map, "cs0_map", this), this);
	m_galileo->set_map(1, address_map_constructor(&magictg_state::cs1_map, "cs1_map", this), this);
	m_galileo->set_map(2, address_map_constructor(&magictg_state::cs2_map, "cs2_map", this), this);
	m_galileo->set_map(3, address_map_constructor(&magictg_state::cs3_map, "cs3_map", this), this);
	m_galileo->set_simm0_size(0x00800000);

	ADSP2181(config, m_adsp, 32'000'000); // twice a 16 MHz CLKIN, unverified
	m_adsp->set_addrmap(AS_PROGRAM, &magictg_state::adsp_program_map);
	m_adsp->set_addrmap(AS_DATA, &magictg_state::adsp_data_map);
	m_adsp->set_addrmap(AS_IO, &magictg_state::adsp_io_map);
	m_adsp->set_internal_peripherals(true);
	m_adsp->bdma_read().set(FUNC(magictg_state::adsp_bdma_r));
	m_adsp->pf_in().set(FUNC(magictg_state::adsp_pf_r));
	m_adsp->dmovlay().set(FUNC(magictg_state::adsp_dmovlay_w));
	m_adsp->sport_dt<0>().set(m_dac[0], FUNC(dac_16bit_r2r_twos_complement_device::write));
	m_adsp->sport_dt<1>().set(m_dac[1], FUNC(dac_16bit_r2r_twos_complement_device::write));

	SPEAKER(config, "speaker", 2).front();

	// AD1866R dual DAC; SPORT1 is clocked and framed by SPORT0
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[0]).add_route(ALL_OUTPUTS, "speaker", 1.0, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_dac[1]).add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	// TODO: a second Voodoo, at PCI device 8, makes an SLI pair
	VOODOO_1_PCI(config, m_voodoo, m_mips, "screen");
	m_voodoo->set_fbmem(2);
	m_voodoo->set_tmumem(4, 0);
	m_voodoo->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo->subdevice<generic_voodoo_device>("voodoo")->stall_callback().set(m_galileo, FUNC(gt64xxx_device::pci_stall));

	ACCLAIM_PINKEYE(config, PCI_ID_PINKEYE).irq_cb().set(FUNC(magictg_state::pinkeye_irq_w));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024, 1024);
	screen.set_visarea(0, 511, 16, 447);
	screen.set_screen_update(m_voodoo, FUNC(voodoo_1_pci_device::screen_update));
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( magictg )
	ROM_REGION64_BE( 0x400000, "pci:00.0:rom", 0 )
	ROM_LOAD16_BYTE( "magic.u34", 0x000000, 0x100000, CRC(2e8971e2) SHA1(9bdf433a7c7257389ebdf131317ef26a7d4e1ba2) )
	ROM_LOAD16_BYTE( "magic.u35", 0x000001, 0x100000, CRC(e2202143) SHA1(f07b7da81508cd4594f66e34dabd904a21eb03f0) )
	ROM_LOAD16_BYTE( "magic.u32", 0x200000, 0x100000, CRC(f1d530e3) SHA1(fcc392804cd6b98917a869cc5d3826278b7ba90b) )
	ROM_LOAD16_BYTE( "magic.u33", 0x200001, 0x100000, CRC(b2330cfc) SHA1(559c35426588b349ef31bf8b296b950912f6fcc7) )

	ROM_REGION( 0x80000, "adsp_rom", 0 )
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION( 0x1000000, "adsp_data", 0 )
	ROM_LOAD( "magic.snd0.u8",  0x000000, 0x400000, CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14", 0x400000, 0x400000, CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13", 0x800000, 0x400000, CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7",  0xc00000, 0x400000, CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "pci:09.0:jpeg", 0 )
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "pci:09.0:key", 0 )
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
ROM_END

ROM_START( magictga )
	ROM_REGION64_BE( 0x400000, "pci:00.0:rom", 0 )
	ROM_LOAD16_BYTE( "magic.u63", 0x000000, 0x100000, CRC(a10d45f1) SHA1(0ede10f19cf70baf7b43e3f672532b4be1a179f8) )
	ROM_LOAD16_BYTE( "magic.u64", 0x000001, 0x100000, CRC(8fdb6060) SHA1(b638244cad86dc60435a4a9150a5b639f5d61a3f) )
	ROM_LOAD16_BYTE( "magic.u61", 0x200000, 0x100000, CRC(968891d6) SHA1(67ab87039864bb148d20795333ffa7a23e3b84f2) )
	ROM_LOAD16_BYTE( "magic.u62", 0x200001, 0x100000, CRC(690946eb) SHA1(6c9b02367704309f4fde5cbd9d195a45c32c3861) )

	// this set was incomplete, none of these ROMs were dumped for it, are they the same?
	ROM_REGION( 0x80000, "adsp_rom", 0 )
	ROM_LOAD( "magic.20u", 0x00000, 0x80000, BAD_DUMP CRC(50968301) SHA1(e9bdd0c942f0c66e18aa8de5a04edb51cdf1fee8) )

	ROM_REGION( 0x1000000, "adsp_data", 0 )
	ROM_LOAD( "magic.snd0.u8",  0x000000, 0x400000, BAD_DUMP CRC(3cb81717) SHA1(9d35796381ca57e9782e0338c456e63c31d11266) )
	ROM_LOAD( "magic.snd1.u14", 0x400000, 0x400000, BAD_DUMP CRC(b4ef9977) SHA1(dedc79e5d506bb0d1649a41b9912dcc999e1da72) )
	ROM_LOAD( "magic.snd2.u13", 0x800000, 0x400000, BAD_DUMP CRC(3728f16e) SHA1(6b7da30b100d053e95aa96edf74a0474f1493dfb) )
	ROM_LOAD( "magic.snd3.u7",  0xc00000, 0x400000, BAD_DUMP CRC(11a1cb63) SHA1(a1048d3cd580747c20eb0b4e816e7e4e0f5c8c2b) )

	ROM_REGION( 0x2000000, "pci:09.0:jpeg", 0 )
	ROM_LOAD( "magic_s0.u9",  0x0000000, 0x800000, BAD_DUMP CRC(a01b5b99) SHA1(e77f2e9b08a97d6118e1e307b38ea79d0177e9b8) )
	ROM_LOAD( "magic_s1.u10", 0x0800000, 0x800000, BAD_DUMP CRC(d5a1a557) SHA1(2511ee8d08da765a2fa2d42fb504793f9e8b615c) )
	ROM_LOAD( "magic_s2.u12", 0x1000000, 0x800000, BAD_DUMP CRC(06ed6770) SHA1(884a3e4c97a50fa926546eb6def2c11c5732ba88) )
	ROM_LOAD( "magic_s3.u11", 0x1800000, 0x800000, BAD_DUMP CRC(71d4c252) SHA1(aeab2542b9d5fb63f4d60b808010a657a895c1d7) )

	ROM_REGION( 0x400000, "pci:09.0:key", 0 )
	ROM_LOAD( "magic.k0.u20", 0x000000, 0x400000, BAD_DUMP CRC(63ab0e9e) SHA1(c4f0b009860ee499496ed7fc1f14ef1e221c1085) )
ROM_END

} // anonymous namespace


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

GAME( 1997, magictg,  0,       magictg, magictg, magictg_state, empty_init, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, magictga, magictg, magictg, magictg, magictg_state, empty_init, ROT0, "Acclaim", "Magic the Gathering: Armageddon (set 2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
