// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "cpu/m68000/m68000.h"

#define m68340SIM_MCR          (0x00)
//                             (0x02)
#define m68340SIM_SYNCR        (0x04)
#define m68340SIM_AVR_RSR      (0x06)
//                             (0x08)
//                             (0x0a)
//                             (0x0c)
//                             (0x0e)
#define m68340SIM_PORTA        (0x11)
#define m68340SIM_DDRA         (0x13)
#define m68340SIM_PPRA1        (0x15)
#define m68340SIM_PPRA2        (0x17)
#define m68340SIM_PORTB        (0x19)
#define m68340SIM_PORTB1       (0x1b)
#define m68340SIM_DDRB         (0x1d)
#define m68340SIM_PPARB        (0x1f)
#define m68340SIM_SWIV_SYPCR   (0x20)
#define m68340SIM_PICR         (0x22)
#define m68340SIM_PITR         (0x24)
#define m68340SIM_SWSR         (0x26)
//                             (0x28)
//                             (0x2a)
//                             (0x2c)
//                             (0x2e)
//                             (0x30)
//                             (0x32)
//                             (0x34)
//                             (0x36)
//                             (0x38)
//                             (0x3a)
//                             (0x3c)
//                             (0x3e)
#define m68340SIM_AM_CS0       (0x40)
#define m68340SIM_BA_CS0       (0x44)
#define m68340SIM_AM_CS1       (0x48)
#define m68340SIM_BA_CS1       (0x4c)
#define m68340SIM_AM_CS2       (0x50)
#define m68340SIM_BA_CS2       (0x54)
#define m68340SIM_AM_CS3       (0x58)
#define m68340SIM_BA_CS3       (0x5c)






class m68340_sim
{
	public:

	UINT32 m_am[4];
	UINT32 m_ba[4];

	void reset(void);
};
