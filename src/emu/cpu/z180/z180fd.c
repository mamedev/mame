/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP(fd,00) { illegal_1(cpustate); op_00(cpustate);                                   } /* DB   FD          */
OP(fd,01) { illegal_1(cpustate); op_01(cpustate);                                   } /* DB   FD          */
OP(fd,02) { illegal_1(cpustate); op_02(cpustate);                                   } /* DB   FD          */
OP(fd,03) { illegal_1(cpustate); op_03(cpustate);                                   } /* DB   FD          */
OP(fd,04) { illegal_1(cpustate); op_04(cpustate);                                   } /* DB   FD          */
OP(fd,05) { illegal_1(cpustate); op_05(cpustate);                                   } /* DB   FD          */
OP(fd,06) { illegal_1(cpustate); op_06(cpustate);                                   } /* DB   FD          */
OP(fd,07) { illegal_1(cpustate); op_07(cpustate);                                   } /* DB   FD          */

OP(fd,08) { illegal_1(cpustate); op_08(cpustate);                                   } /* DB   FD          */
OP(fd,09) { cpustate->R++; ADD16(IY,BC);                                    } /* ADD  IY,BC       */
OP(fd,0a) { illegal_1(cpustate); op_0a(cpustate);                                   } /* DB   FD          */
OP(fd,0b) { illegal_1(cpustate); op_0b(cpustate);                                   } /* DB   FD          */
OP(fd,0c) { illegal_1(cpustate); op_0c(cpustate);                                   } /* DB   FD          */
OP(fd,0d) { illegal_1(cpustate); op_0d(cpustate);                                   } /* DB   FD          */
OP(fd,0e) { illegal_1(cpustate); op_0e(cpustate);                                   } /* DB   FD          */
OP(fd,0f) { illegal_1(cpustate); op_0f(cpustate);                                   } /* DB   FD          */

OP(fd,10) { illegal_1(cpustate); op_10(cpustate);                                   } /* DB   FD          */
OP(fd,11) { illegal_1(cpustate); op_11(cpustate);                                   } /* DB   FD          */
OP(fd,12) { illegal_1(cpustate); op_12(cpustate);                                   } /* DB   FD          */
OP(fd,13) { illegal_1(cpustate); op_13(cpustate);                                   } /* DB   FD          */
OP(fd,14) { illegal_1(cpustate); op_14(cpustate);                                   } /* DB   FD          */
OP(fd,15) { illegal_1(cpustate); op_15(cpustate);                                   } /* DB   FD          */
OP(fd,16) { illegal_1(cpustate); op_16(cpustate);                                   } /* DB   FD          */
OP(fd,17) { illegal_1(cpustate); op_17(cpustate);                                   } /* DB   FD          */

OP(fd,18) { illegal_1(cpustate); op_18(cpustate);                                   } /* DB   FD          */
OP(fd,19) { cpustate->R++; ADD16(IY,DE);                                    } /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(cpustate); op_1a(cpustate);                                   } /* DB   FD          */
OP(fd,1b) { illegal_1(cpustate); op_1b(cpustate);                                   } /* DB   FD          */
OP(fd,1c) { illegal_1(cpustate); op_1c(cpustate);                                   } /* DB   FD          */
OP(fd,1d) { illegal_1(cpustate); op_1d(cpustate);                                   } /* DB   FD          */
OP(fd,1e) { illegal_1(cpustate); op_1e(cpustate);                                   } /* DB   FD          */
OP(fd,1f) { illegal_1(cpustate); op_1f(cpustate);                                   } /* DB   FD          */

OP(fd,20) { illegal_1(cpustate); op_20(cpustate);                                   } /* DB   FD          */
OP(fd,21) { cpustate->R++; cpustate->_IY = ARG16(cpustate);                                 } /* LD   IY,w        */
OP(fd,22) { cpustate->R++; cpustate->ea = ARG16(cpustate); WM16(cpustate,  cpustate->ea, &cpustate->IY );               } /* LD   (w),IY      */
OP(fd,23) { cpustate->R++; cpustate->_IY++;                                         } /* INC  IY          */
OP(fd,24) { cpustate->R++; cpustate->_HY = INC(cpustate, cpustate->_HY);                                    } /* INC  HY          */
OP(fd,25) { cpustate->R++; cpustate->_HY = DEC(cpustate, cpustate->_HY);                                    } /* DEC  HY          */
OP(fd,26) { cpustate->R++; cpustate->_HY = ARG(cpustate);                                       } /* LD   HY,n        */
OP(fd,27) { illegal_1(cpustate); op_27(cpustate);                                   } /* DB   FD          */

