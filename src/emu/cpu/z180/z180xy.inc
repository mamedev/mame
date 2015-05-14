// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { _B = RLC(RM(m_ea) ); WM( m_ea,_B );                        } /* RLC  B=(XY+o)    */
OP(xycb,01) { _C = RLC(RM(m_ea) ); WM( m_ea,_C );                        } /* RLC  C=(XY+o)    */
OP(xycb,02) { _D = RLC(RM(m_ea) ); WM( m_ea,_D );                        } /* RLC  D=(XY+o)    */
OP(xycb,03) { _E = RLC(RM(m_ea) ); WM( m_ea,_E );                        } /* RLC  E=(XY+o)    */
OP(xycb,04) { _H = RLC(RM(m_ea) ); WM( m_ea,_H );                        } /* RLC  H=(XY+o)    */
OP(xycb,05) { _L = RLC(RM(m_ea) ); WM( m_ea,_L );                        } /* RLC  L=(XY+o)    */
OP(xycb,06) { WM( m_ea, RLC(RM(m_ea) ) );                                } /* RLC  (XY+o)      */
OP(xycb,07) { _A = RLC(RM(m_ea) ); WM( m_ea,_A );                        } /* RLC  A=(XY+o)    */

OP(xycb,08) { _B = RRC(RM(m_ea) ); WM( m_ea,_B );                        } /* RRC  B=(XY+o)    */
OP(xycb,09) { _C = RRC(RM(m_ea) ); WM( m_ea,_C );                        } /* RRC  C=(XY+o)    */
OP(xycb,0a) { _D = RRC(RM(m_ea) ); WM( m_ea,_D );                        } /* RRC  D=(XY+o)    */
OP(xycb,0b) { _E = RRC(RM(m_ea) ); WM( m_ea,_E );                        } /* RRC  E=(XY+o)    */
OP(xycb,0c) { _H = RRC(RM(m_ea) ); WM( m_ea,_H );                        } /* RRC  H=(XY+o)    */
OP(xycb,0d) { _L = RRC(RM(m_ea) ); WM( m_ea,_L );                        } /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM( m_ea,RRC(RM(m_ea) ) );                             } /* RRC  (XY+o)      */
OP(xycb,0f) { _A = RRC(RM(m_ea) ); WM( m_ea,_A );                        } /* RRC  A=(XY+o)    */

OP(xycb,10) { _B = RL(RM(m_ea) ); WM( m_ea,_B );                     } /* RL   B=(XY+o)    */
OP(xycb,11) { _C = RL(RM(m_ea) ); WM( m_ea,_C );                     } /* RL   C=(XY+o)    */
OP(xycb,12) { _D = RL(RM(m_ea) ); WM( m_ea,_D );                     } /* RL   D=(XY+o)    */
OP(xycb,13) { _E = RL(RM(m_ea) ); WM( m_ea,_E );                     } /* RL   E=(XY+o)    */
OP(xycb,14) { _H = RL(RM(m_ea) ); WM( m_ea,_H );                     } /* RL   H=(XY+o)    */
OP(xycb,15) { _L = RL(RM(m_ea) ); WM( m_ea,_L );                     } /* RL   L=(XY+o)    */
OP(xycb,16) { WM( m_ea,RL(RM(m_ea) ) );                              } /* RL   (XY+o)      */
OP(xycb,17) { _A = RL(RM(m_ea) ); WM( m_ea,_A );                     } /* RL   A=(XY+o)    */

OP(xycb,18) { _B = RR(RM(m_ea) ); WM( m_ea,_B );                     } /* RR   B=(XY+o)    */
OP(xycb,19) { _C = RR(RM(m_ea) ); WM( m_ea,_C );                     } /* RR   C=(XY+o)    */
OP(xycb,1a) { _D = RR(RM(m_ea) ); WM( m_ea,_D );                     } /* RR   D=(XY+o)    */
OP(xycb,1b) { _E = RR(RM(m_ea) ); WM( m_ea,_E );                     } /* RR   E=(XY+o)    */
OP(xycb,1c) { _H = RR(RM(m_ea) ); WM( m_ea,_H );                     } /* RR   H=(XY+o)    */
OP(xycb,1d) { _L = RR(RM(m_ea) ); WM( m_ea,_L );                     } /* RR   L=(XY+o)    */
OP(xycb,1e) { WM( m_ea,RR(RM(m_ea) ) );                              } /* RR   (XY+o)      */
OP(xycb,1f) { _A = RR(RM(m_ea) ); WM( m_ea,_A );                     } /* RR   A=(XY+o)    */

