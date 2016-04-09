// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Andreas Naive, Charles MacDonald
/***************************************************************************

    Hitachi FD1089FD1094 encryption emulation

****************************************************************************

    Copyright Nicola Salmoria, Andreas Naive, and Charles MacDonald.

    All rights reserved.

****************************************************************************

    The FD1094 is a custom CPU based on the 68000, which runs encrypted code.
    The decryption key is stored in 8KB of battery-backed RAM; when the battery
    dies, the CPU can no longer decrypt the program code and the game stops
    working (though the CPU itself still works - it just uses a wrong decryption
    key).

    Being a 68000, the encryption works on 16-bit words. Only words fetched from
    program space are decrypted; words fetched from data space are not affected.

    The decryption can logically be split in two parts. The first part consists
    of a series of conditional XORs and bitswaps, controlled by the decryption
    key, which will be described in the next paragraph. The second part does a replacement
    of several values with FFFF. This last step is done to prevent usage of any
    PC-relative opcode, which would easily allow an intruder to dump decrypted
    values from program space. The FFFF replacement may affect either ~300 values
    or ~5000, depending on the decryption key.

    The main part of the decryption can itself be subdivided in five consecutive
    steps. The first one is executed only if bit 15 of the encrypted value is 1;
    the second one only if bit 14 of the _current_ value is 1; the third one only
    if bit 13 of the current value is 1; the fourth one is executed whenever one
    of the first three has been executed; the fifth one is always executed. Every
    step can be thought as consisting of a serie of operations, with some steps
    avoiding some of them: a unconditional bitswap, some conditional XORs,
    a unconditional XOR and some conditional bitswaps.

    In the end, the decryption of a value at a given address is controlled by 32
    boolean variables; 8 of them change at every address (repeating after 0x2000
    words), and constitute the main key which is stored in the battery-backed
    RAM; the other 24 don't change with the address, and depend solely on bytes
    1, 2, and 3 of the battery-backed RAM, modified by the "state" which the CPU
    is in.

    The CPU can be in one of 256 possible states. The 8 bits of the state modify
    the 24 bits of the global key in a fixed way, which isn't affected by the
    battery-backed RAM.
    On reset, the CPU goes in state 0x00. The state can then be modified by the
    program, executing the instruction
    CMPI.L  #$00xxFFFF, D0
    where xx is the state.
    When an interrupt happens, the CPU enters "irq mode", forcing a specific
    state, which is stored in byte 0 of the battery-backed RAM. Irq mode can also
    be selected by the program with the instruction
    CMPI.L  #$0200FFFF, D0
    When RTE is executed, the CPU leaves irq mode, restoring the previous state.
    This can also be done by the program with the instruction
    CMPI.L  #$0300FFFF, D0

    Since bytes 0-3 of the battery-backed RAM are used to store the irq state and
    the global key, they have a double use: this one, and the normal 8-bit key
    that changes at every address. To prevent that double use, the CPU fetches
    the 8-bit key from a different place when decrypting words 0-3, but this only
    happens after wrapping around at least once; when decrypting the first four
    words of memory, which correspond to the initial SP and initial PC vectors,
    the 8-bit key is taken from bytes 0-3 of RAM. Instead, when fetching the
    vectors, the global key is handled differently, to prevent double use of
    those bytes. But this special handling of the global key doesn't apply to
    normal operations: reading words 1-3 from program space results in bytes 1-3
    of RAM being used both for the 8-bit key and for the 24-bit global key.



    There is still uncertainty about the assignment of two global key bits.

    key[1]
    ------
    key_0b invert;  \ bits 7,5 always 1 for now (but 0 in a bad CPU)
    global_xor0;    /
    key_5b invert;  bit 6
    key_2b invert;  bit 4
    key_1b invert;  bit 3 always 1 for now (but 0 in a bad CPU)
    global_xor1;    bit 2
    key_0c invert;  bit 1
    global_swap2;   bit 0

    key[2]
    ------
    key_1a invert;  bit 7 always 1 for now (but 0 in a bad CPU)
    key_6b invert;  bit 6 always 1 for now (but 0 in a bad CPU)
    global_swap0a;  bit 5
    key_7a invert;  bit 4
    key_4a invert;  bit 3
    global_swap0b;  bit 2
    key_6a invert;  bit 1
    key_3a invert;  bit 0

    key[3]
    ------
    key_2a invert;  bit 7 always 1 for now (but 0 in a bad CPU)
    global_swap3;   bit 6 always 1 for now (but 0 in a bad CPU)
    key_5a_invert;  bit 5
    global_swap1;   bit 4
    key_3b invert;  bit 3
    global_swap4;   bit 2
    key_0a invert;  bit 1
    key_4b invert;  bit 0


    Analysis of the data contained in the 8k key data indicates some regularities.
    To begin with, in all the keys seen so far, bit 7 ($80) in key values at
    addresses $0004-$0FFF is always set to 1. Similarly, bit 6 ($40) in key values
    at addresses $1000-$1FFF is always set to 1.

    Even more interesting, however, is that analyzing the low 6 bits of the key
    data reveals that a simple linear congruential generator has been used
    consistently to generate the key bits. The LCG is of the form:

        temp = A * val;
        val' = temp + (temp << 16);

    and it appears to be calculated to at least 22 bits. In all cases seen so far,
    the value of 'A' is fixed at $29. To generate the low 6 bits of the key, the
    result of the LCG is shifted right 16 bits and inverted.

    The following pseudo-code will generate 7 of the 8 bits of the key data
    successfully for all known keys, given the values of the 'shift' and 'B'
    parameters, as well as an initial 'seed' for the generator:

    void genkey(UINT32 seed, UINT8 *output)
    {
        int bytenum;

        for (bytenum = 4; bytenum < 8192; bytenum++)
        {
            UINT8 byteval;

            seed = seed * 0x29;
            seed += seed << 16;

            byteval = (~seed >> 16) & 0x3f;
            byteval |= (bytenum < 0x1000) ? 0x80 : 0x40;

            output[bytenum] = byteval;
        }
    }

    This only leaves one bit per key value (and the global key) left to determine.
    It is worth pointing out that this remaining bit is the same bit that controls
    how many opcodes to blank to $FFFF: 0 means a smaller subset (~300), while 1
    indicates a much larger subset (~5000). Looking at the correlations between
    where the key has this bit set to 0, and the presence of opcodes that would
    be blanked as a result, seems to imply that the key is generated based on the
    plaintext. That is, this final bit is set to 1 by default (hence blanking
    more aggressively), and cleared to 0 if any plaintext words affected by the
    byte in question would be incorrectly blanked.


    When the keys were generated, the LCG seed wasn't input directly. Instead,
    another value was entered, which in most cases was derived from the current
    date/time. The LCG seed is obtained from that value via a multiplication.
    The current date/time was also used in most cases to select the three bytes of
    the global key. Interestingly, the global key must be inverted and read in
    decimal representation to see this, while the seed must be read in hexadecimal
    representation.

    For some reason, bit 3 of the first byte of the global key was always set to 1
    regardless of the value input into the key generator program, so e.g. the
    input "88 01 23" would become "80 01 23".

    The very first byte of internal RAM, which indicates the IRQ state, doesn't
    seem to follow the same procedure. The IRQ state was probably decided at an
    earlier time, not during the final key generation.


    summary:
    --------

       +----------------------------------------------------- 317- part #
       |       +--------------------------------------------- IRQ state (hex)
       |       |     +--------------------------------------- global key (inverted, dec)
       |       |     |        +------------------------------ main key seed (hex) (LCG seed = seed * 0x2F1E21)
       |       |     |        |        +--------------------- game
       |       |     |        |        |        +------------ year
       |       |     |        |        |        |          +- inferred key generation date
       |       |     |        |        |        |          |
    --------  -- --------  ------  --------   ----  -----------------------------
    0041      12 87 06 19  895963  bullet     1987  87/06/19 (atypical)
    0045      34 97 02 39  384694  suprleag   1987  (atypical)
    0046      E3 87 10 07  012563  hwchampj   1987  87/10/07 (atypical)
    0049      F1 87 10 28  8932F7  shinobi2   1987  87/10/28 (atypical)
    0050      F1 87 10 28  8932F7  shinobi1   1987  87/10/28 (atypical)
    0053      00 00 00 00  020000  sonicbom   1987  atypical
    0056      CD 80 01 23  032ABC  thndrbld   1987  88/01/23 (atypical)
    0059      45 80 03 30  343210  aceattac   1988  88/03/30 (atypical)
    0060      45 80 03 30  343210  aceattaca  1988  88/03/30 (atypical)
    0065      FC 80 06 06  880606  altbeastj1 1988  88/06/06
    0068      20 80 06 10  880610  altbeastj3 1988  88/06/10
    0069      20 80 06 10  880610  altbeast5  1998 88/06/10
    0070      59 80 08 06  880806  passshtj   1988  88/08/06
    0074      47 80 08 06  880806  passshta   1988  88/08/06
    0071      20 80 08 09  880809  passsht    1988  88/08/09
    0079      98 80 09 05  880906  exctleag   1988  88/09/05-88/09/06 (atypical)
    0080      96 80 08 26  880826  passsht    1988  88/08/26
    0058-02C  FF 80 10 07  881007  sspirtfc   1988  88/10/07
    0084      0E 80 10 31  881031  wb31       1988  88/10/31
    0085      26 80 11 08  881108  wb32       1988  88/11/08
    0087      69 80 11 08  881108  wb34       1988  88/11/08
    0089      52 80 11 29  881129  wb33       1988  88/11/29
    0058-03B  71 80 11 25  881125  ggroundj   1988  88/11/25
    0058-03C  04 80 11 27  881127  gground    1988  88/11/27
    0090      AB 80 01 27  247333  wrestwar1  1989  atypical
    0091      68 80 11 27  881127  tetris1    1988  88/11/27
    0092      10 80 11 28  881128  tetris2    1988  88/11/28
    0093      25 80 11 29  881129  tetris     1988  88/11/29
    0093A     35 02 09 17  900209  tetris3    1988  90/02/09
    0096      21 80 11 21  881121  ddux       1988  88/11/21
    0102      AB 80 02 03  04588A  wrestwar2  1989  atypical
    0058-04B  27 03 27 14  032714  crkdownj   1989  89/03/27 14:xx
    0058-04C  19 03 27 05  032705  crkdown    1989  89/03/27 05:xx
    0058-04D  DC 03 27 06  032706  crkdownu   1989  89/03/27 06:xx
    0110      19 81 03 29  032916  goldnaxe1  1989  89/03/29 16:xx
    0115      12 04 05 11  040511  bayroutej  1989  89/04/05 11:xx
    0116      11 03 30 09  033009  bayroute   1989  89/03/30 09:xx
    0118      22 81 03 07  030719  toutrun    1989  89/03/07 19:xx
    toutrun2  22 81 03 07  031113  toutrun2   1989  89/03/11 13:xx (atypical)
    0120      0D 81 03 29  032916  goldnaxe3  1989  89/03/29 16:xx
    0121      35 81 03 29  032916  goldnaxej  1989  89/03/29 16:xx
    0122      03 81 04 04  890404  goldnaxeu  1989  89/04/04
    0058-05B  92 81 06 09  890609  sgmastj    1989  89/06/09
    0058-05C  30 81 06 13  890613  sgmastc    1989  89/06/13
    0058-05D  50 81 06 09  890609  sgmast     1989  89/06/09
    0124A     80 06 21 11  890621  smgpj      1989  89/06/21 11:xx
    0125A     DE 06 15 16  890615  smgpu      1989  89/06/15 16:xx
    0126      54 05 28 01  890528  smgp5      1989  89/05/28 01:xx
    0126A     74 06 16 15  890616  smgp       1989  89/06/16 15:xx
    0127A     5F 81 07 06  890706  fpoint     1989  89/07/06
    0127A     5F 81 07 06  890706  fpoint1    1989  89/07/06
    0128      55 00 28 20  890828  eswatj     1989  89/08/28 20:xx
    0129      0A 00 28 20  890828  eswatu     1989  89/08/28 20:xx
    0130      EC 00 28 19  890828  eswat      1989  89/08/28 19:xx
    0134      DE 81 11 30  891130  loffirej   1989  89/11/30
    0135      98 81 11 31  891131  loffireu   1989  89/11/31
    0136      12 81 11 29  891129  loffire    1989  89/11/29
    0139      49 03 25 15  891125  bloxeed    1990  89/11/25 15:xx
    0142      91 01 24 17  900124  mvpj       1989  90/01/24 17:xx
    0143      20 02 02 18  900202  mvp        1989  90/02/02 18:xx
    0144      2E 02 23 18  022318  rachero    1989  90/02/23 18:xx
    0058-06B  88 03 15 09  900315  roughrac   1990  90/03/15 09:xx
    0146      10 04 26 17  900426  astormj    1990  90/04/26 17:xx
    0147      2D 04 14 14  900414  astormu    1990  90/04/14 14:xx
    0148      50 04 26 15  900426  astorm3    1990  90/04/26 15:xx
    0153      FC 04 10 14  900410  pontoon    1990  90/04/10 14:xx
    0154      ED 05 01 13  900501  astorm     1990  90/05/01 13:xx
    0157      20 07 20 10  900720  mwalkj     1990  90/07/20 10:xx
    0158      DE 07 15 15  900715  mwalku     1990  90/07/15 15:xx
    0159      39 07 20 10  900720  mwalk      1990  90/07/20 10:xx
    0162      8F 01 14 15  900914  gprider1   1990  90/09/14 15:xx
    0163      99 01 13 15  900913  gprider    1990  90/09/13 15:xx
    5023      EF 04 18 05  900917  ryukyu     1990  90/09/17 12:18? (atypical)
    0165      56 82 11 25  901125  lghostu    1990  90/11/25
    0166      A2 82 11 24  901124  lghost     1990  90/11/24
    0169B     48 06 35 32  901205  abcop      1990  90/12/05 14:35? (atypical)
    0058-08B  4E 04 17 15  910206  qsww       1991  91/02/06 12:17? (atypical)
    0175      91 83 03 22  910322  cltchitrj  1991  91/03/22
    0176      FC 83 03 14  910314  cltchitr   1991  91/03/14
    0179B     73 06 55 17  910318  cottonj    1991  91/03/18 14:55? (atypical)
    0180      73 03 53 00  910403  cottonu    1991  91/04/03 11:53? (atypical)
    0181A     73 06 55 17  910318  cotton     1991  91/03/18 14:55? (atypical)
    0058-09D  91 83 06 26  910618  dcclubfd   1991  91/06/18-91/06/26 (atypical)
    0182      07 07 12 14  921401  ddcrewj2   1991  92/07/12 14:01? (atypical)
    0184      07 07 12 16  921622  ddcrew2    1991  92/07/12 16:22? (atypical)
    0185      91 07 06 16  910704  ddcrewj    1991  91/07/04-91/07/06 16:xx (atypical)
    0186      5F 83 07 01  912030  ddcrewu    1991  91/07/01 20:30? (atypical)
    0187      91 84 07 42  910744  ddcrew1    1991  92/07/xx 07:44? (atypical)
    0190      07 07 17 16  921716  ddcrew     1992  92/07/07 17:16? (atypical)
    0194      4A 07 10 13  920623  desertbrj  1992  92/06/23 07:10? (atypical)
    0196      4A 20 12 22  920623  desertbr   1992  92/06/23 20:12? (atypical)
    0197A     3F 84 06 19  920612  wwallyja   1992  92/06/12-92/06/19 (atypical)
    0197B     3F 84 06 19  920612  wwallyj    1992  92/06/12-92/06/19 (atypical)

    ----

    Bad CPUs that gave some more information about the global key:

              global01 global02 global03
              -------- -------- --------
              .....    ..       ..
    unknown   11111111 11110110 10111110  (Shinobi 16A, part no. unreadable, could be dead)
    unknown   10101011 11111000 11010101  (unknown ddcrewa key)
    dead      00001111 00001111 00001111  (Alien Storm CPU with no battery)
    bad       11100000 10101011 10111001  (flaky 317-0049)

    ----

    Notes:

    We start in state 0.
    Vectors are fetched:
       SP.HI @ $000000 -> mainkey = key[0], globalkey = { $00, $00, $00 }, less aggressive blanking
       SP.LO @ $000002 -> mainkey = key[1], globalkey = { $00, $00, $00 }, less aggressive blanking
       PC.HI @ $000004 -> mainkey = key[2], globalkey = { key[1], $00, $00 }
       PC.LO @ $000006 -> mainkey = key[3], globalkey = { key[1], key[2], $00 }

    driver      FD1094      SP plain  SP enc    PC plain  PC enc    States Used (majority one marked with asterisk, if known)
    --------    --------    --------  --------  --------  --------  ---------------------
    aceattac    317-0059    00000000  A711AF59  00000414  AF59AACD  00 12 17 31 45 90 FC*
    aceattaca   317-0060    00000000  A711AF59  00000400  AF59EADD  00 17 31 45 90 FC*
    altbeast5   317-0069    FFFFFF00  B2F7F299  00000400  CCDDEF58  00 0F 18 20 93* A7 D8
    altbeastj1  317-0065    FFFFFF00  C9C5F299  00000400  CCDDECDD  00 09 12 67* 99 FC
    altbeastj3  317-0068    FFFFFF00  B2F7F299  00000400  CCDDEF58  00 0F 18 20 93* A7 D8
    astorm      317-0154    FFFFFF00  B297F23F  00000400  BD5999DD  00 1F 48* 5F 66 ED
    astorm3     317-0148    FFFFFF00  ED9FF23F  00000400  FC58B9DC  00 06 36 50 CC D9*
    astormj     317-0146    FFFFFF00  ED8CF23F  00000400  FC58BC59  00 10 29* 44 CE DF
    astormu     317-0147    FFFFFF00  B297F23F  00000400  55334533  00 19 2D 46 58* DF
    bayroute    317-0116    00504000  5EB40000  00001000  5533A184  00 04* 11 18
    bayroutej   317-0115    00504000  56150000  00001000  85948DCF  00 05* 12 16
    bloxeed     317-0139    00000000  5D92A711  00001D38  BD59FFE8  00 49 53 55 56* DF
    bullet      317-0041    00000000  57355D96  00001882  8DDC8CF4  00 12 21 69* FE
    cltchitr    317-0176    00000000  A711A711  00000416  AF59EBCD  00 01* 80 FC
    cltchitrj   317-0175    00000000  5F94A711  00000416  AF59AFFB  00 03 19* 91
    cotton      317-0181a   00204000  5DB20000  00000716  CCDD0716  00 0E* 73
    cottonj     317-0179b   00204000  5DB20000  00000720  CCDD0720  00 0E* 73
    cottonu     317-0180    00204000  5DB20000  00000716  A1840716  00 0E* 73
    ddcrew      317-0190    00000000  AF595D92  00000408  BD59FD7F  00 07 16* 17
    ddcrew1     317-0187    00000000  5F945D96  00000408  8DDCE758  00 04 07* 13 91
    ddcrew2     317-0184    00000000  AF595D92  00000408  5533FD7F  00 07 12* 16 22
    ddcrewj     317-0185    00000000  5F945D92  00000408  5533FD7F  00 06 07* 09 91
    ddcrewu     317-0186    00000000  AF59A711  00000408  8DDCC381  00 01 07 5F 91*
    ddux        317-0096    00000000  5F94AF59  00000406  AF5987A0  00 21 28 70* D9
    desertbr    317-0196    00000000  55335D92  00001838  5533F46A  00 3C 4A 82 DA*
    desertbrj   317-0194    00000000  55335D92  00001838  BD59E720  00 3C 4A 82 DA*
    eswat       317-0130    00000000  A711AF59  00000400  5533BC59  00 05 0C EC FA*
    eswatj      317-0128    00000000  A711AF59  00000400  55334735  00 55 63 CB* D5
    eswatu      317-0129    00000000  5537AF59  00000400  55334735  00 0A 12 C3* CC
    exctleag    317-0079    00000000  5537AF59  00000410  83018384  00 02 39 4B 85 98 E5*
    fpoint      317-0127a   00000000  AF59AF59  00001A40  8DDC9960  00 15* 35 5F 82 DB
    fpoint1     317-0127a   00000000  AF59AF59  00001A40  8DDC9960  00 15* 35 5F 82 DB
    goldnaxe1   317-0110    FFFFFF00  ED62F2B9  00000400  AF59A785  00 19 2E 31 48* 5D
    goldnaxe3   317-0120    FFFFFF00  ED62F2B9  00000400  AF59A785  00 0A 0D 44 C7 EF*
    goldnaxej   317-0121    FFFFFF00  C9D6F2B9  00000400  AF59A785  00 12* 35 58 7A 9E
    goldnaxeu   317-0122    FFFFFF00  E53AF2B9  00000400  A184A196  00 03 51* 72 99 F6
    hwchampj    317-0046    00000000  5D925D96  000008AC  AF59AA41  00 8B* E3
    lghost      317-0166    00000000  5F94A711  00000404  EE588241  00 A2 DC E0*
    lghostu     317-0165    00000000  AF59A711  00000404  EE58C340  00 56 C4 DE*
    mvp         317-0143    00000000  5F94A711  00000416  BD59DC5B  00 19* 20 88 98
    mwalk       317-0159    FFFFFF00  C9C7F23F  00000400  5533BD7F  00 39 51* AA CC
    mwalkj      317-0157    FFFFFF00  B2F7F23F  00000400  5533BD7F  00 20 88 99 CF*
    mwalku      317-0158    FFFFFF00  C572F23F  00000400  8DDC8DDD  00 49* 90 C0 DE
    passsht     317-0080    00000000  AF59AF59  00003202  C2003923  00 11 52* 96 EE
    passshta    317-0074    00000000  AF59AF59  000031E4  C2003F8C  00 12* 47 83 A7
    passshtj    317-0070    00000000  5D92AF59  000031E4  C2003F8C  00 12 59 83* FE
    pontoon     317-0153    00000000  A7115D92  00000E0C  FC585979  00 19 70 DC* FC
    ryukyu      317-5023    00203800  AF49D30B  0000042E  FC5863B5  00 DC* EF
    shinobi2    317-0049    FFFFFF00  C9C5F25F  00000400  AF598395  00 53 88 98 9B 9C* F1
    sonicbom    317-0053    00000000  5735AF59  00001000  FC587133  00*
    suprleag    317-0045    00000000  A711AF59  0000108E  BD59CE5B  00 12* 34 56 AB CD
    tetris1     317-0091    00000000  5D92AF59  00000410  AF59AE58  99 25 42 5B 68 FC
    tetris2     317-0092    00000000  5735AF59  00000410  AF598685  00 10 52 74 97* FC
    wb32        317-0085    FFFFFF7E  B2F78997  00000500  AF590500  00 10 13 26 77
    wb33        317-0089    FFFFFF7E  E5C78997  00000500  AF590500  00 23 40 52 71*
    wb34        317-0087    FFFFFF7E  B2978997  00000500  AF590500  00 11* 64 69 82
    wrestwar1   317-0090    00000000  5D96AF59  00000414  8301AE18  00 12* A7 AB CC F9 FC
    wrestwar2   317-0102    00000000  5D96AF59  00000414  EE588E5B  00 12* A7 AB CC F9 FC
    wwallyj     317-0197b   00000000  AF595D96  00000416  CCDDCE5B  00 16* 32 3F 43
    wwallyja    317-0197a   00000000  AF595D96  00000416  CCDDCE5B  00 16* 32 3F 43

*****************************************************************************/

