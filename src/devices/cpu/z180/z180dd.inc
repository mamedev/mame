// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
OP(illegal,1) {
	logerror("Z180 '%s' ill. opcode $%02x $%02x\n",
			tag().c_str(), m_odirect->read_byte((_PCD-1)&0xffff), m_odirect->read_byte(_PCD));
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP(dd,00) { illegal_1(); op_00();                                   } /* DB   DD          */
OP(dd,01) { illegal_1(); op_01();                                   } /* DB   DD          */
OP(dd,02) { illegal_1(); op_02();                                   } /* DB   DD          */
OP(dd,03) { illegal_1(); op_03();                                   } /* DB   DD          */
OP(dd,04) { illegal_1(); op_04();                                   } /* DB   DD          */
OP(dd,05) { illegal_1(); op_05();                                   } /* DB   DD          */
OP(dd,06) { illegal_1(); op_06();                                   } /* DB   DD          */
OP(dd,07) { illegal_1(); op_07();                                   } /* DB   DD          */

OP(dd,08) { illegal_1(); op_08();                                   } /* DB   DD          */
OP(dd,09) { m_R++; ADD16(IX,BC);                                    } /* ADD  IX,BC       */
OP(dd,0a) { illegal_1(); op_0a();                                   } /* DB   DD          */
OP(dd,0b) { illegal_1(); op_0b();                                   } /* DB   DD          */
OP(dd,0c) { illegal_1(); op_0c();                                   } /* DB   DD          */
OP(dd,0d) { illegal_1(); op_0d();                                   } /* DB   DD          */
OP(dd,0e) { illegal_1(); op_0e();                                   } /* DB   DD          */
OP(dd,0f) { illegal_1(); op_0f();                                   } /* DB   DD          */

OP(dd,10) { illegal_1(); op_10();                                   } /* DB   DD          */
OP(dd,11) { illegal_1(); op_11();                                   } /* DB   DD          */
OP(dd,12) { illegal_1(); op_12();                                   } /* DB   DD          */
OP(dd,13) { illegal_1(); op_13();                                   } /* DB   DD          */
OP(dd,14) { illegal_1(); op_14();                                   } /* DB   DD          */
OP(dd,15) { illegal_1(); op_15();                                   } /* DB   DD          */
OP(dd,16) { illegal_1(); op_16();                                   } /* DB   DD          */
OP(dd,17) { illegal_1(); op_17();                                   } /* DB   DD          */

OP(dd,18) { illegal_1(); op_18();                                   } /* DB   DD          */
OP(dd,19) { m_R++; ADD16(IX,DE);                                    } /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(); op_1a();                                   } /* DB   DD          */
OP(dd,1b) { illegal_1(); op_1b();                                   } /* DB   DD          */
OP(dd,1c) { illegal_1(); op_1c();                                   } /* DB   DD          */
OP(dd,1d) { illegal_1(); op_1d();                                   } /* DB   DD          */
OP(dd,1e) { illegal_1(); op_1e();                                   } /* DB   DD          */
OP(dd,1f) { illegal_1(); op_1f();                                   } /* DB   DD          */

OP(dd,20) { illegal_1(); op_20();                                   } /* DB   DD          */
OP(dd,21) { m_R++; _IX = ARG16();                                 } /* LD   IX,w        */
OP(dd,22) { m_R++; m_ea = ARG16(); WM16( m_ea, &m_IX );               } /* LD   (w),IX      */
OP(dd,23) { m_R++; _IX++;                                         } /* INC  IX          */
OP(dd,24) { m_R++; _HX = INC(_HX);                                    } /* INC  HX          */
OP(dd,25) { m_R++; _HX = DEC(_HX);                                    } /* DEC  HX          */
OP(dd,26) { m_R++; _HX = ARG();                                       } /* LD   HX,n        */
OP(dd,27) { illegal_1(); op_27();                                   } /* DB   DD          */

