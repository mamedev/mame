// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
OP(illegal,2)
{
	logerror("Z180 '%s' ill. opcode $ed $%02x\n",
			tag(), m_odirect->read_byte((_PCD-1)&0xffff));
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP(ed,00) { unsigned n = ARG(); _B = IN( n );                       } /* IN0  B,(n)       */
OP(ed,01) { unsigned n = ARG(); OUT( n, _B );                       } /* OUT0 (n),B       */
OP(ed,02) { illegal_2();                                            } /* DB   ED          */
OP(ed,03) { illegal_2();                                            } /* DB   ED          */
OP(ed,04) { TST( _B );                                                } /* TST  B           */
OP(ed,05) { illegal_2();                                            } /* DB   ED          */
OP(ed,06) { illegal_2();                                            } /* DB   ED          */
OP(ed,07) { illegal_2();                                            } /* DB   ED          */

OP(ed,08) { unsigned n = ARG(); _C = IN( n );                       } /* IN0  C,(n)       */
OP(ed,09) { unsigned n = ARG(); OUT( n, _C );                       } /* OUT0 (n),C       */
OP(ed,0a) { illegal_2();                                            } /* DB   ED          */
OP(ed,0b) { illegal_2();                                            } /* DB   ED          */
OP(ed,0c) { TST( _C );                                                } /* TST  C           */
OP(ed,0d) { illegal_2();                                            } /* DB   ED          */
OP(ed,0e) { illegal_2();                                            } /* DB   ED          */
OP(ed,0f) { illegal_2();                                            } /* DB   ED          */

OP(ed,10) { unsigned n = ARG(); _D = IN( n );                       } /* IN0  D,(n)       */
OP(ed,11) { unsigned n = ARG(); OUT( n, _D );                       } /* OUT0 (n),D       */
OP(ed,12) { illegal_2();                                            } /* DB   ED          */
OP(ed,13) { illegal_2();                                            } /* DB   ED          */
OP(ed,14) { TST( _D );                                                } /* TST  D           */
OP(ed,15) { illegal_2();                                            } /* DB   ED          */
OP(ed,16) { illegal_2();                                            } /* DB   ED          */
OP(ed,17) { illegal_2();                                            } /* DB   ED          */

OP(ed,18) { unsigned n = ARG(); _E = IN( n );                       } /* IN0  E,(n)       */
OP(ed,19) { unsigned n = ARG(); OUT( n, _E );                       } /* OUT0 (n),E       */
OP(ed,1a) { illegal_2();                                            } /* DB   ED          */
OP(ed,1b) { illegal_2();                                            } /* DB   ED          */
OP(ed,1c) { TST( _E );                                                } /* TST  E           */
OP(ed,1d) { illegal_2();                                            } /* DB   ED          */
OP(ed,1e) { illegal_2();                                            } /* DB   ED          */
OP(ed,1f) { illegal_2();                                            } /* DB   ED          */

OP(ed,20) { unsigned n = ARG(); _H = IN( n );                       } /* IN0  H,(n)       */
OP(ed,21) { unsigned n = ARG(); OUT( n, _H );                       } /* OUT0 (n),H       */
OP(ed,22) { illegal_2();                                            } /* DB   ED          */
OP(ed,23) { illegal_2();                                            } /* DB   ED          */
OP(ed,24) { TST( _H );                                                } /* TST  H           */
OP(ed,25) { illegal_2();                                            } /* DB   ED          */
OP(ed,26) { illegal_2();                                            } /* DB   ED          */
OP(ed,27) { illegal_2();                                            } /* DB   ED          */

OP(ed,28) { unsigned n = ARG(); _L = IN( n );                       } /* IN0  L,(n)       */
OP(ed,29) { unsigned n = ARG(); OUT( n, _L );                       } /* OUT0 (n),L       */
OP(ed,2a) { illegal_2();                                            } /* DB   ED          */
OP(ed,2b) { illegal_2();                                            } /* DB   ED          */
OP(ed,2c) { TST( _L );                                                } /* TST  L           */
OP(ed,2d) { illegal_2();                                            } /* DB   ED          */
OP(ed,2e) { illegal_2();                                            } /* DB   ED          */
OP(ed,2f) { illegal_2();                                            } /* DB   ED          */

OP(ed,30) { unsigned n = ARG(); IN( n );                          } /* IN0  (n)         */
OP(ed,31) { unsigned n = ARG(); OUT( n, 0 );                      } /* OUT0 (n)         */
OP(ed,32) { illegal_2();                                            } /* DB   ED          */
OP(ed,33) { illegal_2();                                            } /* DB   ED          */
OP(ed,34) { TST( RM(_HL) );                                         } /* TST  (HL)        */
OP(ed,35) { illegal_2();                                            } /* DB   ED          */
OP(ed,36) { illegal_2();                                            } /* DB   ED          */
OP(ed,37) { illegal_2();                                            } /* DB   ED          */

OP(ed,38) { unsigned n = ARG(); _A = IN( n );                       } /* IN0  A,(n)       */
OP(ed,39) { unsigned n = ARG(); OUT( n, _A );                       } /* OUT0 (n),A       */
OP(ed,3a) { illegal_2();                                            } /* DB   ED          */
OP(ed,3b) { illegal_2();                                            } /* DB   ED          */
OP(ed,3c) { TST( _A );                                                } /* TST  A           */
OP(ed,3d) { illegal_2();                                            } /* DB   ED          */
OP(ed,3e) { illegal_2();                                            } /* DB   ED          */
OP(ed,3f) { illegal_2();                                            } /* DB   ED          */

OP(ed,40) { _B = IN(_BC); _F = (_F & CF) | SZP[_B];                 } /* IN   B,(C)       */
OP(ed,41) { OUT(_BC,_B);                                          } /* OUT  (C),B       */
OP(ed,42) { SBC16( BC );                                            } /* SBC  HL,BC       */
OP(ed,43) { m_ea = ARG16(); WM16( m_ea, &m_BC );                  } /* LD   (w),BC      */
OP(ed,44) { NEG;                                                    } /* NEG              */
OP(ed,45) { RETN;                                                   } /* RETN;            */
OP(ed,46) { m_IM = 0;                                               } /* IM   0           */
OP(ed,47) { LD_I_A;                                                 } /* LD   I,A         */

OP(ed,48) { _C = IN(_BC); _F = (_F & CF) | SZP[_C];                 } /* IN   C,(C)       */
OP(ed,49) { OUT(_BC,_C);                                          } /* OUT  (C),C       */
OP(ed,4a) { ADC16( BC );                                            } /* ADC  HL,BC       */
OP(ed,4b) { m_ea = ARG16(); RM16( m_ea, &m_BC );                  } /* LD   BC,(w)      */
OP(ed,4c) { MLT( BC );                                              } /* MLT  BC          */
OP(ed,4d) { RETI;                                                   } /* RETI             */
OP(ed,4e) { m_IM = 0;                                               } /* IM   0           */
OP(ed,4f) { LD_R_A;                                                 } /* LD   R,A         */

OP(ed,50) { _D = IN(_BC); _F = (_F & CF) | SZP[_D];                 } /* IN   D,(C)       */
OP(ed,51) { OUT(_BC,_D);                                          } /* OUT  (C),D       */
OP(ed,52) { SBC16( DE );                                            } /* SBC  HL,DE       */
OP(ed,53) { m_ea = ARG16(); WM16( m_ea, &m_DE );                  } /* LD   (w),DE      */
OP(ed,54) { NEG;                                                    } /* NEG              */
OP(ed,55) { RETN;                                                   } /* RETN;            */
OP(ed,56) { m_IM = 1;                                               } /* IM   1           */
OP(ed,57) { LD_A_I;                                                 } /* LD   A,I         */

OP(ed,58) { _E = IN(_BC); _F = (_F & CF) | SZP[_E];                 } /* IN   E,(C)       */
OP(ed,59) { OUT(_BC,_E);                                          } /* OUT  (C),E       */
OP(ed,5a) { ADC16( DE );                                            } /* ADC  HL,DE       */
OP(ed,5b) { m_ea = ARG16(); RM16( m_ea, &m_DE );                  } /* LD   DE,(w)      */
OP(ed,5c) { MLT( DE );                                              } /* MLT  DE          */
OP(ed,5d) { RETI;                                                   } /* RETI             */
OP(ed,5e) { m_IM = 2;                                               } /* IM   2           */
OP(ed,5f) { LD_A_R;                                                 } /* LD   A,R         */

OP(ed,60) { _H = IN(_BC); _F = (_F & CF) | SZP[_H];                 } /* IN   H,(C)       */
OP(ed,61) { OUT(_BC,_H);                                          } /* OUT  (C),H       */
OP(ed,62) { SBC16( HL );                                            } /* SBC  HL,HL       */
OP(ed,63) { m_ea = ARG16(); WM16( m_ea, &m_HL );                  } /* LD   (w),HL      */
OP(ed,64) { unsigned m = ARG(); TST( m );                           } /* TST  m           */
OP(ed,65) { RETN;                                                   } /* RETN;            */
OP(ed,66) { m_IM = 0;                                               } /* IM   0           */
OP(ed,67) { RRD;                                                    } /* RRD  (HL)        */

OP(ed,68) { _L = IN(_BC); _F = (_F & CF) | SZP[_L];                 } /* IN   L,(C)       */
OP(ed,69) { OUT(_BC,_L);                                          } /* OUT  (C),L       */
OP(ed,6a) { ADC16( HL );                                            } /* ADC  HL,HL       */
OP(ed,6b) { m_ea = ARG16(); RM16( m_ea, &m_HL );                  } /* LD   HL,(w)      */
OP(ed,6c) { MLT( HL );                                              } /* MLT  HL          */
OP(ed,6d) { RETI;                                                   } /* RETI             */
OP(ed,6e) { m_IM = 0;                                               } /* IM   0           */
OP(ed,6f) { RLD;                                                    } /* RLD  (HL)        */

OP(ed,70) { UINT8 res = IN(_BC); _F = (_F & CF) | SZP[res];         } /* IN   0,(C)       */
OP(ed,71) { OUT(_BC,0);                                             } /* OUT  (C),0       */
OP(ed,72) { SBC16( SP );                                            } /* SBC  HL,SP       */
OP(ed,73) { m_ea = ARG16(); WM16( m_ea, &m_SP );                  } /* LD   (w),SP      */
OP(ed,74) { unsigned m = ARG(); _F = (_F & CF) | SZP[IN(_C) & m];   } /* TSTIO m          */
OP(ed,75) { RETN;                                                   } /* RETN;            */
OP(ed,76) { SLP;                                                    } /* SLP              */
OP(ed,77) { illegal_2();                                            } /* DB   ED,77       */

OP(ed,78) { _A = IN(_BC); _F = (_F & CF) | SZP[_A];                 } /* IN   E,(C)       */
OP(ed,79) { OUT(_BC,_A);                                          } /* OUT  (C),E       */
OP(ed,7a) { ADC16( SP );                                            } /* ADC  HL,SP       */
OP(ed,7b) { m_ea = ARG16(); RM16( m_ea, &m_SP );                  } /* LD   SP,(w)      */
OP(ed,7c) { MLT( SP );                                              } /* MLT  SP          */
OP(ed,7d) { RETI;                                                   } /* RETI             */
OP(ed,7e) { m_IM = 2;                                               } /* IM   2           */
OP(ed,7f) { illegal_2();                                            } /* DB   ED,7F       */

OP(ed,80) { illegal_2();                                            } /* DB   ED          */
OP(ed,81) { illegal_2();                                            } /* DB   ED          */
OP(ed,82) { illegal_2();                                            } /* DB   ED          */
OP(ed,83) { OTIM;                                                   } /* OTIM             */
OP(ed,84) { illegal_2();                                            } /* DB   ED          */
OP(ed,85) { illegal_2();                                            } /* DB   ED          */
OP(ed,86) { illegal_2();                                            } /* DB   ED          */
OP(ed,87) { illegal_2();                                            } /* DB   ED          */

OP(ed,88) { illegal_2();                                            } /* DB   ED          */
OP(ed,89) { illegal_2();                                            } /* DB   ED          */
OP(ed,8a) { illegal_2();                                            } /* DB   ED          */
OP(ed,8b) { OTDM;                                                   } /* OTDM             */
OP(ed,8c) { illegal_2();                                            } /* DB   ED          */
OP(ed,8d) { illegal_2();                                            } /* DB   ED          */
OP(ed,8e) { illegal_2();                                            } /* DB   ED          */
OP(ed,8f) { illegal_2();                                            } /* DB   ED          */

OP(ed,90) { illegal_2();                                            } /* DB   ED          */
OP(ed,91) { illegal_2();                                            } /* DB   ED          */
OP(ed,92) { illegal_2();                                            } /* DB   ED          */
OP(ed,93) { OTIMR;                                                  } /* OTIMR            */
OP(ed,94) { illegal_2();                                            } /* DB   ED          */
OP(ed,95) { illegal_2();                                            } /* DB   ED          */
OP(ed,96) { illegal_2();                                            } /* DB   ED          */
OP(ed,97) { illegal_2();                                            } /* DB   ED          */

OP(ed,98) { illegal_2();                                            } /* DB   ED          */
OP(ed,99) { illegal_2();                                            } /* DB   ED          */
OP(ed,9a) { illegal_2();                                            } /* DB   ED          */
OP(ed,9b) { OTDMR;                                                  } /* OTDMR            */
OP(ed,9c) { illegal_2();                                            } /* DB   ED          */
OP(ed,9d) { illegal_2();                                            } /* DB   ED          */
OP(ed,9e) { illegal_2();                                            } /* DB   ED          */
OP(ed,9f) { illegal_2();                                            } /* DB   ED          */

OP(ed,a0) { LDI;                                                    } /* LDI              */
OP(ed,a1) { CPI;                                                    } /* CPI              */
OP(ed,a2) { INI;                                                    } /* INI              */
OP(ed,a3) { OUTI;                                                   } /* OUTI             */
OP(ed,a4) { illegal_2();                                            } /* DB   ED          */
OP(ed,a5) { illegal_2();                                            } /* DB   ED          */
OP(ed,a6) { illegal_2();                                            } /* DB   ED          */
OP(ed,a7) { illegal_2();                                            } /* DB   ED          */

OP(ed,a8) { LDD;                                                    } /* LDD              */
OP(ed,a9) { CPD;                                                    } /* CPD              */
OP(ed,aa) { IND;                                                    } /* IND              */
OP(ed,ab) { OUTD;                                                   } /* OUTD             */
OP(ed,ac) { illegal_2();                                            } /* DB   ED          */
OP(ed,ad) { illegal_2();                                            } /* DB   ED          */
OP(ed,ae) { illegal_2();                                            } /* DB   ED          */
OP(ed,af) { illegal_2();                                            } /* DB   ED          */

OP(ed,b0) { LDIR;                                                   } /* LDIR             */
OP(ed,b1) { CPIR;                                                   } /* CPIR             */
OP(ed,b2) { INIR;                                                   } /* INIR             */
OP(ed,b3) { OTIR;                                                   } /* OTIR             */
OP(ed,b4) { illegal_2();                                            } /* DB   ED          */
OP(ed,b5) { illegal_2();                                            } /* DB   ED          */
OP(ed,b6) { illegal_2();                                            } /* DB   ED          */
OP(ed,b7) { illegal_2();                                            } /* DB   ED          */

OP(ed,b8) { LDDR;                                                   } /* LDDR             */
OP(ed,b9) { CPDR;                                                   } /* CPDR             */
OP(ed,ba) { INDR;                                                   } /* INDR             */
OP(ed,bb) { OTDR;                                                   } /* OTDR             */
OP(ed,bc) { illegal_2();                                            } /* DB   ED          */
OP(ed,bd) { illegal_2();                                            } /* DB   ED          */
OP(ed,be) { illegal_2();                                            } /* DB   ED          */
OP(ed,bf) { illegal_2();                                            } /* DB   ED          */

OP(ed,c0) { illegal_2();                                            } /* DB   ED          */
OP(ed,c1) { illegal_2();                                            } /* DB   ED          */
OP(ed,c2) { illegal_2();                                            } /* DB   ED          */
OP(ed,c3) { illegal_2();                                            } /* DB   ED          */
OP(ed,c4) { illegal_2();                                            } /* DB   ED          */
OP(ed,c5) { illegal_2();                                            } /* DB   ED          */
OP(ed,c6) { illegal_2();                                            } /* DB   ED          */
OP(ed,c7) { illegal_2();                                            } /* DB   ED          */

OP(ed,c8) { illegal_2();                                            } /* DB   ED          */
OP(ed,c9) { illegal_2();                                            } /* DB   ED          */
OP(ed,ca) { illegal_2();                                            } /* DB   ED          */
OP(ed,cb) { illegal_2();                                            } /* DB   ED          */
OP(ed,cc) { illegal_2();                                            } /* DB   ED          */
OP(ed,cd) { illegal_2();                                            } /* DB   ED          */
OP(ed,ce) { illegal_2();                                            } /* DB   ED          */
OP(ed,cf) { illegal_2();                                            } /* DB   ED          */

OP(ed,d0) { illegal_2();                                            } /* DB   ED          */
OP(ed,d1) { illegal_2();                                            } /* DB   ED          */
OP(ed,d2) { illegal_2();                                            } /* DB   ED          */
OP(ed,d3) { illegal_2();                                            } /* DB   ED          */
OP(ed,d4) { illegal_2();                                            } /* DB   ED          */
OP(ed,d5) { illegal_2();                                            } /* DB   ED          */
OP(ed,d6) { illegal_2();                                            } /* DB   ED          */
OP(ed,d7) { illegal_2();                                            } /* DB   ED          */

OP(ed,d8) { illegal_2();                                            } /* DB   ED          */
OP(ed,d9) { illegal_2();                                            } /* DB   ED          */
OP(ed,da) { illegal_2();                                            } /* DB   ED          */
OP(ed,db) { illegal_2();                                            } /* DB   ED          */
OP(ed,dc) { illegal_2();                                            } /* DB   ED          */
OP(ed,dd) { illegal_2();                                            } /* DB   ED          */
OP(ed,de) { illegal_2();                                            } /* DB   ED          */
OP(ed,df) { illegal_2();                                            } /* DB   ED          */

OP(ed,e0) { illegal_2();                                            } /* DB   ED          */
OP(ed,e1) { illegal_2();                                            } /* DB   ED          */
OP(ed,e2) { illegal_2();                                            } /* DB   ED          */
OP(ed,e3) { illegal_2();                                            } /* DB   ED          */
OP(ed,e4) { illegal_2();                                            } /* DB   ED          */
OP(ed,e5) { illegal_2();                                            } /* DB   ED          */
OP(ed,e6) { illegal_2();                                            } /* DB   ED          */
OP(ed,e7) { illegal_2();                                            } /* DB   ED          */

OP(ed,e8) { illegal_2();                                            } /* DB   ED          */
OP(ed,e9) { illegal_2();                                            } /* DB   ED          */
OP(ed,ea) { illegal_2();                                            } /* DB   ED          */
OP(ed,eb) { illegal_2();                                            } /* DB   ED          */
OP(ed,ec) { illegal_2();                                            } /* DB   ED          */
OP(ed,ed) { illegal_2();                                            } /* DB   ED          */
OP(ed,ee) { illegal_2();                                            } /* DB   ED          */
OP(ed,ef) { illegal_2();                                            } /* DB   ED          */

OP(ed,f0) { illegal_2();                                            } /* DB   ED          */
OP(ed,f1) { illegal_2();                                            } /* DB   ED          */
OP(ed,f2) { illegal_2();                                            } /* DB   ED          */
OP(ed,f3) { illegal_2();                                            } /* DB   ED          */
OP(ed,f4) { illegal_2();                                            } /* DB   ED          */
OP(ed,f5) { illegal_2();                                            } /* DB   ED          */
OP(ed,f6) { illegal_2();                                            } /* DB   ED          */
OP(ed,f7) { illegal_2();                                            } /* DB   ED          */

OP(ed,f8) { illegal_2();                                            } /* DB   ED          */
OP(ed,f9) { illegal_2();                                            } /* DB   ED          */
OP(ed,fa) { illegal_2();                                            } /* DB   ED          */
OP(ed,fb) { illegal_2();                                            } /* DB   ED          */
OP(ed,fc) { illegal_2();                                            } /* DB   ED          */
OP(ed,fd) { illegal_2();                                            } /* DB   ED          */
OP(ed,fe) { illegal_2();                                            } /* DB   ED          */
OP(ed,ff) { illegal_2();                                            } /* DB   ED          */
