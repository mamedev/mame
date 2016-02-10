// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "cpu/i86/i86.h"
#include "sound/sn76496.h"
#include "sound/speaker.h"
#include "video/pc_t1t.h"
#include "machine/ins8250.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"
#include "machine/pc_fdc.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/ser_mouse.h"
#include "bus/pc_joy/pc_joy.h"
#include "bus/isa/fdc.h"
#include "imagedev/cassette.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist.h"

class pcjr_state : public driver_device
{
public:
	pcjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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

	DECLARE_WRITE_LINE_MEMBER(out2_changed);
	DECLARE_WRITE_LINE_MEMBER(keyb_interrupt);

	DECLARE_WRITE8_MEMBER(pc_nmi_enable_w);
	DECLARE_READ8_MEMBER(pcjr_nmi_enable_r);
	DECLARE_WRITE_LINE_MEMBER(pic8259_set_int_line);

	DECLARE_WRITE8_MEMBER(pcjr_ppi_portb_w);
	DECLARE_READ8_MEMBER(pcjr_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(pcjr_fdc_dor_w);
	DECLARE_READ8_MEMBER(pcjx_port_1ff_r);
	DECLARE_WRITE8_MEMBER(pcjx_port_1ff_w);
	void pcjx_set_bank(int unk1, int unk2, int unk3);

	int load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(pcjr_cart1) { return load_cart(image, m_cart1); }
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(pcjr_cart2) { return load_cart(image, m_cart2); }
	void pc_speaker_set_spkrdata(UINT8 data);

	UINT8 m_pc_spkrdata;
	UINT8 m_pit_out2;
	UINT8 m_pcjr_dor;
	UINT8 m_pcjx_1ff_count;
	UINT8 m_pcjx_1ff_val;
	UINT8 m_pcjx_1ff_bankval;
	UINT8 m_pcjx_1ff_bank[20][2];
	int m_ppi_portc_switch_high;
	UINT8 m_ppi_portb;

	UINT8 m_pc_keyb_data;
	UINT8 m_transferring;
	UINT8 m_latch;
	UINT32 m_raw_keyb_data;
	int m_signal_count;
	UINT8 m_nmi_enabled;

	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	emu_timer *m_pc_int_delay_timer;
	emu_timer *m_pcjr_watchdog;
	emu_timer *m_keyb_signal_timer;

	enum
	{
		TIMER_IRQ_DELAY,
		TIMER_WATCHDOG,
		TIMER_KB_SIGNAL
	};

	void machine_reset() override;
	DECLARE_DRIVER_INIT(pcjr);
};

static INPUT_PORTS_START( ibmpcjr )
	PORT_INCLUDE(pc_keyboard)

	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("pcvideo_pcjr:screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(pcjr_state, pcjr)
{
	m_pc_int_delay_timer = timer_alloc(TIMER_IRQ_DELAY);
	m_pcjr_watchdog = timer_alloc(TIMER_WATCHDOG);
	m_keyb_signal_timer = timer_alloc(TIMER_KB_SIGNAL);
	membank( "bank10" )->set_base( m_ram->pointer() );
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

void pcjr_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_IRQ_DELAY:
			m_maincpu->set_input_line(0, param ? ASSERT_LINE : CLEAR_LINE);
			break;

		case TIMER_WATCHDOG:
			if(m_pcjr_dor & 0x20)
				m_pic8259->ir6_w(1);
			else
				m_pic8259->ir6_w(0);
			break;

		case TIMER_KB_SIGNAL:
			m_raw_keyb_data = m_raw_keyb_data >> 1;
			m_signal_count--;

			if ( m_signal_count <= 0 )
			{
				m_keyb_signal_timer->adjust( attotime::never, 0, attotime::never );
				m_transferring = 0;
			}
			break;
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

WRITE_LINE_MEMBER(pcjr_state::pic8259_set_int_line)
{
	UINT32 pc = m_maincpu->pc();
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
void pcjr_state::pc_speaker_set_spkrdata(UINT8 data)
{
	m_pc_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}

WRITE_LINE_MEMBER(pcjr_state::out2_changed)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
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

WRITE_LINE_MEMBER(pcjr_state::keyb_interrupt)
{
	int data;

	if(state && (data = m_keyboard->read(machine().driver_data()->generic_space(), 0)))
	{
		UINT8   parity = 0;
		int     i;

		m_latch = 1;

		if(m_transferring)
			return;

		m_pc_keyb_data = data;

		/* Calculate the raw data */
		for( i = 0; i < 8; i++ )
		{
			if ( ( 1 << i ) & data )
			{
				parity ^= 1;
			}
		}
		m_raw_keyb_data = 0;
		m_raw_keyb_data = ( m_raw_keyb_data << 2 ) | ( parity ? 1 : 2 );
		for( i = 0; i < 8; i++ )
		{
			m_raw_keyb_data = ( m_raw_keyb_data << 2 ) | ( ( data & 0x80 ) ? 1 : 2 );
			data <<= 1;
		}
		/* Insert start bit */
		m_raw_keyb_data = ( m_raw_keyb_data << 2 ) | 1;
		m_signal_count = 20 + 22;

		/* we are now transferring a byte of keyboard data */
		m_transferring = 1;

		/* Set timer */
		m_keyb_signal_timer->adjust( attotime::from_usec(220), 0, attotime::from_usec(220) );
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_enabled && m_latch);
	}
}

READ8_MEMBER(pcjr_state::pcjr_nmi_enable_r)
{
	m_latch = 0;
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return m_nmi_enabled;
}

WRITE8_MEMBER(pcjr_state::pc_nmi_enable_w)
{
	m_nmi_enabled = data & 0x80;
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_nmi_enabled && m_latch);
}

WRITE8_MEMBER(pcjr_state::pcjr_ppi_portb_w)
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	m_cassette->change_state(( data & 0x08 ) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
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
READ8_MEMBER(pcjr_state::pcjr_ppi_portc_r)
{
	int data=0xff;

	data&=~0x80;
	data &= ~0x04;      /* floppy drive installed */
	if ( m_ram->size() > 64 * 1024 )    /* more than 64KB ram installed */
		data &= ~0x08;
	data = ( data & ~0x01 ) | ( m_latch ? 0x01: 0x00 );
	if ( ! ( m_ppi_portb & 0x08 ) )
	{
		double tap_val = m_cassette->input();

		if ( tap_val < 0 )
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
		if ( m_ppi_portb & 0x01 )
		{
			data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
		}
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );
	data = ( data & ~0x40 ) | ( ( m_raw_keyb_data & 0x01 ) ? 0x40 : 0x00 );

	return data;
}

WRITE8_MEMBER(pcjr_state::pcjr_fdc_dor_w)
{
	logerror("fdc: dor = %02x\n", data);
	UINT8 pdor = m_pcjr_dor;
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = nullptr;

	if(m_fdc->subdevice("1"))
		floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	m_pcjr_dor = data;

	if(floppy0)
		floppy0->mon_w(!(m_pcjr_dor & 1));
	if(floppy1)
		floppy1->mon_w(!(m_pcjr_dor & 2));

	if(m_pcjr_dor & 1)
		m_fdc->set_floppy(floppy0);
	else if(m_pcjr_dor & 2)
		m_fdc->set_floppy(floppy1);
	else
		m_fdc->set_floppy(nullptr);

	if((pdor^m_pcjr_dor) & 0x80)
		m_fdc->reset();

	if(m_pcjr_dor & 0x20) {
		if((pdor & 0x40) && !(m_pcjr_dor & 0x40))
			m_pcjr_watchdog->adjust(attotime::from_seconds(3));
	} else {
		m_pcjr_watchdog->adjust(attotime::never);
		m_pic8259->ir6_w(0);
	}
}

// pcjx port 0x1ff, some info from Toshiya Takeda

void pcjr_state::pcjx_set_bank(int unk1, int unk2, int unk3)
{
	logerror("pcjx: 0x1ff 0:%02x 1:%02x 2:%02x\n", unk1, unk2, unk3);
}

WRITE8_MEMBER(pcjr_state::pcjx_port_1ff_w)
{
	switch(m_pcjx_1ff_count) {
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

READ8_MEMBER(pcjr_state::pcjx_port_1ff_r)
{
	if(m_pcjx_1ff_count == 2)
		pcjx_set_bank(m_pcjx_1ff_bankval, m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][0], m_pcjx_1ff_bank[m_pcjx_1ff_bankval & 0x1f][1]);

	m_pcjx_1ff_count = 0;
	return 0x60; // expansion?
}

int pcjr_state::load_cart(device_image_interface &image, generic_slot_device *slot)
{
	UINT32 size = slot->common_get_size("rom");
	bool imagic_hack = false;

	if (image.software_entry() == nullptr)
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
				image.seterror(IMAGE_ERROR_UNSUPPORTED, "Invalid header size" );
				return IMAGE_INIT_FAIL;
		}
		if (size - header_size == 0xa000)
		{
			// alloc 64K for the imagic carts, so to handle the necessary mirroring
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
		UINT8 *ROM = slot->get_rom_base();
		memcpy(ROM + 0xe000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0xc000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0xa000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0x8000, ROM + 0x2000, 0x2000);
		memcpy(ROM + 0x6000, ROM, 0x2000);
		memcpy(ROM + 0x4000, ROM, 0x2000);
		memcpy(ROM + 0x2000, ROM, 0x2000);
	}
	return IMAGE_INIT_PASS;
}


static SLOT_INTERFACE_START( pcjr_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(pcjr_com)
	SLOT_INTERFACE("microsoft_mouse", MSFT_SERIAL_MOUSE)
	SLOT_INTERFACE("mousesys_mouse", MSYSTEM_SERIAL_MOUSE)
SLOT_INTERFACE_END

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

static GFXDECODE_START( pcjr )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_8_charlayout, 3, 1 )
GFXDECODE_END


static ADDRESS_MAP_START(ibmpcjr_map, AS_PROGRAM, 8, pcjr_state)
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_RAMBANK("bank14")
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xd0000, 0xdffff) AM_DEVREAD("cartslot2", generic_slot_device, read_rom)
	AM_RANGE(0xe0000, 0xeffff) AM_DEVREAD("cartslot1", generic_slot_device, read_rom)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(ibmpcjr_io, AS_IO, 8, pcjr_state)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x00a0, 0x00a0) AM_READWRITE(pcjr_nmi_enable_r, pc_nmi_enable_w )
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", sn76496_device, write)
	AM_RANGE(0x00f2, 0x00f2) AM_WRITE(pcjr_fdc_dor_w)
	AM_RANGE(0x00f4, 0x00f5) AM_DEVICE("fdc", upd765a_device, map)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE("ins8250", ins8250_device, ins8250_r, ins8250_w)
	AM_RANGE(0x0378, 0x037b) AM_DEVREADWRITE("lpt_0", pc_lpt_device, read, write)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE("pcvideo_pcjr", pcvideo_pcjr_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibmpcjx_map, AS_PROGRAM, 8, pcjr_state )
	AM_RANGE(0x80000, 0xb7fff) AM_ROM AM_REGION("kanji",0)
	AM_IMPORT_FROM( ibmpcjr_map )
ADDRESS_MAP_END

static ADDRESS_MAP_START(ibmpcjx_io, AS_IO, 8, pcjr_state)
	AM_RANGE(0x01ff, 0x01ff) AM_READWRITE(pcjx_port_1ff_r, pcjx_port_1ff_w)
	AM_IMPORT_FROM( ibmpcjr_io )
ADDRESS_MAP_END

static MACHINE_CONFIG_START( ibmpcjr, pcjr_state)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 4900000)
	MCFG_CPU_PROGRAM_MAP(ibmpcjr_map)
	MCFG_CPU_IO_MAP(ibmpcjr_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

/*
  On the PC Jr the input for clock 1 seems to be selectable
  based on bit 4(/5?) written to output port A0h. This is not
  supported yet.
 */
	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz/12)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz/12)
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz/12)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pcjr_state, out2_changed))

	MCFG_PIC8259_ADD( "pic8259", WRITELINE(pcjr_state, pic8259_set_int_line), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(CONSTANT(0xff))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(pcjr_state, pcjr_ppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(pcjr_state, pcjr_ppi_portc_r))

	MCFG_DEVICE_ADD( "ins8250", INS8250, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("serport", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("serport", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("serport", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))

	MCFG_RS232_PORT_ADD( "serport", pcjr_com, nullptr )
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ins8250", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ins8250", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ins8250", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ins8250", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ins8250", ins8250_uart_device, cts_w))

	/* video hardware */
	MCFG_PCVIDEO_PCJR_ADD("pcvideo_pcjr")

	MCFG_GFXDECODE_ADD("gfxdecode", "pcvideo_pcjr:palette", pcjr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
	MCFG_SOUND_ADD("sn76496", SN76496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* printer */
	MCFG_DEVICE_ADD("lpt_0", PC_LPT, 0)
	MCFG_PC_LPT_IRQ_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir7_w))

	MCFG_PC_JOY_ADD("pc_joy")

	/* cassette */
	MCFG_CASSETTE_ADD( "cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_UPD765A_ADD("fdc", false, false)

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pcjr_floppies, "525dd", isa8_fdc_device::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_PC_KEYB_ADD("pc_keyboard", WRITELINE(pcjr_state, keyb_interrupt))

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot1", generic_plain_slot, "ibmpcjr_cart")
	MCFG_GENERIC_EXTENSIONS("bin,jrc")
	MCFG_GENERIC_LOAD(pcjr_state, pcjr_cart1)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot2", generic_plain_slot, "ibmpcjr_cart")
	MCFG_GENERIC_EXTENSIONS("bin,jrc")
	MCFG_GENERIC_LOAD(pcjr_state, pcjr_cart2)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","ibmpcjr_cart")
	MCFG_SOFTWARE_LIST_ADD("flop_list","ibmpcjr_flop")
MACHINE_CONFIG_END

static GFXDECODE_START( ibmpcjx )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_8_charlayout, 3, 1 )
	GFXDECODE_ENTRY( "kanji", 0x0000, kanji_layout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_DERIVED( ibmpcjx, ibmpcjr )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ibmpcjx_map)
	MCFG_CPU_IO_MAP(ibmpcjx_io)

	MCFG_DEVICE_REMOVE("fdc:0");
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pcjr_floppies, "35dd", isa8_fdc_device::floppy_formats)
	MCFG_SLOT_FIXED(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pcjr_floppies, "35dd", isa8_fdc_device::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_GFXDECODE_MODIFY("gfxdecode", ibmpcjx)
MACHINE_CONFIG_END



ROM_START( ibmpcjr )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("bios.rom", 0xf0000, 0x10000,CRC(31e3a7aa) SHA1(1f5f7013f18c08ff50d7942e76c4fbd782412414))

	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("cga.chr",     0x00000, 0x01000, CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card
ROM_END

ROM_START( ibmpcjx )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("unk")
	ROM_SYSTEM_BIOS( 0, "5601jda", "5601jda" )
	ROMX_LOAD("5601jda.bin", 0xf0000, 0x10000, CRC(b1e12366) SHA1(751feb16b985aa4f1ec1437493ff77e2ebd5e6a6), ROM_BIOS(1))
	ROMX_LOAD("basicjx.rom",   0xe8000, 0x08000, NO_DUMP, ROM_BIOS(1)) // boot fails due of this.
	ROM_SYSTEM_BIOS( 1, "unk", "unk" )
	ROMX_LOAD("ipljx.rom", 0xe0000, 0x20000, CRC(36a7b2de) SHA1(777db50c617725e149bca9b18cf51ce78f6dc548), ROM_BIOS(2))

	ROM_REGION(0x08100,"gfx1", 0) //TODO: needs a different charset
	ROM_LOAD("cga.chr",     0x00000, 0x01000, BAD_DUMP CRC(42009069) SHA1(ed08559ce2d7f97f68b9f540bddad5b6295294dd)) // from an unknown clone cga card

	ROM_REGION(0x38000,"kanji", 0)
	ROM_LOAD("kanji.rom",     0x00000, 0x38000, BAD_DUMP CRC(eaa6e3c3) SHA1(35554587d02d947fae8446964b1886fff5c9d67f)) // hand-made rom
ROM_END

/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
// pcjr
COMP( 1983, ibmpcjr,    ibm5150,    0,          ibmpcjr,    ibmpcjr, pcjr_state,    pcjr,       "International Business Machines", "IBM PC Jr", MACHINE_IMPERFECT_COLORS )
COMP( 1985, ibmpcjx,    ibm5150,    0,          ibmpcjx,    ibmpcjr, pcjr_state,    pcjr,       "International Business Machines", "IBM PC JX", MACHINE_IMPERFECT_COLORS | MACHINE_NOT_WORKING)
