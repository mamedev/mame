// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

        NCR Decision Mate V

        04/01/2012 Skeleton driver.

****************************************************************************/


#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/am9517a.h"
#include "dmv_keyb.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "sound/spkrdev.h"
#include "video/upd7220.h"
#include "emupal.h"

// expansion slots
#include "bus/dmv/dmvbus.h"
#include "bus/dmv/k012.h"
#include "bus/dmv/k210.h"
#include "bus/dmv/k220.h"
#include "bus/dmv/k230.h"
#include "bus/dmv/k233.h"
#include "bus/dmv/k801.h"
#include "bus/dmv/k803.h"
#include "bus/dmv/k806.h"
#include "bus/dmv/ram.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/dmv_dsk.h"
#include "imagedev/snapquik.h"

#include "dmv.lh"


namespace {

class dmv_state : public driver_device
{
public:
	dmv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_hgdc(*this, "upd7220")
		, m_dmac(*this, "dma8237")
		, m_pit(*this, "pit8253")
		, m_fdc(*this, "i8272")
		, m_floppy0(*this, "i8272:0")
		, m_floppy1(*this, "i8272:1")
		, m_keyboard(*this, "keyboard")
		, m_speaker(*this, "speaker")
		, m_video_ram(*this, "video_ram")
		, m_palette(*this, "palette")
		, m_ram(*this, "ram")
		, m_bootrom(*this, "boot")
		, m_chargen(*this, "chargen")
		, m_slot1(*this, "slot1")
		, m_slot2(*this, "slot2")
		, m_slot2a(*this, "slot2a")
		, m_slot3(*this, "slot3")
		, m_slot4(*this, "slot4")
		, m_slot5(*this, "slot5")
		, m_slot6(*this, "slot6")
		, m_slot7(*this, "slot7")
		, m_slot7a(*this, "slot7a")
		, m_leds(*this, "led%u", 1U)
	{ }

	void dmv(machine_config &config);

private:
	void update_halt_line();

	void leds_w(uint8_t data);
	void dma_hrq_changed(int state);
	void dmac_eop(int state);
	void dmac_dack3(int state);
	void fdc_irq(int state);
	void pit_out0(int state);
	void timint_w(int state);
	void fdd_motor_w(uint8_t data);
	uint8_t sys_status_r();
	void tc_set_w(uint8_t data);
	void switch16_w(uint8_t data);
	uint8_t ramsel_r();
	uint8_t romsel_r();
	void ramsel_w(uint8_t data);
	void romsel_w(uint8_t data);
	uint8_t kb_mcu_port1_r();
	void kb_mcu_port1_w(uint8_t data);
	void kb_mcu_port2_w(uint8_t data);
	void rambank_w(offs_t offset, uint8_t data);
	uint8_t program_r(offs_t offset);
	void program_w(offs_t offset, uint8_t data);
	uint8_t exp_program_r(offs_t offset);
	void exp_program_w(offs_t offset, uint8_t data);
	void thold7_w(int state);

	void update_busint(int slot, int state);
	void busint2_w(int state)    { update_busint(0, state); }
	void busint2a_w(int state)   { update_busint(1, state); }
	void busint3_w(int state)    { update_busint(2, state); }
	void busint4_w(int state)    { update_busint(3, state); }
	void busint5_w(int state)    { update_busint(4, state); }
	void busint6_w(int state)    { update_busint(5, state); }
	void busint7_w(int state)    { update_busint(6, state); }
	void busint7a_w(int state)   { update_busint(7, state); }

	void update_irqs(int slot, int state);
	void irq2_w(int state)       { update_irqs(0, state); }
	void irq2a_w(int state)      { update_irqs(1, state); }
	void irq3_w(int state)       { update_irqs(2, state); }
	void irq4_w(int state)       { update_irqs(3, state); }
	void irq5_w(int state)       { update_irqs(4, state); }
	void irq6_w(int state)       { update_irqs(5, state); }
	void irq7_w(int state)       { update_irqs(6, state); }
	void irq7a_w(int state)      { update_irqs(7, state); }

	static void floppy_formats(format_registration &fr);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	uint8_t program_read(int cas, offs_t offset);
	void program_write(int cas, offs_t offset, uint8_t data);

