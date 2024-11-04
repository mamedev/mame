// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"

#include "cpu/i86/i86.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "sound/spkrdev.h"
#include "pc_t1t.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/isa/fdc.h"
#include "bus/pc_joy/pc_joy.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/terminal.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class pcjr_state : public driver_device
{
public:
	pcjr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_pit8253(*this, "pit8253"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_cart1(*this, "cartslot1"),
		m_cart2(*this, "cartslot2"),
		m_ram(*this, RAM_TAG),
		m_fdc(*this, "fdc"),
		m_keyboard(*this, "pc_keyboard")
	{ }

	void ibmpcjx(machine_config &config);
	void ibmpcjr(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void ibmpcjr_io(address_map &map) ATTR_COLD;
	void ibmpcjr_map(address_map &map) ATTR_COLD;
	void ibmpcjx_io(address_map &map) ATTR_COLD;
	void ibmpcjx_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(delayed_irq);
	TIMER_CALLBACK_MEMBER(watchdog_expired);
	TIMER_CALLBACK_MEMBER(kb_signal);

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8253_device> m_pit8253;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart1;
	required_device<generic_slot_device> m_cart2;
	required_device<ram_device> m_ram;
	required_device<upd765a_device> m_fdc;
	required_device<pc_keyboard_device> m_keyboard;

	void out2_changed(int state);
	void keyb_interrupt(int state);

	void pc_nmi_enable_w(uint8_t data);
	uint8_t pcjr_nmi_enable_r();
	void pic8259_set_int_line(int state);

	void pcjr_ppi_portb_w(uint8_t data);
	uint8_t pcjr_ppi_portc_r();
	void pcjr_fdc_dor_w(uint8_t data);
	uint8_t pcjx_port_1ff_r();
	void pcjx_port_1ff_w(uint8_t data);
	void pcjx_set_bank(int unk1, int unk2, int unk3);

	std::pair<std::error_condition, std::string> load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart1_load) { return load_cart(image, m_cart1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart2_load) { return load_cart(image, m_cart2); }
	void pc_speaker_set_spkrdata(uint8_t data);

	uint8_t m_pc_spkrdata = 0;
	uint8_t m_pit_out2 = 0;
	uint8_t m_pcjr_dor = 0;
	uint8_t m_pcjx_1ff_count = 0;
	uint8_t m_pcjx_1ff_val = 0;
	uint8_t m_pcjx_1ff_bankval = 0;
	uint8_t m_pcjx_1ff_bank[20][2]{};
	int m_ppi_portc_switch_high = 0;
	uint8_t m_ppi_portb = 0;

	uint8_t m_pc_keyb_data = 0;
	uint8_t m_transferring = 0;
	uint8_t m_latch = 0;
	uint32_t m_raw_keyb_data = 0;
	int m_signal_count = 0;
	uint8_t m_nmi_enabled = 0;

	emu_timer *m_pc_int_delay_timer = nullptr;
	emu_timer *m_pcjr_watchdog = nullptr;
	emu_timer *m_keyb_signal_timer = nullptr;
};

static INPUT_PORTS_START( ibmpcjr )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("pcvideo_pcjr:screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )
INPUT_PORTS_END

void pcjr_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());

	m_pc_int_delay_timer = timer_alloc(FUNC(pcjr_state::delayed_irq), this);
	m_pcjr_watchdog = timer_alloc(FUNC(pcjr_state::watchdog_expired), this);
	m_keyb_signal_timer = timer_alloc(FUNC(pcjr_state::kb_signal), this);
}

void pcjr_state::machine_reset()
{
	m_pc_spkrdata = 0;
	m_pit_out2 = 1;
	m_ppi_portc_switch_high = 0;
	m_ppi_portb = 0;
	m_pcjr_dor = 0;
	m_speaker->level_w(0);

	m_pcjx_1ff_count = 0;
	m_pcjx_1ff_val = 0;
	m_pcjx_1ff_bankval = 0;
	memset(m_pcjx_1ff_bank, 0, sizeof(m_pcjx_1ff_bank));

	m_transferring = 0;
	m_latch = 0;
	m_raw_keyb_data = 0;
	m_nmi_enabled = 0x80;
}

TIMER_CALLBACK_MEMBER(pcjr_state::delayed_irq)
{
	m_maincpu->set_input_line(0, param ? ASSERT_LINE : CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(pcjr_state::watchdog_expired)
{
	if (m_pcjr_dor & 0x20)
		m_pic8259->ir6_w(1);
	else
		m_pic8259->ir6_w(0);
}

TIMER_CALLBACK_MEMBER(pcjr_state::kb_signal)
{
	m_raw_keyb_data = m_raw_keyb_data >> 1;
	m_signal_count--;

	if (m_signal_count <= 0)
	{
		m_keyb_signal_timer->adjust(attotime::never, 0, attotime::never);
		m_transferring = 0;
	}
}

/*************************************************************
 *
 * PCJR pic8259 configuration
 *
 * Part of the PCJR CRT POST test at address F0452/F0454 writes
 * to the PIC enabling an IRQ which is then immediately fired,
 * however it is expected that the actual IRQ is taken one
 * instruction later (the irq bit is reset by the instruction
 * at F0454). Delaying taking of an IRQ by one instruction for
 * all cases breaks floppy emulation. This seems to be a really
 * tight corner case. For now we delay the IRQ by one instruction
 * only for the PCJR and only when it's inside the POST checks.
 *
 *************************************************************/

void pcjr_state::pic8259_set_int_line(int state)
{
	uint32_t pc = m_maincpu->pc();
	if ( (pc == 0xF0453) || (pc == 0xFF196) )
	{
		m_pc_int_delay_timer->adjust( m_maincpu->cycles_to_attotime(20), state );
	}
	else
	{
		m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
	}
}

/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/
void pcjr_state::pc_speaker_set_spkrdata(uint8_t data)
{
	m_pc_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}

void pcjr_state::out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
	m_cassette->output(state ? 1.0 : -1.0);
}

/*************************************************************
 *
 * PCJR NMI and raw keybaord handling
 *
 * raw signals on the keyboard cable:
 * ---_-b0b1b2b3b4b5b6b7pa----------------------
 *    | | | | | | | | | | |
 *    | | | | | | | | | | *--- 11 stop bits ( -- = 1 stop bit )
 *    | | | | | | | | | *----- parity bit ( 0 = _-, 1 = -_ )
 *    | | | | | | | | *------- bit 7 ( 0 = _-, 1 = -_ )
 *    | | | | | | | *--------- bit 6 ( 0 = _-, 1 = -_ )
 *    | | | | | | *----------- bit 5 ( 0 = _-, 1 = -_ )
 *    | | | | | *------------- bit 4 ( 0 = _-, 1 = -_ )
 *    | | | | *--------------- bit 3 ( 0 = _-, 1 = -_ )
 *    | | | *----------------- bit 2 ( 0 = _-, 1 = -_ )
 *    | | *------------------- bit 1 ( 0 = _-, 1 = -_ )
 *    | *--------------------- bit 0 ( 0 = _-, 1 = -_ )
 *    *----------------------- start bit (always _- )
 *
 * An entire bit lasts for 440 uSec, half bit time is 220 uSec.
 * Transferring an entire byte takes 21 x 440uSec. The extra
 * time of the stop bits is to allow the CPU to do other things
 * besides decoding keyboard signals.
 *
 * These signals get inverted before going to the PCJR
 * handling hardware. The sequence for the start then
 * becomes:
 *
 * __-_b0b1.....
 *   |
 *   *---- on the 0->1 transition of the start bit a keyboard
 *         latch signal is set to 1 and an NMI is generated
 *         when enabled.
 *         The keyboard latch is reset by reading from the
 *         NMI enable port (A0h).
 *
 *************************************************************/

void pcjr_state::keyb_interrupt(int state)
{
	int data;

	if (state && (data = m_keyboard->read()))
	{
		m_latch = 1;

		if(m_transferring)
			return;

		m_pc_keyb_data = data;

		/* Calculate the raw data */
		uint8_t parity = 0;
		for (int i = 0; i < 8; i++)
		{
			if (BIT(data, i))
			{
				parity ^= 1;
			}
		}
		m_raw_keyb_data = 0;
		m_raw_keyb_data = (m_raw_keyb_data << 2) | (parity ? 1 : 2);
		for (int i = 0; i < 8; i++)
		{
			m_raw_keyb_data = (m_raw_keyb_data << 2) | ((data & 0x80) ? 1 : 2);
			data <<= 1;
		}
		/* Insert start bit */
		m_raw_keyb_data = (m_raw_keyb_data << 2) | 1;
		m_signal_count = 20 + 22;

		/* we are now transferring a byte of keyboard data */
		m_transferring = 1;

		/* Set timer */
		m_keyb_signal_timer->adjust(attotime::from_usec(220), 0, attotime::from_usec(220));
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_enabled && m_latch);
	}
}

uint8_t pcjr_state::pcjr_nmi_enable_r()
{
	m_latch = 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_nmi_enabled;
}

void pcjr_state::pc_nmi_enable_w(uint8_t data)
{
	m_nmi_enabled = data & 0x80;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_enabled && m_latch);
}

