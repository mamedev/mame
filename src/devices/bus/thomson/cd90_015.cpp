// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-015 - Floppy drive selectler built from a wd1770
//
// Handles up to two 5.25 dual-sided drives (DD 90-320)

#include "emu.h"
#include "cd90_015.h"
#include "formats/thom_dsk.h"

DEFINE_DEVICE_TYPE(CD90_015, cd90_015_device, "cd90_015", "Thomson CD90-015 floppy drive selectler")

cd90_015_device::cd90_015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CD90_015, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "%u", 0U),
	m_rom(*this, "rom")
{
}

ROM_START(cd90_015)
	ROM_REGION( 0x7c0, "rom", 0 )
	ROM_LOAD ( "cd90-015.rom", 0x000, 0x7c0, CRC(821d34c1) SHA1(31a6bb81baaeec5fc8de457c97264f9dfa92c18b) )
ROM_END

void cd90_015_device::rom_map(address_map &map)
{
	map(0x000, 0x7bf).rom().region(m_rom, 0);
}

void cd90_015_device::io_map(address_map &map)
{
	map(0, 7).m(m_fdc, FUNC(mc6843_device::map));
	map(8, 9).rw(FUNC(cd90_015_device::motor_r), FUNC(cd90_015_device::select_w));
}

const tiny_rom_entry *cd90_015_device::device_rom_region() const
{
	return ROM_NAME(cd90_015);
}

void cd90_015_device::floppy_drives(device_slot_interface &device)
{
	device.option_add("dd90_015", FLOPPY_525_SD);
}

void cd90_015_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_THOMSON_525_FORMAT);
}

void cd90_015_device::device_add_mconfig(machine_config &config)
{
	MC6843(config, m_fdc, 16_MHz_XTAL / 32); // Comes from the main board
	m_fdc->force_ready();
	FLOPPY_CONNECTOR(config, m_floppy[0], floppy_drives, "dd90_015", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
}

void cd90_015_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	m_floppy[id]->get_device()->mon_w(1);
}

void cd90_015_device::device_start()
{
	for(int i=0; i != 4; i++)
		m_motor_timer[i] = timer_alloc(i);
	save_item(NAME(m_select));
}

void cd90_015_device::device_reset()
{
	m_select = 0;
	for(int i=0; i != 4; i++) {
		floppy_image_device *f = m_floppy[i]->get_device();
		if(f)
			f->mon_w(1);
	}
	m_fdc->set_floppy(nullptr);
}

void cd90_015_device::select_w(u8 data)
{
	u8 started = (~m_select) & data;
	m_select = data & 0xf;

	for(int i = 0; i != 5; i++)
		if(m_select & (1 << i)) {
			m_fdc->set_floppy(m_floppy[i]->get_device());
			goto found;
		}
	m_fdc->set_floppy(nullptr);

 found:
	for(int i = 0; i != 4; i++)
		if(started & (1 << i)) {
			if(m_floppy[i]->get_device()) {
				m_floppy[i]->get_device()->mon_w(0);
				m_motor_timer[i]->adjust(attotime::from_seconds(5));
			}
		}

	logerror("select_w %x\n", m_select);
}

u8 cd90_015_device::motor_r()
{
	u8 res = 0xf;
	for(int i=0; i != 4; i++)
		if(m_floppy[i]->get_device() && !m_floppy[i]->get_device()->mon_r())
			res &= ~(1 << i);
	if(!machine().side_effects_disabled())
		logerror("motor_r %x\n", res);
	return res;
}
