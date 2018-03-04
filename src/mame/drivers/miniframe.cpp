// license:GPL-2.0+
// copyright-holders:Dirk Best, R. Belmont
/***************************************************************************

    Convergent Miniframe

    Preliminary driver by R. Belmont based on unixpc.cpp by Dirk Best & R. Belmont

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "screen.h"

/***************************************************************************
    DRIVER STATE
***************************************************************************/

class miniframe_state : public driver_device
{
public:
	miniframe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_wd2797(*this, "wd2797"),
			m_floppy(*this, "wd2797:0:525dd"),
			m_ramrombank(*this, "ramrombank"),
			m_mapram(*this, "mapram")
	{ }

	required_device<m68010_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<wd2797_device> m_wd2797;
	required_device<floppy_image_device> m_floppy;
	required_device<address_map_bank_device> m_ramrombank;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ16_MEMBER(ram_mmu_r);
	DECLARE_WRITE16_MEMBER(ram_mmu_w);

	DECLARE_WRITE16_MEMBER(general_ctrl_w);

	DECLARE_WRITE_LINE_MEMBER( wd2797_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( wd2797_drq_w );

	required_shared_ptr<uint16_t> m_mapram;

	void miniframe(machine_config &config);
	void miniframe_mem(address_map &map);
	void ramrombank_map(address_map &map);
private:
	uint16_t *m_ramptr;
	uint32_t m_ramsize;
	uint16_t m_diskdmasize;
	uint32_t m_diskdmaptr;
	bool m_fdc_intrq;
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

READ16_MEMBER( miniframe_state::ram_mmu_r )
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

WRITE16_MEMBER( miniframe_state::ram_mmu_w )
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

WRITE16_MEMBER( miniframe_state::general_ctrl_w )
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

	// reset cpu so that it can pickup the new values
	m_maincpu->reset();

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

ADDRESS_MAP_START(miniframe_state::miniframe_mem)
	AM_RANGE(0x000000, 0x3fffff) AM_DEVICE("ramrombank", address_map_bank_device, amap16)
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_SHARE("mapram")
	AM_RANGE(0x450000, 0x450001) AM_WRITE(general_ctrl_w)
	AM_RANGE(0x800000, 0x81ffff) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0xc00000, 0xc00007) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0xc40000, 0xc40007) AM_DEVREADWRITE8("baudgen", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0xc90000, 0xc90003) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0x00ff)
ADDRESS_MAP_END

ADDRESS_MAP_START(miniframe_state::ramrombank_map)
	AM_RANGE(0x000000, 0x3fffff) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0x400000, 0x7fffff) AM_READWRITE(ram_mmu_r, ram_mmu_w)
ADDRESS_MAP_END

/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( miniframe )
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static SLOT_INTERFACE_START( miniframe_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

MACHINE_CONFIG_START(miniframe_state::miniframe)
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68010, XTAL(10'000'000))
	MCFG_CPU_PROGRAM_MAP(miniframe_mem)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("2M")

	// RAM/ROM bank
	MCFG_DEVICE_ADD("ramrombank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(ramrombank_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x400000)

	// floppy
	MCFG_DEVICE_ADD("wd2797", WD2797, 1000000)
//  MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(miniframe_state, wd2797_intrq_w))
//  MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(miniframe_state, wd2797_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("wd2797:0", miniframe_floppies, "525dd", floppy_image_device::default_floppy_formats)

	// 8263s
	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(76800)
	MCFG_PIT8253_CLK1(76800)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	// chain clock 1 output into clock 2
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pit8253", pit8253_device, write_clk2))
	// and ir4 on the PIC
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("pic8259", pic8259_device, ir4_w))

	MCFG_DEVICE_ADD("baudgen", PIT8253, 0)
	MCFG_PIT8253_CLK0(1228800)
	MCFG_PIT8253_CLK1(1228800)
	MCFG_PIT8253_CLK2(1228800)

	// PIC8259s
	MCFG_DEVICE_ADD("pic8259", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", M68K_IRQ_4))
	MCFG_PIC8259_IN_SP_CB(VCC)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( minifram )
	ROM_REGION16_BE(0x400000, "bootrom", 0)
	ROM_LOAD16_BYTE("72-00357.bin", 0x000001, 0x002000, CRC(17c2749c) SHA1(972b5300b4d6ec65536910eab2b8550b9df9bb4d))
	ROM_LOAD16_BYTE("72-00356.bin", 0x000000, 0x002000, CRC(28b6c23a) SHA1(479e739a8154b6754e2e9b1fcfeb99d6ceaf9dbe))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT   STATE         INIT  COMPANY  FULLNAME  FLAGS
COMP( 1985, minifram,  0,      0,      miniframe,  miniframe, miniframe_state, 0,    "Convergent",  "Miniframe",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