void pcjr_state::pcjr_ppi_portb_w(uint8_t data)
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata(data & 0x02);

	m_cassette->change_state((data & 0x08) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

/*
 * Port C connections on a PCJR (notes from schematics):
 * PC0 - KYBD LATCH
 * PC1 - MODEM CD INSTALLED
 * PC2 - DISKETTE CD INSTALLED
 * PC3 - ATR CD IN
 * PC4 - cassette audio
 * PC5 - OUT2 from 8253
 * PC6 - KYBD IN
 * PC7 - (keyboard) CABLE CONNECTED
 */
uint8_t pcjr_state::pcjr_ppi_portc_r()
{
	int data = 0xff;

	data &= ~0x80;
	data &= ~0x04;      /* floppy drive installed */
	if (m_ram->size() > 64 * 1024)    /* more than 64KB ram installed */
		data &= ~0x08;
	data = (data & ~0x01) | (m_latch ? 0x01 : 0x00);
	if (!(m_ppi_portb & 0x08))
	{
		double tap_val = m_cassette->input();

		if (tap_val < 0)
		{
			data &= ~0x10;
		}
		else
		{
			data |= 0x10;
		}
	}
	else
	{
		if (m_ppi_portb & 0x01)
		{
			data = (data & ~0x10) | (m_pit_out2 ? 0x10 : 0x00);
		}
	}
	data = (data & ~0x20) | (m_pit_out2 ? 0x20 : 0x00);
	data = (data & ~0x40) | ((m_raw_keyb_data & 0x01) ? 0x40 : 0x00);

	return data;
}

void pcjr_state::pcjr_fdc_dor_w(uint8_t data)
{
	logerror("fdc: dor = %02x\n", data);
	uint8_t pdor = m_pcjr_dor;
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = nullptr;

	if (m_fdc->subdevice("1"))
		floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	m_pcjr_dor = data;

	if (floppy0)
		floppy0->mon_w(!(m_pcjr_dor & 1));
	if (floppy1)
		floppy1->mon_w(!(m_pcjr_dor & 2));

	if (m_pcjr_dor & 1)
		m_fdc->set_floppy(floppy0);
	else if (m_pcjr_dor & 2)
		m_fdc->set_floppy(floppy1);
	else
		m_fdc->set_floppy(nullptr);

	m_fdc->reset_w(!BIT(m_pcjr_dor, 7));

	if (m_pcjr_dor & 0x20)
	{
		if ((pdor & 0x40) && !(m_pcjr_dor & 0x40))
			m_pcjr_watchdog->adjust(attotime::from_seconds(3));
	}
	else
	{
		m_pcjr_watchdog->adjust(attotime::never);
		m_pic8259->ir6_w(0);
	}
}

// pcjx port 0x1ff, some info from Toshiya Takeda

void pcjr_state::pcjx_set_bank(int unk1, int unk2, int unk3)
{
	logerror("pcjx: 0x1ff 0:%02x 1:%02x 2:%02x\n", unk1, unk2, unk3);
}

void pcjr_state::pcjx_port_1ff_w(uint8_t data)
{
	switch (m_pcjx_1ff_count)
	{
	case 0:
		m_pcjx_1ff_bankval = data;
		m_pcjx_1ff_count++;
		break;
	case 1:
		m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][0] = data;
		m_pcjx_1ff_count++;
		break;
	case 2:
		m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][1] = data;
		m_pcjx_1ff_count = 0;
		pcjx_set_bank(m_pcjx_1ff_bankval, m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][0], data);
		break;
	}
}

