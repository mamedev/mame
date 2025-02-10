// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05commonsys.h"

/*
    Both the Elan EU3A05 and EU3A14 CPU types implement some kind of custom interrupt handling

    It isn't clear if this is a completely new addition to the CPU, or just an interface / controller
    sitting on top of the existing NMI or IRQ support in the core providing custom vectors.

    The interrupt handlers are 16 4-byte entries starting at 0xffb0 in memory space

*/


/*
    -----------------------
    Custom Interrupt purposes
    -----------------------

    TETRIS  (enables 5007 : 0a, 5008: 0f)

    ffb0 (enabled)
    nothing of note?

    ffb4 (enabled)
    stuff with 500e, 500c and 500d

    ffb8 (enabled)
    stuff with 50a4 / 50a5 / 50a6  and memory address e2

    ffbc (enabled)
    stuff with 50a4 / 50a5 / 50a6  and memory address e2 (similar to above, different bits)

    ffc0 - doesn't exist
    ffc4 - doesn't exist
    ffc8 - doesn't exist
    ffd0 - doesn't exist

    ffd4 (enabled)
    main irq?

    ffd8
    jumps straight to an rti

    ffdc (enabled) - probably P2 input related? ADC interrupt?
    accesses 501d / 501b

    -----------------------

    SPACE INVADERS

    ffb0
    rti

    ffb4
    rti

    ffb8  (enabled by phoenix)
    rti

    ffbc  (enabled by phoenix)
    decreases 301  bit 02
    stuff with 50a5

    ffc0  (enabled by phoenix)
    decreases 302
    stuff with 50a5 bit 04

    ffc4  (enabled by phoenix)
    decreases 303
    stuff with 50a5  bit 08

    ffc8  (enabled by phoenix)
    decreases 304
    stuff with 50a5  bit 10

    ffcc  (enabled by phoenix)
    uses 307
    stuff with 50a5  bit 20

    ffd0
    dead loop

    ffd4  (enabled by all games)
    main interrupt

    ffd8
    dead loop

    ffdc
    dead loop

    ffe0
    dead loop

    ffe4
    rti

    ffe8
    dead loop

    ffec
    dead loop

    -----------------------

    AIR BLASTER JOYSTICK

    all these 60xx jumps expect bank 00 or 0e or 3a or 7d to be active, so IRQs must be masked

    ffb0: jmp to 6000  (ends up jumping to pointer from RAM)
    ffb4: jmp to e08e  (stuff with 500c/500d/506e etc.)
    ffb8: jmp to 601c  (stub handler) (has function in bank 0e - writes 00 then 01 to 50a5)
    ffbc: jmp to 602a  (stub handler)
    ffc0: jmp to 6038  (stub handler)
    ffc4: jmp to 6046  (stub handler)
    ffc8: jmp to 6054  (stub handler)
    ffcc: jmp to 6062  (stub handler)
    ffd0: jmp to 6070  (stub handler)
    ffd4: jmp to 607e  (valid code - main IRQ?)
    ffd8: jmp to 608c  (stub handler)
    ffdc: jmp to 609a  (stub handler)
    ffe0: jmp to 60a8  (stub handler)
    ffe4: jmp to 60b6  (stub handler)
    ffe8: jmp to 60c4  (stub handler)
    ffec: jmp to 60d2  (stub handler)

    fff0: 7d

    fffa: e0 60 (60e0 vector) (stub handler)
    fffc: 88 e1 (e188 startup vector)
    fffe: 02 e0 (e002 vector)


    -----------------------

    GOLDEN TEE HOME

    ffb0  rti
    ffb4  rti
    ffb8  rti
    ffbc  rti

    ffc0  rti
    ffc4  rti
    ffc8  rti
    ffcc  rti

    ffd0  rti
    ffd4  main irq?
    ffd8  rti
    ffdc  rti

    ffe0  something with 5045 bit 0x08 and 9d in ram (increase or decrease)  (ADC interrupt)
    ffe4  something with 5045 bit 0x20 and 9c in ram (increase of decrease)  (ADC interrupt)

    ffe8  rti
    ffec  rti

    regular NMI (e3f0 - jump to ($19e2) which seems to point to rti, but could move..)
    regular IRQ (e3f3 - points to rti)

*/