	void ifsel_r(int ifsel, offs_t offset, uint8_t &data);
	void ifsel_w(int ifsel, offs_t offset, uint8_t data);
	uint8_t ifsel0_r(offs_t offset)  { uint8_t data = 0xff;   ifsel_r(0, offset, data);   return data; }
	uint8_t ifsel1_r(offs_t offset)  { uint8_t data = 0xff;   ifsel_r(1, offset, data);   return data; }
	uint8_t ifsel2_r(offs_t offset)  { uint8_t data = 0xff;   ifsel_r(2, offset, data);   return data; }
	uint8_t ifsel3_r(offs_t offset)  { uint8_t data = 0xff;   ifsel_r(3, offset, data);   return data; }
	uint8_t ifsel4_r(offs_t offset)  { uint8_t data = 0xff;   ifsel_r(4, offset, data);   return data; }
	void ifsel0_w(offs_t offset, uint8_t data) { ifsel_w(0, offset, data); }
	void ifsel1_w(offs_t offset, uint8_t data) { ifsel_w(1, offset, data); }
	void ifsel2_w(offs_t offset, uint8_t data) { ifsel_w(2, offset, data); }
	void ifsel3_w(offs_t offset, uint8_t data) { ifsel_w(3, offset, data); }
	void ifsel4_w(offs_t offset, uint8_t data) { ifsel_w(4, offset, data); }

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

	void dmv_io(address_map &map) ATTR_COLD;
	void dmv_mem(address_map &map) ATTR_COLD;
	void upd7220_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd7220_device> m_hgdc;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<dmv_keyboard_device> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint16_t> m_video_ram;
	required_device<palette_device> m_palette;
	required_memory_region m_ram;
	required_memory_region m_bootrom;
	required_memory_region m_chargen;

	required_device<dmvcart_slot_device> m_slot1;
	required_device<dmvcart_slot_device> m_slot2;
	required_device<dmvcart_slot_device> m_slot2a;
	required_device<dmvcart_slot_device> m_slot3;
	required_device<dmvcart_slot_device> m_slot4;
	required_device<dmvcart_slot_device> m_slot5;
	required_device<dmvcart_slot_device> m_slot6;
	required_device<dmvcart_slot_device> m_slot7;
	required_device<dmvcart_slot_device> m_slot7a;
	output_finder<8> m_leds;

