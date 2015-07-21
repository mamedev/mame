// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
/**********************************************************************

    Motorola MC68230 PI/T Parallell Interface and Timer

**********************************************************************/
#pragma once

#ifndef __68230PIT_H__
#define __68230PIT_H__

#include "emu.h"

/*-----------------------------------------------------------------------
  Registers                RS1-RS5   R/W Description
-------------------------------------------------------------------------*/
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
class pit68230_device :  public device_t
{
public:
	// construction/destruction
	pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( data_r );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
        UINT8  m_pgcr;  // Port General Control register
	UINT8  m_psrr;  // Port Service Request register
	UINT8  m_paddr; // Port A Data Direction register
	UINT8  m_pbddr; // Port B Data Direction register
	UINT8  m_pcddr; // Port C Data Direction register
	UINT8  m_pacr;  // Port A Control register
	UINT8  m_pbcr;  // Port B Control register
	UINT8  m_padr;  // Port A Data register
	UINT8  m_pbdr;  // Port B Data register
	UINT8  m_psr;   // Port Status Register
};


// device type definition
extern const device_type PIT68230;

#endif // __68230PIT__
