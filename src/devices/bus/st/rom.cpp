// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Generic ST ROM cartridge

#include "emu.h"
#include "rom.h"

namespace {

class st_rom_device : public device_t, public device_stcart_interface, public device_image_interface
{
public:
	st_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~st_rom_device();

	virtual void map(address_space_installer &space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom,stc"; }
	virtual const char *image_interface() const noexcept override { return "st_cart"; }
	virtual const char *image_type_name() const noexcept override { return "romimage"; }
	virtual const char *image_brief_type_name() const noexcept override { return "rom"; }
	virtual std::pair<std::error_condition, std::string> call_load() override;

private:
	std::vector<u16> m_romdata;
};

st_rom_device::st_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST_ROM, tag, owner, clock),
	device_stcart_interface(mconfig, *this),
	device_image_interface(mconfig, *this)
{
}

st_rom_device::~st_rom_device()
{
}

void st_rom_device::map(address_space_installer &space)
{
	if(m_romdata.empty())
		return;

	space.install_rom(0xfa0000, 0xfa0000 + m_romdata.size()*2 - 1, 0x20000 - m_romdata.size()*2, m_romdata.data());
}

void st_rom_device::device_start()
{
}

void st_rom_device::device_reset()
{
}

std::pair<std::error_condition, std::string> st_rom_device::call_load()
{
	u32 size = length();
	u32 p2;
	for(p2 = 0; size >= 1<<p2; p2++);
	u32 size2 = 1 << (p2-1);
	if((size2 != size && size2 != size-4) || size2 < 256 || size2 > 128*1024)
		return std::make_pair(std::error_condition(image_error::INVALIDLENGTH), std::string("Invalid file size"));

	if(size != size2)
		fseek(4, SEEK_SET);
	m_romdata.resize(size2/2);
	fread(m_romdata.data(), size2);

#ifdef LSB_FIRST
	for(u16 &c : m_romdata)
		c = (c >> 8) | (c << 8);
#endif
	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ST_ROM, device_stcart_interface, st_rom_device, "st_rom", "ST Rom cartridge")
