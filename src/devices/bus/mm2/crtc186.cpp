// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 CRTC186 emulation

*********************************************************************/

#include "emu.h"
#include "crtc186.h"

DEFINE_DEVICE_TYPE(NOKIA_CRTC186, crtc186_device, "nokia_crtc186", "Nokia MikroMikko 2 CRTC186")

crtc186_device::crtc186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NOKIA_CRTC186, tag, owner, clock),
	device_mikromikko2_expansion_bus_card_interface(mconfig, *this),
	m_video_ram(*this, "ram", 0x4000, ENDIANNESS_LITTLE),
	m_char_rom(*this, "chargen"),
	m_attr_rom(*this, "attr"),
	m_vpac(*this, "crt9007"),
	m_drb0(*this, "crt9212_0"),
	m_drb1(*this, "crt9212_1"),
	m_timer_vidldsh(*this, "vidldsh"),
	m_sio(*this, "i8251"),
	m_kb(*this, "kb"),
	m_ctrl(*this, "control"),
	m_screen(*this, "screen")
{
}

void crtc186_device::map(address_map &map)
{
	map(0x00, 0x3f).rw(FUNC(crtc186_device::vpac_r), FUNC(crtc186_device::vpac_w));
	//map(0x00, 0x01).rw(m_sio, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w)).umask16(0xff00);
	//map(0x02, 0x03).rw(m_sio, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w)).umask16(0xff00);
	//map(0x00, 0x3f).rw(m_vpac, FUNC(crt9007_device::read), FUNC(crt9007_device::write)).umask16(0x00ff);
	map(0x40, 0x4f).w(m_ctrl, FUNC(ls259_device::write_d0)).umask16(0x00ff);
}

ROM_START( crtc186 )
	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "9067e.ic40", 0x0000, 0x2000, CRC(fa719d92) SHA1(af6cc03a8171b9c95e8548c5e0268816344d7367) )

	ROM_REGION( 0x2000, "attr", 0 )
	ROM_LOAD( "9026a.ic26", 0x0000, 0x2000, CRC(fe1da600) SHA1(3a5512b08d8f7bb5a0ff3f50bcf33de649a0489d) )

	ROM_REGION( 0x100, "timing", 0 )
	ROM_LOAD( "739025b.ic8", 0x000, 0x100, CRC(c538b10a) SHA1(9810732a52ee6b8313d27462b27acc7e4d5badeb) )
ROM_END

const tiny_rom_entry *crtc186_device::device_rom_region() const
{
	return ROM_NAME( crtc186 );
}

void crtc186_device::vpac_mem(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(crtc186_device::videoram_r));
}

void crtc186_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set(FUNC(crtc186_device::int_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(71.77);
	m_screen->set_screen_update(FUNC(crtc186_device::screen_update));
	m_screen->set_size(800, 420);
	m_screen->set_visarea(0, 800-1, 0, 420-1);

	CRT9007(config, m_vpac, XTAL(35'452'500)/8);
	m_vpac->set_addrmap(0, &crtc186_device::vpac_mem);
	m_vpac->set_character_width(10);
	m_vpac->int_callback().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_vpac->vlt_callback().set(FUNC(crtc186_device::vpac_vlt_w));
	m_vpac->drb_callback().set(FUNC(crtc186_device::vpac_drb_w));
	m_vpac->wben_callback().set(FUNC(crtc186_device::vpac_wben_w));
	m_vpac->cblank_callback().set(FUNC(crtc186_device::vpac_cblank_w));
	m_vpac->slg_callback().set(FUNC(crtc186_device::vpac_slg_w));
	m_vpac->sld_callback().set(FUNC(crtc186_device::vpac_sld_w));
	m_vpac->set_screen(m_screen);

	CRT9212(config, m_drb0, 0);
	m_drb0->set_wen2(1);
	m_drb0->dout().set(FUNC(crtc186_device::vidla_w));

	CRT9212(config, m_drb1, 0);
	m_drb1->set_wen2(1);
	m_drb1->dout().set(FUNC(crtc186_device::drb_attr_w));

	TIMER(config, "vidldsh").configure_generic(FUNC(crtc186_device::vidldsh_tick));

	LS259(config, m_ctrl);
	m_ctrl->q_out_cb<0>().set(FUNC(crtc186_device::cpl_w));
	m_ctrl->q_out_cb<1>().set(FUNC(crtc186_device::blc_w));
	m_ctrl->q_out_cb<2>().set(FUNC(crtc186_device::mode_w));
	m_ctrl->q_out_cb<3>().set(FUNC(crtc186_device::modeg_w));
	m_ctrl->q_out_cb<5>().set(FUNC(crtc186_device::c70_50_w));
	m_ctrl->q_out_cb<6>().set(FUNC(crtc186_device::crb_w));
	m_ctrl->q_out_cb<7>().set(FUNC(crtc186_device::cru_w));

	clock_device &sio_clock(CLOCK(config, "sio_clock", XTAL(16'000'000)/4/26));
	sio_clock.signal_handler().set(m_sio, FUNC(i8251_device::write_txc));
	sio_clock.signal_handler().append(m_sio, FUNC(i8251_device::write_rxc));

	I8251(config, m_sio, 0);
	m_sio->rxrdy_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	m_sio->txrdy_handler().set("irqs", FUNC(input_merger_device::in_w<2>));
	m_sio->txd_handler().set(m_kb, FUNC(mm2_keyboard_device::rxd_w));

	NOKIA_MM2_KBD(config, m_kb, 0);
	m_kb->txd_handler().set(m_sio, FUNC(i8251_device::write_rxd));
}

void crtc186_device::device_start()
{
	m_bus->memspace().install_ram(0xd0000, 0xd3fff, m_video_ram);
	m_bus->memspace().install_read_tap(0xd8000, 0xdbfff, "charrom", [this](offs_t offset, uint16_t &data, uint16_t mem_mask) {
		if (ACCESSING_BITS_0_7) {
			data |= m_char_rom->base()[(offset >> 1) & 0x1fff];
		}
	});
	m_bus->iospace().install_device(0xf980, 0xf9ff, *this, &crtc186_device::map);

	// state saving
	save_item(NAME(m_vidla));
	save_item(NAME(m_cpl));
	save_item(NAME(m_blc));
	save_item(NAME(m_mode));
	save_item(NAME(m_modeg));
	save_item(NAME(m_c70_50));
	save_item(NAME(m_cru));
	save_item(NAME(m_crb));
	save_item(NAME(m_cursor_x));
	save_item(NAME(m_cursor_y));
}

uint32_t crtc186_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_blc) {
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	bitmap.fill(m_cpl ? rgb_t::black() : rgb_t::white(), cliprect);

	for (int sy = 0; sy < 28; sy++)
	{
		for (int y = 0; y < 15; y++)
		{
			for (int sx = 0; sx < 80; sx++)
			{
				bool const cursor = (sx == m_cursor_x) && (sy == m_cursor_y);
				offs_t const vram_addr = (sy * 80) + sx;
				u16 const data = m_video_ram[vram_addr & 0x1fff];

				offs_t const char_addr = ((data & 0x1ff) << 4) + y;
				u8 char_data = m_char_rom->base()[char_addr & 0x1fff];

				rgb_t const bgcolor = BIT(data, 11) ? halflit() : rgb_t::white();

				for (int bit = 0; bit < 10; bit++)
				{
					bool pixel = m_cpl;
					if (bit > 0 && bit < 9) {
						pixel = BIT(char_data, 0) ^ m_cpl;
						char_data >>= 1;
					}

					if (cursor) {
						if (!m_cru)
							pixel ^= 1;
						else if (m_cru && y == 14)
							pixel = 1;
					}

					bitmap.pix((sy * 15) + y, (sx * 10) + bit) = pixel ? rgb_t::black() : bgcolor;
				}
			}
		}
	}

	return 0;
}

