/****************************************************************************

    Power Macintosh x400 "Alchemy" hardware
    Loosely based off the DingusPPC driver

    The "Alchemy" board is a follow-on to the much maligned Cordyceps (aka "Elixir").
    It ditches the copypasted Quadra 630 architecture for a new PCI-based one
    centered around the "PSX" PCI/memory controller. The backplane is still based
    on the Quadra 630, though, so these boards could be used as drop-in replacements
    in the "Bongo" all-in-one case.

	Basic architecture:
	- PowerPC 603e
	- PSX (DRAM/ROM controller + Bandit PCI)
	- Valkyrie-AR (Valkyrie from Quadra 630 and Cordyceps but mapped differently)
	- O'Hare PCI-to-Mac I/O chip, same as in Power Mac 7500 "TNT"
	
	Current status: Tries to play the boot chime, but the DMA engine hangs
	and so does the CPU. Skipping this gets us to a point where the 68k
	emulator runs, but we have a black screen.



 ****************************************************************************/

#include "emu.h"

#include "awacs_macrisc.h"
#include "bandit.h"
#include "cuda.h"
#include "heathrow.h"
#include "valkyrie.h"

#include "bus/adb/adb.h"
#include "bus/adb/cards.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/pci/pci_slot.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "softlist_dev.h"

#define LOG_NVRAM   (1U << 1)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define MAIN_BUS_FREQUENCY 40_MHz_XTAL

namespace { // anonymous namespace

class pmac6400_state : public driver_device
{
public:
	pmac6400_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
	    m_pci_root(*this, "pci"),
	    m_psx(*this, "pci:00.0"),
        m_ohare(*this, "pci:10.0"),
		m_video(*this, "valkyrie"),
		m_scsibus(*this, "scsi"),
		m_adbbus(*this, "adb"),
		m_cuda(*this, "cuda"),
		m_ram(*this, RAM_TAG),
	    m_nvram(*this, "nvram")
	{
	}
    

	void pmac6400(machine_config &config);

	void pmac6400_map(address_map &map) ATTR_COLD;

	void init_pmac6400();
private:
	required_device<ppc603e_device> m_maincpu;
	required_device<pci_root_device> m_pci_root;
    required_device<applpsx_host_device> m_psx;
    required_device<ohare_device> m_ohare;
	required_device<valkyrie_device> m_video;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<adb_bus_device> m_adbbus;
	required_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
    required_device<nvram_device> m_nvram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void cuda_reset_w(int state)
	{
		m_maincpu->set_input_line(INPUT_LINE_HALT, state);
		m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	}

	void nmi_irq(int state)
	{
		// ??
	}


	u32 nvram_addr_r(offs_t offset, u32 mem_mask);
	void nvram_addr_w(offs_t offset, u32 data, u32 mem_mask);
	u32 nvram_data_r(offs_t offset, u32 mem_mask);
	void nvram_data_w(offs_t offset, u32 data, u32 mem_mask);

	u32 machine_id_r(offs_t offset, u32 mem_mask);

	void slot_irq_handler(int line, int state);

	u16 m_machine_id;

	u32 m_nvram_addr;
	u8 m_nvram_data[0x2000];
};

void pmac6400_state::machine_start()
{
	m_nvram->set_base(&m_nvram_data[0], sizeof(m_nvram_data));

	m_pci_root->set_irq_handler(pci_irq_handler(*this, FUNC(pmac6400_state::slot_irq_handler)));
}

void pmac6400_state::machine_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

void pmac6400_state::init_pmac6400()
{
}


u32 pmac6400_state::nvram_addr_r(offs_t offset, u32 mem_mask)
{
	return m_nvram_addr;
}

void pmac6400_state::nvram_addr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_nvram_addr);
	LOGMASKED(LOG_NVRAM, "NVRAM addr: %08x (mask %08x)\n", m_nvram_addr, mem_mask);
}

u32 pmac6400_state::nvram_data_r(offs_t offset, u32 mem_mask)
{
	offset >>= 2;
	// high address (page) plus the 5-bit index within the data window; mask to
	// the backing store so a stray address register can't run off the end
	const u32 addr = (offset + (m_nvram_addr << 5)) & (sizeof(m_nvram_data) - 1);
	LOGMASKED(LOG_NVRAM, "NVRAM read @ %x (nvram_addr %x offset %x)\n", addr, m_nvram_addr, offset);
	return m_nvram_data[addr];
}

void pmac6400_state::nvram_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset >>= 2;
	const u32 addr = (offset + (m_nvram_addr << 5)) & (sizeof(m_nvram_data) - 1);
	COMBINE_DATA(&m_nvram_data[addr]);
	LOGMASKED(LOG_NVRAM, "NVRAM write: %02x @ %x (nvram_addr %x offset %x)\n", data & 0xff, addr, m_nvram_addr, offset);
}

u32 pmac6400_state::machine_id_r(offs_t offset, u32 mem_mask)
{
    // same as catalyst machineregister
    // 0xE0 = pmac6400, 0xF0 = pmac5400
	return m_machine_id;
}