uint8_t pcjr_state::pcjx_port_1ff_r()
{
	if (m_pcjx_1ff_count == 2)
		pcjx_set_bank(m_pcjx_1ff_bankval, m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][0], m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][1]);

	m_pcjx_1ff_count = 0;
	return 0x60; // expansion?
}

std::pair<std::error_condition, std::string> pcjr_state::load_cart(
		device_image_interface &image,
		generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");
	bool imagic_hack = false;

	if (!image.loaded_through_softlist())
	{
		int header_size = 0;

		// Check for supported header sizes
		switch (size & 0x3ff)
		{
			case 0x80:
				header_size = 0x80;
				break;
			case 0x200:
				header_size = 0x200;
				break;
			default:
				return std::make_pair(image_error::INVALIDIMAGE, "Invalid header length (must be 128 bytes or 512 bytes)");
		}
		if ((size - header_size) == 0xa000)
		{
			// alloc 64K for the imagic carts, so as to handle the necessary mirroring
			size += 0x6000;
			imagic_hack = true;
		}

		size -= header_size;
		image.fseek(header_size, SEEK_SET);
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	if (imagic_hack)
	{
		// in this case the image consists of 2x8K chunks
		// the first chunk is unique, the second is repeated 4 times up to 0xa000 size

		// mirroring
		uint8_t *ROM = slot->get_rom_base();
		memcpy(ROM + 0xe000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0xc000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0xa000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0x8000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0x6000, ROM, 0x2000);
		memcpy(ROM + 0x4000, ROM, 0x2000);
		memcpy(ROM + 0x2000, ROM, 0x2000);
	}

	return std::make_pair(std::error_condition(), std::string());
}


static void pcjr_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

static void pcjr_com(device_slot_interface &device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal",SERIAL_TERMINAL);
	device.option_add("null_modem",NULL_MODEM);
}

static const gfx_layout pc_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static const gfx_layout kanji_layout =
{
	16, 16,                 /* 8 x 8 characters */
	RGN_FRAC(1,1),                  /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ STEP16(0,1) },
	/* y offsets */
	{ STEP16(0,16) },
	16*16                   /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_pcjr )
	GFXDECODE_ENTRY("gfx1", 0x0000, pc_8_charlayout, 3, 1)
