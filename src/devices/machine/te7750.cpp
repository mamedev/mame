// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    TE7750 Super I/O Expander
    by Tokyo Electron Device Ltd. (TEL)

    The TE7750 and its successors are CMOS I/O expanders equipped with
    nine 8-bit parallel ports, which can programmed for input or
    output either in hardware or in software.

    In soft mode (IOS2-0 = 0), control registers CR1, CR2 and CR3 to
    set P1, P3, P4, P6, P7, P9 for byte input or output and P20-23,
    P24-27, P50-53, P54-57 for separate nibble input or output. All
    ports are set for input upon reset.

    In hard mode, the state of the IOS pins upon reset determines
    how many of P2, P3, P4, P5, P6 and P7 are set for output. P1 is
    always set for input and P9 is always set for output. CR1-CR3 are
    not used and separate nibble I/O is not available.

    CR0 determines the direction of each bit of P8 in both modes.

    The MS pin is offered for dual bus compatibility. When MS is 0,
    RD and WR need to be brought active low separately. When MS is 1,
    RD becomes a Motorola-style R/W control signal (R = 1, W = 0),
    and WR becomes an active-low "M Enable" input.

    TE7750, TE7751, TE7752, TE7753 and TE7754 appear to be nearly
    identical functionally, though they have three different pinouts
    (TE7751/TE7752 appear to be pin-compatible, as are TE7753/TE7754).
    One known difference is that TE7753 resets all output latches to
    "H" and TE7754 resets them to "L" (the latches can be written in
    soft mode before the ports are set for output). TE7750, TE7751 and
    TE7752 should do either one or the other, but available
    documentation for these parts is incomplete.

***********************************************************************

    A3 A2 A1 A0   Register  D7  D6  D5  D4  D3  D2  D1  D0
    -----------   --------  --  --  --  --  --  --  --  --
     0  0  0  0   Port 1    R/W R/W R/W R/W R/W R/W R/W R/W
     0  0  0  1   Port 2    R/W R/W R/W R/W R/W R/W R/W R/W
     0  0  1  0   Port 3    R/W R/W R/W R/W R/W R/W R/W R/W
     0  0  1  1   Port 4    R/W R/W R/W R/W R/W R/W R/W R/W
     0  1  0  0   Port 5    R/W R/W R/W R/W R/W R/W R/W R/W
     0  1  0  1   Port 6    R/W R/W R/W R/W R/W R/W R/W R/W
     0  1  1  0   Port 7    R/W R/W R/W R/W R/W R/W R/W R/W
     0  1  1  1   Port 8    R/W R/W R/W R/W R/W R/W R/W R/W
     1  0  0  0   Port 9    R/W R/W R/W R/W R/W R/W R/W R/W
     1  0  0  1   CR0        W   W   W   W   W   W   W   W
     1  0  1  0   CR1                    W*  W*      W*  W*
     1  0  1  1   CR2                    W*  W*      W*  W*
     1  1  0  0   CR3                    W*          W*

    * CR1-CR3 are only writable in soft mode.

    IOS0 IOS1 IOS2    P1  P2  P3  P4  P5  P6  P7  P8  P9
    ---- ---- ----    --  --  --  --  --  --  --  --  --
      0    0    0     CR1 CR1 CR1 CR2 CR2 CR2 CR3 CR0 CR3
      0    0    1      I   O   O   O   O   O   O  CR0  O
      0    1    0      I   I   O   O   O   O   O  CR0  O
      0    1    1      I   I   I   O   O   O   O  CR0  O
      1    0    0      I   I   I   I   O   O   O  CR0  O
      1    0    1      I   I   I   I   I   O   O  CR0  O
      1    1    0      I   I   I   I   I   I   O  CR0  O
      1    1    1      I   I   I   I   I   I   I  CR0  O

