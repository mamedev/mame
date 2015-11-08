// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        NCR Decision Mate V

        04/01/2012 Skeleton driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/upd765.h"
#include "machine/am9517a.h"
#include "machine/pit8253.h"
#include "machine/dmv_keyb.h"
#include "sound/speaker.h"
#include "video/upd7220.h"
#include "formats/dmv_dsk.h"

// expansion slots
#include "bus/dmv/dmvbus.h"
#include "bus/dmv/k210.h"
#include "bus/dmv/k220.h"
#include "bus/dmv/k230.h"
#include "bus/dmv/k233.h"
#include "bus/dmv/k801.h"
#include "bus/dmv/k803.h"
#include "bus/dmv/k806.h"
#include "bus/dmv/ram.h"

#include "softlist.h"

#include "dmv.lh"

class dmv_state : public driver_device
{
public:
	dmv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_hgdc(*this, "upd7220"),
			m_dmac(*this, "dma8237"),
			m_pit(*this, "pit8253"),
			m_fdc(*this, "i8272"),
			m_floppy0(*this, "i8272:0"),
			m_floppy1(*this, "i8272:1"),
			m_keyboard(*this, "keyboard"),
			m_speaker(*this, "speaker"),
			m_video_ram(*this, "video_ram"),
			m_palette(*this, "palette"),
			m_ram(*this, "ram"),
			m_bootrom(*this, "boot"),
			m_chargen(*this, "chargen"),
			m_slot1(*this, "slot1"),
			m_slot2(*this, "slot2"),
			m_slot2a(*this, "slot2a"),
			m_slot3(*this, "slot3"),
			m_slot4(*this, "slot4"),
			m_slot5(*this, "slot5"),
			m_slot6(*this, "slot6"),
			m_slot7(*this, "slot7"),
			m_slot7a(*this, "slot7a")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<i8272a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<dmv_keyboard_device> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<UINT16> m_video_ram;
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

