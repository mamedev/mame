// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
/**********************************************************************

    Motorola MC68230 PI/T Parallell Interface and Timer

**********************************************************************/

/*
  Registers  
-----------------------------------------------------------------------
  Offset        Reset R/W 
  RS1-RS5 Name  Value Reset Description
-----------------------------------------------------------------------
  0x00 RW PGCR         No   Port General Control register
  0x01 RW PSRR         No   Port Service Request register
  0x02 RW PADDR        No   Port A Data Direction register
  0x03 RW PBDDR        No   Port B Data Direction register
  0x04 RW PCDDR        No   Port C Data Direction register
  0x05 RW PIVR         No   Port Interrupt vector register
  0x06 RW PACR         No   Port A Control register
  0x07 RW PBCR         No   Port B Control register
  0x08 RW PADR         May  Port A Data register
  0x09 RW PBDR         May  Port B Data register
  0x0a RO PAAR         No   Port A Alternate register
  0x0b RO PBAR         No   Port B Alternate register
  0x0c RW PCDR         No   Port C Data register
  0x0d RW PSR          May  Port Status register
  0x0e n/a
  0x0f n/a
  0x10 RW TCR          No   Timer Control Register
  0x11 RW TIVR         No   Timer Interrupt Vector Register
  0x12 n/a
  0x13 RW CPRH         No   Counter Preload Register High
  0x14 RW CPRM         No   Counter Preload Register Middle
  0x15 RW CPRL         No   Counter Preload Register Low
  0x17 RO CNTRH        No   Counter Register High
  0x18 RO CNTRM        No   Counter Register Middle
  0x19 RO CNTRL        No   Counter Register Low
  0x1A RW TSR          May  Timer Status Register

*/


