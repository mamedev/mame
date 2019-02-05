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
#include "cpu/mips/mips1.h"
#include "cpu/mips/mips3.h"
#include "machine/timer.h"
#include "machine/decioga.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "machine/dec_lk201.h"
#include "machine/am79c90.h"
#include "machine/dc7085.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "video/bt459.h"
#include "video/decsfb.h"

class decstation_state : public driver_device
{
public:
	decstation_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_scantimer(*this, "scantimer"),
		m_sfb(*this, "sfb"),
		m_lk201(*this, "lk201"),
		m_ioga(*this, "ioga"),
		m_rtc(*this, "rtc"),
		m_scc0(*this, "scc0"),
		m_scc1(*this, "scc1"),
		m_asc(*this, "scsibus:7:asc"),
		m_vrom(*this, "gfx"),
		m_bt459(*this, "bt459"),
		m_lance(*this, "am79c90"),
		m_kn01vram(*this, "vram"),
		m_dz(*this, "dc7085")
		{ }

	void kn01(machine_config &config);
	void kn02ba(machine_config &config);

	void init_decstation();

protected:
	DECLARE_READ_LINE_MEMBER(brcond0_r) { return ASSERT_LINE; }
	DECLARE_WRITE_LINE_MEMBER(ioga_irq_w);
	DECLARE_WRITE_LINE_MEMBER(dz_irq_w);

	DECLARE_READ32_MEMBER(cfb_r);
	DECLARE_WRITE32_MEMBER(cfb_w);

	DECLARE_READ32_MEMBER(kn01_status_r);
	DECLARE_WRITE32_MEMBER(kn01_control_w);
	DECLARE_READ32_MEMBER(bt478_palette_r);
	DECLARE_WRITE32_MEMBER(bt478_palette_w);
	DECLARE_READ32_MEMBER(pcc_r);
	DECLARE_WRITE32_MEMBER(pcc_w);
	DECLARE_WRITE32_MEMBER(planemask_w);
	DECLARE_WRITE32_MEMBER(vram_w);

	DECLARE_READ32_MEMBER(dz_r);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);

	void ncr5394(device_t *device);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t kn01_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mips1core_device_base> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<timer_device> m_scantimer;
	optional_device<decsfb_device> m_sfb;
	optional_device<lk201_device> m_lk201;
	optional_device<dec_ioga_device> m_ioga;
	required_device<mc146818_device> m_rtc;
	optional_device<z80scc_device> m_scc0, m_scc1;
	optional_device<ncr53c94_device> m_asc;
	optional_memory_region m_vrom;
	optional_device<bt459_device> m_bt459;
	required_device<am79c90_device> m_lance;
	optional_shared_ptr<uint32_t> m_kn01vram;
	optional_device<dc7085_device> m_dz;

	void kn01_map(address_map &map);
	void threemin_map(address_map &map);

	u8 *m_vrom_ptr;

	u32 m_kn01_control, m_kn01_status;
	u32 m_palette[256], m_overlay[256];
	u8 m_r, m_g, m_b, m_entry, m_stage;
	u32 m_planemask;

	u16 m_pcc_regs[0x40/4];
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

void decstation_state::video_start()
{
}

uint32_t decstation_state::kn01_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	uint8_t *vram = (uint8_t *)m_kn01vram.target();

	for (y = 0; y < 864; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024; x++)
		{
			pixels = vram[(y * 1024) + x];
			*scanline++ = m_palette[pixels];
		}
	}

	return 0;
}

uint32_t decstation_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bt459->screen_update(screen, bitmap, cliprect, (uint8_t *)m_sfb->get_vram());
	return 0;
}

READ32_MEMBER(decstation_state::cfb_r)
{
	uint32_t addr = offset << 2;

	//logerror("cfb_r: reading at %x\n", addr);

	if (addr < 0x80000)
	{
		return m_vrom_ptr[addr>>2] & 0xff;
	}

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
	}

	return 0xffffffff;
}

WRITE32_MEMBER(decstation_state::cfb_w)
{
	uint32_t addr = offset << 2;

	if ((addr >= 0x100000) && (addr < 0x100200))
	{
		return;
	}

	if ((addr >= 0x1c0000) && (addr < 0x200000))
	{
		//printf("Bt459: %08x (mask %08x) @ %x\n", data, mem_mask, offset<<2);
		return;
	}

	if ((addr >= 0x200000) && (addr < 0x400000))
	{
	}
}

