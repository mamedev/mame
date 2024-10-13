// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

  sun3.c: preliminary driver for Sun 3x models.

  status: 3/80 POSTs, 3/460 needs its unique RTC chip (also used by non-3x Sun 3s).

  TODO:
    - Improve interrupt controller emulation.
    - Figure out how the IOMMU works.
    - Sun custom MMU for original Sun 3 models.
    - AM7990 LANCE chip support for everyone.
    - Figure out how the parallel printer port maps to Centronics and make it so.
    - Much more...


    Sun-3x Models
    ------------

    3/80
        Processor(s):   68030 @ 20MHz, 68882 @ 20MHz, 68030 on-chip
                        MMU, 3 MIPS, 0.16 MFLOPS
        CPU:            501-1401/1650
        Chassis type:   square pizza box
        Bus:            P4 connector (not same as P4 on 3/60)
        Memory:         16M or 40M physical, 4G virtual, 100ns cycle
        Notes:          Similar packaging to SparcStation 1. Parallel
                        port, SCSI port, AUI Ethernet, 1.44M 3.5" floppy
                        (720K on early units?). No onboard framebuffer.
                        Code-named "Hydra". Type-4 keyboard and Sun-4
                        mouse, plugged together and into the machine
                        with a small DIN plug. 1M x 9 30-pin 100ns
                        SIMMs. Boot ROM versions 3.0.2 and later allow
                        using 4M SIMMs in some slots for up to 40M (see
                        Misc Q&A #15).

    3/460
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          A 3/260 upgraded with a 3/4xx CPU board. Uses
                        original 3/2xx memory boards.

    3/470
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Chassis type:   deskside
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          Rare. Code-named "Pegasus". 8M standard, uses
                        same memory boards as 3/2xx.

    3/480
        Processor(s):   68030 @ 33 MHz, 68882, 68030 on-chip MMU,
                        7 MIPS, 0.6 MFLOPS
        CPU:            501-1299/1550
        Chassis type:   rackmount
        Bus:            VME
        Memory:         128M physical with ECC, 4G/process virtual,
                        64K cache, 80ns cycle
        Notes:          Rare. Code-named "Pegasus". 8M standard, uses
                        same memory boards as 3/2xx.

    Sun3X notes from NetBSD and Linux:

    RAM_END    0x40000000
    P4DAC      0x50200000
    VIDEO_P4ID 0x50300000
    BW2_ADDR   0x50400000
    ENA_PLANE  0x50600000
    FPA_ADDR   0x5c000000
    IOMMU      0x60000000
    ENABLEREG  0x61000000
    BUSERRREG  0x61000400
    DIAGREG    0x61000800
    IDPROM1    0x61000c00 (3/470)
    MEMREG     0x61001000
    INTERREG   0x61001400
    SCC1       0x62000000 (keyboard/mouse)
    SCC2       0x62002000 (serial console)
    EEPROM     0x64000000
    IDPROM2    0x640007d8 (3/80)
    CLOCK2     0x640007f8 (3/80 Mostek 48T02)
    CLOCK1     0x64002000 (3/470 Intersil 7170)
    INTELETH   0x65000000
    LANCEETH   0x65002000
    EMULEXSCSI 0x66000000 (3/80 5394)
    EMULLEXDMA 0x66001000 (3/80)
    PCACHETAG  0x68000000
    ECCPARREG  0x6a1e0000
    IOCTAGS    0x6c000000
    IOCFLUSH   0x6d000000
    FDC        0x6e000000 (3/80 Intel 82077)
    FDC_CNTRL  0x6e000400
    FDC_VEC    0x6e000800
    PRINTER    0x6f00003c (3/80)

    The Sun3x System Enable Register controls the function of a few
    on-board devices and general system operation.  It is cleared when
    the system is reset.

    15                                                               0
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---.---.---+
    |BT |FPP|DMA| 0 |VID|RES|FPA|DIA| 0 |CCH|IOC|LBK|DCH|  UNUSED   |
    +---+---+---+---+---+---+---+---+---+---+---+---+---+---.---.---+

    Where: DCH = debug mode for system cache
    LBK = VME loopback
    IOC = I/O cache enable
    CCH = system cache enable
    DIA = diagnostic switch
    FPA = enable floating-point accelerator
    RES = 0 for hi-res, 1 for low res
    VID = enable video display
    DMA = enable system DVMA
    FPP = enable 68881/2 FPU
    BT  = 0 for boot state, 1 for normal state

    bad '030 MMU mapping: L fef82000 -> P 00000000

****************************************************************************/

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "cpu/m68000/m68030.h"
#include "imagedev/floppy.h"
#include "machine/icm7170.h"
#include "machine/ncr53c90.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"

#include "bus/rs232/rs232.h"
#include "bus/sunkbd/sunkbd.h"
#include "bus/sunmouse/sunmouse.h"

#include "screen.h"

#include "formats/flopimg.h"


namespace {

#define TIMEKEEPER_TAG  "timekpr"
#define SCC1_TAG        "scc1"
#define SCC2_TAG        "scc2"
#define FDC_TAG         "fdc"
#define FLOPPY_CONN_TAG "fdc:0"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"
#define KEYBOARD_TAG    "keyboard"
#define MOUSE_TAG       "mouseport"

class sun3x_state : public driver_device
{
public:
	sun3x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scc1(*this, SCC1_TAG),
		m_scc2(*this, SCC2_TAG),
		m_esp(*this, "scsibus:7:esp"),
		m_fdc(*this, FDC_TAG),
		m_floppy_connector(*this, FLOPPY_CONN_TAG),
		m_p_ram(*this, "p_ram"),
		m_bw2_vram(*this, "bw2_vram")
	{ }

	void sun3_80(machine_config &config);
	void sun3_460(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	optional_device<ncr53c90_device> m_esp;
	optional_device<n82077aa_device> m_fdc;
	optional_device<floppy_connector> m_floppy_connector;
	virtual void machine_reset() override ATTR_COLD;

	required_shared_ptr<uint32_t> m_p_ram;
	optional_shared_ptr<uint32_t> m_bw2_vram;

	uint32_t enable_r();
	void enable_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t buserr_r();
	void buserr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t diag_r();
	void diag_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t printer_r();
	void printer_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t iommu_r(offs_t offset);
	void iommu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t irqctrl_r();
	void irqctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t memreg_r();
	void memreg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t memrerraddr_r();
	void memrerraddr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fdc_control_r();
	void fdc_control_w(uint32_t data);
	uint32_t cause_buserr_r();
	void cause_buserr_w(uint32_t data);
	void ramwrite_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t fpa_r();
	uint32_t p4id_r();

	TIMER_DEVICE_CALLBACK_MEMBER(sun380_timer);

	uint32_t bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void sun3_460_mem(address_map &map) ATTR_COLD;
	void sun3_80_mem(address_map &map) ATTR_COLD;

	uint32_t m_enable = 0, m_buserr = 0, m_diag = 0, m_printer = 0, m_irqctrl = 0, m_memreg = 0, m_memerraddr = 0;
	uint32_t m_iommu[0x800]{};
	bool m_bInBusErr = false;
};

void sun3x_state::sun3_80_mem(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram().share("p_ram").w(FUNC(sun3x_state::ramwrite_w));
	map(0x40000000, 0x40000003).rw(FUNC(sun3x_state::cause_buserr_r), FUNC(sun3x_state::cause_buserr_w));
	map(0x50300000, 0x50300003).r(FUNC(sun3x_state::p4id_r));
	map(0x50400000, 0x504fffff).ram().share(m_bw2_vram);
	map(0x60000000, 0x60001fff).rw(FUNC(sun3x_state::iommu_r), FUNC(sun3x_state::iommu_w));
	map(0x61000000, 0x61000003).rw(FUNC(sun3x_state::enable_r), FUNC(sun3x_state::enable_w));
	map(0x61000400, 0x61000403).rw(FUNC(sun3x_state::buserr_r), FUNC(sun3x_state::buserr_w));
	map(0x61000800, 0x61000803).rw(FUNC(sun3x_state::diag_r), FUNC(sun3x_state::diag_w));
	map(0x61001000, 0x61001003).rw(FUNC(sun3x_state::memreg_r), FUNC(sun3x_state::memreg_w));
	map(0x61001004, 0x61001007).rw(FUNC(sun3x_state::memrerraddr_r), FUNC(sun3x_state::memrerraddr_w));
	map(0x61001400, 0x61001403).rw(FUNC(sun3x_state::irqctrl_r), FUNC(sun3x_state::irqctrl_w));
	map(0x62000000, 0x6200000f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x62002000, 0x6200200f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x63000000, 0x6301ffff).rom().region("user1", 0);
	map(0x64000000, 0x640007ff).rw(TIMEKEEPER_TAG, FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0x66000000, 0x6600003f).m(m_esp, FUNC(ncr53c90_device::map)).umask32(0xff000000);
	map(0x6e000000, 0x6e000007).m(m_fdc, FUNC(n82077aa_device::map));
	map(0x6e000400, 0x6e000403).rw(FUNC(sun3x_state::fdc_control_r), FUNC(sun3x_state::fdc_control_w));
	map(0x6f00003c, 0x6f00003f).rw(FUNC(sun3x_state::printer_r), FUNC(sun3x_state::printer_w));
	map(0xfefe0000, 0xfefeffff).rom().region("user1", 0);
}

void sun3x_state::sun3_460_mem(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram().share("p_ram").w(FUNC(sun3x_state::ramwrite_w));
	map(0x09000000, 0x09000003).rw(FUNC(sun3x_state::cause_buserr_r), FUNC(sun3x_state::cause_buserr_w));
	map(0x50300000, 0x50300003).r(FUNC(sun3x_state::p4id_r));
	map(0x50400000, 0x504fffff).ram().share(m_bw2_vram);
	map(0x5c000f14, 0x5c000f17).r(FUNC(sun3x_state::fpa_r));
	map(0x60000000, 0x60001fff).rw(FUNC(sun3x_state::iommu_r), FUNC(sun3x_state::iommu_w));
	map(0x61000000, 0x61000003).rw(FUNC(sun3x_state::enable_r), FUNC(sun3x_state::enable_w));
	map(0x61000400, 0x61000403).rw(FUNC(sun3x_state::buserr_r), FUNC(sun3x_state::buserr_w));
	map(0x61000800, 0x61000803).rw(FUNC(sun3x_state::diag_r), FUNC(sun3x_state::diag_w));
	map(0x61001000, 0x61001003).rw(FUNC(sun3x_state::memreg_r), FUNC(sun3x_state::memreg_w));
	map(0x61001004, 0x61001007).rw(FUNC(sun3x_state::memrerraddr_r), FUNC(sun3x_state::memrerraddr_w));
	map(0x61001400, 0x61001403).rw(FUNC(sun3x_state::irqctrl_r), FUNC(sun3x_state::irqctrl_w));
	map(0x62000000, 0x6200000f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x62002000, 0x6200200f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x63000000, 0x6301ffff).rom().region("user1", 0);
	map(0x64002000, 0x64002011).rw("rtc", FUNC(icm7170_device::read), FUNC(icm7170_device::write));
	map(0x6f00003c, 0x6f00003f).rw(FUNC(sun3x_state::printer_r), FUNC(sun3x_state::printer_w));
	map(0xfefe0000, 0xfefeffff).rom().region("user1", 0);
}

uint32_t sun3x_state::p4id_r()
{
	return (1<<24); // 0 = hires bw2 1600x1280, 1 = bw2 1152x900, 0x45 is "Ibis" color, blt 0x68 is "Lego" color
}

void  sun3x_state::fdc_control_w(uint32_t data)
{
	logerror("FDC write %02x (%08x)\n", data >> 24, m_maincpu->pc());
}

uint32_t sun3x_state::fdc_control_r()
{
	// Type of floppy present
	// 0 = no floppy in drive
	// 1 = ed
	// 2 = hd
	// 3 = dd

	if (m_fdc)
	{
		floppy_image_device *fdev = m_floppy_connector->get_device();
		if(fdev->exists()) {
			uint32_t variant = fdev->get_variant();
			switch(variant) {
			case floppy_image::SSSD:
			case floppy_image::SSDD:
			case floppy_image::DSDD:
				return 3 << 24;

			case floppy_image::DSHD:
				return 2 << 24;

			case floppy_image::DSED:
				return 1 << 24;
			}
		}
	}

	return 0 << 24;
}

void sun3x_state::ramwrite_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t *pRAM = (uint32_t *)m_p_ram.target();

	if (((m_memreg & 0xf0000000) == 0x70000000) &&
		(m_irqctrl & 0x01000000) &&
		!(m_bInBusErr))
	{
		m_memerraddr = offset<<2;

		// low 4 bits of memreg are the byte lane(s) involved, negative logic
		m_memreg |= 0x0f;
		switch (mem_mask)
		{
			case 0xff000000:
				m_memreg &= ~0x08;
				break;

			case 0x00ff0000:
				m_memerraddr += 1;
				m_memreg &= ~0x04;
				break;

			case 0x0000ff00:
				m_memerraddr += 2;
				m_memreg &= ~0x02;
				break;

			case 0x000000ff:
				m_memerraddr += 3;
				m_memreg &= ~0x01;
				break;

			case 0x0000ffff:
				m_memerraddr += 2;
				m_memreg &= ~0x03;
				break;

			case 0xffff0000:
				m_memreg &= ~0x0c;
				break;

			case 0xffffffff:    // no address adjust, show all 4 lanes as problematic
				break;
		}

		m_bInBusErr = true; // prevent recursion
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}

	COMBINE_DATA(&pRAM[offset]);
}

uint32_t sun3x_state::enable_r()
{
	return m_enable;
}

void sun3x_state::enable_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to enable (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_enable);
}

uint32_t sun3x_state::buserr_r()
{
	uint32_t rv = m_buserr;
	m_buserr = 0;
	return rv;
}

void sun3x_state::buserr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to buserr (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_buserr);
}

