// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

The CPS1 system is made of 3 boards: A, B and C. The first two games also exist
a 2-board setups, where the C-board is integrated into the B-board.

There are several revisions of the A-board, but they are functionally equivalent
and interchangeable except for 89626A-4 DASH used by sf2ce which has a 12MHz xtal
replacing the 10MHz one. Note that it's likely that from one point onwards Capcom
simply switched to make only 89626A-4 DASH board, that is all games after a
certain point should use the 12MHz xtal not just sf2ce.

Known A-board revisions:

88617A-4
88617A-5
88617A-7
88617A-7b
89626A-4
89626A-4 DASH

Game                                                         Year  B-board #       B-board PALs       C-board #           CPS-B #          C-board PALs
-----------------------------------------------------------  ----  ---------  ---------------------  -----------  -----------------------  ------------
Forgotten Worlds (World, newer)                              1988  88621B-2   LW621            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Forgotten Worlds (World)                                           88621B-2   LW621            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Forgotten Worlds (USA, B-Board 88618B-2, Rev. A)                   88618B-2   LWCHR            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Forgotten Worlds (USA, B-Board 88618B-2, Rev. AA)                  88618B-2   LWCHR            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Forgotten Worlds (USA, B-Board 88618B-2, Rev. C)                   88618B-2   LWCHR            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Forgotten Worlds (USA, B-Board 88621B-2, Rev. C)                   88621B-2   LW621            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Lost Worlds (Japan Old Ver.)                                       88618B-2   LWCHR            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Lost Worlds (Japan)                                                88618B-2   LWCHR            LWIO  None         CPS-B-01  DL-0411-10001  N/A

Ghouls'n Ghosts (World)                                      1988  88620-B-?  DM620            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Ghouls'n Ghosts (USA)                                              88620-B-2  DM620            LWIO  None         CPS-B-01  DL-0411-10001  N/A
Daimakaimura (Japan)                                               88622B-2   DM22A            LWIO  88622-C-1    CPS-B-01  DL-0411-10001  None
Daimakaimura (Japan Resale Ver.)                                   91634B-2   DAM63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Strider (USA, B-Board 89624B-2)                              1989  89624B-2   ST24M1           LWIO  88622-C-1/2  CPS-B-01  DL-0411-10001  None
Strider (USA, B-Board 89624B-3)                                    89624B-3   ST24B2           LWIO  88622-C-2    CPS-B-01  DL-0411-10001  None
Strider Hiryu (Japan)                                              88622B-3   ST22B            LWIO  88622-C-2    CPS-B-01  DL-0411-10001  None
Strider Hiryu (Japan Resale Ver.)                                  91634B-2   STH63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Dynasty Wars (USA, B-Board 88622B-3)                         1989  88622B-3   TK22B            LWIO  88622-C-2    CPS-B-02  DL-0411-10002  None
Dynasty Wars (USA, B-Board 89624B-?)                               89624B-?   TK24B1           LWIO? ?            CPS-B-02  DL-0411-10002
Tenchi wo Kurau (Japan)                                            88622B-3   TK22B            LWIO  88622-C-2    CPS-B-02  DL-0411-10002  None
Tenchi wo Kurau (Japan Resale Ver.)                                91634B-2   TK163B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Willow (World)                                               1989  89624B-3   WL24B            LWIO  88622-C-4    CPS-B-03  DL-0411-10003  None
Willow (USA Old Ver.)                                              89624B-3   WL24B            LWIO  88622-C-2    CPS-B-03  DL-0411-10003  None
Willow (USA)                                                       89624B-3   WL24B            LWIO  88622-C-2/4  CPS-B-03  DL-0411-10003  None
Willow (Japan)                                                     88622B-3   WL22B            LWIO  88622-C-2    CPS-B-03  DL-0411-10003  None

U.N. Squadron (USA)                                          1989  89624B-2   AR24B            LWIO  88622-C-4    CPS-B-11  DL-0411-10004  None
Area 88 (Japan)                                                    88622B-3   AR22B            LWIO  88622-C-4    CPS-B-11  DL-0411-10004  None
Area 88 (Japan Resale Ver.)                                        91634B-2   ARA63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Final Fight (World, set 1)                                   1989  89624B-3   S224B            IOB1  88622-C-5    CPS-B-04  DL-0411-10005  None
Final Fight (World, set 2)                                         89624B-3   S224B            LWIO  88622-C-5    CPS-B-04  DL-0411-10005  None
Final Fight (USA, set 1)                                           89624B-3   S224B            IOB1  88622-C-5    CPS-B-04  DL-0411-10005  None
Final Fight (USA, set 2)                                           89624B-3   S224B            IOB1  88622-C-5    CPS-B-04  DL-0411-10005  None
Final Fight (USA 900112)                                           89624B-3   S224B            IOB1  88622-C-5    CPS-B-04@ DL-0411-10001  None
Final Fight (USA 900424)                                           89624B-3   S224B            IOB1  88622-C-5    CPS-B-03  DL-0411-10003  None
Final Fight (USA 900613)                                           89624B-3   S224B            IOB1  88622-C-5    CPS-B-05  DL-0411-10006  None
Final Fight (Japan)                                                ?          S222B            ?     ?            CPS-B-04  DL-0411-10005
Final Fight (Japan 900112)                                         89625B-1   S222B            LWIO  88622-C-5    CPS-B-01  DL-0411-10001  None
Final Fight (Japan 900305)                                         88622B-3   S222B            LWIO  88622-C-5    CPS-B-02  DL-0411-10002  None
Final Fight (Japan 900613)                                         89625B-1   S222B            LWIO  88622-C-5    CPS-B-05  DL-0411-10006  None

1941: Counter Attack (World)                                 1990  89624B-3   YI24B            IOB1  88622-C-5    CPS-B-05  DL-0411-10006  None
1941: Counter Attack (World 900227)                                89624B-3   YI24B            IOB1  88622-C-5    CPS-B-05  DL-0411-10006  None
1941: Counter Attack (USA 900227)                                  89624B-3   YI24B            IOB1  88622-C-5    CPS-B-05  DL-0411-10006  None
1941: Counter Attack (Japan)                                       89625B-1   YI22B            LWIO? 88622-C-5    CPS-B-05  DL-0411-10006  None

Mercs (World 900302)                                         1990  89624B-3   O224B            IOB1  90628-C-1    CPS-B-12  DL-0411-10007  C628
Mercs (USA 900302)                                                 89624B-3   O224B            IOB1  90628-C-1/2  CPS-B-12  DL-0411-10007  C628
Mercs (USA 900608)                                                 89624B-3   O224B            IOB1  90628-C-1    CPS-B-12  DL-0411-10007  C628
Senjou no Ookami II (Japan 900302)                                 89625B-1   O222B            LWIO? 90628-C-1    CPS-B-12  DL-0411-10007  C628

Mega Twins (World 900619)                                    1990  89624B-3   CK24B            IOB1  88622-C-5    CPS-B-14  DL-0411-10009  None
Chiki Chiki Boys (Japan 900619)                                    89625B-1   CK22B            ?     ?            CPS-B-14  DL-0411-10009  None

Magic Sword: Heroic Fantasy (World 900623)                   1990  89624B-3   MS24B            IOB1  88622-C-5    CPS-B-13  DL-0411-10008  None
Magic Sword: Heroic Fantasy (World 900725)                         89624B-3   MS24B            IOB1  88622-C-5    CPS-B-13  DL-0411-10008  None
Magic Sword: Heroic Fantasy (USA 900725)                           89624B-3   MS24B            IOB1  88622-C-5    CPS-B-13  DL-0411-10008  None
Magic Sword: Heroic Fantasy (Japan 900623)                         89625B-1   MS22B            IOB1  88622-C-5    CPS-B-13  DL-0411-10008  None

Carrier Air Wing (World 901009)                              1990  89624B-3   CA24B            IOB1  88622-C-5    CPS-B-16  DL-0411-10011  None
Carrier Air Wing (World 901012)                                    89624B-3   CA24B            IOB1  88622-C-5    CPS-B-16  DL-0411-10011  None
Carrier Air Wing (USA 901012)                                      89624B-3   CA24B            IOB1  88622-C-5    CPS-B-16  DL-0411-10011  None
U.S. Navy (Japan 901012)                                           89625B-1   CA22B            IOB1  88622-C-5    CPS-B-16  DL-0411-10011  None

Nemo (World 901109)                                          1990  89624B-3   NM24B            IOB1  88622-C-5    CPS-B-15  DL-0411-10010  None
Nemo (World 901130)                                                89624B-3   NM24B            IOB1  88622-C-5    CPS-B-15  DL-0411-10010  None
Nemo (Japan 901120)                                                89625B-1   NM22B            ?     ?            CPS-B-15  DL-0411-10010

Street Fighter II: The World Warrior (World 910214)          1991  90629B-2   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632
Street Fighter II: The World Warrior (World 910228)                90629B-3   STF29            IOB2  90632C-1     CPS-B-18  DL-0411-10013  C632B
Street Fighter II: The World Warrior (World 910318)                90629B-3   STF29            IOB1  90632C-1     CPS-B-05  DL-0411-10006  C632
Street Fighter II: The World Warrior (World 910522)                90629B-3   STF29            IOB1  90632C-1     CPS-B-11  DL-0411-10004  C632
Street Fighter II: The World Warrior (USA 910206)                  90629B-2   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632
Street Fighter II: The World Warrior (USA 910214)                  90629B-3   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632
Street Fighter II: The World Warrior (USA 910228)                  90629B-3   STF29            IOB2  90632C-1     CPS-B-18  DL-0411-10013  C632B
Street Fighter II: The World Warrior (USA 910306)                  90629B-3   STF29            IOB1  90632C-1     CPS-B-12  DL-0411-10007  C632
Street Fighter II: The World Warrior (USA 910318)                  90629B-3   STF29            IOB1  90632C-1     CPS-B-05  DL-0411-10006  C632
Street Fighter II: The World Warrior (USA 910411)                  90629B-3   STF29            IOB1  90632C-1     CPS-B-15  DL-0411-10010  C632
Street Fighter II: The World Warrior (USA 910522, Rev. G)          90629B-3   STF29            IOB1  90632C-1     CPS-B-11  DL-0411-10004  C632
Street Fighter II: The World Warrior (USA 910522, Rev. I)          90629B-3   STF29            IOB1  90632C-1     CPS-B-14  DL-0411-10009  C632
Street Fighter II: The World Warrior (USA 911101)                  90629B-3   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632
Street Fighter II: The World Warrior (Japan 910214)                90629B-2   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632
Street Fighter II: The World Warrior (Japan 910306)                90629B-3   STF29            IOB1  90632C-1     CPS-B-12  DL-0411-10007  C632
Street Fighter II: The World Warrior (Japan 910411)                90629B-3   STF29            IOB1  90632C-1     CPS-B-15  DL-0411-10010  C632
Street Fighter II: The World Warrior (Japan 910522)                90629B-3   STF29            IOB1  90632C-1     CPS-B-13  DL-0411-10008  C632
Street Fighter II: The World Warrior (Japan 911210)                90629B-?   STF29            IOB1  ?            CPS-B-13  DL-0411-10008  C632
Street Fighter II: The World Warrior (Japan 920312)                90629B-2   STF29            IOB1  90632C-1     CPS-B-17  DL-0411-10012  C632

Three Wonders* (World 910513)                                1991  89624B-3   RT24B            IOB1  90630C-4     CPS-B-21  DL-0921-10014          IOC1
Three Wonders* (World 910520)                                      89624B-3   RT24B            IOB1  90630C-4     CPS-B-21  DL-0921-10014          IOC1
Three Wonders* (USA 910520)                                        89624B-3   RT24B            IOB1  90630C-4     CPS-B-21  DL-0921-10014          IOC1
Wonder 3* (Japan 910520)                                           89625B-1   RT22B            IOB1  90630C-4     CPS-B-21  DL-0921-10014          IOC1

The King of Dragons* (World 910711)                          1991  90629B-3   KD29B            IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
The King of Dragons* (World 910805)                                90629B-3   KD29B            IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
The King of Dragons* (USA 910910)                                  90629B-3   KD29B            IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
The King of Dragons* (Japan 910805, B-Board 89625B-1)              89625B-1   KD22B            IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
The King of Dragons* (Japan 910805, B-Board 90629B-3)              90629B-3   KD29B            IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1

Captain Commando* (World 911014)                             1991  91635B-2   CC63B    CCPRG   IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Captain Commando* (World 911202)                                   91635B-2   CC63B    CCPRG1  IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632B   IOC1
Captain Commando* (USA 910928)                                     91635B-2   CC63B    CCPRG1  IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Captain Commando* (Japan 910928)                                   91634B-2   CC63B    CCPRG   IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Captain Commando* (Japan 911202)                                   91634B-2   CC63B    CCPRG1  IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632B   IOC1

Knights of the Round* (World 911127)                         1991  91635B-2   KR63B    BPRG1   IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Knights of the Round* (USA 911127)                                 91635B-2   KR63B    BPRG1   IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Knights of the Round* (Japan 911127, B-Board 89625B-1)             89625B-1   KR22B            LWIO  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1
Knights of the Round* (Japan 911127, B-Board 91634B-2)             91634B-2   KR63B    BPRG1   IOB1  90631C-5     CPS-B-21  DL-0921-10014  C632    IOC1

Street Fighter II': Champion Edition (World 920313)          1992  91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (World 920513)                91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (USA 920313)                  91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (USA 920513)                  91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (USA 920803)                  91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (Japan 920322)                91634B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (Japan 920513)                91634B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Champion Edition (Japan 920803)                91634B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Adventure Quiz Capcom World 2* (Japan 920611)                1992  89625B-1   Q522B            LWIO  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Adventure Quiz Capcom World 2* (Japan 920611)                      91634B-2   Q563B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Adventure Quiz Capcom World 2 (Japan 920611)                       90629B-3   Q529B            IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Varth: Operation Thunderstorm (World 920612)                 1992  89624B-?   VA24B            IOB1  ?            CPS-B-04  DL-0411-10005
Varth: Operation Thunderstorm (World 920714)                       89624B-3   VA24B            IOB1  88622-C-5    CPS-B-04  DL-0411-10005  None
Varth: Operation Thunderstorm (USA 920612)                         91635B-2   VA63B    BPRG1   IOB1  88622-C-5    CPS-B-04  DL-0411-10005  None
Varth: Operation Thunderstorm* (Japan 920714)                      88622B-3   VA22B            LWIO  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Varth: Operation Thunderstorm* (Japan Resale Ver. 920714)          91634B-2   VA63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Quiz & Dragons: Capcom Quiz Game* (USA 920701)               1992  89625B-1   QD22B            IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Quiz & Dragons: Capcom Quiz Game (Japan Resale Ver. 940921)  1994  91634B-2   QAD63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Warriors of Fate* (World 921002)                             1992  91635B-2   TK263B   BPRG1   IOB1  ?            CPS-B-21  DL-0921-10014          IOC1
Warriors of Fate* (World 921031)                                   91635B-2   TK263B   BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Warriors of Fate* (USA 921031)                                     91635B-2   TK263B   BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Sangokushi II* (Asia 921005)                                       91634B-2   TK263B   BPRG1   IOB1  ?            CPS-B-21  DL-0921-10014          IOC1
Tenchi wo Kurau II: Sekiheki no Tatakai* (Japan 921031)            91634B-2   TK263B   BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Street Fighter II': Hyper Fighting (World 921209)            1992  91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II': Hyper Fighting (USA 921209)                    91635B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Street Fighter II' Turbo: Hyper Fighting (Japan 921209)            91634B-2   S9263B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Cadillacs and Dinosaurs* (World 930201)                      1993  91635B-2   CD63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Cadillacs and Dinosaurs* (USA 930201)                              91635B-2   CD63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Cadillacs: Kyouryuu Shin Seiki* (Japan 930201)                     91634B-2   CD63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

