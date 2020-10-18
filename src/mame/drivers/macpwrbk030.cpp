// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/macpwrbk030.cpp
    Mac PowerBooks with a 68030 CPU and M50753 PMU
    By R. Belmont

    These are basically late-period Mac IIs without NuBus and with
    Egret/Cuda replaced with the PMU.

****************************************************************************/

#include "emu.h"

#include "machine/macrtc.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/applefdc.h"
#include "machine/ram.h"
#include "machine/sonydriv.h"
#include "machine/swim.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/macadb.h"
#include "machine/ncr5380.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "sound/asc.h"
#include "formats/ap_dsk35.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define C7M (7833600)
#define C15M (C7M*2)
#define C32M (C15M*2)

class macpb030_state : public driver_device
{
public:
	macpb030_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via1"),
		m_via2(*this, "via2"),
		m_macadb(*this, "macadb"),
		m_ncr5380(*this, "ncr5380"),
		m_ram(*this, RAM_TAG),
		m_iwm(*this, "fdc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_asc(*this, "asc"),
		m_scc(*this, "scc"),
		m_vram(*this, "vram")
	{
	}

	void macpb140(machine_config &config);
	void macpb145(machine_config &config);
	void macpb160(machine_config &config);
	void macpb170(machine_config &config);
	void macpb180(machine_config &config);
	void macpb180c(machine_config &config);
	void macpd210(machine_config &config);
	void macpb140_map(address_map &map);
	void macpb160_map(address_map &map);
	void macpb165c_map(address_map &map);
	void macpd210_map(address_map &map);

	void init_macpb140();
	void init_macpb160();

private:
	required_device<m68030_device> m_maincpu;
	required_device<via6522_device> m_via1;
	required_device<via6522_device> m_via2;
	required_device<macadb_device> m_macadb;
	required_device<ncr5380_device> m_ncr5380;
	required_device<ram_device> m_ram;
	required_device<applefdc_base_device> m_iwm;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<asc_device> m_asc;
	required_device<z80scc_device> m_scc;
	required_shared_ptr<u32> m_vram;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	u32 screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_macpbwd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u32 *m_ram_ptr, *m_rom_ptr;
	u32 m_ram_mask, m_ram_size, m_rom_size;

	emu_timer *m_6015_timer;

	WRITE_LINE_MEMBER(adb_irq_w) { m_adb_irq_pending = state; }
	int m_adb_irq_pending;

	u16 mac_via_r(offs_t offset);
	void mac_via_w(offs_t offset, u16 data, u16 mem_mask);
	u16 mac_via2_r(offs_t offset);
	void mac_via2_w(offs_t offset, u16 data, u16 mem_mask);
	u8 mac_via_in_a();
	u8 mac_via_in_b();
	void mac_via_out_a(u8 data);
	void mac_via_out_b(u8 data);
	u8 mac_via2_in_a();
	u8 mac_via2_in_b();
	void mac_via2_out_a(u8 data);
	void mac_via2_out_b(u8 data);
	void field_interrupts();
	DECLARE_WRITE_LINE_MEMBER(via_irq_w);
	DECLARE_WRITE_LINE_MEMBER(via2_irq_w);
	TIMER_CALLBACK_MEMBER(mac_6015_tick);
	WRITE_LINE_MEMBER(via_cb2_w) { m_macadb->adb_data_w(state); }
	int m_via_cycles, m_via_interrupt, m_via2_interrupt, m_scc_interrupt, m_asc_interrupt, m_last_taken_interrupt;
	int m_irq_count, m_ca1_data, m_ca2_data;

	u32 rom_switch_r(offs_t offset);
	bool m_overlay;

	u16 scsi_r(offs_t offset, u16 mem_mask);
	void scsi_w(offs_t offset, u16 data, u16 mem_mask);
	uint32_t scsi_drq_r(offs_t offset, uint32_t mem_mask = ~0);
	void scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	u16 mac_scc_r(offs_t offset)
	{
		u16 result = m_scc->dc_ab_r(offset);
		return (result << 8) | result;
	}
	void mac_scc_2_w(offs_t offset, u16 data) { m_scc->dc_ab_w(offset, data >> 8); }

	u16 mac_iwm_r(offs_t offset, u16 mem_mask)
	{
		u16 result = m_iwm->read(offset >> 8);
		return (result << 8) | result;
	}
	void mac_iwm_w(offs_t offset, u16 data, u16 mem_mask)
	{
		if (ACCESSING_BITS_0_7)
			m_iwm->write((offset >> 8), data & 0xff);
		else
			m_iwm->write((offset >> 8), data>>8);
	}

	u32 buserror_r();

	DECLARE_WRITE_LINE_MEMBER(asc_irq_w)
	{
		m_asc_interrupt = state;
		field_interrupts();
	}

	// ID for PowerBook Duo 210
	u32 pd210_id_r() { return 0xa55a1004; }

	uint8_t mac_gsc_r(offs_t offset);
	void mac_gsc_w(uint8_t data);
	void macgsc_palette(palette_device &palette) const;

	uint32_t macwd_r(offs_t offset, uint32_t mem_mask = ~0);
	void macwd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	u32 m_colors[3], m_count, m_clutoffs, m_wd_palette[256];
};

// 4-level grayscale
void macpb030_state::macgsc_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xff, 0xff, 0xff);
	palette.set_pen_color(1, 0x7f, 0x7f, 0x7f);
	palette.set_pen_color(2, 0x3f, 0x3f, 0x3f);
	palette.set_pen_color(3, 0x00, 0x00, 0x00);
}