#include "emu.h"
#include "fd1094.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
const device_type FD1094 = &device_creator<fd1094_device>;

/*
317-0162 CPU also needs to mask:
0x107a,
0x127a,
0x147a,
0x167a,
0x187a,
0x1a7a,
0x1c7a,
0x1e7a,
this only happens with 317-0162 so far; I assume it is a fault in the CPU.
*/
const UINT16 fd1094_device::s_masked_opcodes[] =
{
	0x013a,0x033a,0x053a,0x073a,0x083a,0x093a,0x0b3a,0x0d3a,0x0f3a,

	0x103a,       0x10ba,0x10fa,    0x113a,0x117a,0x11ba,0x11fa,
	0x123a,       0x12ba,0x12fa,    0x133a,0x137a,0x13ba,0x13fa,
	0x143a,       0x14ba,0x14fa,    0x153a,0x157a,0x15ba,
	0x163a,       0x16ba,0x16fa,    0x173a,0x177a,0x17ba,
	0x183a,       0x18ba,0x18fa,    0x193a,0x197a,0x19ba,
	0x1a3a,       0x1aba,0x1afa,    0x1b3a,0x1b7a,0x1bba,
	0x1c3a,       0x1cba,0x1cfa,    0x1d3a,0x1d7a,0x1dba,
	0x1e3a,       0x1eba,0x1efa,    0x1f3a,0x1f7a,0x1fba,

	0x203a,0x207a,0x20ba,0x20fa,    0x213a,0x217a,0x21ba,0x21fa,
	0x223a,0x227a,0x22ba,0x22fa,    0x233a,0x237a,0x23ba,0x23fa,
	0x243a,0x247a,0x24ba,0x24fa,    0x253a,0x257a,0x25ba,
	0x263a,0x267a,0x26ba,0x26fa,    0x273a,0x277a,0x27ba,
	0x283a,0x287a,0x28ba,0x28fa,    0x293a,0x297a,0x29ba,
	0x2a3a,0x2a7a,0x2aba,0x2afa,    0x2b3a,0x2b7a,0x2bba,
	0x2c3a,0x2c7a,0x2cba,0x2cfa,    0x2d3a,0x2d7a,0x2dba,
	0x2e3a,0x2e7a,0x2eba,0x2efa,    0x2f3a,0x2f7a,0x2fba,

	0x303a,0x307a,0x30ba,0x30fa,    0x313a,0x317a,0x31ba,0x31fa,
	0x323a,0x327a,0x32ba,0x32fa,    0x333a,0x337a,0x33ba,0x33fa,
	0x343a,0x347a,0x34ba,0x34fa,    0x353a,0x357a,0x35ba,
	0x363a,0x367a,0x36ba,0x36fa,    0x373a,0x377a,0x37ba,
	0x383a,0x387a,0x38ba,0x38fa,    0x393a,0x397a,0x39ba,
	0x3a3a,0x3a7a,0x3aba,0x3afa,    0x3b3a,0x3b7a,0x3bba,
	0x3c3a,0x3c7a,0x3cba,0x3cfa,    0x3d3a,0x3d7a,0x3dba,
	0x3e3a,0x3e7a,0x3eba,0x3efa,    0x3f3a,0x3f7a,0x3fba,

	0x41ba,0x43ba,0x44fa,0x45ba,0x46fa,0x47ba,0x49ba,0x4bba,0x4cba,0x4cfa,0x4dba,0x4fba,

	0x803a,0x807a,0x80ba,0x80fa,    0x81fa,
	0x823a,0x827a,0x82ba,0x82fa,    0x83fa,
	0x843a,0x847a,0x84ba,0x84fa,    0x85fa,
	0x863a,0x867a,0x86ba,0x86fa,    0x87fa,
	0x883a,0x887a,0x88ba,0x88fa,    0x89fa,
	0x8a3a,0x8a7a,0x8aba,0x8afa,    0x8bfa,
	0x8c3a,0x8c7a,0x8cba,0x8cfa,    0x8dfa,
	0x8e3a,0x8e7a,0x8eba,0x8efa,    0x8ffa,

	0x903a,0x907a,0x90ba,0x90fa,    0x91fa,
	0x923a,0x927a,0x92ba,0x92fa,    0x93fa,
	0x943a,0x947a,0x94ba,0x94fa,    0x95fa,
	0x963a,0x967a,0x96ba,0x96fa,    0x97fa,
	0x983a,0x987a,0x98ba,0x98fa,    0x99fa,
	0x9a3a,0x9a7a,0x9aba,0x9afa,    0x9bfa,
	0x9c3a,0x9c7a,0x9cba,0x9cfa,    0x9dfa,
	0x9e3a,0x9e7a,0x9eba,0x9efa,    0x9ffa,

	0xb03a,0xb07a,0xb0ba,0xb0fa,    0xb1fa,
	0xb23a,0xb27a,0xb2ba,0xb2fa,    0xb3fa,
	0xb43a,0xb47a,0xb4ba,0xb4fa,    0xb5fa,
	0xb63a,0xb67a,0xb6ba,0xb6fa,    0xb7fa,
	0xb83a,0xb87a,0xb8ba,0xb8fa,    0xb9fa,
	0xba3a,0xba7a,0xbaba,0xbafa,    0xbbfa,
	0xbc3a,0xbc7a,0xbcba,0xbcfa,    0xbdfa,
	0xbe3a,0xbe7a,0xbeba,0xbefa,    0xbffa,

	0xc03a,0xc07a,0xc0ba,0xc0fa,    0xc1fa,
	0xc23a,0xc27a,0xc2ba,0xc2fa,    0xc3fa,
	0xc43a,0xc47a,0xc4ba,0xc4fa,    0xc5fa,
	0xc63a,0xc67a,0xc6ba,0xc6fa,    0xc7fa,
	0xc83a,0xc87a,0xc8ba,0xc8fa,    0xc9fa,
	0xca3a,0xca7a,0xcaba,0xcafa,    0xcbfa,
	0xcc3a,0xcc7a,0xccba,0xccfa,    0xcdfa,
	0xce3a,0xce7a,0xceba,0xcefa,    0xcffa,

	0xd03a,0xd07a,0xd0ba,0xd0fa,    0xd1fa,
	0xd23a,0xd27a,0xd2ba,0xd2fa,    0xd3fa,
	0xd43a,0xd47a,0xd4ba,0xd4fa,    0xd5fa,
	0xd63a,0xd67a,0xd6ba,0xd6fa,    0xd7fa,
	0xd83a,0xd87a,0xd8ba,0xd8fa,    0xd9fa,
	0xda3a,0xda7a,0xdaba,0xdafa,    0xdbfa,
	0xdc3a,0xdc7a,0xdcba,0xdcfa,    0xddfa,
	0xde3a,0xde7a,0xdeba,0xdefa,    0xdffa
};




