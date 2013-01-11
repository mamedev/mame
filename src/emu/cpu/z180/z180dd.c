OP(illegal,1) {
	logerror("Z180 '%s' ill. opcode $%02x $%02x\n",
			cpustate->device->tag(), cpustate->direct->read_decrypted_byte((cpustate->_PCD-1)&0xffff), cpustate->direct->read_decrypted_byte(cpustate->_PCD));
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP(dd,00) { illegal_1(cpustate); op_00(cpustate);                                   } /* DB   DD          */
OP(dd,01) { illegal_1(cpustate); op_01(cpustate);                                   } /* DB   DD          */
OP(dd,02) { illegal_1(cpustate); op_02(cpustate);                                   } /* DB   DD          */
OP(dd,03) { illegal_1(cpustate); op_03(cpustate);                                   } /* DB   DD          */
OP(dd,04) { illegal_1(cpustate); op_04(cpustate);                                   } /* DB   DD          */
OP(dd,05) { illegal_1(cpustate); op_05(cpustate);                                   } /* DB   DD          */
OP(dd,06) { illegal_1(cpustate); op_06(cpustate);                                   } /* DB   DD          */
OP(dd,07) { illegal_1(cpustate); op_07(cpustate);                                   } /* DB   DD          */

OP(dd,08) { illegal_1(cpustate); op_08(cpustate);                                   } /* DB   DD          */
OP(dd,09) { cpustate->R++; ADD16(IX,BC);                                    } /* ADD  IX,BC       */
OP(dd,0a) { illegal_1(cpustate); op_0a(cpustate);                                   } /* DB   DD          */
OP(dd,0b) { illegal_1(cpustate); op_0b(cpustate);                                   } /* DB   DD          */
OP(dd,0c) { illegal_1(cpustate); op_0c(cpustate);                                   } /* DB   DD          */
OP(dd,0d) { illegal_1(cpustate); op_0d(cpustate);                                   } /* DB   DD          */
OP(dd,0e) { illegal_1(cpustate); op_0e(cpustate);                                   } /* DB   DD          */
OP(dd,0f) { illegal_1(cpustate); op_0f(cpustate);                                   } /* DB   DD          */

OP(dd,10) { illegal_1(cpustate); op_10(cpustate);                                   } /* DB   DD          */
OP(dd,11) { illegal_1(cpustate); op_11(cpustate);                                   } /* DB   DD          */
OP(dd,12) { illegal_1(cpustate); op_12(cpustate);                                   } /* DB   DD          */
OP(dd,13) { illegal_1(cpustate); op_13(cpustate);                                   } /* DB   DD          */
OP(dd,14) { illegal_1(cpustate); op_14(cpustate);                                   } /* DB   DD          */
OP(dd,15) { illegal_1(cpustate); op_15(cpustate);                                   } /* DB   DD          */
OP(dd,16) { illegal_1(cpustate); op_16(cpustate);                                   } /* DB   DD          */
OP(dd,17) { illegal_1(cpustate); op_17(cpustate);                                   } /* DB   DD          */

OP(dd,18) { illegal_1(cpustate); op_18(cpustate);                                   } /* DB   DD          */
OP(dd,19) { cpustate->R++; ADD16(IX,DE);                                    } /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(cpustate); op_1a(cpustate);                                   } /* DB   DD          */
OP(dd,1b) { illegal_1(cpustate); op_1b(cpustate);                                   } /* DB   DD          */
OP(dd,1c) { illegal_1(cpustate); op_1c(cpustate);                                   } /* DB   DD          */
OP(dd,1d) { illegal_1(cpustate); op_1d(cpustate);                                   } /* DB   DD          */
OP(dd,1e) { illegal_1(cpustate); op_1e(cpustate);                                   } /* DB   DD          */
OP(dd,1f) { illegal_1(cpustate); op_1f(cpustate);                                   } /* DB   DD          */