OP(fd,28) { illegal_1(cpustate); op_28(cpustate);                                   } /* DB   FD          */
OP(fd,29) { cpustate->R++; ADD16(IY,IY);                                    } /* ADD  IY,IY       */
OP(fd,2a) { cpustate->R++; cpustate->ea = ARG16(cpustate); RM16(cpustate,  cpustate->ea, &cpustate->IY );               } /* LD   IY,(w)      */
OP(fd,2b) { cpustate->R++; cpustate->_IY--;                                         } /* DEC  IY          */
OP(fd,2c) { cpustate->R++; cpustate->_LY = INC(cpustate, cpustate->_LY);                                    } /* INC  LY          */
OP(fd,2d) { cpustate->R++; cpustate->_LY = DEC(cpustate, cpustate->_LY);                                    } /* DEC  LY          */
OP(fd,2e) { cpustate->R++; cpustate->_LY = ARG(cpustate);                                       } /* LD   LY,n        */
OP(fd,2f) { illegal_1(cpustate); op_2f(cpustate);                                   } /* DB   FD          */

OP(fd,30) { illegal_1(cpustate); op_30(cpustate);                                   } /* DB   FD          */
OP(fd,31) { illegal_1(cpustate); op_31(cpustate);                                   } /* DB   FD          */
OP(fd,32) { illegal_1(cpustate); op_32(cpustate);                                   } /* DB   FD          */
OP(fd,33) { illegal_1(cpustate); op_33(cpustate);                                   } /* DB   FD          */
OP(fd,34) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, INC(cpustate, RM(cpustate, cpustate->ea)) );                      } /* INC  (IY+o)      */
OP(fd,35) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, DEC(cpustate, RM(cpustate, cpustate->ea)) );                      } /* DEC  (IY+o)      */
OP(fd,36) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, ARG(cpustate) );                          } /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(cpustate); op_37(cpustate);                                   } /* DB   FD          */

OP(fd,38) { illegal_1(cpustate); op_38(cpustate);                                   } /* DB   FD          */
OP(fd,39) { cpustate->R++; ADD16(IY,SP);                                    } /* ADD  IY,SP       */
OP(fd,3a) { illegal_1(cpustate); op_3a(cpustate);                                   } /* DB   FD          */
OP(fd,3b) { illegal_1(cpustate); op_3b(cpustate);                                   } /* DB   FD          */
OP(fd,3c) { illegal_1(cpustate); op_3c(cpustate);                                   } /* DB   FD          */
OP(fd,3d) { illegal_1(cpustate); op_3d(cpustate);                                   } /* DB   FD          */
OP(fd,3e) { illegal_1(cpustate); op_3e(cpustate);                                   } /* DB   FD          */
OP(fd,3f) { illegal_1(cpustate); op_3f(cpustate);                                   } /* DB   FD          */

OP(fd,40) { illegal_1(cpustate); op_40(cpustate);                                   } /* DB   FD          */
OP(fd,41) { illegal_1(cpustate); op_41(cpustate);                                   } /* DB   FD          */
OP(fd,42) { illegal_1(cpustate); op_42(cpustate);                                   } /* DB   FD          */
OP(fd,43) { illegal_1(cpustate); op_43(cpustate);                                   } /* DB   FD          */
OP(fd,44) { cpustate->R++; cpustate->_B = cpustate->_HY;                                        } /* LD   B,HY        */
OP(fd,45) { cpustate->R++; cpustate->_B = cpustate->_LY;                                        } /* LD   B,LY        */
OP(fd,46) { cpustate->R++; EAY(cpustate); cpustate->_B = RM(cpustate, cpustate->ea);                                } /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(cpustate); op_47(cpustate);                                   } /* DB   FD          */

