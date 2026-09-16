// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech Sound Generation Board

    Piano : ADDR C200 GO

***************************************************************************/

#include "emu.h"
#include "sgb.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_sgb )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("sgb-mpf.bin", 0x0000, 0x1000, CRC(2d51d964) SHA1(13f3c7243d691b66ad5f0dcbbc16488c8905a394))
ROM_END


//-------------------------------------------------
//  mpf_sgb_device - constructor
//-------------------------------------------------

class mpf_sgb_device : public device_t, public device_mpf1_exp_interface
{
public:
	mpf_sgb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MPF_SGB, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
		, m_ay(*this, "ay8910")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		SPEAKER(config, "mono").front_center();
		AY8910(config, m_ay, DERIVED_CLOCK(1, 1)).add_route(ALL_OUTPUTS, "mono", 1.0);
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_sgb );
	}

	virtual void device_start() override { }
	virtual void device_reset() override
	{
		program_space().install_rom(0xc000, 0xcfff, m_rom->base());

		io_space().install_read_handler(0xc1, 0xc1, emu::rw_delegate(*m_ay, FUNC(ay8910_device::data_r)));
		io_space().install_write_handler(0xc2, 0xc3, emu::rw_delegate(*m_ay, FUNC(ay8910_device::data_address_w)));
	}

private:
	required_memory_region m_rom;
	required_device<ay8910_device> m_ay;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_SGB, device_mpf1_exp_interface, mpf_sgb_device, "mpf1_sgb", "Multitech SGB-MPF (Sound Generation Board)")