OP(dd,20) { illegal_1(cpustate); op_20(cpustate);                                   } /* DB   DD          */
OP(dd,21) { cpustate->R++; cpustate->_IX = ARG16(cpustate);                                 } /* LD   IX,w        */
OP(dd,22) { cpustate->R++; cpustate->ea = ARG16(cpustate); WM16(cpustate,  cpustate->ea, &cpustate->IX );               } /* LD   (w),IX      */
OP(dd,23) { cpustate->R++; cpustate->_IX++;                                         } /* INC  IX          */
OP(dd,24) { cpustate->R++; cpustate->_HX = INC(cpustate, cpustate->_HX);                                    } /* INC  HX          */
OP(dd,25) { cpustate->R++; cpustate->_HX = DEC(cpustate, cpustate->_HX);                                    } /* DEC  HX          */
OP(dd,26) { cpustate->R++; cpustate->_HX = ARG(cpustate);                                       } /* LD   HX,n        */
OP(dd,27) { illegal_1(cpustate); op_27(cpustate);                                   } /* DB   DD          */

OP(dd,28) { illegal_1(cpustate); op_28(cpustate);                                   } /* DB   DD          */
OP(dd,29) { cpustate->R++; ADD16(IX,IX);                                    } /* ADD  IX,IX       */
OP(dd,2a) { cpustate->R++; cpustate->ea = ARG16(cpustate); RM16(cpustate,  cpustate->ea, &cpustate->IX );               } /* LD   IX,(w)      */
OP(dd,2b) { cpustate->R++; cpustate->_IX--;                                         } /* DEC  IX          */
OP(dd,2c) { cpustate->R++; cpustate->_LX = INC(cpustate, cpustate->_LX);                                    } /* INC  LX          */
OP(dd,2d) { cpustate->R++; cpustate->_LX = DEC(cpustate, cpustate->_LX);                                    } /* DEC  LX          */
OP(dd,2e) { cpustate->R++; cpustate->_LX = ARG(cpustate);                                       } /* LD   LX,n        */
OP(dd,2f) { illegal_1(cpustate); op_2f(cpustate);                                   } /* DB   DD          */

OP(dd,30) { illegal_1(cpustate); op_30(cpustate);                                   } /* DB   DD          */
OP(dd,31) { illegal_1(cpustate); op_31(cpustate);                                   } /* DB   DD          */
OP(dd,32) { illegal_1(cpustate); op_32(cpustate);                                   } /* DB   DD          */
OP(dd,33) { illegal_1(cpustate); op_33(cpustate);                                   } /* DB   DD          */
OP(dd,34) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, INC(cpustate, RM(cpustate, cpustate->ea)) );                      } /* INC  (IX+o)      */
OP(dd,35) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, DEC(cpustate, RM(cpustate, cpustate->ea)) );                      } /* DEC  (IX+o)      */
OP(dd,36) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, ARG(cpustate) );                          } /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(cpustate); op_37(cpustate);                                   } /* DB   DD          */

OP(dd,38) { illegal_1(cpustate); op_38(cpustate);                                   } /* DB   DD          */
OP(dd,39) { cpustate->R++; ADD16(IX,SP);                                    } /* ADD  IX,SP       */
OP(dd,3a) { illegal_1(cpustate); op_3a(cpustate);                                   } /* DB   DD          */
OP(dd,3b) { illegal_1(cpustate); op_3b(cpustate);                                   } /* DB   DD          */
OP(dd,3c) { illegal_1(cpustate); op_3c(cpustate);                                   } /* DB   DD          */
OP(dd,3d) { illegal_1(cpustate); op_3d(cpustate);                                   } /* DB   DD          */
OP(dd,3e) { illegal_1(cpustate); op_3e(cpustate);                                   } /* DB   DD          */
OP(dd,3f) { illegal_1(cpustate); op_3f(cpustate);                                   } /* DB   DD          */

OP(dd,40) { illegal_1(cpustate); op_40(cpustate);                                   } /* DB   DD          */
OP(dd,41) { illegal_1(cpustate); op_41(cpustate);                                   } /* DB   DD          */
OP(dd,42) { illegal_1(cpustate); op_42(cpustate);                                   } /* DB   DD          */
OP(dd,43) { illegal_1(cpustate); op_43(cpustate);                                   } /* DB   DD          */
OP(dd,44) { cpustate->R++; cpustate->_B = cpustate->_HX;                                        } /* LD   B,HX        */
OP(dd,45) { cpustate->R++; cpustate->_B = cpustate->_LX;                                        } /* LD   B,LX        */
OP(dd,46) { cpustate->R++; EAX(cpustate); cpustate->_B = RM(cpustate, cpustate->ea);                                } /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(cpustate); op_47(cpustate);                                   } /* DB   DD          */

