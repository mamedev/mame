// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
imm6-26 PROM Memory Module

This card has sockets for sixteen 1702A or similar 256×8 PROMs providing
up to 4KiB of program storage.  This card has additional features for
INTELLEC 8 systems, including module select lines and DIP switches for
mapping PROMs in place of console RAM.  These features are not emulated
by this class.

In theory you could populate any combination of sockets, and use
multiple cards provided you don't populate corresponding sockets on two
cards.  To simplify things, we assume contiguous sockets are populated
starting with PROM 0 (A2).  In practice, you'd want to populate this
socket as the boot vector is at address zero.  If you want to simulate
unpopulated sockets, just zero the corresponding area in your image
file.


Universal edge connector

                         1    2
                  GND    3    4  GND
                         5    6
                         7    8
                         9   10
                MAD 0   11   12  MAD 1
                MAD 2   13   14  MAD 3
                MAD 4   15   16  MAD 5
                MAD 6   17   18  MAD 7
                MAD 8   19   20  MAD 9
                        21   22
                MDI 0   23   24
                MDI 1   25   26
                MDI 3   27   28
                MDI 2   29   30
                MDI 5   31   32
                MDI 4   33   34
                MDI 7   35   36
                MDI 6   37   38
                        39   40
                        41   42
                 -10V   43   44  -10V
                        45   46
                        47   48
                        49   50
                        51   52
                        53   54
                        55   56
              /MAD 12   57   58  MS 12
               MAD 13   59   60  MAD 12
                MS 13   61   62  /MAD 13
              /MAD 14   63   64  MS 14
               MAD 15   65   66  MAD 14
                MS 15   67   68  /MAD 15
                        69   70
                        71   72
                        73   74
                        75   76
                        77   78
                        79   80
                        81   82  /ASMB
                        83   84
                        85   86
                        87   88
                        89   90
              /ADR STB  91   92
          RAM MOD ENBL  93   94  MAD 11
                        95   96  MAD 10
       /PROM MOD ENBL   97   98
                  +5V   99  100  +5V

MAD 0...MAD 7       mapped to MA0...MA7 on INTELLEC 4
MAD 8...MAD 11      mapped to C0...C3 on INTELLEC 4
MAD 12...MAD 15     not used on INTELLEC 4, used for module selection on INTELLEC 8
/MAD 12.../MAD 15   not used on INTELLEC 4, used for module selection on INTELLEC 8
MS 12...MS 15       not used on INTELLEC 4, used for module selection on INTELLEC 8
MDI 0...7           data outputs
/ASMB               global enable mapped to /PROM SEL on INTELLEC 4, not jumpered in on INTELLEC 8
/ADR STB            not jumpered in on INTELLEC 4 and INTELLEC 8, latch enables jumpered to GND
RAM MOD ENBL        not jumpered in on INTELLEC 4, allows PROM to overlay RAM on INTELLEC 8
/PROM MOD ENBL      not jumpered in on INTELLEC 4, used as global enable on INTELLEC 8
*/

#include "emu.h"
#include "prommemory.h"


namespace {

class imm6_26_device
		: public device_t
		, public bus::intellec4::device_univ_card_interface
		, public device_image_interface
{
public:
	imm6_26_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool        is_readable()                   const noexcept override { return true; }
	virtual bool        is_writeable()                  const noexcept override { return false; }
	virtual bool        is_creatable()                  const noexcept override { return false; }
	virtual bool        is_reset_on_load()              const noexcept override { return false; }
	virtual char const *file_extensions()               const noexcept override { return "rom,bin"; }
	virtual char const *image_type_name()               const noexcept override { return "promimage"; }
	virtual char const *image_brief_type_name()         const noexcept override { return "prom"; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void allocate();
	void unmap();

	std::unique_ptr<u8 []>  m_data;
};


imm6_26_device::imm6_26_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_PROM_MEMORY, tag, owner, clock)
	, bus::intellec4::device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_data()
{
}


std::pair<std::error_condition, std::string> imm6_26_device::call_load()
{
	if ((length() > 4096U) || (length() % 256U))
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				"Invalid PROM image size (must be a multiple of 256 bytes no larger than 4K)");
	}

	unmap();
	allocate();
	if (fread(m_data.get(), length()) != length())
		return std::make_pair(image_error::UNSPECIFIED, "Error reading file");

	// FIXME: gimme a cookie!
	rom_space().install_rom(0x1000U, offs_t(0x1000U + length()), m_data.get());

	return std::make_pair(std::error_condition(), std::string());
}

void imm6_26_device::call_unload()
{
	unmap();
}


void imm6_26_device::device_start()
{
	allocate();

	save_pointer(NAME(m_data), 4096U);
}


void imm6_26_device::allocate()
{
	if (!m_data)
	{
		m_data = std::make_unique<u8 []>(4096U);
		std::fill(m_data.get(), m_data.get() + 4096U, 0U);
	}
}

void imm6_26_device::unmap()
{
	// FIXME: where's my magic cookie?
	rom_space().unmap_read(0x1000U, 0x1fffU);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(INTELLEC4_PROM_MEMORY, bus::intellec4::device_univ_card_interface, imm6_26_device, "intlc4_imm6_26", "Intel imm6-26 PROM Memory Module")