u32 macpb030_state::buserror_r()
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

void macpb030_state::field_interrupts()
{
	int take_interrupt = -1;

	if (m_scc_interrupt)
	{
		take_interrupt = 4;
	}
	else if (m_via2_interrupt)
	{
		take_interrupt = 2;
	}
	else if (m_via_interrupt)
	{
		take_interrupt = 1;
	}

	if (m_last_taken_interrupt > -1)
	{
		m_maincpu->set_input_line(m_last_taken_interrupt, CLEAR_LINE);
		m_last_taken_interrupt = -1;
	}

	if (take_interrupt > -1)
	{
		m_maincpu->set_input_line(take_interrupt, ASSERT_LINE);
		m_last_taken_interrupt = take_interrupt;
	}
}

void macpb030_state::machine_start()
{
	m_ram_ptr = (u32*)m_ram->pointer();
	m_ram_size = m_ram->size()>>1;
	m_ram_mask = m_ram_size - 1;
	m_rom_ptr = (u32*)memregion("bootrom")->base();
	m_rom_size = memregion("bootrom")->bytes();
	m_via_cycles = -50;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = m_asc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_irq_count = m_ca1_data = m_ca2_data = 0;

	m_6015_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(macpb030_state::mac_6015_tick),this));
	m_6015_timer->adjust(attotime::never);
}

void macpb030_state::machine_reset()
{
	m_overlay = true;
	m_via_interrupt = m_via2_interrupt = m_scc_interrupt = m_asc_interrupt = 0;
	m_last_taken_interrupt = -1;
	m_irq_count = m_ca1_data = m_ca2_data = 0;

	// put ROM mirror at 0
	address_space& space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_read_bank(0x00000000, memory_end & ~memory_mirror, memory_mirror, "bank1");
	membank("bank1")->set_base(m_rom_ptr);

	// start 60.15 Hz timer
	m_6015_timer->adjust(attotime::from_hz(60.15), 0, attotime::from_hz(60.15));
}

void macpb030_state::init_macpb140()
{
}

void macpb030_state::init_macpb160()
{
}

u32 macpb030_state::screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 const *const video_ram = (const uint16_t *)m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u16 *const line = &bitmap.pix(y);

		for (int x = 0; x < 640; x += 16)
		{
			uint16_t const word = video_ram[((y * 640) / 16) + ((x / 16) ^ 1)];
			for (int b = 0; b < 16; b++)
			{
				line[x + b] = (word >> (15 - b)) & 0x0001;
			}
		}
	}
	return 0;
}

