// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Moniterm MacViking II 1024x768 fixed-resolution monochrome board

  https://wiki.preterhuman.net/Moniterm_MACVIKING_II

  VRAM from Fs040000 to Fs05FFFF (RAM test goes up to Fs065800 for some reason)
  Read from Fs000000 enables VBL, write to Fs000000 disables VBL
  Write to Fs080000 acks VBL

  Crystal (pixel clock) is 72 MHz.

  TODO: Actual raster parameters are unknown.  We've gone with the Apple 19"
  1024x768 monitor's htotal/vtotal but this used a custom monitor.  There are
  no register writes other than the IRQ enable/ack so tracing, the CRTC is
  pure TTL.

***************************************************************************/

#include "emu.h"
#include "nubus_vikbw.h"
#include "screen.h"

namespace {

static constexpr u32 VRAM_SIZE = 0x20000;       // 4x HM53461 VRAMs (64K x 4 bit)

class nubus_vikbw_device : public device_t,
						   public device_nubus_card_interface
{
public:
	// construction/destruction
	nubus_vikbw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint32_t viking_ack_r();
	void viking_ack_w(uint32_t data);
	uint32_t viking_enable_r();
	void viking_disable_w(uint32_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	std::vector<uint32_t> m_vram;
	uint32_t m_vbl_disable, m_palette[2];
};

ROM_START( vikbw )
	ROM_REGION(0x2000, "vikingrom", 0)
	ROM_LOAD( "viking.bin",   0x000000, 0x002000, CRC(92cf04d1) SHA1(d08349edfc82a0bd5ea848e053e1712092308f74) )
ROM_END

void nubus_vikbw_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_vikbw_device::screen_update));
	screen.set_raw(72000000, 1324, 0, 1024, 803, 0, 768);
}

const tiny_rom_entry *nubus_vikbw_device::device_rom_region() const
{
	return ROM_NAME( vikbw );
}

nubus_vikbw_device::nubus_vikbw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_vikbw_device(mconfig, NUBUS_VIKBW, tag, owner, clock)
{
}

nubus_vikbw_device::nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_vbl_disable(0)
{
}

void nubus_vikbw_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom("vikingrom", true);

	slotspace = get_slotspace();

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	install_bank(slotspace+0x40000, slotspace+0x40000+VRAM_SIZE-1, &m_vram[0]);
	install_bank(slotspace+0x940000, slotspace+0x940000+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_device(slotspace, slotspace+3, read32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_enable_r)), write32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_disable_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0x80000+3, read32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_ack_r)), write32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_ack_w)));
}

void nubus_vikbw_device::device_reset()
{
	m_vbl_disable = 1;
	std::fill(m_vram.begin(), m_vram.end(), 0);

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[1] = rgb_t(0, 0, 0);
}

uint32_t nubus_vikbw_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);
	for (int y = 0; y < 768; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1024/8; x++)
		{
			uint8_t const pixels = vram8[(y * 128) + x];

			*scanline++ = m_palette[BIT(pixels, 7)];
			*scanline++ = m_palette[BIT(pixels, 6)];
			*scanline++ = m_palette[BIT(pixels, 5)];
			*scanline++ = m_palette[BIT(pixels, 4)];
			*scanline++ = m_palette[BIT(pixels, 3)];
			*scanline++ = m_palette[BIT(pixels, 2)];
			*scanline++ = m_palette[BIT(pixels, 1)];
			*scanline++ = m_palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

void nubus_vikbw_device::viking_ack_w(uint32_t data)
{
	lower_slot_irq();
}

uint32_t nubus_vikbw_device::viking_ack_r()
{
	return 0;
}

void nubus_vikbw_device::viking_disable_w(uint32_t data)
{
	m_vbl_disable = 1;
}

uint32_t nubus_vikbw_device::viking_enable_r()
{
	m_vbl_disable = 0;
	return 0;
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NUBUS_VIKBW, device_nubus_card_interface, nubus_vikbw_device, "nb_vikbw", "Moniterm MacViking II video card")
