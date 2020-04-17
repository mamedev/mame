// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

// 8080 mode ops, all timings unknown
#define OP80(num,func_name) void nec_common_device::func_name##_80()

OP80( 0x00, i_nop   ) { CLK(1); } // 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
OP80( 0x01, i_lxib  ) { Wreg(CW) = fetchword(); CLK(1); }
OP80( 0x02, i_staxb ) { PutMemB(DS0, Wreg(CW), Breg(AL)); CLK(1); }
OP80( 0x03, i_inxb  ) { Wreg(CW)++; CLK(1); }
OP80( 0x04, i_inrb  ) { IncByteReg(CH); CLK(1); }
OP80( 0x05, i_dcrb  ) { DecByteReg(CH); CLK(1); }
OP80( 0x06, i_mvib  ) { Breg(CH) = fetch(); CLK(1); }
OP80( 0x07, i_rlc   ) { uint32_t dst = Breg(AL); ROL_BYTE; Breg(AL) = dst; CLK(1); }
OP80( 0x09, i_dadb  ) { uint32_t dst = Wreg(BW) + Wreg(CW); SetCFW(dst); Wreg(BW) = dst; CLK(1); }
OP80( 0x0a, i_ldaxb ) { Breg(AL) = GetMemB(DS0, Wreg(CW)); CLK(1); }
OP80( 0x0b, i_dcxb  ) { Wreg(CW)--; CLK(1); }
OP80( 0x0c, i_inrc  ) { IncByteReg(CL); CLK(1); }
OP80( 0x0d, i_dcrc  ) { DecByteReg(CL); CLK(1); }
OP80( 0x0e, i_mvic  ) { Breg(CL) = fetch(); CLK(1); }
OP80( 0x0f, i_rrc   ) { uint32_t dst = Breg(AL); ROR_BYTE; Breg(AL) = dst; CLK(1); }
OP80( 0x11, i_lxid  ) { Wreg(DW) = fetchword(); CLK(1); }
OP80( 0x12, i_staxd ) { PutMemB(DS0, Wreg(DW), Breg(AL)); CLK(1); }
OP80( 0x13, i_inxd  ) { Wreg(DW)++; CLK(1); }
OP80( 0x14, i_inrd  ) { IncByteReg(DH); CLK(1); }
OP80( 0x15, i_dcrd  ) { DecByteReg(DH); CLK(1); }
OP80( 0x16, i_mvid  ) { Breg(DH) = fetch(); CLK(1); }
OP80( 0x17, i_ral   ) { uint32_t dst = Breg(AL); ROLC_BYTE; Breg(AL) = dst; CLK(1); }
OP80( 0x19, i_dadd  ) { uint32_t dst = Wreg(BW) + Wreg(DW); SetCFW(dst); Wreg(BW) = dst; CLK(1); }
OP80( 0x1a, i_ldaxd ) { Breg(AL) = GetMemB(DS0, Wreg(DW)); CLK(1); }
OP80( 0x1b, i_dcxd  ) { Wreg(DW)--; CLK(1); }
OP80( 0x1c, i_inre  ) { IncByteReg(DL); CLK(1); }
OP80( 0x1d, i_dcre  ) { DecByteReg(DL); CLK(1); }
OP80( 0x1e, i_mvie  ) { Breg(DL) = fetch(); CLK(1); }
OP80( 0x1f, i_rar   ) { uint32_t dst = Breg(AL); RORC_BYTE; Breg(AL) = dst; CLK(1); }
OP80( 0x21, i_lxih  ) { Wreg(BW) = fetchword(); CLK(1); }
OP80( 0x22, i_shld  ) { PutMemW(DS0, fetchword(), Wreg(BW)); CLK(1); }
OP80( 0x23, i_inxh  ) { Wreg(BW)++; CLK(1); }
OP80( 0x24, i_inrh  ) { IncByteReg(BH); CLK(1); }
OP80( 0x25, i_dcrh  ) { DecByteReg(BH); CLK(1); }
OP80( 0x26, i_mvih  ) { Breg(BH) = fetch(); CLK(1); }
OP80( 0x27, i_daa   ) { ADJ4(6, 0x60); CLK(1); }
OP80( 0x29, i_dadh  ) { uint32_t dst = Wreg(BW) + Wreg(BW); SetCFW(dst); Wreg(BW) = dst; CLK(1); }
OP80( 0x2a, i_lhld  ) { Wreg(BW) = GetMemW(DS0, fetchword()); CLK(1); }
OP80( 0x2b, i_dcxh  ) { Wreg(BW)--; CLK(1); }
OP80( 0x2c, i_inrl  ) { IncByteReg(BL); CLK(1); }
OP80( 0x2d, i_dcrl  ) { DecByteReg(BL); CLK(1); }
OP80( 0x2e, i_mvil  ) { Breg(BL) = fetch(); CLK(1); }
OP80( 0x2f, i_cma   ) { Breg(AL) = ~Breg(AL); CLK(1); }
OP80( 0x31, i_lxis  ) { Wreg(BP) = fetchword(); CLK(1); }
OP80( 0x32, i_sta   ) { PutMemB(DS0, fetchword(), Breg(AL)); CLK(1); }
OP80( 0x33, i_inxs  ) { Wreg(BP)++; CLK(1); }
OP80( 0x34, i_inrm  ) { uint32_t tmp = GetMemB(DS0, Wreg(BW)); uint32_t tmp1 = tmp+1; m_OverVal = (tmp==0x7f); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutMemB(DS0, Wreg(BW), (BYTE)tmp1); CLK(1); }
OP80( 0x35, i_dcrm  ) { uint32_t tmp = GetMemB(DS0, Wreg(BW)); uint32_t tmp1 = tmp-1; m_OverVal = (tmp==0x80); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutMemB(DS0, Wreg(BW), (BYTE)tmp1); CLK(1); }
OP80( 0x36, i_mvim  ) { PutMemB(DS0, Wreg(BW), fetch()); CLK(1); }
OP80( 0x37, i_stc   ) { m_CarryVal = 1; CLK(1); }
OP80( 0x39, i_dads  ) { uint32_t dst = Wreg(BW) + Wreg(BP); SetCFW(dst); Wreg(BW) = dst; CLK(1); }
OP80( 0x3a, i_lda   ) { Breg(AL) = GetMemB(DS0, fetchword()); CLK(1); }
OP80( 0x3b, i_dcxs  ) { Wreg(BP)--; CLK(1); }
OP80( 0x3c, i_inra  ) { IncByteReg(AL); CLK(1); }
OP80( 0x3d, i_dcra  ) { DecByteReg(AL); CLK(1); }
OP80( 0x3e, i_mvia  ) { Breg(AL) = fetch(); CLK(1); }
OP80( 0x3f, i_cmc   ) { m_CarryVal = !m_CarryVal; CLK(1); }
OP80( 0x40, i_movbb ) { CLK(1); }
OP80( 0x41, i_movbc ) { Breg(CH) = Breg(CL); CLK(1); }
OP80( 0x42, i_movbd ) { Breg(CH) = Breg(DH); CLK(1); }
OP80( 0x43, i_movbe ) { Breg(CH) = Breg(DL); CLK(1); }
OP80( 0x44, i_movbh ) { Breg(CH) = Breg(BH); CLK(1); }
OP80( 0x45, i_movbl ) { Breg(CH) = Breg(BL); CLK(1); }
OP80( 0x46, i_movbm ) { Breg(CH) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x47, i_movba ) { Breg(CH) = Breg(AL); CLK(1); }
OP80( 0x48, i_movcb ) { Breg(CL) = Breg(CH); CLK(1); }
OP80( 0x49, i_movcc ) { CLK(1); }
OP80( 0x4a, i_movcd ) { Breg(CL) = Breg(DH); CLK(1); }
OP80( 0x4b, i_movce ) { Breg(CL) = Breg(DL); CLK(1); }
OP80( 0x4c, i_movch ) { Breg(CL) = Breg(BH); CLK(1); }
OP80( 0x4d, i_movcl ) { Breg(CL) = Breg(BL); CLK(1); }
OP80( 0x4e, i_movcm ) { Breg(CL) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x4f, i_movca ) { Breg(CL) = Breg(AL); CLK(1); }
OP80( 0x50, i_movdb ) { Breg(DH) = Breg(CH); CLK(1); }
OP80( 0x51, i_movdc ) { Breg(DH) = Breg(CL); CLK(1); }
OP80( 0x52, i_movdd ) { CLK(1); }
OP80( 0x53, i_movde ) { Breg(DH) = Breg(DL); CLK(1); }
OP80( 0x54, i_movdh ) { Breg(DH) = Breg(BH); CLK(1); }
OP80( 0x55, i_movdl ) { Breg(DH) = Breg(BL); CLK(1); }
OP80( 0x56, i_movdm ) { Breg(DH) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x57, i_movda ) { Breg(DH) = Breg(AL); CLK(1); }
OP80( 0x58, i_moveb ) { Breg(DL) = Breg(CH); CLK(1); }
OP80( 0x59, i_movec ) { Breg(DL) = Breg(CL); CLK(1); }
OP80( 0x5a, i_moved ) { Breg(DL) = Breg(DH); CLK(1); }
OP80( 0x5b, i_movee ) { CLK(1); }
OP80( 0x5c, i_moveh ) { Breg(DL) = Breg(BH); CLK(1); }
OP80( 0x5d, i_movel ) { Breg(DL) = Breg(BL); CLK(1); }
OP80( 0x5e, i_movem ) { Breg(DL) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x5f, i_movea ) { Breg(DL) = Breg(AL); CLK(1); }
OP80( 0x60, i_movhb ) { Breg(BH) = Breg(CH); CLK(1); }
OP80( 0x61, i_movhc ) { Breg(BH) = Breg(CL); CLK(1); }
OP80( 0x62, i_movhd ) { Breg(BH) = Breg(DH); CLK(1); }
OP80( 0x63, i_movhe ) { Breg(BH) = Breg(DL); CLK(1); }
OP80( 0x64, i_movhh ) { CLK(1); }
OP80( 0x65, i_movhl ) { Breg(BH) = Breg(BL); CLK(1); }
OP80( 0x66, i_movhm ) { Breg(BH) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x67, i_movha ) { Breg(BH) = Breg(AL); CLK(1); }
OP80( 0x68, i_movlb ) { Breg(BL) = Breg(CH); CLK(1); }
OP80( 0x69, i_movlc ) { Breg(BL) = Breg(CL); CLK(1); }
OP80( 0x6a, i_movld ) { Breg(BL) = Breg(DH); CLK(1); }
OP80( 0x6b, i_movle ) { Breg(BL) = Breg(DL); CLK(1); }
OP80( 0x6c, i_movlh ) { Breg(BL) = Breg(BH); CLK(1); }
OP80( 0x6d, i_movll ) { CLK(1); }
OP80( 0x6e, i_movlm ) { Breg(BL) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x6f, i_movla ) { Breg(BL) = Breg(AL); CLK(1); }
OP80( 0x70, i_movmb ) { PutMemB(DS0, Wreg(BW), Breg(CH)); CLK(1); }
OP80( 0x71, i_movmc ) { PutMemB(DS0, Wreg(BW), Breg(CL)); CLK(1); }
OP80( 0x72, i_movmd ) { PutMemB(DS0, Wreg(BW), Breg(DH)); CLK(1); }
OP80( 0x73, i_movme ) { PutMemB(DS0, Wreg(BW), Breg(DL)); CLK(1); }
OP80( 0x74, i_movmh ) { PutMemB(DS0, Wreg(BW), Breg(BH)); CLK(1); }
OP80( 0x75, i_movml ) { PutMemB(DS0, Wreg(BW), Breg(BL)); CLK(1); }
OP80( 0x76, i_hlt   ) { logerror("%06x: HALT\n",PC()); m_halted=1; m_icount=0; }
OP80( 0x77, i_movma ) { PutMemB(DS0, Wreg(BW), Breg(AL)); CLK(1); }
OP80( 0x78, i_movab ) { Breg(AL) = Breg(CH); CLK(1); }
OP80( 0x79, i_movac ) { Breg(AL) = Breg(CL); CLK(1); }
OP80( 0x7a, i_movad ) { Breg(AL) = Breg(DH); CLK(1); }
OP80( 0x7b, i_movae ) { Breg(AL) = Breg(DL); CLK(1); }
OP80( 0x7c, i_movah ) { Breg(AL) = Breg(BH); CLK(1); }
OP80( 0x7d, i_moval ) { Breg(AL) = Breg(BL); CLK(1); }
OP80( 0x7e, i_movam ) { Breg(AL) = GetMemB(DS0, Wreg(BW)); CLK(1); }
OP80( 0x7f, i_movaa ) { CLK(1); }
OP80( 0x80, i_addb  ) { uint32_t src = Breg(CH), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x81, i_addc  ) { uint32_t src = Breg(CL), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x82, i_addd  ) { uint32_t src = Breg(DH), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x83, i_adde  ) { uint32_t src = Breg(DL), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x84, i_addh  ) { uint32_t src = Breg(BH), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x85, i_addl  ) { uint32_t src = Breg(BL), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x86, i_addm  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x87, i_adda  ) { uint32_t src = Breg(AL), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x88, i_adcb  ) { uint32_t src = Breg(CH), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x89, i_adcc  ) { uint32_t src = Breg(CL), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8a, i_adcd  ) { uint32_t src = Breg(DH), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8b, i_adce  ) { uint32_t src = Breg(DL), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8c, i_adch  ) { uint32_t src = Breg(BH), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8d, i_adcl  ) { uint32_t src = Breg(BL), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8e, i_adcm  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x8f, i_adca  ) { uint32_t src = Breg(AL), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0x90, i_subb  ) { uint32_t src = Breg(CH), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x91, i_subc  ) { uint32_t src = Breg(CL), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x92, i_subd  ) { uint32_t src = Breg(DH), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x93, i_sube  ) { uint32_t src = Breg(DL), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x94, i_subh  ) { uint32_t src = Breg(BH), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x95, i_subl  ) { uint32_t src = Breg(BL), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x96, i_subm  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x97, i_suba  ) { uint32_t src = Breg(AL), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x98, i_sbbb  ) { uint32_t src = Breg(CH), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x99, i_sbbc  ) { uint32_t src = Breg(CL), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9a, i_sbbd  ) { uint32_t src = Breg(DH), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9b, i_sbbe  ) { uint32_t src = Breg(DL), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9c, i_sbbh  ) { uint32_t src = Breg(BH), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9d, i_sbbl  ) { uint32_t src = Breg(BL), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9e, i_sbbm  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0x9f, i_sbba  ) { uint32_t src = Breg(AL), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0xa0, i_anab  ) { uint32_t src = Breg(CH), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa1, i_anac  ) { uint32_t src = Breg(CL), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa2, i_anad  ) { uint32_t src = Breg(DH), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa3, i_anae  ) { uint32_t src = Breg(DL), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa4, i_anah  ) { uint32_t src = Breg(BH), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa5, i_anal  ) { uint32_t src = Breg(BL), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa6, i_anam  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa7, i_anaa  ) { uint32_t src = Breg(AL), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xa8, i_xrab  ) { uint32_t src = Breg(CH), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xa9, i_xrac  ) { uint32_t src = Breg(CL), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xaa, i_xrad  ) { uint32_t src = Breg(DH), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xab, i_xrae  ) { uint32_t src = Breg(DL), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xac, i_xrah  ) { uint32_t src = Breg(BH), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xad, i_xral  ) { uint32_t src = Breg(BL), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xae, i_xram  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xaf, i_xraa  ) { uint32_t src = Breg(AL), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb0, i_orab  ) { uint32_t src = Breg(CH), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb1, i_orac  ) { uint32_t src = Breg(CL), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb2, i_orad  ) { uint32_t src = Breg(DH), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb3, i_orae  ) { uint32_t src = Breg(DL), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb4, i_orah  ) { uint32_t src = Breg(BH), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb5, i_oral  ) { uint32_t src = Breg(BL), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb6, i_oram  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb7, i_oraa  ) { uint32_t src = Breg(AL), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xb8, i_cmpb  ) { uint32_t src = Breg(CH), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xb9, i_cmpc  ) { uint32_t src = Breg(CL), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xba, i_cmpd  ) { uint32_t src = Breg(DH), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xbb, i_cmpe  ) { uint32_t src = Breg(DL), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xbc, i_cmph  ) { uint32_t src = Breg(BH), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xbd, i_cmpl  ) { uint32_t src = Breg(BL), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xbe, i_cmpm  ) { uint32_t src = GetMemB(DS0, Wreg(BW)), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xbf, i_cmpa  ) { uint32_t src = Breg(AL), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xc0, i_rnz   ) { if (!ZF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xc1, i_popb  ) { POP80(Wreg(CW)); }
OP80( 0xc2, i_jnz   ) { uint32_t addr = fetchword(); if (!ZF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xc3, i_jmp   ) { m_ip = fetchword(); CHANGE_PC; CLK(1); } // 0xcb
OP80( 0xc4, i_cnz   ) { uint32_t addr = fetchword(); if (!ZF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xc5, i_pushb ) { PUSH80(Wreg(CW)); }
OP80( 0xc6, i_adi   ) { uint32_t src = fetch(), dst = Breg(AL); ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0xc7, i_rst0  ) { PUSH80(m_ip); m_ip = 0; CHANGE_PC; CLK(1); }
OP80( 0xc8, i_rz    ) { if (ZF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xc9, i_ret   ) { POP80(m_ip); CHANGE_PC; CLK(1); } //0xd9
OP80( 0xca, i_jz    ) { uint32_t addr = fetchword(); if (ZF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xcc, i_cz    ) { uint32_t addr = fetchword(); if (ZF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xcd, i_call  ) { uint32_t addr = fetchword(); PUSH80(m_ip); m_ip = addr; CHANGE_PC; CLK(1); } // 0xdd, 0xfd
OP80( 0xce, i_aci   ) { uint32_t src = fetch(), dst = Breg(AL); src+=CF; ADDB; Breg(AL) = dst; CLK(1); }
OP80( 0xcf, i_rst1  ) { PUSH80(m_ip); m_ip = 8; CHANGE_PC; CLK(1); }
OP80( 0xd0, i_rnc   ) { if (!CF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xd1, i_popd  ) { POP80(Wreg(DW)); }
OP80( 0xd2, i_jnc   ) { uint32_t addr = fetchword(); if (!CF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xd3, i_out   ) { uint8_t port = fetch(); write_port_byte(port, Breg(AL)); CLK(1); }
OP80( 0xd4, i_cnc   ) { uint32_t addr = fetchword(); if (!CF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xd5, i_pushd ) { PUSH80(Wreg(DW)); }
OP80( 0xd6, i_sui   ) { uint32_t src = fetch(), dst = Breg(AL); SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0xd7, i_rst2  ) { PUSH80(m_ip); m_ip = 0x10; CHANGE_PC; CLK(1); }
OP80( 0xd8, i_rc    ) { if (CF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xda, i_jc    ) { uint32_t addr = fetchword(); if (CF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xdb, i_in    ) { uint8_t port = fetch(); Breg(AL) = read_port_byte(port); CLK(1); }
OP80( 0xdc, i_cc    ) { uint32_t addr = fetchword(); if (CF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xde, i_sbi   ) { uint32_t src = fetch(), dst = Breg(AL); src+=CF; SUBB; Breg(AL) = dst; CLK(1); }
OP80( 0xdf, i_rst3  ) { PUSH80(m_ip); m_ip = 0x18; CHANGE_PC; CLK(1); }
OP80( 0xe0, i_rpo   ) { if (!PF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xe1, i_poph  ) { POP80(Wreg(BW)); }
OP80( 0xe2, i_jpo   ) { uint32_t addr = fetchword(); if (!PF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xe3, i_xthl  ) { uint32_t tmp = Wreg(BW); POP80(Wreg(BW)); PUSH80(tmp); CLK(1); }
OP80( 0xe4, i_cpo   ) { uint32_t addr = fetchword(); if (!PF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xe5, i_pushh ) { PUSH80(Wreg(BW)); }
OP80( 0xe6, i_ani   ) { uint32_t src = fetch(), dst = Breg(AL); ANDB; Breg(AL) = dst; CLK(1); }
OP80( 0xe7, i_rst4  ) { PUSH80(m_ip); m_ip = 0x20; CHANGE_PC; CLK(1); }
OP80( 0xe8, i_rpe   ) { if (PF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xe9, i_pchl  ) { m_ip = Wreg(BW); CHANGE_PC; CLK(1); };
OP80( 0xea, i_jpe   ) { uint32_t addr = fetchword(); if (PF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xeb, i_xchg  ) { uint32_t tmp = Wreg(BW); Wreg(BW) = Wreg(DW); Wreg(DW) = tmp; CLK(1); }
OP80( 0xec, i_cpe   ) { uint32_t addr = fetchword(); if (PF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xed, i_calln ) { uint32_t addr = fetch();
	switch (addr) {
		case 0xed:
			nec_interrupt(fetch(), BRK); break;
		case 0xfd:
			m_em = 1; POP(m_ip); POP(Sreg(PS)); i_popf(); CHANGE_PC; CLK(1); break;
		default:
			PUSH80(m_ip); m_ip = addr | (fetch() << 8); CHANGE_PC; CLK(1); break;
	}
}
OP80( 0xee, i_xri   ) { uint32_t src = fetch(), dst = Breg(AL); XORB; Breg(AL) = dst; CLK(1); }
OP80( 0xef, i_rst5  ) { PUSH80(m_ip); m_ip = 0x28; CHANGE_PC; CLK(1); }
OP80( 0xf0, i_rp    ) { if (!SF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xf1, i_popf  ) { uint16_t tmp; POP80(tmp); Breg(AL) = tmp >> 8; ExpandFlags((CompressFlags() & 0xff00) | (tmp & 0xff))}
OP80( 0xf2, i_jp    ) { uint32_t addr = fetchword(); if (!SF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xf3, i_di    ) { SetIF(0); CLK(1); }
OP80( 0xf4, i_cp    ) { uint32_t addr = fetchword(); if (!SF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xf5, i_pushf ) { uint16_t f = CompressFlags(); PUSH80((Breg(AL) << 8) | (f & 0xff)); }
OP80( 0xf6, i_ori   ) { uint32_t src = fetch(), dst = Breg(AL); ORB; Breg(AL) = dst; CLK(1); }
OP80( 0xf7, i_rst6  ) { PUSH80(m_ip); m_ip = 0x30; CHANGE_PC; CLK(1); }
OP80( 0xf8, i_rm    ) { if (SF) { POP80(m_ip); CHANGE_PC; } CLK(1); }
OP80( 0xf9, i_sphl  ) { Wreg(BP) = Wreg(BW); CLK(1); };
OP80( 0xfa, i_jm    ) { uint32_t addr = fetchword(); if (SF) { m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xfb, i_ei    ) { SetIF(1); CLK(1); }
OP80( 0xfc, i_cm    ) { uint32_t addr = fetchword(); if (SF) { PUSH80(m_ip); m_ip = addr; CHANGE_PC; } CLK(1); }
OP80( 0xfe, i_cpi   ) { uint32_t src = fetch(), dst = Breg(AL); SUBB; CLK(1); }
OP80( 0xff, i_rst7  ) { PUSH80(m_ip); m_ip = 0x38; CHANGE_PC; CLK(1); }