OP(dd,28) { illegal_1(); op_28();                                   } /* DB   DD          */
OP(dd,29) { m_R++; ADD16(IX,IX);                                    } /* ADD  IX,IX       */
OP(dd,2a) { m_R++; m_ea = ARG16(); RM16( m_ea, &m_IX );               } /* LD   IX,(w)      */
OP(dd,2b) { m_R++; _IX--;                                         } /* DEC  IX          */
OP(dd,2c) { m_R++; _LX = INC(_LX);                                    } /* INC  LX          */
OP(dd,2d) { m_R++; _LX = DEC(_LX);                                    } /* DEC  LX          */
OP(dd,2e) { m_R++; _LX = ARG();                                       } /* LD   LX,n        */
OP(dd,2f) { illegal_1(); op_2f();                                   } /* DB   DD          */

OP(dd,30) { illegal_1(); op_30();                                   } /* DB   DD          */
OP(dd,31) { illegal_1(); op_31();                                   } /* DB   DD          */
OP(dd,32) { illegal_1(); op_32();                                   } /* DB   DD          */
OP(dd,33) { illegal_1(); op_33();                                   } /* DB   DD          */
OP(dd,34) { m_R++; EAX(); WM( m_ea, INC(RM(m_ea)) );                      } /* INC  (IX+o)      */
OP(dd,35) { m_R++; EAX(); WM( m_ea, DEC(RM(m_ea)) );                      } /* DEC  (IX+o)      */
OP(dd,36) { m_R++; EAX(); WM( m_ea, ARG() );                          } /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(); op_37();                                   } /* DB   DD          */

OP(dd,38) { illegal_1(); op_38();                                   } /* DB   DD          */
OP(dd,39) { m_R++; ADD16(IX,SP);                                    } /* ADD  IX,SP       */
OP(dd,3a) { illegal_1(); op_3a();                                   } /* DB   DD          */
OP(dd,3b) { illegal_1(); op_3b();                                   } /* DB   DD          */
OP(dd,3c) { illegal_1(); op_3c();                                   } /* DB   DD          */
OP(dd,3d) { illegal_1(); op_3d();                                   } /* DB   DD          */
OP(dd,3e) { illegal_1(); op_3e();                                   } /* DB   DD          */
OP(dd,3f) { illegal_1(); op_3f();                                   } /* DB   DD          */

OP(dd,40) { illegal_1(); op_40();                                   } /* DB   DD          */
OP(dd,41) { illegal_1(); op_41();                                   } /* DB   DD          */
OP(dd,42) { illegal_1(); op_42();                                   } /* DB   DD          */
OP(dd,43) { illegal_1(); op_43();                                   } /* DB   DD          */
OP(dd,44) { m_R++; _B = _HX;                                        } /* LD   B,HX        */
OP(dd,45) { m_R++; _B = _LX;                                        } /* LD   B,LX        */
OP(dd,46) { m_R++; EAX(); _B = RM(m_ea);                                } /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(); op_47();                                   } /* DB   DD          */

OP(dd,48) { illegal_1(); op_48();                                   } /* DB   DD          */
OP(dd,49) { illegal_1(); op_49();                                   } /* DB   DD          */
OP(dd,4a) { illegal_1(); op_4a();                                   } /* DB   DD          */
OP(dd,4b) { illegal_1(); op_4b();                                   } /* DB   DD          */
OP(dd,4c) { m_R++; _C = _HX;                                        } /* LD   C,HX        */
OP(dd,4d) { m_R++; _C = _LX;                                        } /* LD   C,LX        */
OP(dd,4e) { m_R++; EAX(); _C = RM(m_ea);                                } /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(); op_4f();                                   } /* DB   DD          */