OP(xycb,20) { _B = SLA(RM(m_ea) ); WM( m_ea,_B );                        } /* SLA  B=(XY+o)    */
OP(xycb,21) { _C = SLA(RM(m_ea) ); WM( m_ea,_C );                        } /* SLA  C=(XY+o)    */
OP(xycb,22) { _D = SLA(RM(m_ea) ); WM( m_ea,_D );                        } /* SLA  D=(XY+o)    */
OP(xycb,23) { _E = SLA(RM(m_ea) ); WM( m_ea,_E );                        } /* SLA  E=(XY+o)    */
OP(xycb,24) { _H = SLA(RM(m_ea) ); WM( m_ea,_H );                        } /* SLA  H=(XY+o)    */
OP(xycb,25) { _L = SLA(RM(m_ea) ); WM( m_ea,_L );                        } /* SLA  L=(XY+o)    */
OP(xycb,26) { WM( m_ea,SLA(RM(m_ea) ) );                             } /* SLA  (XY+o)      */
OP(xycb,27) { _A = SLA(RM(m_ea) ); WM( m_ea,_A );                        } /* SLA  A=(XY+o)    */

OP(xycb,28) { _B = SRA(RM(m_ea) ); WM( m_ea,_B );                        } /* SRA  B=(XY+o)    */
OP(xycb,29) { _C = SRA(RM(m_ea) ); WM( m_ea,_C );                        } /* SRA  C=(XY+o)    */
OP(xycb,2a) { _D = SRA(RM(m_ea) ); WM( m_ea,_D );                        } /* SRA  D=(XY+o)    */
OP(xycb,2b) { _E = SRA(RM(m_ea) ); WM( m_ea,_E );                        } /* SRA  E=(XY+o)    */
OP(xycb,2c) { _H = SRA(RM(m_ea) ); WM( m_ea,_H );                        } /* SRA  H=(XY+o)    */
OP(xycb,2d) { _L = SRA(RM(m_ea) ); WM( m_ea,_L );                        } /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM( m_ea,SRA(RM(m_ea) ) );                             } /* SRA  (XY+o)      */
OP(xycb,2f) { _A = SRA(RM(m_ea) ); WM( m_ea,_A );                        } /* SRA  A=(XY+o)    */

OP(xycb,30) { _B = SLL(RM(m_ea) ); WM( m_ea,_B );                        } /* SLL  B=(XY+o)    */
OP(xycb,31) { _C = SLL(RM(m_ea) ); WM( m_ea,_C );                        } /* SLL  C=(XY+o)    */
OP(xycb,32) { _D = SLL(RM(m_ea) ); WM( m_ea,_D );                        } /* SLL  D=(XY+o)    */
OP(xycb,33) { _E = SLL(RM(m_ea) ); WM( m_ea,_E );                        } /* SLL  E=(XY+o)    */
OP(xycb,34) { _H = SLL(RM(m_ea) ); WM( m_ea,_H );                        } /* SLL  H=(XY+o)    */
OP(xycb,35) { _L = SLL(RM(m_ea) ); WM( m_ea,_L );                        } /* SLL  L=(XY+o)    */
OP(xycb,36) { WM( m_ea,SLL(RM(m_ea) ) );                             } /* SLL  (XY+o)      */
OP(xycb,37) { _A = SLL(RM(m_ea) ); WM( m_ea,_A );                        } /* SLL  A=(XY+o)    */

OP(xycb,38) { _B = SRL(RM(m_ea) ); WM( m_ea,_B );                        } /* SRL  B=(XY+o)    */
OP(xycb,39) { _C = SRL(RM(m_ea) ); WM( m_ea,_C );                        } /* SRL  C=(XY+o)    */
OP(xycb,3a) { _D = SRL(RM(m_ea) ); WM( m_ea,_D );                        } /* SRL  D=(XY+o)    */
OP(xycb,3b) { _E = SRL(RM(m_ea) ); WM( m_ea,_E );                        } /* SRL  E=(XY+o)    */
OP(xycb,3c) { _H = SRL(RM(m_ea) ); WM( m_ea,_H );                        } /* SRL  H=(XY+o)    */
OP(xycb,3d) { _L = SRL(RM(m_ea) ); WM( m_ea,_L );                        } /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM( m_ea,SRL(RM(m_ea) ) );                             } /* SRL  (XY+o)      */
OP(xycb,3f) { _A = SRL(RM(m_ea) ); WM( m_ea,_A );                        } /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46();                                            } /* BIT  0,B=(XY+o)  */
OP(xycb,41) { xycb_46();                                                      } /* BIT  0,C=(XY+o)  */
OP(xycb,42) { xycb_46();                                            } /* BIT  0,D=(XY+o)  */
OP(xycb,43) { xycb_46();                                            } /* BIT  0,E=(XY+o)  */
OP(xycb,44) { xycb_46();                                            } /* BIT  0,H=(XY+o)  */
OP(xycb,45) { xycb_46();                                            } /* BIT  0,L=(XY+o)  */
OP(xycb,46) { BIT_XY(0,RM(m_ea));                                     } /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46();                                            } /* BIT  0,A=(XY+o)  */