uint16_t crtc186_device::vpac_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_0_7) {
		data |= m_vpac->read(offset);
	}

	if (ACCESSING_BITS_8_15) {
		if (offset == 0)
			data |= m_sio->data_r() << 8;
		else if (offset == 1)
			data |= m_sio->status_r() << 8;
	}

	return data;
}

void crtc186_device::vpac_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7) {
		m_vpac->write(offset, data & 0xff);

		if (offset == 0x18)
			m_cursor_y = data & 0xff;
		else if (offset == 0x19)
			m_cursor_x = data & 0xff;
	}

	if (ACCESSING_BITS_8_15) {
		if (offset == 0)
			m_sio->data_w(data >> 8);
		else if (offset == 1)
			m_sio->control_w(data >> 8);
	}
}

uint8_t crtc186_device::videoram_r(offs_t offset)
{
	u16 data = m_video_ram[offset >> 1];

	// character
	m_drb0->write(data & 0xff);

	// attributes
	m_drb1->write(data >> 8);

	return data & 0xff;
}

void crtc186_device::vpac_vlt_w(int state)
{
	m_drb0->ren_w(state);
	m_drb0->clrcnt_w(state);

	m_drb1->ren_w(state);
	m_drb1->clrcnt_w(state);
}

void crtc186_device::vpac_drb_w(int state)
{
	m_drb0->tog_w(state);
	m_drb1->tog_w(state);
}

void crtc186_device::vpac_wben_w(int state)
{
	m_drb0->wen1_w(state);
	m_drb1->wen1_w(state);
}

void crtc186_device::vpac_cblank_w(int state)
{
}

void crtc186_device::vpac_slg_w(int state)
{
}

void crtc186_device::vpac_sld_w(int state)
{
}

void crtc186_device::vidla_w(uint8_t data)
{
	m_vidla = data;
}

void crtc186_device::drb_attr_w(uint8_t data)
{
}

void crtc186_device::set_vidldsh_timer()
{
	const XTAL busdotclk = XTAL(35'452'500) / (m_mode ? 2 : 1);
	const XTAL vidcclk = busdotclk / (m_modeg ? 16 : 10);

	m_vpac->set_character_width(m_modeg ? 16 : 10);
	m_vpac->set_unscaled_clock(vidcclk);

	m_timer_vidldsh->adjust(attotime::from_hz(vidcclk), 0, attotime::from_hz(vidcclk));
}

TIMER_DEVICE_CALLBACK_MEMBER( crtc186_device::vidldsh_tick )
{
	m_drb0->rclk_w(0);
	m_drb0->wclk_w(0);
	m_drb1->rclk_w(0);
	m_drb1->wclk_w(0);

	m_drb0->rclk_w(1);
	m_drb0->wclk_w(1);
	m_drb1->rclk_w(1);
	m_drb1->wclk_w(1);

	// TODO draw character
}