OP(dd,50) { illegal_1(); op_50();                                   } /* DB   DD          */
OP(dd,51) { illegal_1(); op_51();                                   } /* DB   DD          */
OP(dd,52) { illegal_1(); op_52();                                   } /* DB   DD          */
OP(dd,53) { illegal_1(); op_53();                                   } /* DB   DD          */
OP(dd,54) { m_R++; _D = _HX;                                        } /* LD   D,HX        */
OP(dd,55) { m_R++; _D = _LX;                                        } /* LD   D,LX        */
OP(dd,56) { m_R++; EAX(); _D = RM(m_ea);                                } /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(); op_57();                                   } /* DB   DD          */

OP(dd,58) { illegal_1(); op_58();                                   } /* DB   DD          */
OP(dd,59) { illegal_1(); op_59();                                   } /* DB   DD          */
OP(dd,5a) { illegal_1(); op_5a();                                   } /* DB   DD          */
OP(dd,5b) { illegal_1(); op_5b();                                   } /* DB   DD          */
OP(dd,5c) { m_R++; _E = _HX;                                        } /* LD   E,HX        */
OP(dd,5d) { m_R++; _E = _LX;                                        } /* LD   E,LX        */
OP(dd,5e) { m_R++; EAX(); _E = RM(m_ea);                                } /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(); op_5f();                                   } /* DB   DD          */

OP(dd,60) { m_R++; _HX = _B;                                        } /* LD   HX,B        */
OP(dd,61) { m_R++; _HX = _C;                                        } /* LD   HX,C        */
OP(dd,62) { m_R++; _HX = _D;                                        } /* LD   HX,D        */
OP(dd,63) { m_R++; _HX = _E;                                        } /* LD   HX,E        */
OP(dd,64) {                                                         } /* LD   HX,HX       */
OP(dd,65) { m_R++; _HX = _LX;                                       } /* LD   HX,LX       */
OP(dd,66) { m_R++; EAX(); _H = RM(m_ea);                                } /* LD   H,(IX+o)    */
OP(dd,67) { m_R++; _HX = _A;                                        } /* LD   HX,A        */

OP(dd,68) { m_R++; _LX = _B;                                        } /* LD   LX,B        */
OP(dd,69) { m_R++; _LX = _C;                                        } /* LD   LX,C        */
OP(dd,6a) { m_R++; _LX = _D;                                        } /* LD   LX,D        */
OP(dd,6b) { m_R++; _LX = _E;                                        } /* LD   LX,E        */
OP(dd,6c) { m_R++; _LX = _HX;                                       } /* LD   LX,HX       */
OP(dd,6d) {                                                         } /* LD   LX,LX       */
OP(dd,6e) { m_R++; EAX(); _L = RM(m_ea);                                } /* LD   L,(IX+o)    */
OP(dd,6f) { m_R++; _LX = _A;                                        } /* LD   LX,A        */

OP(dd,70) { m_R++; EAX(); WM( m_ea, _B );                               } /* LD   (IX+o),B    */
OP(dd,71) { m_R++; EAX(); WM( m_ea, _C );                               } /* LD   (IX+o),C    */
OP(dd,72) { m_R++; EAX(); WM( m_ea, _D );                               } /* LD   (IX+o),D    */
OP(dd,73) { m_R++; EAX(); WM( m_ea, _E );                               } /* LD   (IX+o),E    */
OP(dd,74) { m_R++; EAX(); WM( m_ea, _H );                               } /* LD   (IX+o),H    */
OP(dd,75) { m_R++; EAX(); WM( m_ea, _L );                               } /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(); op_76();                                   }         /* DB   DD          */
OP(dd,77) { m_R++; EAX(); WM( m_ea, _A );                               } /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(); op_78();                                   } /* DB   DD          */
OP(dd,79) { illegal_1(); op_79();                                   } /* DB   DD          */
OP(dd,7a) { illegal_1(); op_7a();                                   } /* DB   DD          */
OP(dd,7b) { illegal_1(); op_7b();                                   } /* DB   DD          */
OP(dd,7c) { m_R++; _A = _HX;                                        } /* LD   A,HX        */
OP(dd,7d) { m_R++; _A = _LX;                                        } /* LD   A,LX        */
OP(dd,7e) { m_R++; EAX(); _A = RM(m_ea);                                } /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(); op_7f();                                   } /* DB   DD          */

