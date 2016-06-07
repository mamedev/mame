// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

  sun3.c: preliminary driver for Sun 3x models.

  status: 3/80 POSTs, 3/460 needs its unique RTC chip (also used by non-3x Sun 3s).

  TODO:
    - Z8530 SCC needs to actually speak serial so we can hook up the mouse and keyboard.
    - Improve interrupt controller emulation.
    - Figure out how the IOMMU works.
    - Intersil 7170 device for 3/460 and 3/480 (they use the same PROMs).
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
#include "cpu/m68000/m68000.h"
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/scsicd.h"
#include "machine/ncr539x.h"
#include "machine/upd765.h"
#include "formats/pc_dsk.h"
#include "formats/mfi_dsk.h"
#include "bus/rs232/rs232.h"

#define TIMEKEEPER_TAG  "timekpr"
#define SCC1_TAG        "scc1"
#define SCC2_TAG        "scc2"
#define ESP_TAG         "esp"
#define FDC_TAG         "fdc"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"

class sun3x_state : public driver_device
{
public:
	sun3x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scc1(*this, SCC1_TAG),
		m_scc2(*this, SCC2_TAG),
		m_fdc(*this, FDC_TAG),
		m_p_ram(*this, "p_ram"),
		m_bw2_vram(*this, "bw2_vram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	optional_device<n82077aa_device> m_fdc;
	virtual void machine_reset() override;

	required_shared_ptr<UINT32> m_p_ram;
	optional_shared_ptr<UINT32> m_bw2_vram;

	DECLARE_READ32_MEMBER(enable_r);
	DECLARE_WRITE32_MEMBER(enable_w);
	DECLARE_READ32_MEMBER(buserr_r);
	DECLARE_WRITE32_MEMBER(buserr_w);
	DECLARE_READ32_MEMBER(diag_r);
	DECLARE_WRITE32_MEMBER(diag_w);
	DECLARE_READ32_MEMBER(printer_r);
	DECLARE_WRITE32_MEMBER(printer_w);
	DECLARE_READ32_MEMBER(iommu_r);
	DECLARE_WRITE32_MEMBER(iommu_w);
	DECLARE_READ32_MEMBER(irqctrl_r);
	DECLARE_WRITE32_MEMBER(irqctrl_w);
	DECLARE_READ32_MEMBER(memreg_r);
	DECLARE_WRITE32_MEMBER(memreg_w);
	DECLARE_READ32_MEMBER(memrerraddr_r);
	DECLARE_WRITE32_MEMBER(memrerraddr_w);
	DECLARE_READ32_MEMBER(fdc_control_r);
	DECLARE_WRITE32_MEMBER(fdc_control_w);
	DECLARE_READ32_MEMBER(cause_buserr_r);
	DECLARE_WRITE32_MEMBER(cause_buserr_w);
	DECLARE_WRITE32_MEMBER(ramwrite_w);
	DECLARE_READ32_MEMBER(fpa_r);
	DECLARE_READ32_MEMBER(p4id_r);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	TIMER_DEVICE_CALLBACK_MEMBER(sun380_timer);

	UINT32 bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	UINT32 m_enable, m_buserr, m_diag, m_printer, m_irqctrl, m_memreg, m_memerraddr;
	UINT32 m_iommu[0x800];
	bool m_bInBusErr;
};

static ADDRESS_MAP_START(sun3_80_mem, AS_PROGRAM, 32, sun3x_state)
	AM_RANGE(0x00000000, 0x03ffffff) AM_RAM AM_SHARE("p_ram") AM_WRITE(ramwrite_w)
	AM_RANGE(0x40000000, 0x40000003) AM_READWRITE(cause_buserr_r, cause_buserr_w)
	AM_RANGE(0x50300000, 0x50300003) AM_READ(p4id_r)
	AM_RANGE(0x50400000, 0x504fffff) AM_RAM AM_SHARE("bw2_vram")
	AM_RANGE(0x60000000, 0x60001fff) AM_READWRITE(iommu_r, iommu_w)
	AM_RANGE(0x61000000, 0x61000003) AM_READWRITE(enable_r, enable_w)
	AM_RANGE(0x61000400, 0x61000403) AM_READWRITE(buserr_r, buserr_w)
	AM_RANGE(0x61000800, 0x61000803) AM_READWRITE(diag_r, diag_w)
	AM_RANGE(0x61001000, 0x61001003) AM_READWRITE(memreg_r, memreg_w)
	AM_RANGE(0x61001004, 0x61001007) AM_READWRITE(memrerraddr_r, memrerraddr_w)
	AM_RANGE(0x61001400, 0x61001403) AM_READWRITE(irqctrl_r, irqctrl_w)
	AM_RANGE(0x62000000, 0x6200000f) AM_DEVREADWRITE8(SCC1_TAG, z80scc_device, ba_cd_inv_r, ba_cd_inv_w, 0xff00ff00)
	AM_RANGE(0x62002000, 0x6200200f) AM_DEVREADWRITE8(SCC2_TAG, z80scc_device, ba_cd_inv_r, ba_cd_inv_w, 0xff00ff00)
	AM_RANGE(0x63000000, 0x6301ffff) AM_ROM AM_REGION("user1",0)
	AM_RANGE(0x64000000, 0x640007ff) AM_DEVREADWRITE8(TIMEKEEPER_TAG, timekeeper_device, read, write, 0xffffffff)
	AM_RANGE(0x66000000, 0x6600003f) AM_DEVREADWRITE8(ESP_TAG, ncr539x_device, read, write, 0xff000000)
	AM_RANGE(0x6e000000, 0x6e000007) AM_DEVICE8(FDC_TAG, n82077aa_device, map, 0xffffffff)
	AM_RANGE(0x6e000400, 0x6e000403) AM_READWRITE(fdc_control_r, fdc_control_w)
	AM_RANGE(0x6f00003c, 0x6f00003f) AM_READWRITE(printer_r, printer_w)
	AM_RANGE(0xfefe0000, 0xfefeffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(sun3_460_mem, AS_PROGRAM, 32, sun3x_state)
	AM_RANGE(0x00000000, 0x03ffffff) AM_RAM AM_SHARE("p_ram") AM_WRITE(ramwrite_w)
	AM_RANGE(0x09000000, 0x09000003) AM_READWRITE(cause_buserr_r, cause_buserr_w)
	AM_RANGE(0x50300000, 0x50300003) AM_READ(p4id_r)
	AM_RANGE(0x50400000, 0x504fffff) AM_RAM AM_SHARE("bw2_vram")
	AM_RANGE(0x5c000f14, 0x5c000f17) AM_READ(fpa_r)
	AM_RANGE(0x60000000, 0x60001fff) AM_READWRITE(iommu_r, iommu_w)
	AM_RANGE(0x61000000, 0x61000003) AM_READWRITE(enable_r, enable_w)
	AM_RANGE(0x61000400, 0x61000403) AM_READWRITE(buserr_r, buserr_w)
	AM_RANGE(0x61000800, 0x61000803) AM_READWRITE(diag_r, diag_w)
	AM_RANGE(0x61001000, 0x61001003) AM_READWRITE(memreg_r, memreg_w)
	AM_RANGE(0x61001004, 0x61001007) AM_READWRITE(memrerraddr_r, memrerraddr_w)
	AM_RANGE(0x61001400, 0x61001403) AM_READWRITE(irqctrl_r, irqctrl_w)
	AM_RANGE(0x62000000, 0x6200000f) AM_DEVREADWRITE8(SCC1_TAG, z80scc_device, ba_cd_inv_r, ba_cd_inv_w, 0xff00ff00)
	AM_RANGE(0x62002000, 0x6200200f) AM_DEVREADWRITE8(SCC2_TAG, z80scc_device, ba_cd_inv_r, ba_cd_inv_w, 0xff00ff00)
	AM_RANGE(0x63000000, 0x6301ffff) AM_ROM AM_REGION("user1",0)

	AM_RANGE(0x6f00003c, 0x6f00003f) AM_READWRITE(printer_r, printer_w)
	AM_RANGE(0xfefe0000, 0xfefeffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

READ32_MEMBER( sun3x_state::p4id_r )
{
	return (1<<24); // 0 = hires bw2 1600x1280, 1 = bw2 1152x900, 0x45 is "Ibis" color, blt 0x68 is "Lego" color
}

WRITE32_MEMBER( sun3x_state::fdc_control_w )
{
	logerror("FDC write %02x (%08x)\n", data >> 24, space.device().safe_pc());
}

READ32_MEMBER( sun3x_state::fdc_control_r )
{
	// Type of floppy present
	// 0 = no floppy in drive
	// 1 = ed
	// 2 = hd
	// 3 = dd

	if(m_fdc) {
		floppy_image_device *fdev = machine().device<floppy_connector>(":fdc:0")->get_device();
		if(fdev->exists()) {
			UINT32 variant = fdev->get_variant();
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

WRITE32_MEMBER(sun3x_state::ramwrite_w)
{
	UINT32 *pRAM = (UINT32 *)m_p_ram.target();

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
		m_maincpu->set_input_line_and_vector(M68K_IRQ_7, ASSERT_LINE, 2);
	}

	COMBINE_DATA(&pRAM[offset]);
}

READ32_MEMBER(sun3x_state::enable_r)
{
	return m_enable;
}

WRITE32_MEMBER(sun3x_state::enable_w)
{
//  printf("sun3x: %08x to enable (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_enable);
}

READ32_MEMBER(sun3x_state::buserr_r)
{
	UINT32 rv = m_buserr;
	m_buserr = 0;
	return rv;
}

WRITE32_MEMBER(sun3x_state::buserr_w)
{
//  printf("sun3x: %08x to buserr (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_buserr);
}

READ32_MEMBER(sun3x_state::diag_r)
{
	return m_diag;
}

WRITE32_MEMBER(sun3x_state::diag_w)
{
//  printf("sun3x: %08x to diag (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_diag);
}

READ32_MEMBER(sun3x_state::printer_r)
{
	return m_printer;
}

WRITE32_MEMBER(sun3x_state::printer_w)
{
//  printf("sun3x: %08x to printer (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_printer);
}

READ32_MEMBER(sun3x_state::irqctrl_r)
{
	return m_irqctrl;
}

WRITE32_MEMBER(sun3x_state::irqctrl_w)
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

READ32_MEMBER(sun3x_state::memreg_r)
{
	return m_memreg;
}

WRITE32_MEMBER(sun3x_state::memreg_w)
{
//  printf("sun3x: %08x to memory control (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_memreg);
}

READ32_MEMBER(sun3x_state::memrerraddr_r)
{
	m_bInBusErr = false;
	m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	return m_memerraddr;
}

WRITE32_MEMBER(sun3x_state::memrerraddr_w)
{
//  printf("sun3x: %08x to memory error address (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_memerraddr);
}

READ32_MEMBER(sun3x_state::iommu_r)
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
WRITE32_MEMBER(sun3x_state::iommu_w)
{
	COMBINE_DATA(&m_iommu[offset]);
}

READ32_MEMBER(sun3x_state::fpa_r)
{
	m_buserr |= 0x04000000;
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xffffffff;
}

READ32_MEMBER(sun3x_state::cause_buserr_r)
{
	m_buserr |= 0x20000000;
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0xffffffff;
}

WRITE32_MEMBER(sun3x_state::cause_buserr_w)
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

UINT32 sun3x_state::bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels;
	static const UINT32 palette[2] = { 0, 0xffffff };
	UINT8 *m_vram = (UINT8 *)m_bw2_vram.target();

	for (y = 0; y < 900; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1152/8; x++)
		{
			pixels = m_vram[(y * (1152/8)) + (BYTE4_XOR_BE(x))];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
	}

	return 0;
}

/* Input ports */
static INPUT_PORTS_START( sun3x )
INPUT_PORTS_END


void sun3x_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();

	memcpy((UINT8*)m_p_ram.target(),user1,0x10000);

	m_maincpu->reset();

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

FLOPPY_FORMATS_MEMBER( sun3x_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( sun_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( sun3_80, sun3x_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 20000000)
	MCFG_CPU_PROGRAM_MAP(sun3_80_mem)

	MCFG_M48T02_ADD(TIMEKEEPER_TAG)

	MCFG_SCC8530_ADD(SCC1_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)
	MCFG_SCC8530_ADD(SCC2_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD(RS232A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, ctsb_w))

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_6)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_5)

	MCFG_DEVICE_ADD(ESP_TAG, NCR539X, 20000000/2)
	MCFG_LEGACY_SCSI_PORT("scsi")

	MCFG_N82077AA_ADD("fdc", n82077aa_device::MODE_PS2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", sun_floppies, "35hd", sun3x_state::floppy_formats)

	// the timekeeper has no interrupt output, so 3/80 includes a dedicated timer circuit
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer", sun3x_state, sun380_timer, attotime::from_hz(100))

	MCFG_SCREEN_ADD("bwtwo", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(sun3x_state, bw2_update)
	MCFG_SCREEN_SIZE(1152,900)
	MCFG_SCREEN_VISIBLE_AREA(0, 1152-1, 0, 900-1)
	MCFG_SCREEN_REFRESH_RATE(72)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sun3_460, sun3x_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68030, 33000000)
	MCFG_CPU_PROGRAM_MAP(sun3_460_mem)

	MCFG_M48T02_ADD(TIMEKEEPER_TAG)

	MCFG_SCC8530_ADD(SCC1_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)
	MCFG_SCC8530_ADD(SCC2_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD(RS232A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(SCC2_TAG, z80scc_device, ctsb_w))
MACHINE_CONFIG_END

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
	ROMX_LOAD( "sun3_80_v3.0.3", 0x0000, 0x20000, CRC(8f983115) SHA1(e4be2dcbb29fc5c60ed9d838ab241c634fdd24e5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev302", "Rev 3.0.2")
	ROMX_LOAD( "sun3_80_v3.0.2", 0x0000, 0x20000, CRC(c09a3592) SHA1(830187dfe58e65289533717a797d2c42da86ac4e), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_80_v3.0",   0x0000, 0x20000, CRC(47e3b012) SHA1(1e045b6f542aaf7808d6567c28a9e734a8c5d815), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "rev292", "Rev 2.9.2")
	ROMX_LOAD( "sun3_80_v2.9.2", 0x0000, 0x20000, CRC(32bcf711) SHA1(7ecd4a0d0988c1d1d53fd79ac16c8456ed73ace1), ROM_BIOS(4))

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
	ROMX_LOAD( "3_400_l.300", 0x00000, 0x10000, CRC(1312a04b) SHA1(6c3b67ba3567991897a48fe20f589ebbfcf0a35d), ROM_BIOS(1))
	ROMX_LOAD( "3_400_h.300", 0x10000, 0x10000, CRC(8d688672) SHA1(a5593844ce6af6c4f7f39bb653dc8f964b73b095), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "rev291", "Rev 2.9.1")
	ROMX_LOAD( "sun3_460_v2.9.1_0", 0x00000, 0x10000, CRC(d62dbf09) SHA1(4a6b5fd7840b44fe93c9058a8973d8dd3c9f7d24), ROM_BIOS(2))
	ROMX_LOAD( "sun3_460_v2.9.1_1", 0x10000, 0x10000, CRC(3b5a5942) SHA1(ed6250e3c07d7cb62d4dd517a8637c8d37e16dc5), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY         FULLNAME       FLAGS */
COMP( 198?, sun3_80,   0,       0,   sun3_80,   sun3x, driver_device,     0,  "Sun Microsystems", "Sun 3/80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // Hydra
COMP( 198?, sun3_460,  0,       0,   sun3_460,  sun3x, driver_device,     0,  "Sun Microsystems", "Sun 3/460/470/480", MACHINE_NOT_WORKING | MACHINE_NO_SOUND) // Pegasus
