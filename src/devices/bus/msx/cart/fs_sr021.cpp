// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***********************************************************************************

Emulation of the FS-SR021 MSX2 Word Processor Cartridge.

TODO:
- Display of JIS level 1 characters might not be fully correct, the tools do seem to display fine.
- Unknown how JIS level 2 characters are input/displayed.

***********************************************************************************/
#include "emu.h"
#include "fs_sr021.h"

namespace {

class msx_cart_fs_sr021_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_fs_sr021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_FS_SR021, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_bank(*this, "bank%u", 0U)
		, m_view{ {*this, "view0000"}, {*this, "view2000"}, {*this, "view4000"}, {*this, "view6000"}, {*this, "view8000"}, {*this, "viewa000"} }
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr size_t BANK_SIZE = 0x2000;

	template <int Bank> void bank_w(u8 data);
	template <int Bank> void set_view();
	void control_w(u8 data);
	u8 bank_r(offs_t offset);
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);

	memory_bank_array_creator<6> m_bank;
	memory_view m_view[6];
	u8 m_selected_bank[6];
	u8 m_control;
	u32 m_kanji_latch;
};

void msx_cart_fs_sr021_device::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
	save_item(NAME(m_kanji_latch));
}

void msx_cart_fs_sr021_device::device_reset()
{
	m_control = 0;
	for (int i = 0; i < 6; i++)
		m_view[i].select(0);
	m_bank[2]->set_entry(0);
}

std::error_condition msx_cart_fs_sr021_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_fs_sr021_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x200000)
	{
		message = "msx_cart_fs_sr021_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	if (!cart_sram_region())
	{
		message = "msx_cart_fs_sr021_device: Required region 'sram' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_sram_region()->bytes() != 0x4000)
	{
		message = "msx_cart_fs_sr021_device: Region 'sram' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	for (int i = 0; i < 6; i++)
	{
		m_bank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);
		m_bank[i]->configure_entry(0x80, cart_sram_region()->base());
		m_bank[i]->configure_entry(0x81, cart_sram_region()->base() + BANK_SIZE);
	}

	for (int pg = 0; pg < 3; pg++)
	{
		const offs_t start = pg * 0x4000;
		const int index = pg * 2;
		page(pg)->install_view(start, start + 0x1fff, m_view[index]);
		m_view[index][0].install_read_bank(start, start + 0x1fff, m_bank[index]);
		m_view[index][1].install_readwrite_bank(start, start + 0x1fff, m_bank[index]);
		page(pg)->install_view(start + 0x2000, start + 0x3fff, m_view[index + 1]);
		m_view[index + 1][0].install_read_bank(start + 0x2000, start + 0x3fff, m_bank[index + 1]);
		m_view[index + 1][1].install_readwrite_bank(start + 0x2000, start + 0x3fff, m_bank[index + 1]);
	}

	m_view[3][2].install_read_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][2].install_read_handler(0x7ff0, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_r)));
	m_view[3][3].install_readwrite_bank(0x6000, 0x7fff, m_bank[3]);
	m_view[3][3].install_read_handler(0x7ff0, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_r)));

	page(1)->install_write_handler(0x6000, 0x6000, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<0>))); // 0000-1fff
	page(1)->install_write_handler(0x6400, 0x6400, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<1>))); // 2000-3fff
	page(1)->install_write_handler(0x6800, 0x6800, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<2>))); // 4000-5fff
	page(1)->install_write_handler(0x6c00, 0x6c00, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<3>))); // 6000-7fff
	page(1)->install_write_handler(0x7000, 0x7000, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<4>))); // 8000-9fff
	page(1)->install_write_handler(0x7800, 0x7800, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::bank_w<5>))); // a000-bfff
	page(1)->install_write_handler(0x7ff9, 0x7ff9, emu::rw_delegate(*this, FUNC(msx_cart_fs_sr021_device::control_w)));

	io_space().install_write_tap(0xd8, 0xd9, "kanji_w", [this] (offs_t ofs, u8 &data, u8) { this->kanji_w(ofs, data); });
	io_space().install_read_tap(0xd9, 0xd9, "kanji_r", [this] (offs_t ofs, u8 &data, u8) { data &= this->kanji_r(ofs); });

	return std::error_condition();
}

template <int Bank>
void msx_cart_fs_sr021_device::set_view()
{
	bool ram_active = (m_selected_bank[Bank] >= 0x80 && m_selected_bank[Bank] < 0x82);
	if (Bank == 3)
		m_view[Bank].select((BIT(m_control, 2) ? 2 : 0) | (ram_active ? 1 : 0));
	else
		m_view[Bank].select(ram_active ? 1 : 0);
}

template <int Bank>
void msx_cart_fs_sr021_device::bank_w(u8 data)
{
	m_selected_bank[Bank] = data;
	m_bank[Bank]->set_entry(data);
	set_view<Bank>();
}

void msx_cart_fs_sr021_device::control_w(u8 data)
{
	m_control = data;
	set_view<3>();
}

u8 msx_cart_fs_sr021_device::bank_r(offs_t offset)
{
	return m_selected_bank[offset];
}

u8 msx_cart_fs_sr021_device::kanji_r(offs_t offset)
{
	u8 result = 0xff;

	u32 latch = bitswap<17>(m_kanji_latch, 4, 3, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 2, 1, 0);
	// This might not be correct, fixes most characters on the menu screen.
	if (!(latch & 0x18000))
	{
		latch |= 0x40000;
	}
	result = cart_rom_region()->base()[0x100000 | latch];

	if (!machine().side_effects_disabled())
	{
		m_kanji_latch = (m_kanji_latch & ~0x1f) | ((m_kanji_latch + 1) & 0x1f);
	}
	return result;
}

void msx_cart_fs_sr021_device::kanji_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_kanji_latch = (m_kanji_latch & 0x007e0) | ((data & 0x3f) << 11);
	else
		m_kanji_latch = (m_kanji_latch & 0x1f800) | ((data & 0x3f) << 5);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FS_SR021, msx_cart_interface, msx_cart_fs_sr021_device, "msx_cart_fs_sr021", "MSX Cartridge - FS-SR021")
