// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/***********************************************************************

  Bingo Roll / Bell Star


************************************************************************


  PCB Layout (Bell Star V3):

                        ___________________________________________
   ____________________| | | | | | | | | | | | | | | | | | | | | | |____________________
  |                                  22x2 edge connector                                |
  |                     ___________   ___________   ___________                         |
  |                    |  9CANH1M  | |  9CANH1M  | |  9CANH1M  |                        |
  |                    | ULN2003AN | | ULN2003AN | | ULN2003AN |                        |
  |                    |___________| |___________| |___________|                        |
  |                                                                                     |
  |       _______       ______________________________                                  |
  |      |JM04AB |     |                              |                                 |
  |      |LM/386N|     |      P8256AH  L6140860       |                                 |
  |      |_______|     |                              |                                 |
  |                    |         Intel 1975           |                                 |
  |                    |______________________________|                                 |
  |                                                                                     |
  |         ____________    ___________     ____________    ____________                |
  |        |  9BARTCK   |  | YMZ284-D  |   | 898-3-R10K |  | 898-3-R10K |               |
  |        | SN74HCTD0N |  | 9814QAAA  |   |            |  |            |               |
  |        |____________|  |___________|   |____________|  |____________|               |
  |                                                                                     |
  | _____   _______________                                                             |
  ||     | |               |                _____________   _____________               |
  ||     | | MHS E         |               |   86AEY4K   | |   86AEY4K   |              |
  ||     | | S-82716-4     |               | SN74HCT540N | | SN74HCT540N |              |
  || (1) | | 8944          |               |_____________| |_____________|              |
  ||     | |               |                _____________   _____________    _________  |
  ||     | |               |               |   83C931K   | |   83C931K   |  | 88A59LM | |
  ||     | |               |               | SN74HCT573N | | SN74HCT573N |  | LM339N  | |
  ||     | |               |               |_____________| |_____________|  |_________| |
  ||_____| |_______________|                                                            |
  |                                                                                     |
  |         _______________                 ________________    ________________        |
  |        |               |               |  Bell Star V3  |  |  Bell Star V3  |       |
  |        | N800186-12    |               |  V.3.6.02 L    |  |  V.3.6.02 H    |       |
  |        | 9941GPA BR    |               |  TMS27C256-10  |  |  TMS27C256-10  |       |
  |        | 8944          |               |________________|  |________________|       |
  |        | AMD           |                ________________    ________________        |
  |        | INTEL '78 '82 |               |  B.S. Grafik   |  |  B.S. Grafik   |       |
  |        |               |               |  11.10.02 L    |  |  11.10.02 H    |       |
  |        |               |               |   M27C256B     |  |   M27C256B     |       |
  |        |_______________|               |________________|  |________________|       |
  |                                                                                     |
  |                                         ________________    ________________        |
  |           _____________                |    HY62256B    |  |   HY62256B     |       |
  |          |   83C931K   |               |     LP-70      |  |    LP-70       |       |
  |          | SN74HCT573N |               |  9846B KOREA   |  |   9846B KOREA  |       |
  |          |_____________|               |________________|  |________________|       |
  |                                                                                     |
  |         _____________________                                                       |
  |        |      74HCT540N      |        __________   ________   __________   _______  |
  |        | C5944ME / Hnn9931 D |       |  86A4CY  | |74HCT08N| | 06F6L9K  | |ADM690 | |
  |        |_____________________|       |SN74HCT02N| |D0945PS | |SN74HCT32N| |AN/9927| |
  |                                      |__________| |________| |__________| |_______| |
  |                                                                                     |
  |            ______________             __________    ________________                |
  |           |  PIC 16C54C  |           |          |  |                |               |
  |           |    04/P      |           |  24C04A  |  | 3.6 V. BATTERY |               |
  |           |______________|           |__________|  |________________|               |
  |                                                                                     |
  |_____________________________________________________________________________________|


  (1) Siemens HYB5141758J-60 (Germany).

  - 1x Xtal 16 MHz.
  - 1x Xtal ?? MHz.


  Etched in copper:

  "VIP-3 PALOMA ELEKTRONIK"
  "gruber electronic gmbh"
  "KMG-Layout 007051"
  "Rev 1.0"
  "0103 VIP 3"


************************************************************************

  Pinout:

  ROLLING JOKER+ / BELL STAR+

  +--------------------+----------------------+
  |        Solder      |       Component      |
  +----------------+---+----+-----------------+
  |           GND  | A |  1 |  GND            |
  |           GND  | B |  2 |  GND            |
  |           GND  | C |  3 |  GND            |
  |       +5 Volt  | D |  4 |  +5 Volt        |
  |      +12 Volt  | E |  5 |  +12 Volt       |
  |    Lamp Hold1  | F |  6 |  Lamp Start     |
  |    Lamp Hold2  | H |  7 |  Lamp Hold5     |
  |    Lamp Clear  | J |  8 |  Lamp Hold4     |
  |        Coin 1  | K |  9 |  Lamp Hold3     |
  |    Counter In  | L | 10 |  Remote         |
  |   Counter Out  | M | 11 |  Button Hold1   |
  |Counter Remote  | N | 12 |  Button Clear   |
  |  Button Hold5  | P | 13 |  Button Start   |
  |  Owner Bookk.  | R | 14 |  Rental Bookk.  |
  |  Button Hold2  | S | 15 |  Button Hold4   |
  |        Coin 2  | T | 16 |  Button Hold3   |
  |            nc  | U | 17 |  Hopper Count   |
  |            nc  | V | 18 |  Hopper Drive   |
  |   Video Green  | W | 19 |  Video Red      |
  |    Video Sync  | X | 20 |  Video Blue     |
  |  Speaker 1(+)  | Y | 21 |  Video Ground   |
  |  Credit Clear  | Z | 22 |  Speaker 1(-)   |
  +----------------+---+----+-----------------+

************************************************************************

  PCB Layout (BINGOR1):

              +--------------------------+
              ||||||||||||||||||||||||||||
  +-----------+    22x2 edge connector   +-----------+
  |                                                  |
  |                      +-----+ +-----+             |
  |                      |  M  | |  M  |             |
  |                      +-----+ +-----+             |
  |                    +-----------------+           |
  |                    | P8256AH         |           |
  |                    | L4203043        |           |
  |                    | INTEL 1975      |           |
  |                    +-----------------+           |
  |     +-----+ +-----+                              |
  |     |  L  | |  L  |                              |
  |     +-----+ +-----+                              |
  |     +-----+ +-----+                              |
  |     |  K  | |  K  |                              |
  |     +-----+ +-----+                              |
  |      +---------------------------------------+   |
  |      |   +-----+                    XTAL     |   |
  |      |   |  I  |                   .----.    |   |
  |      |   +-----+                             |   |
  |      | +--------------+                      |   |
  |      | |BINGO         |         +----------+ |   |
  |      | |V.29.4.95 H   |         |N80C186-12| |   |
  |      | |              |         |BR 9709CPA| |   |
  |      | +--------------+ +------+|AMD       | |   |
  |      | +--------------+ |  K   ||INTEL78 82| |   |
  | +--+ | |BINGO         | +------+|          | |   |
  | |  | | |V.29.4.99 L   | +------+|          | |   |
  | |J | | |              | |  I   |+----------+ |   |
  | |  | | +--------------+ +------+             |   |
  | |  | |   +-----+      +------+ +---+ +-----+ |   |
  | +--+ |   |  I  |      |  H   | | G | |  F  | |   |
  |      |   +-----+      +------+ +---+ +-----+ |   |
  |      +---------------------------------------+   |
  |                        +------+      +------+    |
  |                        |  D   |      |  E   |    |
  |                        +------+      +------+    |
  |    +----------+                                  |
  |    |   MHS    |  +------------+ +------------+   |
  |    | S-82716-4|  |  HYUNDAI   | |  HYUNDAI   |   |
  |    |          |  |HY6116ALP-10| |HY6116ALP-10|   |
  |    |     9210 |  |9306A  KOREA| |9306A  KOREA|   |
  |    |          |  +------------+ +------------+   |
  |    |          |  +------------+ +------------+   |
  |    +----------+  |BINGO       | |BINGO       |   |
  |                  |Turbo  L    | |Turbo  H    |   |
  |                  |     M27C512| |     M27C512|   |
  | +--++--++--++--+ +------------+ +------------+   |
  | |  ||  ||  ||  |          +-----+ +-------+      |
  | |C ||C ||C ||C |   ----   |  B  | |       |      |
  | |  ||  ||  ||  |  /Batt\  +-----+ +-------+      |
  | |  ||  ||  ||  | [======]           +---+        |
  | +--++--++--++--+  \ery /            | A |        |
  |                    ----             +---+        |
  +--------------------------------------------------+

  1x XTAL = 16 MHz.

  A = 9423 / LTC690 / CN8
  B = P9242 / MM74HCT08N
  C = NEC USA / D41464C-10
  D = 46COJFK / SN74HCT373N
  E = PC74HCT32P / 751180T / Hnn9145PB
  F = SN74HCT02N
  G = 24C04A / /P CFW / 9512
  H = PIC16C54B / 04/P /98245AJ
  I = SN74HCT573N
  J = SAA1099P / 232300 / DSG9336M2
  K = SN74HCT540N
  L = 898-3-R10K / 9343
  M = ULN2003A / WB91K9409 / PHIL