OP(fd,48) { illegal_1(cpustate); op_48(cpustate);                                   } /* DB   FD          */
OP(fd,49) { illegal_1(cpustate); op_49(cpustate);                                   } /* DB   FD          */
OP(fd,4a) { illegal_1(cpustate); op_4a(cpustate);                                   } /* DB   FD          */
OP(fd,4b) { illegal_1(cpustate); op_4b(cpustate);                                   } /* DB   FD          */
OP(fd,4c) { cpustate->R++; cpustate->_C = cpustate->_HY;                                        } /* LD   C,HY        */
OP(fd,4d) { cpustate->R++; cpustate->_C = cpustate->_LY;                                        } /* LD   C,LY        */
OP(fd,4e) { cpustate->R++; EAY(cpustate); cpustate->_C = RM(cpustate, cpustate->ea);                                } /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(cpustate); op_4f(cpustate);                                   } /* DB   FD          */

OP(fd,50) { illegal_1(cpustate); op_50(cpustate);                                   } /* DB   FD          */
OP(fd,51) { illegal_1(cpustate); op_51(cpustate);                                   } /* DB   FD          */
OP(fd,52) { illegal_1(cpustate); op_52(cpustate);                                   } /* DB   FD          */
OP(fd,53) { illegal_1(cpustate); op_53(cpustate);                                   } /* DB   FD          */
OP(fd,54) { cpustate->R++; cpustate->_D = cpustate->_HY;                                        } /* LD   D,HY        */
OP(fd,55) { cpustate->R++; cpustate->_D = cpustate->_LY;                                        } /* LD   D,LY        */
OP(fd,56) { cpustate->R++; EAY(cpustate); cpustate->_D = RM(cpustate, cpustate->ea);                                } /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(cpustate); op_57(cpustate);                                   } /* DB   FD          */

OP(fd,58) { illegal_1(cpustate); op_58(cpustate);                                   } /* DB   FD          */
OP(fd,59) { illegal_1(cpustate); op_59(cpustate);                                   } /* DB   FD          */
OP(fd,5a) { illegal_1(cpustate); op_5a(cpustate);                                   } /* DB   FD          */
OP(fd,5b) { illegal_1(cpustate); op_5b(cpustate);                                   } /* DB   FD          */
OP(fd,5c) { cpustate->R++; cpustate->_E = cpustate->_HY;                                        } /* LD   E,HY        */
OP(fd,5d) { cpustate->R++; cpustate->_E = cpustate->_LY;                                        } /* LD   E,LY        */
OP(fd,5e) { cpustate->R++; EAY(cpustate); cpustate->_E = RM(cpustate, cpustate->ea);                                } /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(cpustate); op_5f(cpustate);                                   } /* DB   FD          */

OP(fd,60) { cpustate->R++; cpustate->_HY = cpustate->_B;                                        } /* LD   HY,B        */
OP(fd,61) { cpustate->R++; cpustate->_HY = cpustate->_C;                                        } /* LD   HY,C        */
OP(fd,62) { cpustate->R++; cpustate->_HY = cpustate->_D;                                        } /* LD   HY,D        */
OP(fd,63) { cpustate->R++; cpustate->_HY = cpustate->_E;                                        } /* LD   HY,E        */
OP(fd,64) { cpustate->R++;                                                  } /* LD   HY,HY       */
OP(fd,65) { cpustate->R++; cpustate->_HY = cpustate->_LY;                                       } /* LD   HY,LY       */
OP(fd,66) { cpustate->R++; EAY(cpustate); cpustate->_H = RM(cpustate, cpustate->ea);                                } /* LD   H,(IY+o)    */
OP(fd,67) { cpustate->R++; cpustate->_HY = cpustate->_A;                                        } /* LD   HY,A        */