void pmac6400_state::slot_irq_handler(int line, int state)
{
	// TODO: determine how PCI IRQs fire on this board
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/


void pmac6400_state::pmac6400_map(address_map &map)
{
    map(0x00000000, 0x03ffffff).ram();

	map(0x00000000, 0xffffffff).m(m_video, FUNC(valkyrie_device::valkyriear_map));

    map(0xffc00000, 0xffffffff).rom().region("bootrom", 0);
}

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

void pmac6400_state::pmac6400(machine_config &config)
{
    m_machine_id = 0xE03F;

	PPC603E(config, m_maincpu, 225'000'000);
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_maincpu->set_addrmap(AS_PROGRAM, &pmac6400_state::pmac6400_map);
	config.set_perfect_quantum(m_maincpu);

    // Bandit actually integrated into the PSX chip
	PCI_ROOT(config, m_pci_root, 0);
	APPLPSX(config, m_psx, 50_MHz_XTAL, "maincpu");
	m_psx->set_dev_offset(1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M, 32M, 64M");

	OHARE(config, m_ohare);
	m_ohare->set_maincpu_tag("maincpu");
	m_ohare->irq_callback().set_inputline(m_maincpu, PPC_IRQ);
	m_ohare->iobus_a_r_callback().set(FUNC(pmac6400_state::machine_id_r));
    
	screamer_device &screamer(SCREAMER(config, "codec", 45.1584_MHz_XTAL / 2));
	screamer.dma_output().set(m_ohare, FUNC(ohare_device::codec_dma_read));
	screamer.dma_input().set(m_ohare, FUNC(ohare_device::codec_dma_write));
	m_ohare->codec_r_callback().set(screamer, FUNC(screamer_device::read_macrisc));
	m_ohare->codec_w_callback().set(screamer, FUNC(screamer_device::write_macrisc));
	
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
			[] (device_t *device)
			{
				device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
				device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1);
			});
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);

	VALKYRIE(config, m_video, 31.3344_MHz_XTAL);
	m_video->write_irq().set(m_ohare, FUNC(ohare_device::set_irq_line<0x18>));


	SPEAKER(config, "speaker", 2).front();
	screamer.add_route(0, "speaker", 1.0, 0);
	screamer.add_route(1, "speaker", 1.0, 1);

	ADB_BUS(config, m_adbbus);
	ADB_CONNECTOR(config, "adb:0", adb_devices, "hle_keyboard");
	ADB_CONNECTOR(config, "adb:1", adb_devices, "hle_mouse");

	CUDA_V2XX(config, m_cuda, XTAL(32'768));
	m_cuda->set_default_bios_tag("341s0060");
	m_cuda->reset_callback().set(FUNC(pmac6400_state::cuda_reset_w));
	m_cuda->linechange_callback().set(m_adbbus, FUNC(adb_bus_device::adb_host_line_w));
	m_cuda->via_clock_callback().set(m_ohare, FUNC(ohare_device::cb1_w));
	m_cuda->via_data_callback().set(m_ohare, FUNC(ohare_device::cb2_w));
	m_cuda->nmi_callback().set(FUNC(pmac6400_state::nmi_irq));
	

	m_adbbus->out_adb_callback().set(m_cuda, FUNC(cuda_device::set_adb_line));
	m_adbbus->out_poweron_callback().set(m_cuda, FUNC(cuda_device::set_adb_power));

	config.set_perfect_quantum(m_maincpu);

	m_ohare->pb3_callback().set(m_cuda, FUNC(cuda_device::get_treq));
	m_ohare->pb4_callback().set(m_cuda, FUNC(cuda_device::set_byteack));
	m_ohare->pb5_callback().set(m_cuda, FUNC(cuda_device::set_tip));
	m_ohare->cb2_callback().set(m_cuda, FUNC(cuda_device::set_via_data));
}


#define PPC_MAKE_BRANCH_ALWAYS(x) \
	ROM_FILL(x,   1, 0x48) \
	ROM_FILL(x+1, 1, 0x00) \
	// .

#define PPC_ASSEMBLE_NOP(x) \
	ROM_FILL(x,   1, 0x7f) \
	ROM_FILL(x+1, 1, 0xff) \
	ROM_FILL(x+2, 1, 0xfb) \
	ROM_FILL(x+3, 1, 0x78) \
	// .

ROM_START( pmac6400 )
	ROM_REGION64_BE(0x400000, "bootrom", 0)
	ROM_LOAD( "6f5724c0.bin", 0x000000, 0x400000, CRC(ec9914be) SHA1(822ab19b360b8fa25238e531dcb85a4459f7c8be) )

	// HACK: there's a bug somewhere that will cause the
	// bootrom to hang when it tries to play the chime.
	// disable that function for now so we can focus on bringing up other stuff
	PPC_ASSEMBLE_NOP(0x303044)

ROM_END

} // anonymous namespace



//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT   CLASS           INIT            COMPANY           FULLNAME                   FLAGS
COMP( 1996, pmac6400, 0,        0,      pmac6400, macadb, pmac6400_state, init_pmac6400,  "Apple Computer", "Power Macintosh 6400", MACHINE_NOT_WORKING)