//**************************************************************************
//  DECRYPTION CACHE HELPER
//**************************************************************************

//-------------------------------------------------
//  fd1094_decryption_cache - constructor
//-------------------------------------------------

fd1094_decryption_cache::fd1094_decryption_cache(fd1094_device &fd1094)
	: m_fd1094(fd1094),
		m_baseaddress(0),
		m_size(0),
		m_rgnoffset(0)
{
	reset();
}


//-------------------------------------------------
//  reset - reset the cache
//-------------------------------------------------

void fd1094_decryption_cache::reset()
{
	// reset all allocated cache buffers
	for (auto & elem : m_decrypted_opcodes)
		elem.clear();
}


//-------------------------------------------------
//  configure - configure the address and size
//  of the region we are caching
//-------------------------------------------------

void fd1094_decryption_cache::configure(offs_t baseaddress, UINT32 size, offs_t rgnoffset)
{
	// if something important changes, throw away what we have
	if (m_baseaddress != baseaddress || m_size != size || m_rgnoffset != rgnoffset)
	{
		m_baseaddress = baseaddress;
		m_size = size;
		m_rgnoffset = rgnoffset;
		reset();
	}
}


//-------------------------------------------------
//  decrypted_opcodes - return a pointer to the
//  decrypted opcodes for the given state
//-------------------------------------------------