	bool        m_ramoutdis;
	int         m_switch16;
	int         m_thold7;
	int         m_dma_hrq;
	int         m_ram_bank;
	bool        m_color_mode;
	int         m_eop_line;
	int         m_dack3_line;
	int         m_sd_poll_state;
	int         m_floppy_motor;
	int         m_busint[8];
	int         m_irqs[8];
};

void dmv_state::tc_set_w(uint8_t data)
{
	m_fdc->tc_w(true);
}

void dmv_state::switch16_w(uint8_t data)
{
	m_switch16 = !m_switch16;
	update_halt_line();
}

void dmv_state::leds_w(uint8_t data)
{
	/*
	    LEDs    Value       Significance
	    ---------------------------------------
	    None    0xFF        Check complete
	    1+8     0x7E        Sumcheck error
	    2+8     0xBE        GDC error
	    3+8     0xDE        Disk drive error
	    4+8     0xEE        16-bit processor error
	    5+8     0xF6        Keyboard error
	    6+8     0xFA        DMA error
	    7+8     0xFC        Memory error
	    All     0x00        Processor error
	*/

	for(int i=0; i<8; i++)
		m_leds[7-i] = BIT(data, i);
}

uint8_t dmv_state::ramsel_r()
{
	m_ramoutdis = false;
	return 0;
}

uint8_t dmv_state::romsel_r()
{
	m_ramoutdis = true;
	return 0;
}

void dmv_state::ramsel_w(uint8_t data)
{
	m_ramoutdis = false;
}

void dmv_state::romsel_w(uint8_t data)
{
	m_ramoutdis = true;
}

void dmv_state::rambank_w(offs_t offset, uint8_t data)
{
	m_ram_bank = offset;
}

void dmv_state::fdd_motor_w(uint8_t data)
{
	m_pit->write_gate0(1);
	m_pit->write_gate0(0);

	m_floppy_motor = 0;
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(m_floppy_motor);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(m_floppy_motor);
}

uint8_t dmv_state::sys_status_r()
{
	/*
	    Main system status
	    x--- ---- FDD index
	    -x--- --- IRQ 2
	    --x--- -- IRQ 3
	    ---x--- - IRQ 4
	    ---- x--- FDC interrupt
	    ---- -x-- FDD ready
	    ---- --x- 16-bit CPU available (active low)
	    ---- ---x FDD motor (active low)
	*/
	uint8_t data = 0x00;

	if (m_floppy_motor)
		data |= 0x01;

	// 16-bit CPU
	if (!(m_slot7->av16bit() || m_slot7a->av16bit()))
		data |= 0x02;

	if (m_floppy0->get_device() && !m_floppy0->get_device()->ready_r())
		data |= 0x04;

	if (m_fdc->get_irq())
		data |= 0x08;

	if (m_irqs[3])
		data |= 0x10;   // IRQ 4

	if (m_irqs[2])
		data |= 0x20;   // IRQ 3

	if (m_irqs[0])
		data |= 0x40;   // IRQ 2

	return data;
}

UPD7220_DISPLAY_PIXELS_MEMBER( dmv_state::hgdc_display_pixels )
{
	if (m_color_mode)
	{
		// 96KB videoram (32KB green + 32KB red + 32KB blue)
		uint16_t green = m_video_ram[(0x00000 + (address & 0x3fff))];
		uint16_t red   = m_video_ram[(0x04000 + (address & 0x3fff))];
		uint16_t blue  = m_video_ram[(0x08000 + (address & 0x3fff))];

		for(int xi=0; xi<16; xi++)
		{
			int r = BIT(red,   xi) ? 255 : 0;
			int g = BIT(green, xi) ? 255 : 0;
			int b = BIT(blue,  xi) ? 255 : 0;

			if (bitmap.cliprect().contains(x + xi, y))
				bitmap.pix(y, x + xi) = rgb_t(r, g, b);
		}
	}
	else
	{
		rgb_t const *const palette = m_palette->palette()->entry_list_raw();

		// 32KB videoram
		uint16_t gfx = m_video_ram[(address & 0x3fff)];

		for(int xi=0;xi<16;xi++)
		{
			if (bitmap.cliprect().contains(x + xi, y))
				bitmap.pix(y, x + xi) = ((gfx >> xi) & 1) ? palette[2] : palette[0];
		}
	}
}

UPD7220_DRAW_TEXT_LINE_MEMBER( dmv_state::hgdc_draw_text )
{
	for( int x = 0; x < pitch; x++ )
	{
		uint8_t tile = m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] & 0xff;
		uint8_t attr = m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] >> 8;

		rgb_t bg, fg;
		if (m_color_mode)
		{
			bg = rgb_t(attr & 0x20 ? 0 : 255, attr & 0x40 ? 0 : 255, attr & 0x80 ? 0 : 255);
			fg = rgb_t(attr & 0x04 ? 255 : 0, attr & 0x08 ? 255 : 0, attr & 0x10 ? 255 : 0);
		}
		else
		{
			const rgb_t *palette = m_palette->palette()->entry_list_raw();
			bg = palette[(attr & 1) ? 2 : 0];
			fg = palette[(attr & 1) ? 0 : 2];
		}

		for( int yi = 0; yi < lr; yi++)
		{
			uint8_t tile_data = m_chargen->base()[(tile*16+yi) & 0x7ff];

			if((attr & 2) && (m_screen->frame_number() & 0x10)) // FIXME: blink freq
				tile_data = 0;

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			for( int xi = 0; xi < 8; xi++)
			{
				int pen = (tile_data >> xi) & 1 ? 1 : 0;

				int res_x = x * 8 + xi;
				int res_y = y + yi;

				if(!m_screen->visible_area().contains(res_x, res_y))
					continue;

				if(yi >= 16) { pen = 0; }

				bitmap.pix(res_y, res_x) = pen ? fg : bg;
			}
		}
	}
}