***********************************************************************

    Table of pin assignments

                 ---Parallel Port 1---
            P10 P11 P12 P13 P14 P15 P16 P17
    TE7750    4   5   6   7   8   9  10  11
    TE7751   20  21  22  23  24  25  26  27
    TE7753   17  18  19  20  21  22  23  24

                 ---Parallel Port 2---
            P10 P11 P12 P13 P14 P15 P16 P17
    TE7750   22  23  24  25  26  27  33  34
    TE7751   28  29  30  31  32  33  34  35
    TE7753   27  28  29  30  31  32  33  34

                 ---Parallel Port 3---
            P30 P31 P32 P33 P34 P35 P36 P37
    TE7750   43  46  47  48  49  50  51  52
    TE7751   36  37  38  39  41  42  43  44
    TE7753   35  36  37  38  39  40  41  42

                 ---Parallel Port 4---
            P40 P41 P42 P43 P44 P45 P46 P47
    TE7750   12  13  16  17  18  19  20  21
    TE7751   45  46  47  48  49  50  51  52
    TE7753   43  44  45  46  47  48  49  52

                 ---Parallel Port 5---
            P50 P51 P52 P53 P54 P55 P56 P57
    TE7750   35  36  37  38  39  40  41  42
    TE7751   74  75  76  77  78  79  80  81
    TE7753   73  74  77  78  79  80  81  82

                 ---Parallel Port 6---
            P60 P61 P62 P63 P64 P65 P66 P67
    TE7750   66  67  68  69  70  71  72  73
    TE7751   82  83  84  85  86  87  88  89
    TE7753   83  84  85  86  87  88  89  90

                 ---Parallel Port 7---
            P70 P71 P72 P73 P74 P75 P76 P77
    TE7750   53  54  55  56  60  63  64  65
    TE7751   92  93  94  95  96  97  98  99
    TE7753   91  92  93  94  95  96  97  98

                 ---Parallel Port 8---
            P80 P81 P82 P83 P84 P85 P86 P87
    TE7750   80  81  82  83  84  85  86  87
    TE7751  100   1   2   3   4   5   6   7
    TE7753   99   2   3   4   5   6   7   8

                 ---Parallel Port 9---
            P90 P91 P92 P93 P94 P95 P96 P97
    TE7750   76  77  78  79  93  94  95  96
    TE7751    9  10  11  12  14  15  17  18
    TE7753    9  10  11  12  13  14  15  16

              ---Bidirectional Data Bus---
             D0  D1  D2  D3  D4  D5  D6  D7
    TE7750  100 101 102 103 106 107 108 109
    TE7751   53  54  55  56  57  58  59  60
    TE7753   53  54  55  56  57  58  59  60

                 ---Address Inputs---
                 A0    A1    A2    A3
    TE7750      110   111   112   113
    TE7751       66    67    68    69
    TE7753       65    66    67    68

               ---Input/Output Select---
                 RW0      RW1      RW2 - TE7750
                IOS0     IOS1     IOS2 - TE7751/TE7753
    TE7750        97       98       99
    TE7751        70       71       72
    TE7753        69       70       71

                  ---Chip Select---
                 #CS     #RD     #WR - MS = 0
                 #CS     R#W    #MEN - MS = 1
    TE7750       116     114     115
    TE7751        63      64      65
    TE7753        62      63      64

                  ---Other Inputs---
                  MS           RESET
    TE7750         3            120
    TE7751        73             62
    TE7753        72             61


    TE7750: 14,44,74,104 - Vdd
            1,15,31,45,61,75,91,105 - Vss
            2,28,29,30,32,57,58,59,62,88,89,90,92,117,118,119 - NC

    TE7751: 16,89 - Vdd
            8,13,19,40,61,90 - Vss

    TE7753: 1,25,51,75 - Vdd
            26,50,76,100 - Vss

**********************************************************************/

#include "emu.h"
#include "te7750.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(TE7750, te7750_device, "te7750", "TE7750 Super I/O Expander")
DEFINE_DEVICE_TYPE(TE7751, te7751_device, "te7751", "TE7751 Super I/O Expander")
DEFINE_DEVICE_TYPE(TE7752, te7752_device, "te7752", "TE7752 Super I/O Expander")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  te7750_device - constructor
//-------------------------------------------------

te7750_device::te7750_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_input_cb(*this, 0xff)
	, m_output_cb(*this)
	, m_ios_cb(*this, 0) // assume soft mode unless specified
{
	std::fill(std::begin(m_data_dir), std::end(m_data_dir), 0xff);
}

te7750_device::te7750_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: te7750_device(mconfig, TE7750, tag, owner, clock)
{
}

//-------------------------------------------------
//  te7751_device - constructor
//-------------------------------------------------

te7751_device::te7751_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: te7750_device(mconfig, TE7751, tag, owner, clock)
{
}

//-------------------------------------------------
//  te7752_device - constructor
//-------------------------------------------------