UINT16 *fd1094_decryption_cache::decrypted_opcodes(UINT8 state)
{
	// if we have already decrypted this state, use it
	if (!m_decrypted_opcodes[state].empty())
		return &m_decrypted_opcodes[state][0];

	// otherwise, allocate and decrypt
	m_decrypted_opcodes[state].resize(0x200000/2);
	memset(&m_decrypted_opcodes[state][0], 0, 0x200000);
	m_fd1094.decrypt(m_baseaddress, m_size, m_rgnoffset, &m_decrypted_opcodes[state][0], state);
	return &m_decrypted_opcodes[state][0];
}



//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  fd1094_device - constructor
//-------------------------------------------------


fd1094_device::fd1094_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_device(mconfig, tag, owner, clock, "fd1094", __FILE__),
		m_decrypted_opcodes_bank(*this, "^fd1094_decrypted_opcodes"),
		m_state(0x00),
		m_irqmode(false),
		m_cache(*this),
		m_srcbase(*this, DEVICE_SELF),
		m_key(*this, "key")
{
	// override the name after the m68000 initializes
	m_name.assign("FD1094");

	// add the decrypted opcodes map
//  m_address_map[AS_DECRYPTED_OPCODES] = ADDRESS_MAP_NAME(decrypted_opcodes_map);

	// create the initial masked opcode table
	memset(m_masked_opcodes_lookup, 0, sizeof(m_masked_opcodes_lookup));
	for (auto opcode : s_masked_opcodes)
	{
		m_masked_opcodes_lookup[0][opcode >> 4] |= 1 << ((opcode >> 1) & 7);
		m_masked_opcodes_lookup[1][opcode >> 4] |= 1 << ((opcode >> 1) & 7);
	}

	// add some more opcodes for the more aggressive table
	for (int opcode = 0; opcode < 65536; opcode += 2)
		if ((opcode & 0xff80) == 0x4e80 || (opcode & 0xf0f8) == 0x50c8 || (opcode & 0xf000) == 0x6000)
			m_masked_opcodes_lookup[1][opcode >> 4] |= 1 << ((opcode >> 1) & 7);

	m_state_change = state_change_delegate(FUNC(fd1094_device::default_state_change), this);
}