u32 macpb030_state::screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const *const vram8 = (uint8_t *)m_vram.target();

	for (int y = 0; y < 400; y++)
	{
		u16 *line = &bitmap.pix(y);

		for (int x = 0; x < 640 / 4; x++)
		{
			uint8_t const pixels = vram8[(y * 160) + (BYTE4_XOR_BE(x))];

			*line++ = ((pixels >> 6) & 3);
			*line++ = ((pixels >> 4) & 3);
			*line++ = ((pixels >> 2) & 3);
			*line++ = (pixels & 3);
		}
	}
	return 0;
}

u32 macpb030_state::screen_update_macpbwd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) /* Color PowerBooks using an off-the-shelf WD video chipset */
{
	u8 const *vram8 = (uint8_t *)m_vram.target();

	//    vram8 += 0x40000;

	for (int y = 0; y < 480; y++)
	{
		u32 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 640; x++)
		{
			uint8_t const pixels = vram8[(y * 640) + (BYTE4_XOR_BE(x))];
			*scanline++ = m_wd_palette[pixels];
		}
	}

	return 0;
}

u16 macpb030_state::mac_via_r(offs_t offset)
{
	u16 data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via1->read(offset);

	m_maincpu->adjust_icount(m_via_cycles);

	return (data & 0xff) | (data << 8);
}

void macpb030_state::mac_via_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via1->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via1->write(offset, (data >> 8) & 0xff);

	m_maincpu->adjust_icount(m_via_cycles);
}

uint16_t macpb030_state::mac_via2_r(offs_t offset)
{
	int data;

	offset >>= 8;
	offset &= 0x0f;

	data = m_via2->read(offset);
	return (data & 0xff) | (data << 8);
}

void macpb030_state::mac_via2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset >>= 8;
	offset &= 0x0f;

	if (ACCESSING_BITS_0_7)
		m_via2->write(offset, data & 0xff);
	if (ACCESSING_BITS_8_15)
		m_via2->write(offset, (data >> 8) & 0xff);
}

WRITE_LINE_MEMBER(macpb030_state::via_irq_w)
{
	m_via_interrupt = state;
	field_interrupts();
}

WRITE_LINE_MEMBER(macpb030_state::via2_irq_w)
{
	m_via2_interrupt = state;
	field_interrupts();
}

u32 macpb030_state::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay)
	{
		address_space& space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram->size() - 1;
		void *memory_data = m_ram->pointer();
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_readwrite_bank(0x00000000, memory_end & ~memory_mirror, memory_mirror, "bank1");
		membank("bank1")->set_base(memory_data);
		m_overlay = false;
	}

	//printf("rom_switch_r: offset %08x ROM_size -1 = %08x, masked = %08x\n", offset, m_rom_size-1, offset & ((m_rom_size - 1)>>2));

	return m_rom_ptr[offset & ((m_rom_size - 1)>>2)];
}

TIMER_CALLBACK_MEMBER(macpb030_state::mac_6015_tick)
{
	/* signal VBlank on CA1 input on the VIA */
	m_ca1_data ^= 1;
	m_via1->write_ca1(m_ca1_data);

	if (++m_irq_count == 60)
	{
		m_irq_count = 0;

		m_ca2_data ^= 1;
		/* signal 1 Hz irq on CA2 input on the VIA */
		m_via1->write_ca2(m_ca2_data);
	}
}

u16 macpb030_state::scsi_r(offs_t offset, u16 mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_r: offset %x mask %x\n", offset, mem_mask);

	if ((reg == 6) && (offset == 0x130))
	{
		reg = R5380_CURDATA_DTACK;
	}

	return m_ncr5380->ncr5380_read_reg(reg) << 8;
}