OP(dd,48) { illegal_1(cpustate); op_48(cpustate);                                   } /* DB   DD          */
OP(dd,49) { illegal_1(cpustate); op_49(cpustate);                                   } /* DB   DD          */
OP(dd,4a) { illegal_1(cpustate); op_4a(cpustate);                                   } /* DB   DD          */
OP(dd,4b) { illegal_1(cpustate); op_4b(cpustate);                                   } /* DB   DD          */
OP(dd,4c) { cpustate->R++; cpustate->_C = cpustate->_HX;                                        } /* LD   C,HX        */
OP(dd,4d) { cpustate->R++; cpustate->_C = cpustate->_LX;                                        } /* LD   C,LX        */
OP(dd,4e) { cpustate->R++; EAX(cpustate); cpustate->_C = RM(cpustate, cpustate->ea);                                } /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(cpustate); op_4f(cpustate);                                   } /* DB   DD          */

OP(dd,50) { illegal_1(cpustate); op_50(cpustate);                                   } /* DB   DD          */
OP(dd,51) { illegal_1(cpustate); op_51(cpustate);                                   } /* DB   DD          */
OP(dd,52) { illegal_1(cpustate); op_52(cpustate);                                   } /* DB   DD          */
OP(dd,53) { illegal_1(cpustate); op_53(cpustate);                                   } /* DB   DD          */
OP(dd,54) { cpustate->R++; cpustate->_D = cpustate->_HX;                                        } /* LD   D,HX        */
OP(dd,55) { cpustate->R++; cpustate->_D = cpustate->_LX;                                        } /* LD   D,LX        */
OP(dd,56) { cpustate->R++; EAX(cpustate); cpustate->_D = RM(cpustate, cpustate->ea);                                } /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(cpustate); op_57(cpustate);                                   } /* DB   DD          */

OP(dd,58) { illegal_1(cpustate); op_58(cpustate);                                   } /* DB   DD          */
OP(dd,59) { illegal_1(cpustate); op_59(cpustate);                                   } /* DB   DD          */
OP(dd,5a) { illegal_1(cpustate); op_5a(cpustate);                                   } /* DB   DD          */
OP(dd,5b) { illegal_1(cpustate); op_5b(cpustate);                                   } /* DB   DD          */
OP(dd,5c) { cpustate->R++; cpustate->_E = cpustate->_HX;                                        } /* LD   E,HX        */
OP(dd,5d) { cpustate->R++; cpustate->_E = cpustate->_LX;                                        } /* LD   E,LX        */
OP(dd,5e) { cpustate->R++; EAX(cpustate); cpustate->_E = RM(cpustate, cpustate->ea);                                } /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(cpustate); op_5f(cpustate);                                   } /* DB   DD          */

OP(dd,60) { cpustate->R++; cpustate->_HX = cpustate->_B;                                        } /* LD   HX,B        */
OP(dd,61) { cpustate->R++; cpustate->_HX = cpustate->_C;                                        } /* LD   HX,C        */
OP(dd,62) { cpustate->R++; cpustate->_HX = cpustate->_D;                                        } /* LD   HX,D        */
OP(dd,63) { cpustate->R++; cpustate->_HX = cpustate->_E;                                        } /* LD   HX,E        */
OP(dd,64) {                                                         } /* LD   HX,HX       */
OP(dd,65) { cpustate->R++; cpustate->_HX = cpustate->_LX;                                       } /* LD   HX,LX       */
OP(dd,66) { cpustate->R++; EAX(cpustate); cpustate->_H = RM(cpustate, cpustate->ea);                                } /* LD   H,(IX+o)    */
OP(dd,67) { cpustate->R++; cpustate->_HX = cpustate->_A;                                        } /* LD   HX,A        */