//-------------------------------------------------
//  change_state - set the current state of the
//  chip
//-------------------------------------------------

void fd1094_device::change_state(int newstate)
{
	// adjust state and IRQ mode
	switch (newstate & 0x300)
	{
		case 0x0000:                // 0x00xx: select state xx
			m_state = newstate & 0xff;
			break;

		case STATE_RESET:           // 0x01xx: select state xx and exit irq mode
			m_state = newstate & 0xff;
			m_irqmode = false;
			break;

		case STATE_IRQ:             // 0x02xx: enter irq mode
			m_irqmode = true;
			break;

		case STATE_RTE:             // 0x03xx: exit irq mode
			m_irqmode = false;
			break;
	}

	// notify of the state change
	m_state_change(state());

	// force a flush of the prefetch cache on any state change
	set_state_int(M68K_PREF_ADDR, 0x0010);
}



//**************************************************************************
//  DEVICE OVERRIDES
//**************************************************************************

//-------------------------------------------------
//  device_start - one-time device initialization
//-------------------------------------------------

void fd1094_device::device_start()
{
	// if no ROM region, see if there's a memory share with our name
	if (!m_srcbase.found())
	{
		memory_share *share = memshare(DEVICE_SELF);
		if (share != nullptr)
			m_srcbase.set_target(reinterpret_cast<UINT16 *>(share->ptr()), share->bytes() / 2);
		else
			throw emu_fatalerror("FD1094 found no data to decrypt!");
	}

	// determine length and configure our cache
	m_cache.configure(0x000000, m_srcbase.bytes(), 0x000000);
	change_state(STATE_RESET);

	// start the base device
	m68000_device::device_start();

	// register for the state changing callbacks we need in the m68000
	set_cmpild_callback(write32_delegate(FUNC(fd1094_device::cmp_callback),this));
	set_rte_callback(write_line_delegate(FUNC(fd1094_device::rte_callback),this));
	static_set_irq_acknowledge_callback(*this, device_irq_acknowledge_delegate(FUNC(fd1094_device::irq_callback), this));

	// save state
	save_item(NAME(m_state));
	save_item(NAME(m_irqmode));
}


