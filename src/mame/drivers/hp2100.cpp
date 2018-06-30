// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-06 Skeleton

HP 2100 and HP 1000 minicomputers with a front panel. Produced between 1966 and 1970. The CPU is a 2116A, a hard-wired design.
The computer can have 32K of 16-bit words of magnetic core. It had 16 I/O slots for various accessories.
See https://en.wikipedia.org/wiki/HP_2100 for more info.

**************** Contents of readme files.
**HP 1000 boot proms:
12992-80002     059F                                  7905/6/20/25
12992-80004     0672                                  ICD
12992-80005     0673                                  CS80
12992-80006     073A     1816-0962                    magtape
12992-80007     06C6     1816-1051                    9885 floppy
12992-80009     0749                                  cartridge tape
                053A     1816-0420                    paper tape
                07FB     1816-0862
                077F     5081-2361

**HP 1000 firmware:
 13207-60001


            1816-
            0545.u2  0539.u3   0540.u4   0541.u5   0542.u6   0543.u7
            0538.u9  0544.u10  0549.u11  0548.u12  0547.u13  0546.u14


 * *-*
 *-* *
 *-* *

HP1000 E/F Firmware
                    PN            CKSUM
                    ===========   =====
2113 Base Set       02113-80006   BCDB
                    02113-80007   8741
                    02113-80008   0889

2117 Base Set       02117-80016   C08F
                    02117-80017   84E4
                    02117-80018   2C45

E-FPP               5090-0589     0530
                    5090-0590     C6B6
                    5090-0591     DE78

E-DMI               13307-80036   EA76      13307-80033 (same cksum)
                    13307-80037   64CA      13307-80034 (same cksum)
                    13307-80038   D7B8      13307-80035 (same cksum)

F-DMI/FFP           5180-0141     FCD1
                    5180-0142     8C42
                    5180-0143     E820

F-VIS               12824-80007   FF2C
                    12824-80008   F4B8
                    12825-80009   BF83

F-SIS               12823-80019   B7E9
                    12823-80020   7EAE
                    12823-80021   2CE9

SIGNAL/1000         92835-80001   1EEC
                    92835-80002   D419
                    92835-80003   C034

DS/1000             91740-80067   3F44
                    91740-80068   340A
                    91740-80069   FF62

RTE-IV EMA          92067-80001   F054
                    92067-80002   B744
                    92067-80003   AAAA

RTE6/VM OS          92084-80001   E946
                    92084-80102   E116      92084-80002   E10E (earlier rev)
                    92084-80103   C670      92084-80003   C654 (earlier rev)

RTE6/VM EMA/VMA     92084-80004   D166
                    92084-80005   8C02
                    92084-80006   B412

2000TSB IOP Firmware (from page 81-11 of M/E/F CE manual)

1816-0532 ... 1816-0537   (6 proms)     2100
1816-0538 ... 1816-0549   (12 proms)    21MX   13207-60001 firmware board
1816-0996 ... 1816-1001   (6 proms)     21MX-E 13304A firmware acc board needed


E-Series Control Memory Map
===========================
Control Memory Module Allocation      Module No    Octal        Software Entry Point

HP Base Set                          -+ 0            00000-00377  yes      1k
                                      | 1            00400-00777  yes
                                      | 2            01000-01377  yes
                                      + 3            01400-01777  yes
Available For User Microprogramming  -+ 4            02000-02377  no       2k
                                      | 5            02400-02777  no
                                      | 6            03000-03377  no
                                      | 7            03400-03777  no
                                      | 8            04000-04377  no       3k
                                      | 9            04400-04777  no
                                      | 10           05000-05377  no
                                      | 11           05400-05777  no
                                      | 12           06000-06377  no       4k
                                      | 13           06400-06777  no
                                      | 14           07000-07377  no
                                      | 15           07400-07777  no
                                      | 16           10000-10377  no       5k
                                      | 17           10400-10777  no
                                      | 18           11000-11377  no
                                      | 19           11400-11777  no
                                      | 20           12000-12377  no       6k
                                      | 21           12400-12777  no
                                      | 22           13000-13377  no
                                      | 23           13400-13777  no
                                      | 24           14000-14377  no       7k
                                      | 25           14400-14777  no
                                      | 26           15000-15377  no
                                      | 27           15400-15777  no
                                      | 28           16000-16377  no       8k
                                      | 29           16400-16777  no
                                      | 30           17000-17377  no
                                      + 31           17400-17777  no       8k