The Punisher* (World 930422)                                 1993  91635B-2   PS63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
The Punisher* (USA 930422)                                         91635B-2   PS63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
The Punisher* (Hispanic 930422)                                    91635B-2   PS63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
The Punisher* (Japan 930422)                                       91634B-2   PS63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Saturday Night Slam Masters* (World 930713)                  1993  91635B-2   MB63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Saturday Night Slam Masters* (USA 930713)                          91635B-2   MB63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1
Muscle Bomber: The Body Explosion* (Japan 930713)                  91634B-2   MB63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Muscle Bomber Duo: Ultimate Team Battle* (World 931206)      1993  91635B-?   MB63B    BPRG1   IOB1  ?            CPS-B-21  DL-0921-10014          IOC1
Muscle Bomber Duo: Heat Up Warriors* (Japan 931206)                91634B-2   MB63B    BPRG1   IOB1  92641C-1     CPS-B-21  DL-0921-10014          IOC1

Ken Sei Mogura: Street Fighter II (Japan 940418, Ver 1.00)   1994  91634B-2   KNM10B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Pnickies (Japan 940608)                                      1994  89625B-1   PKB10B           IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Quiz Tonosama no Yabou 2: Zenkoku-ban (Japan 950123)         1995  90629B-3   TN2292           IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Pang! 3 (Euro 950511)                                        1995  94916-10   CP1B1F,CP1B8K,CP1B9K   92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Pang! 3 (Euro 950601)                                              94916-10   CP1B1F,CP1B8K,CP1B9KA  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Pang! 3: Kaitou Tachi no Karei na Gogo (Japan 950511)              94916-10   CP1B1F,CP1B8K,CP1B9K   92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Mega Man: The Power Battle (CPS1, USA 951006)                1995  91635B-2   RCM63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Mega Man: The Power Battle (CPS1, Asia 951006)                     91634B-2   RCM63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1
Rockman: The Power Battle (CPS1, Japan 950922)                     91634B-2   RCM63B   BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1

Ganbare! Marine Kun (Japan 2K0411)                           2000  91634B-2   GBPR2    BPRG1   IOB1  92631C-6     CPS-B-21  DL-0921-10014  C632    IOC1


@the original number (CPS-B-01) was scratched out and "04" stamped over it.
*denotes Suicide Battery

The C628/C632 PALs on some C-boards probably handle the extra inputs (6 buttons/third player).

You can set the suicide CPS-B-21 chips to their default layer register and priority bit values
if you pull pins 45 and 46 high (floating the pins seems to work, too). The default is the same
values as Street Fighter 2 CE/Turbo.



CPS-A Registers
---------------
0x00-0x01     OBJ RAM base (/256)
0x02-0x03     Scroll1 (8x8) RAM base (/256)
0x04-0x05     Scroll2 (16x16) RAM base (/256)
0x06-0x07     Scroll3 (32x32) RAM base (/256)
0x08-0x09     rowscroll RAM base (/256)
0x0a-0x0b     Palette base (/256) after this register is written to, the palette
              is copied from gfxram to the dedicated ram. The palette control
              register (see below) determines how the copy should happen.
              Tests on a msword pcb show that the minimum alignment for the palette
              is 0x400 bytes. The hardware seems to ignore bit 1, while when bit 0
              is set the palette doesn't seem to be copied. However, some games set
              bit 0 during boot (ghouls, strider, 1941) so it still isn't clear
              what bit 0 should actually do.
0x0c-0x0d     Scroll 1 X
0x0e-0x0f     Scroll 1 Y
0x10-0x11     Scroll 2 X
0x12-0x13     Scroll 2 Y
0x14-0x15     Scroll 3 X
0x16-0x17     Scroll 3 Y
0x18-0x19     Starfield 1 X
0x1a-0x1b     Starfield 1 Y
0x1c-0x1d     Starfield 2 X
0x1e-0x1f     Starfield 2 Y
0x20-0x21     start offset for the rowscroll matrix
0x22-0x23     video control. Usually 0x0e.
              bit 0 enables rowscroll on layer 2.
              bit 15 is flip screen.
              ghouls sets bit 14. Purpose unknown.
              1941 uses bits 1-3 by setting them to 0 on screen transitions,
              however it also uses the normal layer control register so there
              doesn't seem to be an obvious effect.

              Games known to use rowscroll:
              SF2
              Mega Twins (underwater, cave)
              Carrier Air Wing (hazy background at beginning of mission 8, put 07 at ff8501 to jump there)
              Magic Sword (fire on floor 3; screen distort after continue)
              Varth (title screen, end of stage 4)
              Captain Commando (end game sequence)

              Tests done on msword at the beginning of gameplay (many thanks to Corrado Tomaselli for these):
              3e  is the default value set by the game (not 0e like most games)
              3c  the last two rows of scroll1 are repeated on the whole screen
              3a  scroll2 is disabled
              36  scroll3 is disabled
              2e  no visible differences
              1e  no visible differences
              one might suspect that bits 4&5 should disable the star layers, but
              Strider sets this register to 0x0e so that's not possible.

              TODO:
              the scroll2/scroll3 disable bits are supported by the emulation,
              while the scroll1 weird effect is not (it doesn't seem to make a
              difference in any game).


CPS-B Registers
---------------
Unlike CPS-A registers, which are at fixed addresses, CPS-B registers move from game to game.
Following example strider

0x66-0x67   Layer control register
            bits 14-15 seem to be unused
                ghouls sets bits 15 in service mode when you press button 2 in
                the input test, with no apparent effect on the pcb.
                qtono2j sets them both at the game over screen.
            bits 6-13 (4 groups of 2 bits) select layer draw order
            bits 1-5 enable the three tilemap layers and the two starfield
                layers (the bit order changes from game to game).
                Only Forgotten Worlds and Strider use the starfield.
            bit 0 could be rowscroll related. It is set by captain commando,
                varth, mtwins, mssword, cawing while rowscroll is active. However
                kodj and sf2 do NOT set this bit while they are using rowscroll.
                Tests on the msword pcb show that even if this bit is not set,
                rowscroll still works. Therefore, the purpose of this bit is unclear.
0x68-0x69   Priority mask \   Tiles in the layer just below sprites can have
0x6a-0x6b   Priority mask |   four priority levels, each one associated with one
0x6c-0x6d   Priority mask |   of these masks. The masks indicate pens in the tile
0x6e-0x6f   Priority mask /   that have priority over sprites.
0x70-0x71   Palette control register. This indicates which palette
            pages to copy when the palette base register is written to.
            There is one CPS2 game (Slammasters II) setting this to 0x2f; all the other
            games normally set it to 0x3f, though in some cases different values are
            used during boot:
            ghouls  0x02 (and palette base is set to 9105; palette base is 9100 normally)
            strider 0x02 (and palette base is set to 9145; palette base is 9140 normally)
            1941    0x02 (and palette base is set to 9145; palette base is 9140 normally)
            unsquad 0x0f
            kod     0x0f
            mtwins  0x0f

            bit 0: copy page 0 (sprites)
            bit 1: copy page 1 (scroll1)
            bit 2: copy page 2 (scroll2)
            bit 3: copy page 3 (scroll3)
            bit 4: copy page 4 (stars1)
            bit 5: copy page 5 (stars2)

            An important quirk is that if the first bits are not set, page 0 in
            gfxram is not skipped but instead is copied to the first enabled page.
            For the other pages, if the bit is not set the gfxram page is skipped.
            Example: 0x0a
            bit 0 is not set so palette page 0 (sprites) is not updated
            bit 1 is set so palette page 1 (scroll1) is updated; since bit 0 was
            not set, it is taken from gfxram page 0
            bit 2 is not set so palette page 2 (scroll2) is not updated; gfxram
            page 1 is skipped
            bit 3 is set so palette page 3 (scroll3) is updated; it is taken from
            gfxram page 2

            bits 0-3 have been verified on a msword pcb, while bits 4-5 are only
            supposed.


A special note has to be made about tile/sprite codes. Even if all graphics are
stored together in the same ROMs, the hardware knows which part of the ROM space
is 8x8 tiles, 16x16 tiles, 16x16 spites, 32x32 tiles, and all games tested only
draw tiles if their code falls in the valid range. If a tile is out of range, it
is replaced by transparent pixels.
Ideally, this shouldn't be important as far as the emulation is concerned, since
games should only request tiles from valid ranges. In practice, many games contain
bugs which make them try to display out of range tiles. The masking applied by
the hardware therefore needs to be emulated properly, otherwise glitches appear.

There are various versions of the ROM board (B-board), so the implementation
details may vary, but in general the tile ranges are controlled by a PAL found
on the B-board (see the table at the top of this file).

The A-board passes 23 bits of address to the B-board when requesting gfx ROM data.
The B-board selects 64 bits of data, that is 16 4bpp pixels, and returns half of
them depending on a signal from the C board.
The 23 address bits are laid out this way (note that the top 3 bits select the
tile type; the purpose of the top bit is unknown):

sprite  000ccccccccccccccccyyyy
scroll1 001?ccccccccccccccccyyy
scroll2 010ccccccccccccccccyyyy
scroll3 011ccccccccccccccyyyyyx
stars   100000000sxxxxxyyyyyyyy (to be verified)

where
c is the tile code
y is the y position in the tile
x is the x position in the tile (only applicable to 32x32 tiles)

E.g. on the 89624B-3 board, used by cawing and others, the top 13 bits of the
address are passed to the 16L8B PAL @1A, which outputs the /OE signal for roms
1,3,5,7 (together), 2,4,6,8 (together), 10,12,14,16,20,22,24,26 (together) or
11,13,15,17,21,23,25,27 (together).
Note that in cawing only sockets 1,3,5,7 are populated, so effectively there is
only one bank.

The above would mean that
1) 8x8 and 16x16 tiles have a 16-bit tile code, while
   32x32 tiles have a 14-bit tile code
2) which ROM bank to use is determined by
   bits 15-7 of a 8x8 tile code,
   bits 15-6 of a 16x16 tile code,
   bits 13-4 of a 32x32 tile code

If the PAL decides that the tile code is out of range and doesn't output any /OE
signal, no ROM is read and pullup resistors force the result to all 1 (which
means a transparent tile).

Note that there are several known cases (nemo, cawing, 3wonders, varth, etc.) where
16x16 tiles are used for BOTH sprites and scroll2.

Ideally, the PALs should be dumped and loaded like the ROMs, and the mapping should
be derived from the loaded PALs at run time.
In practice, this is unlikely to happen, so we'll have to tolerate using hardcoded
maps for every game.


Known Bug List
==============
CPS2:
* CPS2 can do raster effects, certainly used by ssf2 (Cammy, DeeJay, T.Hawk levels),
  msh (lava level, early in attract mode) and maybe others (xmcotaj, vsavj).
  IRQ4 is some sort of scanline interrupt used for that purpose.

* Its unknown what CPS2_OBJ_BASE register (0x400000) does but it is not a object base
  register. The base is 0x7000 for all games even if 0x7080 is written to this register
  (checked on real HW). Maybe it sets the object bank used when cps2_objram_bank is set.

* Sprites are currently lagged by one frame to keep sync with backgrounds. This causes
  sprites to stay on screen one frame longer (visible in VSAV attract mode).

Marvel Vs. Capcom
* Sometimes corrupt gfx are displayed on the 32x32 layer as the screen flashes at the
  start of super combo moves. The problem seems to be due to tiles being fetched before
  the first 32x32 tile offset and results in data coming from 16x16 or 8x8 tiles instead.

CPS1:
SF2
* Missing chain in the foreground in Ken's level, and sign in Chun Li's level.
  Those graphics are in the backmost layer. Probably they are leftover from game
  development and aren't supposed to be visible.

3wonders
* writes to output ports 42, 44, 46.

qad
* layer enable mask incomplete


Unknown issues
==============

There are often some redundant high bits in the scroll layer's attributes.
I think that these are spare bits that the game uses to store additional
information, not used by the hardware.
The games seem to use them to mark platforms, kill zones and no-go areas.

***************************************************************************/

#include "emu.h"
#include "includes/cps1.h"

#define VERBOSE 0

/********************************************************************

            Configuration table:

********************************************************************/

/* Game specific data */

#define GFXTYPE_SPRITES   (1<<0)
#define GFXTYPE_SCROLL1   (1<<1)
#define GFXTYPE_SCROLL2   (1<<2)
#define GFXTYPE_SCROLL3   (1<<3)
#define GFXTYPE_STARS     (1<<4)


#define __not_applicable__  -1,-1,-1,-1,-1,-1,-1

/*                     CPSB ID    multiply protection      unknown      ctrl     priority masks   palctrl    layer enable masks  */
#define CPS_B_01      -1, 0x0000,          __not_applicable__,          0x26,{0x28,0x2a,0x2c,0x2e},0x30, {0x02,0x04,0x08,0x30,0x30}
#define CPS_B_02     0x20,0x0002,          __not_applicable__,          0x2c,{0x2a,0x28,0x26,0x24},0x22, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_03      -1, 0x0000,          __not_applicable__,          0x30,{0x2e,0x2c,0x2a,0x28},0x26, {0x20,0x10,0x08,0x00,0x00}
#define CPS_B_04     0x20,0x0004,          __not_applicable__,          0x2e,{0x26,0x30,0x28,0x32},0x2a, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_05     0x20,0x0005,          __not_applicable__,          0x28,{0x2a,0x2c,0x2e,0x30},0x32, {0x02,0x08,0x20,0x14,0x14}
#define CPS_B_11     0x32,0x0401,          __not_applicable__,          0x26,{0x28,0x2a,0x2c,0x2e},0x30, {0x08,0x10,0x20,0x00,0x00}
#define CPS_B_12     0x20,0x0402,          __not_applicable__,          0x2c,{0x2a,0x28,0x26,0x24},0x22, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_13     0x2e,0x0403,          __not_applicable__,          0x22,{0x24,0x26,0x28,0x2a},0x2c, {0x20,0x02,0x04,0x00,0x00}
#define CPS_B_14     0x1e,0x0404,          __not_applicable__,          0x12,{0x14,0x16,0x18,0x1a},0x1c, {0x08,0x20,0x10,0x00,0x00}
#define CPS_B_15     0x0e,0x0405,          __not_applicable__,          0x02,{0x04,0x06,0x08,0x0a},0x0c, {0x04,0x02,0x20,0x00,0x00}
#define CPS_B_16     0x00,0x0406,          __not_applicable__,          0x0c,{0x0a,0x08,0x06,0x04},0x02, {0x10,0x0a,0x0a,0x00,0x00}
#define CPS_B_17     0x08,0x0407,          __not_applicable__,          0x14,{0x12,0x10,0x0e,0x0c},0x0a, {0x08,0x10,0x02,0x00,0x00}
#define CPS_B_18     0x10,0x0408,          __not_applicable__,          0x1c,{0x1a,0x18,0x16,0x14},0x12, {0x10,0x08,0x02,0x00,0x00}
#define CPS_B_21_DEF 0x32,  -1,   0x00,0x02,0x04,0x06, 0x08, -1,  -1,   0x26,{0x28,0x2a,0x2c,0x2e},0x30, {0x02,0x04,0x08,0x30,0x30} // pang3 sets layer enable to 0x26 on startup
#define CPS_B_21_BT1 0x32,0x0800, 0x0e,0x0c,0x0a,0x08, 0x06,0x04,0x02,  0x28,{0x26,0x24,0x22,0x20},0x30, {0x20,0x04,0x08,0x12,0x12}
#define CPS_B_21_BT2  -1,   -1,   0x1e,0x1c,0x1a,0x18,  -1, 0x0c,0x0a,  0x20,{0x2e,0x2c,0x2a,0x28},0x30, {0x30,0x08,0x30,0x00,0x00}
#define CPS_B_21_BT3  -1,   -1,   0x06,0x04,0x02,0x00, 0x0e,0x0c,0x0a,  0x20,{0x2e,0x2c,0x2a,0x28},0x30, {0x20,0x12,0x12,0x00,0x00}
#define CPS_B_21_BT4  -1,   -1,   0x06,0x04,0x02,0x00, 0x1e,0x1c,0x1a,  0x28,{0x26,0x24,0x22,0x20},0x30, {0x20,0x10,0x02,0x00,0x00}
#define CPS_B_21_BT5 0x32,  -1,   0x0e,0x0c,0x0a,0x08, 0x1e,0x1c,0x1a,  0x20,{0x2e,0x2c,0x2a,0x28},0x30, {0x20,0x04,0x02,0x00,0x00}
#define CPS_B_21_BT6  -1,   -1,    -1,  -1,  -1,  -1,   -1,  -1,  -1,   0x20,{0x2e,0x2c,0x2a,0x28},0x30, {0x20,0x14,0x14,0x00,0x00}
#define CPS_B_21_BT7  -1,   -1,    -1,  -1,  -1,  -1,   -1,  -1,  -1,   0x2c,{ -1,  -1,  -1,  -1 },0x12, {0x14,0x02,0x14,0x00,0x00}
#define CPS_B_21_QS1  -1,   -1,    -1,  -1,  -1,  -1,   -1,  -1,  -1,   0x22,{0x24,0x26,0x28,0x2a},0x2c, {0x10,0x08,0x04,0x00,0x00}
#define CPS_B_21_QS2  -1,   -1,    -1,  -1,  -1,  -1,   -1, 0x2e,0x20,  0x0a,{0x0c,0x0e,0x00,0x02},0x04, {0x16,0x16,0x16,0x00,0x00}
#define CPS_B_21_QS3 0x0e,0x0c00,  -1,  -1,  -1,  -1,  0x2c, -1,  -1,   0x12,{0x14,0x16,0x08,0x0a},0x0c, {0x04,0x02,0x20,0x00,0x00}
#define CPS_B_21_QS4 0x2e,0x0c01,  -1,  -1,  -1,  -1,  0x1c,0x1e,0x08,  0x16,{0x00,0x02,0x28,0x2a},0x2c, {0x04,0x08,0x10,0x00,0x00}
#define CPS_B_21_QS5 0x1e,0x0c02,  -1,  -1,  -1,  -1,  0x0c, -1,  -1,   0x2a,{0x2c,0x2e,0x30,0x32},0x1c, {0x04,0x08,0x10,0x00,0x00}
#define HACK_B_1      -1,   -1,    -1,  -1,  -1,  -1,   -1,  -1,  -1,   0x14,{0x12,0x10,0x0e,0x0c},0x0a, {0x0e,0x0e,0x0e,0x30,0x30}
#define HACK_B_2      -1,   -1,   0x0e,0x0c,0x0a,0x08, 0x06,0x04,0x02,  0x28,{0x26,0x24,0x22,0x20},0x22, {0x20,0x04,0x08,0x12,0x12}