************************************************************************

  PCB Layout (BINGOR2):

              +--------------------------+
              ||||||||||||||||||||||||||||
  +-----------+    22x2 edge connector   +-----------+
  |                                                  |
  |                      +-----+ +-----+             |
  |                      |  M  | |  M  |             |
  |                      +-----+ +-----+             |
  |                    +-----------------+           |
  |                    | P8256AH         |           |
  |                    | L4203043        |           |
  |                    | INTEL 1975      |           |
  |                    +-----------------+           |
  |     +-----+ +-----+                              |
  |     |  L  | |  L  |                              |
  |     +-----+ +-----+                              |
  |     +-----+ +-----+                              |
  |     |  K  | |  K  |                              |
  |     +-----+ +-----+                              |
  |      +---------------------------------------+   |
  |      |   +-----+                    XTAL     |   |
  |      |   |  I  |                   .----.    |   |
  |      |   +-----+                             |   |
  |      | +--------------+                      |   |
  |      | |BINGO ROLL    |  +----+ +----------+ |   |
  |      | |Vip2 v.26.2.02|  | N  | |N80C186-12| |   |
  |      | |             H|  +----+ |BR 9709CPA| |   |
  |      | +--------------+ +------+|AMD       | |   |
  |      | +--------------+ |  K   ||INTEL78 82| |   |
  | +--+ | |BINGO ROLL    | +------+|          | |   |
  | |  | | |Vip2 v.26.2.02| +------+|          | |   |
  | |J | | |             L| |  I   |+----------+ |   |
  | |  | | +--------------+ +------+             |   |
  | |  | |   +-----+      +------+ +---+ +-----+ |   |
  | +--+ |   |  I  |      |  H   | | G | |  F  | |   |
  |      |   +-----+      +------+ +---+ +-----+ |   |
  |      +---------------------------------------+   |
  |                        +------+      +------+    |
  |                        |  D   |      |  E   |    |
  |                        +------+      +------+    |
  |    +----------+                                  |
  |    |   MHS    |  +------------+ +------------+   |
  |    | S-82716-4|  |HM6116L-70  | |HM6116L-70  |   |
  |    |          |  |9914H C9423 | |9914H C9423 |   |
  |    |     9210 |  |            | |            |   |
  |    |          |  +------------+ +------------+   |
  |    |          |  +------------+ +------------+   |
  |    +----------+  |BINGO ROLL  | |BINGO ROLL  |   |
  |                  |GRAFIK  L   | |GRAFIK  H   |   |
  |                  |            | |            |   |
  | +--++--++--++--+ +------------+ +------------+   |
  | |  ||  ||  ||  |          +-----+ +-------+      |
  | |C ||C ||C ||C |   ----   |  B  | |       |      |
  | |  ||  ||  ||  |  /Batt\  +-----+ +-------+      |
  | |  ||  ||  ||  | [======]           +---+        |
  | +--++--++--++--+  \ery /            | A |        |
  |                    ----             +---+        |
  +--------------------------------------------------+

  1x XTAL = 16 MHz.

  A = ACM690AN / 9903
  B = P9242 / MM74HCT08N
  C = NEC USA / D41464C-10
  D = 46COJFK / SN74HCT373N
  E = SN74HC132N
  F = SN74HCT02N
  G = 24C04A / /P CFW / 9512
  H = PIC16C54C / 04/P /98245AJ
  I = SN74HCT573N
  J = SAA1099P / 218080 / DSG9249M2
  K = SN74HCT540N
  L = 898-3-R10K / 9836
  M = ULN2003A / WB91K9409 / PHIL
  N = LW339N / CPB6D143


************************************************************************

  PCB Layout (BINGOR3):

              +--------------------------+
              ||||||||||||||||||||||||||||
  +-----------+    22x2 edge connector   +-----------+
  |                                                  |
  |                      +-----+ +-----+             |
  |                      |  M  | |  M  |             |
  |                      +-----+ +-----+             |
  |                    +-----------------+           |
  |                    | P8256AH         |           |
  |                    | L4203043        |           |
  |                    | INTEL 1975      |           |
  |                    +-----------------+           |
  |     +-----+ +-----+                              |
  |     |  L  | |  L  |                              |
  |     +-----+ +-----+                              |
  |     +-----+ +-----+                              |
  |     |  K  | |  K  |                              |
  |     +-----+ +-----+                              |
  |      +---------------------------------------+   |
  |      |   +-----+                    XTAL     |   |
  |      |   |  I  |                   .----.    |   |
  |      |   +-----+                             |   |
  |      | +--------------+                      |   |
  |      | |BELL STAR     |         +----------+ |   |
  |      | |VIP2H 27.07   |         |N80C186-12| |   |
  |      | |              |         |BR 9709CPA| |   |
  |      | +--------------+ +------+|AMD       | |   |
  |      | +--------------+ |  K   ||INTEL78 82| |   |
  | +--+ | |BELL STAR     | +------+|          | |   |
  | |  | | |VIP2L 27.07   | +------+|          | |   |
  | |J | | |              | |  I   |+----------+ |   |
  | |  | | +--------------+ +------+             |   |
  | |  | |   +-----+      +------+ +---+ +-----+ |   |
  | +--+ |   |  I  |      |  H   | | G | |  F  | |   |
  |      |   +-----+      +------+ +---+ +-----+ |   |
  |      +---------------------------------------+   |
  |                        +------+      +------+    |
  |                        |  D   |      |  E   |    |
  |                        +------+      +------+    |
  |    +----------+                                  |
  |    |   MHS    |  +------------+ +------------+   |
  |    | S-82716-4|  |HM6116L-70  | |HM6116L-70  |   |
  |    |          |  |9914H C9423 | |9914H C9423 |   |
  |    |     9210 |  |            | |            |   |
  |    |          |  +------------+ +------------+   |
  |    |          |  +------------+ +------------+   |
  |    +----------+  |B.S.GRAFIK  | |B.S.GRAFIK  |   |
  |                  |11.10.02  L | |11.10.02  H |   |
  |                  |            | |            |   |
  | +--++--++--++--+ +------------+ +------------+   |
  | |  ||  ||  ||  |          +-----+ +-------+      |
  | |C ||C ||C ||C |   ----   |  B  | |       |      |
  | |  ||  ||  ||  |  /Batt\  +-----+ +-------+      |
  | |  ||  ||  ||  | [======]           +---+        |
  | +--++--++--++--+  \ery /            | A |        |
  |                    ----             +---+        |
  +--------------------------------------------------+

  1x XTAL = 16 MHz.

  A = ACM690AN / 9903
  B = P9242 / MM74HCT08N
  C = NEC USA / D41464C-10
  D = 46COJFK / SN74HCT373N
  E = SN74HC132N
  F = SN74HCT02N
  G = 24C04A / /P CFW / 9512
  H = PIC16C54A / 04/P /98245AJ
  I = SN74HCT573N
  J = SAA1093P / 218080 / DSG9249M2
  K = SN74HCT540N
  L = 898-3-R10K / 9921
  M = ULN2003A / WB91K9409 / PHIL


