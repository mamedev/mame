// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Sigma Designs fixed-resolution monochrome video card
  1664x1200 or 832x600 according to the ad, and exist as mode entries
  in the declaration ROM.

  VRAM at Fs000000, mirrored at Fs900000.
  Fs0BFFEC: write 0x04 to enable VBL, 0x01 to ack VBL

  Card has one ASIC, a Sigma Designs 53C280A.  This is shared with the PC ISA
  "LaserView Plus" card.

  Crystals:
  160.00 MHz
  122.925 MHz
  99.108 MHz
  16.0 MHz

  CRTC parameters are always programmed for 1664x1200, the card apparently
  doubles up the output for the 832x600 mode.

  TODO: Find what makes it sense the higher resolution mode.

***************************************************************************/

#include "emu.h"

#include "laserview.h"

#include "emupal.h"
#include "screen.h"

#include <algorithm>

static constexpr u32 VRAM_SIZE = 0x40000;   // ROM tests for 512K, but card has 8x uPD41264 (64K x 4 bit) = 256 KiB
namespace {

	class nubus_laserview_device : public device_t,
								   public device_nubus_card_interface
	{
	public:
		// construction/destruction
		nubus_laserview_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	private:
		u8 regs_r(offs_t offset);
		void regs_w(offs_t offset, u8 data);

		u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

		void vbl_w(int state);

		required_device<screen_device> m_screen;

		std::unique_ptr<u32[]> m_vram;
		u16 m_htotal, m_hvis, m_vtotal, m_vvis;
		u32 m_vbl_disable;
		u8 m_prot_latch;
	};

ROM_START( laserview )
	ROM_REGION(0x8000, "declrom", 0)
	ROM_LOAD( "lva-m2-00020_v3.00.bin", 0x000000, 0x008000, CRC(569d1fb7) SHA1(fd505505226abb5fea7c10ed14e8841077ef1be6) )
ROM_END

void nubus_laserview_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(nubus_laserview_device::screen_update));
	m_screen->set_raw(99108000/2, 1008, 0, 832, 622, 0, 600);
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(FUNC(nubus_laserview_device::vbl_w));

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_laserview_device::device_rom_region() const
{
	return ROM_NAME( laserview );
}

nubus_laserview_device::nubus_laserview_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NUBUS_LASERVIEW, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_screen(*this, "screen"),
	m_htotal(0), m_hvis(0), m_vtotal(0), m_vvis(0),
	m_vbl_disable(1), m_prot_latch(0)
{
}

void nubus_laserview_device::device_start()
{
	const u32 slotspace = get_slotspace();

	install_declaration_rom("declrom", true);

	m_vram = std::make_unique<u32[]>(VRAM_SIZE / sizeof(u32));
	install_bank(slotspace, slotspace + VRAM_SIZE - 1, &m_vram[0]);
	install_bank(slotspace+0x900000, slotspace+0x900000+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_device(slotspace+0xB0000, slotspace+0xBFFFF, emu::rw_delegate(*this, FUNC(nubus_laserview_device::regs_r)), emu::rw_delegate(*this, FUNC(nubus_laserview_device::regs_w)));

	save_item(NAME(m_htotal));
	save_item(NAME(m_hvis));
	save_item(NAME(m_vtotal));
	save_item(NAME(m_vvis));
	save_item(NAME(m_vbl_disable));
	save_item(NAME(m_prot_latch));
	save_pointer(NAME(m_vram), VRAM_SIZE / sizeof(u32));
}

void nubus_laserview_device::device_reset()
{
	m_vbl_disable = 1;
	std::fill_n(&m_vram[0], VRAM_SIZE / sizeof(u32), 0);
}

u32 nubus_laserview_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	auto const vram8 = util::big_endian_cast<u8 const>(&m_vram[0]);
	for (int y = 0; y < 600; y++)
	{
		u16 *scanline = &bitmap.pix(y);
		for (int x = 0; x < 832/8; x++)
		{
			u8 const pixels = vram8[(y * 104) + x + 0x20];

			*scanline++ = BIT(pixels, 7);
			*scanline++ = BIT(pixels, 6);
			*scanline++ = BIT(pixels, 5);
			*scanline++ = BIT(pixels, 4);
			*scanline++ = BIT(pixels, 3);
			*scanline++ = BIT(pixels, 2);
			*scanline++ = BIT(pixels, 1);
			*scanline++ = BIT(pixels, 0);
		}
	}
	return 0;
}

void nubus_laserview_device::vbl_w(int state)
{
	if ((state) && (!m_vbl_disable))
	{
		raise_slot_irq();
	}
}

void nubus_laserview_device::regs_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x00f9:
			m_prot_latch = ~data;
			break;

		case 0x0493:
			m_vvis = (m_vvis & 0xff) | (data << 8);
			break;

		case 0x0687:
			m_hvis = (m_hvis & 0xff) | (data << 8);
			break;

		case 0x078f:
			m_vtotal = (m_vtotal & 0xff) | (data << 8);
			break;

		case 0x0883:
			m_htotal = (m_htotal & 0xff) | (data<<8);
			break;

		case 0x08ff:
			if (data == 0xc1)
			{
				const rectangle visarea(0, (m_hvis / 2) - 1, 0, (m_vvis / 2) - 1);
				m_screen->configure(m_htotal / 2, m_vtotal / 2, visarea, attotime::from_ticks((m_htotal / 2) * (m_vtotal / 2), 160000000 / 2).as_attoseconds());
			}
			break;

		case 0x20f9:
			m_prot_latch = ~data ^ 0xff;
			break;

		case 0xb095:
			m_vvis = (m_vvis & 0xff00) | data;
			break;

		case 0xdd91:
			m_vtotal = (m_vtotal & 0xff00) | data;
			break;

		case 0x8085:
			m_htotal = (m_htotal & 0xff00) | data;
			break;

		case 0x8089:
			m_hvis = (m_hvis & 0xff00) | data;
			break;

		case 0xffef:
			if ((data & 0xff) == 0x04)
			{
				m_vbl_disable = 0;
			}
			else if ((data & 0xff) == 1)
			{
				lower_slot_irq();
			}
			else
			{
				m_vbl_disable = 1;
				lower_slot_irq();
			}
			break;
	}

}

u8 nubus_laserview_device::regs_r(offs_t offset)
{
	switch (offset)
	{
		case 0xff04:        // vblank status
			return m_screen->vblank();

		case 0xff08:
			return m_prot_latch;

		case 0xfffc:
			return 0xe4;
	}

	return 0xff;
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_LASERVIEW, device_nubus_card_interface, nubus_laserview_device, "nb_laserview", "Sigma Designs LaserView video card")