uint32_t sun3x_state::diag_r()
{
	return m_diag;
}

void sun3x_state::diag_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to diag (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_diag);
}

uint32_t sun3x_state::printer_r()
{
	return m_printer;
}

void sun3x_state::printer_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to printer (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_printer);
}

uint32_t sun3x_state::irqctrl_r()
{
	return m_irqctrl;
}

void sun3x_state::irqctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to interrupt control (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_irqctrl);

	if (data & 0x01000000)
	{
		if (data & 0x02000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
		}
		if (data & 0x04000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
		}
		if (data & 0x08000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
		}
		if (!(data & 0x80000000))
		{
			m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
		}
	}
	else    // master enable clear, clear all interrupts
	{
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	}
}

uint32_t sun3x_state::memreg_r()
{
	return m_memreg;
}

void sun3x_state::memreg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to memory control (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_memreg);
}

uint32_t sun3x_state::memrerraddr_r()
{
	m_bInBusErr = false;
	m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	return m_memerraddr;
}

void sun3x_state::memrerraddr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("sun3x: %08x to memory error address (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_memerraddr);
}

uint32_t sun3x_state::iommu_r(offs_t offset)
{
	return m_iommu[offset];
}

// IOMMU entry defs:
// address mask:  0x03ffe000
// cache inhibit: 0x00000040
// full block:    0x00000020
// modified:      0x00000010
// used:          0x00000008
// write prot:    0x00000004
// bad:           0x00000002
// valid:         0x00000001
void sun3x_state::iommu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_iommu[offset]);
}

