// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech Printer

    PRT-MPF :
      Printer driver utility           : ADDR 65AC GO
      Z80-Disassembler listing utility : ADDR 6000 GO
      Memory dump utility              : ADDR 6300 GO
      BASIC program listing utility    : ADDR 6400 GO
      Printer line feed                : ADDR 6500 GO

    PRT-MPF-IP : Address Mnemonic Function
                 6A00    SHIFT    Drive the thermal head shift right
                 6A10    PLINEFD  Line feed
                 6A30    PLINE    Drive the paper vertically by two lines
                 6A40    MTPPRT   Print out the contents of the line buffer

***************************************************************************/

#include "emu.h"
#include "prt.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_prt )
	ROM_REGION(0x1000, "u5", 0)
	ROM_LOAD("prt-ib.u5", 0x0000, 0x1000, CRC(730f2fb0) SHA1(f31536ee9dbb9babb9ce16a7490db654ca0b5749))
ROM_END

ROM_START( mpf_prt_ip )
	ROM_REGION(0x1000, "u5", 0)
	ROM_LOAD("prt-ip_v1.1.u5", 0x0000, 0x1000, CRC(4dd2a4eb) SHA1(6a3e7daa7834d67fd572261ed4a9a62c4594fe3f))
ROM_END


//-------------------------------------------------
//  mpf_prt_device - constructor
//-------------------------------------------------

class mpf_prt_device : public device_t, public device_mpf1_exp_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::PRINTER; }

	mpf_prt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: mpf_prt_device(mconfig, MPF_PRT, tag, owner, clock)
	{
	}

protected:
	mpf_prt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom_u5(*this, "u5")
		, m_rom_u6(*this, "u6")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override
	{
		// TODO: Seikosha MTP201A thermal printer

		GENERIC_SOCKET(config, "u6", generic_linear_slot, nullptr, "bin,rom");
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_prt );
	}

	virtual void device_start() override { }
	virtual void device_reset() override
	{
		program_space().install_rom(0x6000, 0x6fff, m_rom_u5->base());
		program_space().install_read_handler(0x7000, 0x7fff, emu::rw_delegate(*m_rom_u6, FUNC(generic_slot_device::read_rom)));

		io_space().install_write_handler(0xca, 0xca, emu::rw_delegate(*this, FUNC(mpf_prt_device::prt_w)));
		io_space().install_read_handler(0xcb, 0xcb, emu::rw_delegate(*this, FUNC(mpf_prt_device::prt_r)));
	}

private:
	required_memory_region m_rom_u5;
	required_device<generic_slot_device> m_rom_u6;

	uint8_t prt_r()
	{
		uint8_t data = 0x00;

		// bit 0 TGP
		// bit 1 HP

		return data;
	}

	void prt_w(uint8_t data)
	{
		// bit 0 TH7
		// bit 1 TH6
		// bit 2 TH5
		// bit 3 TH4
		// bit 4 TH3
		// bit 5 TH2
		// bit 6 TH1
		// bit 7 MOTOR
	}
};


class mpf_prt_ip_device : public mpf_prt_device
{
public:
	mpf_prt_ip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: mpf_prt_device(mconfig, MPF_PRT_IP, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_prt_ip );
	}

};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_PRT, device_mpf1_exp_interface, mpf_prt_device, "mpf1_prt", "Multitech PRT-MPF (Printer)")
DEFINE_DEVICE_TYPE_PRIVATE(MPF_PRT_IP, device_mpf1_exp_interface, mpf_prt_ip_device, "mpf1_prt_ip", "Multitech PRT-MPF-IP (Printer)")
