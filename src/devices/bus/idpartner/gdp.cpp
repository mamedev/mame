// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**********************************************************************

    Iskra Delta GDP Card

**********************************************************************/

#include "emu.h"
#include "gdp.h"

#include "machine/z80pio.h"
#include "video/ef9365.h"
#include "video/scn2674.h"

#include "screen.h"

namespace {

class idpartner_gdp_device :
    public device_t,
    public bus::idpartner::device_exp_card_interface
{
public:
    // construction/destruction
    idpartner_gdp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
    // device_t implementation
    virtual void device_start() override;
    virtual void device_reset() override;
    virtual void device_add_mconfig(machine_config &config) override;

private:
    void char_w(u8 data);
    void attr_w(u8 data);
    void scroll_w(u8 data);
    void gdc_map(address_map &map);
    void int_w(int state) { m_bus->int_w(state); }
    void nmi_w(int state) { m_bus->nmi_w(state); }

    required_device<ef9365_device> m_gdc;
    //required_device<scn2674_device> m_avdc;
    required_device<z80pio_device> m_pio;
};


idpartner_gdp_device::idpartner_gdp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : device_t(mconfig, IDPARTNER_GDP, tag, owner, clock)
    , bus::idpartner::device_exp_card_interface(mconfig, *this)
    , m_gdc(*this, "gdc")
    //, m_avdc(*this, "avdc")
    , m_pio(*this, "pio")
{
}

/*----------------------------------
  device_t implementation
----------------------------------*/

void idpartner_gdp_device::device_start()
{
}

void idpartner_gdp_device::device_reset()
{
    m_bus->io().install_read_handler (0x20, 0x2f, emu::rw_delegate(m_gdc, FUNC(ef9365_device::data_r))); // Thomson GDP EF9367
    m_bus->io().install_write_handler(0x20, 0x2f, emu::rw_delegate(m_gdc, FUNC(ef9365_device::data_w))); // Thomson GDP EF9367
    m_bus->io().install_read_handler (0x30, 0x33, emu::rw_delegate(m_pio, FUNC(z80pio_device::read_alt))); // PIO - Graphics
    m_bus->io().install_write_handler(0x30, 0x33, emu::rw_delegate(m_pio, FUNC(z80pio_device::write_alt))); // PIO - Graphics
    m_bus->io().install_write_handler(0x34, 0x34, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::char_w))); // char reg
    m_bus->io().install_write_handler(0x35, 0x35, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::attr_w))); // attr reg
    m_bus->io().install_write_handler(0x36, 0x36, emu::rw_delegate(*this, FUNC(idpartner_gdp_device::scroll_w))); // scroll reg/common input
    //map(0x38,0x3f).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write)); // AVDC SCN2674
}

void idpartner_gdp_device::char_w(u8 data)
{
    //printf("char:%02x\n",data);
}

void idpartner_gdp_device::attr_w(u8 data)
{
    //printf("attr:%02x\n",data);
}

void idpartner_gdp_device::scroll_w(u8 data)
{
    //printf("scroll:%02x\n",data);
}

void idpartner_gdp_device::gdc_map(address_map &map)
{
    map(0x0000, 0x1ffff).ram();
}

void idpartner_gdp_device::device_add_mconfig(machine_config &config)
{
    screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
    screen.set_screen_update(m_gdc, FUNC(ef9365_device::screen_update));
    screen.set_size(1024, 512);
    screen.set_visarea(0, 1024-1, 0, 512-1);
    screen.set_refresh_hz(25);
    PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

    EF9365(config, m_gdc, XTAL(24'000'000) / 16); // EF9367 1.5 MHz
    m_gdc->set_screen("screen");
    m_gdc->set_addrmap(0, &idpartner_gdp_device::gdc_map);
    m_gdc->set_palette_tag("palette");
    m_gdc->set_nb_bitplanes(1);
    m_gdc->set_display_mode(ef9365_device::DISPLAY_MODE_1024x512);

    //SCN2674(config, m_avdc, XTAL(24'000'000) / 16); // SCN2674B
    //m_avdc->set_screen("screen");
    //m_avdc->set_character_width(12);
    //m_avdc->intr_callback().set(FUNC(idpartner_gdp_device::nmi_w));

    Z80PIO(config, m_pio, XTAL(8'000'000) / 2);
    m_pio->out_int_callback().set(FUNC(idpartner_gdp_device::int_w));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(IDPARTNER_GDP, bus::idpartner::device_exp_card_interface, idpartner_gdp_device, "gdp", "Iskra Delta Partner GDP")