OP(xycb,48) { xycb_4e();                                            } /* BIT  1,B=(XY+o)  */
OP(xycb,49) { xycb_4e();                                                      } /* BIT  1,C=(XY+o)  */
OP(xycb,4a) { xycb_4e();                                            } /* BIT  1,D=(XY+o)  */
OP(xycb,4b) { xycb_4e();                                            } /* BIT  1,E=(XY+o)  */
OP(xycb,4c) { xycb_4e();                                            } /* BIT  1,H=(XY+o)  */
OP(xycb,4d) { xycb_4e();                                            } /* BIT  1,L=(XY+o)  */
OP(xycb,4e) { BIT_XY(1,RM(m_ea));                                     } /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e();                                            } /* BIT  1,A=(XY+o)  */

OP(xycb,50) { xycb_56();                                            } /* BIT  2,B=(XY+o)  */
OP(xycb,51) { xycb_56();                                                      } /* BIT  2,C=(XY+o)  */
OP(xycb,52) { xycb_56();                                            } /* BIT  2,D=(XY+o)  */
OP(xycb,53) { xycb_56();                                            } /* BIT  2,E=(XY+o)  */
OP(xycb,54) { xycb_56();                                            } /* BIT  2,H=(XY+o)  */
OP(xycb,55) { xycb_56();                                            } /* BIT  2,L=(XY+o)  */
OP(xycb,56) { BIT_XY(2,RM(m_ea));                                     } /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56();                                            } /* BIT  2,A=(XY+o)  */

OP(xycb,58) { xycb_5e();                                            } /* BIT  3,B=(XY+o)  */
OP(xycb,59) { xycb_5e();                                                      } /* BIT  3,C=(XY+o)  */
OP(xycb,5a) { xycb_5e();                                            } /* BIT  3,D=(XY+o)  */
OP(xycb,5b) { xycb_5e();                                            } /* BIT  3,E=(XY+o)  */
OP(xycb,5c) { xycb_5e();                                            } /* BIT  3,H=(XY+o)  */
OP(xycb,5d) { xycb_5e();                                            } /* BIT  3,L=(XY+o)  */
OP(xycb,5e) { BIT_XY(3,RM(m_ea));                                     } /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e();                                            } /* BIT  3,A=(XY+o)  */

OP(xycb,60) { xycb_66();                                            } /* BIT  4,B=(XY+o)  */
OP(xycb,61) { xycb_66();                                                      } /* BIT  4,C=(XY+o)  */
OP(xycb,62) { xycb_66();                                            } /* BIT  4,D=(XY+o)  */
OP(xycb,63) { xycb_66();                                            } /* BIT  4,E=(XY+o)  */
OP(xycb,64) { xycb_66();                                            } /* BIT  4,H=(XY+o)  */
OP(xycb,65) { xycb_66();                                            } /* BIT  4,L=(XY+o)  */
OP(xycb,66) { BIT_XY(4,RM(m_ea));                                     } /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66();                                            } /* BIT  4,A=(XY+o)  */

OP(xycb,68) { xycb_6e();                                            } /* BIT  5,B=(XY+o)  */
OP(xycb,69) { xycb_6e();                                                      } /* BIT  5,C=(XY+o)  */
OP(xycb,6a) { xycb_6e();                                            } /* BIT  5,D=(XY+o)  */
OP(xycb,6b) { xycb_6e();                                            } /* BIT  5,E=(XY+o)  */
OP(xycb,6c) { xycb_6e();                                            } /* BIT  5,H=(XY+o)  */
OP(xycb,6d) { xycb_6e();                                            } /* BIT  5,L=(XY+o)  */
OP(xycb,6e) { BIT_XY(5,RM(m_ea));                                     } /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e();                                            } /* BIT  5,A=(XY+o)  */

