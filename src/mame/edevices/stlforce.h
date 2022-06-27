// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "edevices.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_video(*this, "edevices_vid"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_okibank(*this, "okibank")
	{ }

	void stlforce(machine_config &config);
	void twinbrat(machine_config &config);

	void init_twinbrat();

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<edevices_device> m_video;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	optional_memory_bank m_okibank;

	void eeprom_w(uint8_t data);
	void oki_bank_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void stlforce_map(address_map &map);
	void twinbrat_oki_map(address_map &map);
};
