// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro SD Card

    Original Interface Hardware
    ===========================

    This is the original interface design, as conceived by Martin Mather
    in 2006.

    User Port     SD Card
     (Master)     (Slave)
    =========     =======
      CB1/PB1 ==> S_CLK  (Clock)
          CB2 <== S_MISO (Dout)
          PB0 ==> S_MOSI (Din)
           0V ==> S_SEL  (Select)

    TurboMMC Interface Hardware
    ===========================

    The TurboMMC hardware is a more complicated, as it uses PB[4:2] to
    control three buffers, providing two different operating modes.

    When PB[4:2] are 010, then the hardware is configured as above, and
    the interface is compatible with the original hardware.

    When PB[4:2] are 101, then the hardware is re-configured as:

    User Port     SD Card
     (Master)     (Slave)
    =========     =======
      CB1/PB1 ==> S_CLK  (Clock)
          n/c <== S_MISO (Dout)
          CB2 ==> S_MOSI (Din)
           0V ==> S_SEL  (Select)

**********************************************************************/


#include "emu.h"
#include "sdcard.h"

#include "machine/spi_sdcard.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_sdcard_device

class bbc_sdcard_device : public device_t, public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_bbc_userport_interface implementation
	virtual void pb_w(uint8_t data) override;
	virtual void write_cb1(int state) override;

	required_device<spi_sdcard_device> m_sdcard;
};


// ======================> bbc_sdcardt_device

class bbc_sdcardt_device : public bbc_sdcard_device
{
public:
	// construction/destruction
	bbc_sdcardt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_bbc_userport_interface implementation
	virtual void pb_w(uint8_t data) override;
	virtual void write_cb1(int state) override;
	virtual void write_cb2(int state) override;

private:
	bool m_turbo;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_sdcard_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set([this](int state) { m_slot->cb2_w(state); });
}

void bbc_sdcardt_device::device_add_mconfig(machine_config &config)
{
	SPI_SDCARD(config, m_sdcard, 0);
	m_sdcard->set_prefer_sdhc();
	m_sdcard->spi_miso_callback().set([this](int state) { if (!m_turbo) m_slot->cb2_w(state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_sdcard_device - constructor
//-------------------------------------------------

bbc_sdcard_device::bbc_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_userport_interface(mconfig, *this)
	, m_sdcard(*this, "sdcard")
{
}

bbc_sdcard_device::bbc_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sdcard_device(mconfig, BBC_SDCARD, tag, owner, clock)
{
}

bbc_sdcardt_device::bbc_sdcardt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_sdcard_device(mconfig, BBC_SDCARDT, tag, owner, clock)
	, m_turbo(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_sdcard_device::device_start()
{
}

void bbc_sdcardt_device::device_start()
{
	save_item(NAME(m_turbo));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_sdcard_device::write_cb1(int state)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_clock_w(state);
}

void bbc_sdcard_device::pb_w(uint8_t data)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_mosi_w(BIT(data, 0));
	m_sdcard->spi_clock_w(BIT(data, 1));

	m_slot->cb1_w(BIT(data, 1));
}


void bbc_sdcardt_device::write_cb1(int state)
{
	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_clock_w(state);
}

void bbc_sdcardt_device::write_cb2(int state)
{
	if (m_turbo)
	{
		m_sdcard->spi_ss_w(1);
		m_sdcard->spi_mosi_w(state);
	}
}

void bbc_sdcardt_device::pb_w(uint8_t data)
{
	m_turbo = (data & 0x1c) == 0x14;

	m_sdcard->spi_ss_w(1);
	m_sdcard->spi_mosi_w(BIT(data, 0));
	m_sdcard->spi_clock_w(BIT(data, 1));

	m_slot->cb1_w(BIT(data, 1));
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BBC_SDCARD, device_bbc_userport_interface, bbc_sdcard_device, "bbc_sdcard", "BBC Micro SD Card")
DEFINE_DEVICE_TYPE_PRIVATE(BBC_SDCARDT, device_bbc_userport_interface, bbc_sdcardt_device, "bbc_sdcardt", "BBC Micro Turbo SD Card")