************************************************************************

  PCB Layout (BINGOR4):

              +--------------------------+
              ||||||||||||||||||||||||||||
  +-----------+    22x2 edge connector   +-----------+
  |                                                  |
  |                      +-----+ +-----+             |
  |                      |  M  | |  M  |             |
  |                      +-----+ +-----+             |
  |                    +-----------------+           |
  |                    | P8256AH         |           |
  |                    | L4203043        |           |
  |                    | INTEL 1975      |           |
  |                    +-----------------+           |
  |     +-----+ +-----+                              |
  |     |  L  | |  L  |                              |
  |     +-----+ +-----+                              |
  |     +-----+ +-----+                              |
  |     |  K  | |  K  |                              |
  |     +-----+ +-----+                              |
  |      +---------------------------------------+   |
  |      |   +-----+                    XTAL     |   |
  |      |   |  I  |                   .----.    |   |
  |      |   +-----+                             |   |
  |      | +--------------+                      |   |
  |      | |BELL STAR     |         +----------+ |   |
  |      | |VIP2H 23.07   |         |N80C186-12| |   |
  |      | |              |         |BR 9709CPA| |   |
  |      | +--------------+ +------+|AMD       | |   |
  |      | +--------------+ |  K   ||INTEL78 82| |   |
  | +--+ | |BELL STAR     | +------+|          | |   |
  | |  | | |VIP2L 23.07   | +------+|          | |   |
  | |J | | |              | |  I   |+----------+ |   |
  | |  | | +--------------+ +------+             |   |
  | |  | |   +-----+      +------+ +---+ +-----+ |   |
  | +--+ |   |  I  |      |  H   | | G | |  F  | |   |
  |      |   +-----+      +------+ +---+ +-----+ |   |
  |      +---------------------------------------+   |
  |                        +------+      +------+    |
  |                        |  D   |      |  E   |    |
  |                        +------+      +------+    |
  |    +----------+                                  |
  |    |   MHS    |  +------------+ +------------+   |
  |    | S-82716-4|  |HM6116L-70  | |HM6116L-70  |   |
  |    |          |  |9914H C9423 | |9914H C9423 |   |
  |    |     9210 |  |            | |            |   |
  |    |          |  +------------+ +------------+   |
  |    |          |  +------------+ +------------+   |
  |    +----------+  |B.S.GRAFIK  | |B.S.GRAFIK  |   |
  |                  |11.10.02  L | |11.10.02  H |   |
  |                  |            | |            |   |
  | +--++--++--++--+ +------------+ +------------+   |
  | |  ||  ||  ||  |          +-----+ +-------+      |
  | |C ||C ||C ||C | +------+ |  B  | |       |      |
  | |  ||  ||  ||  | |      | +-----+ +-------+      |
  | |  ||  ||  ||  | |Batter|           +---+        |
  | +--++--++--++--+ |y     |           | A |        |
  |                  +------+           +---+        |
  +--------------------------------------------------+

  1x XTAL = 16 MHz.

  A = ACM690AN / 9903
  B = P9242 / MM74HCT08N
  C = NEC USA / D41464C-10
  D = 46COJFK / SN74HCT373N
  E = SN74HC132N
  F = SN74HCT02N
  G = 24C04A / /P CFW / 9512
  H = PIC16C54A / 04/P /98245AJ
  I = SN74HCT573N
  J = SAA1099P / 232230 / DSG9249M2
  K = SN74HCT540N
  L = 898-3-R10K / 9921
  M = ULN2003A / WB91K9409 / PHIL


************************************************************************

  PCB Layout (VIP2000):

     +---------------------------------------------------------------------------------+
     |                                            +--------+                           |
     |                                            |74HCT32N|                  BATTERY  |
     |                           +----+  +----+   +--------+                 3.6V 80mAh|
     |                           |HY62|  |VIP |                        +-+             |
     |              +-+ +-+ +-+  |256B|  |BIOS|    +-----+   +-+  +-+  |D|             |
     |   +-+   +-+  | | | | | |  |    |  |8 H |    |AT49F|   | |  |E|  +-+         +---+
  +--+   | |   | |  | | | | | |  |    |  |    |    |4096-|   | |  |M|  +-+   +-+   |   +-+
  |--    |K|   |A|  |B| |C| |C|  |    |  |    |    |12RC |   |C|  |P|  | |   | |   |RS232|
  |--    | |   | |  | | | | | |  |    |  |M272|    |     |   | |  |T|  |I|   |L|   |     |
  |--    | |   | |  | | | | | |  |    |  |56  |    |     |   | |  |Y|  | |   | |   |   +-+
  |--    +-+   +-+  +-+ +-+ +-+  +----+  +----+    +-----+   +-+  +-+  +-+   +-+   +---+
  |--                                                                                  |
  |--2              +-+ +-+ +-+  +----+  +----+   +-+ +-+  +-+ +----+ +-+ +----+ +----+|
  |--2   +-+   +-+  | | | | | |  |HY62|  |VIP |   | | | |  | | |    | | | |HY62| |SLAV||
  |--P   | |   | |  | | | | | |  |256B|  |BIOS|   | | | |  |M| |P80C| | | |256B| |E   ||
  |--I   |K|   |A|  |B| |C| |C|  |    |  |8 H |   |C| |C|  | | |31BH| |C| |    | |19.0||
  |--N   | |   | |  | | | | | |  |    |  |    |   | | | |  +-+ |I   | | | |    | |9.01||
  |--    | |   | |  | | | | | |  |    |  |    |   | | | |  +-+ |    | | | |    | |    ||
  |--C   +-+   +-+  +-+ +-+ +-+  |    |  |M272|   +-+ +-+  | | |    | +-+ |    | |2725||
  |--O                           |    |  |56  |            |N| |    |     |    | |6   ||
  |--N   +-+        +-+          +----+  +----+            | | |    |     +----+ +----+|
  |--N   |:|   +-+  | | +-+                                +-+ |    |                  |
  |--E   |:|   | |  | | | |                                  X |    |                  |
  |--C   |:|   |A|  |B| |H|                                  T |    |                  |
  |--T   |:|   | |  | | | |  +----------+    +----------+    A |    |                  |
  |--O   |:|   | |  | | | |  |  MHS E   | X  |N80C186-12|    L +----+    +-+           |
  |--R   |:|   +-+  +-+ +-+  | S 82716-4| T  |0037CXC BR|    3           | |  +-+  +-+ |
  |--    |:|                 |          | A  |          |                |G|  |:|  |:| |
  |--    |:|            +-+  | 8941     | L  |(c) AMD   |X     +-+  +-+  | |  |:|  |:| |
  +--+   +-+            | |  |          | 1  |          |T     |E|  |F|  | |  +-+  +-+ |
     |         +-+      |I|  |          |    |          |A     +-+  +-+  +-+           |
     |         |J|      | |  +----------+    +----------+L                    +-+ +--+ |
     |         +-+      +-+                              2     +-+  +-+  +-+  |:| |::| |
     |                       +----------+                      | |  | |  | |  |:| |::| |
     |                       |HYB514175B|                      |O|  |O|  |K|  |:| |::| |
     |                       |J-60      |                      | |  | |  | |  |:| +--+ |
     |                       +----------+       VIP 2000       +-+  +-+  +-+  +-+      |
     |                                   (c)PALOMA-ELEKTRONIK                          |
     +---------------------------------------------------------------------------------+

  XTAL1: 10.0000M
  XTAL2: R240VB40
  XTAL3: 11.0592M

  A: 898-3-R10K / 0035
  B: 74HCT540N
  C: 74HCT573N
  D: ADM690
  E: 24C04A
  F: EMPTY SOCKET
  G: HEF40106BP
  H: YMZ284
  I: 74HCT00N
  J: EMI14AB LM 386N-4
  K: ULN2003JN
  L: MAX232N
  M: 74HCT32N
  N: 74HCT02N
  O: PIC16F627-04 (protected)


