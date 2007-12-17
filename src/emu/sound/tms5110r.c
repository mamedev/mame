/* TMS5110 ROM Tables */

/* Note: all the tables in this file were read from the real TMS5110A chip, except
         for the chirptable and the interp_coeff
*/


/* This is the energy lookup table (4-bits -> 10-bits) */
const static unsigned short energytable[0x10] = {
0*2,   1*2,   2*2,   3*2,
4*2,   6*2,   8*2,   11*2,
16*2,  23*2,  33*2,  47*2,
63*2,  85*2,  114*2, 511 }; /*note: the last value (511) is not a true energy value, it's just a stop-sentinel */


/* This is the tms5110 pitchtable */
const static unsigned short pitchtable [0x20]={
0,   15,  16,  17,
19,  21,  22,  25,
26,  29,  32,  36,
40,  42,  46,  50,
55,  60,  64,  68,
72,  76,  80,  84,
86,  93,  101, 110,
120, 132, 144, 159 };


/* These are the reflection coefficient lookup tables */

/* K1 is (5-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k1table[0x20] = {
-501*64, -498*64, -497*64, -495*64,
-493*64, -491*64, -488*64, -482*64,
-478*64, -474*64, -469*64, -464*64,
-459*64, -452*64, -445*64, -437*64,
-412*64, -380*64, -339*64, -288*64,
-227*64, -158*64,  -81*64,   -1*64,
  80*64,  157*64,  226*64,  287*64,
 337*64,  379*64,  411*64,  436*64 };

/* K2 is (5-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k2table[0x20] = {
-328*64, -303*64, -274*64, -244*64,
-211*64, -175*64, -138*64,  -99*64,
 -59*64,  -18*64,   24*64,   64*64,
 105*64,  143*64,  180*64,  215*64,
 248*64,  278*64,  306*64,  331*64,
 354*64,  374*64,  392*64,  408*64,
 422*64,  435*64,  445*64,  455*64,
 463*64,  470*64,  476*64,  506*64 };

/* K3 is (4-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k3table[0x10] = {
-441*64, -387*64, -333*64, -279*64,
-225*64, -171*64, -117*64,  -63*64,
  -9*64,   45*64,   98*64,  152*64,
 206*64,  260*64,  314*64,  368*64 };

/* K4 is (4-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k4table[0x10] = {
-328*64, -273*64, -217*64, -161*64,
-106*64,  -50*64,    5*64,   61*64,
 116*64,  172*64,  228*64,  283*64,
 339*64,  394*64,  450*64,  506*64 };

/* K5 is (4-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k5table[0x10] = {
-328*64, -282*64, -235*64, -189*64,
-142*64,  -96*64,  -50*64,   -3*64,
  43*64,   90*64,  136*64,  182*64,
 229*64,  275*64,  322*64,  368*64 };

/* K6 is (4-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k6table[0x10] = {
-256*64, -212*64, -168*64, -123*64,
 -79*64,  -35*64,   10*64,   54*64,
  98*64,  143*64,  187*64,  232*64,
 276*64,  320*64,  365*64,  409*64 };

/* K7 is (4-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k7table[0x10] = {
-308*64, -260*64, -212*64, -164*64,
-117*64,  -69*64,  -21*64,   27*64,
  75*64,  122*64,  170*64,  218*64,
 266*64,  314*64,  361*64,  409*64 };

/* K8 is (3-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k8table[0x08] = {
-256*64, -161*64,  -66*64,   29*64,
 124*64,  219*64,  314*64,  409*64 };

/* K9 is (3-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k9table[0x08] = {
-256*64, -176*64,  -96*64,  -15*64,
  65*64,  146*64,  226*64,  307*64 };

/* K10 is (3-bits -> 9 bits+sign, 2's comp. fractional (-1 < x < 1) */

const static int k10table[0x08] = {
-205*64, -132*64,  -59*64,   14*64,
  87*64,  160*64,  234*64,  307*64 };


/* chirp table */

const static signed char chirptable[51] = {
0x00, 0x2a, (char)0xd4, 0x32,
(char)0xb2, 0x12, 0x25, 0x14,
0x02, (char)0xe1, (char)0xc5, 0x02,
0x5f, 0x5a, 0x05, 0x0f,
0x26, (char)0xfc, (char)0xa5, (char)0xa5,
(char)0xd6, (char)0xdd, (char)0xdc, (char)0xfc,
0x25, 0x2b, 0x22, 0x21,
0x0f, (char)0xff, (char)0xf8, (char)0xee,
(char)0xed, (char)0xef, (char)0xf7, (char)0xf6,
(char)0xfa, 0x00, 0x03, 0x02,
0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00
};

/* interpolation coefficients */

const static char interp_coeff[8] = {
8, 8, 8, 4, 4, 2, 2, 1
};