uint32_t sun3x_state::fpa_r()
{
	m_buserr |= 0x04000000;
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xffffffff;
}

uint32_t sun3x_state::cause_buserr_r()
{
	m_buserr |= 0x20000000;
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xffffffff;
}

void sun3x_state::cause_buserr_w(uint32_t data)
{
	m_buserr |= 0x20000000;
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(sun3x_state::sun380_timer)
{
	if ((m_irqctrl & 0x81000000) == 0x81000000)
	{
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE);
	}
}

uint32_t sun3x_state::bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const uint32_t palette[2] = { 0, 0xffffff };
	auto const vram = util::big_endian_cast<uint8_t const>(m_bw2_vram.target());

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1152/8; x++)
		{
			uint8_t const pixels = vram[(y * (1152 / 8)) + x];

			*scanline++ = palette[BIT(pixels, 7)];
			*scanline++ = palette[BIT(pixels, 6)];
			*scanline++ = palette[BIT(pixels, 5)];
			*scanline++ = palette[BIT(pixels, 4)];
			*scanline++ = palette[BIT(pixels, 3)];
			*scanline++ = palette[BIT(pixels, 2)];
			*scanline++ = palette[BIT(pixels, 1)];
			*scanline++ = palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

/* Input ports */
static INPUT_PORTS_START( sun3x )
INPUT_PORTS_END


void sun3x_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x10000);

	memset(m_iommu, 0, sizeof(m_iommu));

	m_enable = 0;
	m_buserr = 0;
	m_diag = 0;
	m_printer = 0;
	m_irqctrl = 0;
	m_memreg = 0;
	m_memerraddr = 0;
	m_bInBusErr = false;
}