void macpb030_state::scsi_w(offs_t offset, u16 data, u16 mem_mask)
{
	int reg = (offset >> 3) & 0xf;

	//  logerror("macplus_scsi_w: data %x offset %x mask %x\n", data, offset, mem_mask);

	if ((reg == 0) && (offset == 0x100))
	{
		reg = R5380_OUTDATA_DTACK;
	}

	m_ncr5380->ncr5380_write_reg(reg, data);
}

uint32_t macpb030_state::scsi_drq_r(offs_t offset, uint32_t mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			return m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24;

		case 0xffff0000:
			return (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<16);

		case 0xffffffff:
			return (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<24) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<16) | (m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK)<<8) | m_ncr5380->ncr5380_read_reg(R5380_CURDATA_DTACK);

		default:
			logerror("scsi_drq_r: unknown mem_mask %08x\n", mem_mask);
	}

	return 0;
}

void macpb030_state::scsi_drq_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (mem_mask)
	{
		case 0xff000000:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			break;

		case 0xffff0000:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>16);
			break;

		case 0xffffffff:
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>24);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>16);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data>>8);
			m_ncr5380->ncr5380_write_reg(R5380_OUTDATA_DTACK, data&0xff);
			break;

		default:
			logerror("scsi_drq_w: unknown mem_mask %08x\n", mem_mask);
			break;
	}
}

uint8_t macpb030_state::mac_gsc_r(offs_t offset)
{
	if (offset == 1)
	{
		return 5;
	}

	return 0;
}

void macpb030_state::mac_gsc_w(uint8_t data)
{
}

uint32_t macpb030_state::macwd_r(offs_t offset, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0xf6:
		if (m_screen->vblank())
		{
			return 0xffffffff;
		}
		else
		{
			return 0;
		}

	default:
		//            printf("macwd_r: @ %x, mask %08x (PC=%x)\n", offset, mem_mask, m_maincpu->pc());
		break;
	}
	return 0;
}

void macpb030_state::macwd_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset)
	{
	case 0xf2:
		if (mem_mask == 0xff000000) // DAC control
		{
			m_clutoffs = data >> 24;
			m_count = 0;
		}
		else if (mem_mask == 0x00ff0000) // DAC data
		{
			m_colors[m_count++] = (data >> 16) & 0xff;
			if (m_count == 3)
			{
				//                    printf("RAMDAC: color %d = %02x %02x %02x\n", m_rbv_clutoffs, m_rbv_colors[0], m_rbv_colors[1], m_rbv_colors[2]);
				m_wd_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
				m_clutoffs++;
				m_count = 0;
			}
		}
		else
		{
			printf("macwd: Unknown DAC write, data %08x, mask %08x\n", data, mem_mask);
		}
		break;

	default:
		//            printf("macwd_w: %x @ %x, mask %08x (PC=%x)\n", data, offset, mem_mask, m_maincpu->pc());
		break;
	}
}

/***************************************************************************
    ADDRESS MAPS
****************************************************************************/

// ROM detects the "Jaws" ASIC by checking for I/O space mirrored at 0x01000000 boundries
void macpb030_state::macpb140_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50000000, 0x50001fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w)).mirror(0x01f00000);
	map(0x50002000, 0x50003fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w)).mirror(0x01f00000);
	map(0x50004000, 0x50005fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w)).mirror(0x01f00000);
	map(0x50006000, 0x50007fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w)).mirror(0x01f00000);
	map(0x50010000, 0x50011fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w)).mirror(0x01f00000);
	map(0x50012060, 0x50012063).r(FUNC(macpb030_state::scsi_drq_r)).mirror(0x01f00000);
	map(0x50014000, 0x50015fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write)).mirror(0x01f00000);
	map(0x50016000, 0x50017fff).rw(FUNC(macpb030_state::mac_iwm_r), FUNC(macpb030_state::mac_iwm_w)).mirror(0x01f00000);
	map(0x50024000, 0x50027fff).r(FUNC(macpb030_state::buserror_r)).mirror(0x01f00000); // bus error here to make sure we aren't mistaken for another decoder

	map(0xfee08000, 0xfeffffff).ram().share("vram");
}