OP(dd,80) { illegal_1(); op_80();                                   } /* DB   DD          */
OP(dd,81) { illegal_1(); op_81();                                   } /* DB   DD          */
OP(dd,82) { illegal_1(); op_82();                                   } /* DB   DD          */
OP(dd,83) { illegal_1(); op_83();                                   } /* DB   DD          */
OP(dd,84) { m_R++; ADD(_HX);                                      } /* ADD  A,HX        */
OP(dd,85) { m_R++; ADD(_LX);                                      } /* ADD  A,LX        */
OP(dd,86) { m_R++; EAX(); ADD(RM(m_ea));                              } /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(); op_87();                                   } /* DB   DD          */

OP(dd,88) { illegal_1(); op_88();                                   } /* DB   DD          */
OP(dd,89) { illegal_1(); op_89();                                   } /* DB   DD          */
OP(dd,8a) { illegal_1(); op_8a();                                   } /* DB   DD          */
OP(dd,8b) { illegal_1(); op_8b();                                   } /* DB   DD          */
OP(dd,8c) { m_R++; ADC(_HX);                                      } /* ADC  A,HX        */
OP(dd,8d) { m_R++; ADC(_LX);                                      } /* ADC  A,LX        */
OP(dd,8e) { m_R++; EAX(); ADC(RM(m_ea));                              } /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(); op_8f();                                   } /* DB   DD          */

OP(dd,90) { illegal_1(); op_90();                                   } /* DB   DD          */
OP(dd,91) { illegal_1(); op_91();                                   } /* DB   DD          */
OP(dd,92) { illegal_1(); op_92();                                   } /* DB   DD          */
OP(dd,93) { illegal_1(); op_93();                                   } /* DB   DD          */
OP(dd,94) { m_R++; SUB(_HX);                                      } /* SUB  HX          */
OP(dd,95) { m_R++; SUB(_LX);                                      } /* SUB  LX          */
OP(dd,96) { m_R++; EAX(); SUB(RM(m_ea));                              } /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(); op_97();                                   } /* DB   DD          */

OP(dd,98) { illegal_1(); op_98();                                   } /* DB   DD          */
OP(dd,99) { illegal_1(); op_99();                                   } /* DB   DD          */
OP(dd,9a) { illegal_1(); op_9a();                                   } /* DB   DD          */
OP(dd,9b) { illegal_1(); op_9b();                                   } /* DB   DD          */
OP(dd,9c) { m_R++; SBC(_HX);                                      } /* SBC  A,HX        */
OP(dd,9d) { m_R++; SBC(_LX);                                      } /* SBC  A,LX        */
OP(dd,9e) { m_R++; EAX(); SBC(RM(m_ea));                              } /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(); op_9f();                                   } /* DB   DD          */

OP(dd,a0) { illegal_1(); op_a0();                                   } /* DB   DD          */
OP(dd,a1) { illegal_1(); op_a1();                                   } /* DB   DD          */
OP(dd,a2) { illegal_1(); op_a2();                                   } /* DB   DD          */
OP(dd,a3) { illegal_1(); op_a3();                                   } /* DB   DD          */
OP(dd,a4) { m_R++; AND(_HX);                                      } /* AND  HX          */
OP(dd,a5) { m_R++; AND(_LX);                                      } /* AND  LX          */
OP(dd,a6) { m_R++; EAX(); AND(RM(m_ea));                              } /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(); op_a7();                                   } /* DB   DD          */

