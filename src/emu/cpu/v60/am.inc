// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
// NOTE for bit string / field addressing
// ************************************
// m_moddim must be passed as 10 for bit string instructions,
// and as 11 for bit field instructions




// Addressing mode functions and tables
#include "am1.inc" // ReadAM
#include "am2.inc" // ReadAMAddress
#include "am3.inc" // WriteAM

/*
  Input:
  m_modadd
    m_moddim

  Output:
    m_amout
    amLength
*/

UINT32 v60_device::ReadAM()
{
	m_modm = m_modm?1:0;
	m_modval = OpRead8(m_modadd);
	return (this->*s_AMTable1[m_modm][m_modval >> 5])();
}

UINT32 v60_device::BitReadAM()
{
	m_modm = m_modm?1:0;
	m_modval = OpRead8(m_modadd);
	return (this->*s_BAMTable1[m_modm][m_modval >> 5])();
}



/*
  Input:
  m_modadd
    m_moddim

  Output:
    m_amout
    m_amflag
    amLength
*/

UINT32 v60_device::ReadAMAddress()
{
	m_modm = m_modm?1:0;
	m_modval = OpRead8(m_modadd);
	return (this->*s_AMTable2[m_modm][m_modval >> 5])();
}

UINT32 v60_device::BitReadAMAddress()
{
	m_modm = m_modm?1:0;
	m_modval = OpRead8(m_modadd);
	return (this->*s_BAMTable2[m_modm][m_modval >> 5])();
}

/*
  Input:
  m_modadd
    m_moddim
    m_modwritevalb / H/W

  Output:
    m_amout
    amLength
*/

UINT32 v60_device::WriteAM()
{
	m_modm = m_modm?1:0;
	m_modval = OpRead8(m_modadd);
	return (this->*s_AMTable3[m_modm][m_modval >> 5])();
}
