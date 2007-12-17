/*

  machine/model3.c: functions emulating various aspects of the Model 3 hardware

*/

#include "driver.h"

static int m3_step;

/******************************************************************/
/* Real3D TAP Port                                                */
/******************************************************************/

/*
 * TAP Finite State Machine
 *
 * Y are states and X are outgoing paths. Constructed from information on page
 * 167 of the 3D-RAM manual.
 */

#define NEXT(new_state) fsm[state][new_state]

static INT32  state;  // current state
static INT32  fsm[][2] =  {
                            {  1,  0 },  // 0  Test-Logic/Reset
                            {  1,  2 },  // 1  Run-Test/Idle
                            {  3,  9 },  // 2  Select-DR-Scan
                            {  4,  5 },  // 3  Capture-DR
                            {  4,  5 },  // 4  Shift-DR
                            {  6,  8 },  // 5  Exit1-DR
                            {  6,  7 },  // 6  Pause-DR
                            {  4,  8 },  // 7  Exit2-DR
                            {  1,  2 },  // 8  Update-DR
                            { 10,  0 },  // 9  Select-IR-Scan
                            { 11, 12 },  // 10 Capture-IR
                            { 11, 12 },  // 11 Shift-IR
                            { 13, 15 },  // 12 Exit1-IR
                            { 13, 14 },  // 13 Pause-IR
                            { 11, 15 },  // 14 Exit2-IR
                            {  1,  2 }   // 15 Update-IR
                        };

/*
 * TAP Registers
 */

static UINT64   current_instruction;    // latched IR (not always equal to IR)
static UINT64   ir;                     // instruction register (46 bits)

static UINT8    id_data[32];            // ASIC ID code data buffer
static INT32      id_size;                // size of ID data in bits
//static INT32      ptr;                    // current bit ptr for data

static int      tdo;                    // bit shifted out to TDO

/*
 * insert_bit():
 *
 * Inserts a bit into an arbitrarily long bit field. Bit 0 is assumed to be
 * the MSB of the first byte in the buffer.
 */

static void insert_bit(UINT8 *buf, INT32 bit_num, INT32 bit)
{
    INT32 bit_in_byte;

    bit_in_byte = 7 - (bit_num & 7);

    buf[bit_num / 8] &= ~(1 << bit_in_byte);
    buf[bit_num / 8] |= (bit << bit_in_byte);
}

/*
 * insert_id():
 *
 * Inserts a 32-bit ID code into the ID bit field.
 */

static void insert_id(UINT32 id, INT32 start_bit)
{
    INT32 i;

    for (i = 31; i >= 0; i--)
        insert_bit(id_data, start_bit++, (id >> i) & 1);
}

/*
 * shift():
 *
 * Shifts the data buffer right (towards LSB at byte 0) by 1 bit. The size of
 * the number of bits must be specified. The bit shifted out of the LSB is
 * returned.
 */

static int shift(UINT8 *data, INT32 num_bits)
{
    INT32     i;
    int    shift_out, shift_in;

    /*
     * This loop takes care of all the fully-filled bytes
     */

    shift_in = 0;
	shift_out = 0;
    for (i = 0; i < num_bits / 8; i++)
    {
        shift_out = data[i] & 1;
        data[i] >>= 1;
        data[i] |= (shift_in << 7);
        shift_in = shift_out;   // carry over to next element's MSB
    }

    /*
     * Take care of the last partial byte (if there is one)
     */

    if ((num_bits & 7) != 0)
    {
        shift_out = (data[i] >> (8 - (num_bits & 7))) & 1;
        data[i] >>= 1;
        data[i] |= (shift_in << 7);
    }

    return shift_out;
}

/*
 * int tap_read(void);
 *
 * Reads TDO.
 *
 * Returns:
 *      TDO.
 */

int model3_tap_read(void)
{
    return tdo;
}

/*
 * void tap_write(int tck, int tms, int tdi, int trst);
 *
 * Writes to the TAP. State changes only occur on the rising edge of the clock
 * (tck = 1.)
 *
 * Parameters:
 *      tck  = Clock.
 *      tms  = Test mode select.
 *      tdi  = Serial data input. Must be 0 or 1 only!
 *      trst = Reset.
 */

