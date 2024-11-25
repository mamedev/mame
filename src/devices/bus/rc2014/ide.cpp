// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 IDE

****************************************************************************/

#include "emu.h"
#include "ide.h"
#include "machine/i8255.h"
#include "bus/ata/ataintf.h"

namespace {

//**************************************************************************
//  rc2014_ide_base
//**************************************************************************

class rc2014_ide_base : public device_t, public device_rc2014_card_interface
{
protected:
	// construction/destruction
	rc2014_ide_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t ppi_pa_r() { return m_data.b.l; }
	uint8_t ppi_pb_r() { return m_data.b.h; }
	void ppi_pa_w(uint8_t data) { m_data.b.l = data; }
	void ppi_pb_w(uint8_t data) { m_data.b.h = data; }
	virtual void ppi_pc_w(uint8_t data) = 0;

	// base-class members
	required_device<ata_interface_device> m_ata;
	required_device<i8255_device> m_ppi;

	PAIR16 m_data;
	uint8_t m_prev;
};

rc2014_ide_base::rc2014_ide_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_ata(*this, "ata")
	, m_ppi(*this, "82c55")
	, m_prev(0)
{
}

void rc2014_ide_base::device_start()
{
}

void rc2014_ide_base::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi, 0);
	m_ppi->in_pa_callback().set(FUNC(rc2014_ide_base::ppi_pa_r));
	m_ppi->in_pb_callback().set(FUNC(rc2014_ide_base::ppi_pb_r));
	m_ppi->out_pa_callback().set(FUNC(rc2014_ide_base::ppi_pa_w));
	m_ppi->out_pb_callback().set(FUNC(rc2014_ide_base::ppi_pb_w));
	m_ppi->out_pc_callback().set(FUNC(rc2014_ide_base::ppi_pc_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", "hdd", false);
}

//**************************************************************************
//  RC2014 82C55 IDE Interface
//  Module author: Ed Brindley
//**************************************************************************

class rc2014_82c55_ide_device : public rc2014_ide_base
{
public:
	// construction/destruction
	rc2014_82c55_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void ppi_pc_w(uint8_t data) override;
private:
	required_ioport m_sw;
	required_ioport_array<4> m_jp;

	uint8_t m_da0;
	uint8_t m_da1;
	uint8_t m_dcs1;
	uint8_t m_dior;
};

rc2014_82c55_ide_device::rc2014_82c55_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rc2014_ide_base(mconfig, RC2014_82C55_IDE, tag, owner, clock)
	, m_sw(*this, "SW1")
	, m_jp(*this, "JP%u", 1U)
{
}

void rc2014_82c55_ide_device::device_reset()
{
	m_da0 = (m_jp[0]->read() == 1) ? 4 : 0;
	m_dcs1 = (m_jp[1]->read() == 1) ? 0 : 4;
	m_da1 = (m_jp[2]->read() == 1) ? 6 : 1;
	m_dior = (m_jp[3]->read() == 1) ? 1 : 6;

	uint8_t base = m_sw->read(); // SW1
	// A15-A8 not connected
	m_bus->installer(AS_IO)->install_readwrite_handler(base, base+0x03, 0, 0xff00, 0, read8sm_delegate(m_ppi, FUNC(i8255_device::read)), write8sm_delegate(m_ppi, FUNC(i8255_device::write)));
}

void rc2014_82c55_ide_device::ppi_pc_w(uint8_t data)
{
	// On IDE connector DIOW and DIOR are inverted
	// write is on falling and read on rising edge
	uint8_t offset = BIT(data,2) << 2 | BIT(data,m_da1) << 1 | BIT(data,m_da0);

	if (BIT(data,3)) // DCS0
	{
		if (BIT(data,5)==0 && BIT(m_prev,5)==1) // DIOW
		{
			m_ata->cs0_w(offset, m_data.w);
		}
		if (BIT(data,m_dior)==1 && BIT(m_prev,m_dior)==0) // DIOR
		{
			m_data.w = m_ata->cs0_r(offset);
		}
	}
	if (BIT(data,m_dcs1)) // DCS1
	{
		if (BIT(data,5)==0 && BIT(m_prev,5)==1) // DIOW
		{
			m_ata->cs1_w(offset, m_data.w);
		}
		if (BIT(data,m_dior)==1 && BIT(m_prev,m_dior)==0) // DIOR
		{
			m_data.w = m_ata->cs1_r(offset);
		}
	}
	if (BIT(data,7)) // DRESET
	{
		// RESET IDE Disk
		m_ata->reset();
	}
	m_prev = data;

}

static INPUT_PORTS_START( rc2014_82c55_ide_jumpers )
	PORT_START("SW1")
	PORT_DIPNAME(0x04, 0x00, "0x04") PORT_DIPLOCATION("Base Address:1")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x04, DEF_STR( On ) )
	PORT_DIPNAME(0x08, 0x00, "0x08") PORT_DIPLOCATION("Base Address:2")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x08, DEF_STR( On ) )
	PORT_DIPNAME(0x10, 0x00, "0x10") PORT_DIPLOCATION("Base Address:3")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x10, DEF_STR( On ) )
	PORT_DIPNAME(0x20, 0x20, "0x20") PORT_DIPLOCATION("Base Address:4")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x00, "0x40") PORT_DIPLOCATION("Base Address:5")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x40, DEF_STR( On ) )
	PORT_DIPNAME(0x80, 0x00, "0x80") PORT_DIPLOCATION("Base Address:6")
	PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(0x80, DEF_STR( On ) )
	PORT_START("JP1")
	PORT_CONFNAME( 0x1, 0x0, "DA0" )
	PORT_CONFSETTING( 0x0, "PC0" )
	PORT_CONFSETTING( 0x1, "PC4" )
	PORT_START("JP2")
	PORT_CONFNAME( 0x1, 0x0, "DCS1" )
	PORT_CONFSETTING( 0x0, "PC4" )
	PORT_CONFSETTING( 0x1, "PC0" )
	PORT_START("JP3")
	PORT_CONFNAME( 0x1, 0x0, "DA1" )
	PORT_CONFSETTING( 0x0, "PC1" )
	PORT_CONFSETTING( 0x1, "PC6" )
	PORT_START("JP4")
	PORT_CONFNAME( 0x1, 0x0, "DIOR" )
	PORT_CONFSETTING( 0x0, "PC6" )
	PORT_CONFSETTING( 0x1, "PC1" )
	PORT_START("JP5")
	PORT_CONFNAME( 0x1, 0x0, "Power" )
	PORT_CONFSETTING( 0x0, "Bus" )
	PORT_CONFSETTING( 0x1, "External" )
