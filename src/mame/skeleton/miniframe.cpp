// license:GPL-2.0+
// copyright-holders:Dirk Best, R. Belmont
/***************************************************************************

    Convergent Miniframe

    Preliminary driver by R. Belmont based on unixpc.cpp by Dirk Best & R. Belmont

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68010.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "screen.h"


namespace {

/***************************************************************************
    DRIVER STATE
***************************************************************************/

class miniframe_state : public driver_device
{
public:
	miniframe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_wd2797(*this, "wd2797")
		, m_floppy(*this, "wd2797:0:525dd")
		, m_ramrombank(*this, "ramrombank")
		, m_mapram(*this, "mapram")
	{ }

	void miniframe(machine_config &config);

private:
	required_device<m68010_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<wd2797_device> m_wd2797;
	required_device<floppy_image_device> m_floppy;
	required_device<address_map_bank_device> m_ramrombank;

	[[maybe_unused]] uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t ram_mmu_r(offs_t offset);
	void ram_mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void general_ctrl_w(uint16_t data);

	void wd2797_intrq_w(int state);
	void wd2797_drq_w(int state);

	required_shared_ptr<uint16_t> m_mapram;

	void miniframe_mem(address_map &map) ATTR_COLD;
	void ramrombank_map(address_map &map) ATTR_COLD;

	uint16_t *m_ramptr = nullptr;
	uint32_t m_ramsize = 0;
	uint16_t m_diskdmasize = 0;
	uint32_t m_diskdmaptr = 0;
	bool m_fdc_intrq = false;
};


/***************************************************************************
    MEMORY
***************************************************************************/

static constexpr unsigned MMU_MAX_PAGES = 1024;
static constexpr uint16_t MMU_WRITE_ENABLE = 0x8000;
static constexpr uint16_t MMU_STATUS_MASK  = 0x6000;
static constexpr uint16_t MMU_STATUS_NOT_PRESENT = 0x0000;
static constexpr uint16_t MMU_STATUS_PRESENT_NOT_ACCESSED = 0x2000;
static constexpr uint16_t MMU_STATUS_ACCESSED_NOT_WRITTEN = 0x4000;
static constexpr uint16_t MMU_STATUS_ACCESSED_WRITTEN = 0x6000;

uint16_t miniframe_state::ram_mmu_r(offs_t offset)
{
	uint8_t fc = m_maincpu->get_fc();
	uint16_t mapentry = m_mapram[(offset >> 11) & 0x7ff];

	if ((offset < ((512*1024)>>1)) && (fc != M68K_FC_SUPERVISOR_DATA) && (fc != M68K_FC_SUPERVISOR_PROGRAM))
	{
		fatalerror("mmu: user mode access to lower 512K, need to generate a fault\n");
	}

	if ((mapentry & MMU_STATUS_MASK) != MMU_STATUS_NOT_PRESENT)
	{
		uint32_t addr = (offset & 0x7ff) | ((mapentry & 0xfff) << 11);
		//printf("mmu_r: orig %x entry %04x xlate %x\n", offset, mapentry, addr);

		// indicate page has been read
		if ((mapentry & MMU_STATUS_MASK) == MMU_STATUS_PRESENT_NOT_ACCESSED)
		{
			m_mapram[(offset >> 11) & 0x7ff] &= ~MMU_STATUS_MASK;
			m_mapram[(offset >> 11) & 0x7ff] |= MMU_STATUS_ACCESSED_NOT_WRITTEN;
		}

		return m_ramptr[addr];
	}
	else
	{
		fatalerror("miniframe: invalid MMU page accessed, need to throw a fault\n");
	}
}

void miniframe_state::ram_mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint8_t fc = m_maincpu->get_fc();
	uint16_t mapentry = m_mapram[(offset >> 11) & 0x7ff];

	if ((offset < ((512*1024)>>1)) && (fc != M68K_FC_SUPERVISOR_DATA) && (fc != M68K_FC_SUPERVISOR_PROGRAM))
	{
		fatalerror("mmu: user mode access to lower 512K, need to generate a fault\n");
	}

	if ((mapentry & MMU_STATUS_MASK) != MMU_STATUS_NOT_PRESENT)
	{
		uint32_t addr = (offset & 0x7ff) | ((mapentry & 0xfff) << 11);
		//printf("mmu_w: orig %x entry %04x xlate %x\n", offset, mapentry, addr);

		if (!(mapentry & MMU_WRITE_ENABLE) && (fc != M68K_FC_SUPERVISOR_PROGRAM) && (fc != M68K_FC_SUPERVISOR_DATA))
		{
			fatalerror("mmu: write protection violation, need to throw a fault\n");
		}

		// indicate page has been written
		// we know it's OK to just OR this
		m_mapram[(offset >> 11) & 0x7ff] |= MMU_STATUS_ACCESSED_WRITTEN;

		COMBINE_DATA(&m_ramptr[addr]);
	}
	else
	{
		fatalerror("miniframe: invalid MMU page accessed, need to throw a fault\n");
	}
}