OP(dd,68) { cpustate->R++; cpustate->_LX = cpustate->_B;                                        } /* LD   LX,B        */
OP(dd,69) { cpustate->R++; cpustate->_LX = cpustate->_C;                                        } /* LD   LX,C        */
OP(dd,6a) { cpustate->R++; cpustate->_LX = cpustate->_D;                                        } /* LD   LX,D        */
OP(dd,6b) { cpustate->R++; cpustate->_LX = cpustate->_E;                                        } /* LD   LX,E        */
OP(dd,6c) { cpustate->R++; cpustate->_LX = cpustate->_HX;                                       } /* LD   LX,HX       */
OP(dd,6d) {                                                         } /* LD   LX,LX       */
OP(dd,6e) { cpustate->R++; EAX(cpustate); cpustate->_L = RM(cpustate, cpustate->ea);                                } /* LD   L,(IX+o)    */
OP(dd,6f) { cpustate->R++; cpustate->_LX = cpustate->_A;                                        } /* LD   LX,A        */

OP(dd,70) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_B );                               } /* LD   (IX+o),B    */
OP(dd,71) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_C );                               } /* LD   (IX+o),C    */
OP(dd,72) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_D );                               } /* LD   (IX+o),D    */
OP(dd,73) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_E );                               } /* LD   (IX+o),E    */
OP(dd,74) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_H );                               } /* LD   (IX+o),H    */
OP(dd,75) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_L );                               } /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(cpustate); op_76(cpustate);                                   }         /* DB   DD          */
OP(dd,77) { cpustate->R++; EAX(cpustate); WM(cpustate,  cpustate->ea, cpustate->_A );                               } /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(cpustate); op_78(cpustate);                                   } /* DB   DD          */
OP(dd,79) { illegal_1(cpustate); op_79(cpustate);                                   } /* DB   DD          */
OP(dd,7a) { illegal_1(cpustate); op_7a(cpustate);                                   } /* DB   DD          */
OP(dd,7b) { illegal_1(cpustate); op_7b(cpustate);                                   } /* DB   DD          */
OP(dd,7c) { cpustate->R++; cpustate->_A = cpustate->_HX;                                        } /* LD   A,HX        */
OP(dd,7d) { cpustate->R++; cpustate->_A = cpustate->_LX;                                        } /* LD   A,LX        */
OP(dd,7e) { cpustate->R++; EAX(cpustate); cpustate->_A = RM(cpustate, cpustate->ea);                                } /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(cpustate); op_7f(cpustate);                                   } /* DB   DD          */

OP(dd,80) { illegal_1(cpustate); op_80(cpustate);                                   } /* DB   DD          */
OP(dd,81) { illegal_1(cpustate); op_81(cpustate);                                   } /* DB   DD          */
OP(dd,82) { illegal_1(cpustate); op_82(cpustate);                                   } /* DB   DD          */
OP(dd,83) { illegal_1(cpustate); op_83(cpustate);                                   } /* DB   DD          */
OP(dd,84) { cpustate->R++; ADD(cpustate->_HX);                                      } /* ADD  A,HX        */
OP(dd,85) { cpustate->R++; ADD(cpustate->_LX);                                      } /* ADD  A,LX        */
OP(dd,86) { cpustate->R++; EAX(cpustate); ADD(RM(cpustate, cpustate->ea));                              } /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(cpustate); op_87(cpustate);                                   } /* DB   DD          */

OP(dd,88) { illegal_1(cpustate); op_88(cpustate);                                   } /* DB   DD          */
OP(dd,89) { illegal_1(cpustate); op_89(cpustate);                                   } /* DB   DD          */
OP(dd,8a) { illegal_1(cpustate); op_8a(cpustate);                                   } /* DB   DD          */
OP(dd,8b) { illegal_1(cpustate); op_8b(cpustate);                                   } /* DB   DD          */
OP(dd,8c) { cpustate->R++; ADC(cpustate->_HX);                                      } /* ADC  A,HX        */
OP(dd,8d) { cpustate->R++; ADC(cpustate->_LX);                                      } /* ADC  A,LX        */
OP(dd,8e) { cpustate->R++; EAX(cpustate); ADC(RM(cpustate, cpustate->ea));                              } /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(cpustate); op_8f(cpustate);                                   } /* DB   DD          */

