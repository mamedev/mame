// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstr??m
/**********************************************************************
*
*   Motorola MC68230 PI/T Parallell Interface and Timer
*
*                           _____   _____
*                   D5   1 |*    \_/     | 48  D4
*                   D6   2 |             | 47  D3
*                   D7   3 |             | 46  D2
*                  PA0   4 |             | 45  D1
*                  PA1   5 |             | 44  D0
*                  PA2   6 |             | 43  R/W*
*                  PA3   7 |             | 42  DTACK*
*                  PA4   8 |             | 41  CS*
*                  PA5   9 |             | 40  CLK
*                  PA6  10 |             | 39  RESET*
*                  PA7  11 |             | 38  VSS
*                  Vcc  12 |   TS68230   | 37  PC7/TIACK*
*                   H1  13 |   SC87845   | 36  PC6/PIACK*
*                   H2  14 |             | 35  PC5/PIRQ*
*                   H3  15 |             | 34  PC4/DMAREQ*
*                   H4  16 |             | 33  PC3/TOUT
*                  PB0  17 |             | 32  PC2/TIN
*                  PB1  18 |             | 31  PC1
*                  PB2  19 |             | 30  PC0
*                  PB3  20 |             | 29  RS1
*                  PB4  21 |             | 28  RS2
*                  PB5  22 |             | 27  RS3
*                  PB6  23 |             | 26  RS4
*                  PB7  24 |_____________| 25  RS5
*
**********************************************************************/

#pragma once

#ifndef __68230PIT_H__
#define __68230PIT_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PIT68230_PA_OUTPUT_CALLBACK(_write) \
		devcb = &pit68230_device::set_pa_wr_callback (*device, DEVCB_ ## _write);

#define MCFG_PIT68230_PB_OUTPUT_CALLBACK(_write) \
		devcb = &pit68230_device::set_pb_wr_callback (*device, DEVCB_ ## _write);

#define MCFG_PIT68230_H2_CALLBACK(_write) \
		devcb = &pit68230_device::set_h2_wr_callback (*device, DEVCB_ ## _write);

/*-----------------------------------------------------------------------
 * Registers                RS1-RS5   R/W Description
 * -------------------------------------------------------------------------*/
#define PIT_68230_PGCR        0x00 /* RW Port General Control register   */
#define PIT_68230_PSRR        0x01 /* RW Port Service Request register   */
#define PIT_68230_PADDR       0x02 /* RW Port A Data Direction register  */
#define PIT_68230_PBDDR       0x03 /* RW Port B Data Direction register  */
#define PIT_68230_PCDDR       0x04 /* RW Port C Data Direction register  */
#define PIT_68230_PIVR        0x05 /* RW Port Interrupt vector register  */
#define PIT_68230_PACR        0x06 /* RW Port A Control register         */
#define PIT_68230_PBCR        0x07 /* RW Port B Control register         */
#define PIT_68230_PADR        0x08 /* RW Port A Data register            */
#define PIT_68230_PBDR        0x09 /* RW Port B Data register            */
#define PIT_68230_PAAR        0x0a /* RO Port A Alternate register       */
#define PIT_68230_PBAR        0x0b /* RO Port B Alternate register       */
#define PIT_68230_PCDR        0x0c /* RW Port C Data register            */
#define PIT_68230_PSR         0x0d /* RW Port Status register            */
#define PIT_68230_TCR         0x10 /* RW Timer Control Register          */
#define PIT_68230_TIVR        0x11 /* RW Timer Interrupt Vector Register */
#define PIT_68230_CPRH        0x13 /* RW Counter Preload Register High   */
#define PIT_68230_CPRM        0x14 /* RW Counter Preload Register Middle */
#define PIT_68230_CPRL        0x15 /* RW Counter Preload Register Low    */
#define PIT_68230_CNTRH       0x17 /* RO Counter Register High           */
#define PIT_68230_CNTRM       0x18 /* RO Counter Register Middle         */
#define PIT_68230_CNTRL       0x19 /* RO Counter Register Low            */
#define PIT_68230_TSR         0x1A /* RW Timer Status Register           */

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class pit68230_device :  public device_t, public device_execute_interface
{
public:
// construction/destruction
pit68230_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source);
pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
template<class _Object> static devcb_base &set_pa_wr_callback (device_t &device, _Object object)
{
		return downcast<pit68230_device &>(device).m_write_pa.set_callback (object);
}
template<class _Object> static devcb_base &set_h2_wr_callback (device_t &device, _Object object)
{
		return downcast<pit68230_device &>(device).m_write_h2.set_callback (object);
}

DECLARE_WRITE8_MEMBER (write);
DECLARE_READ8_MEMBER (read);

void h1_set (UINT8 state);
void portb_setbit (UINT8 bit, UINT8 state);

protected:
// device-level overrides
virtual void device_start () override;
virtual void device_reset () override;
virtual void device_timer (emu_timer &timer, device_timer_id id, int param, void *ptr) override;
virtual void execute_run () override;
int m_icount;
devcb_write8 m_write_pa;
devcb_write_line m_write_h2;

// peripheral ports
UINT8 m_pgcr;           // Port General Control register
UINT8 m_psrr;           // Port Service Request register
UINT8 m_paddr;          // Port A Data Direction register
UINT8 m_pbddr;          // Port B Data Direction register
UINT8 m_pcddr;          // Port C Data Direction register
UINT8 m_pacr;           // Port A Control register
UINT8 m_pbcr;           // Port B Control register
UINT8 m_padr;           // Port A Data register
UINT8 m_pbdr;           // Port B Data register
UINT8 m_psr;            // Port Status Register
};

// device type definition
extern const device_type PIT68230;
#endif /* __68230PIT_H__ */