enum
{
	PCC_CMDR = 0,
	PCC_XPOS,   // 04
	PCC_YPOS,   // 08
	PCC_XMIN1,  // 0c
	PCC_XMAX1,  // 10
	PCC_YMIN1,  // 14
	PCC_YMAX1,  // 18
	PCC_UNK1,   // 1c
	PCC_UNK2,   // 20
	PCC_UNK3,   // 24
	PCC_UNK4,   // 28
	PCC_XMIN2,  // 2c
	PCC_XMAX2,  // 30
	PCC_YMIN2,  // 34
	PCC_YMAX2,  // 38
	PCC_MEMORY  // 3c
};

READ32_MEMBER(decstation_state::pcc_r)
{
	return m_pcc_regs[offset];
}

WRITE32_MEMBER(decstation_state::pcc_w)
{
	m_pcc_regs[offset] = data & 0xffff;
}

WRITE32_MEMBER(decstation_state::planemask_w)
{
	// value written is smeared across all 4 byte lanes
	data &= 0xff;
	m_planemask = (data) || (data << 8) || (data << 16) || (data << 24);
}

WRITE32_MEMBER(decstation_state::vram_w)
{
	u32 *vram = (u32 *)m_kn01vram.target();
//  u32 effective_planemask = (m_planemask & mem_mask);
//  vram[offset] = (vram[offset] & ~effective_planemask) | (data & effective_planemask);
	COMBINE_DATA(&vram[offset]);
}

TIMER_DEVICE_CALLBACK_MEMBER(decstation_state::scanline_timer)
{
	int scanline = m_screen->vpos();

	if ((scanline == m_pcc_regs[PCC_YMIN2]) && (m_pcc_regs[PCC_CMDR] & 0x0400))
	{
		m_kn01_status |= 0x200;
	}

	if ((scanline == m_pcc_regs[PCC_YMIN1]) && (m_pcc_regs[PCC_CMDR] & 0x0100))
	{
		int x, y;
		u8 *vram = (u8 *)m_kn01vram.target();
		u32 rgba, r, g, b;

		x = m_pcc_regs[PCC_XMIN1] - 212;
		y = m_pcc_regs[PCC_YMIN1] - 34;
		//printf("sampling for VRGTRB and friends at X=%d Y=%d\n", x, y);
		m_kn01_status &= ~7;
		if ((x >= 0) && (x <= 1023) && (y >= 0) && (y <= 863))
		{
			rgba = m_palette[vram[(y * 1024) + x]];
			r = (rgba >> 16) & 0xff;
			g = (rgba >> 8) & 0xff;
			b = (rgba & 0xff);

			//printf("R=%d, G=%d, B=%d\n", r, g, b);
			if (r > b) m_kn01_status |= 1;
			if (r > g) m_kn01_status |= 2;
			if (b > g) m_kn01_status |= 4;
		}
	}
}

READ32_MEMBER(decstation_state::bt478_palette_r)
{
	u8 rv = 0;

	if (offset == 1)
	{
		switch (m_stage)
		{
			case 0:
				m_stage++;
				rv = (m_palette[m_entry] >> 16) & 0xff;
				break;

			case 1:
				m_stage++;
				rv = (m_palette[m_entry] >> 8) & 0xff;
				break;

			case 2:
				rv = m_palette[m_entry] & 0xff;
				m_entry++;
				m_entry &= 0xff;
				m_stage = 0;
		}
	}
	else if (offset == 5)
	{
		switch (m_stage)
		{
			case 0:
				m_stage++;
				rv = (m_overlay[m_entry] >> 16) & 0xff;
				break;

			case 1:
				m_stage++;
				rv = (m_overlay[m_entry] >> 8) & 0xff;
				break;

			case 2:
				rv = m_overlay[m_entry] & 0xff;
				m_entry++;
				m_entry &= 0xff;
				m_stage = 0;
		}
	}

	return rv;
}

