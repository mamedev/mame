// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

Emulation of SuperSoniqs' Franky cartridge, the predecessor of SuperSoniqs'
PlaySonic cartridge.

**********************************************************************************/

#include "emu.h"
#include "franky.h"

#include "video/315_5124.h"

#include "speaker.h"

namespace {

class msx_cart_franky_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_franky_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_FRANKY, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_vdp(*this, "vdp")
		, m_screen(*this, "screen")
	{ }

protected:
	virtual void device_start() override;

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<sega315_5124_device> m_vdp;
	required_device<screen_device> m_screen;
};


void msx_cart_franky_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "franky").front_center();

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	m_screen->set_raw(XTAL(10'738'635)/2,
		sega315_5124_device::WIDTH,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10,
		sega315_5124_device::HEIGHT_NTSC,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	m_screen->set_refresh_hz(XTAL(10'738'635)/2 / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
	m_screen->set_screen_update(FUNC(msx_cart_franky_device::screen_update));

	SEGA315_5246(config, m_vdp, XTAL(10'738'635));
	m_vdp->set_screen(m_screen);
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set(*this, FUNC(msx_cart_franky_device::irq_out));
	// There is no NMI signal on the cartridge slot, so no way to hook this up
	//m_vdp->n_nmi().set_inputline(maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "franky", 1.00);
}


void msx_cart_franky_device::device_start()
{
	io_space().install_write_handler(0x48, 0x49, emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::psg_w)));
	io_space().install_read_handler(0x48, 0x48, emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::vcount_read)));
	io_space().install_read_handler(0x49, 0x49, emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::hcount_read)));
	io_space().install_readwrite_handler(0x88, 0x88, emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::data_read)), emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::data_write)));
	io_space().install_readwrite_handler(0x89, 0x89, emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::control_read)), emu::rw_delegate(*m_vdp, FUNC(sega315_5124_device::control_write)));
}


uint32_t msx_cart_franky_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_vdp->screen_update(screen, bitmap, cliprect);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FRANKY, msx_cart_interface, msx_cart_franky_device, "msx_cart_franky", "MSX Cartridge - Franky")
