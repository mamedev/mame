/******************************************************************************
 *  Microtan 65
 *
 *  machine driver
 *
 *  Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *
 *  Thanks go to Geoff Macdonald <mail@geoff.org.uk>
 *  for his site http:://www.geo255.redhotant.com
 *  and to Fabrice Frances <frances@ensica.fr>
 *  for his site http://www.ifrance.com/oric/microtan.html
 *
 *****************************************************************************/

/* Core includes */
#include "emu.h"
#include "includes/microtan.h"

/* Components */
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6551acia.h"
#include "sound/ay8910.h"

/* Devices */
#include "imagedev/cassette.h"
//#include "imagedev/snapquik.h"


#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)




static const char keyboard[8][9][8] = {
    { /* normal */
        { 27,'1','2','3','4','5','6','7'},
        {'8','9','0','-','=','`',127,  9},
        {'q','w','e','r','t','y','u','i'},
        {'o','p','[',']', 13,127,  0,  0},
        {'a','s','d','f','g','h','j','k'},
        {'l',';', 39, 92,  0,'z','x','c'},
        {'v','b','n','m',',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Shift */
        { 27,'!','@','#','$','%','^','&'},
        {'*','(',')','_','+','~',127,  9},
        {'Q','W','E','R','T','Y','U','I'},
        {'O','P','{','}', 13,127,  0,  0},
        {'A','S','D','F','G','H','J','K'},
        {'L',':','"','|',  0,'Z','X','C'},
        {'V','B','N','M','<','>','?',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Control */
        { 27,'1','2','3','4','5','6','7'},
        {'8','9','0','-','=','`',127,  9},
        { 17, 23,  5, 18, 20, 25, 21,  9},
        { 15, 16, 27, 29, 13,127,  0,  0},
        {  1, 19,  4,  6,  7,  8, 10, 11},
        { 12,';', 39, 28,  0, 26, 24,  3},
        { 22,  2, 14, 13,',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Shift+Control */
        { 27,'!',  0,'#','$','%', 30,'&'},
        {'*','(',')', 31,'+','~',127,  9},
        { 17, 23,  5, 18, 20, 25, 21,  9},
        { 15, 16, 27, 29, 13,127,  0,  0},
        {  1, 19,  4,  6,  7,  8, 10, 11},
        { 12,':','"', 28,  0, 26, 24,  3},
        { 22,  2, 14, 13,',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* CapsLock */
        { 27,'1','2','3','4','5','6','7'},
        {'8','9','0','-','=','`',127,  9},
        {'Q','W','E','R','T','Y','U','I'},
        {'O','P','[',']', 13,127,  0,  0},
        {'A','S','D','F','G','H','J','K'},
        {'L',';', 39, 92,  0,'Z','X','C'},
        {'V','B','N','M',',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Shift+CapsLock */
        { 27,'!','@','#','$','%','^','&'},
        {'*','(',')','_','+','~',127,  9},
        {'Q','W','E','R','T','Y','U','I'},
        {'O','P','{','}', 13,127,  0,  0},
        {'A','S','D','F','G','H','J','K'},
        {'L',':','"','|',  0,'Z','X','C'},
        {'V','B','N','M','<','>','?',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Control+CapsLock */
        { 27,'1','2','3','4','5','6','7'},
        {'8','9','0','-','=','`',127,  9},
        { 17, 23,  5, 18, 20, 25, 21,  9},
        { 15, 16, 27, 29, 13,127,  0,  0},
        {  1, 19,  4,  6,  7,  8, 10, 11},
        { 12,';', 39, 28,  0, 26, 24,  3},
        { 22,  2, 14, 13,',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
    { /* Shift+Control+CapsLock */
        { 27,'!',  0,'#','$','%', 30,'&'},
        {'*','(',')', 31,'+','~',127,  9},
        { 17, 23,  5, 18, 20, 25, 21,  9},
        { 15, 16, 27, 29, 13,127,  0,  0},
        {  1, 19,  4,  6,  7,  8, 10, 11},
        { 12,':','"', 28,  0, 26, 24,  3},
        { 22,  2, 14, 13,',','.','/',  0},
        { 10,' ','-',',', 13,'.','0','1'},
        {'2','3','4','5','6','7','8','9'},
    },
};

static UINT8 read_dsw(running_machine &machine)
{
	UINT8 result;
	switch(machine.phase())
	{
		case MACHINE_PHASE_RESET:
		case MACHINE_PHASE_RUNNING:
			result = machine.root_device().ioport("DSW")->read();
			break;

		default:
			result = 0x00;
			break;
	}
	return result;
}

static void microtan_set_irq_line(running_machine &machine)
{
	microtan_state *state = machine.driver_data<microtan_state>();
    /* The 6502 IRQ line is active low and probably driven
       by open collector outputs (guess). Since MAME/MESS use
       a non-0 value for ASSERT_LINE we OR the signals here */
    machine.device("maincpu")->execute().set_input_line(0, state->m_via_0_irq_line | state->m_via_1_irq_line | state->m_kbd_irq_line);
}

static cassette_image_device *cassette_device_image(running_machine &machine)
{
	return machine.device<cassette_image_device>(CASSETTE_TAG);
}

/**************************************************************
 * VIA callback functions for VIA #0
 **************************************************************/
static READ8_DEVICE_HANDLER (via_0_in_a )
{
    int data = device->machine().root_device().ioport("JOY")->read();
    LOG(("microtan_via_0_in_a %02X\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_0_in_b )
{
    int data = 0xff;
    LOG(("microtan_via_0_in_b %02X\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_0_in_ca1 )
{
    int data = 1;
    LOG(("microtan_via_0_in_ca1 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_0_in_cb1 )
{
    int data = 1;
    LOG(("microtan_via_0_in_cb1 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_0_in_ca2 )
{
    int data = 1;
    LOG(("microtan_via_0_in_ca2 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_0_in_cb2 )
{
    int data = 1;
    LOG(("microtan_via_0_in_cb2 %d\n", data));
    return data;
}

static WRITE8_DEVICE_HANDLER ( via_0_out_a )
{
    LOG(("microtan_via_0_out_a %02X\n", data));
}

static WRITE8_DEVICE_HANDLER (via_0_out_b )
{
    LOG(("microtan_via_0_out_b %02X\n", data));
    /* bit #7 is the cassette output signal */
    cassette_device_image(device->machine())->output(data & 0x80 ? +1.0 : -1.0);
}

static WRITE8_DEVICE_HANDLER ( via_0_out_ca2 )
{
    LOG(("microtan_via_0_out_ca2 %d\n", data));
}

static WRITE8_DEVICE_HANDLER (via_0_out_cb2 )
{
    LOG(("microtan_via_0_out_cb2 %d\n", data));
}

static void via_0_irq(device_t *device, int state)
{
	microtan_state *drvstate = device->machine().driver_data<microtan_state>();
    LOG(("microtan_via_0_irq %d\n", state));
    drvstate->m_via_0_irq_line = state;
    microtan_set_irq_line(device->machine());
}

/**************************************************************
 * VIA callback functions for VIA #1
 **************************************************************/
static READ8_DEVICE_HANDLER ( via_1_in_a )
{
    int data = 0xff;
    LOG(("microtan_via_1_in_a %02X\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_1_in_b )
{
    int data = 0xff;
    LOG(("microtan_via_1_in_b %02X\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_1_in_ca1 )
{
    int data = 1;
    LOG(("microtan_via_1_in_ca1 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_1_in_cb1 )
{
    int data = 1;
    LOG(("microtan_via_1_in_cb1 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_1_in_ca2 )
{
    int data = 1;
    LOG(("microtan_via_1_in_ca2 %d\n", data));
    return data;
}

static READ8_DEVICE_HANDLER ( via_1_in_cb2 )
{
    int data = 1;
    LOG(("microtan_via_1_in_cb2 %d\n", data));
    return data;
}

static WRITE8_DEVICE_HANDLER ( via_1_out_a )
{
    LOG(("microtan_via_1_out_a %02X\n", data));
}

static WRITE8_DEVICE_HANDLER ( via_1_out_b )
{
    LOG(("microtan_via_1_out_b %02X\n", data));
}

static WRITE8_DEVICE_HANDLER (via_1_out_ca2 )
{
    LOG(("microtan_via_1_out_ca2 %d\n", data));
}

static WRITE8_DEVICE_HANDLER ( via_1_out_cb2 )
{
    LOG(("microtan_via_1_out_cb2 %d\n", data));
}

static void via_1_irq(device_t *device, int state)
{
	microtan_state *drvstate = device->machine().driver_data<microtan_state>();
    LOG(("microtan_via_1_irq %d\n", state));
    drvstate->m_via_1_irq_line = state;
    microtan_set_irq_line(device->machine());
}

/**************************************************************
 * VIA interface structure
 **************************************************************/
const via6522_interface microtan_via6522_0 =
{
	/* VIA#1 at bfc0-bfcf*/
	DEVCB_HANDLER(via_0_in_a),   DEVCB_HANDLER(via_0_in_b),
	DEVCB_HANDLER(via_0_in_ca1), DEVCB_HANDLER(via_0_in_cb1),
	DEVCB_HANDLER(via_0_in_ca2), DEVCB_HANDLER(via_0_in_cb2),
	DEVCB_HANDLER(via_0_out_a),  DEVCB_HANDLER(via_0_out_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_HANDLER(via_0_out_ca2),DEVCB_HANDLER(via_0_out_cb2),
	DEVCB_LINE(via_0_irq)
};

const via6522_interface microtan_via6522_1 =
{
	/* VIA#1 at bfe0-bfef*/
	DEVCB_HANDLER(via_1_in_a),   DEVCB_HANDLER(via_1_in_b),
	DEVCB_HANDLER(via_1_in_ca1), DEVCB_HANDLER(via_1_in_cb1),
	DEVCB_HANDLER(via_1_in_ca2), DEVCB_HANDLER(via_1_in_cb2),
	DEVCB_HANDLER(via_1_out_a),  DEVCB_HANDLER(via_1_out_b),
	DEVCB_NULL, DEVCB_NULL,
	DEVCB_HANDLER(via_1_out_ca2),DEVCB_HANDLER(via_1_out_cb2),
	DEVCB_LINE(via_1_irq)
};

static TIMER_CALLBACK(microtan_read_cassette)
{
	double level = (cassette_device_image(machine))->input();
	via6522_device *via_0 = machine.device<via6522_device>("via6522_0");

	LOG(("microtan_read_cassette: %g\n", level));
	if (level < -0.07)
		via_0->write_cb2(0);
	else if (level > +0.07)
		via_0->write_cb2(1);
}

READ8_MEMBER(microtan_state::microtan_sound_r)
{
    int data = 0xff;
    LOG(("microtan_sound_r: -> %02x\n", data));
    return data;
}

WRITE8_MEMBER(microtan_state::microtan_sound_w)
{
    LOG(("microtan_sound_w: <- %02x\n", data));
}


READ8_MEMBER(microtan_state::microtan_bffx_r)
{
    int data = 0xff;
    switch( offset & 3 )
    {
    case  0: /* BFF0: read enables chunky graphics */
        m_chunky_graphics = 1;
        LOG(("microtan_bff0_r: -> %02x (chunky graphics on)\n", data));
        break;
    case  1: /* BFF1: read undefined (?) */
        LOG(("microtan_bff1_r: -> %02x\n", data));
        break;
    case  2: /* BFF2: read undefined (?) */
        LOG(("microtan_bff2_r: -> %02x\n", data));
        break;
    default: /* BFF3: read keyboard ASCII value */
        data = m_keyboard_ascii;
        LOG(("microtan_bff3_r: -> %02x (keyboard ASCII)\n", data));
    }
    return data;
}


/* This callback is called one clock cycle after BFF2 is written (delayed nmi) */
static TIMER_CALLBACK(microtan_pulse_nmi)
{
    machine.device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(microtan_state::microtan_bffx_w)
{
    switch( offset & 3 )
    {
    case 0: /* BFF0: write reset keyboard interrupt flag */
        /* This removes bit 7 from the ASCII value of the last key pressed. */
        LOG(("microtan_bff0_w: %d <- %02x (keyboard IRQ clear )\n", offset, data));
        m_keyboard_ascii &= ~0x80;
        m_kbd_irq_line = CLEAR_LINE;
        microtan_set_irq_line(machine());
        break;
    case 1: /* BFF1: write delayed NMI */
        LOG(("microtan_bff1_w: %d <- %02x (delayed NMI)\n", offset, data));
        machine().scheduler().timer_set(machine().device<cpu_device>("maincpu")->cycles_to_attotime(8), FUNC(microtan_pulse_nmi));
        break;
    case 2: /* BFF2: write keypad column write (what is this meant for?) */
        LOG(("microtan_bff2_w: %d <- %02x (keypad column)\n", offset, data));
        m_keypad_column = data;
        break;
    default: /* BFF3: write disable chunky graphics */
        LOG(("microtan_bff3_w: %d <- %02x (chunky graphics off)\n", offset, data));
        m_chunky_graphics = 0;
    }
}

static void store_key(running_machine &machine, int key)
{
	microtan_state *state = machine.driver_data<microtan_state>();
    LOG(("microtan: store key '%c'\n", key));
    state->m_keyboard_ascii = key | 0x80;
    state->m_kbd_irq_line = ASSERT_LINE;
    microtan_set_irq_line(machine);
}

INTERRUPT_GEN( microtan_interrupt )
{
	microtan_state *state = device->machine().driver_data<microtan_state>();
    int mod, row, col, chg, newvar;
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };

    if( state->m_repeat )
    {
        if( !--state->m_repeat )
            state->m_repeater = 4;
    }
    else if( state->m_repeater )
    {
        state->m_repeat = state->m_repeater;
    }


    row = 9;
    newvar = device->machine().root_device().ioport("ROW8")->read();
    chg = state->m_keyrows[--row] ^ newvar;

	while ( !chg && row > 0)
	{
		newvar = device->machine().root_device().ioport(keynames[row - 1])->read();
		chg = state->m_keyrows[--row] ^ newvar;
	}
    if (!chg)
		--row;

    if (row >= 0)
    {
        state->m_repeater = 0x00;
        state->m_mask = 0x00;
        state->m_key = 0x00;
        state->m_lastrow = row;
        /* CapsLock LED */
		if( row == 3 && chg == 0x80 )
			set_led_status(device->machine(), 1, (state->m_keyrows[3] & 0x80) ? 0 : 1);

        if (newvar & chg)  /* key(s) pressed ? */
        {
            mod = 0;

            /* Shift modifier */
            if ( (state->m_keyrows[5] & 0x10) || (state->m_keyrows[6] & 0x80) )
                mod |= 1;

            /* Control modifier */
            if (state->m_keyrows[3] & 0x40)
                mod |= 2;

            /* CapsLock modifier */
            if (state->m_keyrows[3] & 0x80)
                mod |= 4;

            /* find newvar key */
            state->m_mask = 0x01;
            for (col = 0; col < 8; col ++)
            {
                if (chg & state->m_mask)
                {
                    newvar &= state->m_mask;
                    state->m_key = keyboard[mod][row][col];
                    break;
                }
                state->m_mask <<= 1;
            }
            if( state->m_key )   /* normal key */
            {
                state->m_repeater = 30;
                store_key(device->machine(), state->m_key);
            }
            else
            if( (row == 0) && (chg == 0x04) ) /* Ctrl-@ (NUL) */
                store_key(device->machine(), 0);
            state->m_keyrows[row] |= newvar;
        }
        else
        {
            state->m_keyrows[row] = newvar;
        }
        state->m_repeat = state->m_repeater;
    }
    else
    if ( state->m_key && (state->m_keyrows[state->m_lastrow] & state->m_mask) && state->m_repeat == 0 )
    {
        store_key(device->machine(), state->m_key);
    }
}

DRIVER_INIT_MEMBER(microtan_state,microtan)
{
    UINT8 *dst = memregion("gfx2")->base();
    int i;
    address_space *space = machine().device("maincpu")->memory().space(AS_PROGRAM);

    for (i = 0; i < 256; i++)
    {
        switch (i & 3)
        {
        case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
        case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
        case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
        case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
        }
        dst += 4;
        switch ((i >> 2) & 3)
        {
        case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
        case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
        case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
        case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
        }
        dst += 4;
        switch ((i >> 4) & 3)
        {
        case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
        case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
        case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
        case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
        }
        dst += 4;
        switch ((i >> 6) & 3)
        {
        case 0: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x00; break;
        case 1: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xf0; break;
        case 2: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0x0f; break;
        case 3: dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = 0xff; break;
        }
        dst += 4;
    }

    switch (read_dsw(machine()) & 3)
    {
        case 0:  // 1K only :)
            space->nop_readwrite(0x0400, 0xbbff);
            break;
        case 1:  // +7K TANEX
            space->install_ram(0x0400, 0x1fff,NULL);
            space->nop_readwrite(0x2000, 0xbbff);
            break;
        default: // +7K TANEX + 40K TANRAM
            space->install_ram(0x0400, 0xbbff, NULL);
            break;
    }

	m_timer = machine().scheduler().timer_alloc(FUNC(microtan_read_cassette));
}

MACHINE_RESET( microtan )
{
	microtan_state *state = machine.driver_data<microtan_state>();
	int i;
	static const char *const keynames[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7", "ROW8" };

	for (i = 1; i < 10;  i++)
	{
		state->m_keyrows[i] = machine.root_device().ioport(keynames[i-1])->read();
	}
	set_led_status(machine, 1, (state->m_keyrows[3] & 0x80) ? 0 : 1);
}