DEFINE_DEVICE_TYPE(ELAN_EU3A05_COMMONSYS, elan_eu3a05commonsys_device, "elan_eu3a05commonsys", "Elan EU3A05/EU3A14 Common System")


elan_eu3a05commonsys_device::elan_eu3a05commonsys_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_is_pal(false),
	m_allow_timer_irq(true),
	m_whichtimer(0)
{
}

elan_eu3a05commonsys_device::elan_eu3a05commonsys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonsys_device(mconfig, ELAN_EU3A05_COMMONSYS, tag, owner, clock)
{
}

/*

rad_bb3 plays with address 0x5009 in this way, but never sets it, something else must set it (and 1->0 is 'activate' or 'acknowledge')

lda $5009
and #$ef
sta $5009

lda $5009
and #$df
sta $5009

0x5006 looks interesting too, again just seems to be masking out bits

(as one block of code)
lda $5006
and #$f0
sta $5006
lda $5006
and $#8f
sta $5006

todo: investigate rad_hnt3 which polls this address

*/

void elan_eu3a05commonsys_device::map(address_map &map)
{
	map(0x00, 0x00).ram();
	map(0x01, 0x01).ram();
	// 5002
	map(0x03, 0x03).r(FUNC(elan_eu3a05commonsys_device::elan_eu3a05_5003_r));
	map(0x04, 0x04).ram();
	// 5005
	map(0x06, 0x06).ram();
	map(0x07, 0x08).rw(FUNC(elan_eu3a05commonsys_device::intmask_r), FUNC(elan_eu3a05commonsys_device::intmask_w));
	map(0x09, 0x09).r(FUNC(elan_eu3a05commonsys_device::radica_5009_unk_r)); // rad_hnt3 polls this on startup
	map(0x0a, 0x0a).ram();
	map(0x0b, 0x0b).rw(FUNC(elan_eu3a05commonsys_device::elan_eu3a05_pal_ntsc_r), FUNC(elan_eu3a05commonsys_device::elan_eu3a05_500b_unk_w)); // PAL / NTSC flag at least
	map(0x0c, 0x0d).rw(FUNC(elan_eu3a05commonsys_device::elan_eu3a05_rombank_r), FUNC(elan_eu3a05commonsys_device::elan_eu3a05_rombank_w));
	// 0e
}


TIMER_CALLBACK_MEMBER(elan_eu3a05commonsys_device::unknown_timer_tick)
{
	// rad_bb3 unmasks the interrupt, but the jumps use pointers in RAM, which haven't been set up at the time
	// of unmasking, so we need to find some kind of global enable / disable, or timer enable.
	if (m_allow_timer_irq)
		generate_custom_interrupt(m_whichtimer);
}


void elan_eu3a05commonsys_device::device_start()
{
	save_item(NAME(m_rombank_lo));
	save_item(NAME(m_rombank_hi));
	save_item(NAME(m_intmask));
	save_item(NAME(m_custom_irq));
	save_item(NAME(m_custom_nmi));
	save_item(NAME(m_custom_irq_vector));
	save_item(NAME(m_custom_nmi_vector));

	m_unk_timer = timer_alloc(FUNC(elan_eu3a05commonsys_device::unknown_timer_tick), this);
	m_unk_timer->adjust(attotime::never);
}

void elan_eu3a05commonsys_device::device_reset()
{
	// all interrupts disabled?
	m_intmask[0] = 0x00;
	m_intmask[1] = 0x00;

	m_custom_irq = 0;
	m_custom_irq_vector = 0x0000;

	m_custom_nmi = 0;
	m_custom_nmi_vector = 0x0000;

	m_rombank_lo = 0x7f;
	m_rombank_hi = 0x00;
	m_bank->set_bank(0x7f);

	// generate at a fixed frequency for now, but can probably be configured.  drives 3D stages in Air Blaster Joystick
	m_unk_timer->adjust(attotime::from_hz(4096), 0, attotime::from_hz(2048));
}