void macpb030_state::macpb160_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::mac_iwm_r), FUNC(macpb030_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(macpb030_state::mac_gsc_r), FUNC(macpb030_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

void macpb030_state::macpb165c_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::mac_iwm_r), FUNC(macpb030_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to detect we're not the grayscale 160/165/180
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	// on-board color video on 165c/180c
	map(0xfc000000, 0xfc07ffff).ram().share("vram").mirror(0x00380000); // 512k of VRAM
	map(0xfc400000, 0xfcefffff).rw(FUNC(macpb030_state::macwd_r), FUNC(macpb030_state::macwd_w));
	// fc4003c8 = DAC control, fc4003c9 = DAC data
	// fc4003da bit 3 is VBL
	map(0xfcff8000, 0xfcffffff).rom().region("vrom", 0x0000);
}

void macpb030_state::macpd210_map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(macpb030_state::rom_switch_r)).mirror(0x0ff00000);

	map(0x50f00000, 0x50f01fff).rw(FUNC(macpb030_state::mac_via_r), FUNC(macpb030_state::mac_via_w));
	map(0x50f02000, 0x50f03fff).rw(FUNC(macpb030_state::mac_via2_r), FUNC(macpb030_state::mac_via2_w));
	map(0x50f04000, 0x50f05fff).rw(FUNC(macpb030_state::mac_scc_r), FUNC(macpb030_state::mac_scc_2_w));
	map(0x50f06000, 0x50f07fff).rw(FUNC(macpb030_state::scsi_drq_r), FUNC(macpb030_state::scsi_drq_w));
	map(0x50f10000, 0x50f11fff).rw(FUNC(macpb030_state::scsi_r), FUNC(macpb030_state::scsi_w));
	map(0x50f12060, 0x50f12063).r(FUNC(macpb030_state::scsi_drq_r));
	map(0x50f14000, 0x50f15fff).rw(m_asc, FUNC(asc_device::read), FUNC(asc_device::write));
	map(0x50f16000, 0x50f17fff).rw(FUNC(macpb030_state::mac_iwm_r), FUNC(macpb030_state::mac_iwm_w));
	map(0x50f20000, 0x50f21fff).rw(FUNC(macpb030_state::mac_gsc_r), FUNC(macpb030_state::mac_gsc_w));
	map(0x50f24000, 0x50f27fff).r(FUNC(macpb030_state::buserror_r)); // bus error here to make sure we aren't mistaken for another decoder

	map(0x5ffffffc, 0x5fffffff).r(FUNC(macpb030_state::pd210_id_r));

	map(0x60000000, 0x6001ffff).ram().share("vram").mirror(0x0ffe0000);
}

u8 macpb030_state::mac_via_in_a()
{
	return 0x81 | 0x12;
}

u8 macpb030_state::mac_via_in_b()
{
	int val = 0;
	// TODO: is this valid for VIA2 PMU machines?
	/* video beam in display (! VBLANK && ! HBLANK basically) */

	if (m_screen->vpos() >= 480)
		val |= 0x40;

	//  printf("%s VIA1 IN_B = %02x\n", machine().describe_context().c_str(), val);

	return val;
}

void macpb030_state::mac_via_out_a(u8 data)
{
	sony_set_sel_line(m_iwm.target(), (data & 0x20) >> 5);
}

void macpb030_state::mac_via_out_b(u8 data)
{
}

u8 macpb030_state::mac_via2_in_a()
{
	return m_macadb->get_pm_data_recv();
}

u8 macpb030_state::mac_via2_in_b()
{
	if (m_macadb->get_pm_ack() == 2)
	{
		return 0xcf;
	}
	else
	{
		return 0xcd;
	}
}

void macpb030_state::mac_via2_out_a(u8 data)
{
	m_macadb->set_pm_data_send(data);
}

void macpb030_state::mac_via2_out_b(u8 data)
{
	m_macadb->pmu_req_w((data >> 2) & 1);
}

/***************************************************************************
    DEVICE CONFIG
***************************************************************************/

