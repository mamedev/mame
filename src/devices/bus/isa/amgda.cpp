// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Advanced Monochrome Graphics Display Adapter (also known as All-Points-Addressable-8, or APA8).
 *
 * This adapter is an entry level, bitmap-addressable, monochrome graphics
 * adapter designed for use with the IBM RT PC. It is intended to be paired
 * with an IBM 6153 Advanced Monochrome Graphics Display, a 12", monochrome,
 * white phosphor CRT. The combination produces an interlaced 720x512 pixel
 * image with a 92Hz field rate and 46Hz frame rate.
 *
 * The adapter features 64KiB of video RAM, which can be accessed in several
 * modes applying a variety of masks, shifts and logic functions to data from
 * the host or read from video RAM.
 *
 * The 16 bit ISA data bus is connected byte-swapped to the adapter video RAM
 * and registers. When accessed by the host RT PC, the byte-swapping performed
 * by the IOCC makes these registers appear to be in host byte order (i.e.,
 * big-endian).
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume III (84X0873), Second Edition (September 1986), International Business Machines Corporation 1986.
 *
 * TODO:
 *  - fix failing interrupt diagnostic
 *  - vblank/hblank timing, interlace
 *  - interrupt clear
 *  - undefined functions
 *  - D3 content
 */
#include "emu.h"

#include "amgda.h"
#include "screen.h"

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_MODE (1U << 3)
#define LOG_VRAM (1U << 4)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW|LOG_MODE|LOG_VRAM)
#include "logmacro.h"


namespace {

class isa16_amgda_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_amgda_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA16_AMGDA, tag, owner, clock)
		, device_isa16_card_interface(mconfig, *this)
		, m_screen(*this, "screen")
{
}

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void pio_map(address_map &map);
	void mem_map(address_map &map);

	u16 vram_r(offs_t offset);
	void vram_w(offs_t offset, u16 data, u16 mem_mask);

	u16 ras_r();
	u16 btr_r();
	void dcr_w(u16 data);
	void dmr_w(u16 data);

	void irq_w(int state);

	u16 address_step(u16 offset) const;
	u8 alu(unsigned const byte) const;

private:
	required_device<screen_device> m_screen;

	std::unique_ptr<u8[]> m_vram;

	enum ras_mask : u8
	{
		RAS_EF  = 0x01, // even field
		RAS_HS  = 0x02, // horizontal sync
		RAS_VS  = 0x04, // vertical sync
		RAS_SV  = 0x08, // serialized video
		RAS_X   = 0x10, // x stepping
		RAS_IP  = 0x20, // interrupt pending
		RAS_DEC = 0x40, // decrement
		RAS_LD  = 0x80, // load address
	};
	u8 m_ras; // ras status register
	enum dcr_mask : u16
	{
		DCR_ROT  = 0x0007, // rotate count
		DCR_FUNC = 0x0038, // logic function unit control
		DCR_MODE = 0x0300, // memory mode
		DCR_DEC  = 0x0400, // decrement
		DCR_X    = 0x0800, // x stepping
		DCR_BLK  = 0x1000, // block transfer
		DCR_IE   = 0x2000, // interrupt enable
		DCR_SE   = 0x4000, // sync enable
		DCR_VE   = 0x8000, // video enable
	};
	enum dcr_mode : u16
	{
		MODE_SYSTEM  = 0x0000,
		MODE_OVERLAY = 0x0100,
		MODE_ADAPTER = 0x0200,
		MODE_AUTO    = 0x0300,
	};
	u16 m_dcr;  // data control register
	u8 m_dm[2]; // data mask registers
	u8 m_wm[2]; // write mask registers

	// internal state
	u8 m_sy[2]; // system data latches
	u8 m_d[3];  // on-card data latches
	u16 m_ba;   // block address
	bool m_aw;  // automatic write
};

void isa16_amgda_device::device_add_mconfig(machine_config &config)
{
	// monitor has 25.7MHz nominal bandwidth
	// OSC: 44.22MHz
	// H: 24.68KHz, retrace: 8.0us
	// V: 92Hz field, 46Hz frame
	// 720x512, 64Kx8 video ram
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(44'220'000 / 2, 800, 0, 720, 600, 0, 512);
	m_screen->set_screen_update(FUNC(isa16_amgda_device::screen_update));
	m_screen->screen_vblank().set(
		[this](int state)
		{
			if (state)
			{
				m_ras ^= RAS_EF;

				if (m_dcr & DCR_IE)
					irq_w(1);
			}
		});
}