uint8_t elan_eu3a05commonsys_device::intmask_r(offs_t offset)
{
	return m_intmask[offset];
}

void elan_eu3a05commonsys_device::intmask_w(offs_t offset, uint8_t data)
{
	m_intmask[offset] = data;
}


void elan_eu3a05commonsys_device::generate_custom_interrupt(int level)
{
	// Air Blaster uses brk in the code, which is problematic for custom IRQs
	//  m_custom_irq = 1;
	//  m_custom_irq_vector = 0xffd4;
	//  m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);

	// 5007        5008
	// --ee --v-   ssss ss-t
	//   10        5432 10
	// vector = 0xffb0 + 4 * bit position from right

	// each bit seems to relate to an IRQ level
	// v = vbl interrupt bit (vector 0xffd4)
	// t = used for object movement (enemies / bullets) on air blaster chase levels (vector 0xffb0) needs to be frequent, timer? or hbl?
	// s = sound channel related? (air blaster enables s1)
	// e = 'event' interrupts (rad_gtg)

	// vbl irq masking is important for air blaster or it will suffer from random crashes

	uint16_t intmask = (m_intmask[0] << 8) | m_intmask[1];

	if (intmask & (1 << level))
	{
		m_custom_nmi = 1;
		m_custom_nmi_vector = 0xffb0 + level * 4;
		m_cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}

uint8_t elan_eu3a05commonsys_device::nmi_vector_r(offs_t offset)
{
	if (m_custom_nmi)
	{
		return m_custom_nmi_vector >> (offset*8);
	}
	else
	{
		if(machine().side_effects_disabled())
			return 0x00;

		logerror("NMI without custom vector!");

		return 0x00;
	}
}

// not currently used
uint8_t elan_eu3a05commonsys_device::irq_vector_r(offs_t offset)
{
	if(machine().side_effects_disabled())
		return 0x00;

	if (m_custom_irq)
	{
		return m_custom_irq_vector >> (offset*8);
	}
	else
	{
		if(machine().side_effects_disabled())
			return 0x00;

		fatalerror("IRQ without custom vector!");
	}
}


void elan_eu3a05commonsys_device::elan_eu3a05_rombank_w(offs_t offset, uint8_t data)
{
	if (offset == 0x00)
	{
		// written with the banking?
		//logerror("%s: elan_eu3a05_rombank_hi_w (set ROM bank) %02x\n", machine().describe_context(), data);
		m_rombank_hi = data;

		m_bank->set_bank(m_rombank_lo | (m_rombank_hi << 8));
	}
	else
	{
		//logerror("%s: elan_eu3a05_rombank_lo_w (select ROM bank) %02x\n", machine().describe_context(), data);
		m_rombank_lo = data;
	}
}

uint8_t elan_eu3a05commonsys_device::elan_eu3a05_rombank_r(offs_t offset)
{
	if (offset == 0x00)
	{
		return m_rombank_hi;
	}
	else
	{
		return m_rombank_lo;
	}
}


uint8_t elan_eu3a05commonsys_device::elan_eu3a05_pal_ntsc_r()
{
	// the text under the radica logo differs between regions, sometimes the titles too
	logerror("%s: elan_eu3a05_pal_ntsc_r (region + more?)\n", machine().describe_context());
	if (!m_is_pal) return 0xff; // NTSC
	else return 0x00; // PAL
}

uint8_t elan_eu3a05commonsys_device::elan_eu3a05_5003_r()
{
	/* masked with 0x0f, 0x01 or 0x03 depending on situation..

	  I think it might just be an RNG because if you return 0x00
	  your shots can never hit the stage 3 enemies in Phoenix and
	  if you return 0xff they always hit.  On the real hardware it
	  seems to be random.  Could also just be a crude frame counter
	  used for the same purpose.

	*/

	logerror("%s: elan_eu3a05_5003_r (RNG?)\n", machine().describe_context());

	return machine().rand();
}



void elan_eu3a05commonsys_device::elan_eu3a05_500b_unk_w(uint8_t data)
{
	// this is the PAL / NTSC flag when read, so what are writes?
	logerror("%s: elan_eu3a05_500b_unk_w %02x\n", machine().describe_context(), data);
}