************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/gen_latch.h"
#include "machine/intelfsh.h"
#include "machine/msm6242.h"
#include "sound/ay8910.h"
#include "sound/saa1099.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bingor_state : public driver_device
{
public:
	bingor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_blit_ram(*this, "blit_ram"),
		m_maincpu(*this, "maincpu"),
		m_slavecpu(*this, "slavecpu"),
		m_palette(*this, "palette")
	{ }

	void bingor(machine_config &config);
	void bingor2(machine_config &config);
	void vip2000(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint16_t> m_blit_ram;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_slavecpu;
	required_device<palette_device> m_palette;

	uint32_t screen_update_bingor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vip2000_outputs_w(uint16_t data);

	void bingor2_map(address_map &map) ATTR_COLD;
	void bingor_io(address_map &map) ATTR_COLD;
	void bingor_map(address_map &map) ATTR_COLD;
	void slave_io(address_map &map) ATTR_COLD;
	void slave_map(address_map &map) ATTR_COLD;
	void vip2000_io(address_map &map) ATTR_COLD;
	void vip2000_map(address_map &map) ATTR_COLD;
};


/************************************
*          Video Hardware           *
************************************/

void bingor_state::video_start()
{
	m_palette->basemem().set(&m_blit_ram[0x300/2], 0x10 * sizeof(uint16_t), 16, ENDIANNESS_LITTLE, 2);
}

uint32_t bingor_state::screen_update_bingor(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	int count = (0x2000 / 2);
	for(int y = 0; y < 256; y++)
	{
		for(int x = 0; x < 286; x += 4)
		{
			uint32_t color = (m_blit_ram[count] & 0xf000) >> 12;

			if(cliprect.contains(x + 3, y))
				bitmap.pix(y, x + 3) = m_palette->pen(color);

			color = (m_blit_ram[count] & 0x0f00) >> 8;

			if(cliprect.contains(x + 2, y))
				bitmap.pix(y, x + 2) = m_palette->pen(color);

			color = (m_blit_ram[count] & 0x00f0) >> 4;

			if(cliprect.contains(x + 1, y))
				bitmap.pix(y, x + 1) = m_palette->pen(color);

			color = (m_blit_ram[count] & 0x000f) >> 0;

			if(cliprect.contains(x + 0, y))
				bitmap.pix(y, x + 0) = m_palette->pen(color);

			count++;
		}
	}

	return 0;
}


/************************************
*      Memory Map Information       *
************************************/

void bingor_state::bingor_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x90000, 0x9ffff).rom().region("gfx", 0);
	map(0xa0000, 0xaffff).ram().share("blit_ram");
	map(0xa0300, 0xa031f).w(m_palette, FUNC(palette_device::write16)); //wrong
	map(0xf0000, 0xfffff).rom().region("boot_prg", 0);
}

void bingor_state::bingor2_map(address_map &map)
{
	bingor_map(map);
	map(0xe0000, 0xfffff).rom().region("boot_prg", 0); // banked?
}

void bingor_state::bingor_io(address_map &map)
{
	map(0x0100, 0x0103).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
}

void bingor_state::vip2000_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x40000, 0x4ffff).ram().share("blit_ram");
	map(0x40300, 0x4031f).w(m_palette, FUNC(palette_device::write16)); //wrong
	//map(0x50000, 0x5ffff).rom().region("gfx", 0);
	map(0x60000, 0x60003).w("ymz", FUNC(ymz284_device::address_data_w)).umask16(0x00ff);
	map(0x80000, 0xeffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0xf0000, 0xfffff).rom().region("boot_prg", 0);
}

void bingor_state::vip2000_io(address_map &map)
{
	map(0x0000, 0x0001).nopr(); // watchdog
	map(0x0080, 0x009f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write)).umask16(0x00ff);
	map(0x0100, 0x0100).r("fromslave", FUNC(generic_latch_8_device::read)).w("toslave", FUNC(generic_latch_8_device::write));
	map(0x0280, 0x0281).w(FUNC(bingor_state::vip2000_outputs_w));
}

void bingor_state::slave_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void bingor_state::slave_io(address_map &map)
{
	map(0x0000, 0x0000).r("toslave", FUNC(generic_latch_8_device::read)).w("fromslave", FUNC(generic_latch_8_device::write));
	map(0xc000, 0xcfff).ram();
}


/************************************
*           Input ports             *
************************************/

static INPUT_PORTS_START( bingor )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/************************************
*         Graphics Layouts          *
************************************/

static const gfx_layout bingor_layout =
{
	8, 8,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8 * 32
};


/************************************
*    Graphics Decode Information    *
************************************/

static GFXDECODE_START( gfx_bingor )
	GFXDECODE_ENTRY( "gfx", 0, bingor_layout, 0, 2  )
GFXDECODE_END


/**************************************
*          Machine Driver(s)          *
**************************************/

void bingor_state::bingor(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &bingor_state::bingor_map);
	m_maincpu->set_addrmap(AS_IO, &bingor_state::bingor_io);
	m_maincpu->set_periodic_int(FUNC(bingor_state::nmi_line_pulse), attotime::from_hz(30));
	// Other interrupts generated by internal timers

	PIC16C57(config, "pic", 12000000); //?? Mhz

	GFXDECODE(config, "gfxdecode", m_palette, gfx_bingor);
//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 400-1, 0, 300-1);
	screen.set_screen_update(FUNC(bingor_state::screen_update_bingor));

	PALETTE(config, m_palette).set_format(palette_device::RGBI_4444, 0x10);

	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", 6000000).add_route(ALL_OUTPUTS, "mono", 0.50);
}


void bingor_state::bingor2(machine_config &config)
{
	bingor(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bingor_state::bingor2_map);
}


void bingor_state::vip2000_outputs_w(uint16_t data)
{
	m_slavecpu->set_input_line(MCS51_INT0_LINE, BIT(data, 15) ? CLEAR_LINE : ASSERT_LINE);
}

