// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, R. Belmont
/****************************************************************************

    drivers/palm.c
    Palm (MC68328) emulation

    Driver by Ryan Holtz

    Additional bug fixing by R. Belmont

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "video/mc68328lcd.h"
#include "video/sed1375.h"

#include "screen.h"
#include "speaker.h"

#include "pilot1k.lh"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class palm_base_state : public driver_device
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(pen_check);

protected:
	palm_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_screen(*this, "screen")
		, m_io_penx(*this, "PENX")
		, m_io_peny(*this, "PENY")
		, m_io_penb(*this, "PENB")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	offs_t dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params);

	virtual int spi_from_hw();

	required_device<mc68328_base_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_ioport m_io_penx;
	required_ioport m_io_peny;
	required_ioport m_io_penb;

	u16 m_spim_data;
};

class palm_state : public palm_base_state
{
public:
	palm_state(const machine_config &mconfig, device_type type, const char *tag)
		: palm_base_state(mconfig, type, tag)
		, m_lcdctrl(*this, "lcdctrl")
		, m_io_portd(*this, "PORTD")
	{ }

	void palmiii(machine_config &config);
	void pilot1k(machine_config &config);
	void palmvx(machine_config &config);
	void palmv(machine_config &config);
	void palmpro(machine_config &config);
	void pilot5k(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(button_check);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void palm_base(machine_config &config);

	void mem_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void check_pen_adc_read();
	void adc_vcc_y_w(int state);
	void adc_gnd_y_w(int state);
	void adc_vcc_x_w(int state);
	void adc_gnd_x_w(int state);
	void adc_csn_w(int state);
	int power_nmi_r();

	enum : int
	{
		PORTF_Y_VCCN_BIT        = 0,
		PORTF_Y_GND_BIT         = 1,
		PORTF_X_VCCN_BIT        = 2,
		PORTF_X_GND_BIT         = 3,
		PORTF_ADC_CSN_BIT       = 7
	};

	required_device<mc68328_lcd_device> m_lcdctrl;
	required_ioport m_io_portd;

	u8 m_port_f_latch;
	bool m_adc_csn;
	bool m_adc_vcc_x;
	bool m_adc_gnd_x;
	bool m_adc_vcc_y;
	bool m_adc_gnd_y;
};

class palmez_base_state : public palm_base_state
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(button_check);

protected:
	palmez_base_state(const machine_config &mconfig, device_type type, const char *tag, const u8 hardware_id)
		: palm_base_state(mconfig, type, tag)
		, m_rows(*this, "ROW%u", 0u)
		, m_hardware_id(hardware_id)
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void palmez_base(machine_config &config);

	void mem_map(address_map &map) ATTR_COLD;

	void adc_enable_w(int state);

	template <int Line> void kbd_row_w(int state);
	template <int Line> void kbd_col_w(int state);
	template <int Line> int kbd_scan_r();

	void hardware_id_req_w(int state);

	void spi_to_hw(int state);
	virtual int spi_from_hw() override;

	required_ioport_array<3> m_rows;

	u8 m_key_row_mask;
	u8 m_key_col_mask;

	bool m_hardware_id_asserted;
	const u8 m_hardware_id;

	bool m_adc_enabled;
	u8 m_adc_cmd_bit_count;
	u8 m_adc_response_bit_count;
	u8 m_adc_cmd;
};

class palmiiic_state : public palmez_base_state
{
public:
	palmiiic_state(const machine_config &mconfig, device_type type, const char *tag)
		: palmez_base_state(mconfig, type, tag, 0x09)
		, m_sed1375(*this, "lcdctrl")
	{ }

	void palmiiic(machine_config &config);

protected:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<sed1375_device> m_sed1375;
};

class palmm100_state : public palmez_base_state
{
public:
	palmm100_state(const machine_config &mconfig, device_type type, const char *tag)
		: palmez_base_state(mconfig, type, tag, 0x05)
		, m_lcdctrl(*this, "lcdctrl")
	{ }

	void palmm100(machine_config &config);

protected:
	template <int Line> int hardware_subid_r();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<mc68328_lcd_device> m_lcdctrl;
};


/***************************************************************************
    MACHINE HARDWARE
***************************************************************************/

// Shared platform hardware

void palm_base_state::machine_start()
{
	save_item(NAME(m_spim_data));
}

void palm_base_state::machine_reset()
{
	// Copy boot ROM
	u8* bios = memregion("bios")->base();
	memset(m_ram->pointer(), 0, m_ram->size());
	memcpy(m_ram->pointer(), bios, 0x20000);

	m_spim_data = 0xffff;
}

INPUT_CHANGED_MEMBER(palm_base_state::pen_check)
{
	m_maincpu->irq5_w(m_io_penb->read() & 1);
}

int palm_base_state::spi_from_hw()
{
	int out_state = BIT(m_spim_data, 15);
	m_spim_data <<= 1;
	m_spim_data |= 1;
	return out_state;
}


// First-generation Palm hardware ("IDT")