OP(fd,68) { cpustate->R++; cpustate->_LY = cpustate->_B;                                        } /* LD   LY,B        */
OP(fd,69) { cpustate->R++; cpustate->_LY = cpustate->_C;                                        } /* LD   LY,C        */
OP(fd,6a) { cpustate->R++; cpustate->_LY = cpustate->_D;                                        } /* LD   LY,D        */
OP(fd,6b) { cpustate->R++; cpustate->_LY = cpustate->_E;                                        } /* LD   LY,E        */
OP(fd,6c) { cpustate->R++; cpustate->_LY = cpustate->_HY;                                       } /* LD   LY,HY       */
OP(fd,6d) { cpustate->R++;                                                  } /* LD   LY,LY       */
OP(fd,6e) { cpustate->R++; EAY(cpustate); cpustate->_L = RM(cpustate, cpustate->ea);                                } /* LD   L,(IY+o)    */
OP(fd,6f) { cpustate->R++; cpustate->_LY = cpustate->_A;                                        } /* LD   LY,A        */

OP(fd,70) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_B );                               } /* LD   (IY+o),B    */
OP(fd,71) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_C );                               } /* LD   (IY+o),C    */
OP(fd,72) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_D );                               } /* LD   (IY+o),D    */
OP(fd,73) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_E );                               } /* LD   (IY+o),E    */
OP(fd,74) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_H );                               } /* LD   (IY+o),H    */
OP(fd,75) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_L );                               } /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(cpustate); op_76(cpustate);                                   }         /* DB   FD          */
OP(fd,77) { cpustate->R++; EAY(cpustate); WM(cpustate,  cpustate->ea, cpustate->_A );                               } /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(cpustate); op_78(cpustate);                                   } /* DB   FD          */
OP(fd,79) { illegal_1(cpustate); op_79(cpustate);                                   } /* DB   FD          */
OP(fd,7a) { illegal_1(cpustate); op_7a(cpustate);                                   } /* DB   FD          */
OP(fd,7b) { illegal_1(cpustate); op_7b(cpustate);                                   } /* DB   FD          */
OP(fd,7c) { cpustate->R++; cpustate->_A = cpustate->_HY;                                        } /* LD   A,HY        */
OP(fd,7d) { cpustate->R++; cpustate->_A = cpustate->_LY;                                        } /* LD   A,LY        */
OP(fd,7e) { cpustate->R++; EAY(cpustate); cpustate->_A = RM(cpustate, cpustate->ea);                                } /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(cpustate); op_7f(cpustate);                                   } /* DB   FD          */

OP(fd,80) { illegal_1(cpustate); op_80(cpustate);                                   } /* DB   FD          */
OP(fd,81) { illegal_1(cpustate); op_81(cpustate);                                   } /* DB   FD          */
OP(fd,82) { illegal_1(cpustate); op_82(cpustate);                                   } /* DB   FD          */
OP(fd,83) { illegal_1(cpustate); op_83(cpustate);                                   } /* DB   FD          */
OP(fd,84) { cpustate->R++; ADD(cpustate->_HY);                                      } /* ADD  A,HY        */
OP(fd,85) { cpustate->R++; ADD(cpustate->_LY);                                      } /* ADD  A,LY        */
OP(fd,86) { cpustate->R++; EAY(cpustate); ADD(RM(cpustate, cpustate->ea));                              } /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(cpustate); op_87(cpustate);                                   } /* DB   FD          */

OP(fd,88) { illegal_1(cpustate); op_88(cpustate);                                   } /* DB   FD          */
OP(fd,89) { illegal_1(cpustate); op_89(cpustate);                                   } /* DB   FD          */
OP(fd,8a) { illegal_1(cpustate); op_8a(cpustate);                                   } /* DB   FD          */
OP(fd,8b) { illegal_1(cpustate); op_8b(cpustate);                                   } /* DB   FD          */
OP(fd,8c) { cpustate->R++; ADC(cpustate->_HY);                                      } /* ADC  A,HY        */
OP(fd,8d) { cpustate->R++; ADC(cpustate->_LY);                                      } /* ADC  A,LY        */
OP(fd,8e) { cpustate->R++; EAY(cpustate); ADC(RM(cpustate, cpustate->ea));                              } /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(cpustate); op_8f(cpustate);                                   } /* DB   FD          */