/*
CPS_B_21_DEF is CPS-B-21 at default settings (no battery)
CPS_B_21_BTx are various battery configurations
CPS_B_21_QSx are various battery configurations in Q-Sound games
*/


// LWCHR and LW621 are equivalent as far as the game is concerned, though the
// equations are different

#define mapper_LWCHR    { 0x8000, 0x8000, 0, 0 }, mapper_LWCHR_table
static const struct gfx_range mapper_LWCHR_table[] =
{
	// verified from PAL dump (PAL16P8B @ 3A):
	// bank 0 = pin 19 (ROMs 1,5,8,12)
	// bank 1 = pin 16 (ROMs 2,6,9,13)
	// pin 12 and pin 14 are always enabled (except for stars)
	// note that allowed codes go up to 0x1ffff but physical ROM is half that size

	/* type            start    end      bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SCROLL1, 0x00000, 0x1ffff, 0 },

	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL2, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ 0 }
};

#define mapper_LW621    { 0x8000, 0x8000, 0, 0 }, mapper_LW621_table
static const struct gfx_range mapper_LW621_table[] =
{
	// verified from PAL dump (PAL @ 1A):
	// bank 0 = pin 18
	// bank 1 = pin 14
	// pins 19, 16, 17, and 12 give an alternate half-size mapping which would
	// allow to use smaller ROMs:
	// pin 19
	// 0 00000-03fff
	// pin 16
	// 0 04000-07fff
	// 1 00000-1ffff
	// pin 17
	// 2 00000-1ffff
	// 3 00000-1ffff
	// 4 00000-1ffff
	// pin 12
	// 3 00000-1ffff
	//
	// note that allowed codes go up to 0x1ffff but physical ROM is half that size

	/* type            start    end      bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SCROLL1, 0x00000, 0x1ffff, 0 },

	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL2, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ 0 }
};


// DM620, DM22A and DAM63B are equivalent as far as the game is concerned, though
// the equations are quite different

#define mapper_DM620    { 0x8000, 0x2000, 0x2000, 0 }, mapper_DM620_table
static const struct gfx_range mapper_DM620_table[] =
{
	// verified from PAL dump (PAL16P8B @ 2A):
	// bank 0 = pin 19 (ROMs  5,6,7,8)
	// bank 1 = pin 16 (ROMs  9,11,13,15,18,20,22,24)
	// bank 2 = pin 14 (ROMs 10,12,14,16,19,21,23,25)
	// pin 12 is never enabled
	// note that bank 0 is enabled whenever banks 1 or 2 are not enabled,
	// which would make it highly redundant, so I'm relying on the table
	// to be scanned top to bottom and using a catch-all clause at the end.

	/* type            start   end     bank */
	{ GFXTYPE_SCROLL3, 0x8000, 0xbfff, 1 },

	{ GFXTYPE_SPRITES, 0x2000, 0x3fff, 2 },

	{ GFXTYPE_STARS | GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 0 },
	{ 0 }
};

#define mapper_DM22A    { 0x4000, 0x4000, 0x2000, 0x2000 }, mapper_DM22A_table
static const struct gfx_range mapper_DM22A_table[] =
{
	// verified from PAL dump
	// bank 0 = pin 19
	// bank 1 = pin 16
	// bank 2 = pin 14
	// bank 3 = pin 12

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x01fff, 0 },
	{ GFXTYPE_SCROLL1, 0x02000, 0x03fff, 0 },

	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 1 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 2 },

	{ GFXTYPE_SPRITES, 0x02000, 0x03fff, 3 },
	{ 0 }
};

#define mapper_DAM63B   { 0x8000, 0x8000, 0, 0 }, mapper_DAM63B_table
static const struct gfx_range mapper_DAM63B_table[] =
{
	// verified from PAL dump:
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// pins 12,13,14,15 are always enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x01fff, 0 },
	{ GFXTYPE_SCROLL1, 0x02000, 0x02fff, 0 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 0 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },
	{ GFXTYPE_SPRITES, 0x02000, 0x03fff, 1 },
	{ 0 }
};


// ST24M1 and ST22B are equivalent except for the stars range which is
// different. This has no practical effect.

#define mapper_ST24M1   { 0x8000, 0x8000, 0, 0 }, mapper_ST24M1_table
static const struct gfx_range mapper_ST24M1_table[] =
{
	// verified from PAL dump
	// bank 0 = pin 19 (ROMs 2,4,6,8)
	// bank 1 = pin 16 (ROMs 1,3,5,7)
	// pin 12 and pin 14 are never enabled

	/* type            start    end      bank */
	{ GFXTYPE_STARS,   0x00000, 0x003ff, 0 },
	{ GFXTYPE_SPRITES, 0x00000, 0x04fff, 0 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 0 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x07fff, 1 },
	{ GFXTYPE_SCROLL1, 0x07000, 0x07fff, 1 },
	{ 0 }
};

#define mapper_ST22B    { 0x4000, 0x4000, 0x4000, 0x4000 }, mapper_ST22B_table
static const struct gfx_range mapper_ST22B_table[] =
{
	// verified from PAL dump
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// bank 2 = pin 14 (ROMs 3,7,11,15,19,21,26,28)
	// bank 3 = pin 12 (ROMS 4,8,12,16,20,22,27,29)

	/* type            start    end      bank */
	{ GFXTYPE_STARS,   0x00000, 0x1ffff, 0 },
	{ GFXTYPE_SPRITES, 0x00000, 0x03fff, 0 },

	{ GFXTYPE_SPRITES, 0x04000, 0x04fff, 1 },
	{ GFXTYPE_SCROLL2, 0x04000, 0x07fff, 1 },

	{ GFXTYPE_SCROLL3, 0x00000, 0x03fff, 2 },

	{ GFXTYPE_SCROLL3, 0x04000, 0x07fff, 3 },
	{ GFXTYPE_SCROLL1, 0x07000, 0x07fff, 3 },
	{ 0 }
};


#define mapper_TK22B    { 0x4000, 0x4000, 0x4000, 0x4000 }, mapper_TK22B_table
static const struct gfx_range mapper_TK22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// bank 2 = pin 14 (ROMs 3,7,11,15,19,21,26,28)
	// bank 3 = pin 12 (ROMS 4,8,12,16,20,22,27,29)

	/* type            start  end      bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },

	{ GFXTYPE_SPRITES, 0x4000, 0x5fff, 1 },
	{ GFXTYPE_SCROLL1, 0x6000, 0x7fff, 1 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 2 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x7fff, 3 },
	{ 0 }
};


#define mapper_WL24B    { 0x8000, 0x8000, 0, 0 }, mapper_WL24B_table
static const struct gfx_range mapper_WL24B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// bank 1 = pin 12 (ROMs 10,12,14,16,20,22,24,26)
	// pin 14 and pin 19 are never enabled

	/* type            start  end      bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL3, 0x5000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL1, 0x7000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 1 },
	{ 0 }
};


#define mapper_S224B    { 0x8000, 0, 0, 0 }, mapper_S224B_table
static const struct gfx_range mapper_S224B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 12 & pin 14 give an alternate half-size mapping which would allow to
	// populate the 8-bit ROM sockets instead of the 16-bit ones:
	// pin 12
	// 0 00000 - 03fff
	// pin 14
	// 0 04000 - 043ff
	// 1 04400 - 04bff
	// 2 06000 - 07fff
	// 3 04c00 - 05fff
	// pin 19 is never enabled

	/* type            start  end      bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x43ff, 0 },
	{ GFXTYPE_SCROLL1, 0x4400, 0x4bff, 0 },
	{ GFXTYPE_SCROLL3, 0x4c00, 0x5fff, 0 },
	{ GFXTYPE_SCROLL2, 0x6000, 0x7fff, 0 },
	{ 0 }
};


#define mapper_YI24B    { 0x8000, 0, 0, 0 }, mapper_YI24B_table
static const struct gfx_range mapper_YI24B_table[] =
{
	// verified from JED:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 12 & pin 14 give an alternate half-size mapping which would allow to
	// populate the 8-bit ROM sockets instead of the 16-bit ones:
	// pin 12
	// 0 0000-1fff
	// 3 2000-3fff
	// pin 14
	// 1 4000-47ff
	// 2 4800-7fff
	// pin 19 is never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x1fff, 0 },
	{ GFXTYPE_SCROLL3, 0x2000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x47ff, 0 },
	{ GFXTYPE_SCROLL2, 0x4800, 0x7fff, 0 },
	{ 0 }
};


// AR24B and AR22B are equivalent, but since we could dump both PALs we are
// documenting both.

#define mapper_AR24B    { 0x8000, 0, 0, 0 }, mapper_AR24B_table
static const struct gfx_range mapper_AR24B_table[] =
{
	// verified from JED:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 12 & pin 14 give an alternate half-size mapping which would allow to
	// populate the 8-bit ROM sockets instead of the 16-bit ones:
	// pin 12
	// 0 0000-2fff
	// 1 3000-3fff
	// pin 14
	// 2 4000-5fff
	// 3 6000-7fff
	// pin 19 is never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x4000, 0x5fff, 0 },
	{ GFXTYPE_SCROLL3, 0x6000, 0x7fff, 0 },
	{ 0 }
};

#define mapper_AR22B    { 0x4000, 0x4000, 0, 0 }, mapper_AR22B_table
static const struct gfx_range mapper_AR22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// pins 12 and 14 are tristated

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x5fff, 1 },
	{ GFXTYPE_SCROLL3, 0x6000, 0x7fff, 1 },
	{ 0 }
};


#define mapper_O224B    { 0x8000, 0x4000, 0, 0 }, mapper_O224B_table
static const struct gfx_range mapper_O224B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 2,4,6,8)
	// bank 1 = pin 12 (ROMs 10,12,14,16,20,22,24,26)
	// pin 16 & pin 14 appear to be an alternate half-size mapping for bank 0
	// but scroll1 is missing:
	// pin 16
	// 2 00c00 - 03bff
	// 3 03c00 - 03fff
	// pin 14
	// 3 04000 - 04bff
	// 0 04c00 - 07fff

	/* type            start   end     bank */
	{ GFXTYPE_SCROLL1, 0x0000, 0x0bff, 0 },
	{ GFXTYPE_SCROLL2, 0x0c00, 0x3bff, 0 },
	{ GFXTYPE_SCROLL3, 0x3c00, 0x4bff, 0 },
	{ GFXTYPE_SPRITES, 0x4c00, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0xa7ff, 1 },
	{ GFXTYPE_SCROLL2, 0xa800, 0xb7ff, 1 },
	{ GFXTYPE_SCROLL3, 0xb800, 0xbfff, 1 },
	{ 0 }
};


#define mapper_MS24B    { 0x8000, 0, 0, 0 }, mapper_MS24B_table
static const struct gfx_range mapper_MS24B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 14 duplicates pin 16 allowing to populate the 8-bit ROM sockets
	// instead of the 16-bit ones.
	// pin 12 is enabled only for sprites:
	// 0 0000-3fff
	// pin 19 is never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL2, 0x5000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },
	{ 0 }
};


#define mapper_CK24B    { 0x8000, 0, 0, 0 }, mapper_CK24B_table
static const struct gfx_range mapper_CK24B_table[] =
{
	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL1, 0x3000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x4000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },
	{ 0 }
};


#define mapper_NM24B    { 0x8000, 0, 0, 0 }, mapper_NM24B_table
static const struct gfx_range mapper_NM24B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 12 & pin 14 give an alternate half-size mapping which would allow to
	// populate the 8-bit ROM sockets instead of the 16-bit ones:
	// pin 12
	// 0 00000 - 03fff
	// 2 00000 - 03fff
	// pin 14
	// 1 04000 - 047ff
	// 0 04800 - 067ff
	// 2 04800 - 067ff
	// 3 06800 - 07fff
	// pin 19 is never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x4000, 0x47ff, 0 },
	{ GFXTYPE_SPRITES, 0x4800, 0x67ff, 0 },
	{ GFXTYPE_SCROLL2, 0x4800, 0x67ff, 0 },
	{ GFXTYPE_SCROLL3, 0x6800, 0x7fff, 0 },
	{ 0 }
};


// CA24B and CA22B are equivalent, but since we could dump both PALs we are
// documenting both.

#define mapper_CA24B    { 0x8000, 0, 0, 0 }, mapper_CA24B_table
static const struct gfx_range mapper_CA24B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// pin 12 & pin 14 give an alternate half-size mapping which would allow to
	// populate the 8-bit ROM sockets instead of the 16-bit ones:
	// pin 12
	// 0 0000-2fff
	// 2 0000-2fff
	// 3 3000-3fff
	// pin 14
	// 3 4000-4fff
	// 1 5000-57ff
	// 0 5800-7fff
	// 2 5800-7fff
	// pin 19 is never enabled (actually it is always enabled when PAL pin 1 is 1, purpose unknown)

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL3, 0x3000, 0x4fff, 0 },
	{ GFXTYPE_SCROLL1, 0x5000, 0x57ff, 0 },
	{ GFXTYPE_SPRITES, 0x5800, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x5800, 0x7fff, 0 },
	{ 0 }
};

