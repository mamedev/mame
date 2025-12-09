// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom MDCR

    https://site.acornatom.nl/hardware/storage/mdcr/

    Error messages:
      101 No cassette
      102 End of tape
      103 Nothing on tape
      104 No preamble
      105 Incorrect load
      106 Write protect
      107 File not found

**********************************************************************/

#include "emu.h"
#include "mdcr.h"

#include "machine/mdcr.h"


namespace {

class atom_mdcr_device : public device_t, public device_acorn_bus_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::TAPE; }

	atom_mdcr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_MDCR, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_mdcr(*this, "mdcr")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;
	virtual void write_cb2(int state) override;

private:
	required_device<mdcr_device> m_mdcr;

	void cb1_w(int state);

	bool m_output_enable;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_mdcr_device::device_add_mconfig(machine_config &config)
{
	MDCR(config, m_mdcr);
	m_mdcr->rdc_cb().set(FUNC(atom_mdcr_device::cb1_w));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t atom_mdcr_device::pb_r()
{
	uint8_t data = 0x00;

	data |= m_mdcr->rda() << 0;
	data |= m_mdcr->wen() << 1;
	data |= m_mdcr->bet() << 6;
	data |= m_mdcr->cip() << 7;

	return data;
}

void atom_mdcr_device::pb_w(uint8_t data)
{
	m_output_enable = !BIT(data, 4);

	if (m_output_enable)
	{
		m_mdcr->fwd(BIT(data, 2));
		m_mdcr->rev(BIT(data, 3));
		m_mdcr->wdc(BIT(data, 5));
	}
}

void atom_mdcr_device::write_cb2(int state)
{
	if (m_output_enable)
	{
		m_mdcr->wda(state);
	}
}

void atom_mdcr_device::cb1_w(int state)
{
	m_bus->cb1_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_MDCR, device_acorn_bus_interface, atom_mdcr_device, "atom_mdcr", "Atom MDCR")