OP(fd,90) { illegal_1(cpustate); op_90(cpustate);                                   } /* DB   FD          */
OP(fd,91) { illegal_1(cpustate); op_91(cpustate);                                   } /* DB   FD          */
OP(fd,92) { illegal_1(cpustate); op_92(cpustate);                                   } /* DB   FD          */
OP(fd,93) { illegal_1(cpustate); op_93(cpustate);                                   } /* DB   FD          */
OP(fd,94) { cpustate->R++; SUB(cpustate->_HY);                                      } /* SUB  HY          */
OP(fd,95) { cpustate->R++; SUB(cpustate->_LY);                                      } /* SUB  LY          */
OP(fd,96) { cpustate->R++; EAY(cpustate); SUB(RM(cpustate, cpustate->ea));                              } /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(cpustate); op_97(cpustate);                                   } /* DB   FD          */

OP(fd,98) { illegal_1(cpustate); op_98(cpustate);                                   } /* DB   FD          */
OP(fd,99) { illegal_1(cpustate); op_99(cpustate);                                   } /* DB   FD          */
OP(fd,9a) { illegal_1(cpustate); op_9a(cpustate);                                   } /* DB   FD          */
OP(fd,9b) { illegal_1(cpustate); op_9b(cpustate);                                   } /* DB   FD          */
OP(fd,9c) { cpustate->R++; SBC(cpustate->_HY);                                      } /* SBC  A,HY        */
OP(fd,9d) { cpustate->R++; SBC(cpustate->_LY);                                      } /* SBC  A,LY        */
OP(fd,9e) { cpustate->R++; EAY(cpustate); SBC(RM(cpustate, cpustate->ea));                              } /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(cpustate); op_9f(cpustate);                                   } /* DB   FD          */

OP(fd,a0) { illegal_1(cpustate); op_a0(cpustate);                                   } /* DB   FD          */
OP(fd,a1) { illegal_1(cpustate); op_a1(cpustate);                                   } /* DB   FD          */
OP(fd,a2) { illegal_1(cpustate); op_a2(cpustate);                                   } /* DB   FD          */
OP(fd,a3) { illegal_1(cpustate); op_a3(cpustate);                                   } /* DB   FD          */
OP(fd,a4) { cpustate->R++; AND(cpustate->_HY);                                      } /* AND  HY          */
OP(fd,a5) { cpustate->R++; AND(cpustate->_LY);                                      } /* AND  LY          */
OP(fd,a6) { cpustate->R++; EAY(cpustate); AND(RM(cpustate, cpustate->ea));                              } /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(cpustate); op_a7(cpustate);                                   } /* DB   FD          */

OP(fd,a8) { illegal_1(cpustate); op_a8(cpustate);                                   } /* DB   FD          */
OP(fd,a9) { illegal_1(cpustate); op_a9(cpustate);                                   } /* DB   FD          */
OP(fd,aa) { illegal_1(cpustate); op_aa(cpustate);                                   } /* DB   FD          */
OP(fd,ab) { illegal_1(cpustate); op_ab(cpustate);                                   } /* DB   FD          */
OP(fd,ac) { cpustate->R++; XOR(cpustate->_HY);                                      } /* XOR  HY          */
OP(fd,ad) { cpustate->R++; XOR(cpustate->_LY);                                      } /* XOR  LY          */
OP(fd,ae) { cpustate->R++; EAY(cpustate); XOR(RM(cpustate, cpustate->ea));                              } /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(cpustate); op_af(cpustate);                                   } /* DB   FD          */

OP(fd,b0) { illegal_1(cpustate); op_b0(cpustate);                                   } /* DB   FD          */
OP(fd,b1) { illegal_1(cpustate); op_b1(cpustate);                                   } /* DB   FD          */
OP(fd,b2) { illegal_1(cpustate); op_b2(cpustate);                                   } /* DB   FD          */
OP(fd,b3) { illegal_1(cpustate); op_b3(cpustate);                                   } /* DB   FD          */
OP(fd,b4) { cpustate->R++; OR(cpustate->_HY);                                           } /* OR   HY          */
OP(fd,b5) { cpustate->R++; OR(cpustate->_LY);                                           } /* OR   LY          */
OP(fd,b6) { cpustate->R++; EAY(cpustate); OR(RM(cpustate, cpustate->ea));                                   } /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(cpustate); op_b7(cpustate);                                   } /* DB   FD          */