#define mapper_CA22B    { 0x4000, 0x4000, 0, 0 }, mapper_CA22B_table
static const struct gfx_range mapper_CA22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// pin 12 and pin 14 are never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x2fff, 0 },
	{ GFXTYPE_SCROLL3, 0x3000, 0x3fff, 0 },

	{ GFXTYPE_SCROLL3, 0x4000, 0x4fff, 1 },
	{ GFXTYPE_SCROLL1, 0x5000, 0x57ff, 1 },
	{ GFXTYPE_SPRITES, 0x5800, 0x7fff, 1 },
	{ GFXTYPE_SCROLL2, 0x5800, 0x7fff, 1 },
	{ 0 }
};


#define mapper_STF29    { 0x8000, 0x8000, 0x8000, 0 }, mapper_STF29_table
static const struct gfx_range mapper_STF29_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 5,6,7,8)
	// bank 1 = pin 14 (ROMs 14,15,16,17)
	// bank 2 = pin 12 (ROMS 24,25,26,27)

	/* type            start    end      bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES, 0x10000, 0x11fff, 2 },
	{ GFXTYPE_SCROLL3, 0x02000, 0x03fff, 2 },
	{ GFXTYPE_SCROLL1, 0x04000, 0x04fff, 2 },
	{ GFXTYPE_SCROLL2, 0x05000, 0x07fff, 2 },
	{ 0 }
};


// RT24B and RT22B are equivalent, but since we could dump both PALs we are
// documenting both.

#define mapper_RT24B    { 0x8000, 0x8000, 0, 0 }, mapper_RT24B_table
static const struct gfx_range mapper_RT24B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 16 (ROMs 1,3,5,7)
	// bank 1 = pin 19 (ROMs 2,4,6,8)
	// pin 12 & pin 14 are never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x53ff, 0 },
	{ GFXTYPE_SCROLL1, 0x5400, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 1 },
	{ GFXTYPE_SCROLL2, 0x2800, 0x7fff, 1 },
	{ GFXTYPE_SPRITES, 0x5400, 0x7fff, 1 },
	{ 0 }
};

#define mapper_RT22B    { 0x4000, 0x4000, 0x4000, 0x4000 }, mapper_RT22B_table
static const struct gfx_range mapper_RT22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// bank 2 = pin 14 (ROMs 3,7,11,15,19,21,26,28)
	// bank 3 = pin 12 (ROMS 4,8,12,16,20,22,27,29)

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },

	{ GFXTYPE_SPRITES, 0x4000, 0x53ff, 1 },
	{ GFXTYPE_SCROLL1, 0x5400, 0x6fff, 1 },
	{ GFXTYPE_SCROLL3, 0x7000, 0x7fff, 1 },

	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 2 },
	{ GFXTYPE_SCROLL2, 0x2800, 0x3fff, 2 },

	{ GFXTYPE_SCROLL2, 0x4000, 0x7fff, 3 },
	{ GFXTYPE_SPRITES, 0x5400, 0x7fff, 3 },
	{ 0 }
};


#define mapper_KD29B    { 0x8000, 0x8000, 0, 0 }, mapper_KD29B_table
static const struct gfx_range mapper_KD29B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,2,3,4)
	// bank 1 = pin 14 (ROMs 10,11,12,13)
	// pin 12 is never enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0x8fff, 1 },
	{ GFXTYPE_SCROLL2, 0x9000, 0xbfff, 1 },
	{ GFXTYPE_SCROLL1, 0xc000, 0xd7ff, 1 },
	{ GFXTYPE_SCROLL3, 0xd800, 0xffff, 1 },
	{ 0 }
};


#define mapper_CC63B    { 0x8000, 0x8000, 0, 0 }, mapper_CC63B_table
static const struct gfx_range mapper_CC63B_table[] =
{
	// verified from PAL dump:
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// pins 12,13,14,15 are always enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL1, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL2, 0x8000, 0xffff, 1 },
	{ GFXTYPE_SCROLL3, 0x8000, 0xffff, 1 },
	{ 0 }
};


#define mapper_KR63B    { 0x8000, 0x8000, 0, 0 }, mapper_KR63B_table
static const struct gfx_range mapper_KR63B_table[] =
{
	// verified from PAL dump:
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// pins 12,13,14,15 are always enabled

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x7fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SCROLL1, 0x8000, 0x9fff, 1 },
	{ GFXTYPE_SPRITES, 0x8000, 0xcfff, 1 },
	{ GFXTYPE_SCROLL2, 0x8000, 0xcfff, 1 },
	{ GFXTYPE_SCROLL3, 0xd000, 0xffff, 1 },
	{ 0 }
};


#define mapper_S9263B   { 0x8000, 0x8000, 0x8000, 0 }, mapper_S9263B_table
static const struct gfx_range mapper_S9263B_table[] =
{
	// verified from PAL dump:
	// FIXME there is some problem with this dump since pin 14 is never enabled
	// instead of being the same as pin 15 as expected
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// bank2 = pin 15 (ROMs 10,12) & pin 14 (ROMs 11,13)
	// pins 12 and 13 are the same as 14 and 15

	/* type            start    end      bank */
	{ GFXTYPE_SPRITES, 0x00000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES, 0x10000, 0x11fff, 2 },
	{ GFXTYPE_SCROLL3, 0x02000, 0x03fff, 2 },
	{ GFXTYPE_SCROLL1, 0x04000, 0x04fff, 2 },
	{ GFXTYPE_SCROLL2, 0x05000, 0x07fff, 2 },
	{ 0 }
};


// VA22B and VA63B are equivalent, but since we could dump both PALs we are
// documenting both.

#define mapper_VA22B    { 0x4000, 0x4000, 0, 0 }, mapper_VA22B_table
static const struct gfx_range mapper_VA22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19 (ROMs 1,5, 9,13,17,24,32,38)
	// bank 1 = pin 16 (ROMs 2,6,10,14,18,25,33,39)
	// pin 12 and pin 14 are never enabled

	/* type                                                                  start    end      bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x03fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x04000, 0x07fff, 1 },
	{ 0 }
};

#define mapper_VA63B    { 0x8000, 0, 0, 0 }, mapper_VA63B_table
static const struct gfx_range mapper_VA63B_table[] =
{
	// verified from PAL dump (PAL # uncertain):
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// pins 12,13,14,15,16,17 are never enabled

	/* type                                                                  start    end      bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
	{ 0 }
};


#define mapper_Q522B    { 0x8000, 0, 0, 0 }, mapper_Q522B_table
static const struct gfx_range mapper_Q522B_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x0000, 0x6fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x7000, 0x77ff, 0 },
	{ GFXTYPE_SCROLL1,                   0x7800, 0x7fff, 0 },
	{ 0 }
};


#define mapper_TK263B   { 0x8000, 0x8000, 0, 0 }, mapper_TK263B_table
static const struct gfx_range mapper_TK263B_table[] =
{
	// verified from PAL dump:
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// pins 12,13,14,15 are always enabled

	/* type                                                                  start    end      bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x08000, 0x0ffff, 1 },
	{ 0 }
};


#define mapper_CD63B    { 0x8000, 0x8000, 0, 0 }, mapper_CD63B_table
static const struct gfx_range mapper_CD63B_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES,                   0x1000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xdfff, 1 },
	{ GFXTYPE_SCROLL3,                   0xe000, 0xffff, 1 },
	{ 0 }
};


#define mapper_PS63B    { 0x8000, 0x8000, 0, 0 }, mapper_PS63B_table
static const struct gfx_range mapper_PS63B_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES,                   0x1000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xdbff, 1 },
	{ GFXTYPE_SCROLL3,                   0xdc00, 0xffff, 1 },
	{ 0 }
};


#define mapper_MB63B    { 0x8000, 0x8000, 0x8000, 0 }, mapper_MB63B_table
static const struct gfx_range mapper_MB63B_table[] =
{
	/* type                              start    end      bank */
	{ GFXTYPE_SCROLL1,                   0x00000, 0x00fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x01000, 0x07fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x08000, 0x0ffff, 1 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x10000, 0x167ff, 2 },
	{ GFXTYPE_SCROLL3,                   0x16800, 0x17fff, 2 },
	{ 0 }
};


#define mapper_QD22B    { 0x4000, 0, 0, 0 }, mapper_QD22B_table
static const struct gfx_range mapper_QD22B_table[] =
{
	// verified from PAL dump:
	// bank 0 = pin 19

	/* type            start   end     bank */
	{ GFXTYPE_SPRITES, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL1, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL2, 0x0000, 0x3fff, 0 },
	{ GFXTYPE_SCROLL3, 0x0000, 0x3fff, 0 },
	{ 0 }
};


#define mapper_QAD63B   { 0x8000, 0, 0, 0 }, mapper_QAD63B_table
static const struct gfx_range mapper_QAD63B_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SCROLL1,                   0x0000, 0x07ff, 0 },
	{ GFXTYPE_SCROLL3,                   0x0800, 0x1fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x2000, 0x7fff, 0 },
	{ 0 }
};


#define mapper_TN2292   { 0x8000, 0x8000, 0, 0 }, mapper_TN2292_table
static const struct gfx_range mapper_TN2292_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x1000, 0x3fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x4000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0xffff, 1 },
	{ 0 }
};


#define mapper_RCM63B   { 0x8000, 0x8000, 0x8000, 0x8000 }, mapper_RCM63B_table
static const struct gfx_range mapper_RCM63B_table[] =
{
	// verified from PAL dump:
	// bank0 = pin 19 (ROMs 1,3) & pin 18 (ROMs 2,4)
	// bank1 = pin 17 (ROMs 5,7) & pin 16 (ROMs 6,8)
	// bank0 = pin 15 (ROMs 10,12) & pin 14 (ROMs 11,13)
	// bank1 = pin 13 (ROMs 14,16) & pin 12 (ROMs 15,17)

	/* type                                                                  start    end      bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x08000, 0x0ffff, 1 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x10000, 0x17fff, 2 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x18000, 0x1ffff, 3 },
	{ 0 }
};


#define mapper_PKB10B   { 0x8000, 0, 0, 0 }, mapper_PKB10B_table
static const struct gfx_range mapper_PKB10B_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SCROLL1,                   0x0000, 0x0fff, 0 },
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x1000, 0x5fff, 0 },
	{ GFXTYPE_SCROLL3,                   0x6000, 0x7fff, 0 },
	{ 0 }
};


#define mapper_pang3    { 0x8000, 0x8000, 0, 0 }, mapper_pang3_table
static const struct gfx_range mapper_pang3_table[] =
{
	/* type                              start   end     bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x0000, 0x7fff, 0 },

	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0x8000, 0x9fff, 1 },
	{ GFXTYPE_SCROLL1,                   0xa000, 0xbfff, 1 },
	{ GFXTYPE_SCROLL3,                   0xc000, 0xffff, 1 },
	{ 0 }
};


#define mapper_sfzch    { 0x20000, 0, 0, 0 }, mapper_sfzch_table
static const struct gfx_range mapper_sfzch_table[] =
{
	/* type                                                                  start    end      bank */
	{ GFXTYPE_SPRITES | GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 0 },
	{ 0 }
};


/*
  I don't know if CPS2 ROM boards use PALs as well; since all games seem to be
  well behaved, I'll just assume that there is no strong checking of gfx type.
  (sprites are not listed here because they are addressed linearly by the CPS2
  sprite code)
 */
#define mapper_cps2 { 0x20000, 0x20000, 0, 0 }, mapper_cps2_table
static const struct gfx_range mapper_cps2_table[] =
{
	/* type                                                start    end      bank */
	{ GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 0x00000, 0x1ffff, 1 },   // 20000-3ffff physical
	{ 0 }
};



/*
Name     knm10b;
PartNo   ;
Date     ;
Revision ;
Designer ;
Company  ;
Assembly ;
Location ;
Device   g16v8;

 Dedicated input pins

pin 1   = I0;  Input
pin 2   = I1;  Input
pin 3   = I2;  Input
pin 4   = I3;  Input
pin 5   = I4;  Input
pin 6   = I5;  Input
pin 7   = I6;  Input
pin 8   = I7;  Input
pin 9   = I8;  Input
pin 11  = I9;  Input

 Programmable output pins

pin 12  = B0;  Combinatorial output
pin 13  = B1;  Combinatorial output
pin 14  = B2;  Combinatorial output
pin 15  = B3;  Combinatorial output
pin 16  = B4;  Combinatorial output
pin 17  = B5;  Combinatorial output
pin 18  = B6;  Combinatorial output
pin 19  = B7;  Combinatorial output

 Output equations

!B7 = !I0 & !I1 & !I2 & !I3 & !I4 & !I5 & !I9
    #  I0 & !I1 & !I2 & !I3 & !I4 & !I5 &  I9;
!B6 = !I0 & !I1 & !I2 & !I3 & !I4 & !I5 & !I9
    #  I0 & !I1 & !I2 & !I3 & !I4 & !I5 &  I9;
!B5 = !I0 & !I1 & !I2 & !I3 & !I4 &  I5 & !I9
    #  I0 & !I1 & !I2 & !I3 & !I4 &  I5 &  I9;
!B4 = !I0 & !I1 & !I2 & !I3 & !I4 &  I5 & !I9
    #  I0 & !I1 & !I2 & !I3 & !I4 &  I5 &  I9;
!B3 = !I0 & !I1 & !I2 & !I3 &  I4 & !I5 & !I6 & !I7 & !I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 & !I8 & !I9
    # !I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 & !I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 & !I8 & !I9
    # !I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 &  I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 &  I8 & !I9
    # !I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 &  I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 &  I8 & !I9
    #  I0 & !I1 & !I2 & !I3 &  I4 & !I5 & !I6 & !I7 & !I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 & !I8 &  I9
    #  I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 & !I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 & !I8 &  I9
    #  I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 &  I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 &  I8 &  I9
    #  I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 &  I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 &  I8 &  I9;
!B2 = !I0 & !I1 & !I2 & !I3 &  I4 & !I5 & !I6 & !I7 & !I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 & !I8 & !I9
    # !I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 & !I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 & !I8 & !I9
    # !I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 &  I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 &  I8 & !I9
    # !I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 &  I8 & !I9
    # !I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 &  I8 & !I9
    #  I0 & !I1 & !I2 & !I3 &  I4 & !I5 & !I6 & !I7 & !I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 & !I8 &  I9
    #  I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 & !I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 & !I8 &  I9
    #  I0 & !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 &  I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 &  I8 &  I9
    #  I0 & !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 &  I8 &  I9
    #  I0 & !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 &  I8 &  I9;
!B1 = !I1 & !I2 & !I3 &  I4 & !I5 & !I6 & !I7 & !I8
    # !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 & !I8
    # !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 & !I8
    # !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 & !I8
    # !I1 & !I2 &  I3 & !I4 & !I5 & !I6 & !I7 &  I8
    # !I1 &  I2 & !I3 & !I4 & !I5 &  I6 & !I7 &  I8
    # !I1 &  I2 &  I3 & !I4 & !I5 & !I6 &  I7 &  I8
    # !I1 &  I2 & !I3 & !I4 & !I5 &  I6 &  I7 &  I8;
!B0 =  I0 &  I9;

*/
// wrong, need to figure this out from the PAL

#define mapper_KNM10B    { 0x8000, 0x8000, 0x8000, 0 }, mapper_KNM10B_table
static const struct gfx_range mapper_KNM10B_table[] =
{
	/* type             start    end      bank */

	{ GFXTYPE_SPRITES , 0x00000, 0x07fff, 0 },
	{ GFXTYPE_SPRITES , 0x08000, 0x0ffff, 1 },
	{ GFXTYPE_SPRITES , 0x10000, 0x17fff, 2 },
	{ GFXTYPE_SCROLL2 , 0x04000, 0x07fff, 2 },
	{ GFXTYPE_SCROLL1,  0x01000, 0x01fff, 2 },
	{ GFXTYPE_SCROLL3 , 0x02000, 0x03fff, 2 },
	{ 0 }
};