GFXDECODE_END


void pcjr_state::ibmpcjr_map(address_map &map)
{
	map.unmap_value_high();
	map(0xb8000, 0xbffff).m("pcvideo_pcjr:vram", FUNC(address_map_bank_device::amap8));
	map(0xd0000, 0xdffff).r(m_cart2, FUNC(generic_slot_device::read_rom));
	map(0xe0000, 0xeffff).r(m_cart1, FUNC(generic_slot_device::read_rom));
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}


void pcjr_state::ibmpcjr_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0020, 0x0021).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x0043).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x0063).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x00a0, 0x00a0).rw(FUNC(pcjr_state::pcjr_nmi_enable_r), FUNC(pcjr_state::pc_nmi_enable_w));
	map(0x00c0, 0x00c0).w("sn76496", FUNC(sn76496_device::write));
	map(0x00f2, 0x00f2).w(FUNC(pcjr_state::pcjr_fdc_dor_w));
	map(0x00f4, 0x00f5).m(m_fdc, FUNC(upd765a_device::map));
	map(0x0200, 0x0207).rw("pc_joy", FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
	map(0x02f8, 0x02ff).rw("ins8250", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x0378, 0x037b).rw("lpt_0", FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write));
	map(0x03d0, 0x03df).r("pcvideo_pcjr", FUNC(pcvideo_pcjr_device::read)).w("pcvideo_pcjr", FUNC(pcvideo_pcjr_device::write));
}