INPUT_PORTS_END

ioport_constructor rc2014_82c55_ide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_82c55_ide_jumpers );
}

//**************************************************************************
//  RC2014 IDE Hard Drive Module
//  Module author: Spencer Owen
//  Based on design from Ed Brindley, but simplified
//**************************************************************************

class rc2014_ide_hdd_device : public rc2014_ide_base
{
public:
	// construction/destruction
	rc2014_ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void ppi_pc_w(uint8_t data) override;
private:
	required_ioport m_jp;
};

rc2014_ide_hdd_device::rc2014_ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: rc2014_ide_base(mconfig, RC2014_IDE_HDD, tag, owner, clock)
	, m_jp(*this, "J1")
{
}

void rc2014_ide_hdd_device::device_reset()
{
	uint8_t base = m_jp->read() << 3;
	// A15-A8 are not connected, A7,A6 and A2 must be 0
	m_bus->installer(AS_IO)->install_readwrite_handler(base, base+0x03, 0, 0xff00, 0, read8sm_delegate(m_ppi, FUNC(i8255_device::read)), write8sm_delegate(m_ppi, FUNC(i8255_device::write)));
}

void rc2014_ide_hdd_device::ppi_pc_w(uint8_t data)
{
	uint8_t offset = data & 7;

	if (BIT(data,3)) // DCS0
	{
		if (BIT(data,5)==0 && BIT(m_prev,5)==1) // DIOW
		{
			m_ata->cs0_w(offset, m_data.w);
		}
		if (BIT(data,6)==1 && BIT(m_prev,6)==0) // DIOR
		{
			m_data.w = m_ata->cs0_r(offset);
		}
	}
	if (BIT(data,4)) // DCS1
	{
		if (BIT(data,5)==0 && BIT(m_prev,5)==1) // DIOW
		{
			m_ata->cs1_w(offset, m_data.w);
		}
		if (BIT(data,6)==1 && BIT(m_prev,6)==0) // DIOR
		{
			m_data.w = m_ata->cs1_r(offset);
		}
	}
	if (BIT(data,7)) // DRESET
	{
		// RESET IDE Disk
		m_ata->reset();
	}
	m_prev = data;

}

static INPUT_PORTS_START( rc2014_ide_hdd_jumpers )
	PORT_START("J1")
	PORT_CONFNAME( 0x7, 0x4, "Base Address" )
	PORT_CONFSETTING( 0x1, "0x08" )
	PORT_CONFSETTING( 0x2, "0x10" )
	PORT_CONFSETTING( 0x3, "0x18" )
	PORT_CONFSETTING( 0x4, "0x20" )
	PORT_CONFSETTING( 0x5, "0x28" )
	PORT_CONFSETTING( 0x6, "0x30" )
	PORT_CONFSETTING( 0x7, "0x38" )
	PORT_START("JP1")
	PORT_CONFNAME( 0x1, 0x0, "Power" )
	PORT_CONFSETTING( 0x0, "Bus" )
	PORT_CONFSETTING( 0x1, "External" )
INPUT_PORTS_END

ioport_constructor rc2014_ide_hdd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_ide_hdd_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_82C55_IDE, device_rc2014_card_interface, rc2014_82c55_ide_device, "rc2014_82c55_ide", "RC2014 82C55 IDE Interface")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_IDE_HDD, device_rc2014_card_interface, rc2014_ide_hdd_device, "rc2014_ide_hdd", "RC2014 IDE Hard Drive Module")