static const applefdc_interface mac_iwm_interface =
{
	sony_set_lines,
	sony_set_enable_lines,

	sony_read_data,
	sony_write_data,
	sony_read_status
};

static const floppy_interface mac_floppy_interface =
{
	FLOPPY_STANDARD_3_5_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(apple35_mac),
	"floppy_3_5"
};

static INPUT_PORTS_START( macadb )
INPUT_PORTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void macpb030_state::macpb140(machine_config &config)
{
	M68030(config, m_maincpu, C15M);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb140_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_macpb140));

	PALETTE(config, m_palette, palette_device::MONOCHROME_INVERTED);

	MACADB(config, m_macadb, C15M);
	m_macadb->set_pmu_mode(true);
	m_macadb->set_pmu_is_via1(false);

	LEGACY_IWM(config, m_iwm, &mac_iwm_interface);
	sonydriv_floppy_image_device::legacy_2_drives_add(config, &mac_floppy_interface);

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));

	NCR5380(config, m_ncr5380, C7M);
	m_ncr5380->set_scsi_port("scsi");

	SCC85C30(config, m_scc, C7M);
//  m_scc->intrq_callback().set(FUNC(macpb030_state::set_scc_interrupt));

	VIA6522(config, m_via1, C7M/10);
	m_via1->readpa_handler().set(FUNC(macpb030_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macpb030_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macpb030_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macpb030_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macpb030_state::via_irq_w));
	m_via1->cb2_handler().set(FUNC(macpb030_state::via_cb2_w));

	VIA6522(config, m_via2, C7M/10);
	m_via2->readpa_handler().set(FUNC(macpb030_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macpb030_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macpb030_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macpb030_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macpb030_state::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(FUNC(macpb030_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
}

// PowerBook 145 = 140 @ 25 MHz (still 2MB RAM - the 145B upped that to 4MB)
void macpb030_state::macpb145(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

// PowerBook 170 = 140 @ 25 MHz with an active-matrix LCD (140/145/145B were passive)
void macpb030_state::macpb170(machine_config &config)
{
	macpb140(config);
	m_maincpu->set_clock(25000000);

	m_ram->set_default_size("4M");
	m_ram->set_extra_options("6M,8M");
}

void macpb030_state::macpb160(machine_config &config)
{
	M68030(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb160_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60.15);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1260));
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_size(700, 480);
	m_screen->set_visarea(0, 639, 0, 399);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_macpb160));

	PALETTE(config, m_palette, FUNC(macpb030_state::macgsc_palette), 16);

	MACADB(config, m_macadb, C15M);
	m_macadb->set_pmu_mode(true);
	m_macadb->set_pmu_is_via1(false);

	LEGACY_IWM(config, m_iwm, &mac_iwm_interface);
	sonydriv_floppy_image_device::legacy_2_drives_add(config, &mac_floppy_interface);

	scsi_port_device &scsibus(SCSI_PORT(config, "scsi"));
	scsibus.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));
	scsibus.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));

	NCR5380(config, m_ncr5380, C7M);
	m_ncr5380->set_scsi_port("scsi");

	SCC85C30(config, m_scc, C7M);
	//  m_scc->intrq_callback().set(FUNC(macpb030_state::set_scc_interrupt));

	VIA6522(config, m_via1, C7M / 10);
	m_via1->readpa_handler().set(FUNC(macpb030_state::mac_via_in_a));
	m_via1->readpb_handler().set(FUNC(macpb030_state::mac_via_in_b));
	m_via1->writepa_handler().set(FUNC(macpb030_state::mac_via_out_a));
	m_via1->writepb_handler().set(FUNC(macpb030_state::mac_via_out_b));
	m_via1->irq_handler().set(FUNC(macpb030_state::via_irq_w));
	m_via1->cb2_handler().set(FUNC(macpb030_state::via_cb2_w));

	VIA6522(config, m_via2, C7M / 10);
	m_via2->readpa_handler().set(FUNC(macpb030_state::mac_via2_in_a));
	m_via2->readpb_handler().set(FUNC(macpb030_state::mac_via2_in_b));
	m_via2->writepa_handler().set(FUNC(macpb030_state::mac_via2_out_a));
	m_via2->writepb_handler().set(FUNC(macpb030_state::mac_via2_out_b));
	m_via2->irq_handler().set(FUNC(macpb030_state::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	ASC(config, m_asc, C15M, asc_device::asc_type::ASC);
	m_asc->irqf_callback().set(FUNC(macpb030_state::asc_irq_w));
	m_asc->add_route(0, "lspeaker", 1.0);
	m_asc->add_route(1, "rspeaker", 1.0);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("2M");
	m_ram->set_extra_options("4M,6M,8M");

	SOFTWARE_LIST(config, "flop35_list").set_original("mac_flop");
}

void macpb030_state::macpb180(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
}

void macpb030_state::macpb180c(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_clock(33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpb165c_map);

	m_screen->set_size(800, 525);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(macpb030_state::screen_update_macpbwd));
	m_screen->set_no_palette();
}