OP(fd,b8) { illegal_1(cpustate); op_b8(cpustate);                                   } /* DB   FD          */
OP(fd,b9) { illegal_1(cpustate); op_b9(cpustate);                                   } /* DB   FD          */
OP(fd,ba) { illegal_1(cpustate); op_ba(cpustate);                                   } /* DB   FD          */
OP(fd,bb) { illegal_1(cpustate); op_bb(cpustate);                                   } /* DB   FD          */
OP(fd,bc) { cpustate->R++; CP(cpustate->_HY);                                           } /* CP   HY          */
OP(fd,bd) { cpustate->R++; CP(cpustate->_LY);                                           } /* CP   LY          */
OP(fd,be) { cpustate->R++; EAY(cpustate); CP(RM(cpustate, cpustate->ea));                                   } /* CP   (IY+o)      */
OP(fd,bf) { illegal_1(cpustate); op_bf(cpustate);                                   } /* DB   FD          */

OP(fd,c0) { illegal_1(cpustate); op_c0(cpustate);                                   } /* DB   FD          */
OP(fd,c1) { illegal_1(cpustate); op_c1(cpustate);                                   } /* DB   FD          */
OP(fd,c2) { illegal_1(cpustate); op_c2(cpustate);                                   } /* DB   FD          */
OP(fd,c3) { illegal_1(cpustate); op_c3(cpustate);                                   } /* DB   FD          */
OP(fd,c4) { illegal_1(cpustate); op_c4(cpustate);                                   } /* DB   FD          */
OP(fd,c5) { illegal_1(cpustate); op_c5(cpustate);                                   } /* DB   FD          */
OP(fd,c6) { illegal_1(cpustate); op_c6(cpustate);                                   } /* DB   FD          */
OP(fd,c7) { illegal_1(cpustate); op_c7(cpustate);                                   } /* DB   FD          */

OP(fd,c8) { illegal_1(cpustate); op_c8(cpustate);                                   } /* DB   FD          */
OP(fd,c9) { illegal_1(cpustate); op_c9(cpustate);                                   } /* DB   FD          */
OP(fd,ca) { illegal_1(cpustate); op_ca(cpustate);                                   } /* DB   FD          */
OP(fd,cb) { cpustate->R++; EAY(cpustate); cpustate->extra_cycles += exec_xycb(cpustate,ARG(cpustate));                          } /* **   FD CB xx    */
OP(fd,cc) { illegal_1(cpustate); op_cc(cpustate);                                   } /* DB   FD          */
OP(fd,cd) { illegal_1(cpustate); op_cd(cpustate);                                   } /* DB   FD          */
OP(fd,ce) { illegal_1(cpustate); op_ce(cpustate);                                   } /* DB   FD          */
OP(fd,cf) { illegal_1(cpustate); op_cf(cpustate);                                   } /* DB   FD          */

OP(fd,d0) { illegal_1(cpustate); op_d0(cpustate);                                   } /* DB   FD          */
OP(fd,d1) { illegal_1(cpustate); op_d1(cpustate);                                   } /* DB   FD          */
OP(fd,d2) { illegal_1(cpustate); op_d2(cpustate);                                   } /* DB   FD          */
OP(fd,d3) { illegal_1(cpustate); op_d3(cpustate);                                   } /* DB   FD          */
OP(fd,d4) { illegal_1(cpustate); op_d4(cpustate);                                   } /* DB   FD          */
OP(fd,d5) { illegal_1(cpustate); op_d5(cpustate);                                   } /* DB   FD          */
OP(fd,d6) { illegal_1(cpustate); op_d6(cpustate);                                   } /* DB   FD          */
OP(fd,d7) { illegal_1(cpustate); op_d7(cpustate);                                   } /* DB   FD          */