	virtual void machine_start();
	virtual void machine_reset();
	void update_halt_line();

	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(dmac_eop);
	DECLARE_WRITE_LINE_MEMBER(dmac_dack3);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq);
	DECLARE_WRITE_LINE_MEMBER(pit_out0);
	DECLARE_WRITE_LINE_MEMBER(timint_w);
	DECLARE_WRITE8_MEMBER(fdd_motor_w);
	DECLARE_READ8_MEMBER(sys_status_r);
	DECLARE_WRITE8_MEMBER(tc_set_w);
	DECLARE_WRITE8_MEMBER(switch16_w);
	DECLARE_READ8_MEMBER(ramsel_r);
	DECLARE_READ8_MEMBER(romsel_r);
	DECLARE_WRITE8_MEMBER(ramsel_w);
	DECLARE_WRITE8_MEMBER(romsel_w);
	DECLARE_READ8_MEMBER(kb_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(kb_mcu_port1_w);
	DECLARE_WRITE8_MEMBER(kb_mcu_port2_w);
	DECLARE_WRITE8_MEMBER(rambank_w);
	DECLARE_READ8_MEMBER(program_r);
	DECLARE_WRITE8_MEMBER(program_w);
	DECLARE_READ8_MEMBER(exp_program_r);
	DECLARE_WRITE8_MEMBER(exp_program_w);
	DECLARE_WRITE_LINE_MEMBER(thold7_w);

	void update_busint(int slot, int state);
	DECLARE_WRITE_LINE_MEMBER(busint2_w)    { update_busint(0, state); }
	DECLARE_WRITE_LINE_MEMBER(busint2a_w)   { update_busint(1, state); }
	DECLARE_WRITE_LINE_MEMBER(busint3_w)    { update_busint(2, state); }
	DECLARE_WRITE_LINE_MEMBER(busint4_w)    { update_busint(3, state); }
	DECLARE_WRITE_LINE_MEMBER(busint5_w)    { update_busint(4, state); }
	DECLARE_WRITE_LINE_MEMBER(busint6_w)    { update_busint(5, state); }
	DECLARE_WRITE_LINE_MEMBER(busint7_w)    { update_busint(6, state); }
	DECLARE_WRITE_LINE_MEMBER(busint7a_w)   { update_busint(7, state); }

	void update_irqs(int slot, int state);
	DECLARE_WRITE_LINE_MEMBER(irq2_w)       { update_irqs(0, state); }
	DECLARE_WRITE_LINE_MEMBER(irq2a_w)      { update_irqs(1, state); }
	DECLARE_WRITE_LINE_MEMBER(irq3_w)       { update_irqs(2, state); }
	DECLARE_WRITE_LINE_MEMBER(irq4_w)       { update_irqs(3, state); }
	DECLARE_WRITE_LINE_MEMBER(irq5_w)       { update_irqs(4, state); }
	DECLARE_WRITE_LINE_MEMBER(irq6_w)       { update_irqs(5, state); }
	DECLARE_WRITE_LINE_MEMBER(irq7_w)       { update_irqs(6, state); }
	DECLARE_WRITE_LINE_MEMBER(irq7a_w)      { update_irqs(7, state); }

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	UINT8 program_read(address_space &space, int cas, offs_t offset);
	void program_write(address_space &space, int cas, offs_t offset, UINT8 data);

	void ifsel_r(address_space &space, int ifsel, offs_t offset, UINT8 &data);
	void ifsel_w(address_space &space, int ifsel, offs_t offset, UINT8 data);
	DECLARE_READ8_MEMBER(ifsel0_r)  { UINT8 data = 0xff;   ifsel_r(space, 0, offset, data);   return data; }
	DECLARE_READ8_MEMBER(ifsel1_r)  { UINT8 data = 0xff;   ifsel_r(space, 1, offset, data);   return data; }
	DECLARE_READ8_MEMBER(ifsel2_r)  { UINT8 data = 0xff;   ifsel_r(space, 2, offset, data);   return data; }
	DECLARE_READ8_MEMBER(ifsel3_r)  { UINT8 data = 0xff;   ifsel_r(space, 3, offset, data);   return data; }
	DECLARE_READ8_MEMBER(ifsel4_r)  { UINT8 data = 0xff;   ifsel_r(space, 4, offset, data);   return data; }
	DECLARE_WRITE8_MEMBER(ifsel0_w) { ifsel_w(space, 0, offset, data); }
	DECLARE_WRITE8_MEMBER(ifsel1_w) { ifsel_w(space, 1, offset, data); }
	DECLARE_WRITE8_MEMBER(ifsel2_w) { ifsel_w(space, 2, offset, data); }
	DECLARE_WRITE8_MEMBER(ifsel3_w) { ifsel_w(space, 3, offset, data); }
	DECLARE_WRITE8_MEMBER(ifsel4_w) { ifsel_w(space, 4, offset, data); }

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );

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

WRITE8_MEMBER(dmv_state::tc_set_w)
{
	m_fdc->tc_w(true);
}

WRITE8_MEMBER(dmv_state::switch16_w)
{
	m_switch16 = !m_switch16;
	update_halt_line();
}

WRITE8_MEMBER(dmv_state::leds_w)
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
		output_set_led_value(8-i, BIT(data, i));
}

READ8_MEMBER(dmv_state::ramsel_r)
{
	m_ramoutdis = false;
	return 0;
}

READ8_MEMBER(dmv_state::romsel_r)
{
	m_ramoutdis = true;
	return 0;
}

WRITE8_MEMBER(dmv_state::ramsel_w)
{
	m_ramoutdis = false;
}

WRITE8_MEMBER(dmv_state::romsel_w)
{
	m_ramoutdis = true;
}

WRITE8_MEMBER(dmv_state::rambank_w)
{
	m_ram_bank = offset;
}

WRITE8_MEMBER(dmv_state::fdd_motor_w)
{
	m_pit->write_gate0(1);
	m_pit->write_gate0(0);

	m_floppy_motor = 0;
	m_floppy0->get_device()->mon_w(m_floppy_motor);
	m_floppy1->get_device()->mon_w(m_floppy_motor);
}