void isa16_amgda_device::device_start()
{
	save_item(NAME(m_ras));
	save_item(NAME(m_dcr));
	save_item(NAME(m_dm));
	save_item(NAME(m_wm));

	save_item(NAME(m_sy));
	save_item(NAME(m_d));
	save_item(NAME(m_ba));
	save_item(NAME(m_aw));

	m_vram = std::make_unique<u8[]>(0x1'0000);
	save_pointer(NAME(m_vram), 0x1'0000);

	set_isa_device();

	m_isa->install_device(0x0160, 0x016f, *this, &isa16_amgda_device::pio_map);
	m_isa->install_memory(0xd0'0000, 0xd1'ffff, *this, &isa16_amgda_device::mem_map);

	// TODO: ISA bus shared interrupt 11 enable
	//m_isa->space(isa16_device::AS_ISA_IO).install_write_tap(0x06f3, 0x06f3, "irq_arm", [this](offs_t offset, u8 &data, u8 mem_mask) { irq_w(0); });
}

void isa16_amgda_device::device_reset()
{
	irq_w(0);

	m_ras = RAS_LD;
	m_dcr = 0;
	m_dm[0] = 0;
	m_dm[1] = 0;
	m_wm[0] = 0;
	m_wm[1] = 0;

	m_sy[0] = 0;
	m_sy[1] = 0;
	m_d[0] = 0;
	m_d[1] = 0;
	m_d[2] = 0;
	m_ba = 0;
	m_aw = false;
}

u32 isa16_amgda_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_dcr & DCR_VE)
	{
		u8 const *vram = m_vram.get();

		for (s32 y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (s32 x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 8)
			{
				u8 const data = *vram++;

				bitmap.pix(y, x + 0) = BIT(data, 7) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 1) = BIT(data, 6) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 2) = BIT(data, 5) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 3) = BIT(data, 4) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 4) = BIT(data, 3) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 5) = BIT(data, 2) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 6) = BIT(data, 1) ? rgb_t::white() : rgb_t::black();
				bitmap.pix(y, x + 7) = BIT(data, 0) ? rgb_t::white() : rgb_t::black();
			}

			vram += 38;
		}
	}
	else
		bitmap.fill(rgb_t::black());

	return 0;
}

void isa16_amgda_device::pio_map(address_map &map)
{
	map(0x0, 0x1).rw(FUNC(isa16_amgda_device::ras_r), FUNC(isa16_amgda_device::dcr_w)).flags(1);
	map(0x2, 0x3).rw(FUNC(isa16_amgda_device::btr_r), FUNC(isa16_amgda_device::dmr_w)).flags(1);
}

void isa16_amgda_device::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rw(FUNC(isa16_amgda_device::vram_r), FUNC(isa16_amgda_device::vram_w));
}

u16 isa16_amgda_device::vram_r(offs_t offset)
{
	// treat debugger read as x++ system read without side effects
	if (machine().side_effects_disabled())
		return u16(m_vram[offset + 1]) << 8 | m_vram[offset + 0];

	if (m_dcr & DCR_BLK)
	{
		if (m_ras & RAS_LD)
		{
			m_ba = offset;
			m_ras &= ~RAS_LD;
		}
		else
			offset = m_ba;
	}

	u16 address = offset;

	for (unsigned byte = 0; byte < 2; byte++)
	{
		// latch data
		if ((m_dcr & DCR_MODE) != MODE_ADAPTER)
			m_d[byte] = m_vram[address];

		address = address_step(address);
	}

	if (m_dcr & DCR_BLK)
		m_ba = address;

	m_aw = true;

	LOGMASKED(LOG_VRAM, "%s: vram_r 0x%04x data 0x%02x%02x\n", machine().describe_context(), offset, m_d[0], m_d[1]);

	// external data bus is byte-swapped
	return (u16(m_d[1]) << 8) | m_d[0];
}

void isa16_amgda_device::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	// external data bus is byte-swapped
	data = swapendian_int16(data);

	if (m_dcr & DCR_BLK)
	{
		if (m_ras & RAS_LD)
		{
			m_ba = offset;
			m_ras &= ~RAS_LD;
		}
		else
			offset = m_ba;
	}

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_VRAM, "%s: vram_w 0x%04x data 0x%04x\n", machine().describe_context(), offset, data);

	u16 address = offset;

	for (unsigned byte = 0; byte < 2; byte++)
	{
		switch (m_dcr & DCR_MODE)
		{
		case MODE_SYSTEM:
			m_sy[byte] = BIT(data, 8 - byte * 8, 8);
			break;
		case MODE_OVERLAY:
			m_wm[byte] = BIT(data, 8 - byte * 8, 8);
			break;
		}

		if ((m_dcr & DCR_MODE) != MODE_AUTO || m_aw)
			m_vram[address] = (m_vram[address] & m_wm[byte]) | (alu(byte) & ~m_wm[byte]);
		else
			m_d[byte] = m_vram[address];

		address = address_step(address);
	}

	if (m_dcr & DCR_BLK)
		m_ba = address;

	m_aw = !m_aw;
}