static const struct CPS1config cps1_config_table[]=
{
	/* name         CPSB          gfx mapper   in2  in3  out2   kludge */
	{"forgottn",    CPS_B_01,     mapper_LW621 },
	{"forgottna",   CPS_B_01,     mapper_LW621 },
	{"forgottnu",   CPS_B_01,     mapper_LW621 },
	{"forgottnu1",  CPS_B_01,     mapper_LWCHR },
	{"forgottnua",  CPS_B_01,     mapper_LWCHR },
	{"forgottnuaa", CPS_B_01,     mapper_LWCHR },
	{"lostwrld",    CPS_B_01,     mapper_LWCHR },
	{"lostwrldo",   CPS_B_01,     mapper_LWCHR },
	{"ghouls",      CPS_B_01,     mapper_DM620 },
	{"ghoulsu",     CPS_B_01,     mapper_DM620 },
	{"daimakai",    CPS_B_01,     mapper_DM22A },   // equivalent to DM620
	{"daimakair",   CPS_B_21_DEF, mapper_DAM63B },  // equivalent to DM620, also CPS_B_21_DEF is equivalent to CPS_B_01
	{"strider",     CPS_B_01,     mapper_ST24M1 },
	{"striderua",   CPS_B_01,     mapper_ST24M1 },  // wrong, this set uses ST24B2, still not dumped
	{"striderj",    CPS_B_01,     mapper_ST22B },   // equivalent to ST24M1
	{"striderjr",   CPS_B_21_DEF, mapper_ST24M1 },  // wrong, this set uses STH63B, still not dumped
	{"dynwar",      CPS_B_02,     mapper_TK22B },   // wrong, this set uses TK24B1, dumped but equations still not added
	{"dynwara",     CPS_B_02,     mapper_TK22B },
	{"dynwarj",     CPS_B_02,     mapper_TK22B },
	{"dynwarjr",    CPS_B_21_DEF, mapper_TK22B },   // wrong, this set uses TK163B, still not dumped
	{"willow",      CPS_B_03,     mapper_WL24B },
	{"willowu",     CPS_B_03,     mapper_WL24B },
	{"willowuo",    CPS_B_03,     mapper_WL24B },
	{"willowj",     CPS_B_03,     mapper_WL24B },   // wrong, this set uses WL22B, still not dumped
	{"ffight",      CPS_B_04,     mapper_S224B },
	{"ffighta",     CPS_B_04,     mapper_S224B },
	{"ffightu",     CPS_B_04,     mapper_S224B },
	{"ffightu1",    CPS_B_04,     mapper_S224B },
	{"ffightua",    CPS_B_01,     mapper_S224B },
	{"ffightub",    CPS_B_03,     mapper_S224B },   // had 04 handwritten on the CPS_B chip, but clearly isn't.
	{"ffightuc",    CPS_B_05,     mapper_S224B },
	{"ffightj",     CPS_B_04,     mapper_S224B },   // wrong, this set uses S222B
	{"ffightj1",    CPS_B_01,     mapper_S224B },   // wrong, this set uses S222B
	{"ffightj2",    CPS_B_02,     mapper_S224B },   // wrong, this set uses S222B
	{"ffightj3",    CPS_B_05,     mapper_S224B },   // wrong, this set uses S222B
	{"ffightjh",    CPS_B_01,     mapper_S224B },   // wrong, ffightjh hack doesn't even use the S222B PAL, since replaced with a GAL.
	{"1941",        CPS_B_05,     mapper_YI24B },
	{"1941r1",      CPS_B_05,     mapper_YI24B },
	{"1941u",       CPS_B_05,     mapper_YI24B },
	{"1941j",       CPS_B_05,     mapper_YI24B },   // wrong, this set uses YI22B, still not dumped
	{"unsquad",     CPS_B_11,     mapper_AR24B },
	{"area88",      CPS_B_11,     mapper_AR22B },   // equivalent to AR24B
	{"area88r",     CPS_B_21_DEF, mapper_AR22B },   // wrong, this set uses ARA63B, still not dumped
	{"mercs",       CPS_B_12,     mapper_O224B,  0x36, 0, 0x34 },
	{"mercsu",      CPS_B_12,     mapper_O224B,  0x36, 0, 0x34 },
	{"mercsur1",    CPS_B_12,     mapper_O224B,  0x36, 0, 0x34 },
	{"mercsj",      CPS_B_12,     mapper_O224B,  0x36, 0, 0x34 },   // wrong, this set uses O222B, still not dumped
	{"msword",      CPS_B_13,     mapper_MS24B },
	{"mswordr1",    CPS_B_13,     mapper_MS24B },
	{"mswordu",     CPS_B_13,     mapper_MS24B },
	{"mswordj",     CPS_B_13,     mapper_MS24B },   // wrong, this set uses MS22B, dumped but equations still not added
	{"mtwins",      CPS_B_14,     mapper_CK24B },
	{"chikij",      CPS_B_14,     mapper_CK24B },   // wrong, this set uses CK22B, dumped but equations still not added
	{"nemo",        CPS_B_15,     mapper_NM24B },
	{"nemor1",      CPS_B_15,     mapper_NM24B },
	{"nemoj",       CPS_B_15,     mapper_NM24B },   // wrong, this set uses NM22B, still not dumped
	{"cawing",      CPS_B_16,     mapper_CA24B },
	{"cawingr1",    CPS_B_16,     mapper_CA24B },
	{"cawingu",     CPS_B_16,     mapper_CA24B },
	{"cawingj",     CPS_B_16,     mapper_CA22B },   // equivalent to CA24B
	{"cawingbl",    CPS_B_16,     mapper_CA22B },   // equivalent to CA24B
	{"sf2",         CPS_B_11,     mapper_STF29,  0x36 },
	{"sf2eb",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2ed",       CPS_B_05,     mapper_STF29,  0x36 },
	{"sf2ee",       CPS_B_18,     mapper_STF29,  0x3c },
	{"sf2ebbl",     CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"sf2ebbl2",    CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"sf2ebbl3",    CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"sf2stt",      CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"sf2rk",       CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"sf2ua",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2ub",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2uc",       CPS_B_12,     mapper_STF29,  0x36 },
	{"sf2ud",       CPS_B_05,     mapper_STF29,  0x36 },
	{"sf2ue",       CPS_B_18,     mapper_STF29,  0x3c },
	{"sf2uf",       CPS_B_15,     mapper_STF29,  0x36 },
	{"sf2ug",       CPS_B_11,     mapper_STF29,  0x36 },
	{"sf2ui",       CPS_B_14,     mapper_STF29,  0x36 },
	{"sf2uk",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2j",        CPS_B_13,     mapper_STF29,  0x36 },
	{"sf2ja",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2jc",       CPS_B_12,     mapper_STF29,  0x36 },
	{"sf2jf",       CPS_B_15,     mapper_STF29,  0x36 },
	{"sf2jh",       CPS_B_13,     mapper_STF29,  0x36 },
	{"sf2jl",       CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2qp1",      CPS_B_17,     mapper_STF29,  0x36 },
	{"sf2thndr",    CPS_B_17,     mapper_STF29,  0x36 },

	/* from here onwards the CPS-B board has suicide battery and multiply protection */

	{"3wonders",    CPS_B_21_BT1, mapper_RT24B },
	{"3wondersr1",  CPS_B_21_BT1, mapper_RT24B },
	{"3wondersu",   CPS_B_21_BT1, mapper_RT24B },
	{"wonder3",     CPS_B_21_BT1, mapper_RT22B },   // equivalent to RT24B
	{"3wondersb",   CPS_B_21_BT1, mapper_RT24B,  0x36, 0, 0, 0x88 }, // same as 3wonders except some registers are hard wired rather than written to
	{"3wondersh",   HACK_B_2,     mapper_RT24B },  // one port is changed from 3wonders, and no protection
	{"kod",         CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },
	{"kodr1",       CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },
	{"kodu",        CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },
	{"kodj",        CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },
	{"kodja",       CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },   // wrong, this set uses KD22B, still not dumped
	{"kodb",        CPS_B_21_BT2, mapper_KD29B,  0x36, 0, 0x34 },   /* bootleg, doesn't use multiply protection */
	{"captcomm",    CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34 },
	{"captcommr1",  CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34 },
	{"captcommu",   CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34 },
	{"captcommj",   CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34 },
	{"captcommjr1", CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34 },
	{"captcommb",   CPS_B_21_BT3, mapper_CC63B,  0x36, 0x38, 0x34, 3 },
	{"knights",     CPS_B_21_BT4, mapper_KR63B,  0x36, 0, 0x34 },
	{"knightsu",    CPS_B_21_BT4, mapper_KR63B,  0x36, 0, 0x34 },
	{"knightsj",    CPS_B_21_BT4, mapper_KR63B,  0x36, 0, 0x34 },
	{"knightsja",   CPS_B_21_BT4, mapper_KR63B,  0x36, 0, 0x34 },   // wrong, this set uses KR22B, still not dumped
	//{"knightsb",    CPS_B_21_BT4, mapper_KR63B,  0x36, 0, 0x34 },   // wrong, knightsb bootleg doesn't use the KR63B PAL
	{"sf2ce",       CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceea",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceua",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceub",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceuc",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceja",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2cejb",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2cejc",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2bhh",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2rb",       CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2rb2",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2rb3",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2red",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2v004",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2acc",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2ceblp",    CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2acca",     CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2accp2",    CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2amf",      CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 }, // probably wrong but this set is not completely dumped anyway
	{"sf2amf2",     CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2dkot2",    CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2m1",       CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2m2",       CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2m3",       HACK_B_1,     mapper_S9263B, 0,    0, 0, 2 },
	{"sf2m4",       HACK_B_1,     mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2m5",       CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2m6",       CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2m7",       CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2m8",       HACK_B_1,     mapper_S9263B, 0,    0, 0, 2 },
	{"sf2m9",       CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2m10",      HACK_B_1,     mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2dongb",    CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2yyc",      CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2koryu",    CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2mdt",      CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2mdta",     CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2mdtb",     CPS_B_21_DEF, mapper_S9263B, 0x36, 0, 0, 1 },
	{"sf2b",        CPS_B_17,     mapper_STF29,  0x36, 0, 0, 1  },
	{"varth",       CPS_B_04,     mapper_VA63B },   /* CPSB test has been patched out (60=0008) register is also written to, possibly leftover from development */  // wrong, this set uses VA24B, dumped but equations still not added
	{"varthr1",     CPS_B_04,     mapper_VA63B },   /* CPSB test has been patched out (60=0008) register is also written to, possibly leftover from development */  // wrong, this set uses VA24B, dumped but equations still not added
	{"varthu",      CPS_B_04,     mapper_VA63B },   /* CPSB test has been patched out (60=0008) register is also written to, possibly leftover from development */
	{"varthj",      CPS_B_21_BT5, mapper_VA22B },   /* CPSB test has been patched out (72=0001) register is also written to, possibly leftover from development */
	{"varthjr",     CPS_B_21_BT5, mapper_VA63B },   /* CPSB test has been patched out (72=0001) register is also written to, possibly leftover from development */
	{"cworld2j",    CPS_B_21_BT6, mapper_Q522B,  0x36, 0, 0x34 },   /* (ports 36, 34 probably leftover input code from another game) */
	{"cworld2ja",   CPS_B_21_DEF, mapper_Q522B }, // patched set, no battery, could be desuicided // wrong, this set uses Q529B, still not dumped
	{"cworld2jb",   CPS_B_21_BT6, mapper_Q522B,  0x36, 0, 0x34 }, // wrong, this set uses Q563B, still not dumped
	{"wof",         CPS_B_21_QS1, mapper_TK263B },
	{"wofr1",       CPS_B_21_DEF, mapper_TK263B },  // patched set coming from a desuicided board?
	{"wofa",        CPS_B_21_DEF, mapper_TK263B },  // patched set coming from a desuicided board?
	{"wofu",        CPS_B_21_QS1, mapper_TK263B },
	{"wofj",        CPS_B_21_QS1, mapper_TK263B },
	{"wofhfh",      CPS_B_21_DEF, mapper_TK263B, 0x36 },    /* Chinese bootleg */
	{"dino",        CPS_B_21_QS2, mapper_CD63B },   /* layer enable never used */
	{"dinou",       CPS_B_21_QS2, mapper_CD63B },   /* layer enable never used */
	{"dinoj",       CPS_B_21_QS2, mapper_CD63B },   /* layer enable never used */
	{"dinopic",     CPS_B_21_QS2, mapper_CD63B },   /* layer enable never used */
	{"dinopic2",    CPS_B_21_QS2, mapper_CD63B },   /* layer enable never used */
	{"dinohunt",    CPS_B_21_DEF, mapper_CD63B },   /* Chinese bootleg */
	{"punisher",    CPS_B_21_QS3, mapper_PS63B },
	{"punisheru",   CPS_B_21_QS3, mapper_PS63B },
	{"punisherh",   CPS_B_21_QS3, mapper_PS63B },
	{"punisherj",   CPS_B_21_QS3, mapper_PS63B },
	{"punipic",     CPS_B_21_QS3, mapper_PS63B },
	{"punipic2",    CPS_B_21_QS3, mapper_PS63B },
	{"punipic3",    CPS_B_21_QS3, mapper_PS63B },
	{"punisherbz",  CPS_B_21_DEF, mapper_PS63B },   /* Chinese bootleg */
	{"slammast",    CPS_B_21_QS4, mapper_MB63B },
	{"slammastu",   CPS_B_21_QS4, mapper_MB63B },
	{"slampic",     CPS_B_21_QS4, mapper_MB63B },
	{"mbomberj",    CPS_B_21_QS4, mapper_MB63B },
	{"mbombrd",     CPS_B_21_QS5, mapper_MB63B },
	{"mbombrdj",    CPS_B_21_QS5, mapper_MB63B },
	{"sf2hf",       CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2hfu",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"sf2hfj",      CPS_B_21_DEF, mapper_S9263B, 0x36 },
	{"qad",         CPS_B_21_BT7, mapper_QD22B,  0x36 },    /* TODO: layer enable (port 36 probably leftover input code from another game) */
	{"qadjr",       CPS_B_21_DEF, mapper_QAD63B, 0x36, 0x38, 0x34 },    /* (ports 36, 38, 34 probably leftover input code from another game) */
	{"qtono2j",     CPS_B_21_DEF, mapper_TN2292, 0x36, 0x38, 0x34 },    /* (ports 36, 38, 34 probably leftover input code from another game) */
	{"megaman",     CPS_B_21_DEF, mapper_RCM63B },
	{"megamana",    CPS_B_21_DEF, mapper_RCM63B },
	{"rockmanj",    CPS_B_21_DEF, mapper_RCM63B },
	{"pnickj",      CPS_B_21_DEF, mapper_PKB10B },
	{"pang3",       CPS_B_21_DEF, mapper_pang3 },   /* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */   // should use one of these three CP1B1F,CP1B8K,CP1B9KA
	{"pang3r1",     CPS_B_21_DEF, mapper_pang3 },   /* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */   // should use one of these three CP1B1F,CP1B8K,CP1B9K
	{"pang3j",      CPS_B_21_DEF, mapper_pang3 },   /* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */   // should use one of these three CP1B1F,CP1B8K,CP1B9K
	{"pang3b",      CPS_B_21_DEF, mapper_pang3 },   /* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */   // should use one of these three CP1B1F,CP1B8K,CP1B9K
	{"ganbare",     CPS_B_21_DEF, mapper_sfzch },   // wrong, this set uses GBPR2, dumped but equations still not added

	/* CPS Changer */

	{"sfach",       CPS_B_21_DEF, mapper_sfzch },   // wrong, this set uses an unknown PAL, still not dumped
	{"sfzbch",      CPS_B_21_DEF, mapper_sfzch },   // wrong, this set uses an unknown PAL, still not dumped
	{"sfzch",       CPS_B_21_DEF, mapper_sfzch },   // wrong, this set uses an unknown PAL, still not dumped
	{"wofch",       CPS_B_21_DEF, mapper_TK263B },

	/* CPS2 games */

	{"cps2",        CPS_B_21_DEF, mapper_cps2 },

	/* CPS1 board + extra support boards */

	{"kenseim",     CPS_B_21_DEF, mapper_KNM10B },  // wrong, need to convert equations from PAL

	{nullptr}     /* End of table */
};




/* Offset of each palette entry */
#define cps1_palette_entries (32*6)  /* Number colour schemes in palette */


/* CPS-A registers */
#define CPS1_OBJ_BASE           (0x00/2)    /* Base address of objects */
#define CPS1_SCROLL1_BASE       (0x02/2)    /* Base address of scroll 1 */
#define CPS1_SCROLL2_BASE       (0x04/2)    /* Base address of scroll 2 */
#define CPS1_SCROLL3_BASE       (0x06/2)    /* Base address of scroll 3 */
#define CPS1_OTHER_BASE         (0x08/2)    /* Base address of other video */
#define CPS1_PALETTE_BASE       (0x0a/2)    /* Base address of palette */
#define CPS1_SCROLL1_SCROLLX    (0x0c/2)    /* Scroll 1 X */
#define CPS1_SCROLL1_SCROLLY    (0x0e/2)    /* Scroll 1 Y */
#define CPS1_SCROLL2_SCROLLX    (0x10/2)    /* Scroll 2 X */
#define CPS1_SCROLL2_SCROLLY    (0x12/2)    /* Scroll 2 Y */
#define CPS1_SCROLL3_SCROLLX    (0x14/2)    /* Scroll 3 X */
#define CPS1_SCROLL3_SCROLLY    (0x16/2)    /* Scroll 3 Y */
#define CPS1_STARS1_SCROLLX     (0x18/2)    /* Stars 1 X */
#define CPS1_STARS1_SCROLLY     (0x1a/2)    /* Stars 1 Y */
#define CPS1_STARS2_SCROLLX     (0x1c/2)    /* Stars 2 X */
#define CPS1_STARS2_SCROLLY     (0x1e/2)    /* Stars 2 Y */
#define CPS1_ROWSCROLL_OFFS     (0x20/2)    /* base of row scroll offsets in other RAM */
#define CPS1_VIDEOCONTROL       (0x22/2)    /* flip screen, rowscroll enable */


/*
CPS1 VIDEO RENDERER

*/
#define CPS2_OBJ_BASE   0x00    /* Unknown (not base address of objects). Could be bass address of bank used when object swap bit set? */
#define CPS2_OBJ_UK1    0x02    /* Unknown (nearly always 0x807d, or 0x808e when screen flipped) */
#define CPS2_OBJ_PRI    0x04    /* Layers priorities */
#define CPS2_OBJ_UK2    0x06    /* Unknown (usually 0x0000, 0x1101 in ssf2, 0x0001 in 19XX) */
#define CPS2_OBJ_XOFFS  0x08    /* X offset (usually 0x0040) */
#define CPS2_OBJ_YOFFS  0x0a    /* Y offset (always 0x0010) */


MACHINE_RESET_MEMBER(cps_state,cps)
{
	const char *gamename = machine().system().name;
	const struct CPS1config *pCFG = &cps1_config_table[0];

	while (pCFG->name)
	{
		if (strcmp(pCFG->name, gamename) == 0)
			break;

		pCFG++;
	}

	m_game_config = pCFG;

	if (!m_game_config->name)
	{
		gamename = "cps2";
		pCFG = &cps1_config_table[0];

		while(pCFG->name)
		{
			if (strcmp(pCFG->name, gamename) == 0)
				break;

			pCFG++;
		}

		m_game_config = pCFG;
	}

#if 0
	if (strcmp(gamename, "sf2accp2") == 0)
	{
		/* Patch out a odd branch which would be incorrectly interpreted
		   by the cpu core as a 32-bit branch. This branch would make the
		   game crash (address error, since it would branch to an odd address)
		   if location 180ca6 (outside ROM space) isn't 0. Protection check? */
		UINT16 *rom = (UINT16 *)memregion("maincpu")->base();
		rom[0x11756 / 2] = 0x4e71;
	}
	else if (strcmp(gamename, "ghouls") == 0)
	{
		/* Patch out self-test... it takes forever */
		UINT16 *rom = (UINT16 *)memregion("maincpu")->base();
		rom[0x61964 / 2] = 0x4ef9;
		rom[0x61966 / 2] = 0x0000;
		rom[0x61968 / 2] = 0x0400;
	}
#endif
}


inline UINT16 *cps_state::cps1_base( int offset, int boundary )
{
	int base = m_cps_a_regs[offset] * 256;

	/*
	The scroll RAM must start on a 0x4000 boundary.
	Some games do not do this.
	For example:
	   Captain commando     - continue screen will not display
	   Muscle bomber games  - will animate garbage during gameplay
	Mask out the irrelevant bits.
	*/
	base &= ~(boundary - 1);
	return &m_gfxram[(base & 0x3ffff) / 2];
}



WRITE16_MEMBER(cps_state::cps1_cps_a_w)
{
	data = COMBINE_DATA(&m_cps_a_regs[offset]);

	/*
	The main CPU writes the palette to gfxram, and the CPS-B custom copies it
	to the real palette RAM, which is separated from gfxram.
	This is done ONLY after the palette base register is written to. It is not
	known what the exact timing should be, how long it should take and when it
	should happen. We are assuming that the copy happens immediately, since it
	fixes glitches in the ghouls intro, but it might happen at next vblank.
	*/
	if (offset == CPS1_PALETTE_BASE)
		cps1_build_palette(cps1_base(CPS1_PALETTE_BASE, m_palette_align));

	// pzloop2 write to register 24 on startup. This is probably just a bug.
	if (offset == 0x24 / 2 && m_cps_version == 2)
		return;

#ifdef MAME_DEBUG
	if (offset > CPS1_VIDEOCONTROL)
		popmessage("write to CPS-A register %02x contact MAMEDEV", offset * 2);
#endif
}


READ16_MEMBER(cps_state::cps1_cps_b_r)
{
	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	if (offset == m_game_config->cpsb_addr / 2)
		return m_game_config->cpsb_value;

	/* some games use as a protection check the ability to do 16-bit multiplications */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	if (offset == m_game_config->mult_result_lo / 2)
		return (m_cps_b_regs[m_game_config->mult_factor1 / 2] *
				m_cps_b_regs[m_game_config->mult_factor2 / 2]) & 0xffff;

	if (offset == m_game_config->mult_result_hi / 2)
		return (m_cps_b_regs[m_game_config->mult_factor1 / 2] *
				m_cps_b_regs[m_game_config->mult_factor2 / 2]) >> 16;

	if (offset == m_game_config->in2_addr / 2)  /* Extra input ports (on C-board) */
		return ioport("IN2")->read();

	if (offset == m_game_config->in3_addr / 2)  /* Player 4 controls (on C-board) ("Captain Commando") */
		return ioport("IN3")->read();

	if (m_cps_version == 2)
	{
		if (offset == 0x10/2)
		{
			// UNKNOWN--only mmatrix appears to read this, and I'm not sure if the result is actually used
			return m_cps_b_regs[0x10 / 2];
		}
		if (offset == 0x12/2)
			return m_cps_b_regs[0x12 / 2];
	}
#ifdef MAME_DEBUG
	popmessage("CPS-B read port %02x contact MAMEDEV", offset * 2);
#endif
	return 0xffff;
}


WRITE16_MEMBER(cps_state::cps1_cps_b_w)
{
	data = COMBINE_DATA(&m_cps_b_regs[offset]);

	if (m_cps_version == 2)
	{
		/* To mark scanlines for raster effects */
		if (offset == 0x0e/2)
		{
			// UNKNOWN
			return;
		}
		if (offset == 0x10/2)
		{
			m_scanline1 = (data & 0x1ff);
			return;
		}
		if (offset == 0x12/2)
		{
			m_scanline2 = (data & 0x1ff);
			return;
		}
	}


	// additional outputs on C-board
	if (offset == m_game_config->out2_addr / 2)
	{
		if (ACCESSING_BITS_0_7)
		{
			if (m_game_config->cpsb_value == 0x0402)    // Mercs (CN2 connector)
			{
				coin_lockout_w(machine(), 2, ~data & 0x01);
				set_led_status(machine(), 0, data & 0x02);
				set_led_status(machine(), 1, data & 0x04);
				set_led_status(machine(), 2, data & 0x08);
			}
			else    // kod, captcomm, knights
			{
				coin_lockout_w(machine(), 2, ~data & 0x02);
				coin_lockout_w(machine(), 3, ~data & 0x08);
			}
		}
	}

#ifdef MAME_DEBUG
	if (offset != m_game_config->cpsb_addr / 2 &&   // only varth writes here
			offset != m_game_config->mult_factor1 / 2 &&
			offset != m_game_config->mult_factor2 / 2 &&
			offset != m_game_config->layer_control / 2 &&
			offset != m_game_config->unknown1 / 2 &&
			offset != m_game_config->unknown2 / 2 &&
			offset != m_game_config->unknown3 / 2 &&
			offset != m_game_config->priority[0] / 2 &&
			offset != m_game_config->priority[1] / 2 &&
			offset != m_game_config->priority[2] / 2 &&
			offset != m_game_config->priority[3] / 2 &&
			offset != m_game_config->palette_control / 2 &&
			offset != m_game_config->out2_addr / 2 &&
			!m_game_config->bootleg_kludge)
		popmessage("CPS-B write %04x to port %02x contact MAMEDEV", data, offset * 2);
#endif
}


void cps_state::unshuffle( UINT64 *buf, int len )
{
	int i;
	UINT64 t;

	if (len == 2)
		return;

	assert(len % 4 == 0);   /* must not happen */

	len /= 2;

	unshuffle(buf, len);
	unshuffle(buf + len, len);

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}
}


void cps_state::cps2_gfx_decode()
{
	const int banksize = 0x200000;
	int size = memregion("gfx")->bytes();
	int i;

	for (i = 0; i < size; i += banksize)
		unshuffle((UINT64 *)(memregion("gfx")->base() + i), banksize / 8);
}


DRIVER_INIT_MEMBER(cps_state,cps1)
{
	m_scanline1 = 0;
	m_scanline2 = 0;
	m_scancalls = 0;
	m_last_sprite_offset = 0;
	m_pri_ctrl = 0;
	m_objram_bank = 0;
}



DRIVER_INIT_MEMBER(cps_state,cps2_video)
{
	cps2_gfx_decode();

	m_scanline1 = 262;
	m_scanline2 = 262;
	m_scancalls = 0;
	m_last_sprite_offset = 0;
	m_cps2_last_sprite_offset = 0;
	m_pri_ctrl = 0;
	m_objram_bank = 0;
}


void cps_state::cps1_get_video_base()
{
	int layercontrol=0, videocontrol=0, scroll1xoff=0, scroll2xoff=0, scroll3xoff=0;

	/* Re-calculate the VIDEO RAM base */
	if (m_scroll1 != cps1_base(CPS1_SCROLL1_BASE, m_scroll_size))
	{
		m_scroll1 = cps1_base(CPS1_SCROLL1_BASE, m_scroll_size);
		m_bg_tilemap[0]->mark_all_dirty();
	}
	if (m_scroll2 != cps1_base(CPS1_SCROLL2_BASE, m_scroll_size))
	{
		m_scroll2 = cps1_base(CPS1_SCROLL2_BASE, m_scroll_size);
		m_bg_tilemap[1]->mark_all_dirty();
	}
	if (m_scroll3 != cps1_base(CPS1_SCROLL3_BASE, m_scroll_size))
	{
		m_scroll3 = cps1_base(CPS1_SCROLL3_BASE, m_scroll_size);
		m_bg_tilemap[2]->mark_all_dirty();
	}

	/* Some of the sf2 hacks use only sprite port 0x9100 and the scroll layers are offset */
	if (m_game_config->bootleg_kludge == 1)
	{
		m_cps_a_regs[CPS1_OBJ_BASE] = 0x9100;
		scroll1xoff = -0x0c;
		scroll2xoff = -0x0e;
		scroll3xoff = -0x10;
	}
	else
	if (m_game_config->bootleg_kludge == 2)
	{
		m_cps_a_regs[CPS1_OBJ_BASE] = 0x9100;
		scroll1xoff = -0x0c;
		scroll2xoff = -0x10;
		scroll3xoff = -0x10;
	}
	else
	if (m_game_config->bootleg_kludge == 3)
	{
		scroll1xoff = -0x08;
		scroll2xoff = -0x0b;
		scroll3xoff = -0x0c;
	}
	else
	if (m_game_config->bootleg_kludge == 0x88) // 3wondersb
	{
		scroll1xoff = 0x4;
		scroll2xoff = 0x6;
		scroll3xoff = 0xa;
		m_cps_b_regs[0x30/2] = 0x3f;
		m_cps_a_regs[CPS1_VIDEOCONTROL] = 0x3e;
		m_cps_a_regs[CPS1_SCROLL2_BASE] = 0x90c0;
		m_cps_a_regs[CPS1_SCROLL3_BASE] = 0x9100;
		m_cps_a_regs[CPS1_PALETTE_BASE] = 0x9140;
	}

	m_obj = cps1_base(CPS1_OBJ_BASE, m_obj_size);
	m_other = cps1_base(CPS1_OTHER_BASE, m_other_size);

	/* Get scroll values */
	m_scroll1x = m_cps_a_regs[CPS1_SCROLL1_SCROLLX] + scroll1xoff;
	m_scroll1y = m_cps_a_regs[CPS1_SCROLL1_SCROLLY];
	m_scroll2x = m_cps_a_regs[CPS1_SCROLL2_SCROLLX] + scroll2xoff;
	m_scroll2y = m_cps_a_regs[CPS1_SCROLL2_SCROLLY];
	m_scroll3x = m_cps_a_regs[CPS1_SCROLL3_SCROLLX] + scroll3xoff;
	m_scroll3y = m_cps_a_regs[CPS1_SCROLL3_SCROLLY];
	m_stars1x = m_cps_a_regs[CPS1_STARS1_SCROLLX];
	m_stars1y = m_cps_a_regs[CPS1_STARS1_SCROLLY];
	m_stars2x = m_cps_a_regs[CPS1_STARS2_SCROLLX];
	m_stars2y = m_cps_a_regs[CPS1_STARS2_SCROLLY];

	/* Get layer enable bits */
	layercontrol = m_cps_b_regs[m_game_config->layer_control / 2];
	videocontrol = m_cps_a_regs[CPS1_VIDEOCONTROL];
	m_bg_tilemap[0]->enable(layercontrol & m_game_config->layer_enable_mask[0]);
	m_bg_tilemap[1]->enable((layercontrol & m_game_config->layer_enable_mask[1]) && (videocontrol & 4));
	m_bg_tilemap[2]->enable((layercontrol & m_game_config->layer_enable_mask[2]) && (videocontrol & 8));
	m_stars_enabled[0] = layercontrol & m_game_config->layer_enable_mask[3];
	m_stars_enabled[1] = layercontrol & m_game_config->layer_enable_mask[4];

#ifdef MAME_DEBUG
{
	int enablemask = 0;

	if (m_game_config->layer_enable_mask[0] == m_game_config->layer_enable_mask[1])
		enablemask = m_game_config->layer_enable_mask[0];

	if (m_game_config->layer_enable_mask[0] == m_game_config->layer_enable_mask[2])
		enablemask = m_game_config->layer_enable_mask[0];

	if (m_game_config->layer_enable_mask[1] == m_game_config->layer_enable_mask[2])
		enablemask = m_game_config->layer_enable_mask[1];

	if (enablemask)
	{
		if (((layercontrol & enablemask) && (layercontrol & enablemask) != enablemask))
			popmessage("layer %02x contact MAMEDEV", layercontrol & 0xc03f);
	}

	enablemask = m_game_config->layer_enable_mask[0] | m_game_config->layer_enable_mask[1]
			| m_game_config->layer_enable_mask[2]
			| m_game_config->layer_enable_mask[3] | m_game_config->layer_enable_mask[4];

	if (((layercontrol & ~enablemask) & 0x003e) != 0)
		popmessage("layer %02x contact MAMEDEV", layercontrol & 0xc03f);
}
#endif

}


WRITE16_MEMBER(cps_state::cps1_gfxram_w)
{
	int page = (offset >> 7) & 0x3c0;
	COMBINE_DATA(&m_gfxram[offset]);

	if (page == (m_cps_a_regs[CPS1_SCROLL1_BASE] & 0x3c0))
		m_bg_tilemap[0]->mark_tile_dirty(offset / 2 & 0x0fff);

	if (page == (m_cps_a_regs[CPS1_SCROLL2_BASE] & 0x3c0))
		m_bg_tilemap[1]->mark_tile_dirty(offset / 2 & 0x0fff);

	if (page == (m_cps_a_regs[CPS1_SCROLL3_BASE] & 0x3c0))
		m_bg_tilemap[2]->mark_tile_dirty(offset / 2 & 0x0fff);
}



int cps_state::gfxrom_bank_mapper( int type, int code )
{
	const struct gfx_range *range = m_game_config->bank_mapper;
	int shift = 0;

	assert(range);

	switch (type)
	{
		case GFXTYPE_SPRITES: shift = 1; break;
		case GFXTYPE_SCROLL1: shift = 0; break;
		case GFXTYPE_SCROLL2: shift = 1; break;
		case GFXTYPE_SCROLL3: shift = 3; break;
	}

	code <<= shift;

	while (range->type)
	{
		if (code >= range->start && code <= range->end)
		{
			if (range->type & type)
			{
				int base = 0;
				int i;

				for (i = 0; i < range->bank; ++i)
					base += m_game_config->bank_sizes[i];

				return (base + (code & (m_game_config->bank_sizes[range->bank] - 1))) >> shift;
			}
		}

		++range;
	}

#ifdef MAME_DEBUG
//  popmessage("tile %02x/%04x out of range", type, code >> shift);
#endif

	return -1;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

TILEMAP_MAPPER_MEMBER(cps_state::tilemap0_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((col & 0x3f) << 5) + ((row & 0x20) << 6);
}

TILEMAP_MAPPER_MEMBER(cps_state::tilemap1_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x30) << 6);
}

TILEMAP_MAPPER_MEMBER(cps_state::tilemap2_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x07) + ((col & 0x3f) << 3) + ((row & 0x38) << 6);
}

TILE_GET_INFO_MEMBER(cps_state::get_tile0_info)
{
	int code = m_scroll1[2 * tile_index];
	int attr = m_scroll1[2 * tile_index + 1];
	int gfxset;

	code = gfxrom_bank_mapper(GFXTYPE_SCROLL1, code);

	/* allows us to reproduce a problem seen with a ffight board where USA and Japanese
	     roms have been mixed to be reproduced (ffightub) -- it looks like each column
	     should alternate between the left and right side of the 16x16 tiles */
	gfxset = (tile_index & 0x20) >> 5;

	SET_TILE_INFO_MEMBER(gfxset,
			code,
			(attr & 0x1f) + 0x20,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo.group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	// (but still call SET_TILE_INFO_MEMBER, otherwise problems might occur on boot e.g. unsquad)
	if (code == -1)
		tileinfo.pen_data = m_empty_tile;
}

TILE_GET_INFO_MEMBER(cps_state::get_tile1_info)
{
	int code = m_scroll2[2 * tile_index];
	int attr = m_scroll2[2 * tile_index + 1];

	code = gfxrom_bank_mapper(GFXTYPE_SCROLL2, code);

	SET_TILE_INFO_MEMBER(2,
			code,
			(attr & 0x1f) + 0x40,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo.group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	if (code == -1)
		tileinfo.pen_data = m_empty_tile;
}

TILE_GET_INFO_MEMBER(cps_state::get_tile2_info)
{
	int code = m_scroll3[2 * tile_index] & 0x3fff;
	int attr = m_scroll3[2 * tile_index + 1];

	code = gfxrom_bank_mapper(GFXTYPE_SCROLL3, code);

	SET_TILE_INFO_MEMBER(3,
			code,
			(attr & 0x1f) + 0x60,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo.group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	// (but still call SET_TILE_INFO_MEMBER, otherwise problems might occur on boot e.g. unsquad)
	if (code == -1)
		tileinfo.pen_data = m_empty_tile;
}



void cps_state::cps1_update_transmasks()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int mask;

		/* Get transparency registers */
		if (m_game_config->priority[i] != -1)
			mask = m_cps_b_regs[m_game_config->priority[i] / 2] ^ 0xffff;
		else
			mask = 0xffff;  /* completely transparent if priority masks not defined (qad) */

		m_bg_tilemap[0]->set_transmask(i, mask, 0x8000);
		m_bg_tilemap[1]->set_transmask(i, mask, 0x8000);
		m_bg_tilemap[2]->set_transmask(i, mask, 0x8000);
	}
}

VIDEO_START_MEMBER(cps_state,cps)
{
	int i;

	MACHINE_RESET_CALL_MEMBER(cps);

	/* Put in some const */
	m_scroll_size    = 0x4000;  /* scroll1, scroll2, scroll3 */
	m_obj_size       = 0x0800;
	m_cps2_obj_size  = 0x2000;
	m_other_size     = 0x0800;
	m_palette_align  = 0x0400;  /* minimum alignment is a single palette page (512 colors). Verified on pcb. */
	m_palette_size   = cps1_palette_entries * 32; /* Size of palette RAM */
	m_stars_rom_size = 0x2000;  /* first 0x4000 of gfx ROM are used, but 0x0000-0x1fff is == 0x2000-0x3fff */

	/* create tilemaps */
	m_bg_tilemap[0] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cps_state::get_tile0_info),this), tilemap_mapper_delegate(FUNC(cps_state::tilemap0_scan),this),  8,  8, 64, 64);
	m_bg_tilemap[1] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cps_state::get_tile1_info),this), tilemap_mapper_delegate(FUNC(cps_state::tilemap1_scan),this), 16, 16, 64, 64);
	m_bg_tilemap[2] = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cps_state::get_tile2_info),this), tilemap_mapper_delegate(FUNC(cps_state::tilemap2_scan),this), 32, 32, 64, 64);

	/* create empty tiles */
	memset(m_empty_tile, 0x0f, sizeof(m_empty_tile));

	/* front masks will change at runtime to handle sprite occluding */
	cps1_update_transmasks();

	for (i = 0; i < cps1_palette_entries * 16; i++)
		m_palette->set_pen_color(i, rgb_t(0,0,0));

	m_buffered_obj = make_unique_clear<UINT16[]>(m_obj_size / 2);

	if (m_cps_version == 2)
		m_cps2_buffered_obj = make_unique_clear<UINT16[]>(m_cps2_obj_size / 2);

	/* clear RAM regions */
	memset(m_gfxram, 0, m_gfxram.bytes());   /* Clear GFX RAM */
	memset(m_cps_a_regs, 0, 0x40);   /* Clear CPS-A registers */
	memset(m_cps_b_regs, 0, 0x40);   /* Clear CPS-B registers */

	if (m_cps_version == 2)
	{
		memset(m_objram1, 0, m_cps2_obj_size);
		memset(m_objram2, 0, m_cps2_obj_size);
	}

	/* Put in some defaults */
	m_cps_a_regs[CPS1_OBJ_BASE]     = 0x9200;
	m_cps_a_regs[CPS1_SCROLL1_BASE] = 0x9000;
	m_cps_a_regs[CPS1_SCROLL2_BASE] = 0x9040;
	m_cps_a_regs[CPS1_SCROLL3_BASE] = 0x9080;
	m_cps_a_regs[CPS1_OTHER_BASE]   = 0x9100;

	/* This should never be hit, since game_config is set in machine_reset */
	assert_always(m_game_config, "state_game_config hasn't been set up yet");


	/* Set up old base */
	m_scroll1 = nullptr;
	m_scroll2 = nullptr;
	m_scroll3 = nullptr;
	m_obj = nullptr;
	m_other = nullptr;
	cps1_get_video_base();   /* Calculate base pointers */
	cps1_get_video_base();   /* Calculate old base pointers */

	/* state save register */
	save_item(NAME(m_scanline1));
	save_item(NAME(m_scanline2));
	save_item(NAME(m_scancalls));