void pcjr_state::ibmpcjx_map(address_map &map)
{
	map.unmap_value_high();
	map(0x80000, 0x9ffff).ram().share("vram"); // TODO: remove this part of vram hack
	map(0x80000, 0xb7fff).rom().region("kanji", 0);
	map(0xb8000, 0xbffff).m("pcvideo_pcjr:vram", FUNC(address_map_bank_device::amap8));
	map(0xd0000, 0xdffff).r(m_cart1, FUNC(generic_slot_device::read_rom));
	map(0xe0000, 0xfffff).rom().region("bios", 0);
}

void pcjr_state::ibmpcjx_io(address_map &map)
{
	map.unmap_value_high();
	ibmpcjr_io(map);
	map(0x01ff, 0x01ff).rw(FUNC(pcjr_state::pcjx_port_1ff_r), FUNC(pcjr_state::pcjx_port_1ff_w));
}

void pcjr_state::ibmpcjr(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, XTAL(14'318'181)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &pcjr_state::ibmpcjr_map);
	m_maincpu->set_addrmap(AS_IO, &pcjr_state::ibmpcjr_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

/*
  On the PC Jr the input for clock 1 seems to be selectable
  based on bit 4(/5?) written to output port A0h. This is not
  supported yet.
 */
	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(XTAL(14'318'181)/12);
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(XTAL(14'318'181)/12);
	m_pit8253->set_clk<2>(XTAL(14'318'181)/12);
	m_pit8253->out_handler<2>().set(FUNC(pcjr_state::out2_changed));

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set(FUNC(pcjr_state::pic8259_set_int_line));

	i8255_device &ppi(I8255(config, "ppi8255"));
	ppi.in_pa_callback().set_constant(0xff);
	ppi.out_pb_callback().set(FUNC(pcjr_state::pcjr_ppi_portb_w));
	ppi.in_pc_callback().set(FUNC(pcjr_state::pcjr_ppi_portc_r));

	ins8250_device &uart(INS8250(config, "ins8250", XTAL(1'843'200)));
	uart.out_tx_callback().set("serport", FUNC(rs232_port_device::write_txd));
	uart.out_dtr_callback().set("serport", FUNC(rs232_port_device::write_dtr));
	uart.out_rts_callback().set("serport", FUNC(rs232_port_device::write_rts));
	uart.out_int_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));

	rs232_port_device &serport(RS232_PORT(config, "serport", pcjr_com, nullptr));
	serport.rxd_handler().set("ins8250", FUNC(ins8250_uart_device::rx_w));
	serport.dcd_handler().set("ins8250", FUNC(ins8250_uart_device::dcd_w));
	serport.dsr_handler().set("ins8250", FUNC(ins8250_uart_device::dsr_w));
	serport.ri_handler().set("ins8250", FUNC(ins8250_uart_device::ri_w));
	serport.cts_handler().set("ins8250", FUNC(ins8250_uart_device::cts_w));

	/* video hardware */
	PCVIDEO_PCJR(config, "pcvideo_pcjr", 0).set_screen("pcvideo_pcjr:screen");

	GFXDECODE(config, "gfxdecode", "pcvideo_pcjr:palette", gfx_pcjr);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.80);
	SN76496(config, "sn76496", XTAL(14'318'181)/4).add_route(ALL_OUTPUTS, "mono", 0.80);

	/* printer */
	pc_lpt_device &lpt0(PC_LPT(config, "lpt_0"));
	lpt0.irq_handler().set(m_pic8259, FUNC(pic8259_device::ir7_w));

	PC_JOY(config, "pc_joy");

	/* cassette */
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	UPD765A(config, m_fdc, 8'000'000, false, false);

	FLOPPY_CONNECTOR(config, "fdc:0", pcjr_floppies, "525dd", isa8_fdc_device::floppy_formats, true);

	PC_KEYB(config, m_keyboard);
	m_keyboard->keypress().set(FUNC(pcjr_state::keyb_interrupt));

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot1", generic_plain_slot, "ibmpcjr_cart", "bin,jrc").set_device_load(FUNC(pcjr_state::cart1_load));
	GENERIC_CARTSLOT(config, "cartslot2", generic_plain_slot, "ibmpcjr_cart", "bin,jrc").set_device_load(FUNC(pcjr_state::cart2_load));

	/* internal ram */
	RAM(config, m_ram).set_default_size("640K").set_extra_options("128K, 256K, 512K");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("ibmpcjr_cart");
	SOFTWARE_LIST(config, "flop_list").set_original("ibmpcjr_flop");
	SOFTWARE_LIST(config, "pc_list").set_compatible("ibm5150");
}