OP(fd,d8) { illegal_1(cpustate); op_d8(cpustate);                                   } /* DB   FD          */
OP(fd,d9) { illegal_1(cpustate); op_d9(cpustate);                                   } /* DB   FD          */
OP(fd,da) { illegal_1(cpustate); op_da(cpustate);                                   } /* DB   FD          */
OP(fd,db) { illegal_1(cpustate); op_db(cpustate);                                   } /* DB   FD          */
OP(fd,dc) { illegal_1(cpustate); op_dc(cpustate);                                   } /* DB   FD          */
OP(fd,dd) { illegal_1(cpustate); op_dd(cpustate);                                   } /* DB   FD          */
OP(fd,de) { illegal_1(cpustate); op_de(cpustate);                                   } /* DB   FD          */
OP(fd,df) { illegal_1(cpustate); op_df(cpustate);                                   } /* DB   FD          */

OP(fd,e0) { illegal_1(cpustate); op_e0(cpustate);                                   } /* DB   FD          */
OP(fd,e1) { cpustate->R++; POP(cpustate, IY);                                           } /* POP  IY          */
OP(fd,e2) { illegal_1(cpustate); op_e2(cpustate);                                   } /* DB   FD          */
OP(fd,e3) { cpustate->R++; EXSP(IY);                                        } /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(cpustate); op_e4(cpustate);                                   } /* DB   FD          */
OP(fd,e5) { cpustate->R++; PUSH(cpustate,  IY );                                        } /* PUSH IY          */
OP(fd,e6) { illegal_1(cpustate); op_e6(cpustate);                                   } /* DB   FD          */
OP(fd,e7) { illegal_1(cpustate); op_e7(cpustate);                                   } /* DB   FD          */

OP(fd,e8) { illegal_1(cpustate); op_e8(cpustate);                                   } /* DB   FD          */
OP(fd,e9) { cpustate->R++; cpustate->_PC = cpustate->_IY;                                       } /* JP   (IY)        */
OP(fd,ea) { illegal_1(cpustate); op_ea(cpustate);                                   } /* DB   FD          */
OP(fd,eb) { illegal_1(cpustate); op_eb(cpustate);                                   } /* DB   FD          */
OP(fd,ec) { illegal_1(cpustate); op_ec(cpustate);                                   } /* DB   FD          */
OP(fd,ed) { illegal_1(cpustate); op_ed(cpustate);                                   } /* DB   FD          */
OP(fd,ee) { illegal_1(cpustate); op_ee(cpustate);                                   } /* DB   FD          */
OP(fd,ef) { illegal_1(cpustate); op_ef(cpustate);                                   } /* DB   FD          */

OP(fd,f0) { illegal_1(cpustate); op_f0(cpustate);                                   } /* DB   FD          */
OP(fd,f1) { illegal_1(cpustate); op_f1(cpustate);                                   } /* DB   FD          */
OP(fd,f2) { illegal_1(cpustate); op_f2(cpustate);                                   } /* DB   FD          */
OP(fd,f3) { illegal_1(cpustate); op_f3(cpustate);                                   } /* DB   FD          */
OP(fd,f4) { illegal_1(cpustate); op_f4(cpustate);                                   } /* DB   FD          */
OP(fd,f5) { illegal_1(cpustate); op_f5(cpustate);                                   } /* DB   FD          */
OP(fd,f6) { illegal_1(cpustate); op_f6(cpustate);                                   } /* DB   FD          */
OP(fd,f7) { illegal_1(cpustate); op_f7(cpustate);                                   } /* DB   FD          */

OP(fd,f8) { illegal_1(cpustate); op_f8(cpustate);                                   } /* DB   FD          */
OP(fd,f9) { cpustate->R++; cpustate->_SP = cpustate->_IY;                                       } /* LD   SP,IY       */
OP(fd,fa) { illegal_1(cpustate); op_fa(cpustate);                                   } /* DB   FD          */
OP(fd,fb) { illegal_1(cpustate); op_fb(cpustate);                                   } /* DB   FD          */
OP(fd,fc) { illegal_1(cpustate); op_fc(cpustate);                                   } /* DB   FD          */
OP(fd,fd) { illegal_1(cpustate); op_fd(cpustate);                                   } /* DB   FD          */
OP(fd,fe) { illegal_1(cpustate); op_fe(cpustate);                                   } /* DB   FD          */
OP(fd,ff) { illegal_1(cpustate); op_ff(cpustate);                                   } /* DB   FD          */