HP Dynamic Mapping System            -- 32           20000-20377  yes      9k
HP Fast Fortran Processor            -+ 33           20400-20777  no
                                      | 34           21000-21377  yes
                                      + 35           21400-21777  yes
RTE-IV EMA or RTE-6/VM EMA/VMA       -+ 36           22000-22377  yes      10k
                                      + 37           22400-22777  yes
DS/1000                              -+ 38           23000-23377  yes
                                      + 39           23400-23777  yes
HP Reserved                          -+ 40           24000-24377  yes      11k
                                      | 41           24400-24777  no
                                      | 42           25000-25377  no
                                      + 43           25400-25777  no
RTE-6/VM Operating System            -+ 44           26000-26377  yes      12k
                                      + 45           26400-26777  yes
Recommended For User Microprogramming + 46           27000-27377  yes
                                      | 47           27400-27777  yes
                                      | 48           30000-30377  yes      13k
                                      | 49           30400-30777  yes
                                      | 50           31000-31377  yes
                                      | 51           31400-31777  no
                                      | 52           32000-32377  no       14k
                                      | 53           32400-32777  no
                                      | 54           33000-33377  no
                                      | 55           33400-33777  no
                                      | 56           34000-34377  yes      15k
                                      | 57           34400-34777  yes
                                      | 58           35000-35377  yes
                                      | 59           35400-35777  yes
                                      | 60           36000-36377  yes      16k
                                      | 61           36400-36777  no
                                      | 62           37000-37377  yes
                                      | 63           37400-37777  no

F-Series Control Memory Map
===========================
Control Memory Module Allocation      Module No    Octal        Software Entry Point

HP Base Set                          -+ 0            00000-00377  yes      1k
                                      | 1            00400-00777  yes
                                      | 2            01000-01377  yes
                                      + 3            01400-01777  yes
HP Reserved                          -+ 4            02000-02377  yes      2k
                                      | 5            02400-02777  no
                                      | 6            03000-03377  no
                                      | 7            03400-03777  no
                                      | 8            04000-04377  yes      3k
                                      | 9            04400-04777  no
                                      | 10           05000-05377  no
                                      + 11           05400-05777  no
Vector Instruction Set               -+ 12           06000-06377  yes      4k
                                      | 13           06400-06777  no
                                      | 14           07000-07377  no
                                      + 15           07400-07777  no
RTE-6/VM Operating System            -+ 16           10000-10377  yes      5k
                                      | 17           10400-10777  no
HP Reserved                          -+ 18           11000-11377  yes
                                      | 19           11400-11777  no
                                      | 20           12000-12377  yes      6k
                                      | 21           12400-12777  no
                                      | 22           13000-13377  no
                                      | 23           13400-13777  no
                                      | 24           14000-14377  no       7k
                                      | 25           14400-14777  no
                                      | 26           15000-15377  no
                                      + 27           15400-15777  no
Available For User Microprogramming  -+ 28           16000-16377  no       8k
                                      | 29           16400-16777  no
                                      | 30           17000-17377  no
                                      + 31           17400-17777  no
HP Dynamic Mapping System            -- 32           20000-20377  yes      9k
HP Fast Fortran Processor            -+ 33           20400-20777  no
                                      | 34           21000-21377  yes
                                      + 35           21400-21777  yes
RTE-IV EMA or RTE-6/VM EMA/VMA       -+ 36           22000-22377  yes      10k
                                      + 37           22400-22777  yes
DS/1000                              -+ 38           23000-23377  yes
                                      + 39           23400-23777  yes
Scientific Instruction Set           -+ 40           24000-24377  yes      11k
                                      | 41           24400-24777  no
                                      | 42           25000-25377  no
                                      + 43           25400-25777  no
HP Reserved                          -+ 44           26000-26377  no       12k
                                      + 45           26400-26777  no