void miniframe_state::general_ctrl_w(uint16_t data)
{
	if (data & 0x1000)  // ROM mirror at 0 if set
	{
		m_ramrombank->set_bank(1);
	}
	else
	{
		m_ramrombank->set_bank(0);
	}

	logerror("%x to general_ctrl_w\n", data);
}

void miniframe_state::machine_start()
{
	m_ramptr = (uint16_t *)m_ram->pointer();
	m_ramsize = m_ram->size();
}

void miniframe_state::machine_reset()
{
	// force ROM into lower mem on reset
	m_ramrombank->set_bank(0);

	// invalidate all pages by clearing all entries
	memset(m_mapram, 0, MMU_MAX_PAGES * sizeof(uint16_t));
}

/***************************************************************************
    VIDEO
***************************************************************************/

uint32_t miniframe_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void miniframe_state::miniframe_mem(address_map &map)
{
	map(0x000000, 0x3fffff).m(m_ramrombank, FUNC(address_map_bank_device::amap16));
	map(0x400000, 0x4007ff).ram().share("mapram");
	map(0x450000, 0x450001).w(FUNC(miniframe_state::general_ctrl_w));
	map(0x800000, 0x81ffff).rom().region("bootrom", 0);
	map(0xc00000, 0xc00007).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xc40000, 0xc40007).rw("baudgen", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xc90000, 0xc90003).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
}

void miniframe_state::ramrombank_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("bootrom", 0);
	map(0x400000, 0x7fffff).rw(FUNC(miniframe_state::ram_mmu_r), FUNC(miniframe_state::ram_mmu_w));
}

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( miniframe )
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static void miniframe_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void miniframe_state::miniframe(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &miniframe_state::miniframe_mem);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M");

	// RAM/ROM bank
	ADDRESS_MAP_BANK(config, "ramrombank").set_map(&miniframe_state::ramrombank_map).set_options(ENDIANNESS_BIG, 16, 32, 0x400000);

	// floppy
	WD2797(config, m_wd2797, 1000000);
//  m_wd2797->intrq_wr_callback().set(FUNC(miniframe_state::wd2797_intrq_w));
//  m_wd2797->drq_wr_callback().set(FUNC(miniframe_state::wd2797_drq_w));
	FLOPPY_CONNECTOR(config, "wd2797:0", miniframe_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);

	// 8263s
	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(76800);
	pit8253.set_clk<1>(76800);
	pit8253.out_handler<0>().set("pic8259", FUNC(pic8259_device::ir4_w)); // FIXME: fighting for IR4 - error, or needs input merger?
	// chain clock 1 output into clock 2
	pit8253.out_handler<1>().set("pit8253", FUNC(pit8253_device::write_clk2));
	// and ir4 on the PIC
	pit8253.out_handler<1>().append("pic8259", FUNC(pic8259_device::ir4_w));

	pit8253_device &baudgen(PIT8253(config, "baudgen", 0));
	baudgen.set_clk<0>(1228800);
	baudgen.set_clk<1>(1228800);
	baudgen.set_clk<2>(1228800);

	// PIC8259s
	pic8259_device &pic8259(PIC8259(config, "pic8259", 0));
	pic8259.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_4);
	pic8259.in_sp_callback().set_constant(1);
}


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( minifram )
	ROM_REGION16_BE(0x400000, "bootrom", 0)
	ROM_LOAD16_BYTE("72-00357.bin", 0x000001, 0x002000, CRC(17c2749c) SHA1(972b5300b4d6ec65536910eab2b8550b9df9bb4d))
	ROM_LOAD16_BYTE("72-00356.bin", 0x000000, 0x002000, CRC(28b6c23a) SHA1(479e739a8154b6754e2e9b1fcfeb99d6ceaf9dbe))
ROM_END

} // anonymous namespace


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY       FULLNAME     FLAGS
COMP( 1985, minifram, 0,      0,      miniframe, miniframe, miniframe_state, empty_init, "Convergent", "Miniframe", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