static void sun_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

static void sun_cdrom(device_t *device)
{
	downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
	device.set_option_machine_config("cdrom", sun_cdrom);
}

void sun3x_state::sun3_80(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3x_state::sun3_80_mem);

	M48T02(config, TIMEKEEPER_TAG, 0);

	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	m_scc1->out_txda_callback().set(KEYBOARD_TAG, FUNC(sun_keyboard_port_device::write_txd));
	m_scc1->out_txdb_callback().set(MOUSE_TAG, FUNC(sun_mouse_port_device::write_txd));

	SUNKBD_PORT(config, KEYBOARD_TAG, default_sun_keyboard_devices, "type3hle").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));
	SUNMOUSE_PORT(config, MOUSE_TAG, default_sun_mouse_devices, "hle1200").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, "tape");
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("esp", NCR53C90).clock(20000000/2); // Emulex 2400138 (68-pin PLCC)

	N82077AA(config, m_fdc, 24000000, n82077aa_device::mode_t::PS2);
	FLOPPY_CONNECTOR(config, "fdc:0", sun_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);

	// the timekeeper has no interrupt output, so 3/80 includes a dedicated timer circuit
	TIMER(config, "timer").configure_periodic(FUNC(sun3x_state::sun380_timer), attotime::from_hz(100));

	screen_device &bwtwo(SCREEN(config, "bwtwo", SCREEN_TYPE_RASTER));
	bwtwo.set_screen_update(FUNC(sun3x_state::bw2_update));
	bwtwo.set_size(1152,900);
	bwtwo.set_visarea(0, 1152-1, 0, 900-1);
	bwtwo.set_refresh_hz(72);
}