OP(xycb,70) { xycb_76();                                            } /* BIT  6,B=(XY+o)  */
OP(xycb,71) { xycb_76();                                                      } /* BIT  6,C=(XY+o)  */
OP(xycb,72) { xycb_76();                                            } /* BIT  6,D=(XY+o)  */
OP(xycb,73) { xycb_76();                                            } /* BIT  6,E=(XY+o)  */
OP(xycb,74) { xycb_76();                                            } /* BIT  6,H=(XY+o)  */
OP(xycb,75) { xycb_76();                                            } /* BIT  6,L=(XY+o)  */
OP(xycb,76) { BIT_XY(6,RM(m_ea));                                     } /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76();                                            } /* BIT  6,A=(XY+o)  */

OP(xycb,78) { xycb_7e();                                            } /* BIT  7,B=(XY+o)  */
OP(xycb,79) { xycb_7e();                                                      } /* BIT  7,C=(XY+o)  */
OP(xycb,7a) { xycb_7e();                                            } /* BIT  7,D=(XY+o)  */
OP(xycb,7b) { xycb_7e();                                            } /* BIT  7,E=(XY+o)  */
OP(xycb,7c) { xycb_7e();                                            } /* BIT  7,H=(XY+o)  */
OP(xycb,7d) { xycb_7e();                                            } /* BIT  7,L=(XY+o)  */
OP(xycb,7e) { BIT_XY(7,RM(m_ea));                                     } /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e();                                            } /* BIT  7,A=(XY+o)  */