#if 0
	/* these do not need to be saved, because they are recovered from cps_a_regs in cps1_postload */
	save_item(NAME(m_scroll1x));
	save_item(NAME(m_scroll1y));
	save_item(NAME(m_scroll2x));
	save_item(NAME(m_scroll2y));
	save_item(NAME(m_scroll3x));
	save_item(NAME(m_scroll3y));
	save_item(NAME(m_stars1x));
	save_item(NAME(m_stars1y));
	save_item(NAME(m_stars2x));
	save_item(NAME(m_stars2y));
	save_item(NAME(m_stars_enabled));
#endif
	save_item(NAME(m_last_sprite_offset));
	save_item(NAME(m_pri_ctrl));
	save_item(NAME(m_objram_bank));

	save_pointer(NAME(m_buffered_obj.get()), m_obj_size / 2);
	if (m_cps_version == 2)
	{
		save_item(NAME(m_cps2_last_sprite_offset));
		save_pointer(NAME(m_cps2_buffered_obj.get()), m_cps2_obj_size / 2);
	}

	machine().save().register_postload(save_prepost_delegate(FUNC(cps_state::cps1_get_video_base), this));
}

VIDEO_START_MEMBER(cps_state,cps1)
{
	m_cps_version = 1;
	VIDEO_START_CALL_MEMBER(cps);
}

