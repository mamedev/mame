// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CD 90-015 - Floppy disk interface built from a MC6843 single-density controller
//
// Handles up to four 5.25 single-sided drives (UD 90-070). Formatted capacity is 80 KB.
// The lack of a track zero sensor is confirmed from schematics.

#include "emu.h"
#include "cd90_015.h"
#include "formats/sap_dsk.h"
#include "formats/thom_dsk.h"

DEFINE_DEVICE_TYPE(CD90_015, cd90_015_device, "cd90_015", "Thomson CD 90-015 floppy drive controller")

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
	map(0x10, 0x17).m(m_fdc, FUNC(mc6843_device::map));
	map(0x18, 0x19).rw(FUNC(cd90_015_device::motor_r), FUNC(cd90_015_device::select_w));
}

const tiny_rom_entry *cd90_015_device::device_rom_region() const
{
	return ROM_NAME(cd90_015);
}

class ud90_070_device : public floppy_image_device {
public:
	ud90_070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const char *image_interface() const noexcept override { return "floppy_5_25"; }

protected:
	virtual void setup_characteristics() override;
};

DEFINE_DEVICE_TYPE(UD90_070, ud90_070_device, "ud90_070", "Thomson UD 90-070 5.25\" single-sided disk drive")

ud90_070_device::ud90_070_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppy_image_device(mconfig, UD90_070, tag, owner, clock)
{
}

void ud90_070_device::setup_characteristics()
{
	m_form_factor = floppy_image::FF_525;
	m_tracks = 40;
	m_sides = 1;
	m_has_trk00_sensor = false;
	set_rpm(300);

	m_variants.push_back(floppy_image::SSSD);
}

void cd90_015_device::floppy_drives(device_slot_interface &device)
{
	device.option_add("ud90_070", UD90_070);
}

void cd90_015_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_THOMSON_525_FORMAT);
	fr.add(FLOPPY_SAP_FORMAT);
}

void cd90_015_device::device_add_mconfig(machine_config &config)
{
	MC6843(config, m_fdc, DERIVED_CLOCK(1, 2)); // Comes from the main board
	m_fdc->force_ready();
	FLOPPY_CONNECTOR(config, m_floppy[0], floppy_drives, "ud90_070", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], floppy_drives, nullptr,    floppy_formats).enable_sound(true);
}

TIMER_CALLBACK_MEMBER(cd90_015_device::motor_tick)
{
	m_floppy[param]->get_device()->mon_w(1);
}

void cd90_015_device::device_start()
{
	for(int i=0; i != 4; i++)
		m_motor_timer[i] = timer_alloc(FUNC(cd90_015_device::motor_tick), this);
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
				m_motor_timer[i]->adjust(attotime::from_seconds(5), i);
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