void palm_state::machine_start()
{
	palm_base_state::machine_start();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x000000, m_ram->size() - 1, m_ram->pointer());

	//save_item(NAME(m_port_f_latch));
	save_item(NAME(m_adc_csn));
	save_item(NAME(m_adc_vcc_x));
	save_item(NAME(m_adc_gnd_x));
	save_item(NAME(m_adc_vcc_y));
	save_item(NAME(m_adc_gnd_y));
}

void palm_state::machine_reset()
{
	palm_base_state::machine_reset();

	m_adc_csn = false;
	m_adc_vcc_x = false;
	m_adc_gnd_x = false;
	m_adc_vcc_y = false;
	m_adc_gnd_y = false;
}

u32 palm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_lcdctrl->video_update(bitmap, cliprect);
	return 0;
}

INPUT_CHANGED_MEMBER(palm_state::button_check)
{
	const u8 button_state = m_io_portd->read();
	m_maincpu->port_d_in_w(BIT(button_state, param), param);
}

void palm_state::check_pen_adc_read()
{
	if (!m_adc_csn)
	{
		if (m_adc_vcc_x && m_adc_gnd_y)
		{
			m_spim_data = (0xff - m_io_penx->read()) * 2;
		}
		else if (m_adc_vcc_y && m_adc_gnd_x)
		{
			m_spim_data = (0xff - m_io_peny->read()) * 2;
		}
	}
}

void palm_state::adc_vcc_y_w(int state)
{
	m_adc_vcc_y = state;
	check_pen_adc_read();
}

void palm_state::adc_gnd_y_w(int state)
{
	m_adc_gnd_y = state;
	check_pen_adc_read();
}

void palm_state::adc_vcc_x_w(int state)
{
	m_adc_vcc_x = state;
	check_pen_adc_read();
}

void palm_state::adc_gnd_x_w(int state)
{
	m_adc_gnd_x = state;
	check_pen_adc_read();
}

void palm_state::adc_csn_w(int state)
{
	m_adc_csn = state;
	check_pen_adc_read();
}

int palm_state::power_nmi_r()
{
	return 1;
}


// Basic 68EZ328-based Palm hardware

void palmez_base_state::machine_start()
{
	palm_base_state::machine_start();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x000000, m_ram->size() - 1, m_ram->pointer());

	save_item(NAME(m_key_row_mask));
	save_item(NAME(m_key_col_mask));

	save_item(NAME(m_hardware_id_asserted));

	save_item(NAME(m_adc_enabled));
	save_item(NAME(m_adc_cmd_bit_count));
	save_item(NAME(m_adc_response_bit_count));
	save_item(NAME(m_adc_cmd));
}

void palmez_base_state::machine_reset()
{
	palm_base_state::machine_reset();

	m_key_row_mask = 0;
	m_key_col_mask = 0;

	m_hardware_id_asserted = false;

	m_adc_enabled = false;
	m_adc_cmd_bit_count = 8;
	m_adc_response_bit_count = 0;
	m_adc_cmd = 0;
}

INPUT_CHANGED_MEMBER(palmez_base_state::button_check)
{
	const u8 button_state = m_rows[param]->read();
	for (int bit = 0; bit < 4; bit++)
	{
		m_maincpu->port_d_in_w(BIT(button_state, bit), bit);
	}
}

void palmez_base_state::adc_enable_w(int state)
{
	const bool was_enabled = m_adc_enabled;
	m_adc_enabled = !state;
	if (!was_enabled && m_adc_enabled)
	{
		m_adc_cmd_bit_count = 8;
		m_spim_data = 0;
	}
}

template <int Line>
void palmez_base_state::kbd_row_w(int state)
{
	m_key_row_mask &= ~(1 << Line);
	m_key_row_mask |= !state << Line;
}

template <int Line>
void palmez_base_state::kbd_col_w(int state)
{
	m_key_col_mask &= ~(1 << Line);
	m_key_col_mask |= state << Line;
}

template <int Line>
int palmez_base_state::kbd_scan_r()
{
	if (m_hardware_id_asserted)
	{
		return BIT(~m_hardware_id, Line);
	}

	int state = 0;
	for (int i = 0; i < 3; i++)
	{
		if (BIT(m_key_row_mask, i))
		{
			state |= BIT(m_rows[i]->read(), Line);
		}
	}
	return state;
}

void palmez_base_state::spi_to_hw(int state)
{
	if (m_adc_enabled && m_adc_cmd_bit_count > 0)
	{
		m_adc_cmd_bit_count--;
		m_adc_cmd &= ~(1 << m_adc_cmd_bit_count);
		m_adc_cmd |= state << m_adc_cmd_bit_count;
	}
}