void sun3x_state::sun3_460(machine_config &config)
{
	/* basic machine hardware */
	M68030(config, m_maincpu, 33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3x_state::sun3_460_mem);

	ICM7170(config, "rtc", 32768).irq().set_inputline(m_maincpu, M68K_IRQ_7);

	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));
}

/* ROM definition */

ROM_START( sun3_80 )
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/80 V1.0 Bootprom
Sun 3/80 V2.2 Bootprom
Sun 3/80 V2.3 Bootprom
Sun 3/80 V2.9.2 Bootprom
Sun 3/80 V3.0 Bootprom
Sun 3/80 V3.0.2 Bootprom
Sun 3/80 V3.0.3 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev303", "Rev 3.0.3")
	ROMX_LOAD( "sun3_80_v3.0.3", 0x0000, 0x20000, CRC(8f983115) SHA1(e4be2dcbb29fc5c60ed9d838ab241c634fdd24e5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev302", "Rev 3.0.2")
	ROMX_LOAD( "sun3_80_v3.0.2", 0x0000, 0x20000, CRC(c09a3592) SHA1(830187dfe58e65289533717a797d2c42da86ac4e), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_80_v3.0",   0x0000, 0x20000, CRC(47e3b012) SHA1(1e045b6f542aaf7808d6567c28a9e734a8c5d815), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "rev292", "Rev 2.9.2")
	ROMX_LOAD( "sun3_80_v2.9.2", 0x0000, 0x20000, CRC(32bcf711) SHA1(7ecd4a0d0988c1d1d53fd79ac16c8456ed73ace1), ROM_BIOS(3))

	// default NVRAM: includes valid settings for console on framebuffer, boot from SCSI disk, Ethernet ID, more
	ROM_REGION( 0x800, TIMEKEEPER_TAG, 0 )
	ROM_LOAD( "timekpr_380.bin", 0x000000, 0x000800, CRC(e76f1aae) SHA1(8e7c36e3928887a94a8133e8416ee4126c31edd7) )
ROM_END

ROM_START( sun3_460 )
	ROM_REGION32_BE( 0x20000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/460/480 V1.2.3 Bootprom
Sun 3/460/480 V2.9.1 Bootprom (2 Files, one for odd and one for even addresses)
Sun 3/460/480 V2.9.2 Bootprom
Sun 3/460/480 V2.9.3 Bootprom
Sun 3/460/480 V3.0 Bootprom (2 Files, one for odd and one for even addresses)
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "3_400_l.300", 0x00000, 0x10000, CRC(1312a04b) SHA1(6c3b67ba3567991897a48fe20f589ebbfcf0a35d), ROM_BIOS(0))
	ROMX_LOAD( "3_400_h.300", 0x10000, 0x10000, CRC(8d688672) SHA1(a5593844ce6af6c4f7f39bb653dc8f964b73b095), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev291", "Rev 2.9.1")
	ROMX_LOAD( "sun3_460_v2.9.1_0", 0x00000, 0x10000, CRC(d62dbf09) SHA1(4a6b5fd7840b44fe93c9058a8973d8dd3c9f7d24), ROM_BIOS(1))
	ROMX_LOAD( "sun3_460_v2.9.1_1", 0x10000, 0x10000, CRC(3b5a5942) SHA1(ed6250e3c07d7cb62d4dd517a8637c8d37e16dc5), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT        COMPANY             FULLNAME             FLAGS
COMP( 1989, sun3_80,  0,      0,      sun3_80,  sun3x, sun3x_state, empty_init, "Sun Microsystems", "Sun 3/80",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Hydra
COMP( 1989, sun3_460, 0,      0,      sun3_460, sun3x, sun3x_state, empty_init, "Sun Microsystems", "Sun 3/460/470/480", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Pegasus