void macpb030_state::macpd210(machine_config &config)
{
	macpb160(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &macpb030_state::macpd210_map);

	m_ram->set_extra_options("8M,12M,16M,20M,24M");
}

ROM_START(macpb140)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb145)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb145b)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb170)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("420dbff3.rom", 0x000000, 0x100000, CRC(88ea2081) SHA1(7a8ee468d16e64f2ad10cb8d1a45e6f07cc9e212))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv2.bin", 0x000000, 0x001800, CRC(1a32b5e5) SHA1(7c096324763cfc8d2024893b3e8493b7729b3a92))
ROM_END

ROM_START(macpb160)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpb180)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpb180c)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("e33b2724.rom", 0x000000, 0x100000, CRC(536c60f4) SHA1(c0510682ae6d973652d7e17f3c3b27629c47afac))

	ROM_REGION32_BE(0x8000, "vrom", 0)
	ROM_LOAD("pb180cvrom.bin", 0x0000, 0x8000, CRC(810c75ad) SHA1(3a936e97dee5ceeb25e50197ef504e514ae689a4))

	ROM_REGION(0x1800, "pmu", 0)
	ROM_LOAD("pmuv3.bin", 0x000000, 0x001800, CRC(f2df696c) SHA1(fc312cbfd407c6f0248c6463910e41ad6b5b0daa))
ROM_END

ROM_START(macpd210)
	ROM_REGION32_BE(0x100000, "bootrom", 0)
	ROM_LOAD("ecfa989b.rom", 0x000000, 0x100000, CRC(b86ed854) SHA1(ed1371c97117a5884da4a6605ecfc5abed48ae5a))
ROM_END

COMP(1991, macpb140, 0, 0,         macpb140, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 140", MACHINE_NOT_WORKING)
COMP(1991, macpb170, macpb140, 0,  macpb170, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 170", MACHINE_NOT_WORKING)
COMP(1992, macpb145, macpb140, 0,  macpb145, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 145", MACHINE_NOT_WORKING)
COMP(1992, macpb145b, macpb140, 0, macpb170, macadb, macpb030_state, init_macpb140, "Apple Computer", "Macintosh PowerBook 145B", MACHINE_NOT_WORKING)
COMP(1992, macpb160, 0, 0,         macpb160, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 160", MACHINE_NOT_WORKING)
COMP(1992, macpb180, macpb160, 0,  macpb180, macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 180", MACHINE_NOT_WORKING)
COMP(1992, macpb180c, macpb160, 0, macpb180c,macadb, macpb030_state, init_macpb160, "Apple Computer", "Macintosh PowerBook 180c", MACHINE_NOT_WORKING)

// PowerBook Duos (may or may not belong in this driver ultimately)
COMP( 1992, macpd210,  0,        0,      macpd210, macadb,  macpb030_state, init_macpb160,      "Apple Computer", "Macintosh PowerBook Duo 210", MACHINE_NOT_WORKING )