te7752_device::te7752_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: te7750_device(mconfig, TE7752, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void te7750_device::device_start()
{
	// save state
	save_item(NAME(m_data_latch));
	save_item(NAME(m_data_dir));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void te7750_device::device_reset()
{
	// reset output latches to zero (guess based on ninjak coin counters)
	std::fill(std::begin(m_data_latch), std::end(m_data_latch), 0);

	// set ports for input or output
	set_ios();
}

//-------------------------------------------------
//  set_port_dir - set data direction for a port
//-------------------------------------------------

void te7750_device::set_port_dir(int port, u8 dir)
{
	// set bitwise direction (1 = input, 0 = output)
	u8 old_dir = m_data_dir[port];
	m_data_dir[port] = dir;

	// update outputs if lines were formerly set for input
	if ((old_dir & ~dir) != 0)
	{
		logerror("Setting P%d & %02X for output (%02X latched)\n", port + 1, dir ^ 0xff, m_data_latch[port]);
		m_output_cb[port](0, m_data_latch[port] | dir, dir ^ 0xff);
	}
}

//-------------------------------------------------
//  set_ios - reset data direction for all ports
//-------------------------------------------------

void te7750_device::set_ios()
{
	// get state of IOS pins (0 for soft mode, 1-7 for hard mode)
	u8 ios = m_ios_cb() & 7;

	// P1: always input in hard mode; reset to input in soft mode
	set_port_dir(0, 0xff);

	// P2-P7: set to input or output depending on IOS setting
	for (int port = 1; port < 7; port++)
		set_port_dir(port, (ios == 0 || ios > port) ? 0xff : 0x00);

	// P8: reset to input in either mode
	set_port_dir(7, 0xff);

	// P9: always output in hard mode; reset to input in soft mode
	set_port_dir(8, (ios == 0) ? 0xff : 0x00);
}

//-------------------------------------------------
//  read - read input port and/or output latch
//-------------------------------------------------

u8 te7750_device::read(offs_t offset)
{
	if (offset < 9)
	{
		// read back P[1-9] output latch
		u8 data = m_data_latch[offset];
		u8 dir = m_data_dir[offset];

		// combine with P[1-9] input lines (if any were defined)
		if (dir != 0x00)
			data = (data & ~dir) | (m_input_cb[offset](0, dir) & dir);

		// put data on the bus
		return data;
	}

	logerror("Attempt to read from register with offset %X\n", offset);
	return 0xff;
}

//-------------------------------------------------
//  write - write to output latch or to a control
//  register
//-------------------------------------------------

void te7750_device::write(offs_t offset, u8 data)
{
	if (offset < 9)
	{
		// set P[1-9] output latch
		m_data_latch[offset] = data;

		// update output lines (if any were defined)
		u8 dir = m_data_dir[offset];
		if (dir != 0xff)
			m_output_cb[offset](0, data | dir, dir ^ 0xff);
	}
	else if (offset == 9)
	{
		// CR0: set direction for each bit of P80-P87
		set_port_dir(7, data);
	}
	else if (offset == 10)
	{
		if ((m_ios_cb() & 7) != 0)
			logerror("Attempt to write %02X to CR1 in hard mode\n", data);
		else
		{
			// CR1: set direction for P10-17, P20-P23, P24-P27 and P30-P37
			set_port_dir(0, BIT(data, 4) ? 0xff : 0x00);
			set_port_dir(1, (BIT(data, 3) ? 0xf0 : 0x00) | (BIT(data, 0) ? 0x0f : 0x00));
			set_port_dir(2, BIT(data, 1) ? 0xff : 0x00);
		}
	}
	else if (offset == 11)
	{
		if ((m_ios_cb() & 7) != 0)
			logerror("Attempt to write %02X to CR2 in hard mode\n", data);
		else
		{
			// CR2: set direction for P40-47, P50-P53, P54-P57 and P60-P67
			set_port_dir(3, BIT(data, 4) ? 0xff : 0x00);
			set_port_dir(4, (BIT(data, 3) ? 0xf0 : 0x00) | (BIT(data, 0) ? 0x0f : 0x00));
			set_port_dir(5, BIT(data, 1) ? 0xff : 0x00);
		}
	}
	else if (offset == 12)
	{
		if ((m_ios_cb() & 7) != 0)
			logerror("Attempt to write %02X to CR3 in hard mode\n", data);
		else
		{
			// CR3: set direction for P70-77 and P90-P97
			set_port_dir(6, BIT(data, 4) ? 0xff : 0x00);
			set_port_dir(8, BIT(data, 1) ? 0xff : 0x00);
		}
	}
	else
		logerror("Attempt to write %02X to register with offset %X\n", data, offset);
}