Recommended For User Microprogramming + 46           27000-27377  yes
                                      | 47           27400-27777  yes
                                      | 48           30000-30377  yes      13k
                                      | 49           30400-30777  yes
                                      | 50           31000-31377  yes
                                      | 51           31400-31777  no
                                      | 52           32000-32377  no       14k
                                      | 53           32400-32777  no
                                      | 54           33000-33377  no
                                      | 55           33400-33777  no
                                      | 56           34000-34377  yes      15k
                                      | 57           34400-34777  yes
                                      | 58           35000-35377  yes
                                      | 59           35400-35777  yes
                                      | 60           36000-36377  yes      16k
                                      | 61           36400-36777  no
                                      | 62           37000-37377  yes
                                      | 63           37400-37777  no

************************************************************************************************************************************/

#include "emu.h"

class hp2100_state : public driver_device
{
public:
	hp2100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
	{ }

void hp2100(machine_config &config);
private:
//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( hp2100 )
INPUT_PORTS_END

MACHINE_CONFIG_START(hp2100_state::hp2100)
MACHINE_CONFIG_END

ROM_START( hp2100 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	// HP 2100 firmware
	ROM_LOAD( "1816-0054.bin", 0x000000, 0x000100, CRC(7c4192a2) SHA1(1f06e392258dd41f1f7ebd8b744a18ba03514e9d) )
	ROM_LOAD( "1816-0055.bin", 0x000000, 0x000100, CRC(6f0a671d) SHA1(febbbd6672ae1e95141ae4dddaea545b253ad18b) )
	ROM_LOAD( "1816-0056.bin", 0x000000, 0x000100, CRC(2a675e62) SHA1(d044911edc40bee386a8b96b9281250f4d9a4ac0) )
	ROM_LOAD( "1816-0057.bin", 0x000000, 0x000100, CRC(c1b3e302) SHA1(9ab7375ad80bc95efc0d744d23c5a45d42deb75a) )
	ROM_LOAD( "1816-0058.bin", 0x000000, 0x000100, CRC(a03f29e6) SHA1(d2bb3beb947e68bd51783b8730a80dd568b23aef) )
	ROM_LOAD( "1816-0059.bin", 0x000000, 0x000100, CRC(4753cd42) SHA1(bb801c842b82a316e92b6bdee196dc0d2e910590) )
	ROM_LOAD( "1816-0368.bin", 0x000000, 0x000100, CRC(6e4f94e6) SHA1(abac74ff4d57a75e33d35f9f2d1007c953c0d89b) )
	ROM_LOAD( "1816-0369.bin", 0x000000, 0x000100, CRC(be27972a) SHA1(c1dc90614f68adbc6870dd4169d296366f32f335) )
	ROM_LOAD( "1816-0370.bin", 0x000000, 0x000100, CRC(bf9d609b) SHA1(e03467d293b0ce7c3a2f99ac3c72f9d70e1f49ab) )
	ROM_LOAD( "1816-0371.bin", 0x000000, 0x000100, CRC(9292ce4a) SHA1(03ee62752bf1924127eef1decfa974395ccf2d7c) )
	ROM_LOAD( "1816-0372.bin", 0x000000, 0x000100, CRC(e38ada24) SHA1(b5e6ba0fcf49e45d38fd273bb078cc82946cfe5b) )
	ROM_LOAD( "1816-0373.bin", 0x000000, 0x000100, CRC(45e03c99) SHA1(509fe9fec58caf3a5983ad2bdf47c22a6e5376d5) )
	ROM_LOAD( "1816-0490.bin", 0x000000, 0x000100, CRC(61e272b0) SHA1(5d35c135653bf1a1891b7672a5a65958e7e52189) )
	ROM_LOAD( "1816-0491.bin", 0x000000, 0x000100, CRC(9e1504bd) SHA1(6258fa775f1979f0031ad930bf23938ea61afad8) )
	ROM_LOAD( "1816-0492.bin", 0x000000, 0x000100, CRC(0c4e7f1f) SHA1(951c69053de4a0d368d1d3cc851e48645cb69ed0) )
	ROM_LOAD( "1816-0493.bin", 0x000000, 0x000100, CRC(fc3f785e) SHA1(7398ef7bf0fe4f689a1c836cc4295ee6a7ef628b) )
	ROM_LOAD( "1816-0494.bin", 0x000000, 0x000100, CRC(a2424034) SHA1(e99f577dc3c23c82a12b6689c65b2d9e20e7f591) )
	ROM_LOAD( "1816-0495.bin", 0x000000, 0x000100, CRC(e79f1401) SHA1(13812f3acd3129e9ab2e2d790bdb25ed966061af) )
	ROM_LOAD( "1816-0862.bin", 0x000000, 0x000100, CRC(780e180b) SHA1(db4bedf30d347b618101efc5da5b15a822e10f6f) )
	ROM_LOAD( "1816-0869.bin", 0x000000, 0x000100, CRC(897e94a9) SHA1(805a4ebc837daeb0b84b44d2eddbde8862ff9a32) )
	// HP 1000 boot proms
	ROM_LOAD( "12992-80002.bin", 0x000000, 0x000100, CRC(62a32a93) SHA1(fadc7b3c8d9d983ba8834ae3be87b3fef4da5a2c) )
	ROM_LOAD( "12992-80004.bin", 0x000000, 0x000100, CRC(9d699b3b) SHA1(dea16198bc959056782333b87893ec88a8fc8e6d) )
	ROM_LOAD( "12992-80005.bin", 0x000000, 0x000100, CRC(f4fb0410) SHA1(c93c47f3e5c41eb5fae60f5bbe81586e62bc278f) )
	ROM_LOAD( "12992-80006.bin", 0x000000, 0x000100, CRC(a18c008e) SHA1(c5b4237f256732f8e3c044dc728e65510e190056) )
	ROM_LOAD( "12992-80007.bin", 0x000000, 0x000100, CRC(505bb24b) SHA1(3579aaf97534768bfb59b376a685a1104330ebd8) )
	ROM_LOAD( "12992-80009.bin", 0x000000, 0x000100, CRC(38904247) SHA1(e41abd203e918d966ace66f198c53953f7c8547d) )
	ROM_LOAD( "1816-0420.bin", 0x000000, 0x000100, CRC(f3d7c188) SHA1(f63287cbb0aa6d20dd179a7f2d15755c8f34f027) )
	ROM_LOAD( "1816-0796.bin", 0x000000, 0x000100, CRC(d3a675a7) SHA1(1f64f40e46d80f770cc00c2c2c9f251f96ff47b0) )
	ROM_LOAD( "5081-2361.bin", 0x000000, 0x000100, CRC(29f71b0c) SHA1(767cc9d57b5921f755dbd3e376a30dd21cfa33cd) )
	// HP 1000 firmware
	ROM_LOAD( "02113-80006.bin", 0x000000, 0x000400, CRC(8cd6f92a) SHA1(fa0048897362b2151bb417b36712f33a6a346113) )
	ROM_LOAD( "02113-80007.bin", 0x000000, 0x000400, CRC(5f92b933) SHA1(461a24aabc631dec2d980c595fbb36207fa08d40) )
	ROM_LOAD( "02113-80008.bin", 0x000000, 0x000400, CRC(52382fef) SHA1(bc05f8860b495c8e46cda7308858730738f76e4f) )
	ROM_LOAD( "02117-80016.bin", 0x000000, 0x000400, CRC(b43e2864) SHA1(b4753042d8750e169f24333545309838d9540a9a) )
	ROM_LOAD( "02117-80017.bin", 0x000000, 0x000400, CRC(03485b73) SHA1(8c5b6c5aed2351aa6e2455205a8bd6d32e69f978) )
	ROM_LOAD( "02117-80018.bin", 0x000000, 0x000400, CRC(dfeffc0c) SHA1(b062faa150bc2df6c92dc312e302e632def8fb00) )
	ROM_LOAD( "12823-80019.bin", 0x000000, 0x000400, CRC(219412d1) SHA1(71175efb74eaaebbcd80ff57bada53b9ac1fc31a) )
	ROM_LOAD( "12823-80020.bin", 0x000000, 0x000400, CRC(797932e2) SHA1(64f722e5893b397f87a94a9d0e06015658d3687d) )
	ROM_LOAD( "12823-80021.bin", 0x000000, 0x000400, CRC(7b79a419) SHA1(06ed29f9f728d16aed4de348aa8de5508ec65eb7) )
	ROM_LOAD( "12824-80007.bin", 0x000000, 0x000400, CRC(11617b11) SHA1(792f9867ced452fc33ed00e44f12e77e7d59de1c) )
	ROM_LOAD( "12824-80008.bin", 0x000000, 0x000400, CRC(72deee30) SHA1(85fb0c3d513c996b25864e1707bbd29a8d9bb50a) )
	ROM_LOAD( "12824-80009.bin", 0x000000, 0x000400, CRC(349fdb59) SHA1(c3de054e7d03428a1c7afc77fcae4e9b29b2d681) )
	ROM_LOAD( "13307-80036.bin", 0x000000, 0x000400, CRC(85cef205) SHA1(e0192c1dcc2d89c8862ba3c28ec173138c1a44ee) )
	ROM_LOAD( "13307-80033.bin", 0x000000, 0x000400, CRC(85cef205) SHA1(e0192c1dcc2d89c8862ba3c28ec173138c1a44ee) )
	ROM_LOAD( "13307-80034.bin", 0x000000, 0x000400, CRC(c726dd35) SHA1(fafd35727b0b29817ccd9278bd7e642aa7163082) )
	ROM_LOAD( "13307-80037.bin", 0x000000, 0x000400, CRC(c726dd35) SHA1(fafd35727b0b29817ccd9278bd7e642aa7163082) )
	ROM_LOAD( "13307-80035.bin", 0x000000, 0x000400, CRC(26c33e75) SHA1(cf40f18dd10b4c43bb93fecc0ca0fd5b0c5425c7) )
	ROM_LOAD( "13307-80038.bin", 0x000000, 0x000400, CRC(26c33e75) SHA1(cf40f18dd10b4c43bb93fecc0ca0fd5b0c5425c7) )
	ROM_LOAD( "1816-0538.u9.bin", 0x000000, 0x000100, CRC(9cf6fe69) SHA1(7fe6c4d890bdc9eb91fa5c0a7218b8994a62fa6c) )
	ROM_LOAD( "1816-0539.u3.bin", 0x000000, 0x000100, CRC(519a08bf) SHA1(fac12d2bb9ebc289fc7da5ffb3b784459fd75285) )
	ROM_LOAD( "1816-0540.u4.bin", 0x000000, 0x000100, CRC(fccb14f8) SHA1(9f71317303d9e0e9d5e32022d78fb0851e614ab5) )
	ROM_LOAD( "1816-0541.u5.bin", 0x000000, 0x000100, CRC(afeb0709) SHA1(6f9c4ab0975b3364940c60f3085824209e40f018) )
	ROM_LOAD( "1816-0542.u6.bin", 0x000000, 0x000100, CRC(bbafb8d8) SHA1(d3e8e0b0b04d79d55f1931585ba82310166f8614) )
	ROM_LOAD( "1816-0543.u7.bin", 0x000000, 0x000100, CRC(6df287d4) SHA1(5a2cd835ecd2082a418e6a672af9ca4d236eb394) )
	ROM_LOAD( "1816-0544.u10.bin", 0x000000, 0x000100, CRC(363c4c2b) SHA1(3a39a871c2833d16b16d74ca43f772d9b7196729) )
	ROM_LOAD( "1816-0545.u2.bin", 0x000000, 0x000100, CRC(11d99e86) SHA1(ccef7b22840ee8030dbac8e4d712cb488c403e35) )
	ROM_LOAD( "1816-0546.u14.bin", 0x000000, 0x000100, CRC(88c78dc0) SHA1(dae14d4fcf86d80c1a1df0fbd8f9aaa4bb5257d8) )
	ROM_LOAD( "1816-0547.u13.bin", 0x000000, 0x000100, CRC(db1f149a) SHA1(279099b2c438114e0ab722b16c956f3be8871f96) )
	ROM_LOAD( "1816-0548.u12.bin", 0x000000, 0x000100, CRC(fb0ad4ad) SHA1(97c4e55a3983181782ba41685b99d7c5a4a692e8) )
	ROM_LOAD( "1816-0549.u11.bin", 0x000000, 0x000100, CRC(8bf77f30) SHA1(6aa73d95b954ff33c1a69a19dd47b30712a574ad) )
	ROM_LOAD( "5090-0589.bin", 0x000000, 0x000400, CRC(e33752ca) SHA1(5956190e9bf4126065992de46a59ebd83757a9b1) )
	ROM_LOAD( "5090-0590.bin", 0x000000, 0x000400, CRC(b423b55c) SHA1(7931f155666da04d09e34d9c9e323c6a6a3e7621) )
	ROM_LOAD( "5090-0591.bin", 0x000000, 0x000400, CRC(f8653fc6) SHA1(3341fc65cd73c1adb0c56f1e0518041206d8b4de) )
	ROM_LOAD( "5180-0141.bin", 0x000000, 0x000400, CRC(215c5f5c) SHA1(1ec17050e63a447f07291f5de479dedf6ff6bdc7) )
	ROM_LOAD( "5180-0142.bin", 0x000000, 0x000400, CRC(919119f3) SHA1(18ebcaf355af1c6920fb90688f692e73d2ca9abf) )
	ROM_LOAD( "5180-0143.bin", 0x000000, 0x000400, CRC(82bf297a) SHA1(34ff0ac268b06072533fdf90d2365a4cf049e9f3) )
	ROM_LOAD( "91740-80067.bin", 0x000000, 0x000400, CRC(d65f5c3f) SHA1(bc3ef3623f0b475499f708647e779c3f82c9998b) )
	ROM_LOAD( "91740-80068.bin", 0x000000, 0x000400, CRC(5c62a4db) SHA1(6064c9d1179ea115c84e7f9133f93c9d0a85d1ab) )
	ROM_LOAD( "91740-80069.bin", 0x000000, 0x000400, CRC(7e8b5ebe) SHA1(2a2c56ab55bc32d5b60c29166969653297354e16) )
	ROM_LOAD( "92067-80001.bin", 0x000000, 0x000400, CRC(4366dafe) SHA1(559b5494cb57f1dc862717b881df51dde22b9ea1) )
	ROM_LOAD( "92067-80002.bin", 0x000000, 0x000400, CRC(b31eed07) SHA1(43004a14080176707bd492335be84ee9e776f97f) )
	ROM_LOAD( "92067-80003.bin", 0x000000, 0x000400, CRC(9f97bbe6) SHA1(e37e6c48786bc3ad6eb1ea9307c8b6e188524276) )
	ROM_LOAD( "92084-80001.bin", 0x000000, 0x000400, CRC(8ebbf4a8) SHA1(f770da090f1df7a9e8c9e69621b5690264b623d7) )
	ROM_LOAD( "92084-80002.bin", 0x000000, 0x000400, CRC(7b02d33a) SHA1(1a0f7f7fee60f26eec9c9aa06b19944ff5de1b1f) )
	ROM_LOAD( "92084-80003.bin", 0x000000, 0x000400, CRC(9e338250) SHA1(6758fe50931ec9b623247f51b5b63d9cbca47061) )
	ROM_LOAD( "92084-80004.bin", 0x000000, 0x000400, CRC(524ee313) SHA1(d9194d566b59974bf17d290b7f9efbbf0874b410) )
	ROM_LOAD( "92084-80005.bin", 0x000000, 0x000400, CRC(9a62d961) SHA1(f1c1faa0760df0e0406a5a2cb7b4fc6bb12f349f) )
	ROM_LOAD( "92084-80006.bin", 0x000000, 0x000400, CRC(26af135b) SHA1(82dbd9be22a6ffa90cfbfad25f80daa281aa3a14) )
	ROM_LOAD( "92084-80102.bin", 0x000000, 0x000400, CRC(644f8bb2) SHA1(10bef2af538c543bf42c5df109506834fbfbcaa8) )
	ROM_LOAD( "92084-80103.bin", 0x000000, 0x000400, CRC(77b7b66c) SHA1(b8a4afc9b48696dd8691918c4d8b89fba4d2526b) )
	ROM_LOAD( "92835-80001.bin", 0x000000, 0x000400, CRC(f6b71580) SHA1(f0e529f1f81ddf9c3e028d666b65f10289e088bb) )
	ROM_LOAD( "92835-80002.bin", 0x000000, 0x000400, CRC(e417c13a) SHA1(bcbe0421079fb18a1c3989e863b1f9a763f4d548) )
	ROM_LOAD( "92835-80003.bin", 0x000000, 0x000400, CRC(1df52f66) SHA1(84fd048d50f6fde9a79e8df618b5a3435a087f85) )
ROM_END

COMP( 1966, hp2100, 0, 0, hp2100, hp2100, hp2100_state, empty_init, "Hewlett-Packard", "HP 2100", MACHINE_IS_SKELETON )
