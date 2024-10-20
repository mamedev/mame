// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech Eprom Programmer Board

    EPB-MPF :
      MPF-1B : ADDR 9000 GO

    EPB-MPF-IBP :
      MPF-1P : G 9000
      MPF-1B : ADDR 9800 GO

***************************************************************************/

#include "emu.h"
#include "epb.h"
#include "machine/i8255.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_epb )
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD("epb-ib.u8", 0x0000, 0x0800, CRC(bbd854f5) SHA1(489e597c12a14a1cf9568d18b299e4f4ae656f71))
ROM_END

ROM_START( mpf_epb_ibp )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("ebp-ibp.u8", 0x0000, 0x1000, CRC(0156387a) SHA1(296842ee5bac23f41f46534663d4f0bfc04075cf))
ROM_END


//-------------------------------------------------
//  mpf_epb_device - constructor
//-------------------------------------------------

class mpf_epb_device : public device_t, public device_mpf1_exp_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::ROM; }

	mpf_epb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: mpf_epb_device(mconfig, MPF_EPB, tag, owner, clock)
	{
	}

protected:
	mpf_epb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
		, m_ppi(*this, "ppi")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		I8255(config, m_ppi);
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_epb );
	}

	virtual void device_start() override
	{
		m_ram = make_unique_clear<uint8_t[]>(0x1000);

		// register for save states
		save_pointer(NAME(m_ram), 0x1000);
	}

	virtual void device_reset() override
	{
		program_space().install_ram(0x8000, 0x8fff, m_ram.get());
		program_space().install_rom(0x9000, 0x97ff, m_rom->base());

		io_space().install_readwrite_handler(0xcc, 0xcf, emu::rw_delegate(*m_ppi, FUNC(i8255_device::read)), emu::rw_delegate(*m_ppi, FUNC(i8255_device::write)));
	}

	required_memory_region m_rom;
	required_device<i8255_device> m_ppi;

	std::unique_ptr<uint8_t[]> m_ram;
};


// ======================> mpf_epb_ibp_device

class mpf_epb_ibp_device : public mpf_epb_device
{
public:
	mpf_epb_ibp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: mpf_epb_device(mconfig, MPF_EPB_IBP, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_epb_ibp );
	}

	virtual void device_start() override
	{
		m_ram = make_unique_clear<uint8_t[]>(0x1800);

		// register for save states
		save_pointer(NAME(m_ram), 0x1800);
	}

	virtual void device_reset() override
	{
		program_space().install_rom(0x9000, 0x9fff, m_rom->base());
		program_space().install_ram(0xd800, 0xefff, m_ram.get());

		io_space().install_readwrite_handler(0x70, 0x7f, emu::rw_delegate(*m_ppi, FUNC(i8255_device::read)), emu::rw_delegate(*m_ppi, FUNC(i8255_device::write)));
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_EPB, device_mpf1_exp_interface, mpf_epb_device, "mpf1_epb", "Multitech EPB-MPF (Eprom Programmer Board)")
DEFINE_DEVICE_TYPE_PRIVATE(MPF_EPB_IBP, device_mpf1_exp_interface, mpf_epb_ibp_device, "mpf1_epb_ibp", "Multitech EPB-MPF-IBP (Eprom Programmer Board)")
