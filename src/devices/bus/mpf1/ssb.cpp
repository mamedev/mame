// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech Speech Synthesizer Board

    PROG 1     : Time-clock program                 ADDR 5000 GO
    CHECK-CODE : Self-test, speak all vocabulary    ADDR 51DC GO
    PROG 2     : Speech program utility             ADDR 5200 GO

    TODO: improve VSP READY callback to Z80 WAIT, currently VSP FIFO overflows.

***************************************************************************/

#include "emu.h"
#include "ssb.h"
#include "sound/tms5220.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "speaker.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_ssb )
	ROM_REGION(0x1000, "u5", 0)
	ROM_LOAD("ssb-mpf.bin", 0x0000, 0x1000, CRC(f926334f) SHA1(35847f8164eed4c0794a8b74e5d7fa972b10eb90))
ROM_END


//-------------------------------------------------
//  mpf_ssb_device - constructor
//-------------------------------------------------

class mpf_ssb_device : public device_t, public device_mpf1_exp_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	mpf_ssb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MPF_SSB, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom_u5(*this, "u5")
		, m_rom_u4(*this, "u4")
		, m_rom_u3(*this, "u3")
		, m_vsp(*this, "vsp")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		SPEAKER(config, "mono").front_center();

		TMS5200(config, m_vsp, 640000);
		m_vsp->ready_cb().set(DEVICE_SELF_OWNER, FUNC(mpf1_exp_device::wait_w));
		m_vsp->add_route(ALL_OUTPUTS, "mono", 0.5);

		GENERIC_SOCKET(config, "u4", generic_linear_slot, nullptr, "bin,rom");
		GENERIC_SOCKET(config, "u3", generic_linear_slot, nullptr, "bin,rom");
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_ssb );
	}

	virtual void device_start() override { }
	virtual void device_reset() override
	{
		program_space().install_rom(0x5000, 0x5fff, m_rom_u5->base());
		program_space().install_read_handler(0x6000, 0x6fff, emu::rw_delegate(*m_rom_u4, FUNC(generic_slot_device::read_rom)));
		program_space().install_read_handler(0x7000, 0x7fff, emu::rw_delegate(*m_rom_u3, FUNC(generic_slot_device::read_rom)));

		io_space().install_readwrite_handler(0xfe, 0xfe, emu::rw_delegate(*m_vsp, FUNC(tms5200_device::status_r)), emu::rw_delegate(*m_vsp, FUNC(tms5200_device::data_w)));
	}

private:
	required_memory_region m_rom_u5;
	required_device<generic_slot_device> m_rom_u4;
	required_device<generic_slot_device> m_rom_u3;
	required_device<tms5200_device> m_vsp;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_SSB, device_mpf1_exp_interface, mpf_ssb_device, "mpf1_ssb", "Multitech SSB-MPF (Speech Synthesizer Board)")