/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER(dmv_state::quickload_cb)
{
	// Avoid loading a program if CP/M-80 is not in memory
	if ((m_ram->base()[0] != 0xc3) || (m_ram->base()[5] != 0xc3))
		return std::make_pair(image_error::UNSUPPORTED, "CP/M must already be running");

	const int mem_avail = 256 * m_ram->base()[7] + m_ram->base()[6] - 512;
	if (mem_avail < image.length())
		return std::make_pair(image_error::UNSPECIFIED, "Insufficient memory available");

	// Load image to the TPA (Transient Program Area)
	uint16_t quickload_size = image.length();
	for (uint16_t i = 0; i < quickload_size; i++)
	{
		uint8_t data;
		if (image.fread(&data, 1) != 1)
			return std::make_pair(image_error::UNSPECIFIED, "Problem reading the image at offset " + std::to_string(i));
		m_ram->base()[i + 0x100] = data;
	}

	m_ram->base()[0x80] = m_ram->base()[0x81] = 0; // clear out command tail

	m_maincpu->set_state_int(Z80_SP, mem_avail + 384); // put the stack a bit before BDOS
	m_maincpu->set_pc(0x100); // start program

	return std::make_pair(std::error_condition(), std::string());
}

static void dmv_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}


void dmv_state::ifsel_r(int ifsel, offs_t offset, uint8_t &data)
{
	for (auto &slot : { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a })
		slot->io_read(ifsel, offset, data);
}

void dmv_state::ifsel_w(int ifsel, offs_t offset, uint8_t data)
{
	for(auto &slot : { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a })
		slot->io_write(ifsel, offset, data);
}

void dmv_state::exp_program_w(offs_t offset, uint8_t data)
{
	program_write((offset >> 16) & 0x07, offset, data);
}

uint8_t dmv_state::exp_program_r(offs_t offset)
{
	return program_read((offset >> 16) & 0x07, offset);
}

void dmv_state::program_w(offs_t offset, uint8_t data)
{
	program_write(m_ram_bank, offset, data);
}

uint8_t dmv_state::program_r(offs_t offset)
{
	return program_read(m_ram_bank, offset);
}

void dmv_state::thold7_w(int state)
{
	if (m_thold7 != state)
	{
		m_thold7 = state;
		update_halt_line();
	}
}

void dmv_state::update_busint(int slot, int state)
{
	m_busint[slot] = state;

	int new_state = CLEAR_LINE;
	for (auto & elem : m_busint)
		if (elem != CLEAR_LINE)
		{
			new_state = ASSERT_LINE;
			break;
		}

	m_slot7a->busint_w(new_state);
	m_slot7->busint_w(new_state);
	m_maincpu->set_input_line(0, new_state);
}

void dmv_state::update_irqs(int slot, int state)
{
	m_irqs[slot] = state;

	switch(slot)
	{
	case 0: // slot 2
		m_slot7->irq2_w(state);
		m_slot7a->irq2_w(state);
		break;
	case 1: // slot 2a
		m_slot7->irq2a_w(state);
		m_slot7a->irq2a_w(state);
		break;
	case 2: // slot 3
		m_slot7->irq3_w(state);
		m_slot7a->irq3_w(state);
		break;
	case 3: // slot 4
		m_slot7->irq4_w(state);
		m_slot7a->irq4_w(state);
		break;
	case 4: // slot 5
		m_slot7->irq5_w(state);
		m_slot7a->irq5_w(state);
		break;
	case 5: // slot 6
		m_slot7->irq6_w(state);
		m_slot7a->irq6_w(state);
		break;
	}
}

void dmv_state::program_write(int cas, offs_t offset, uint8_t data)
{
	bool tramd = false;
	dmvcart_slot_device *slots[] = { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a };
	for(int i=0; i<8 && !tramd; i++)
		tramd = slots[i]->write(offset, data);

	if (!tramd)
	{
		if (cas == 0)
			m_ram->base()[offset & 0xffff] = data;
		else
			m_slot1->ram_write(cas, offset & 0xffff, data);
	}
}

uint8_t dmv_state::program_read(int cas, offs_t offset)
{
	uint8_t data = 0xff;
	if (m_ramoutdis && offset < 0x2000)
	{
		data = m_bootrom->base()[offset];
	}
	else
	{
		bool tramd = false;
		dmvcart_slot_device *slots[] = { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a };
		for(int i=0; i<8 && !tramd; i++)
			tramd = slots[i]->read(offset, data);

		if (!tramd)
		{
			if (cas == 0)
				data = m_ram->base()[offset & 0xffff];
			else
				m_slot1->ram_read(cas, offset & 0xffff, data);
		}
	}

	return data;
}