int palmez_base_state::spi_from_hw()
{
	int state = palm_base_state::spi_from_hw();
	if (m_adc_enabled && m_adc_cmd_bit_count == 0)
	{
		if (m_adc_response_bit_count == 0)
		{
			m_adc_response_bit_count = 16;
			const u8 channel = (m_adc_cmd >> 4) & 7;
			switch (channel)
			{
			case 1: // Pen Y
				m_spim_data = ((0xff - m_io_peny->read()) * 2) << 3;
				break;
			case 2: // Battery Level
				m_spim_data = 0x7ff8;
				break;
			case 5: // Pen X
				m_spim_data = ((0xff - m_io_penx->read()) * 2) << 3;
				break;
			case 6: // Dock Serial
				m_spim_data = 0;
				break;
			default:
				LOG("%s: Unknown ADC Channel: %d (command %02x)\n", machine().describe_context(), channel, m_adc_cmd);
				break;
			}
		}
		else
		{
			m_adc_response_bit_count--;
			if (m_adc_response_bit_count == 0)
			{
				m_adc_cmd_bit_count = 8;
			}
		}
	}
	return state;
}

void palmez_base_state::hardware_id_req_w(int state)
{
	m_hardware_id_asserted = !state;
}


// Palm m100 ("Brad") hardware

u32 palmm100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_lcdctrl->video_update(bitmap, cliprect);
	return 0;
}