OP(dd,90) { illegal_1(cpustate); op_90(cpustate);                                   } /* DB   DD          */
OP(dd,91) { illegal_1(cpustate); op_91(cpustate);                                   } /* DB   DD          */
OP(dd,92) { illegal_1(cpustate); op_92(cpustate);                                   } /* DB   DD          */
OP(dd,93) { illegal_1(cpustate); op_93(cpustate);                                   } /* DB   DD          */
OP(dd,94) { cpustate->R++; SUB(cpustate->_HX);                                      } /* SUB  HX          */
OP(dd,95) { cpustate->R++; SUB(cpustate->_LX);                                      } /* SUB  LX          */
OP(dd,96) { cpustate->R++; EAX(cpustate); SUB(RM(cpustate, cpustate->ea));                              } /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(cpustate); op_97(cpustate);                                   } /* DB   DD          */

OP(dd,98) { illegal_1(cpustate); op_98(cpustate);                                   } /* DB   DD          */
OP(dd,99) { illegal_1(cpustate); op_99(cpustate);                                   } /* DB   DD          */
OP(dd,9a) { illegal_1(cpustate); op_9a(cpustate);                                   } /* DB   DD          */
OP(dd,9b) { illegal_1(cpustate); op_9b(cpustate);                                   } /* DB   DD          */
OP(dd,9c) { cpustate->R++; SBC(cpustate->_HX);                                      } /* SBC  A,HX        */
OP(dd,9d) { cpustate->R++; SBC(cpustate->_LX);                                      } /* SBC  A,LX        */
OP(dd,9e) { cpustate->R++; EAX(cpustate); SBC(RM(cpustate, cpustate->ea));                              } /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(cpustate); op_9f(cpustate);                                   } /* DB   DD          */

OP(dd,a0) { illegal_1(cpustate); op_a0(cpustate);                                   } /* DB   DD          */
OP(dd,a1) { illegal_1(cpustate); op_a1(cpustate);                                   } /* DB   DD          */
OP(dd,a2) { illegal_1(cpustate); op_a2(cpustate);                                   } /* DB   DD          */
OP(dd,a3) { illegal_1(cpustate); op_a3(cpustate);                                   } /* DB   DD          */
OP(dd,a4) { cpustate->R++; AND(cpustate->_HX);                                      } /* AND  HX          */
OP(dd,a5) { cpustate->R++; AND(cpustate->_LX);                                      } /* AND  LX          */
OP(dd,a6) { cpustate->R++; EAX(cpustate); AND(RM(cpustate, cpustate->ea));                              } /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(cpustate); op_a7(cpustate);                                   } /* DB   DD          */

OP(dd,a8) { illegal_1(cpustate); op_a8(cpustate);                                   } /* DB   DD          */
OP(dd,a9) { illegal_1(cpustate); op_a9(cpustate);                                   } /* DB   DD          */
OP(dd,aa) { illegal_1(cpustate); op_aa(cpustate);                                   } /* DB   DD          */
OP(dd,ab) { illegal_1(cpustate); op_ab(cpustate);                                   } /* DB   DD          */
OP(dd,ac) { cpustate->R++; XOR(cpustate->_HX);                                      } /* XOR  HX          */
OP(dd,ad) { cpustate->R++; XOR(cpustate->_LX);                                      } /* XOR  LX          */
OP(dd,ae) { cpustate->R++; EAX(cpustate); XOR(RM(cpustate, cpustate->ea));                              } /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(cpustate); op_af(cpustate);                                   } /* DB   DD          */

OP(dd,b0) { illegal_1(cpustate); op_b0(cpustate);                                   } /* DB   DD          */
OP(dd,b1) { illegal_1(cpustate); op_b1(cpustate);                                   } /* DB   DD          */
OP(dd,b2) { illegal_1(cpustate); op_b2(cpustate);                                   } /* DB   DD          */
OP(dd,b3) { illegal_1(cpustate); op_b3(cpustate);                                   } /* DB   DD          */
OP(dd,b4) { cpustate->R++; OR(cpustate->_HX);                                           } /* OR   HX          */
OP(dd,b5) { cpustate->R++; OR(cpustate->_LX);                                           } /* OR   LX          */
OP(dd,b6) { cpustate->R++; EAX(cpustate); OR(RM(cpustate, cpustate->ea));                                   } /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(cpustate); op_b7(cpustate);                                   } /* DB   DD          */