VIDEO_START_MEMBER(cps_state,cps2)
{
	m_cps_version = 2;
	VIDEO_START_CALL_MEMBER(cps);
}

/***************************************************************************

  Build palette from palette RAM

  12 bit RGB with a 4 bit brightness value.

***************************************************************************/

void cps_state::cps1_build_palette( const UINT16* const palette_base )
{
	int offset, page;
	const UINT16 *palette_ram = palette_base;
	int ctrl = m_cps_b_regs[m_game_config->palette_control/2];

	/*
	The palette is copied only for pages that are enabled in the ctrl
	register. Note that if the first palette pages are skipped, all
	the following pages are scaled down.
	*/
	for (page = 0; page < 6; ++page)
	{
		if (BIT(ctrl, page))
		{
			for (offset = 0; offset < 0x200; ++offset)
			{
				int palette = *(palette_ram++);
				int r, g, b, bright;

				// from my understanding of the schematics, when the 'brightness'
				// component is set to 0 it should reduce brightness to 1/3

				bright = 0x0f + ((palette >> 12) << 1);

				r = ((palette >> 8) & 0x0f) * 0x11 * bright / 0x2d;
				g = ((palette >> 4) & 0x0f) * 0x11 * bright / 0x2d;
				b = ((palette >> 0) & 0x0f) * 0x11 * bright / 0x2d;

				m_palette->set_pen_color (0x200 * page + offset, rgb_t(r, g, b));
			}
		}
		else
		{
			// skip page in gfxram, but only if we have already copied at least one page
			if (palette_ram != palette_base)
				palette_ram += 0x200;
		}
	}
}



/***************************************************************************

                                Sprites
                                =======

  Sprites are represented by a number of 8 byte values

  xx xx yy yy nn nn aa aa

  where xxxx = x position
        yyyy = y position
        nnnn = tile number
        aaaa = attribute word
                    0x0001  colour
                    0x0002  colour
                    0x0004  colour
                    0x0008  colour
                    0x0010  colour
                    0x0020  X Flip
                    0x0040  Y Flip
                    0x0080  X & Y offset toggle (used in Marvel vs. Capcom.)
                    0x0100  X block size (in sprites)
                    0x0200  X block size
                    0x0400  X block size
                    0x0800  X block size
                    0x1000  Y block size (in sprites)
                    0x2000  Y block size
                    0x4000  Y block size
                    0x8000  Y block size

  The end of the table (may) be marked by an attribute value of 0xff00.

***************************************************************************/

void cps_state::cps1_find_last_sprite()    /* Find the offset of last sprite */
{
	int offset = 0;
	/* Locate the end of table marker */
	while (offset < m_obj_size / 2)
	{
		if (m_game_config->bootleg_kludge == 3) {
			/* captcommb - same end of sprite marker as CPS-2 */
			int colour = m_buffered_obj[offset + 1];
			if (colour >= 0x8000)
			{
				/* Marker found. This is the last sprite. */
				m_last_sprite_offset = offset - 4;
				return;
			}
		}
		else
		{
			int colour = m_buffered_obj[offset + 3];
			if ((colour & 0xff00) == 0xff00)
			{
				/* Marker found. This is the last sprite. */
				m_last_sprite_offset = offset - 4;
				return;
			}
		}

		offset += 4;
	}
	/* Sprites must use full sprite RAM */
	m_last_sprite_offset = m_obj_size / 2 - 4;
}


void cps_state::cps1_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
#define DRAWSPRITE(CODE,COLOR,FLIPX,FLIPY,SX,SY)                    \
{                                                                   \
	if (flip_screen())                                           \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap,\
				cliprect,                            \
				CODE,                                               \
				COLOR,                                              \
				!(FLIPX),!(FLIPY),                                  \
				512-16-(SX),256-16-(SY),    screen.priority(),0x02,15);                   \
	else                                                            \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap,\
				cliprect,                            \
				CODE,                                               \
				COLOR,                                              \
				FLIPX,FLIPY,                                        \
				SX,SY, screen.priority(),0x02,15);          \
}


	int i, baseadd;
	UINT16 *base = m_buffered_obj.get();

	/* some sf2 hacks draw the sprites in reverse order */
	if ((m_game_config->bootleg_kludge == 1) || (m_game_config->bootleg_kludge == 2) || (m_game_config->bootleg_kludge == 3))
	{
		base += m_last_sprite_offset;
		baseadd = -4;
	}
	else
	{
		baseadd = 4;
	}

	for (i = m_last_sprite_offset; i >= 0; i -= 4)
	{
		int x = *(base + 0);
		int y = *(base + 1);
		int code = *(base + 2);
		int colour = *(base + 3);
		int col = colour & 0x1f;

//      x -= 0x20;
//      y += 0x20;

		code = gfxrom_bank_mapper(GFXTYPE_SPRITES, code);

		if (code != -1)
		{
			if (colour & 0xff00 )
			{
				/* handle blocked sprites */
				int nx = (colour & 0x0f00) >> 8;
				int ny = (colour & 0xf000) >> 12;
				int nxs, nys, sx, sy;
				nx++;
				ny++;

				if (colour & 0x40)
				{
					/* Y flip */
					if (colour & 0x20)
					{
						for (nys = 0; nys < ny; nys++)
						{
							for (nxs = 0; nxs < nx; nxs++)
							{
								sx = (x + nxs * 16) & 0x1ff;
								sy = (y + nys * 16) & 0x1ff;

								DRAWSPRITE(
//                                      code + (nx - 1) - nxs + 0x10 * (ny - 1 - nys),
										(code & ~0xf) + ((code + (nx - 1) - nxs) & 0xf) + 0x10 * (ny - 1 - nys),
										(col & 0x1f),
										1,1,
										sx,sy);
							}
						}
					}
					else
					{
						for (nys = 0; nys < ny; nys++)
						{
							for (nxs = 0; nxs < nx; nxs++)
							{
								sx = (x + nxs * 16) & 0x1ff;
								sy = (y + nys * 16) & 0x1ff;

								DRAWSPRITE(
//                                      code + nxs + 0x10 * (ny - 1 - nys),
										(code & ~0xf) + ((code + nxs) & 0xf) + 0x10 * (ny - 1 - nys),
										(col & 0x1f),
										0,1,
										sx,sy);
							}
						}
					}
				}
				else
				{
					if (colour & 0x20)
					{
						for (nys = 0; nys < ny; nys++)
						{
							for (nxs = 0; nxs<nx; nxs++)
							{
								sx = (x + nxs * 16) & 0x1ff;
								sy = (y + nys * 16) & 0x1ff;

								DRAWSPRITE(
//                                      code + (nx - 1) - nxs + 0x10 * nys,
										(code & ~0xf) + ((code + (nx - 1) - nxs) & 0xf) + 0x10 * nys,
										(col & 0x1f),
										1,0,
										sx,sy);
							}
						}
					}
					else
					{
						for (nys = 0; nys < ny; nys++)
						{
							for (nxs = 0; nxs < nx; nxs++)
							{
								sx = (x + nxs * 16) & 0x1ff;
								sy = (y + nys * 16) & 0x1ff;

								DRAWSPRITE(
//                                      code + nxs + 0x10 * nys,
										(code & ~0xf) + ((code + nxs) & 0xf) + 0x10 * nys,  // fix 00406: qadjr: When playing as the ninja, there is one broken frame in his animation loop when walking.
										(col & 0x1f),
										0,0,
										sx,sy);
							}
						}
					}
				}
			}
			else
			{
				/* Simple case... 1 sprite */
						DRAWSPRITE(
						code,
						(col & 0x1f),
						colour&0x20,colour&0x40,
						x & 0x1ff,y & 0x1ff);
			}
		}
		base += baseadd;
	}