OP(xycb,80) { _B = RES(0, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  0,B=(XY+o)  */
OP(xycb,81) { _C = RES(0, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  0,C=(XY+o)  */
OP(xycb,82) { _D = RES(0, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  0,D=(XY+o)  */
OP(xycb,83) { _E = RES(0, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  0,E=(XY+o)  */
OP(xycb,84) { _H = RES(0, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  0,H=(XY+o)  */
OP(xycb,85) { _L = RES(0, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM( m_ea, RES(0,RM(m_ea)) );                              } /* RES  0,(XY+o)    */
OP(xycb,87) { _A = RES(0, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  0,A=(XY+o)  */

OP(xycb,88) { _B = RES(1, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  1,B=(XY+o)  */
OP(xycb,89) { _C = RES(1, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  1,C=(XY+o)  */
OP(xycb,8a) { _D = RES(1, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  1,D=(XY+o)  */
OP(xycb,8b) { _E = RES(1, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  1,E=(XY+o)  */
OP(xycb,8c) { _H = RES(1, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  1,H=(XY+o)  */
OP(xycb,8d) { _L = RES(1, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM( m_ea, RES(1,RM(m_ea)) );                              } /* RES  1,(XY+o)    */
OP(xycb,8f) { _A = RES(1, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  1,A=(XY+o)  */

OP(xycb,90) { _B = RES(2, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  2,B=(XY+o)  */
OP(xycb,91) { _C = RES(2, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  2,C=(XY+o)  */
OP(xycb,92) { _D = RES(2, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  2,D=(XY+o)  */
OP(xycb,93) { _E = RES(2, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  2,E=(XY+o)  */
OP(xycb,94) { _H = RES(2, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  2,H=(XY+o)  */
OP(xycb,95) { _L = RES(2, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM( m_ea, RES(2,RM(m_ea)) );                              } /* RES  2,(XY+o)    */
OP(xycb,97) { _A = RES(2, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  2,A=(XY+o)  */

OP(xycb,98) { _B = RES(3, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  3,B=(XY+o)  */
OP(xycb,99) { _C = RES(3, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  3,C=(XY+o)  */
OP(xycb,9a) { _D = RES(3, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  3,D=(XY+o)  */
OP(xycb,9b) { _E = RES(3, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  3,E=(XY+o)  */
OP(xycb,9c) { _H = RES(3, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  3,H=(XY+o)  */
OP(xycb,9d) { _L = RES(3, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM( m_ea, RES(3,RM(m_ea)) );                              } /* RES  3,(XY+o)    */
OP(xycb,9f) { _A = RES(3, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  3,A=(XY+o)  */

OP(xycb,a0) { _B = RES(4, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  4,B=(XY+o)  */
OP(xycb,a1) { _C = RES(4, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  4,C=(XY+o)  */
OP(xycb,a2) { _D = RES(4, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  4,D=(XY+o)  */
OP(xycb,a3) { _E = RES(4, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  4,E=(XY+o)  */
OP(xycb,a4) { _H = RES(4, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  4,H=(XY+o)  */
OP(xycb,a5) { _L = RES(4, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM( m_ea, RES(4,RM(m_ea)) );                              } /* RES  4,(XY+o)    */
OP(xycb,a7) { _A = RES(4, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  4,A=(XY+o)  */

OP(xycb,a8) { _B = RES(5, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  5,B=(XY+o)  */
OP(xycb,a9) { _C = RES(5, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  5,C=(XY+o)  */
OP(xycb,aa) { _D = RES(5, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  5,D=(XY+o)  */
OP(xycb,ab) { _E = RES(5, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  5,E=(XY+o)  */
OP(xycb,ac) { _H = RES(5, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  5,H=(XY+o)  */
OP(xycb,ad) { _L = RES(5, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM( m_ea, RES(5,RM(m_ea)) );                              } /* RES  5,(XY+o)    */
OP(xycb,af) { _A = RES(5, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  5,A=(XY+o)  */

OP(xycb,b0) { _B = RES(6, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  6,B=(XY+o)  */
OP(xycb,b1) { _C = RES(6, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  6,C=(XY+o)  */
OP(xycb,b2) { _D = RES(6, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  6,D=(XY+o)  */
OP(xycb,b3) { _E = RES(6, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  6,E=(XY+o)  */
OP(xycb,b4) { _H = RES(6, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  6,H=(XY+o)  */
OP(xycb,b5) { _L = RES(6, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM( m_ea, RES(6,RM(m_ea)) );                              } /* RES  6,(XY+o)    */
OP(xycb,b7) { _A = RES(6, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  6,A=(XY+o)  */

OP(xycb,b8) { _B = RES(7, RM(m_ea) ); WM( m_ea,_B );                    } /* RES  7,B=(XY+o)  */
OP(xycb,b9) { _C = RES(7, RM(m_ea) ); WM( m_ea,_C );                    } /* RES  7,C=(XY+o)  */
OP(xycb,ba) { _D = RES(7, RM(m_ea) ); WM( m_ea,_D );                    } /* RES  7,D=(XY+o)  */
OP(xycb,bb) { _E = RES(7, RM(m_ea) ); WM( m_ea,_E );                    } /* RES  7,E=(XY+o)  */
OP(xycb,bc) { _H = RES(7, RM(m_ea) ); WM( m_ea,_H );                    } /* RES  7,H=(XY+o)  */
OP(xycb,bd) { _L = RES(7, RM(m_ea) ); WM( m_ea,_L );                    } /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM( m_ea, RES(7,RM(m_ea)) );                              } /* RES  7,(XY+o)    */
OP(xycb,bf) { _A = RES(7, RM(m_ea) ); WM( m_ea,_A );                    } /* RES  7,A=(XY+o)  */

OP(xycb,c0) { _B = SET(0, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  0,B=(XY+o)  */
OP(xycb,c1) { _C = SET(0, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  0,C=(XY+o)  */
OP(xycb,c2) { _D = SET(0, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  0,D=(XY+o)  */
OP(xycb,c3) { _E = SET(0, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  0,E=(XY+o)  */
OP(xycb,c4) { _H = SET(0, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  0,H=(XY+o)  */
OP(xycb,c5) { _L = SET(0, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM( m_ea, SET(0,RM(m_ea)) );                              } /* SET  0,(XY+o)    */
OP(xycb,c7) { _A = SET(0, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  0,A=(XY+o)  */

OP(xycb,c8) { _B = SET(1, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  1,B=(XY+o)  */
OP(xycb,c9) { _C = SET(1, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  1,C=(XY+o)  */
OP(xycb,ca) { _D = SET(1, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  1,D=(XY+o)  */
OP(xycb,cb) { _E = SET(1, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  1,E=(XY+o)  */
OP(xycb,cc) { _H = SET(1, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  1,H=(XY+o)  */
OP(xycb,cd) { _L = SET(1, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM( m_ea, SET(1,RM(m_ea)) );                              } /* SET  1,(XY+o)    */
OP(xycb,cf) { _A = SET(1, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  1,A=(XY+o)  */

OP(xycb,d0) { _B = SET(2, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  2,B=(XY+o)  */
OP(xycb,d1) { _C = SET(2, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  2,C=(XY+o)  */
OP(xycb,d2) { _D = SET(2, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  2,D=(XY+o)  */
OP(xycb,d3) { _E = SET(2, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  2,E=(XY+o)  */
OP(xycb,d4) { _H = SET(2, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  2,H=(XY+o)  */
OP(xycb,d5) { _L = SET(2, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM( m_ea, SET(2,RM(m_ea)) );                              } /* SET  2,(XY+o)    */
OP(xycb,d7) { _A = SET(2, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  2,A=(XY+o)  */

OP(xycb,d8) { _B = SET(3, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  3,B=(XY+o)  */
OP(xycb,d9) { _C = SET(3, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  3,C=(XY+o)  */
OP(xycb,da) { _D = SET(3, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  3,D=(XY+o)  */
OP(xycb,db) { _E = SET(3, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  3,E=(XY+o)  */
OP(xycb,dc) { _H = SET(3, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  3,H=(XY+o)  */
OP(xycb,dd) { _L = SET(3, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM( m_ea, SET(3,RM(m_ea)) );                              } /* SET  3,(XY+o)    */
OP(xycb,df) { _A = SET(3, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  3,A=(XY+o)  */

OP(xycb,e0) { _B = SET(4, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  4,B=(XY+o)  */
OP(xycb,e1) { _C = SET(4, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  4,C=(XY+o)  */
OP(xycb,e2) { _D = SET(4, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  4,D=(XY+o)  */
OP(xycb,e3) { _E = SET(4, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  4,E=(XY+o)  */
OP(xycb,e4) { _H = SET(4, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  4,H=(XY+o)  */
OP(xycb,e5) { _L = SET(4, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM( m_ea, SET(4,RM(m_ea)) );                              } /* SET  4,(XY+o)    */
OP(xycb,e7) { _A = SET(4, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  4,A=(XY+o)  */

OP(xycb,e8) { _B = SET(5, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  5,B=(XY+o)  */
OP(xycb,e9) { _C = SET(5, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  5,C=(XY+o)  */
OP(xycb,ea) { _D = SET(5, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  5,D=(XY+o)  */
OP(xycb,eb) { _E = SET(5, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  5,E=(XY+o)  */
OP(xycb,ec) { _H = SET(5, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  5,H=(XY+o)  */
OP(xycb,ed) { _L = SET(5, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM( m_ea, SET(5,RM(m_ea)) );                              } /* SET  5,(XY+o)    */
OP(xycb,ef) { _A = SET(5, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  5,A=(XY+o)  */

OP(xycb,f0) { _B = SET(6, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  6,B=(XY+o)  */
OP(xycb,f1) { _C = SET(6, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  6,C=(XY+o)  */
OP(xycb,f2) { _D = SET(6, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  6,D=(XY+o)  */
OP(xycb,f3) { _E = SET(6, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  6,E=(XY+o)  */
OP(xycb,f4) { _H = SET(6, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  6,H=(XY+o)  */
OP(xycb,f5) { _L = SET(6, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM( m_ea, SET(6,RM(m_ea)) );                              } /* SET  6,(XY+o)    */
OP(xycb,f7) { _A = SET(6, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  6,A=(XY+o)  */

OP(xycb,f8) { _B = SET(7, RM(m_ea) ); WM( m_ea,_B );                    } /* SET  7,B=(XY+o)  */
OP(xycb,f9) { _C = SET(7, RM(m_ea) ); WM( m_ea,_C );                    } /* SET  7,C=(XY+o)  */
OP(xycb,fa) { _D = SET(7, RM(m_ea) ); WM( m_ea,_D );                    } /* SET  7,D=(XY+o)  */
OP(xycb,fb) { _E = SET(7, RM(m_ea) ); WM( m_ea,_E );                    } /* SET  7,E=(XY+o)  */
OP(xycb,fc) { _H = SET(7, RM(m_ea) ); WM( m_ea,_H );                    } /* SET  7,H=(XY+o)  */
OP(xycb,fd) { _L = SET(7, RM(m_ea) ); WM( m_ea,_L );                    } /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM( m_ea, SET(7,RM(m_ea)) );                              } /* SET  7,(XY+o)    */
OP(xycb,ff) { _A = SET(7, RM(m_ea) ); WM( m_ea,_A );                    } /* SET  7,A=(XY+o)  */