static GFXDECODE_START( gfx_ibmpcjx )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_8_charlayout, 3, 1 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanji_layout, 3, 1 )
GFXDECODE_END

void pcjr_state::ibmpcjx(machine_config &config)
{
	ibmpcjr(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pcjr_state::ibmpcjx_map);
	m_maincpu->set_addrmap(AS_IO, &pcjr_state::ibmpcjx_io);

	config.device_remove("fdc:0");
	FLOPPY_CONNECTOR(config, "fdc:0", pcjr_floppies, "35dd", isa8_fdc_device::floppy_formats, true);
	FLOPPY_CONNECTOR(config, "fdc:1", pcjr_floppies, "35dd", isa8_fdc_device::floppy_formats, true);

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_ibmpcjx);

	/* internal ram */
	m_ram->set_default_size("512K").set_extra_options(""); // only boots with 512k currently
}



ROM_START( ibmpcjr )
	ROM_REGION(0x10000,"bios", 0)
	ROM_SYSTEM_BIOS( 0, "default", "Default" )
	ROMX_LOAD("bios.rom", 0x0000, 0x10000,CRC(31e3a7aa) SHA1(1f5f7013f18c08ff50d7942e76c4fbd782412414), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "quiksilver", "Quicksilver" ) // Alternate bios to boot up faster (Synectics)
	ROMX_LOAD("quiksilv.rom", 0x0000, 0x10000, CRC(86aaa1c4) SHA1(b3d7e8ce5de17441891e0b71e5261ed01a169dc1), ROM_BIOS(1))

	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card
ROM_END

ROM_START( ibmpcjx )
	ROM_REGION(0x20000,"bios", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("unk")
	ROM_SYSTEM_BIOS( 0, "5601jda", "5601jda" )
	ROMX_LOAD("5601jda.bin", 0x10000, 0x10000, CRC(b1e12366) SHA1(751feb16b985aa4f1ec1437493ff77e2ebd5e6a6), ROM_BIOS(0))
	ROMX_LOAD("basicjx.rom",   0x08000, 0x08000, NO_DUMP, ROM_BIOS(0)) // boot fails due to this.
	ROM_SYSTEM_BIOS( 1, "unk", "unk" )
	ROMX_LOAD("ipljx.rom", 0x00000, 0x20000, CRC(36a7b2de) SHA1(777db50c617725e149bca9b18cf51ce78f6dc548), ROM_BIOS(1))

	ROM_REGION(0x08100,"gfx1", 0) //TODO: needs a different charset
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card

	ROM_REGION(0x38000,"kanji", 0)
	ROM_LOAD("kanji.rom",     0x00000, 0x38000, BAD_DUMP CRC(eaa6e3c3) SHA1(35554587d02d947fae8446964b1886fff5c9d67f)) // hand-made rom
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS       INIT        COMPANY                            FULLNAME     FLAGS
COMP( 1983, ibmpcjr, ibm5150, 0,      ibmpcjr, ibmpcjr, pcjr_state, empty_init, "International Business Machines", "IBM PC Jr", MACHINE_IMPERFECT_COLORS )
COMP( 1985, ibmpcjx, ibm5150, 0,      ibmpcjx, ibmpcjr, pcjr_state, empty_init, "International Business Machines", "IBM PC JX", MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING)