template <int Line> int palmm100_state::hardware_subid_r()
{
	return BIT(~0x00, Line);
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void palm_state::mem_map(address_map &map)
{
	map(0xc00000, 0xe07fff).rom().region("bios", 0);
}

void palmez_base_state::mem_map(address_map &map)
{
	map(0x00c00000, 0x00e07fff).rom().region("bios", 0);
	map(0x10c00000, 0x10e07fff).rom().region("bios", 0);
}

void palmiiic_state::mem_map(address_map &map)
{
	palmez_base_state::mem_map(map);
	map(0x1f000000, 0x1f01ffff).m(m_sed1375, FUNC(sed1375_device::map));
}


/***************************************************************************
    INPUTS
***************************************************************************/

static INPUT_PORTS_START(palm_base)
	PORT_START("PENX")
	PORT_BIT(0xff, 0x50, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_MINMAX(0, 0xa0) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("PENY")
	PORT_BIT(0xff, 0x50, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_MINMAX(0, 0xa0) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START("PENB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen Button") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, palm_base_state, pen_check, 0)
INPUT_PORTS_END

static INPUT_PORTS_START(palm)
	PORT_INCLUDE(palm_base)

	PORT_START("PORTD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Power") PORT_CODE(KEYCODE_D)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Up") PORT_CODE(KEYCODE_Y)    PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Down") PORT_CODE(KEYCODE_H)    PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Button 1") PORT_CODE(KEYCODE_F)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Button 2") PORT_CODE(KEYCODE_G)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Button 3") PORT_CODE(KEYCODE_J)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Button 4") PORT_CODE(KEYCODE_K)   PORT_CHANGED_MEMBER(DEVICE_SELF, palm_state, button_check, 6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(palmiiic)
	PORT_INCLUDE(palm_base)

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Button 1") PORT_CODE(KEYCODE_F)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Button 2") PORT_CODE(KEYCODE_G)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Button 3") PORT_CODE(KEYCODE_J)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Button 4") PORT_CODE(KEYCODE_K)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 0)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Up") PORT_CODE(KEYCODE_Y)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Down") PORT_CODE(KEYCODE_H)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 1)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Power") PORT_CODE(KEYCODE_D)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_NAME("Contrast") PORT_CODE(KEYCODE_C)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmiiic_state, button_check, 2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START(palmm100)
	PORT_INCLUDE(palm_base)

	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Button 1") PORT_CODE(KEYCODE_F)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Button 2") PORT_CODE(KEYCODE_G)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Button 3") PORT_CODE(KEYCODE_J)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Button 4") PORT_CODE(KEYCODE_K)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 0)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Down") PORT_CODE(KEYCODE_H)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 1)
	PORT_BIT(0xfd, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Power") PORT_CODE(KEYCODE_D)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_NAME("Up") PORT_CODE(KEYCODE_Y)   PORT_CHANGED_MEMBER(DEVICE_SELF, palmm100_state, button_check, 2)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


/***************************************************************************
    MACHINE/DEVICE DRIVERS
***************************************************************************/

void palm_state::palm_base(machine_config &config)
{
	/* basic machine hardware */
	MC68328(config, m_maincpu, 32768*506);        /* 16.580608 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &palm_state::mem_map);
	m_maincpu->set_dasm_override(FUNC(palm_state::dasm_override));

	m_maincpu->out_port_f<PORTF_Y_VCCN_BIT>().set(FUNC(palm_state::adc_vcc_y_w));
	m_maincpu->out_port_f<PORTF_Y_GND_BIT>().set(FUNC(palm_state::adc_gnd_y_w));
	m_maincpu->out_port_f<PORTF_X_VCCN_BIT>().set(FUNC(palm_state::adc_vcc_x_w));
	m_maincpu->out_port_f<PORTF_X_GND_BIT>().set(FUNC(palm_state::adc_gnd_x_w));
	m_maincpu->out_port_f<PORTF_ADC_CSN_BIT>().set(FUNC(palm_state::adc_csn_w));

	m_maincpu->in_port_c<4>().set(FUNC(palm_state::power_nmi_r));

	m_maincpu->in_port_d<0>().set_ioport(m_io_penb).bit(0);
	m_maincpu->in_port_d<1>().set_ioport(m_io_penb).bit(1);
	m_maincpu->in_port_d<2>().set_ioport(m_io_penb).bit(2);
	m_maincpu->in_port_d<3>().set_ioport(m_io_penb).bit(3);
	m_maincpu->in_port_d<4>().set_ioport(m_io_penb).bit(4);
	m_maincpu->in_port_d<5>().set_ioport(m_io_penb).bit(5);
	m_maincpu->in_port_d<6>().set_ioport(m_io_penb).bit(6);
	m_maincpu->in_port_d<7>().set_ioport(m_io_penb).bit(7);

	m_maincpu->out_pwm().set("dac", FUNC(dac_bit_interface::write));
	m_maincpu->in_spim().set(FUNC(palm_state::spi_from_hw));

	m_maincpu->out_flm().set(m_lcdctrl, FUNC(mc68328_lcd_device::flm_w));
	m_maincpu->out_llp().set(m_lcdctrl, FUNC(mc68328_lcd_device::llp_w));
	m_maincpu->out_lsclk().set(m_lcdctrl, FUNC(mc68328_lcd_device::lsclk_w));
	m_maincpu->out_ld().set(m_lcdctrl, FUNC(mc68328_lcd_device::ld_w));
	m_maincpu->set_lcd_info_changed(m_lcdctrl, FUNC(mc68328_lcd_device::lcd_info_changed));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 220);
	m_screen->set_visarea(0, 160 - 1, 0, 220 - 1);
	m_screen->set_screen_update(FUNC(palm_state::screen_update));

	MC68328_LCD(config, m_lcdctrl, 0);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void palmiiic_state::palmiiic(machine_config &config)
{
	/* basic machine hardware */
	MC68EZ328(config, m_maincpu, 32768*506);        /* 16.580608 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &palmiiic_state::mem_map);
	m_maincpu->set_dasm_override(FUNC(palmiiic_state::dasm_override));

	m_maincpu->out_port_b<1>().set(FUNC(palmiiic_state::adc_enable_w));

	m_maincpu->out_port_c<0>().set(FUNC(palmiiic_state::kbd_row_w<0>));
	m_maincpu->out_port_c<1>().set(FUNC(palmiiic_state::kbd_row_w<1>));
	m_maincpu->out_port_c<2>().set(FUNC(palmiiic_state::kbd_row_w<2>));

	m_maincpu->out_port_d<0>().set(FUNC(palmiiic_state::kbd_col_w<0>));
	m_maincpu->out_port_d<1>().set(FUNC(palmiiic_state::kbd_col_w<1>));
	m_maincpu->out_port_d<2>().set(FUNC(palmiiic_state::kbd_col_w<2>));
	m_maincpu->out_port_d<3>().set(FUNC(palmiiic_state::kbd_col_w<3>));

	m_maincpu->out_port_g<2>().set(FUNC(palmiiic_state::hardware_id_req_w));

	m_maincpu->in_port_d<0>().set(FUNC(palmiiic_state::kbd_scan_r<0>));
	m_maincpu->in_port_d<1>().set(FUNC(palmiiic_state::kbd_scan_r<1>));
	m_maincpu->in_port_d<2>().set(FUNC(palmiiic_state::kbd_scan_r<2>));
	m_maincpu->in_port_d<3>().set(FUNC(palmiiic_state::kbd_scan_r<3>));
	m_maincpu->in_port_d<4>().set_constant(1); // Active-low, indicates hotsync/dock button press
	m_maincpu->in_port_d<6>().set_constant(1); // Active-low, indicates external adapter installed
	m_maincpu->in_port_d<7>().set_constant(1); // Active-low, indicates pending power failure

	m_maincpu->in_port_f<0>().set_constant(1); // Active-high, indicates LCD is at full power
	m_maincpu->in_port_f<6>().set_constant(1); // Active-high, indicates battery enabled
	m_maincpu->in_port_f<7>().set_constant(1); // Active-low, determines sync port attachment

	m_maincpu->out_pwm().set("dac", FUNC(dac_bit_interface::write));
	m_maincpu->in_spim().set(FUNC(palmiiic_state::spi_from_hw));
	m_maincpu->out_spim().set(FUNC(palmiiic_state::spi_to_hw));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("8M");

	/* video hardware */
	SED1375(config, m_sed1375);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 262);
	m_screen->set_visarea(0, 160-1, 0, 220-1);
	m_screen->set_screen_update(m_sed1375, FUNC(sed1375_device::screen_update));

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void palmm100_state::palmm100(machine_config &config)
{
	/* basic machine hardware */
	MC68EZ328(config, m_maincpu, 32768*506);        /* 16.580608 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &palmm100_state::mem_map);
	m_maincpu->set_dasm_override(FUNC(palmm100_state::dasm_override));

	m_maincpu->out_port_b<1>().set(FUNC(palmm100_state::kbd_row_w<0>));
	m_maincpu->out_port_b<3>().set(FUNC(palmm100_state::kbd_row_w<1>));
	m_maincpu->out_port_b<6>().set(FUNC(palmm100_state::kbd_row_w<2>));

	m_maincpu->out_port_d<0>().set(FUNC(palmm100_state::kbd_col_w<0>));
	m_maincpu->out_port_d<1>().set(FUNC(palmm100_state::kbd_col_w<1>));
	m_maincpu->out_port_d<2>().set(FUNC(palmm100_state::kbd_col_w<2>));
	m_maincpu->out_port_d<3>().set(FUNC(palmm100_state::kbd_col_w<3>));

	m_maincpu->out_port_g<2>().set(FUNC(palmm100_state::hardware_id_req_w));
	m_maincpu->out_port_g<5>().set(FUNC(palmm100_state::adc_enable_w));

	m_maincpu->in_port_d<0>().set(FUNC(palmm100_state::kbd_scan_r<0>));
	m_maincpu->in_port_d<1>().set(FUNC(palmm100_state::kbd_scan_r<1>));
	m_maincpu->in_port_d<2>().set(FUNC(palmm100_state::kbd_scan_r<2>));
	m_maincpu->in_port_d<3>().set(FUNC(palmm100_state::kbd_scan_r<3>));
	m_maincpu->in_port_d<4>().set_constant(1); // Active-low, indicates hotsync/dock button press
	m_maincpu->in_port_d<6>().set_constant(1); // Active-low, indicates external adapter installed
	m_maincpu->in_port_d<7>().set_constant(1); // Active-low, indicates pending power failure

	m_maincpu->in_port_e<0>().set(FUNC(palmm100_state::hardware_subid_r<0>));
	m_maincpu->in_port_e<1>().set(FUNC(palmm100_state::hardware_subid_r<1>));
	m_maincpu->in_port_e<2>().set(FUNC(palmm100_state::hardware_subid_r<2>));
	m_maincpu->in_port_e<3>().set(FUNC(palmm100_state::hardware_subid_r<3>));
	m_maincpu->in_port_e<4>().set(FUNC(palmm100_state::hardware_subid_r<4>));
	m_maincpu->in_port_e<5>().set(FUNC(palmm100_state::hardware_subid_r<5>));
	m_maincpu->in_port_e<6>().set(FUNC(palmm100_state::hardware_subid_r<6>));
	m_maincpu->in_port_e<7>().set(FUNC(palmm100_state::hardware_subid_r<7>));

	m_maincpu->in_port_f<0>().set_constant(1); // Active-high, indicates LCD is at full power
	m_maincpu->in_port_f<6>().set_constant(1); // Active-high, indicates battery enabled
	m_maincpu->in_port_f<7>().set_constant(1); // Active-low, determines sync port attachment

	m_maincpu->out_pwm().set("dac", FUNC(dac_bit_interface::write));
	m_maincpu->in_spim().set(FUNC(palmm100_state::spi_from_hw));
	m_maincpu->out_spim().set(FUNC(palmm100_state::spi_to_hw));

	m_maincpu->out_flm().set(m_lcdctrl, FUNC(mc68328_lcd_device::flm_w));
	m_maincpu->out_llp().set(m_lcdctrl, FUNC(mc68328_lcd_device::llp_w));
	m_maincpu->out_lsclk().set(m_lcdctrl, FUNC(mc68328_lcd_device::lsclk_w));
	m_maincpu->out_ld().set(m_lcdctrl, FUNC(mc68328_lcd_device::ld_w));
	m_maincpu->set_lcd_info_changed(m_lcdctrl, FUNC(mc68328_lcd_device::lcd_info_changed));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("2M");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 220);
	m_screen->set_visarea(0, 160 - 1, 0, 220 - 1);
	m_screen->set_screen_update(FUNC(palmm100_state::screen_update));

	MC68328_LCD(config, m_lcdctrl, 0);

	/* audio hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void palm_state::pilot1k(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("512K,1M,2M,4M,8M");

	config.set_default_layout(layout_pilot1k);
}

void palm_state::pilot5k(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("512K").set_extra_options("1M,2M,4M,8M");
}

void palm_state::palmpro(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("1M").set_extra_options("2M,4M,8M");
}

void palm_state::palmiii(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("2M").set_extra_options("4M,8M");
}

void palm_state::palmv(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("2M").set_extra_options("4M,8M");
}

void palm_state::palmvx(machine_config &config)
{
	palm_base(config);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("8M");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

#define PALM_68328_BIOS \
	ROM_REGION16_BE( 0x208000, "bios", 0 )  \
	ROM_SYSTEM_BIOS( 0, "1.0e", "Palm OS 1.0 (English)" )   \
	ROMX_LOAD( "palmos10-en.rom", 0x000000, 0x080000, CRC(82030062) SHA1(00d85c6a0588133cc4651555e9605a61fc1901fc), ROM_GROUPWORD | ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "2.0eper", "Palm OS 2.0 Personal (English)" ) \
	ROMX_LOAD( "palmos20-en-pers.rom", 0x000000, 0x100000, CRC(40ea8baa) SHA1(8e26e213de42da1317c375fb1f394bb945b9d178), ROM_GROUPWORD | ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS( 2, "2.0epro", "Palm OS 2.0 Professional (English)" ) \
	ROMX_LOAD( "palmos20-en-pro.rom", 0x000000, 0x100000, CRC(baa5b36a) SHA1(535bd9548365d300f85f514f318460443a021476), ROM_GROUPWORD | ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS( 3, "2.0eprod", "Palm OS 2.0 Professional (English, Debug)" ) \
	ROMX_LOAD( "palmis20-en-pro-dbg.rom", 0x000000, 0x100000, CRC(0d1d3a3b) SHA1(f18a80baa306d4d46b490589ee9a2a5091f6081c), ROM_GROUPWORD | ROM_BIOS(3) ) \
	ROM_SYSTEM_BIOS( 4, "3.0e", "Palm OS 3.0 (English)" ) \
	ROMX_LOAD( "palmos30-en.rom", 0x008000, 0x200000, CRC(6f461f3d) SHA1(7fbf592b4dc8c222be510f6cfda21d48ebe22413), ROM_GROUPWORD | ROM_BIOS(4) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 5, "3.0ed", "Palm OS 3.0 (English, Debug)" ) \
	ROMX_LOAD( "palmos30-en-dbg.rom", 0x008000, 0x200000, CRC(4deda226) SHA1(1c67d6fee2b6a4acd51cda6ef3490305730357ad), ROM_GROUPWORD | ROM_BIOS(5) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 6, "3.0g", "Palm OS 3.0 (German)" ) \
	ROMX_LOAD( "palmos30-de.rom", 0x008000, 0x200000, CRC(b991d6c3) SHA1(73e7539517b0d931e9fa99d6f6914ad46fb857b4), ROM_GROUPWORD | ROM_BIOS(6) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 7, "3.0f", "Palm OS 3.0 (French)" ) \
	ROMX_LOAD( "palmos30-fr.rom", 0x008000, 0x200000, CRC(a2a9ff6c) SHA1(7cb119f896017e76e4680510bee96207d9d28e44), ROM_GROUPWORD | ROM_BIOS(7) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 8, "3.0s", "Palm OS 3.0 (Spanish)" ) \
	ROMX_LOAD( "palmos30-sp.rom", 0x008000, 0x200000, CRC(63a595be) SHA1(f6e03a2fedf0cbe6228613f50f8e8717e797877d), ROM_GROUPWORD | ROM_BIOS(8) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 9, "3.3e", "Palm OS 3.3 (English)" ) \
	ROMX_LOAD( "palmos33-en-iii.rom", 0x008000, 0x200000, CRC(1eae0253) SHA1(e4626f1d33eca8368284d906b2152dcd28b71bbd), ROM_GROUPWORD | ROM_BIOS(9) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 10, "3.3f", "Palm OS 3.3 (French)" ) \
	ROMX_LOAD( "palmos33-fr-iii.rom", 0x008000, 0x200000, CRC(d7894f5f) SHA1(c7c90df814d4f97958194e0bc28c595e967a4529), ROM_GROUPWORD | ROM_BIOS(10) ) \
	ROM_RELOAD(0x000000, 0x004000)  \
	ROM_SYSTEM_BIOS( 11, "3.3g", "Palm OS 3.3 (German)" ) \
	ROMX_LOAD( "palmos33-de-iii.rom", 0x008000, 0x200000, CRC(a5a99c45) SHA1(209b0154942dab80b56d5e6e68fa20b9eb75f5fe), ROM_GROUPWORD | ROM_BIOS(11) ) \
	ROM_RELOAD(0x000000, 0x004000)

ROM_START( pilot1k )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "1.0e" )
ROM_END

ROM_START( pilot5k )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "1.0e" )
ROM_END

ROM_START( palmpers )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "2.0eper" )
ROM_END

ROM_START( palmpro )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "2.0epro" )
ROM_END

ROM_START( palmiii )
	PALM_68328_BIOS
	ROM_DEFAULT_BIOS( "3.0e" )
ROM_END

ROM_START( palmv )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.1e", "Palm OS 3.1 (English)" )
	ROMX_LOAD( "palmv31-en.rom", 0x008000, 0x200000, CRC(4656b2ae) SHA1(ec66a93441fbccfd8e0c946baa5d79c478c83e85), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "3.1g", "Palm OS 3.1 (German)" )
	ROMX_LOAD( "palmv31-de.rom", 0x008000, 0x200000, CRC(a9631dcf) SHA1(63b44d4d3fc2f2196c96d3b9b95da526df0fac77), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "3.1f", "Palm OS 3.1 (French)" )
	ROMX_LOAD( "palmv31-fr.rom", 0x008000, 0x200000, CRC(0d933a1c) SHA1(d0454f1159705d0886f8a68e1b8a5e96d2ca48f6), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 3, "3.1s", "Palm OS 3.1 (Spanish)" )
	ROMX_LOAD( "palmv31-sp.rom", 0x008000, 0x200000, CRC(cc46ca1f) SHA1(93bc78ca84d34916d7e122b745adec1068230fcd), ROM_GROUPWORD | ROM_BIOS(3) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 4, "3.1j", "Palm OS 3.1 (Japanese)" )
	ROMX_LOAD( "palmv31-jp.rom", 0x008000, 0x200000, CRC(c786db12) SHA1(4975ff2af76892370c5d4d7d6fa87a84480e79d6), ROM_GROUPWORD | ROM_BIOS(4) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 5, "3.1e2", "Palm OS 3.1 (English) v2" )
	ROMX_LOAD( "palmv31-en-2.rom", 0x008000, 0x200000, CRC(caced2bd) SHA1(95970080601f72a77a4c338203ed8809fab17abf), ROM_GROUPWORD | ROM_BIOS(5) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.1e2" )
ROM_END

ROM_START( palmvx )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.3e", "Palm OS 3.3 (English)" )
	ROMX_LOAD( "palmvx33-en.rom", 0x000000, 0x200000, CRC(3fc0cc6d) SHA1(6873d5fa99ac372f9587c769940c9b3ac1745a0a), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmvx40-en.rom", 0x000000, 0x200000, CRC(488e4638) SHA1(10a10fc8617743ebd5df19c1e99ca040ac1da4f5), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmvx41-en.rom", 0x000000, 0x200000, CRC(e59f4dff) SHA1(5e3000db318eeb8cd1f4d9729d0c9ebca560fa4a), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_DEFAULT_BIOS( "4.1e" )
ROM_END

ROM_START( palmiiic )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.5eb", "Palm OS 3.5 (English) beta" )
	ROMX_LOAD( "palmiiic350-en-beta.rom", 0x008000, 0x200000, CRC(d58521a4) SHA1(508742ea1e078737666abd4283cf5e6985401c9e), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "3.5c", "Palm OS 3.5 (Chinese)" )
	ROMX_LOAD( "palmiiic350-ch.rom", 0x008000, 0x200000, CRC(a9779f3a) SHA1(1541102cd5234665233072afe8f0e052134a5334), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmiiic40-en.rom", 0x008000, 0x200000, CRC(6b2a5ad2) SHA1(54321dcaedcc80de57a819cfd599d8d1b2e26eeb), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.0e" )
ROM_END

ROM_START( palmm100 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.51e", "Palm OS 3.5.1 (English)" )
	ROMX_LOAD( "palmm100-351-en.rom", 0x008000, 0x200000, CRC(ae8dda60) SHA1(c46248d6f05cb2f4337985610cedfbdc12ac47cf), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.51e" )
ROM_END

ROM_START( palmm130 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmm130-40-en.rom", 0x008000, 0x400000, CRC(58046b7e) SHA1(986057010d62d5881fba4dede2aba0d4d5008b16), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.0e" )
ROM_END

ROM_START( palmm505 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.0e", "Palm OS 4.0 (English)" )
	ROMX_LOAD( "palmos40-en-m505.rom", 0x008000, 0x400000, CRC(822a4679) SHA1(a4f5e9f7edb1926647ea07969200c5c5e1521bdf), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmos41-en-m505.rom", 0x008000, 0x400000, CRC(d248202a) SHA1(65e1bd08b244c589b4cd10fe573e0376aba90e5f), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.1e" )
ROM_END

ROM_START( palmm515 )
	ROM_REGION16_BE( 0x408000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.1e", "Palm OS 4.1 (English)" )
	ROMX_LOAD( "palmos41-en-m515.rom", 0x008000, 0x400000, CRC(6e143436) SHA1(a0767ea26cc493a3f687525d173903fef89f1acb), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "4.1e" )
ROM_END

ROM_START( visor )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "3.52e", "Palm OS 3.5.2 (English)" )
	ROMX_LOAD( "visor-352-en.rom", 0x008000, 0x200000, CRC(c9e55271) SHA1(749e9142f4480114c5e0d7f21ea354df7273ac5b), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_DEFAULT_BIOS( "3.52e" )
ROM_END

ROM_START( spt1500 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "4.1pim", "Version 4.1 (pim)" )
	ROMX_LOAD( "spt1500v41-pim.rom",      0x008000, 0x200000, CRC(29e50eaf) SHA1(3e920887bdf74f8f83935977b02f22d5217723eb), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "4.1pimn", "Version 4.1 (pimnoft)" )
	ROMX_LOAD( "spt1500v41-pimnoft.rom",  0x008000, 0x200000, CRC(4b44f284) SHA1(4412e946444706628b94d2303b02580817e1d370), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "4.1nopimn", "Version 4.1 (nopimnoft)" )
	ROMX_LOAD( "spt1500v41-nopimnoft.rom",0x008000, 0x200000, CRC(4ba19190) SHA1(d713c1390b82eb4e5fbb39aa10433757c5c49e02), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

ROM_START( spt1700 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "1.03pim", "Version 1.03 (pim)" )
	ROMX_LOAD( "spt1700v103-pim.rom",    0x008000, 0x200000, CRC(9df4ee50) SHA1(243a19796f15219cbd73e116f7dfb236b3d238cd), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

ROM_START( spt1740 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "1.03pim", "Version 1.03 (pim)" )
	ROMX_LOAD( "spt1740v103-pim.rom",    0x008000, 0x200000, CRC(c29f341c) SHA1(b56d7f8a0c15b1105972e24ed52c846b5e27b195), ROM_GROUPWORD | ROM_BIOS(0) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 1, "1.03pimn", "Version 1.03 (pimnoft)" )
	ROMX_LOAD( "spt1740v103-pimnoft.rom", 0x008000, 0x200000, CRC(b2d49d5c) SHA1(c133dc021b6797cdb93b666c5b315b00b5bb0917), ROM_GROUPWORD | ROM_BIOS(1) )
	ROM_RELOAD(0x000000, 0x004000)
	ROM_SYSTEM_BIOS( 2, "1.03nopim", "Version 1.03 (nopim)" )
	ROMX_LOAD( "spt1740v103-nopim.rom",   0x008000, 0x200000, CRC(8ea7e652) SHA1(2a4b5d6a426e627b3cb82c47109cfe2497eba29a), ROM_GROUPWORD | ROM_BIOS(2) )
	ROM_RELOAD(0x000000, 0x004000)
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY          FULLNAME               FLAGS
COMP( 1996, pilot1k,  0,       0,      pilot1k,  palm,     palm_state,     empty_init, "U.S. Robotics", "Pilot 1000",          MACHINE_SUPPORTS_SAVE )
COMP( 1996, pilot5k,  pilot1k, 0,      pilot5k,  palm,     palm_state,     empty_init, "U.S. Robotics", "Pilot 5000",          MACHINE_SUPPORTS_SAVE )
COMP( 1997, palmpers, pilot1k, 0,      pilot5k,  palm,     palm_state,     empty_init, "U.S. Robotics", "Palm Pilot Personal", MACHINE_SUPPORTS_SAVE )
COMP( 1997, palmpro,  pilot1k, 0,      palmpro,  palm,     palm_state,     empty_init, "U.S. Robotics", "Palm Pilot Pro",      MACHINE_SUPPORTS_SAVE )
COMP( 1998, palmiii,  pilot1k, 0,      palmiii,  palm,     palm_state,     empty_init, "3Com",          "Palm III",            MACHINE_SUPPORTS_SAVE )
COMP( 1998, palmiiic, pilot1k, 0,      palmiiic, palmiiic, palmiiic_state, empty_init, "3Com",          "Palm IIIc",           MACHINE_SUPPORTS_SAVE )
COMP( 2000, palmm100, pilot1k, 0,      palmm100, palmm100, palmm100_state, empty_init, "3Com",          "Palm m100",           MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
COMP( 2000, palmm130, pilot1k, 0,      palmiii,  palm,     palm_state,     empty_init, "3Com",          "Palm m130",           MACHINE_NOT_WORKING )
COMP( 2001, palmm505, pilot1k, 0,      palmiii,  palm,     palm_state,     empty_init, "3Com",          "Palm m505",           MACHINE_NOT_WORKING )
COMP( 2001, palmm515, pilot1k, 0,      palmiii,  palm,     palm_state,     empty_init, "3Com",          "Palm m515",           MACHINE_NOT_WORKING )
COMP( 1999, palmv,    pilot1k, 0,      palmv,    palm,     palm_state,     empty_init, "3Com",          "Palm V",              MACHINE_NOT_WORKING )
COMP( 1999, palmvx,   pilot1k, 0,      palmvx,   palm,     palm_state,     empty_init, "3Com",          "Palm Vx",             MACHINE_NOT_WORKING )
COMP( 2001, visor,    pilot1k, 0,      palmvx,   palm,     palm_state,     empty_init, "Handspring",    "Visor Edge",          MACHINE_NOT_WORKING )
COMP( 19??, spt1500,  pilot1k, 0,      palmvx,   palm,     palm_state,     empty_init, "Symbol",        "SPT 1500",            MACHINE_NOT_WORKING )
COMP( 19??, spt1700,  pilot1k, 0,      palmvx,   palm,     palm_state,     empty_init, "Symbol",        "SPT 1700",            MACHINE_NOT_WORKING )
COMP( 19??, spt1740,  pilot1k, 0,      palmvx,   palm,     palm_state,     empty_init, "Symbol",        "SPT 1740",            MACHINE_NOT_WORKING )

#include "palm_dbg.ipp"