void bingor_state::vip2000(machine_config &config)
{
	I80186(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &bingor_state::vip2000_map);
	m_maincpu->set_addrmap(AS_IO, &bingor_state::vip2000_io);
	m_maincpu->set_periodic_int(FUNC(bingor_state::nmi_line_pulse), attotime::from_hz(30));
	// Other interrupts generated by internal timers

	ATMEL_49F4096(config, "flash");

	I80C31(config, m_slavecpu, XTAL(11'059'200));
	m_slavecpu->set_addrmap(AS_PROGRAM, &bingor_state::slave_map);
	m_slavecpu->set_addrmap(AS_IO, &bingor_state::slave_io);

	MSM6242(config, "rtc", XTAL(32'768));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(400, 300);
	screen.set_visarea(0, 400-1, 0, 300-1);
	screen.set_screen_update(FUNC(bingor_state::screen_update_bingor));

	PALETTE(config, m_palette).set_format(palette_device::RGBI_4444, 0x10);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "toslave");
	GENERIC_LATCH_8(config, "fromslave");

	ymz284_device &ymz(YMZ284(config, "ymz", 1250000)); // probably clocked by square wave output of 80186 timer 0
	ymz.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/**************************************
*              ROM Load               *
**************************************/

// I doubt we need to load the EEPROMs
// for now are included till know if there are critical registers inside

ROM_START( bingor1 )    // Strings: Big Roll / Bingo Roll / 1991 AWR / 1992 Rosenauer Electronic Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo v 29.4.99 l.bin", 0x00000, 0x08000, CRC(b6773bff) SHA1(74e375662730e002e05186bd77098fa0d8e43ade) )
	ROM_LOAD16_BYTE( "bingo v 29.4.99 h.bin", 0x00001, 0x08000, CRC(0e18f90a) SHA1(0743302e675f01f8ad42ac2e67ecb1c1bf870ae7) )

	// gfx roms on this one are twice the size of the others
	ROM_REGION16_LE( 0x20000, "gfx", 0 )    // blitter data?
	ROM_LOAD16_BYTE( "bingo turbo l.bin", 0x000000, 0x10000, CRC(86b10566) SHA1(5f74b250ced3574feffdc40b6ed013ec5a0c2c97) )
	ROM_LOAD16_BYTE( "bingo turbo h.bin", 0x000001, 0x10000, CRC(7e18f9d7) SHA1(519b65d6812a3762e3215f4918c834d5a444b28a) )

	ROM_REGION( 0x20000, "pic", 0 ) // protection? (only fragment dumped)
	ROM_LOAD( "pic16c54b.bin", 0x000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) BAD_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor1_24c04a.bin", 0x000000, 0x200, CRC(b169df46) SHA1(ebafc81c6918aae9daa6b90df16161751cfd2590) )
ROM_END

ROM_START( bingor2 )    // Strings: Euro Bingo / 1988-1992 Rosenauer Electronic Austria / Paloma elektronik (c)1997.
	ROM_REGION16_LE( 0x20000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo roll vip2 v26.02.02_l.bin", 0x00000, 0x10000, CRC(40df6ee8) SHA1(1e90ef906e47f95ebde85b6dd32fdfe50c0564fc) )
	ROM_LOAD16_BYTE( "bingo roll vip2 v26.02.02_h.bin", 0x00001, 0x10000, CRC(9154c183) SHA1(a4060294295a9b8df07ce9fcfeefcf009e129817) )

	ROM_REGION16_LE( 0x20000, "gfx", 0 )    // blitter data?
	ROM_LOAD16_BYTE( "bingo roll grafik l.bin", 0x000000, 0x10000, CRC(3e753e13) SHA1(011b5f530e54332be194830c0a1d2ec31425017a) )
	ROM_LOAD16_BYTE( "bingo roll grafik h.bin", 0x000001, 0x10000, CRC(4eec39ad) SHA1(4201d5ec207d30dcac9813dd6866d2b61c168e75) )

	ROM_REGION( 0x20000, "pic", 0 ) // protection (only fragment dumped)
	ROM_LOAD( "pic16c54c.bin", 0x000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) BAD_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor2_24c04a.bin", 0x000000, 0x200, CRC(a7c87036) SHA1(f7d6161bbfdcdc50212f6b71eb2cbbbb18548cc6) )
ROM_END

ROM_START( bingor3 )    // Strings: 1995-1997 Paloma Elektronik / 2002 Paloma Elektronik / Play Star Austria 0316/821193.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bellstar vip2l 27.07_1.bin", 0x00000, 0x08000, CRC(0115bca7) SHA1(0b692b46bc6641296861666f00ec0475dc7296a1) )
	ROM_LOAD16_BYTE( "bellstar vip2l 27.07_2.bin", 0x00001, 0x08000, CRC(c689aa69) SHA1(fb1f477654909f156c30a6be29f84962f4edb1c3) )

	ROM_REGION16_LE( 0x20000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bsg-11.10.02_l.bin", 0x000000, 0x08000, CRC(a8b22477) SHA1(92d638f0f188a43f14487989cf42195311fb2c35) ) //half size?
	ROM_LOAD16_BYTE( "bsg-11.10.02_h.bin", 0x000001, 0x08000, CRC(969d201c) SHA1(7705ceb383ef122538ebf8046041d1c24ec9b9a4) )

	ROM_REGION( 0x20000, "pic", 0 ) // protection (only fragment dumped)
	ROM_LOAD( "pic16c54c.bin", 0x000, 0x400, CRC(5a507be6) SHA1(f4fbfb7e7516eecab32d96b3a34ad88395edac9e) BAD_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor3_24c04a.bin",     0x000000, 0x200, CRC(7a5eb172) SHA1(12d2fc96049427ef1a8acf47242b41b2095d28b6) )
	ROM_LOAD( "bingor3_24c04a_alt.bin", 0x000000, 0x200, CRC(fcff2d26) SHA1(aec1ddd38149404741a057c74bf84bfb4a8e4aa1) )
ROM_END

// this is a mix of 2 of the other sets.. I don't know if it's correct
ROM_START( bingor4 )    // Strings: 1995-1997 Paloma Elektronik / 2002 Paloma Elektronik / Play Star Austria 0316/821193 / CBA Design, Lyon France.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "01.bin", 0x00000, 0x08000, CRC(0115bca7) SHA1(0b692b46bc6641296861666f00ec0475dc7296a1) )
	ROM_LOAD16_BYTE( "02.bin", 0x00001, 0x08000, CRC(c689aa69) SHA1(fb1f477654909f156c30a6be29f84962f4edb1c3) )

	ROM_REGION16_LE( 0x20000, "gfx", 0 )    // blitter data?
	ROM_LOAD16_BYTE( "bingo roll grafik l.bin", 0x000000, 0x10000, CRC(3e753e13) SHA1(011b5f530e54332be194830c0a1d2ec31425017a) )
	ROM_LOAD16_BYTE( "bingo roll grafik h.bin", 0x000001, 0x10000, CRC(4eec39ad) SHA1(4201d5ec207d30dcac9813dd6866d2b61c168e75) )

	ROM_REGION( 0x20000, "pic", 0 ) // protection? (only fragment dumped)
	ROM_LOAD( "pic16c54c.bin", 0x000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) BAD_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor4_24c04a.bin", 0x000000, 0x200, CRC(38cf70a9) SHA1(ba9a1640200963e2d58d761edc13a24fa5ef44c2) )
ROM_END

/*

  BellStar V3
  Same as bingor3, but with different program ROMs

*/

ROM_START( bingor5 )    // BellStar V3.  Strings: 1995-1997 Paloma Elektronik / 2002 Paloma Elektronik / Play Star Austria 0316/821193 / CBA Design, Lyon France.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bellstar_v3_v.3.6.02_l.bin", 0x00000, 0x08000, CRC(56b84a5d) SHA1(1bda4fb972b4f0f0575089b545bf15dfea859948) )
	ROM_LOAD16_BYTE( "bellstar_v3_v.3.6.02_h.bin", 0x00001, 0x08000, CRC(d6945bb8) SHA1(b620f1b547be03c4609bff8d06111d0ea425bae8) )

	ROM_REGION16_LE( 0x20000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "b.s.grafik_11.10.02_l.bin", 0x000000, 0x08000, CRC(a8b22477) SHA1(92d638f0f188a43f14487989cf42195311fb2c35) ) //half size?
	ROM_LOAD16_BYTE( "b.s.grafik_11.10.02_h.bin", 0x000001, 0x08000, CRC(969d201c) SHA1(7705ceb383ef122538ebf8046041d1c24ec9b9a4) )

	ROM_REGION( 0x20000, "pic", 0 ) // protection? (only fragment dumped)
	ROM_LOAD( "pic16c54c.bin", 0x000, 0x400, CRC(5a507be6) SHA1(f4fbfb7e7516eecab32d96b3a34ad88395edac9e) BAD_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor5_24c04a.bin", 0x000000, 0x200, CRC(fcff2d26) SHA1(aec1ddd38149404741a057c74bf84bfb4a8e4aa1) )
ROM_END

/* Bingo Roll / Turbo Bingo?
   1991,
   AWR Hard & Soft
   Austria.

   Same PCB layout as bingor3,
   but 2x crystals:

   1x 16 MHz. (sub board)
   1x 10 MHz. (main board)

   1x MP690P

*/
ROM_START( bingor6 )    // Strings: 1988-1992 AWR Hard & Soft Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo_l.bin", 0x00000, 0x08000, CRC(78df905d) SHA1(197c5a795bdda964db780dd3b9c0f9b76197a610) )
	ROM_LOAD16_BYTE( "bingo_h.bin", 0x00001, 0x08000, CRC(f25958fd) SHA1(8a8c061accc4c2fa8475188a1281518e63fb456d) )

	ROM_REGION16_LE( 0x20000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "turbo_l.bin", 0x000000, 0x10000, CRC(86b10566) SHA1(5f74b250ced3574feffdc40b6ed013ec5a0c2c97) )
	ROM_LOAD16_BYTE( "turbo_h.bin", 0x000001, 0x10000, CRC(7e18f9d7) SHA1(519b65d6812a3762e3215f4918c834d5a444b28a) )

	ROM_REGION( 0x1fff, "pic", 0 ) // decapped, protection?
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, CRC(857e8f5a) SHA1(59666b76ec87f3a736f6c7c8f43e63913f53fbfd) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "bingor6_24c04a.bin", 0x000000, 0x200, CRC(9d271c5f) SHA1(8ac5c4848fb8d9a156ba760324022839fefcbb72) )
ROM_END

/* Bingo Roll (209 / 19-1-95)
   Rosenauer Electronic Austria?.

   Same PCB layout than bingor2, 3, 4

*/
ROM_START( bingor7 )  // Strings: (c) 1988, 1989, 1990, 1991, 1992 Rosenauer Electronic Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo_10.12.94_l.bin", 0x00000, 0x08000, CRC(4cde333a) SHA1(60e2ad9a5b6131fc12de72714e72dc016f3af6de) )
	ROM_LOAD16_BYTE( "bingo_19.1.95_h.bin",  0x00001, 0x08000, CRC(3cc1702d) SHA1(e9489978d3780af0878bc7f047e0ab28a8ddce5d) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "turbo_l.bin", 0x000000, 0x08000, CRC(15715c10) SHA1(1a7506ce3b9ddbacebb5a4501ed19f8fdfb3029b) )
	ROM_LOAD16_BYTE( "turbo_h.bin", 0x000001, 0x08000, CRC(f0564e9e) SHA1(f322cf4587f56db15052a437d6cfc5c7cc6999ab) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need decap
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(6f0daa7f) SHA1(98bffadb0fcc983d451b859cf73c619909daa6d8) )
ROM_END

/* Bingo Roll 2001 (v22.1)
   Rosenauer Electronic Austria?.

   Same PCB layout than bingor2, 3, 4

*/
ROM_START( bingor8 )  // Strings: (c) 1988, 1989, 1990, 1991, 1992 Rosenauer Electronic Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo_roll_2001_v_22.1_l.bin", 0x00000, 0x08000, CRC(2b36d5ce) SHA1(bdb9a14d53fa0c662eee7ab0f0dd56e0b66b33ab) )
	ROM_LOAD16_BYTE( "bingo_roll_2001_v_22.1_h.bin", 0x00001, 0x08000, CRC(23d148f5) SHA1(43a4409727b8caa0d8b12c35b6f85099134af4a6) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bingo_roll_grafik_l.bin", 0x000000, 0x08000, CRC(15715c10) SHA1(1a7506ce3b9ddbacebb5a4501ed19f8fdfb3029b) )
	ROM_LOAD16_BYTE( "bingo_roll_grafik_h.bin", 0x000001, 0x08000, CRC(f0564e9e) SHA1(f322cf4587f56db15052a437d6cfc5c7cc6999ab) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need verification
	ROM_LOAD( "pic16c54.bin", 0x0000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(8dc17c0a) SHA1(bb5c9c013c324089e22dc8ef1b11172d6f38cf64) )
ROM_END

/* Bingo Roll (Vip2 v26.02.02).
   Rosenauer Electronic Austria.

   Same PCB layout than bingor2, 3, 4

*/
ROM_START( bingor9 )  // Strings: (c) 1988, 1989, 1990, 1991, 1992 Rosenauer Electronic Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bingo_roll_vip2_v26.02.02_l.bin", 0x00001, 0x08000, CRC(995dbf58) SHA1(f628cd6b5bdc35839045cbffb9df3a6a731cc465) )
	ROM_LOAD16_BYTE( "bingo_roll_vip2_v26.02.02_h.bin", 0x00000, 0x08000, CRC(1fd01166) SHA1(063240ea20303edfb1744abddac63587d6f40db7) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bingo_roll_+_euro_grafik_l.bin", 0x000000, 0x08000, CRC(15715c10) SHA1(1a7506ce3b9ddbacebb5a4501ed19f8fdfb3029b) )
	ROM_LOAD16_BYTE( "bingo_roll_+_euro_grafik_h.bin", 0x000001, 0x08000, CRC(f0564e9e) SHA1(f322cf4587f56db15052a437d6cfc5c7cc6999ab) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need verification
	ROM_LOAD( "pic16c54.bin", 0x0000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin",     0x000000, 0x200, CRC(116ef31e) SHA1(8f9e47e4e70fa4831e257d33e7b17f556887abe8) )
	ROM_LOAD( "24c04a_alt.bin", 0x000200, 0x200, CRC(a5c37a1e) SHA1(3bf9e26b1c4c13679d9cb4f2dfdf2e27b7187aba) )  // from another identical set.
ROM_END


/* Bell-Star+ 5 (Vip2 v.3.6.02)
   2002 Paloma Elektronik.

   Play Star Austria 0316/821193.
   CBA-Design, Lyon France.
   C1992, AWR.

   Same PCB layout than bingor2, 3, 4

   Serial: 9851.
*/
ROM_START( bellstr1 )  // Strings: C1992, AWR. Copyright (c) 1995-1997 Paloma Elektronik.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bell-star+_5_vip_2_v.3.6.02_l.bin", 0x00000, 0x08000, CRC(0115bca7) SHA1(0b692b46bc6641296861666f00ec0475dc7296a1) )
	ROM_LOAD16_BYTE( "bell-star+_5_vip_2_v.3.6.02_h.bin", 0x00001, 0x08000, CRC(c689aa69) SHA1(fb1f477654909f156c30a6be29f84962f4edb1c3) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "grafik_l.bin", 0x000000, 0x08000, CRC(a8b22477) SHA1(92d638f0f188a43f14487989cf42195311fb2c35) )
	ROM_LOAD16_BYTE( "grafik_h.bin", 0x000001, 0x08000, CRC(969d201c) SHA1(7705ceb383ef122538ebf8046041d1c24ec9b9a4) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need verification
	ROM_LOAD( "pic16c54.bin", 0x0000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(6278b83b) SHA1(a66dcfc6c8533c11e374a5015a2507267187b241) )
ROM_END

/* Bell-Star+ (Vip2 v.25.02.02)
   2002 Paloma Elektronik.

   Play Star Austria 0316/821193.
   CBA-Design, Lyon France.
   C1992, AWR.

   Same PCB layout than bingor2, 3, 4

   Serial: 9843.
*/
ROM_START( bellstr2 )  // Strings: C1992, AWR. Copyright (c) 1995-1997 Paloma Elektronik.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bell-star+_vip2_25.02.02_l.bin",  0x00000, 0x08000, CRC(82ff28c5) SHA1(b0f7c47f32457ebb7783acb94a2ee7d60dbf000f) )
	ROM_LOAD16_BYTE( "bell-star+_vip2_25.02.02_h.bin",  0x00001, 0x08000, CRC(b6243a72) SHA1(2d0f5222a1908076658614035c96a69a579ee3a2) )
//  ROM_LOAD16_BYTE( "bell-star+_vip2_v_25.2.02_h.bin", 0x00001, 0x08000, CRC(c9ab19a0) SHA1(e65a4923a8efb0847b5156b68dbf9f2e5aedfecb) )

	ROM_REGION16_LE( 0x10000, "alt_h", 0 ) // from identical set, but different high program (to check).
	ROM_LOAD16_BYTE( "bell-star+_vip2_v_25.2.02_h.bin", 0x00001, 0x08000, CRC(c9ab19a0) SHA1(e65a4923a8efb0847b5156b68dbf9f2e5aedfecb) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bell-star+_grafik_l.bin", 0x000000, 0x08000, CRC(2c179e12) SHA1(2c3ea8073ab416530c2e7060c50bfd1a3fc6977e) )
	ROM_LOAD16_BYTE( "bell-star+_grafik_h.bin", 0x000001, 0x08000, CRC(ee441cc7) SHA1(18c114cb3e0d7acd59ef914edbe4ebfe979e0841) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need verification
	ROM_LOAD( "pic16c54.bin", 0x0000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(179a39ed) SHA1(4f6a1261efba43cb822f4752dfca8b8a62c58be6) )
	ROM_LOAD( "24c04a_alt.bin", 0x000000, 0x200, CRC(ef197aae) SHA1(bc6766e7e80122b4aaf7c4ffb7dd04b1cb8ba308) )  // from an identical set.
ROM_END

/* Bell-Star+ (Vip2 v.30.1.02)
   2002 Paloma Elektronik.

   Play Star Austria 0316/821193.
   CBA-Design, Lyon France.

   Same PCB layout than bingor2, 3, 4

   Serial: OK.
*/
ROM_START( bellstr3 )  // Strings: Copyright (c) 1995-1997 Paloma Elektronik.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bell-star+_vip2_v.30.1.02_l.bin",  0x00000, 0x08000, CRC(18cd960a) SHA1(16e3e76a47f1f48da4a88faccad8009e4f2be15b) )
	ROM_LOAD16_BYTE( "bell-star+_vip2_v.30.1.02_h.bin",  0x00001, 0x08000, CRC(4ffbfd8c) SHA1(d62080a8db27242603c455ee5eb34670d73b9b36) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bell_star_euro_zg_l.bin", 0x000000, 0x08000, CRC(cc3bfd3e) SHA1(bf9e0a04203c5b9a4150b17a6e01198e4cccb8e4) )
	ROM_LOAD16_BYTE( "bell_star_euro_zg_h.bin", 0x000001, 0x08000, CRC(3d7ee09c) SHA1(4eb297b19eb22c82725d61b7ac01f123fedf6637) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need verification
	ROM_LOAD( "pic16c54.bin", 0x0000, 0x200, CRC(21e8a699) SHA1(8a22292fa3669105d52a9d681d5be345fcfe6607) )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(26ca6607) SHA1(e6f162481bf2e7196f06239e09ed81bb14e99f1e) )
ROM_END


/* Bell Star Plus
   1998 Paloma Elektronik.

   Play Star Austria 0316/821193.
   CBA-Design, Lyon France.

   Same PCB layout than bingor2, 3, 4

   Serial: 9837
*/
ROM_START( bellstrp )  // Strings: Copyright (c) 1998 Paloma Elektronik. Play Star Austria.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "bellstar+_l.bin", 0x00000, 0x08000, CRC(3fba44ba) SHA1(00d7dae8d3c348dc9dcba4e8a8c6ff9c905471c5) )
	ROM_LOAD16_BYTE( "bellstar+_h.bin", 0x00001, 0x08000, CRC(c075a069) SHA1(04be952e9c76a43363db7f1f81ad8ba36e925e11) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "bell_star_plus_-_zgs_l.bin", 0x000000, 0x08000, CRC(2dca3f89) SHA1(dc9ed2793601b161e0afd64d2f5752a9cd2204e7) )
	ROM_LOAD16_BYTE( "bell_star_plus_-_zgs_h.bin", 0x000001, 0x08000, CRC(fc863c7d) SHA1(487fbbbf7e0be3261e33e81476532ced7c5d9c87) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need decap
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(76f2281a) SHA1(bbce75582533747b1eb188ebc1af5eec6075e637) )
ROM_END


/* Vip 2000 (set 1)
   2004 Paloma Elektronik.

*/
ROM_START( vip2000 ) // Strings: (c) 2004 Paloma Elektronik. V24.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "vipbios8l.bin", 0x00000, 0x08000, CRC(a4c2b351) SHA1(bb718584bfe32b9ed27fadfd89b4094d4bbd6d7f) )
	ROM_LOAD16_BYTE( "vipbios8h.bin", 0x00001, 0x08000, CRC(7c42c5ee) SHA1(c419a834ddb245363bacfe70d31cff7c2d4a2d03) )

	ROM_REGION16_BE( 0x80000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "at49f4096.bin", 0x00000, 0x80000, CRC(1d0fd3cf) SHA1(0ad76ea7efa31049a73cc336130cb5ca15480edd) )

	ROM_REGION( 0x08000, "slavecpu", 0 )
	ROM_LOAD( "slave190991.bin", 0x0000, 0x8000, CRC(67feb297) SHA1(442b62e62b614bda2d277e4b827cb89677d6fbce) )

	ROM_REGION( 0x00800, "pic1", 0 )
	ROM_LOAD( "pic16f627_a.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x00800, "pic2", 0 )
	ROM_LOAD( "pic16f627_b.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )
	ROM_LOAD( "24c04a.bin", 0x000, 0x200, CRC(4e231420) SHA1(24dcfc90ef9903692030be7de0f04fc9370021fd) )
ROM_END

/* Vip 2000 (set 2)
   Awrbell Euro 2003.
   Awrseven Card +.

   2001 Paloma Elektronik.
   Play Star Austria 0316/821193.

   Identical to set 3, but different flash ROM and EEPROM.
*/
ROM_START( vip2000a ) // Strings: Awrbell Euro 2003. (c) 2001 Paloma Elektronik.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "vip_bios_5_l.c11", 0x00000, 0x08000, CRC(80172456) SHA1(eca7ebc4410b4e39a459079a8fbaae73ad88acbe) )
	ROM_LOAD16_BYTE( "vip_bios_5_h.c12", 0x00001, 0x08000, CRC(2e871263) SHA1(a12ce20535a7815c578982869d7e2a9517c87543) )

	ROM_REGION16_BE( 0x80000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "at49f4096.bin", 0x00000, 0x80000, CRC(dea09922) SHA1(fe320dfe3c1e7ce360d44771135e0c4616ea39b7) )

	ROM_REGION( 0x08000, "slavecpu", 0 )
	ROM_LOAD( "slave_19.09.01.c34", 0x0000, 0x8000, CRC(67feb297) SHA1(442b62e62b614bda2d277e4b827cb89677d6fbce) )

	ROM_REGION( 0x00800, "pic1", 0 )
	ROM_LOAD( "pic16f627_a.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x00800, "pic2", 0 )
	ROM_LOAD( "pic16f627_b.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )
	ROM_LOAD( "st24c04.bin", 0x000, 0x200, CRC(752a960c) SHA1(de13943ea2015a0a57b5cc30a4ec4737b6e41fd0) )
ROM_END

/* Vip 2000 (set 3)
   Awrbell Euro 2003.
   Awrseven Card +.

   2001 Paloma Elektronik.
   Play Star Austria 0316/821193.

   Identical to set 2, but different flash ROM and EEPROM.
*/
ROM_START( vip2000b ) // Strings: Awrbell Euro 2003. (c) 2001 Paloma Elektronik.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "vip_bios_5_l.ic11", 0x00000, 0x08000, CRC(80172456) SHA1(eca7ebc4410b4e39a459079a8fbaae73ad88acbe) )
	ROM_LOAD16_BYTE( "vip_bios_5_h.ic12", 0x00001, 0x08000, CRC(2e871263) SHA1(a12ce20535a7815c578982869d7e2a9517c87543) )

	ROM_REGION16_BE( 0x80000, "flash", ROMREGION_ERASE00 )
	ROM_LOAD( "at49f4096.ic13", 0x00000, 0x80000, CRC(57a3f487) SHA1(270f110617c868557c5e72639e8dc2641caeb1f3) )

	ROM_REGION( 0x08000, "slavecpu", 0 )
	ROM_LOAD( "slave_19.09.01.ic34", 0x0000, 0x8000, CRC(67feb297) SHA1(442b62e62b614bda2d277e4b827cb89677d6fbce) )

	ROM_REGION( 0x00800, "pic1", 0 )
	ROM_LOAD( "pic16f627_a.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x00800, "pic2", 0 )
	ROM_LOAD( "pic16f627_b.bin", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )
	ROM_LOAD( "24c04a.bin", 0x000, 0x200, CRC(2da36c6a) SHA1(002a6e35fb21d3722d40b6c463bda95fa8a9160f) )
ROM_END


/* Rolling Joker
   1998 Paloma Elektronik.

   Same PCB layout than bingor2, 3, 4

   Serial: 9813
*/
ROM_START( roljokr1 )  // Strings:
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "rol_l.bin", 0x00000, 0x08000, CRC(df4b5758) SHA1(88dfc7a623e4f200d4c90e6118e1a101242cb8ab) )
	ROM_LOAD16_BYTE( "rol_h.bin", 0x00001, 0x08000, CRC(02bf6d89) SHA1(57f0bfeb6ad579b7bb0a022b2c1acd217ccae995) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "rolling_joker_grafik_l.bin", 0x000000, 0x08000, CRC(82069002) SHA1(7b5717b79a691e6ba13664f6fa275fcdda449e9f) )
	ROM_LOAD16_BYTE( "rolling_joker_grafik_h.bin", 0x000001, 0x08000, CRC(3bc2e264) SHA1(25d35a399b3f60dd37b12461eede21c0a3fb5fb2) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need decap
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(a40d8847) SHA1(9856a70a2def59057129fa84dc1a446cdb8e0b76) )
ROM_END

/* Rolling Joker (v23.11.91)
   1997 Paloma Elektronik.
   CBA-Design, Lyon France.

   Same PCB layout than bingor2, 3, 4

   Serial: 9813
*/
ROM_START( roljokr2 )  // Strings: 1993-1997 Karmen International. (c) 1997 Paloma elektronik. CBA-Design, Lyon France.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "rolling_joker_v23.11.91_l.bin", 0x00000, 0x08000, CRC(fbc47b7f) SHA1(d672835e6a05606aee741514994f93d45ef8cd57) )
	ROM_LOAD16_BYTE( "rolling_joker_v23.11.91_h.bin", 0x00001, 0x08000, CRC(9fecbda3) SHA1(22c1f03197034e36f2881b784965a81b461d5427) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "rolling_joker_grafik_l.bin", 0x000000, 0x08000, CRC(82069002) SHA1(7b5717b79a691e6ba13664f6fa275fcdda449e9f) )
	ROM_LOAD16_BYTE( "rolling_joker_grafik_h.bin", 0x000001, 0x08000, CRC(3bc2e264) SHA1(25d35a399b3f60dd37b12461eede21c0a3fb5fb2) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need decap
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(50386e87) SHA1(5f179cc3c5ff43957b2489b97c4227ee9bf16fc2) )
ROM_END

/* Rolling Joker (v.99.7)
   1999 Paloma Elektronik.
   CBA-Design, Lyon France.

   Same PCB layout than bingor2, 3, 4

   Serial: 9730
*/
ROM_START( roljokr3 )  // Strings: (c) 1999 Paloma elektronik. CBA-Design, Lyon France.
	ROM_REGION16_LE( 0x10000, "boot_prg", ROMREGION_ERASE00 )   // i186 code
	ROM_LOAD16_BYTE( "rolling_joker_v.99.7_l.bin", 0x00000, 0x08000, CRC(4f9aca58) SHA1(e524b85d30b6625eec4afd9b19023e6baca1d5f3) )
	ROM_LOAD16_BYTE( "rolling_joker_v.99.7_h.bin", 0x00001, 0x08000, CRC(2fe68133) SHA1(c44c778b5495c9545ca77efec1cb875844c0cbb3) )

	ROM_REGION16_LE( 0x10000, "gfx", ROMREGION_ERASE00 )    // blitter data?
	ROM_LOAD16_BYTE( "rolling_joker_grafik_l.bin", 0x000000, 0x08000, CRC(82069002) SHA1(7b5717b79a691e6ba13664f6fa275fcdda449e9f) )
	ROM_LOAD16_BYTE( "rolling_joker_grafik_h.bin", 0x000001, 0x08000, CRC(3bc2e264) SHA1(25d35a399b3f60dd37b12461eede21c0a3fb5fb2) )

	ROM_REGION( 0x1fff, "pic", 0 ) // need decap
	ROM_LOAD( "pic16c54rc.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x20000, "eeprom", 0 )  // eeprom
	ROM_LOAD( "24c04a.bin", 0x000000, 0x200, CRC(33cb4f30) SHA1(bd48d4194237c60bdc0108daf26a8689428b38f0) )
ROM_END


} // anonymous namespace


/**************************************
*           Game Driver(s)            *
**************************************/
// parent-clone relationship will be set as soon as we have the sets working and could corroborate the gamenames/versions.

//    YEAR  NAME      PARENT   MACHINE   INPUT   STATE         INIT        ROT    COMPANY                          FULLNAME                             FLAGS
GAME( 1992, bingor1,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Rosenauer Electronic Austria?", "Bingo Roll / Bell Star? (set 1)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1992, bingor2,  0,       bingor2,  bingor, bingor_state, empty_init, ROT0, "Rosenauer Electronic Austria?", "Bingo Roll / Bell Star? (set 2)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, bingor3,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bingo Roll / Bell Star? (set 3)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, bingor4,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bingo Roll / Bell Star? (set 4)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, bingor5,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bingo Roll / Bell Star V3? (set 5)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1991, bingor6,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "AWR Hard & Soft Austria?",      "Bingo Roll / Turbo Bingo?",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1995, bingor7,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Rosenauer Electronic Austria",  "Bingo Roll (209 / 19-1-95)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, bingor8,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Rosenauer Electronic Austria",  "Bingo Roll 2001 (v22.1)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1992, bingor9,  0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Rosenauer Electronic Austria",  "Bingo Roll (Vip2 v26.02.02)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2002, bellstr1, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bell-Star+ 5 (Vip2 v.3.6.02)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, bellstr2, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bell-Star+ (Vip2 v.25.02.02)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, bellstr3, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bell-Star+ (Vip2 v.30.1.02)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 1998, bellstrp, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Bell Star Plus",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 2004, vip2000,  0,       vip2000,  bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "unknown 'VIP 2000' game (set 1)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, vip2000a, 0,       vip2000,  bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "unknown 'VIP 2000' game (set 2)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, vip2000b, 0,       vip2000,  bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "unknown 'VIP 2000' game (set 3)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

GAME( 1999, roljokr1, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Rolling Joker",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, roljokr2, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Rolling Joker (v23.11.91)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, roljokr3, 0,       bingor,   bingor, bingor_state, empty_init, ROT0, "Paloma Elektronik?",            "Rolling Joker (v.99.7)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