//-------------------------------------------------
//  device_reset - one-time device initialization
//-------------------------------------------------

void fd1094_device::device_reset()
{
	// flush the cache and switch to the reset state
	m_cache.reset();
	change_state(STATE_RESET);
	fprintf(stderr, "reset done\n");
	// reset the parent
	m68000_device::device_reset();
}


//-------------------------------------------------
//  device_postload - post restore initialization
//-------------------------------------------------

void fd1094_device::device_postload()
{
	// refresh the state
	m_state_change(state());
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  decrypt_one - decrypt a single opcode given
//  the address, data, and keys; note that the
//  address provided is the word address
//  (physical address / 2)
//-------------------------------------------------

UINT16 fd1094_device::decrypt_one(offs_t address, UINT16 val, const UINT8 *main_key, UINT8 state, bool vector_fetch)
{
	// extract and adjust the global key
	UINT8 gkey1 = main_key[1];
	UINT8 gkey2 = main_key[2];
	UINT8 gkey3 = main_key[3];
	if (state & 0x0001)
	{
		gkey1 ^= 0x04;  // global_xor1
		gkey2 ^= 0x80;  // key_1a invert
		gkey3 ^= 0x80;  // key_2a invert
	}
	if (state & 0x0002)
	{
		gkey1 ^= 0x01;  // global_swap2
		gkey2 ^= 0x10;  // key_7a invert
		gkey3 ^= 0x01;  // key_4b invert
	}
	if (state & 0x0004)
	{
		gkey1 ^= 0x80;  // key_0b invert
		gkey2 ^= 0x40;  // key_6b invert
		gkey3 ^= 0x04;  // global_swap4
	}
	if (state & 0x0008)
	{
		gkey1 ^= 0x20;  // global_xor0
		gkey2 ^= 0x02;  // key_6a invert
		gkey3 ^= 0x20;  // key_5a invert
	}
	if (state & 0x0010)
	{
		gkey1 ^= 0x02;  // key_0c invert
		gkey1 ^= 0x40;  // key_5b invert
		gkey2 ^= 0x08;  // key_4a invert
	}
	if (state & 0x0020)
	{
		gkey1 ^= 0x08;  // key_1b invert
		gkey3 ^= 0x08;  // key_3b invert
		gkey3 ^= 0x10;  // global_swap1
	}
	if (state & 0x0040)
	{
		gkey1 ^= 0x10;  // key_2b invert
		gkey2 ^= 0x20;  // global_swap0a
		gkey2 ^= 0x04;  // global_swap0b
	}
	if (state & 0x0080)
	{
		gkey2 ^= 0x01;  // key_3a invert
		gkey3 ^= 0x02;  // key_0a invert
		gkey3 ^= 0x40;  // global_swap3
	}

	// for address xx0000-xx0006 (but only if >= 000008), use key xx2000-xx2006
	UINT8 mainkey;
	if ((address & 0x0ffc) == 0 && address >= 4)
		mainkey = main_key[(address & 0x1fff) | 0x1000];
	else
		mainkey = main_key[address & 0x1fff];

	UINT8 key_F;
	if (address & 0x1000)   key_F = BIT(mainkey,7);
	else                    key_F = BIT(mainkey,6);

	// the CPU has been verified to produce different results when fetching opcodes
	// from 0000-0006 than when fetching the inital SP and PC on reset.
	if (vector_fetch)
	{
		if (address <= 3) gkey3 = 0x00; // supposed to always be the case
		if (address <= 2) gkey2 = 0x00;
		if (address <= 1) gkey1 = 0x00;
		if (address <= 1) key_F = 0;
	}

	UINT8 global_xor0         = 1^BIT(gkey1,5);
	UINT8 global_xor1         = 1^BIT(gkey1,2);
	UINT8 global_swap2        = 1^BIT(gkey1,0);

	UINT8 global_swap0a       = 1^BIT(gkey2,5);
	UINT8 global_swap0b       = 1^BIT(gkey2,2);

	UINT8 global_swap3        = 1^BIT(gkey3,6);
	UINT8 global_swap1        = 1^BIT(gkey3,4);
	UINT8 global_swap4        = 1^BIT(gkey3,2);

	UINT8 key_0a = BIT(mainkey,0) ^ BIT(gkey3,1);
	UINT8 key_0b = BIT(mainkey,0) ^ BIT(gkey1,7);
	UINT8 key_0c = BIT(mainkey,0) ^ BIT(gkey1,1);

	UINT8 key_1a = BIT(mainkey,1) ^ BIT(gkey2,7);
	UINT8 key_1b = BIT(mainkey,1) ^ BIT(gkey1,3);

	UINT8 key_2a = BIT(mainkey,2) ^ BIT(gkey3,7);
	UINT8 key_2b = BIT(mainkey,2) ^ BIT(gkey1,4);

	UINT8 key_3a = BIT(mainkey,3) ^ BIT(gkey2,0);
	UINT8 key_3b = BIT(mainkey,3) ^ BIT(gkey3,3);

	UINT8 key_4a = BIT(mainkey,4) ^ BIT(gkey2,3);
	UINT8 key_4b = BIT(mainkey,4) ^ BIT(gkey3,0);

	UINT8 key_5a = BIT(mainkey,5) ^ BIT(gkey3,5);
	UINT8 key_5b = BIT(mainkey,5) ^ BIT(gkey1,6);

	UINT8 key_6a = BIT(mainkey,6) ^ BIT(gkey2,1);
	UINT8 key_6b = BIT(mainkey,6) ^ BIT(gkey2,6);

	UINT8 key_7a = BIT(mainkey,7) ^ BIT(gkey2,4);


	if (val & 0x8000)           // block invariant: val & 0x8000 != 0
	{
		val = BITSWAP16(val, 15, 9,10,13, 3,12, 0,14, 6, 5, 2,11, 8, 1, 4, 7);

		if (!global_xor1)   if (~val & 0x0800)  val ^= 0x3002;                                      // 1,12,13
							if (~val & 0x0020)  val ^= 0x0044;                                      // 2,6
		if (!key_1b)        if (~val & 0x0400)  val ^= 0x0890;                                      // 4,7,11
		if (!global_swap2)  if (!key_0c)        val ^= 0x0308;                                      // 3,8,9
												val ^= 0x6561;

		if (!key_2b)        val = BITSWAP16(val,15,10,13,12,11,14,9,8,7,6,0,4,3,2,1,5);             // 0-5, 10-14
	}

	if (val & 0x4000)           // block invariant: val & 0x4000 != 0
	{
		val = BITSWAP16(val, 13,14, 7, 0, 8, 6, 4, 2, 1,15, 3,11,12,10, 5, 9);

		if (!global_xor0)   if (val & 0x0010)   val ^= 0x0468;                                      // 3,5,6,10
		if (!key_3a)        if (val & 0x0100)   val ^= 0x0081;                                      // 0,7
		if (!key_6a)        if (val & 0x0004)   val ^= 0x0100;                                      // 8
		if (!key_5b)        if (!key_0b)        val ^= 0x3012;                                      // 1,4,12,13
												val ^= 0x3523;

		if (!global_swap0b) val = BITSWAP16(val, 2,14,13,12, 9,10,11, 8, 7, 6, 5, 4, 3,15, 1, 0);   // 2-15, 9-11
	}

	if (val & 0x2000)           // block invariant: val & 0x2000 != 0
	{
		val = BITSWAP16(val, 10, 2,13, 7, 8, 0, 3,14, 6,15, 1,11, 9, 4, 5,12);

		if (!key_4a)        if (val & 0x0800)   val ^= 0x010c;                                      // 2,3,8
		if (!key_1a)        if (val & 0x0080)   val ^= 0x1000;                                      // 12
		if (!key_7a)        if (val & 0x0400)   val ^= 0x0a21;                                      // 0,5,9,11
		if (!key_4b)        if (!key_0a)        val ^= 0x0080;                                      // 7
		if (!global_swap0a) if (!key_6b)        val ^= 0xc000;                                      // 14,15
												val ^= 0x99a5;

		if (!key_5b)        val = BITSWAP16(val,15,14,13,12,11, 1, 9, 8, 7,10, 5, 6, 3, 2, 4, 0);   // 1,4,6,10
	}

	if (val & 0xe000)           // block invariant: val & 0xe000 != 0
	{
		val = BITSWAP16(val,15,13,14, 5, 6, 0, 9,10, 4,11, 1, 2,12, 3, 7, 8);

		val ^= 0x17ff;

		if (!global_swap4)  val = BITSWAP16(val, 15,14,13, 6,11,10, 9, 5, 7,12, 8, 4, 3, 2, 1, 0);  // 5-8, 6-12
		if (!global_swap3)  val = BITSWAP16(val, 13,15,14,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);  // 15-14-13
		if (!global_swap2)  val = BITSWAP16(val, 15,14,13,12,11, 2, 9, 8,10, 6, 5, 4, 3, 0, 1, 7);  // 10-2-0-7
		if (!key_3b)        val = BITSWAP16(val, 15,14,13,12,11,10, 4, 8, 7, 6, 5, 9, 1, 2, 3, 0);  // 9-4, 3-1
		if (!key_2a)        val = BITSWAP16(val, 13,14,15,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);  // 13-15

		if (!global_swap1)  val = BITSWAP16(val, 15,14,13,12, 9, 8,11,10, 7, 6, 5, 4, 3, 2, 1, 0);  // 11...8
		if (!key_5a)        val = BITSWAP16(val, 15,14,13,12,11,10, 9, 8, 4, 5, 7, 6, 3, 2, 1, 0);  // 7...4
		if (!global_swap0a) val = BITSWAP16(val, 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 0, 3, 2, 1);  // 3...0
	}

	val = BITSWAP16(val, 12,15,14,13,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	if ((val & 0xb080) == 0x8000) val ^= 0x4000;
	if ((val & 0xf000) == 0xc000) val ^= 0x0080;
	if ((val & 0xb100) == 0x0000) val ^= 0x4000;

	// mask out opcodes doing PC-relative addressing, replace them with FFFF
	if ((m_masked_opcodes_lookup[key_F][val >> 4] >> ((val >> 1) & 7)) & 1)
		val = 0xffff;

	return val;
}


//-------------------------------------------------
//  decrypt - decrypt a buffers' worth of opcodes
//-------------------------------------------------

void fd1094_device::decrypt(offs_t baseaddr, UINT32 size, const UINT16 *srcptr, UINT16 *opcodesptr, UINT8 state)
{
	for (offs_t offset = 0; offset < size; offset += 2)
		opcodesptr[offset / 2] = decrypt_one((baseaddr + offset) / 2, srcptr[offset / 2], &m_key[0], state, (baseaddr + offset) < 8);
}


//-------------------------------------------------
//  default_state_change - handle state changes
//  for standard cases
//-------------------------------------------------

void fd1094_device::default_state_change(UINT8 state)
{
	m_decrypted_opcodes_bank->set_base(m_cache.decrypted_opcodes(state));
}



//**************************************************************************
//  STATIC CALLBACKS
//**************************************************************************

//-------------------------------------------------
//  cmp_callback - callback for CMP.L instructions
//  (state change)
//-------------------------------------------------

WRITE32_MEMBER(fd1094_device::cmp_callback)
{
	if (offset == 0 && (data & 0x0000ffff) == 0x0000ffff)
		change_state(data >> 16);
}


//-------------------------------------------------
//  irq_callback - callback when the FD1094 enters
//  interrupt code
//-------------------------------------------------

IRQ_CALLBACK_MEMBER( fd1094_device::irq_callback )
{
	change_state(STATE_IRQ);
	return (0x60 + irqline * 4) / 4; // vector address
}


//-------------------------------------------------
//  rte_callback - callback when an RTE instruction
//  is encountered
//-------------------------------------------------

WRITE_LINE_MEMBER(fd1094_device::rte_callback)
{
	change_state(STATE_RTE);
}