void dmv_state::dmv_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(dmv_state::program_r), FUNC(dmv_state::program_w));
}

void dmv_state::dmv_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(dmv_state::leds_w));
	map(0x10, 0x10).rw(FUNC(dmv_state::ramsel_r), FUNC(dmv_state::ramsel_w));
	map(0x11, 0x11).rw(FUNC(dmv_state::romsel_r), FUNC(dmv_state::romsel_w));
	map(0x12, 0x12).w(FUNC(dmv_state::tc_set_w));
	map(0x13, 0x13).r(FUNC(dmv_state::sys_status_r));
	map(0x14, 0x14).w(FUNC(dmv_state::fdd_motor_w));
	map(0x20, 0x2f).rw(m_dmac, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x40, 0x41).rw("kb_ctrl_mcu", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0x50, 0x51).m(m_fdc, FUNC(i8272a_device::map));
	map(0x80, 0x83).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xa0, 0xa1).rw(m_hgdc, FUNC(upd7220_device::read), FUNC(upd7220_device::write));
	map(0xd0, 0xd7).w(FUNC(dmv_state::switch16_w));
	map(0xe0, 0xe7).w(FUNC(dmv_state::rambank_w));

	map(0x60, 0x6f).rw(FUNC(dmv_state::ifsel0_r), FUNC(dmv_state::ifsel0_w));
	map(0x70, 0x7f).rw(FUNC(dmv_state::ifsel1_r), FUNC(dmv_state::ifsel1_w));
	map(0x30, 0x3f).rw(FUNC(dmv_state::ifsel2_r), FUNC(dmv_state::ifsel2_w));
	map(0xb0, 0xbf).rw(FUNC(dmv_state::ifsel3_r), FUNC(dmv_state::ifsel3_w));
	map(0xc0, 0xcf).rw(FUNC(dmv_state::ifsel4_r), FUNC(dmv_state::ifsel4_w));
}

uint8_t dmv_state::kb_mcu_port1_r()
{
	return !(m_keyboard->sd_poll_r() & !m_sd_poll_state);
}

void dmv_state::kb_mcu_port1_w(uint8_t data)
{
	m_sd_poll_state = BIT(data, 1);
	m_keyboard->sd_poll_w(!m_sd_poll_state);
}

void dmv_state::kb_mcu_port2_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 0));
	m_slot7a->keyint_w(BIT(data, 4));
	m_slot7->keyint_w(BIT(data, 4));
}

void dmv_state::upd7220_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0xffff).ram().share("video_ram");
}

/* Input ports */
INPUT_PORTS_START( dmv )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Video Board" )
	PORT_CONFSETTING( 0x00, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
INPUT_PORTS_END

void dmv_state::machine_start()
{
	m_leds.resolve();

	// register for state saving
	save_item(NAME(m_ramoutdis));
	save_item(NAME(m_switch16));
	save_item(NAME(m_thold7));
	save_item(NAME(m_dma_hrq));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_color_mode));
	save_item(NAME(m_eop_line));
	save_item(NAME(m_dack3_line));
	save_item(NAME(m_sd_poll_state));
	save_item(NAME(m_floppy_motor));
	save_item(NAME(m_busint));
	save_item(NAME(m_irqs));
	save_pointer(NAME(m_ram->base()), m_ram->bytes());
}

void dmv_state::machine_reset()
{
	m_color_mode = ioport("CONFIG")->read() & 0x01;

	m_ramoutdis = true;
	m_ram_bank = 0;
	m_eop_line = 0;
	m_dack3_line = 0;
	m_sd_poll_state = 0;
	m_floppy_motor = 1;
	m_switch16 = 0;
	m_thold7 = 0;
	m_dma_hrq = 0;
	memset(m_busint, 0, sizeof(m_busint));
	memset(m_irqs, 0, sizeof(m_irqs));

	update_halt_line();
}

void dmv_state::update_halt_line()
{
	m_slot7->hold_w(m_dma_hrq);
	m_slot7->switch16_w(m_switch16);
	m_slot7a->hold_w(m_dma_hrq);
	m_slot7a->switch16_w(m_switch16);

	m_maincpu->set_input_line(INPUT_LINE_HALT, (m_thold7 || m_switch16 || m_dma_hrq) ? ASSERT_LINE : CLEAR_LINE);
}