OP(dd,a8) { illegal_1(); op_a8();                                   } /* DB   DD          */
OP(dd,a9) { illegal_1(); op_a9();                                   } /* DB   DD          */
OP(dd,aa) { illegal_1(); op_aa();                                   } /* DB   DD          */
OP(dd,ab) { illegal_1(); op_ab();                                   } /* DB   DD          */
OP(dd,ac) { m_R++; XOR(_HX);                                      } /* XOR  HX          */
OP(dd,ad) { m_R++; XOR(_LX);                                      } /* XOR  LX          */
OP(dd,ae) { m_R++; EAX(); XOR(RM(m_ea));                              } /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(); op_af();                                   } /* DB   DD          */

OP(dd,b0) { illegal_1(); op_b0();                                   } /* DB   DD          */
OP(dd,b1) { illegal_1(); op_b1();                                   } /* DB   DD          */
OP(dd,b2) { illegal_1(); op_b2();                                   } /* DB   DD          */
OP(dd,b3) { illegal_1(); op_b3();                                   } /* DB   DD          */
OP(dd,b4) { m_R++; OR(_HX);                                           } /* OR   HX          */
OP(dd,b5) { m_R++; OR(_LX);                                           } /* OR   LX          */
OP(dd,b6) { m_R++; EAX(); OR(RM(m_ea));                                   } /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(); op_b7();                                   } /* DB   DD          */

OP(dd,b8) { illegal_1(); op_b8();                                   } /* DB   DD          */
OP(dd,b9) { illegal_1(); op_b9();                                   } /* DB   DD          */
OP(dd,ba) { illegal_1(); op_ba();                                   } /* DB   DD          */
OP(dd,bb) { illegal_1(); op_bb();                                   } /* DB   DD          */
OP(dd,bc) { m_R++; CP(_HX);                                           } /* CP   HX          */
OP(dd,bd) { m_R++; CP(_LX);                                           } /* CP   LX          */
OP(dd,be) { m_R++; EAX(); CP(RM(m_ea));                                   } /* CP   (IX+o)      */
OP(dd,bf) { illegal_1(); op_bf();                                   } /* DB   DD          */

OP(dd,c0) { illegal_1(); op_c0();                                   } /* DB   DD          */
OP(dd,c1) { illegal_1(); op_c1();                                   } /* DB   DD          */
OP(dd,c2) { illegal_1(); op_c2();                                   } /* DB   DD          */
OP(dd,c3) { illegal_1(); op_c3();                                   } /* DB   DD          */
OP(dd,c4) { illegal_1(); op_c4();                                   } /* DB   DD          */
OP(dd,c5) { illegal_1(); op_c5();                                   } /* DB   DD          */
OP(dd,c6) { illegal_1(); op_c6();                                   } /* DB   DD          */
OP(dd,c7) { illegal_1(); op_c7();                                   }         /* DB   DD          */

OP(dd,c8) { illegal_1(); op_c8();                                   } /* DB   DD          */
OP(dd,c9) { illegal_1(); op_c9();                                   } /* DB   DD          */
OP(dd,ca) { illegal_1(); op_ca();                                   } /* DB   DD          */
OP(dd,cb) { m_R++; EAX(); m_extra_cycles += exec_xycb(ARG());                          } /* **   DD CB xx    */
OP(dd,cc) { illegal_1(); op_cc();                                   } /* DB   DD          */
OP(dd,cd) { illegal_1(); op_cd();                                   } /* DB   DD          */
OP(dd,ce) { illegal_1(); op_ce();                                   } /* DB   DD          */
OP(dd,cf) { illegal_1(); op_cf();                                   } /* DB   DD          */

OP(dd,d0) { illegal_1(); op_d0();                                   } /* DB   DD          */
OP(dd,d1) { illegal_1(); op_d1();                                   } /* DB   DD          */
OP(dd,d2) { illegal_1(); op_d2();                                   } /* DB   DD          */
OP(dd,d3) { illegal_1(); op_d3();                                   } /* DB   DD          */
OP(dd,d4) { illegal_1(); op_d4();                                   } /* DB   DD          */
OP(dd,d5) { illegal_1(); op_d5();                                   } /* DB   DD          */
OP(dd,d6) { illegal_1(); op_d6();                                   } /* DB   DD          */
OP(dd,d7) { illegal_1(); op_d7();                                   } /* DB   DD          */