READ8_MEMBER(dmv_state::sys_status_r)
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
	UINT8 data = 0x00;

	if (m_floppy_motor)
		data |= 0x01;

	// 16-bit CPU
	if (!(m_slot7->av16bit() || m_slot7a->av16bit()))
		data |= 0x02;

	if (!m_floppy0->get_device()->ready_r())
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
		UINT16 green = m_video_ram[(0x00000 + (address & 0x7fff)) >> 1];
		UINT16 red   = m_video_ram[(0x08000 + (address & 0x7fff)) >> 1];
		UINT16 blue  = m_video_ram[(0x10000 + (address & 0x7fff)) >> 1];

		for(int xi=0; xi<16; xi++)
		{
			int r = ((red   >> xi) & 1) ? 255 : 0;
			int g = ((green >> xi) & 1) ? 255 : 0;
			int b = ((blue  >> xi) & 1) ? 255 : 0;

			if (bitmap.cliprect().contains(x + xi, y))
				bitmap.pix32(y, x + xi) = rgb_t(r, g, b);
		}
	}
	else
	{
		const rgb_t *palette = m_palette->palette()->entry_list_raw();

		// 32KB videoram
		UINT16 gfx = m_video_ram[(address & 0xffff) >> 1];

		for(int xi=0;xi<16;xi++)
		{
			if (bitmap.cliprect().contains(x + xi, y))
				bitmap.pix32(y, x + xi) = ((gfx >> xi) & 1) ? palette[1] : palette[0];
		}
	}
}

UPD7220_DRAW_TEXT_LINE_MEMBER( dmv_state::hgdc_draw_text )
{
	for( int x = 0; x < pitch; x++ )
	{
		UINT8 tile = m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] & 0xff;
		UINT8 attr = m_video_ram[(((addr+x)*2) & 0x1ffff) >> 1] >> 8;

		rgb_t bg, fg;
		if (m_color_mode)
		{
			bg = rgb_t(attr & 0x20 ? 0 : 255, attr & 0x40 ? 0 : 255, attr & 0x80 ? 0 : 255);
			fg = rgb_t(attr & 0x04 ? 255 : 0, attr & 0x08 ? 255 : 0, attr & 0x10 ? 255 : 0);
		}
		else
		{
			const rgb_t *palette = m_palette->palette()->entry_list_raw();
			bg = palette[(attr & 1) ? 1 : 0];
			fg = palette[(attr & 1) ? 0 : 1];
		}

		for( int yi = 0; yi < lr; yi++)
		{
			UINT8 tile_data = m_chargen->base()[(tile*16+yi) & 0x7ff];

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			for( int xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;
				int pen = (tile_data >> xi) & 1 ? 1 : 0;

				res_x = x * 8 + xi;
				res_y = y + yi;

				if(!machine().first_screen()->visible_area().contains(res_x, res_y))
					continue;

				if(yi >= 16) { pen = 0; }

				bitmap.pix32(res_y, res_x) = pen ? fg : bg;
			}
		}
	}
}

static SLOT_INTERFACE_START( dmv_floppies )
		SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
		SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END


void dmv_state::ifsel_r(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	dmvcart_slot_device *slots[] = { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a };
	for(int i=0; i<8; i++)
		slots[i]->io_read(space, ifsel, offset, data);
}

void dmv_state::ifsel_w(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	dmvcart_slot_device *slots[] = { m_slot2, m_slot2a, m_slot3, m_slot4, m_slot5, m_slot6, m_slot7, m_slot7a };
	for(int i=0; i<8; i++)
		slots[i]->io_write(space, ifsel, offset, data);
}

WRITE8_MEMBER(dmv_state::exp_program_w)
{
	program_write(space, (offset >> 16) & 0x07, offset, data);
}

READ8_MEMBER(dmv_state::exp_program_r)
{
	return program_read(space, (offset >> 16) & 0x07, offset);
}

WRITE8_MEMBER(dmv_state::program_w)
{
	program_write(space, m_ram_bank, offset, data);
}

READ8_MEMBER(dmv_state::program_r)
{
	return program_read(space, m_ram_bank, offset);
}

WRITE_LINE_MEMBER( dmv_state::thold7_w )
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
	for (int i=0; i<8; i++)
		if (m_busint[i] != CLEAR_LINE)
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

void dmv_state::program_write(address_space &space, int cas, offs_t offset, UINT8 data)
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