/* F4 Character Displayer */
static const gfx_layout dmv_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ STEP16(0,8) },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_dmv )
	GFXDECODE_ENTRY("chargen", 0x0000, dmv_charlayout, 0, 1)
GFXDECODE_END


//------------------------------------------------------------------------------------
//   I8237
//------------------------------------------------------------------------------------

void dmv_state::dma_hrq_changed(int state)
{
	m_dma_hrq = state;
	update_halt_line();

	// Assert HLDA
	m_dmac->hack_w(state);
}

void dmv_state::dmac_eop(int state)
{
	if (!(m_dack3_line || m_eop_line) && (m_dack3_line || state))
		m_fdc->tc_w(true);

	m_eop_line = state;
}

void dmv_state::dmac_dack3(int state)
{
	if (!(m_dack3_line || m_eop_line) && (state || m_eop_line))
		m_fdc->tc_w(true);

	m_dack3_line = state;
}

void dmv_state::pit_out0(int state)
{
	if (!state)
	{
		m_floppy_motor = 1;
		if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(m_floppy_motor);
		if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(m_floppy_motor);
	}
}

void dmv_state::timint_w(int state)
{
	m_slot7a->timint_w(state);
	m_slot7->timint_w(state);
}

void dmv_state::fdc_irq(int state)
{
	m_slot7a->flexint_w(state);
	m_slot7->flexint_w(state);

	if (state)
		m_fdc->tc_w(false);
}


void dmv_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_DMV_FORMAT);
}


static void dmv_slot1(device_slot_interface &device)
{
	device.option_add("k200", DMV_K200);        // K200 64K RAM expansion
	device.option_add("k202", DMV_K202);        // K202 192K RAM expansion
	device.option_add("k208", DMV_K208);        // K208 448K RAM expansion
}

static void dmv_slot2_6(device_slot_interface &device)
{
	device.option_add("k210", DMV_K210);        // K210 Centronics
	device.option_add("k211", DMV_K211);        // K211 RS-232 Communications Interface
	device.option_add("k212", DMV_K212);        // K212 RS-232 Printer Interface
	device.option_add("k213", DMV_K213);        // K213 RS-232 Plotter Interface
	device.option_add("k233", DMV_K233);        // K233 16K Shared RAM
	device.option_add("k801", DMV_K801);        // K801 RS-232 Switchable Interface
	device.option_add("k803", DMV_K803);        // K803 RTC module
	device.option_add("k806", DMV_K806);        // K806 Mouse module
	device.option_add("c3282", DMV_C3282);      // C3282 External HD Interface
}

static void dmv_slot7(device_slot_interface &device)
{
	device.option_add("k220", DMV_K220);        // K220 Diagnostic Module
	device.option_add("k231", DMV_K231);        // K231 External 8088 module without interrupt controller
	device.option_add("k234", DMV_K234);        // K234 External 68008 module
}


static void dmv_slot2a(device_slot_interface &device)
{
	device.option_add("k012", DMV_K012);        // K012 Internal HD Interface
}

static void dmv_slot7a(device_slot_interface &device)
{
	device.option_add("k230", DMV_K230);        // K230 Internal 8088 module without interrupt controller
	device.option_add("k235", DMV_K235);        // K235 Internal 8088 module with interrupt controller
}

