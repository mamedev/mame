/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { cpustate->_B = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* RLC  B=(XY+o)    */
OP(xycb,01) { cpustate->_C = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* RLC  C=(XY+o)    */
OP(xycb,02) { cpustate->_D = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* RLC  D=(XY+o)    */
OP(xycb,03) { cpustate->_E = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* RLC  E=(XY+o)    */
OP(xycb,04) { cpustate->_H = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* RLC  H=(XY+o)    */
OP(xycb,05) { cpustate->_L = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* RLC  L=(XY+o)    */
OP(xycb,06) { WM(cpustate,  cpustate->ea, RLC( cpustate, RM(cpustate, cpustate->ea) ) );                                } /* RLC  (XY+o)      */
OP(xycb,07) { cpustate->_A = RLC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* RLC  A=(XY+o)    */

OP(xycb,08) { cpustate->_B = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* RRC  B=(XY+o)    */
OP(xycb,09) { cpustate->_C = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* RRC  C=(XY+o)    */
OP(xycb,0a) { cpustate->_D = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* RRC  D=(XY+o)    */
OP(xycb,0b) { cpustate->_E = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* RRC  E=(XY+o)    */
OP(xycb,0c) { cpustate->_H = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* RRC  H=(XY+o)    */
OP(xycb,0d) { cpustate->_L = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM(cpustate,  cpustate->ea,RRC( cpustate, RM(cpustate, cpustate->ea) ) );                             } /* RRC  (XY+o)      */
OP(xycb,0f) { cpustate->_A = RRC( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* RRC  A=(XY+o)    */

OP(xycb,10) { cpustate->_B = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                     } /* RL   B=(XY+o)    */
OP(xycb,11) { cpustate->_C = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                     } /* RL   C=(XY+o)    */
OP(xycb,12) { cpustate->_D = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                     } /* RL   D=(XY+o)    */
OP(xycb,13) { cpustate->_E = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                     } /* RL   E=(XY+o)    */
OP(xycb,14) { cpustate->_H = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                     } /* RL   H=(XY+o)    */
OP(xycb,15) { cpustate->_L = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                     } /* RL   L=(XY+o)    */
OP(xycb,16) { WM(cpustate,  cpustate->ea,RL( cpustate, RM(cpustate, cpustate->ea) ) );                              } /* RL   (XY+o)      */
OP(xycb,17) { cpustate->_A = RL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                     } /* RL   A=(XY+o)    */

OP(xycb,18) { cpustate->_B = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                     } /* RR   B=(XY+o)    */
OP(xycb,19) { cpustate->_C = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                     } /* RR   C=(XY+o)    */
OP(xycb,1a) { cpustate->_D = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                     } /* RR   D=(XY+o)    */
OP(xycb,1b) { cpustate->_E = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                     } /* RR   E=(XY+o)    */
OP(xycb,1c) { cpustate->_H = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                     } /* RR   H=(XY+o)    */
OP(xycb,1d) { cpustate->_L = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                     } /* RR   L=(XY+o)    */
OP(xycb,1e) { WM(cpustate,  cpustate->ea,RR( cpustate, RM(cpustate, cpustate->ea) ) );                              } /* RR   (XY+o)      */
OP(xycb,1f) { cpustate->_A = RR( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                     } /* RR   A=(XY+o)    */

OP(xycb,20) { cpustate->_B = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* SLA  B=(XY+o)    */
OP(xycb,21) { cpustate->_C = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* SLA  C=(XY+o)    */
OP(xycb,22) { cpustate->_D = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* SLA  D=(XY+o)    */
OP(xycb,23) { cpustate->_E = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* SLA  E=(XY+o)    */
OP(xycb,24) { cpustate->_H = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* SLA  H=(XY+o)    */
OP(xycb,25) { cpustate->_L = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* SLA  L=(XY+o)    */
OP(xycb,26) { WM(cpustate,  cpustate->ea,SLA( cpustate, RM(cpustate, cpustate->ea) ) );                             } /* SLA  (XY+o)      */
OP(xycb,27) { cpustate->_A = SLA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* SLA  A=(XY+o)    */

OP(xycb,28) { cpustate->_B = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* SRA  B=(XY+o)    */
OP(xycb,29) { cpustate->_C = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* SRA  C=(XY+o)    */
OP(xycb,2a) { cpustate->_D = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* SRA  D=(XY+o)    */
OP(xycb,2b) { cpustate->_E = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* SRA  E=(XY+o)    */
OP(xycb,2c) { cpustate->_H = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* SRA  H=(XY+o)    */
OP(xycb,2d) { cpustate->_L = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM(cpustate,  cpustate->ea,SRA( cpustate, RM(cpustate, cpustate->ea) ) );                             } /* SRA  (XY+o)      */
OP(xycb,2f) { cpustate->_A = SRA( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* SRA  A=(XY+o)    */

OP(xycb,30) { cpustate->_B = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* SLL  B=(XY+o)    */
OP(xycb,31) { cpustate->_C = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* SLL  C=(XY+o)    */
OP(xycb,32) { cpustate->_D = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* SLL  D=(XY+o)    */
OP(xycb,33) { cpustate->_E = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* SLL  E=(XY+o)    */
OP(xycb,34) { cpustate->_H = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* SLL  H=(XY+o)    */
OP(xycb,35) { cpustate->_L = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* SLL  L=(XY+o)    */
OP(xycb,36) { WM(cpustate,  cpustate->ea,SLL( cpustate, RM(cpustate, cpustate->ea) ) );                             } /* SLL  (XY+o)      */
OP(xycb,37) { cpustate->_A = SLL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* SLL  A=(XY+o)    */

OP(xycb,38) { cpustate->_B = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                        } /* SRL  B=(XY+o)    */
OP(xycb,39) { cpustate->_C = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                        } /* SRL  C=(XY+o)    */
OP(xycb,3a) { cpustate->_D = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                        } /* SRL  D=(XY+o)    */
OP(xycb,3b) { cpustate->_E = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                        } /* SRL  E=(XY+o)    */
OP(xycb,3c) { cpustate->_H = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                        } /* SRL  H=(XY+o)    */
OP(xycb,3d) { cpustate->_L = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                        } /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM(cpustate,  cpustate->ea,SRL( cpustate, RM(cpustate, cpustate->ea) ) );                             } /* SRL  (XY+o)      */
OP(xycb,3f) { cpustate->_A = SRL( cpustate, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                        } /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46(cpustate);                                            } /* BIT  0,B=(XY+o)  */
OP(xycb,41) { xycb_46(cpustate);                                                      } /* BIT  0,C=(XY+o)  */
OP(xycb,42) { xycb_46(cpustate);                                            } /* BIT  0,D=(XY+o)  */
OP(xycb,43) { xycb_46(cpustate);                                            } /* BIT  0,E=(XY+o)  */
OP(xycb,44) { xycb_46(cpustate);                                            } /* BIT  0,H=(XY+o)  */
OP(xycb,45) { xycb_46(cpustate);                                            } /* BIT  0,L=(XY+o)  */
OP(xycb,46) { BIT_XY(0,RM(cpustate, cpustate->ea));                                     } /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46(cpustate);                                            } /* BIT  0,A=(XY+o)  */

OP(xycb,48) { xycb_4e(cpustate);                                            } /* BIT  1,B=(XY+o)  */
OP(xycb,49) { xycb_4e(cpustate);                                                      } /* BIT  1,C=(XY+o)  */
OP(xycb,4a) { xycb_4e(cpustate);                                            } /* BIT  1,D=(XY+o)  */
OP(xycb,4b) { xycb_4e(cpustate);                                            } /* BIT  1,E=(XY+o)  */
OP(xycb,4c) { xycb_4e(cpustate);                                            } /* BIT  1,H=(XY+o)  */
OP(xycb,4d) { xycb_4e(cpustate);                                            } /* BIT  1,L=(XY+o)  */
OP(xycb,4e) { BIT_XY(1,RM(cpustate, cpustate->ea));                                     } /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e(cpustate);                                            } /* BIT  1,A=(XY+o)  */

OP(xycb,50) { xycb_56(cpustate);                                            } /* BIT  2,B=(XY+o)  */
OP(xycb,51) { xycb_56(cpustate);                                                      } /* BIT  2,C=(XY+o)  */
OP(xycb,52) { xycb_56(cpustate);                                            } /* BIT  2,D=(XY+o)  */
OP(xycb,53) { xycb_56(cpustate);                                            } /* BIT  2,E=(XY+o)  */
OP(xycb,54) { xycb_56(cpustate);                                            } /* BIT  2,H=(XY+o)  */
OP(xycb,55) { xycb_56(cpustate);                                            } /* BIT  2,L=(XY+o)  */
OP(xycb,56) { BIT_XY(2,RM(cpustate, cpustate->ea));                                     } /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56(cpustate);                                            } /* BIT  2,A=(XY+o)  */

OP(xycb,58) { xycb_5e(cpustate);                                            } /* BIT  3,B=(XY+o)  */
OP(xycb,59) { xycb_5e(cpustate);                                                      } /* BIT  3,C=(XY+o)  */
OP(xycb,5a) { xycb_5e(cpustate);                                            } /* BIT  3,D=(XY+o)  */
OP(xycb,5b) { xycb_5e(cpustate);                                            } /* BIT  3,E=(XY+o)  */
OP(xycb,5c) { xycb_5e(cpustate);                                            } /* BIT  3,H=(XY+o)  */
OP(xycb,5d) { xycb_5e(cpustate);                                            } /* BIT  3,L=(XY+o)  */
OP(xycb,5e) { BIT_XY(3,RM(cpustate, cpustate->ea));                                     } /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e(cpustate);                                            } /* BIT  3,A=(XY+o)  */

OP(xycb,60) { xycb_66(cpustate);                                            } /* BIT  4,B=(XY+o)  */
OP(xycb,61) { xycb_66(cpustate);                                                      } /* BIT  4,C=(XY+o)  */
OP(xycb,62) { xycb_66(cpustate);                                            } /* BIT  4,D=(XY+o)  */
OP(xycb,63) { xycb_66(cpustate);                                            } /* BIT  4,E=(XY+o)  */
OP(xycb,64) { xycb_66(cpustate);                                            } /* BIT  4,H=(XY+o)  */
OP(xycb,65) { xycb_66(cpustate);                                            } /* BIT  4,L=(XY+o)  */
OP(xycb,66) { BIT_XY(4,RM(cpustate, cpustate->ea));                                     } /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66(cpustate);                                            } /* BIT  4,A=(XY+o)  */

OP(xycb,68) { xycb_6e(cpustate);                                            } /* BIT  5,B=(XY+o)  */
OP(xycb,69) { xycb_6e(cpustate);                                                      } /* BIT  5,C=(XY+o)  */
OP(xycb,6a) { xycb_6e(cpustate);                                            } /* BIT  5,D=(XY+o)  */
OP(xycb,6b) { xycb_6e(cpustate);                                            } /* BIT  5,E=(XY+o)  */
OP(xycb,6c) { xycb_6e(cpustate);                                            } /* BIT  5,H=(XY+o)  */
OP(xycb,6d) { xycb_6e(cpustate);                                            } /* BIT  5,L=(XY+o)  */
OP(xycb,6e) { BIT_XY(5,RM(cpustate, cpustate->ea));                                     } /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e(cpustate);                                            } /* BIT  5,A=(XY+o)  */

OP(xycb,70) { xycb_76(cpustate);                                            } /* BIT  6,B=(XY+o)  */
OP(xycb,71) { xycb_76(cpustate);                                                      } /* BIT  6,C=(XY+o)  */
OP(xycb,72) { xycb_76(cpustate);                                            } /* BIT  6,D=(XY+o)  */
OP(xycb,73) { xycb_76(cpustate);                                            } /* BIT  6,E=(XY+o)  */
OP(xycb,74) { xycb_76(cpustate);                                            } /* BIT  6,H=(XY+o)  */
OP(xycb,75) { xycb_76(cpustate);                                            } /* BIT  6,L=(XY+o)  */
OP(xycb,76) { BIT_XY(6,RM(cpustate, cpustate->ea));                                     } /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76(cpustate);                                            } /* BIT  6,A=(XY+o)  */

OP(xycb,78) { xycb_7e(cpustate);                                            } /* BIT  7,B=(XY+o)  */
OP(xycb,79) { xycb_7e(cpustate);                                                      } /* BIT  7,C=(XY+o)  */
OP(xycb,7a) { xycb_7e(cpustate);                                            } /* BIT  7,D=(XY+o)  */
OP(xycb,7b) { xycb_7e(cpustate);                                            } /* BIT  7,E=(XY+o)  */
OP(xycb,7c) { xycb_7e(cpustate);                                            } /* BIT  7,H=(XY+o)  */
OP(xycb,7d) { xycb_7e(cpustate);                                            } /* BIT  7,L=(XY+o)  */
OP(xycb,7e) { BIT_XY(7,RM(cpustate, cpustate->ea));                                     } /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e(cpustate);                                            } /* BIT  7,A=(XY+o)  */

OP(xycb,80) { cpustate->_B = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  0,B=(XY+o)  */
OP(xycb,81) { cpustate->_C = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  0,C=(XY+o)  */
OP(xycb,82) { cpustate->_D = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  0,D=(XY+o)  */
OP(xycb,83) { cpustate->_E = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  0,E=(XY+o)  */
OP(xycb,84) { cpustate->_H = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  0,H=(XY+o)  */
OP(xycb,85) { cpustate->_L = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM(cpustate,  cpustate->ea, RES(0,RM(cpustate, cpustate->ea)) );                              } /* RES  0,(XY+o)    */
OP(xycb,87) { cpustate->_A = RES(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  0,A=(XY+o)  */

OP(xycb,88) { cpustate->_B = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  1,B=(XY+o)  */
OP(xycb,89) { cpustate->_C = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  1,C=(XY+o)  */
OP(xycb,8a) { cpustate->_D = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  1,D=(XY+o)  */
OP(xycb,8b) { cpustate->_E = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  1,E=(XY+o)  */
OP(xycb,8c) { cpustate->_H = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  1,H=(XY+o)  */
OP(xycb,8d) { cpustate->_L = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM(cpustate,  cpustate->ea, RES(1,RM(cpustate, cpustate->ea)) );                              } /* RES  1,(XY+o)    */
OP(xycb,8f) { cpustate->_A = RES(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  1,A=(XY+o)  */

OP(xycb,90) { cpustate->_B = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  2,B=(XY+o)  */
OP(xycb,91) { cpustate->_C = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  2,C=(XY+o)  */
OP(xycb,92) { cpustate->_D = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  2,D=(XY+o)  */
OP(xycb,93) { cpustate->_E = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  2,E=(XY+o)  */
OP(xycb,94) { cpustate->_H = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  2,H=(XY+o)  */
OP(xycb,95) { cpustate->_L = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM(cpustate,  cpustate->ea, RES(2,RM(cpustate, cpustate->ea)) );                              } /* RES  2,(XY+o)    */
OP(xycb,97) { cpustate->_A = RES(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  2,A=(XY+o)  */

OP(xycb,98) { cpustate->_B = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  3,B=(XY+o)  */
OP(xycb,99) { cpustate->_C = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  3,C=(XY+o)  */
OP(xycb,9a) { cpustate->_D = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  3,D=(XY+o)  */
OP(xycb,9b) { cpustate->_E = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  3,E=(XY+o)  */
OP(xycb,9c) { cpustate->_H = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  3,H=(XY+o)  */
OP(xycb,9d) { cpustate->_L = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM(cpustate,  cpustate->ea, RES(3,RM(cpustate, cpustate->ea)) );                              } /* RES  3,(XY+o)    */
OP(xycb,9f) { cpustate->_A = RES(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  3,A=(XY+o)  */

OP(xycb,a0) { cpustate->_B = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  4,B=(XY+o)  */
OP(xycb,a1) { cpustate->_C = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  4,C=(XY+o)  */
OP(xycb,a2) { cpustate->_D = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  4,D=(XY+o)  */
OP(xycb,a3) { cpustate->_E = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  4,E=(XY+o)  */
OP(xycb,a4) { cpustate->_H = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  4,H=(XY+o)  */
OP(xycb,a5) { cpustate->_L = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM(cpustate,  cpustate->ea, RES(4,RM(cpustate, cpustate->ea)) );                              } /* RES  4,(XY+o)    */
OP(xycb,a7) { cpustate->_A = RES(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  4,A=(XY+o)  */

OP(xycb,a8) { cpustate->_B = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  5,B=(XY+o)  */
OP(xycb,a9) { cpustate->_C = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  5,C=(XY+o)  */
OP(xycb,aa) { cpustate->_D = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  5,D=(XY+o)  */
OP(xycb,ab) { cpustate->_E = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  5,E=(XY+o)  */
OP(xycb,ac) { cpustate->_H = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  5,H=(XY+o)  */
OP(xycb,ad) { cpustate->_L = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM(cpustate,  cpustate->ea, RES(5,RM(cpustate, cpustate->ea)) );                              } /* RES  5,(XY+o)    */
OP(xycb,af) { cpustate->_A = RES(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  5,A=(XY+o)  */

OP(xycb,b0) { cpustate->_B = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  6,B=(XY+o)  */
OP(xycb,b1) { cpustate->_C = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  6,C=(XY+o)  */
OP(xycb,b2) { cpustate->_D = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  6,D=(XY+o)  */
OP(xycb,b3) { cpustate->_E = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  6,E=(XY+o)  */
OP(xycb,b4) { cpustate->_H = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  6,H=(XY+o)  */
OP(xycb,b5) { cpustate->_L = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM(cpustate,  cpustate->ea, RES(6,RM(cpustate, cpustate->ea)) );                              } /* RES  6,(XY+o)    */
OP(xycb,b7) { cpustate->_A = RES(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  6,A=(XY+o)  */

OP(xycb,b8) { cpustate->_B = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* RES  7,B=(XY+o)  */
OP(xycb,b9) { cpustate->_C = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* RES  7,C=(XY+o)  */
OP(xycb,ba) { cpustate->_D = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* RES  7,D=(XY+o)  */
OP(xycb,bb) { cpustate->_E = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* RES  7,E=(XY+o)  */
OP(xycb,bc) { cpustate->_H = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* RES  7,H=(XY+o)  */
OP(xycb,bd) { cpustate->_L = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM(cpustate,  cpustate->ea, RES(7,RM(cpustate, cpustate->ea)) );                              } /* RES  7,(XY+o)    */
OP(xycb,bf) { cpustate->_A = RES(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* RES  7,A=(XY+o)  */

OP(xycb,c0) { cpustate->_B = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  0,B=(XY+o)  */
OP(xycb,c1) { cpustate->_C = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  0,C=(XY+o)  */
OP(xycb,c2) { cpustate->_D = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  0,D=(XY+o)  */
OP(xycb,c3) { cpustate->_E = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  0,E=(XY+o)  */
OP(xycb,c4) { cpustate->_H = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  0,H=(XY+o)  */
OP(xycb,c5) { cpustate->_L = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM(cpustate,  cpustate->ea, SET(0,RM(cpustate, cpustate->ea)) );                              } /* SET  0,(XY+o)    */
OP(xycb,c7) { cpustate->_A = SET(0, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  0,A=(XY+o)  */

OP(xycb,c8) { cpustate->_B = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  1,B=(XY+o)  */
OP(xycb,c9) { cpustate->_C = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  1,C=(XY+o)  */
OP(xycb,ca) { cpustate->_D = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  1,D=(XY+o)  */
OP(xycb,cb) { cpustate->_E = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  1,E=(XY+o)  */
OP(xycb,cc) { cpustate->_H = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  1,H=(XY+o)  */
OP(xycb,cd) { cpustate->_L = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM(cpustate,  cpustate->ea, SET(1,RM(cpustate, cpustate->ea)) );                              } /* SET  1,(XY+o)    */
OP(xycb,cf) { cpustate->_A = SET(1, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  1,A=(XY+o)  */

OP(xycb,d0) { cpustate->_B = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  2,B=(XY+o)  */
OP(xycb,d1) { cpustate->_C = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  2,C=(XY+o)  */
OP(xycb,d2) { cpustate->_D = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  2,D=(XY+o)  */
OP(xycb,d3) { cpustate->_E = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  2,E=(XY+o)  */
OP(xycb,d4) { cpustate->_H = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  2,H=(XY+o)  */
OP(xycb,d5) { cpustate->_L = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM(cpustate,  cpustate->ea, SET(2,RM(cpustate, cpustate->ea)) );                              } /* SET  2,(XY+o)    */
OP(xycb,d7) { cpustate->_A = SET(2, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  2,A=(XY+o)  */

OP(xycb,d8) { cpustate->_B = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  3,B=(XY+o)  */
OP(xycb,d9) { cpustate->_C = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  3,C=(XY+o)  */
OP(xycb,da) { cpustate->_D = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  3,D=(XY+o)  */
OP(xycb,db) { cpustate->_E = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  3,E=(XY+o)  */
OP(xycb,dc) { cpustate->_H = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  3,H=(XY+o)  */
OP(xycb,dd) { cpustate->_L = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM(cpustate,  cpustate->ea, SET(3,RM(cpustate, cpustate->ea)) );                              } /* SET  3,(XY+o)    */
OP(xycb,df) { cpustate->_A = SET(3, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  3,A=(XY+o)  */

OP(xycb,e0) { cpustate->_B = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  4,B=(XY+o)  */
OP(xycb,e1) { cpustate->_C = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  4,C=(XY+o)  */
OP(xycb,e2) { cpustate->_D = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  4,D=(XY+o)  */
OP(xycb,e3) { cpustate->_E = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  4,E=(XY+o)  */
OP(xycb,e4) { cpustate->_H = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  4,H=(XY+o)  */
OP(xycb,e5) { cpustate->_L = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM(cpustate,  cpustate->ea, SET(4,RM(cpustate, cpustate->ea)) );                              } /* SET  4,(XY+o)    */
OP(xycb,e7) { cpustate->_A = SET(4, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  4,A=(XY+o)  */

OP(xycb,e8) { cpustate->_B = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  5,B=(XY+o)  */
OP(xycb,e9) { cpustate->_C = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  5,C=(XY+o)  */
OP(xycb,ea) { cpustate->_D = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  5,D=(XY+o)  */
OP(xycb,eb) { cpustate->_E = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  5,E=(XY+o)  */
OP(xycb,ec) { cpustate->_H = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  5,H=(XY+o)  */
OP(xycb,ed) { cpustate->_L = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM(cpustate,  cpustate->ea, SET(5,RM(cpustate, cpustate->ea)) );                              } /* SET  5,(XY+o)    */
OP(xycb,ef) { cpustate->_A = SET(5, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  5,A=(XY+o)  */

OP(xycb,f0) { cpustate->_B = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  6,B=(XY+o)  */
OP(xycb,f1) { cpustate->_C = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  6,C=(XY+o)  */
OP(xycb,f2) { cpustate->_D = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  6,D=(XY+o)  */
OP(xycb,f3) { cpustate->_E = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  6,E=(XY+o)  */
OP(xycb,f4) { cpustate->_H = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  6,H=(XY+o)  */
OP(xycb,f5) { cpustate->_L = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM(cpustate,  cpustate->ea, SET(6,RM(cpustate, cpustate->ea)) );                              } /* SET  6,(XY+o)    */
OP(xycb,f7) { cpustate->_A = SET(6, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  6,A=(XY+o)  */

OP(xycb,f8) { cpustate->_B = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_B );                    } /* SET  7,B=(XY+o)  */
OP(xycb,f9) { cpustate->_C = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_C );                    } /* SET  7,C=(XY+o)  */
OP(xycb,fa) { cpustate->_D = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_D );                    } /* SET  7,D=(XY+o)  */
OP(xycb,fb) { cpustate->_E = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_E );                    } /* SET  7,E=(XY+o)  */
OP(xycb,fc) { cpustate->_H = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_H );                    } /* SET  7,H=(XY+o)  */
OP(xycb,fd) { cpustate->_L = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_L );                    } /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM(cpustate,  cpustate->ea, SET(7,RM(cpustate, cpustate->ea)) );                              } /* SET  7,(XY+o)    */
OP(xycb,ff) { cpustate->_A = SET(7, RM(cpustate, cpustate->ea) ); WM(cpustate,  cpustate->ea,cpustate->_A );                    } /* SET  7,A=(XY+o)  */