WRITE32_MEMBER(decstation_state::bt478_palette_w)
{
	//printf("VDAC_w: %08x at %08x (mask %08x)\n", data, offset, mem_mask);

	if ((offset == 0) || (offset == 3) || (offset == 4) || (offset == 7))
	{
		m_entry = data & 0xff;
		//printf("VDAC: entry %d\n", m_entry);
		m_stage = 0;
		m_r = m_g = m_b = 0;
	}
	else if (offset == 1)
	{
		switch (m_stage)
		{
			case 0:
				m_r = data & 0xff;
				m_stage++;
				break;

			case 1:
				m_g = data & 0xff;
				m_stage++;
				break;

			case 2:
				m_b = data & 0xff;
				m_palette[m_entry] = rgb_t(m_r, m_g, m_b);
				//printf("palette[%d] = RGB(%02x, %02x, %02x)\n", m_entry, m_r, m_g, m_b);
				m_entry++;
				m_entry &= 0xff;
				m_stage = 0;
				m_r = m_g = m_b = 0;
				break;
		}
	}
	else if (offset == 5)
	{
		switch (m_stage)
		{
			case 0:
				m_r = data & 0xff;
				m_stage++;
				break;

			case 1:
				m_g = data & 0xff;
				m_stage++;
				break;

			case 2:
				m_b = data & 0xff;
				m_overlay[m_entry] = rgb_t(m_r, m_g, m_b);
				//printf("overlay[%d] = RGB(%02x, %02x, %02x)\n", m_entry, m_r, m_g, m_b);
				m_entry++;
				m_entry &= 0xff;
				m_stage = 0;
				m_r = m_g = m_b = 0;
				break;
		}
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

WRITE_LINE_MEMBER(decstation_state::dz_irq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, state);
}

void decstation_state::machine_start()
{
	if (m_vrom)
		m_vrom_ptr = m_vrom->base();
}

void decstation_state::machine_reset()
{
	if (m_ioga)
	{
		m_ioga->set_dma_space(&m_maincpu->space(AS_PROGRAM));
	}

	m_entry = 0;
	m_stage = 0;
	m_r = m_g = m_b = 0;
	m_kn01_status = 0;
}

READ32_MEMBER(decstation_state::kn01_status_r)
{
	//m_kn01_status ^= 0x200; // fake vint for now
	return m_kn01_status;
}

WRITE32_MEMBER(decstation_state::kn01_control_w)
{
	COMBINE_DATA(&m_kn01_control);

	// clear VINT
	if ((m_kn01_control & 0x200) && (m_kn01_status & 0x200))
	{
		m_kn01_status &= ~0x200;
	}
}

READ32_MEMBER(decstation_state::dz_r)
{
	return 0x8000;
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void decstation_state::kn01_map(address_map &map)
{
	map(0x00000000, 0x017fffff).ram();
	map(0x0fc00000, 0x0fcfffff).ram().share("vram").w(FUNC(decstation_state::vram_w));
	map(0x10000000, 0x10000003).w(FUNC(decstation_state::planemask_w));
	map(0x11000000, 0x1100003f).rw(FUNC(decstation_state::pcc_r), FUNC(decstation_state::pcc_w));
	map(0x12000000, 0x1200001f).rw(FUNC(decstation_state::bt478_palette_r), FUNC(decstation_state::bt478_palette_w));
	//map(0x18000000, 0x18000007).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0x0000ffff);
	map(0x1c000000, 0x1c00001b).m(m_dz, FUNC(dc7085_device::map)).umask32(0xffff);
	map(0x1d000000, 0x1d0000ff).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct)).umask32(0x000000ff);
	map(0x1e000000, 0x1effffff).rw(FUNC(decstation_state::kn01_status_r), FUNC(decstation_state::kn01_control_w));
	map(0x1fc00000, 0x1fc3ffff).rom().region("user1", 0);
}

void decstation_state::threemin_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram();  // full 128 MB
	map(0x10000000, 0x1007ffff).rw(FUNC(decstation_state::cfb_r), FUNC(decstation_state::cfb_w));
	map(0x10100000, 0x101001ff).rw(m_sfb, FUNC(decsfb_device::read), FUNC(decsfb_device::write));
	map(0x101c0000, 0x101c000f).m("bt459", FUNC(bt459_device::map)).umask32(0x000000ff);
	map(0x10200000, 0x103fffff).rw(m_sfb, FUNC(decsfb_device::vram_r), FUNC(decsfb_device::vram_w));
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