#undef DRAWSPRITE
}




WRITE16_MEMBER(cps_state::cps2_objram_bank_w)
{
	if (ACCESSING_BITS_0_7)
		m_objram_bank = data & 1;
}

READ16_MEMBER(cps_state::cps2_objram1_r)
{
	if (m_objram_bank & 1)
		return m_objram2[offset];
	else
		return m_objram1[offset];
}

READ16_MEMBER(cps_state::cps2_objram2_r)
{
	if (m_objram_bank & 1)
		return m_objram1[offset];
	else
		return m_objram2[offset];
}

WRITE16_MEMBER(cps_state::cps2_objram1_w)
{
	if (m_objram_bank & 1)
		COMBINE_DATA(&m_objram2[offset]);
	else
		COMBINE_DATA(&m_objram1[offset]);
}

WRITE16_MEMBER(cps_state::cps2_objram2_w)
{
	if (m_objram_bank & 1)
		COMBINE_DATA(&m_objram1[offset]);
	else
		COMBINE_DATA(&m_objram2[offset]);
}

UINT16 *cps_state::cps2_objbase()
{
	int baseptr;
	baseptr = 0x7000;

	if (m_objram_bank & 1)
		baseptr ^= 0x0080;

//popmessage("%04x %d", cps2_port(machine, CPS2_OBJ_BASE), m_objram_bank & 1);

	if (baseptr == 0x7000)
		return m_objram1;
	else //if (baseptr == 0x7080)
		return m_objram2;
}


void cps_state::cps2_find_last_sprite()    /* Find the offset of last sprite */
{
	int offset = 0;
	UINT16 *base = m_cps2_buffered_obj.get();

	/* Locate the end of table marker */
	while (offset < m_cps2_obj_size / 2)
	{
		if (base[offset + 1] >= 0x8000 || base[offset + 3] >= 0xff00)
		{
			/* Marker found. This is the last sprite. */
			m_cps2_last_sprite_offset = offset - 4;
			return;
		}

		offset += 4;
	}
	/* Sprites must use full sprite RAM */
	m_cps2_last_sprite_offset = m_cps2_obj_size / 2 - 4;
}

void cps_state::cps2_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks )
{
#define DRAWSPRITE(CODE,COLOR,FLIPX,FLIPY,SX,SY)                                    \
{                                                                                   \
	if (flip_screen())                                                           \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap,\
				cliprect,                                            \
				CODE,                                                               \
				COLOR,                                                              \
				!(FLIPX),!(FLIPY),                                                  \
				512-16-(SX),256-16-(SY), screen.priority(),primasks[priority],15);                 \
	else                                                                            \
		m_gfxdecode->gfx(2)->prio_transpen(bitmap,\
				cliprect,                                            \
				CODE,                                                               \
				COLOR,                                                              \
				FLIPX,FLIPY,                                                        \
				SX,SY, screen.priority(),primasks[priority],15);                 \
}

	int i;
	UINT16 *base = m_cps2_buffered_obj.get();
	int xoffs = 64 - m_output[CPS2_OBJ_XOFFS /2];
	int yoffs = 16 - m_output[CPS2_OBJ_YOFFS /2];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z) && machine().input().code_pressed(KEYCODE_R))
	{
		return;
	}
#endif

	for (i = m_cps2_last_sprite_offset; i >= 0; i -= 4)
	{
		int x = base[i + 0];
		int y = base[i + 1];
		int priority = (x >> 13) & 0x07;
		int code = base[i + 2] + ((y & 0x6000) << 3);
		int colour = base[i + 3];
		int col = colour & 0x1f;

		if (colour & 0x80)
		{
			x += m_output[CPS2_OBJ_XOFFS /2];  /* fix the offset of some games */
			y += m_output[CPS2_OBJ_YOFFS /2];  /* like Marvel vs. Capcom ending credits */
		}

		if (colour & 0xff00)
		{
			/* handle blocked sprites */
			int nx = (colour & 0x0f00) >> 8;
			int ny = (colour & 0xf000) >> 12;
			int nxs, nys, sx, sy;
			nx++;
			ny++;

			if (colour & 0x40)
			{
				/* Y flip */
				if (colour & 0x20)
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (x + nxs * 16 + xoffs) & 0x3ff;
							sy = (y + nys * 16 + yoffs) & 0x3ff;
							DRAWSPRITE(
									code + (nx - 1) - nxs + 0x10 * (ny - 1 - nys),
									(col & 0x1f),
									1,1,
									sx,sy);
						}
					}
				}
				else
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (x + nxs * 16 + xoffs) & 0x3ff;
							sy = (y + nys * 16 + yoffs) & 0x3ff;

							DRAWSPRITE(
									code + nxs + 0x10 * (ny - 1 - nys),
									(col & 0x1f),
									0,1,
									sx,sy);
						}
					}
				}
			}
			else
			{
				if (colour & 0x20)
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (x + nxs * 16 + xoffs) & 0x3ff;
							sy = (y + nys * 16 + yoffs) & 0x3ff;

							DRAWSPRITE(
									code + (nx - 1) - nxs + 0x10 * nys,
									(col & 0x1f),
									1,0,
									sx,sy);
						}
					}
				}
				else
				{
					for (nys = 0; nys < ny; nys++)
					{
						for (nxs = 0; nxs < nx; nxs++)
						{
							sx = (x + nxs * 16 + xoffs) & 0x3ff;
							sy = (y + nys * 16 + yoffs) & 0x3ff;

							DRAWSPRITE(
//                                      code + nxs + 0x10 * nys,
									(code & ~0xf) + ((code + nxs) & 0xf) + 0x10 * nys,  //  pgear fix
									(col & 0x1f),
									0,0,
									sx,sy);
						}
					}
				}
			}
		}
		else
		{
			/* Simple case... 1 sprite */
			DRAWSPRITE(
					code,
					(col & 0x1f),
					colour&0x20,colour&0x40,
					(x+xoffs) & 0x3ff,(y+yoffs) & 0x3ff);
		}
	}
}




void cps_state::cps1_render_stars( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int offs;
	UINT8 *stars_rom = memregion("stars")->base();

	if (!stars_rom && (m_stars_enabled[0] || m_stars_enabled[1]))
	{
#ifdef MAME_DEBUG
//      popmessage("stars enabled but no stars ROM");
#endif
		return;
	}

	if (m_stars_enabled[0])
	{
		for (offs = 0; offs < m_stars_rom_size / 2; offs++)
		{
			int col = stars_rom[8 * offs + 4];
			if (col != 0x0f)
			{
				int sx = (offs / 256) * 32;
				int sy = (offs % 256);
				sx = (sx - m_stars2x + (col & 0x1f)) & 0x1ff;
				sy = (sy - m_stars2y) & 0xff;
				if (flip_screen())
				{
					sx = 512 - sx;
					sy = 256 - sy;
				}

				col = ((col & 0xe0) >> 1) + (screen.frame_number() / 16 & 0x0f);

				if (cliprect.contains(sx, sy))
					bitmap.pix16(sy, sx) = 0xa00 + col;
			}
		}
	}

	if (m_stars_enabled[1])
	{
		for (offs = 0; offs < m_stars_rom_size / 2; offs++)
		{
			int col = stars_rom[8*offs];
			if (col != 0x0f)
			{
				int sx = (offs / 256) * 32;
				int sy = (offs % 256);
				sx = (sx - m_stars1x + (col & 0x1f)) & 0x1ff;
				sy = (sy - m_stars1y) & 0xff;
				if (flip_screen())
				{
					sx = 512 - sx;
					sy = 256 - sy;
				}

				col = ((col & 0xe0) >> 1) + (screen.frame_number() / 16 & 0x0f);

				if (cliprect.contains(sx, sy))
					bitmap.pix16(sy, sx) = 0x800 + col;
			}
		}
	}
}


void cps_state::cps1_render_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask )
{
	switch (layer)
	{
		case 0:
			cps1_render_sprites(screen, bitmap, cliprect);
			break;
		case 1:
		case 2:
		case 3:
			m_bg_tilemap[layer - 1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, primask);
			break;
	}
}

void cps_state::cps1_render_high_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer )
{
	bitmap_ind16 dummy_bitmap;
	switch (layer)
	{
		case 0:
			/* there are no high priority sprites */
			break;
		case 1:
		case 2:
		case 3:
			m_bg_tilemap[layer - 1]->draw(screen, dummy_bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
			break;
	}
}


/***************************************************************************

    Refresh screen

***************************************************************************/

UINT32 cps_state::screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layercontrol, l0, l1, l2, l3;
	int videocontrol = m_cps_a_regs[CPS1_VIDEOCONTROL];

	flip_screen_set(videocontrol & 0x8000);

	layercontrol = m_cps_b_regs[m_game_config->layer_control / 2];

	/* Get video memory base registers */
	cps1_get_video_base();

	/* Find the offset of the last sprite in the sprite table */
	cps1_find_last_sprite();

	if (m_cps_version == 2)
		cps2_find_last_sprite();

	cps1_update_transmasks();

	m_bg_tilemap[0]->set_scrollx(0, m_scroll1x);
	m_bg_tilemap[0]->set_scrolly(0, m_scroll1y);

	if (videocontrol & 0x01)    /* linescroll enable */
	{
		int scrly = -m_scroll2y;

		m_bg_tilemap[1]->set_scroll_rows(1024);

		int otheroffs = m_cps_a_regs[CPS1_ROWSCROLL_OFFS];

		for (int i = 0; i < 256; i++)
			m_bg_tilemap[1]->set_scrollx((i - scrly) & 0x3ff, m_scroll2x + m_other[(i + otheroffs) & 0x3ff]);
	}
	else
	{
		m_bg_tilemap[1]->set_scroll_rows(1);
		m_bg_tilemap[1]->set_scrollx(0, m_scroll2x);
	}
	m_bg_tilemap[1]->set_scrolly(0, m_scroll2y);
	m_bg_tilemap[2]->set_scrollx(0, m_scroll3x);
	m_bg_tilemap[2]->set_scrolly(0, m_scroll3y);


	/* Blank screen */
	if (m_cps_version == 1)
	{
		// CPS1 games use pen 0xbff as background color; this is used in 3wonders,
		// mtwins (explosion during attract), mercs (intermission).
		bitmap.fill(0xbff, cliprect);
	}
	else
	{
		// CPS2 apparently always force the background to black. Several games would
		// should a blue screen during boot if we used the same code as CPS1.
		// Maybe Capcom changed the background handling due to the problems that
		// it caused on several monitors (because the background extended into the
		// blanking area instead of going black, causing the monitor to clip).
		bitmap.fill(m_palette->black_pen(), cliprect);
	}

	cps1_render_stars(screen, bitmap, cliprect);

	/* Draw layers (0 = sprites, 1-3 = tilemaps) */
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;
	screen.priority().fill(0, cliprect);

	if (m_cps_version == 1)
	{
		if BIT(m_game_config->bootleg_kludge, 7)
			cps1_build_palette(cps1_base(CPS1_PALETTE_BASE, m_palette_align));

		cps1_render_layer(screen, bitmap, cliprect, l0, 0);

		if (l1 == 0)
			cps1_render_high_layer(screen, bitmap, cliprect, l0); /* prepare mask for sprites */

		cps1_render_layer(screen, bitmap, cliprect, l1, 0);

		if (l2 == 0)
			cps1_render_high_layer(screen, bitmap, cliprect, l1); /* prepare mask for sprites */

		cps1_render_layer(screen, bitmap, cliprect, l2, 0);

		if (l3 == 0)
			cps1_render_high_layer(screen, bitmap, cliprect, l2); /* prepare mask for sprites */

		cps1_render_layer(screen, bitmap, cliprect, l3, 0);
	}
	else
	{
		int l0pri, l1pri, l2pri, l3pri;
		int primasks[8], i;
		l0pri = (m_pri_ctrl >> 4 * l0) & 0x0f;
		l1pri = (m_pri_ctrl >> 4 * l1) & 0x0f;
		l2pri = (m_pri_ctrl >> 4 * l2) & 0x0f;
		l3pri = (m_pri_ctrl >> 4 * l3) & 0x0f;

#if 0
if (    (m_output[CPS2_OBJ_BASE /2] != 0x7080 && m_output[CPS2_OBJ_BASE /2] != 0x7000) ||
		m_output[CPS2_OBJ_UK1 /2] != 0x807d ||
		(m_output[CPS2_OBJ_UK2 /2] != 0x0000 && m_output[CPS2_OBJ_UK2 /2] != 0x1101 && m_output[CPS2_OBJ_UK2 /2] != 0x0001))
	popmessage("base %04x uk1 %04x uk2 %04x",
			m_output[CPS2_OBJ_BASE /2],
			m_output[CPS2_OBJ_UK1 /2],
			m_output[CPS2_OBJ_UK2 /2]);

if (0 && machine().input().code_pressed(KEYCODE_Z))
	popmessage("order: %d (%d) %d (%d) %d (%d) %d (%d)",l0,l0pri,l1,l1pri,l2,l2pri,l3,l3pri);
#endif

		/* take out the CPS1 sprites layer */
		if (l0 == 0) { l0 = l1; l1 = 0; l0pri = l1pri; }
		if (l1 == 0) { l1 = l2; l2 = 0; l1pri = l2pri; }
		if (l2 == 0) { l2 = l3; l3 = 0; l2pri = l3pri; }

		{
			int mask0 = 0xaa;
			int mask1 = 0xcc;
			if (l0pri > l1pri) mask0 &= ~0x88;
			if (l0pri > l2pri) mask0 &= ~0xa0;
			if (l1pri > l2pri) mask1 &= ~0xc0;

			primasks[0] = 0xff;
			for (i = 1; i < 8; i++)
			{
				if (i <= l0pri && i <= l1pri && i <= l2pri)
				{
					primasks[i] = 0xfe;
					continue;
				}
				primasks[i] = 0;
				if (i <= l0pri) primasks[i] |= mask0;
				if (i <= l1pri) primasks[i] |= mask1;
				if (i <= l2pri) primasks[i] |= 0xf0;
			}
		}

		cps1_render_layer(screen, bitmap, cliprect, l0, 1);
		cps1_render_layer(screen, bitmap, cliprect, l1, 2);
		cps1_render_layer(screen, bitmap, cliprect, l2, 4);
		cps2_render_sprites(screen, bitmap, cliprect, primasks);
	}

	return 0;
}

void cps_state::screen_eof_cps1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		/* Get video memory base registers */
		cps1_get_video_base();

		if (m_cps_version == 1)
		{
			/* CPS1 sprites have to be delayed one frame */
			memcpy(m_buffered_obj.get(), m_obj, m_obj_size);
		}
	}
}

void cps_state::cps2_set_sprite_priorities()
{
	m_pri_ctrl = m_output[CPS2_OBJ_PRI /2];
}

void cps_state::cps2_objram_latch()
{
	cps2_set_sprite_priorities();
	memcpy(m_cps2_buffered_obj.get(), cps2_objbase(), m_cps2_obj_size);
}
