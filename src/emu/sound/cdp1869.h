/*

                        ________________                                            ________________
           TPA   1  ---|       \/       |---  40  Vdd          PREDISPLAY_   1  ---|       \/       |---  40  Vdd
           TPB   2  ---|                |---  39  PMSEL          *DISPLAY_   2  ---|                |---  39  PAL/NTSC_
          MRD_   3  ---|                |---  38  PMWR_                PCB   3  ---|                |---  38  CPUCLK
          MWR_   4  ---|                |---  37  *CMSEL              CCB1   4  ---|                |---  37  XTAL (DOT)
         MA0/8   5  ---|                |---  36  CMWR_               BUS7   5  ---|                |---  36  XTAL (DOT)_
         MA1/9   6  ---|                |---  35  PMA0                CCB0   6  ---|                |---  35  *ADDRSTB_
        MA2/10   7  ---|                |---  34  PMA1                BUS6   7  ---|                |---  34  MRD_
        MA3/11   8  ---|                |---  33  PMA2                CDB5   8  ---|                |---  33  TPB
        MA4/12   9  ---|                |---  32  PMA3                BUS5   9  ---|                |---  32  *CMSEL
        MA5/13  10  ---|    CDP1869C    |---  31  PMA4                CDB4  10  ---|  CDP1870/76C   |---  31  BURST
        MA6/14  11  ---|    top view    |---  30  PMA5                BUS4  11  ---|    top view    |---  30  *H SYNC_
        MA7/15  12  ---|                |---  29  PMA6                CDB3  12  ---|                |---  29  COMPSYNC_
            N0  13  ---|                |---  28  PMA7                BUS3  13  ---|                |---  28  LUM / (RED)^
            N1  14  ---|                |---  27  PMA8                CDB2  14  ---|                |---  27  PAL CHROM / (BLUE)^
            N2  15  ---|                |---  26  PMA9                BUS2  15  ---|                |---  26  NTSC CHROM / (GREEN)^
      *H SYNC_  16  ---|                |---  25  CMA3/PMA10          CDB1  16  ---|                |---  25  XTAL_ (CHROM)
     *DISPLAY_  17  ---|                |---  24  CMA2                BUS1  17  ---|                |---  24  XTAL (CHROM)
     *ADDRSTB_  18  ---|                |---  23  CMA1                CDB0  18  ---|                |---  23  EMS_
         SOUND  19  ---|                |---  22  CMA0                BUS0  19  ---|                |---  22  EVS_
           VSS  20  ---|________________|---  21  *N=3_                Vss  20  ---|________________|---  21  *N=3_


                 * = INTERCHIP CONNECTIONS      ^ = FOR THE RGB BOND-OUT OPTION (CDP1876C)      _ = ACTIVE LOW

*/

#ifndef __CDP1869_SOUND__
#define __CDP1869_SOUND__

void cdp1869_set_toneamp(int which, int value);
void cdp1869_set_tonefreq(int which, int value);
void cdp1869_set_toneoff(int which, int value);
void cdp1869_set_tonediv(int which, int value);
void cdp1869_set_wnamp(int which, int value);
void cdp1869_set_wnfreq(int which, int value);
void cdp1869_set_wnoff(int which, int value);

#endif