u16 isa16_amgda_device::ras_r()
{
	u16 data = m_ras;

	if (m_dcr & DCR_DEC)
		data |= RAS_DEC;

	if (m_dcr & DCR_X)
		data |= RAS_X;

	if (!machine().side_effects_disabled())
	{
		rectangle const &visible = m_screen->visible_area();
		int const hpos = m_screen->hpos();
		int const vpos = m_screen->vpos();

		if (visible.contains(hpos, vpos))
		{
			unsigned const bit = (vpos - visible.min_y) * 1024 + (hpos - visible.min_x);

			if ((m_dcr & DCR_VE) && BIT(m_vram[bit >> 3], 7 - (bit & 7)))
				data |= RAS_SV;
		}
		else if (vpos < visible.min_y || vpos > visible.max_y)
			data |= RAS_VS;
		else if (hpos < visible.min_x || hpos > visible.max_x)
			data |= RAS_HS;

		LOGMASKED(LOG_REGR, "%s: ras_r 0x%02x\n", machine().describe_context(), data);
	}

	// external data bus is byte-swapped
	return swapendian_int16(data);
}

u16 isa16_amgda_device::btr_r()
{
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGR, "%s: btr_r\n", machine().describe_context());

		m_ras |= RAS_LD;
	}

	return 0;
}

void isa16_amgda_device::dcr_w(u16 data)
{
	// external data bus is byte-swapped
	data = swapendian_int16(data);

	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_REGW, "%s: dcr_w 0x%04x\n", machine().describe_context(), data);

		static char const *const func[] = { "B", "1", "A", "3", "~B", "A|B", "~A", "~A|B" };
		static char const *const mode[] = { "system", "overlay", "adapter", "automatic" };

		LOGMASKED(LOG_MODE, "ve=%u, se=%u, ie=%u, blk=%u, %s%s, mode=%s, function=%s, rotate=%u\n",
			BIT(data, 15), BIT(data, 14), BIT(data, 13), BIT(data, 12),
			(data & DCR_X) ? "x" : "y", (data & DCR_DEC) ? "--" : "++",
			mode[BIT(data, 8, 2)], func[BIT(data, 3, 3)], (data & DCR_ROT));
	}

	if ((data ^ m_dcr) & DCR_BLK)
		m_ras |= RAS_LD;

	if (!(data & DCR_IE))
		irq_w(0);

	m_dcr = data;
}

void isa16_amgda_device::dmr_w(u16 data)
{
	// external data bus is byte-swapped
	data = swapendian_int16(data);

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGW, "%s: dmr_w 0x%04x\n", machine().describe_context(), data);

	m_dm[0] = BIT(data, 8, 8);
	m_dm[1] = BIT(data, 0, 8);
}

void isa16_amgda_device::irq_w(int state)
{
	if (state)
	{
		if (!(m_ras & RAS_IP))
		{
			m_ras |= RAS_IP;
			m_isa->irq11_w(state);
		}
	}
	else
	{
		if (m_ras & RAS_IP)
		{
			m_ras &= ~RAS_IP;
			m_isa->irq11_w(state);
		}
	}
}

u16 isa16_amgda_device::address_step(u16 offset) const
{
	switch (BIT(m_dcr, 10, 2))
	{
	case 0: return offset + 128; // y++
	case 1: return offset - 128; // y--
	case 2: return (offset & 0xff80) | ((offset + 1) & 0x7f); // x++
	case 3: return (offset & 0xff80) | ((offset - 1) & 0x7f); // x--
	}

	// can't happen
	abort();
}

u8 isa16_amgda_device::alu(unsigned const byte) const
{
	// apply data masks and shift
	unsigned const shift = m_dcr & DCR_ROT;
	u8 const a = m_dm[0] & m_sy[byte];
	u8 const b = m_dm[1] & ((m_d[byte + 0] << shift) | (m_d[byte + 1] >> (8 - shift)));

	switch (BIT(m_dcr, 3, 3))
	{
	case 0: return b;
	case 2: return a;
	case 4: return ~b;
	case 5: return a | b;
	case 6: return ~a;
	case 7: return ~(a | b);
	}

	// FIXME: undocumented functions 1 and 3
	return 0;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA16_AMGDA, device_isa16_card_interface, isa16_amgda_device, "isa16_amgda_device", "IBM Advanced Monochrome Graphics Display Adapter")