OP(dd,b8) { illegal_1(cpustate); op_b8(cpustate);                                   } /* DB   DD          */
OP(dd,b9) { illegal_1(cpustate); op_b9(cpustate);                                   } /* DB   DD          */
OP(dd,ba) { illegal_1(cpustate); op_ba(cpustate);                                   } /* DB   DD          */
OP(dd,bb) { illegal_1(cpustate); op_bb(cpustate);                                   } /* DB   DD          */
OP(dd,bc) { cpustate->R++; CP(cpustate->_HX);                                           } /* CP   HX          */
OP(dd,bd) { cpustate->R++; CP(cpustate->_LX);                                           } /* CP   LX          */
OP(dd,be) { cpustate->R++; EAX(cpustate); CP(RM(cpustate, cpustate->ea));                                   } /* CP   (IX+o)      */
OP(dd,bf) { illegal_1(cpustate); op_bf(cpustate);                                   } /* DB   DD          */

OP(dd,c0) { illegal_1(cpustate); op_c0(cpustate);                                   } /* DB   DD          */
OP(dd,c1) { illegal_1(cpustate); op_c1(cpustate);                                   } /* DB   DD          */
OP(dd,c2) { illegal_1(cpustate); op_c2(cpustate);                                   } /* DB   DD          */
OP(dd,c3) { illegal_1(cpustate); op_c3(cpustate);                                   } /* DB   DD          */
OP(dd,c4) { illegal_1(cpustate); op_c4(cpustate);                                   } /* DB   DD          */
OP(dd,c5) { illegal_1(cpustate); op_c5(cpustate);                                   } /* DB   DD          */
OP(dd,c6) { illegal_1(cpustate); op_c6(cpustate);                                   } /* DB   DD          */
OP(dd,c7) { illegal_1(cpustate); op_c7(cpustate);                                   }         /* DB   DD          */

OP(dd,c8) { illegal_1(cpustate); op_c8(cpustate);                                   } /* DB   DD          */
OP(dd,c9) { illegal_1(cpustate); op_c9(cpustate);                                   } /* DB   DD          */
OP(dd,ca) { illegal_1(cpustate); op_ca(cpustate);                                   } /* DB   DD          */
OP(dd,cb) { cpustate->R++; EAX(cpustate); cpustate->extra_cycles += exec_xycb(cpustate,ARG(cpustate));                          } /* **   DD CB xx    */
OP(dd,cc) { illegal_1(cpustate); op_cc(cpustate);                                   } /* DB   DD          */
OP(dd,cd) { illegal_1(cpustate); op_cd(cpustate);                                   } /* DB   DD          */
OP(dd,ce) { illegal_1(cpustate); op_ce(cpustate);                                   } /* DB   DD          */
OP(dd,cf) { illegal_1(cpustate); op_cf(cpustate);                                   } /* DB   DD          */

OP(dd,d0) { illegal_1(cpustate); op_d0(cpustate);                                   } /* DB   DD          */
OP(dd,d1) { illegal_1(cpustate); op_d1(cpustate);                                   } /* DB   DD          */
OP(dd,d2) { illegal_1(cpustate); op_d2(cpustate);                                   } /* DB   DD          */
OP(dd,d3) { illegal_1(cpustate); op_d3(cpustate);                                   } /* DB   DD          */
OP(dd,d4) { illegal_1(cpustate); op_d4(cpustate);                                   } /* DB   DD          */
OP(dd,d5) { illegal_1(cpustate); op_d5(cpustate);                                   } /* DB   DD          */
OP(dd,d6) { illegal_1(cpustate); op_d6(cpustate);                                   } /* DB   DD          */
OP(dd,d7) { illegal_1(cpustate); op_d7(cpustate);                                   } /* DB   DD          */