void dmv_state::dmv(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &dmv_state::dmv_mem);
	m_maincpu->set_addrmap(AS_IO, &dmv_state::dmv_io);

	i8741a_device &kbmcu(I8741A(config, "kb_ctrl_mcu", XTAL(6'000'000)));
	kbmcu.p1_in_cb().set(FUNC(dmv_state::kb_mcu_port1_r)); // bit 0 data from kb
	kbmcu.p1_out_cb().set(FUNC(dmv_state::kb_mcu_port1_w)); // bit 1 data to kb
	kbmcu.p2_out_cb().set(FUNC(dmv_state::kb_mcu_port2_w));

	config.set_perfect_quantum(m_maincpu);

	DMV_KEYBOARD(config, m_keyboard, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_screen_update("upd7220", FUNC(upd7220_device::screen_update));
	m_screen->set_size(640, 400);
	m_screen->set_visarea(0, 640-1, 0, 400-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_dmv);
	PALETTE(config, m_palette, palette_device::RGB_3BIT);
	config.set_default_layout(layout_dmv);

	// devices
	UPD7220(config, m_hgdc, XTAL(5'000'000)/2); // unk clock
	m_hgdc->set_addrmap(0, &dmv_state::upd7220_map);
	m_hgdc->set_display_pixels(FUNC(dmv_state::hgdc_display_pixels));
	m_hgdc->set_draw_text(FUNC(dmv_state::hgdc_draw_text));

	AM9517A(config, m_dmac, 4_MHz_XTAL);
	m_dmac->out_hreq_callback().set(FUNC(dmv_state::dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(dmv_state::dmac_eop));
	m_dmac->in_memr_callback().set(FUNC(dmv_state::program_r));
	m_dmac->out_memw_callback().set(FUNC(dmv_state::program_w));
	m_dmac->in_ior_callback<0>().set([this] () { logerror("Read DMA CH1"); return u8(0); });
	m_dmac->out_iow_callback<0>().set([this] (u8 data) { logerror("Write DMA CH1 %02X", data); });
	m_dmac->in_ior_callback<1>().set([this] () { logerror("Read DMA CH2"); return u8(0); });
	m_dmac->out_iow_callback<1>().set([this] (u8 data) { logerror("Write DMA CH2 %02X", data); });
	m_dmac->in_ior_callback<2>().set(m_hgdc, FUNC(upd7220_device::dack_r));
	m_dmac->out_iow_callback<2>().set(m_hgdc, FUNC(upd7220_device::dack_w));
	m_dmac->in_ior_callback<3>().set(m_fdc, FUNC(i8272a_device::dma_r));
	m_dmac->out_iow_callback<3>().set(m_fdc, FUNC(i8272a_device::dma_w));
	m_dmac->out_dack_callback<3>().set(FUNC(dmv_state::dmac_dack3));

	I8272A(config, m_fdc, 8'000'000, true);
	m_fdc->intrq_wr_callback().set(FUNC(dmv_state::fdc_irq));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq3_w));
	FLOPPY_CONNECTOR(config, "i8272:0", dmv_floppies, "525dd", dmv_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "i8272:1", dmv_floppies, "525dd", dmv_state::floppy_formats);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(50);
	m_pit->out_handler<0>().set(FUNC(dmv_state::pit_out0));
	m_pit->set_clk<2>(XTAL(24'000'000) / 3 / 16);
	m_pit->out_handler<2>().set(FUNC(dmv_state::timint_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	DMVCART_SLOT(config, m_slot1, dmv_slot1, nullptr);
	DMVCART_SLOT(config, m_slot2, dmv_slot2_6, nullptr);
	m_slot2->out_int().set(FUNC(dmv_state::busint2_w));
	m_slot2->out_irq().set(FUNC(dmv_state::irq2_w));
	DMVCART_SLOT(config, m_slot2a, dmv_slot2a, nullptr);
	m_slot2a->out_int().set(FUNC(dmv_state::busint2a_w));
	m_slot2a->out_irq().set(FUNC(dmv_state::irq2a_w));
	DMVCART_SLOT(config, m_slot3, dmv_slot2_6, nullptr);
	m_slot3->out_int().set(FUNC(dmv_state::busint3_w));
	m_slot3->out_irq().set(FUNC(dmv_state::irq3_w));
	DMVCART_SLOT(config, m_slot4, dmv_slot2_6, nullptr);
	m_slot4->out_int().set(FUNC(dmv_state::busint4_w));
	m_slot4->out_irq().set(FUNC(dmv_state::irq4_w));
	DMVCART_SLOT(config, m_slot5, dmv_slot2_6, nullptr);
	m_slot5->out_int().set(FUNC(dmv_state::busint5_w));
	m_slot5->out_irq().set(FUNC(dmv_state::irq5_w));
	DMVCART_SLOT(config, m_slot6, dmv_slot2_6, nullptr);
	m_slot6->out_int().set(FUNC(dmv_state::busint6_w));
	m_slot6->out_irq().set(FUNC(dmv_state::irq6_w));

	DMVCART_SLOT(config, m_slot7, dmv_slot7, nullptr);
	m_slot7->prog_read().set(FUNC(dmv_state::exp_program_r));
	m_slot7->prog_write().set(FUNC(dmv_state::exp_program_w));
	m_slot7->out_thold().set(FUNC(dmv_state::thold7_w));
	m_slot7->out_int().set(FUNC(dmv_state::busint7_w));
	m_slot7->out_irq().set(FUNC(dmv_state::irq7_w));
	DMVCART_SLOT(config, m_slot7a, dmv_slot7a, "k230");
	m_slot7a->prog_read().set(FUNC(dmv_state::exp_program_r));
	m_slot7a->prog_write().set(FUNC(dmv_state::exp_program_w));
	m_slot7a->out_thold().set(FUNC(dmv_state::thold7_w));
	m_slot7a->out_int().set(FUNC(dmv_state::busint7a_w));
	m_slot7a->out_irq().set(FUNC(dmv_state::irq7a_w));

	for (auto &slot : { m_slot1, m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a })
	{
		slot->set_memspace(m_maincpu, AS_PROGRAM);
		slot->set_iospace(m_maincpu, AS_IO);
	}

	SOFTWARE_LIST(config, "flop_list").set_original("dmv");

	QUICKLOAD(config, "quickload", "com,cpm", attotime::from_seconds(3)).set_load_callback(FUNC(dmv_state::quickload_cb));
}

/* ROM definition */
ROM_START( dmv )
	ROM_REGION( 0x2000, "boot", 0 )
	ROM_SYSTEM_BIOS(0, "c07", "C.07.00")    // ROM bears the handwritten note "Color 7.0", this is from the machine that originally had Color, 68K and internal 8088
	ROM_SYSTEM_BIOS(1, "c06", "C.06.00")    // Color machine with older BIOS revision
	ROM_SYSTEM_BIOS(2, "m07", "M.07.00")    // Mono machine with internal 8088 and internal HD
	ROM_SYSTEM_BIOS(3, "m06", "M.06.00")    // Mono machine
	ROM_SYSTEM_BIOS(4, "m05", "M.05.00")    // Mono machine, marked "updated"

	ROMX_LOAD( "dmv_mb_rom_33610.bin", 0x00000,    0x02000,    CRC(bf25f3f0) SHA1(0c7dd37704db4799e340cc836f887cd543e5c964), ROM_BIOS(0) )
	ROMX_LOAD( "dmv_mb_rom_32838.bin", 0x00000,    0x02000,    CRC(d5ceb559) SHA1(e3a05e43aa1b09f0a857b8d54b00bcd321215bf6), ROM_BIOS(1) )
	ROMX_LOAD( "dmv_mb_rom_33609.bin", 0x00000,    0x02000,    CRC(120951b6) SHA1(57bef9cc6379dea5730bc1477e8896508e00a349), ROM_BIOS(2) )
	ROMX_LOAD( "dmv_mb_rom_32676.bin", 0x00000,    0x02000,    CRC(7796519e) SHA1(8d5dd9c1e66c96fcca271b6f673d6a0e784acb33), ROM_BIOS(3) )
	ROMX_LOAD( "dmv_mb_rom_32664.bin", 0x00000,    0x02000,    CRC(6624610e) SHA1(e9226be897d2c5f875784ab77dad8807f14c7714), ROM_BIOS(4) )

	ROM_REGION(0x400, "kb_ctrl_mcu", 0)
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(0) )
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(1) )
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(2) )
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(3) )
	ROMX_LOAD( "dmv_mb_8741_32121.bin",    0x00000,    0x00400,    CRC(a03af298) SHA1(144cba41294c46f5ca79b7ad8ced0e4408168775), ROM_BIOS(4) )

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD( "76161.bin",    0x00000,    0x00800,  CRC(6e4df4f9) SHA1(20ff4fc48e55eaf5131f6573fff93e7f97d2f45d)) // same for both color and monochrome board

	ROM_REGION(0x10000, "ram", ROMREGION_ERASE) // 64K RAM on mainboard
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY  FULLNAME           FLAGS
COMP( 1984, dmv,  0,      0,      dmv,     dmv,   dmv_state, empty_init, "NCR",   "Decision Mate V", MACHINE_SUPPORTS_SAVE)
