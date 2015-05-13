// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

UINT32 nec_common_device::EA_000() { m_EO=Wreg(BW)+Wreg(IX); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_001() { m_EO=Wreg(BW)+Wreg(IY); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_002() { m_EO=Wreg(BP)+Wreg(IX); m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_003() { m_EO=Wreg(BP)+Wreg(IY); m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_004() { m_EO=Wreg(IX); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_005() { m_EO=Wreg(IY); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_006() { m_EO=FETCH(); m_EO+=FETCH()<<8; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_007() { m_EO=Wreg(BW); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }

UINT32 nec_common_device::EA_100() { m_EO=(Wreg(BW)+Wreg(IX)+(INT8)FETCH()); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_101() { m_EO=(Wreg(BW)+Wreg(IY)+(INT8)FETCH()); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_102() { m_EO=(Wreg(BP)+Wreg(IX)+(INT8)FETCH()); m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_103() { m_EO=(Wreg(BP)+Wreg(IY)+(INT8)FETCH()); m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_104() { m_EO=(Wreg(IX)+(INT8)FETCH()); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_105() { m_EO=(Wreg(IY)+(INT8)FETCH()); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_106() { m_EO=(Wreg(BP)+(INT8)FETCH()); m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_107() { m_EO=(Wreg(BW)+(INT8)FETCH()); m_EA=DefaultBase(DS0)+m_EO; return m_EA; }

UINT32 nec_common_device::EA_200() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BW)+Wreg(IX)+(INT16)m_E16; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_201() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BW)+Wreg(IY)+(INT16)m_E16; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_202() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BP)+Wreg(IX)+(INT16)m_E16; m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_203() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BP)+Wreg(IY)+(INT16)m_E16; m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_204() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(IX)+(INT16)m_E16; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_205() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(IY)+(INT16)m_E16; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_206() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BP)+(INT16)m_E16; m_EA=DefaultBase(SS)+m_EO; return m_EA; }
UINT32 nec_common_device::EA_207() { m_E16=FETCH(); m_E16+=FETCH()<<8; m_EO=Wreg(BW)+(INT16)m_E16; m_EA=DefaultBase(DS0)+m_EO; return m_EA; }

const nec_common_device::nec_eahandler nec_common_device::s_GetEA[192]=
{
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,
	&nec_common_device::EA_000, &nec_common_device::EA_001, &nec_common_device::EA_002, &nec_common_device::EA_003, &nec_common_device::EA_004, &nec_common_device::EA_005, &nec_common_device::EA_006, &nec_common_device::EA_007,

	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,
	&nec_common_device::EA_100, &nec_common_device::EA_101, &nec_common_device::EA_102, &nec_common_device::EA_103, &nec_common_device::EA_104, &nec_common_device::EA_105, &nec_common_device::EA_106, &nec_common_device::EA_107,

	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207,
	&nec_common_device::EA_200, &nec_common_device::EA_201, &nec_common_device::EA_202, &nec_common_device::EA_203, &nec_common_device::EA_204, &nec_common_device::EA_205, &nec_common_device::EA_206, &nec_common_device::EA_207
};