OP(dd,d8) { illegal_1(); op_d8();                                   } /* DB   DD          */
OP(dd,d9) { illegal_1(); op_d9();                                   } /* DB   DD          */
OP(dd,da) { illegal_1(); op_da();                                   } /* DB   DD          */
OP(dd,db) { illegal_1(); op_db();                                   } /* DB   DD          */
OP(dd,dc) { illegal_1(); op_dc();                                   } /* DB   DD          */
OP(dd,dd) { illegal_1(); op_dd();                                   } /* DB   DD          */
OP(dd,de) { illegal_1(); op_de();                                   } /* DB   DD          */
OP(dd,df) { illegal_1(); op_df();                                   } /* DB   DD          */

OP(dd,e0) { illegal_1(); op_e0();                                   } /* DB   DD          */
OP(dd,e1) { m_R++; POP(IX);                                           } /* POP  IX          */
OP(dd,e2) { illegal_1(); op_e2();                                   } /* DB   DD          */
OP(dd,e3) { m_R++; EXSP(IX);                                        } /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(); op_e4();                                   } /* DB   DD          */
OP(dd,e5) { m_R++; PUSH( IX );                                        } /* PUSH IX          */
OP(dd,e6) { illegal_1(); op_e6();                                   } /* DB   DD          */
OP(dd,e7) { illegal_1(); op_e7();                                   } /* DB   DD          */

OP(dd,e8) { illegal_1(); op_e8();                                   } /* DB   DD          */
OP(dd,e9) { m_R++; _PC = _IX;                                       } /* JP   (IX)        */
OP(dd,ea) { illegal_1(); op_ea();                                   } /* DB   DD          */
OP(dd,eb) { illegal_1(); op_eb();                                   } /* DB   DD          */
OP(dd,ec) { illegal_1(); op_ec();                                   } /* DB   DD          */
OP(dd,ed) { illegal_1(); op_ed();                                   } /* DB   DD          */
OP(dd,ee) { illegal_1(); op_ee();                                   } /* DB   DD          */
OP(dd,ef) { illegal_1(); op_ef();                                   } /* DB   DD          */

OP(dd,f0) { illegal_1(); op_f0();                                   } /* DB   DD          */
OP(dd,f1) { illegal_1(); op_f1();                                   } /* DB   DD          */
OP(dd,f2) { illegal_1(); op_f2();                                   } /* DB   DD          */
OP(dd,f3) { illegal_1(); op_f3();                                   } /* DB   DD          */
OP(dd,f4) { illegal_1(); op_f4();                                   } /* DB   DD          */
OP(dd,f5) { illegal_1(); op_f5();                                   } /* DB   DD          */
OP(dd,f6) { illegal_1(); op_f6();                                   } /* DB   DD          */
OP(dd,f7) { illegal_1(); op_f7();                                   } /* DB   DD          */

OP(dd,f8) { illegal_1(); op_f8();                                   } /* DB   DD          */
OP(dd,f9) { m_R++; _SP = _IX;                                       } /* LD   SP,IX       */
OP(dd,fa) { illegal_1(); op_fa();                                   } /* DB   DD          */
OP(dd,fb) { illegal_1(); op_fb();                                   } /* DB   DD          */
OP(dd,fc) { illegal_1(); op_fc();                                   } /* DB   DD          */
OP(dd,fd) { illegal_1(); op_fd();                                   } /* DB   DD          */
OP(dd,fe) { illegal_1(); op_fe();                                   } /* DB   DD          */
OP(dd,ff) { illegal_1(); op_ff();                                   } /* DB   DD          */