void model3_tap_write(int tck, int tms, int tdi, int trst)
{
    if (!tck)
        return;

    state = NEXT(tms);

    switch (state)
    {
    case 3:     // Capture-DR

        /*
         * Read ASIC IDs.
         *
         * The ID Sequence is:
         *  - Jupiter
         *  - Mercury
         *  - Venus
         *  - Earth
         *  - Mars
         *  - Mars (again)
         *
         * Note that different Model 3 steps have different chip
         * revisions, hence the different IDs returned below.
         *
         * On Step 1.5 and 1.0, instruction 0x0C631F8C7FFE is used to retrieve
         * the ID codes but Step 2.0 is a little weirder. It seems to use this
         * and either the state of the TAP after reset or other instructions
         * to read the IDs as well. This can be emulated in one of 2 ways:
         * Ignore the instruction and always load up the data or load the
         * data on TAP reset and when the instruction is issued.
         */

        if (m3_step == 0x10)
        {
            insert_id(0x116C7057, 1 + 0 * 32);
            insert_id(0x216C3057, 1 + 1 * 32);
            insert_id(0x116C4057, 1 + 2 * 32);
            insert_id(0x216C5057, 1 + 3 * 32);
            insert_id(0x116C6057, 1 + 4 * 32 + 1);
            insert_id(0x116C6057, 1 + 5 * 32 + 1);
        }
        else if (m3_step == 0x15)
        {
            insert_id(0x316C7057, 1 + 0 * 32);
            insert_id(0x316C3057, 1 + 1 * 32);
            insert_id(0x216C4057, 1 + 2 * 32);      // Lost World may to use 0x016C4057
            insert_id(0x316C5057, 1 + 3 * 32);
            insert_id(0x216C6057, 1 + 4 * 32 + 1);
            insert_id(0x216C6057, 1 + 5 * 32 + 1);
        }
        else if (m3_step >= 0x20)
        {
            insert_id(0x416C7057, 1 + 0 * 32);
            insert_id(0x416C3057, 1 + 1 * 32);
            insert_id(0x316C4057, 1 + 2 * 32);
            insert_id(0x416C5057, 1 + 3 * 32);
            insert_id(0x316C6057, 1 + 4 * 32 + 1);
            insert_id(0x316C6057, 1 + 5 * 32 + 1);
        }

        break;

    case 4:     // Shift-DR

        tdo = shift(id_data, id_size);
        break;

    case 10:    // Capture-IR

        /*
         * Load lower 2 bits with 01 as per IEEE 1149.1-1990
         */

        ir = 1;
        break;

    case 11:    // Shift-IR

        /*
         * Shift IR towards output and load in new data from TDI
         */

        tdo = ir & 1;   // shift LSB to output
        ir >>= 1;
        ir |= ((UINT64) tdi << 45);
        break;

    case 15:    // Update-IR

        /*
         * Latch IR (technically, this should occur on the falling edge of
         * TCK)
         */

        ir &= U64(0x3fffffffffff);
        current_instruction = ir;
        break;

    default:
        break;
    }
}


/*
 * void tap_reset(void);
 *
 * Resets the TAP (simulating a power up or SCAN_RST signal.)
 */

void model3_tap_reset(void)
{
    id_size = 197;  // 197 bits

    state = 0;  // test-logic/reset
}

/*
 * void model3_machine_init(int step)
 *
 * step = BCD hardware rev.  0x10 for 1.0, 0x15 for 1.5, 0x20 for 2.0, etc.
 *
 */

void model3_machine_init(int step)
{
	m3_step = step;
}

/*****************************************************************************/
/* Epson RTC-72421 */

static UINT8 rtc_get_reg(int reg)
{
	mame_system_time systime;

	mame_get_current_datetime(Machine, &systime);

	switch(reg)
	{
		case 0:		// 1-second digit
			return (systime.local_time.second % 10) & 0xf;

		case 1:		// 10-seconds digit
			return (systime.local_time.second / 10) & 0x7;

		case 2:		// 1-minute digit
			return (systime.local_time.minute % 10) & 0xf;

		case 3:		// 10-minute digit
			return (systime.local_time.minute / 10) & 0x7;

		case 4:		// 1-hour digit
			return (systime.local_time.hour % 10) & 0xf;

		case 5:		// 10-hours digit
			return (systime.local_time.hour / 10) & 0x7;

		case 6:		// 1-day digit (days in month)
			return (systime.local_time.mday % 10) & 0xf;

		case 7:		// 10-days digit
			return (systime.local_time.mday / 10) & 0x3;

		case 8:		// 1-month digit
			return ((systime.local_time.month + 1) % 10) & 0xf;

		case 9:		// 10-months digit
			return ((systime.local_time.month + 1) / 10) & 0x1;

		case 10:	// 1-year digit
			return (systime.local_time.year % 10) & 0xf;

		case 11:	// 10-years digit
			return ((systime.local_time.year % 100) / 10) & 0xf;

		case 12:	// day of the week
			return systime.local_time.weekday & 0x7;

		case 13:
			return 0;

		case 14:
			return 0;

		case 15:
			return 0;

		default:
			fatalerror("RTC-72421: Unknown reg %02X", reg);
			return 0;
	}
}

READ32_HANDLER(rtc72421_r)
{
	int reg = offset;
	UINT32 data;
	data = rtc_get_reg(reg) << 24;
	data |= 0x30000;	/* these bits are set to pass the battery voltage test */
	return data;
}

WRITE32_HANDLER(rtc72421_w)
{
}