MACHINE_CONFIG_START(decstation_state::kn01)
	R2000(config, m_maincpu, 16.67_MHz_XTAL, 65536, 131072);
	m_maincpu->set_endianness(ENDIANNESS_LITTLE);
	m_maincpu->set_fpurev(0x340);
	m_maincpu->in_brcond<0>().set(FUNC(decstation_state::brcond0_r));
	m_maincpu->set_addrmap(AS_PROGRAM, &decstation_state::kn01_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(69169800, 1280, 0, 1024, 901, 0, 864);
	m_screen->set_screen_update(FUNC(decstation_state::kn01_screen_update));

	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(decstation_state::scanline_timer), "screen", 0, 1);

	DC7085(config, m_dz, 0);
	m_dz->int_cb().set(FUNC(decstation_state::dz_irq_w));
	m_dz->ch1_tx_cb().set("dc7085:ch1", FUNC(dc7085_channel::rx_w));

	AM79C90(config, m_lance, XTAL(12'500'000));

	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->set_binary(true);
MACHINE_CONFIG_END

MACHINE_CONFIG_START(decstation_state::kn02ba)
	R3000A(config, m_maincpu, 33.333_MHz_XTAL, 65536, 131072);
	m_maincpu->set_endianness(ENDIANNESS_LITTLE);
	m_maincpu->set_fpurev(0x340); // should be R3010A v4.0
	m_maincpu->in_brcond<0>().set(FUNC(decstation_state::brcond0_r));
	m_maincpu->set_addrmap(AS_PROGRAM, &decstation_state::threemin_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(130000000, 1704, 32, (1280+32), 1064, 3, (1024+3));
	m_screen->set_screen_update(FUNC(decstation_state::screen_update));

	DECSFB(config, m_sfb, 25'000'000);  // clock based on white paper which quotes "40ns" gate array cycle times
//  m_sfb->int_cb().set(FUNC(dec_ioga_device::slot0_irq_w));

	BT459(config, m_bt459, 83'020'800);

	AM79C90(config, m_lance, XTAL(12'500'000));
	m_lance->intr_out().set("ioga", FUNC(dec_ioga_device::lance_irq_w));
	m_lance->dma_in().set("ioga", FUNC(dec_ioga_device::lance_dma_r));
	m_lance->dma_out().set("ioga", FUNC(dec_ioga_device::lance_dma_w));

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
	m_scc1->out_txdb_callback().set(m_lk201, FUNC(lk201_device::rx_w));

	LK201(config, m_lk201, 0);
	m_lk201->tx_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc0, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc0, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc0, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc0, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc0, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc0, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", dec_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:1", dec_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:2", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:5", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", dec_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:7", dec_scsi_devices, "asc", true).set_option_machine_config("asc", [this] (device_t *device) { ncr5394(device); });
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

ROM_START( ds3100 )
	ROM_REGION32_LE( 0x40000, "user1", 0 )
	ROM_LOAD( "kn01-aa.v7.01.img", 0x000000, 0x040000, CRC(e2478aa7) SHA1(e789387c52df3e0d83fde97cb48314627ea90b93) )
ROM_END

ROM_START( ds5k133 )
	ROM_REGION32_LE( 0x40000, "user1", 0 )
	// 5.7j                                                                                                                                                                                                                                 sx
	ROM_LOAD( "ds5000-133_005eb.bin", 0x000000, 0x040000, CRC(76a91d29) SHA1(140fcdb4fd2327daf764a35006d05fabfbee8da6) )

	ROM_REGION32_LE( 0x20000, "gfx", 0 )
	ROM_LOAD( "pmagb-ba-rom.img", 0x000000, 0x020000, CRC(91f40ab0) SHA1(a39ce6ed52697a513f0fb2300a1a6cf9e2eabe33) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                 FULLNAME                FLAGS
COMP( 1989, ds3100,  0,      0,      kn01,   decstation, decstation_state, init_decstation, "Digital Equipment Corporation", "DECstation 3100", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1992, ds5k133, 0,      0,      kn02ba, decstation, decstation_state, init_decstation, "Digital Equipment Corporation", "DECstation 5000/133", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