UINT8 dmv_state::program_read(address_space &space, int cas, offs_t offset)
{
	UINT8 data = 0xff;
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

static ADDRESS_MAP_START(dmv_mem, AS_PROGRAM, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(program_r, program_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dmv_io , AS_IO, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(leds_w)
	AM_RANGE(0x10, 0x10) AM_READWRITE(ramsel_r, ramsel_w)
	AM_RANGE(0x11, 0x11) AM_READWRITE(romsel_r, romsel_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(tc_set_w)
	AM_RANGE(0x13, 0x13) AM_READ(sys_status_r)
	AM_RANGE(0x14, 0x14) AM_WRITE(fdd_motor_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE("dma8237", am9517a_device, read, write)
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("kb_ctrl_mcu", upi41_cpu_device, upi41_master_r, upi41_master_w)
	AM_RANGE(0x50, 0x51) AM_DEVICE("i8272", i8272a_device, map)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0xa0, 0xa1) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)
	AM_RANGE(0xd0, 0xd7) AM_WRITE(switch16_w)
	AM_RANGE(0xe0, 0xe7) AM_WRITE(rambank_w)

	AM_RANGE(0x60, 0x6f) AM_READWRITE(ifsel0_r, ifsel0_w)
	AM_RANGE(0x70, 0x7f) AM_READWRITE(ifsel1_r, ifsel1_w)
	AM_RANGE(0x30, 0x3f) AM_READWRITE(ifsel2_r, ifsel2_w)
	AM_RANGE(0xb0, 0xbf) AM_READWRITE(ifsel3_r, ifsel3_w)
	AM_RANGE(0xc0, 0xcf) AM_READWRITE(ifsel4_r, ifsel4_w)
ADDRESS_MAP_END

READ8_MEMBER(dmv_state::kb_mcu_port1_r)
{
	return !(m_keyboard->sd_poll_r() & !m_sd_poll_state);
}

WRITE8_MEMBER(dmv_state::kb_mcu_port1_w)
{
	m_sd_poll_state = BIT(data, 1);
	m_keyboard->sd_poll_w(!m_sd_poll_state);
}

WRITE8_MEMBER(dmv_state::kb_mcu_port2_w)
{
	m_speaker->level_w(BIT(data, 0));
	m_slot7a->keyint_w(BIT(data, 4));
	m_slot7->keyint_w(BIT(data, 4));
}

static ADDRESS_MAP_START( dmv_kb_ctrl_io, AS_IO, 8, dmv_state )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(kb_mcu_port1_r, kb_mcu_port1_w) // bit 0 data from kb, bit 1 data to kb
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(kb_mcu_port2_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 16, dmv_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM  AM_SHARE("video_ram")
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( dmv )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "Video Board" )
	PORT_CONFSETTING( 0x00, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
INPUT_PORTS_END

void dmv_state::machine_start()
{
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

static GFXDECODE_START( dmv )
	GFXDECODE_ENTRY("chargen", 0x0000, dmv_charlayout, 0, 1)
GFXDECODE_END


//------------------------------------------------------------------------------------
//   I8237
//------------------------------------------------------------------------------------

WRITE_LINE_MEMBER( dmv_state::dma_hrq_changed )
{
	m_dma_hrq = state;
	update_halt_line();

	// Assert HLDA
	m_dmac->hack_w(state);
}

WRITE_LINE_MEMBER( dmv_state::dmac_eop )
{
	if (!(m_dack3_line || m_eop_line) && (m_dack3_line || state))
		m_fdc->tc_w(true);

	m_eop_line = state;
}

WRITE_LINE_MEMBER( dmv_state::dmac_dack3 )
{
	if (!(m_dack3_line || m_eop_line) && (state || m_eop_line))
		m_fdc->tc_w(true);

	m_dack3_line = state;
}

WRITE_LINE_MEMBER( dmv_state::pit_out0 )
{
	if (!state)
	{
		m_floppy_motor = 1;
		m_floppy0->get_device()->mon_w(m_floppy_motor);
		m_floppy1->get_device()->mon_w(m_floppy_motor);
	}
}

WRITE_LINE_MEMBER( dmv_state::timint_w )
{
	m_slot7a->timint_w(state);
	m_slot7->timint_w(state);
}

WRITE_LINE_MEMBER( dmv_state::fdc_irq )
{
	m_slot7a->flexint_w(state);
	m_slot7->flexint_w(state);

	if (state)
		m_fdc->tc_w(false);
}


FLOPPY_FORMATS_MEMBER( dmv_state::floppy_formats )
	FLOPPY_DMV_FORMAT
FLOPPY_FORMATS_END


static SLOT_INTERFACE_START(dmv_slot1)
	SLOT_INTERFACE("k200", DMV_K200)            // K200 64K RAM expansion
	SLOT_INTERFACE("k202", DMV_K202)            // K202 192K RAM expansion
	SLOT_INTERFACE("k208", DMV_K208)            // K208 448K RAM expansion
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(dmv_slot2_6)
	SLOT_INTERFACE("k210", DMV_K210)            // K210 Centronics
	SLOT_INTERFACE("k211", DMV_K211)            // K211 RS-232 Communications Interface
	SLOT_INTERFACE("k212", DMV_K212)            // K212 RS-232 Printer Interface
	SLOT_INTERFACE("k213", DMV_K213)            // K213 RS-232 Plotter Interface
	SLOT_INTERFACE("k233", DMV_K233)            // K233 16K Shared RAM
	SLOT_INTERFACE("k801", DMV_K801)            // K801 RS-232 Switchable Interface
	SLOT_INTERFACE("k803", DMV_K803)            // K803 RTC module
	SLOT_INTERFACE("k806", DMV_K806)            // K806 Mouse module
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(dmv_slot7)
	SLOT_INTERFACE("k220", DMV_K220)            // K220 Diagnostic Module
	SLOT_INTERFACE("k231", DMV_K231)            // K231 External 8088 module without interrupt controller
	SLOT_INTERFACE("k234", DMV_K234)            // K234 External 68008 module
SLOT_INTERFACE_END


static SLOT_INTERFACE_START(dmv_slot2a)

SLOT_INTERFACE_END

static SLOT_INTERFACE_START(dmv_slot7a)
	SLOT_INTERFACE("k230", DMV_K230)            // K230 Internal 8088 module without interrupt controller
	SLOT_INTERFACE("k235", DMV_K235)            // K235 Internal 8088 module with interrupt controller
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( dmv, dmv_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_24MHz / 6)
	MCFG_CPU_PROGRAM_MAP(dmv_mem)
	MCFG_CPU_IO_MAP(dmv_io)

	MCFG_CPU_ADD("kb_ctrl_mcu", I8741, XTAL_6MHz)
	MCFG_CPU_IO_MAP(dmv_kb_ctrl_io)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_DMV_KEYBOARD_ADD("keyboard")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dmv)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")
	MCFG_DEFAULT_LAYOUT(layout_dmv)

	// devices
	MCFG_DEVICE_ADD("upd7220", UPD7220, XTAL_5MHz/2) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(dmv_state, hgdc_display_pixels)
	MCFG_UPD7220_DRAW_TEXT_CALLBACK_OWNER(dmv_state, hgdc_draw_text)

	MCFG_DEVICE_ADD( "dma8237", AM9517A, XTAL_4MHz )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(dmv_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(dmv_state, dmac_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(dmv_state, program_r))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(dmv_state, program_w))
	MCFG_I8237_IN_IOR_0_CB(LOGGER("DMA CH1", 0))
	MCFG_I8237_OUT_IOW_0_CB(LOGGER("DMA CH1", 0))
	MCFG_I8237_IN_IOR_1_CB(LOGGER("DMA CH2", 0))
	MCFG_I8237_OUT_IOW_1_CB(LOGGER("DMA CH2", 0))
	MCFG_I8237_IN_IOR_2_CB(DEVREAD8("upd7220", upd7220_device, dack_r))
	MCFG_I8237_OUT_IOW_2_CB(DEVWRITE8("upd7220", upd7220_device, dack_w))
	MCFG_I8237_IN_IOR_3_CB(DEVREAD8("i8272", i8272a_device, mdma_r))
	MCFG_I8237_OUT_IOW_3_CB(DEVWRITE8("i8272", i8272a_device, mdma_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(dmv_state, dmac_dack3))

	MCFG_I8272A_ADD( "i8272", true )
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(dmv_state, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dma8237", am9517a_device, dreq3_w))
	MCFG_FLOPPY_DRIVE_ADD("i8272:0", dmv_floppies, "525dd", dmv_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("i8272:1", dmv_floppies, "525dd", dmv_state::floppy_formats)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(50)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(dmv_state, pit_out0))
	MCFG_PIT8253_CLK2(XTAL_24MHz / 3 / 16)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(dmv_state, timint_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("slot1", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot1, NULL, false)
	MCFG_DEVICE_ADD("slot2", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2_6, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint2_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq2_w))
	MCFG_DEVICE_ADD("slot2a", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2a, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint2a_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq2a_w))
	MCFG_DEVICE_ADD("slot3", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2_6, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint3_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq3_w))
	MCFG_DEVICE_ADD("slot4", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2_6, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint4_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq4_w))
	MCFG_DEVICE_ADD("slot5", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2_6, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint5_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq5_w))
	MCFG_DEVICE_ADD("slot6", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot2_6, NULL, false)
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint6_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq6_w))

	MCFG_DEVICE_ADD("slot7", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot7, NULL, false)
	MCFG_DMVCART_SLOT_PROGRAM_READWRITE_CB(READ8(dmv_state, exp_program_r), WRITE8(dmv_state, exp_program_w))
	MCFG_DMVCART_SLOT_OUT_THOLD_CB(WRITELINE(dmv_state, thold7_w))
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint7_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq7_w))
	MCFG_DEVICE_ADD("slot7a", DMVCART_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(dmv_slot7a, "k230", false)
	MCFG_DMVCART_SLOT_PROGRAM_READWRITE_CB(READ8(dmv_state, exp_program_r), WRITE8(dmv_state, exp_program_w))
	MCFG_DMVCART_SLOT_OUT_THOLD_CB(WRITELINE(dmv_state, thold7_w))
	MCFG_DMVCART_SLOT_OUT_INT_CB(WRITELINE(dmv_state, busint7a_w))
	MCFG_DMVCART_SLOT_OUT_IRQ_CB(WRITELINE(dmv_state, irq7a_w))

	MCFG_SOFTWARE_LIST_ADD("flop_list", "dmv")

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dmv )
	ROM_REGION( 0x2000, "boot", 0 )
	ROM_SYSTEM_BIOS(0, "c07", "C.07.00")    // ROM bears the handwritten note "Color 7.0", this is from the machine that originally had Color, 68K and internal 8088
	ROM_SYSTEM_BIOS(1, "m07", "M.07.00")    // Mono machine with internal 8088 and internal HD
	ROM_SYSTEM_BIOS(2, "m06", "M.06.00")    // Mono machine
	ROM_SYSTEM_BIOS(3, "m05", "M.05.00")    // Mono machine, marked "updated"

	ROMX_LOAD( "dmv_mb_rom_33610.bin", 0x00000,    0x02000,    CRC(bf25f3f0) SHA1(0c7dd37704db4799e340cc836f887cd543e5c964), ROM_BIOS(1) )
	ROMX_LOAD( "dmv_mb_rom_33609.bin", 0x00000,    0x02000,    CRC(120951b6) SHA1(57bef9cc6379dea5730bc1477e8896508e00a349), ROM_BIOS(2) )
	ROMX_LOAD( "dmv_mb_rom_32676.bin", 0x00000,    0x02000,    CRC(7796519e) SHA1(8d5dd9c1e66c96fcca271b6f673d6a0e784acb33), ROM_BIOS(3) )
	ROMX_LOAD( "dmv_mb_rom_32664.bin", 0x00000,    0x02000,    CRC(6624610e) SHA1(e9226be897d2c5f875784ab77dad8807f14c7714), ROM_BIOS(4) )

	ROM_REGION(0x400, "kb_ctrl_mcu", 0)
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(1) )
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(2) )
	ROMX_LOAD( "dmv_mb_8741_32678.bin",    0x00000,    0x00400,    CRC(50d1dc4c) SHA1(2c8251d6c8df9f507e11bf920869657f4d074db1), ROM_BIOS(3) )
	ROMX_LOAD( "dmv_mb_8741_32121.bin",    0x00000,    0x00400,    CRC(a03af298) SHA1(144cba41294c46f5ca79b7ad8ced0e4408168775), ROM_BIOS(4) )

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD( "76161.bin",    0x00000,    0x00800,  CRC(6e4df4f9) SHA1(20ff4fc48e55eaf5131f6573fff93e7f97d2f45d)) // same for both color and monochrome board

	ROM_REGION(0x10000, "ram", ROMREGION_ERASE) // 64K RAM on mainboard
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME             FLAGS */
COMP( 1984, dmv,    0,       0,         dmv,    dmv, driver_device,  0,      "NCR",   "Decision Mate V",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