OP(dd,d8) { illegal_1(cpustate); op_d8(cpustate);                                   } /* DB   DD          */
OP(dd,d9) { illegal_1(cpustate); op_d9(cpustate);                                   } /* DB   DD          */
OP(dd,da) { illegal_1(cpustate); op_da(cpustate);                                   } /* DB   DD          */
OP(dd,db) { illegal_1(cpustate); op_db(cpustate);                                   } /* DB   DD          */
OP(dd,dc) { illegal_1(cpustate); op_dc(cpustate);                                   } /* DB   DD          */
OP(dd,dd) { illegal_1(cpustate); op_dd(cpustate);                                   } /* DB   DD          */
OP(dd,de) { illegal_1(cpustate); op_de(cpustate);                                   } /* DB   DD          */
OP(dd,df) { illegal_1(cpustate); op_df(cpustate);                                   } /* DB   DD          */

OP(dd,e0) { illegal_1(cpustate); op_e0(cpustate);                                   } /* DB   DD          */
OP(dd,e1) { cpustate->R++; POP(cpustate, IX);                                           } /* POP  IX          */
OP(dd,e2) { illegal_1(cpustate); op_e2(cpustate);                                   } /* DB   DD          */
OP(dd,e3) { cpustate->R++; EXSP(IX);                                        } /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(cpustate); op_e4(cpustate);                                   } /* DB   DD          */
OP(dd,e5) { cpustate->R++; PUSH(cpustate,  IX );                                        } /* PUSH IX          */
OP(dd,e6) { illegal_1(cpustate); op_e6(cpustate);                                   } /* DB   DD          */
OP(dd,e7) { illegal_1(cpustate); op_e7(cpustate);                                   } /* DB   DD          */

OP(dd,e8) { illegal_1(cpustate); op_e8(cpustate);                                   } /* DB   DD          */
OP(dd,e9) { cpustate->R++; cpustate->_PC = cpustate->_IX;                                       } /* JP   (IX)        */
OP(dd,ea) { illegal_1(cpustate); op_ea(cpustate);                                   } /* DB   DD          */
OP(dd,eb) { illegal_1(cpustate); op_eb(cpustate);                                   } /* DB   DD          */
OP(dd,ec) { illegal_1(cpustate); op_ec(cpustate);                                   } /* DB   DD          */
OP(dd,ed) { illegal_1(cpustate); op_ed(cpustate);                                   } /* DB   DD          */
OP(dd,ee) { illegal_1(cpustate); op_ee(cpustate);                                   } /* DB   DD          */
OP(dd,ef) { illegal_1(cpustate); op_ef(cpustate);                                   } /* DB   DD          */

OP(dd,f0) { illegal_1(cpustate); op_f0(cpustate);                                   } /* DB   DD          */
OP(dd,f1) { illegal_1(cpustate); op_f1(cpustate);                                   } /* DB   DD          */
OP(dd,f2) { illegal_1(cpustate); op_f2(cpustate);                                   } /* DB   DD          */
OP(dd,f3) { illegal_1(cpustate); op_f3(cpustate);                                   } /* DB   DD          */
OP(dd,f4) { illegal_1(cpustate); op_f4(cpustate);                                   } /* DB   DD          */
OP(dd,f5) { illegal_1(cpustate); op_f5(cpustate);                                   } /* DB   DD          */
OP(dd,f6) { illegal_1(cpustate); op_f6(cpustate);                                   } /* DB   DD          */
OP(dd,f7) { illegal_1(cpustate); op_f7(cpustate);                                   } /* DB   DD          */

OP(dd,f8) { illegal_1(cpustate); op_f8(cpustate);                                   } /* DB   DD          */
OP(dd,f9) { cpustate->R++; cpustate->_SP = cpustate->_IX;                                       } /* LD   SP,IX       */
OP(dd,fa) { illegal_1(cpustate); op_fa(cpustate);                                   } /* DB   DD          */
OP(dd,fb) { illegal_1(cpustate); op_fb(cpustate);                                   } /* DB   DD          */
OP(dd,fc) { illegal_1(cpustate); op_fc(cpustate);                                   } /* DB   DD          */
OP(dd,fd) { illegal_1(cpustate); op_fd(cpustate);                                   } /* DB   DD          */
OP(dd,fe) { illegal_1(cpustate); op_fe(cpustate);                                   } /* DB   DD          */
OP(dd,ff) { illegal_1(cpustate); op_ff(cpustate);                                   } /* DB   DD          */
