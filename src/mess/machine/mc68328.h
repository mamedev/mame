/**********************************************************************

    Motorola 68328 ("DragonBall") System-on-a-Chip private data

    By MooglyGuy
    contact mooglyguy@gmail.com with licensing and usage questions.

**********************************************************************/

#ifndef __MC68328_PRIVATE_H_
#define __MC68328_PRIVATE_H_

typedef struct
{
    // $(FF)FFF000
    UINT8   scr;        // System Control Register
    UINT8   unused0[255];

    // $(FF)FFF100
    UINT16  grpbasea;   // Chip Select Group A Base Register
    UINT16  grpbaseb;   // Chip Select Group B Base Register
    UINT16  grpbasec;   // Chip Select Group C Base Register
    UINT16  grpbased;   // Chip Select Group D Base Register
    UINT16  grpmaska;   // Chip Select Group A Mask Register
    UINT16  grpmaskb;   // Chip Select Group B Mask Register
    UINT16  grpmaskc;   // Chip Select Group C Mask Register
    UINT16  grpmaskd;   // Chip Select Group D Mask Register
    UINT32  csa0;       // Group A Chip Select 0 Register
    UINT32  csa1;       // Group A Chip Select 1 Register
    UINT32  csa2;       // Group A Chip Select 2 Register
    UINT32  csa3;       // Group A Chip Select 3 Register
    UINT32  csb0;       // Group B Chip Select 0 Register
    UINT32  csb1;       // Group B Chip Select 1 Register
    UINT32  csb2;       // Group B Chip Select 2 Register
    UINT32  csb3;       // Group B Chip Select 3 Register
    UINT32  csc0;       // Group C Chip Select 0 Register
    UINT32  csc1;       // Group C Chip Select 1 Register
    UINT32  csc2;       // Group C Chip Select 2 Register
    UINT32  csc3;       // Group C Chip Select 3 Register
    UINT32  csd0;       // Group D Chip Select 0 Register
    UINT32  csd1;       // Group D Chip Select 1 Register
    UINT32  csd2;       // Group D Chip Select 2 Register
    UINT32  csd3;       // Group D Chip Select 3 Register
    UINT8   unused1[176];

    // $(FF)FFF200
    UINT16  pllcr;      // PLL Control Register
    UINT16  pllfsr;     // PLL Frequency Select Register
    UINT8   pad2[3];
    UINT8   pctlr;      // Power Control Register
    UINT8   unused3[248];

    // $(FF)FFF300
    UINT8   ivr;        // Interrupt Vector Register
    UINT8   unused4[1];
    UINT16  icr;        // Interrupt Control Register
    UINT32  imr;        // Interrupt Mask Register
    UINT32  iwr;        // Interrupt Wakeup Enable Register
    UINT32  isr;        // Interrupt Status Register
    UINT32  ipr;        // Interrupt Pending Register
    UINT8   unused5[236];

    // $(FF)FFF400
    UINT8   padir;      // Port A Direction Register
    UINT8   padata;     // Port A Data Register
    UINT8   unused6[1];
    UINT8   pasel;      // Port A Select Register
    UINT8   unused7[4];

    UINT8   pbdir;      // Port B Direction Register
    UINT8   pbdata;     // Port B Data Register
    UINT8   unused8[1];
    UINT8   pbsel;      // Port B Select Register
    UINT8   unused9[4];

    UINT8   pcdir;      // Port C Direction Register
    UINT8   pcdata;     // Port C Data Register
    UINT8   unused10[1];
    UINT8   pcsel;      // Port C Select Register
    UINT8   unused11[4];

    UINT8   pddir;      // Port D Direction Register
    UINT8   pddata;     // Port D Data Register
    UINT8   pdpuen;     // Port D Pullup Enable Register
    UINT8   unused12[1];
    UINT8   pdpol;      // Port D Polarity Register
    UINT8   pdirqen;    // Port D IRQ Enable Register
    UINT8   pddataedge; // Port D Data Edge Level
    UINT8   pdirqedge;  // Port D IRQ Edge Register

    UINT8   pedir;      // Port E Direction Register
    UINT8   pedata;     // Port E Data Register
    UINT8   pepuen;     // Port E Pullup Enable Register
    UINT8   pesel;      // Port E Select Register
    UINT8   unused14[4];

    UINT8   pfdir;      // Port F Direction Register
    UINT8   pfdata;     // Port F Data Register
    UINT8   pfpuen;     // Port F Pullup Enable Register
    UINT8   pfsel;      // Port F Select Register
    UINT8   unused15[4];

    UINT8   pgdir;      // Port G Direction Register
    UINT8   pgdata;     // Port G Data Register
    UINT8   pgpuen;     // Port G Pullup Enable Register
    UINT8   pgsel;      // Port G Select Register
    UINT8   unused16[4];

    UINT8   pjdir;      // Port J Direction Register
    UINT8   pjdata;     // Port J Data Register
    UINT8   unused17[1];
    UINT8   pjsel;      // Port J Select Register
    UINT8   unused18[4];
    UINT8   pkdir;      // Port K Direction Register
    UINT8   pkdata;     // Port K Data Register
    UINT8   pkpuen;     // Port K Pullup Enable Register
    UINT8   pksel;      // Port K Select Register
    UINT8   unused19[4];

    UINT8   pmdir;      // Port M Direction Register
    UINT8   pmdata;     // Port M Data Register
    UINT8   pmpuen;     // Port M Pullup Enable Register
    UINT8   pmsel;      // Port M Select Register
    UINT8   unused20[180];

    // $(FF)FFF500
    UINT16  pwmc;       // PWM Control Register
    UINT16  pwmp;       // PWM Period Register
    UINT16  pwmw;       // PWM Width Register
    UINT16  pwmcnt;     // PWN Counter
    UINT8   unused21[248];

    // $(FF)FFF600
    UINT16  tctl[2];    // Timer Control Register
    UINT16  tprer[2];   // Timer Prescaler Register
    UINT16  tcmp[2];    // Timer Compare Register
    UINT16  tcr[2];     // Timer Capture Register
    UINT16  tcn[2];     // Timer Counter
    UINT16  tstat[2];   // Timer Status
    UINT16  wctlr;      // Watchdog Control Register
    UINT16  wcmpr;      // Watchdog Compare Register
    UINT16  wcn;        // Watchdog Counter
    UINT8   tclear[2];  // Timer Clearable Status
    UINT8   unused22[224];

    // $(FF)FFF700
    UINT16  spisr;      // SPIS Register
    UINT8   unused23[254];

    // $(FF)FFF800
    UINT16  spimdata;   // SPIM Data Register
    UINT16  spimcont;   // SPIM Control/Status Register
    UINT8   unused24[252];

    // $(FF)FFF900
    UINT16  ustcnt;     // UART Status/Control Register
    UINT16  ubaud;      // UART Baud Control Register
    UINT16  urx;        // UART RX Register
    UINT16  utx;        // UART TX Register
    UINT16  umisc;      // UART Misc Register
    UINT8   unused25[246];

    // $(FF)FFFA00
    UINT32  lssa;       // Screen Starting Address Register
    UINT8   unused26[1];
    UINT8   lvpw;       // Virtual Page Width Register
    UINT8   unused27[2];
    UINT16  lxmax;      // Screen Width Register
    UINT16  lymax;      // Screen Height Register
    UINT8   unused28[12];
    UINT16  lcxp;       // Cursor X Position
    UINT16  lcyp;       // Cursor Y Position
    UINT16  lcwch;      // Cursor Width & Height Register
    UINT8   unused29[1];
    UINT8   lblkc;      // Blink Control Register
    UINT8   lpicf;      // Panel Interface Config Register
    UINT8   lpolcf;     // Polarity Config Register
    UINT8   unused30[1];
    UINT8   lacdrc;     // ACD (M) Rate Control Register
    UINT8   unused31[1];
    UINT8   lpxcd;      // Pixel Clock Divider Register
    UINT8   unused32[1];
    UINT8   lckcon;     // Clocking Control Register
    UINT8   unused33[1];
    UINT8   llbar;      // Last Buffer Address Register
    UINT8   unused34[1];
    UINT8   lotcr;      // Octet Terminal Count Register
    UINT8   unused35[1];
    UINT8   lposr;      // Panning Offset Register
    UINT8   unused36[3];
    UINT8   lfrcm;      // Frame Rate Control Modulation Register
    UINT16  lgpmr;      // Gray Palette Mapping Register
    UINT8   unused37[204];

    // $(FF)FFFB00
    UINT32  hmsr;       // RTC Hours Minutes Seconds Register
    UINT32  alarm;      // RTC Alarm Register
    UINT8   unused38[4];
    UINT16  rtcctl;     // RTC Control Register
    UINT16  rtcisr;     // RTC Interrupt Status Register
    UINT16  rtcienr;    // RTC Interrupt Enable Register
    UINT16  stpwtch;    // Stopwatch Minutes
    UINT8   unused42[1260];
} mc68328_regs_t;

typedef struct
{
    const mc68328_interface* iface;

    mc68328_regs_t regs;

    emu_timer *gptimer[2];
    emu_timer *rtc;
    emu_timer *pwm;
} mc68328_t;

#define SCR_BETO                0x80
#define SCR_WPV                 0x40
#define SCR_PRV                 0x20
#define SCR_BETEN               0x10
#define SCR_SO                  0x08
#define SCR_DMAP                0x04
#define SCR_WDTH8               0x01

#define ICR_POL6                0x0100
#define ICR_POL3                0x0200
#define ICR_POL2                0x0400
#define ICR_POL1                0x0800
#define ICR_ET6                 0x1000
#define ICR_ET3                 0x2000
#define ICR_ET2                 0x4000
#define ICR_ET1                 0x8000

#define INT_SPIM                0x000001
#define INT_TIMER2              0x000002
#define INT_UART                0x000004
#define INT_WDT                 0x000008
#define INT_RTC                 0x000010
#define INT_RESERVED            0x000020
#define INT_KB                  0x000040
#define INT_PWM                 0x000080
#define INT_INT0                0x000100
#define INT_INT1                0x000200
#define INT_INT2                0x000400
#define INT_INT3                0x000800
#define INT_INT4                0x001000
#define INT_INT5                0x002000
#define INT_INT6                0x004000
#define INT_INT7                0x008000
#define INT_KBDINTS             0x00ff00
#define INT_IRQ1                0x010000
#define INT_IRQ2                0x020000
#define INT_IRQ3                0x040000
#define INT_IRQ6                0x080000
#define INT_PEN                 0x100000
#define INT_SPIS                0x200000
#define INT_TIMER1              0x400000
#define INT_IRQ7                0x800000

#define INT_M68K_LINE1          (INT_IRQ1)
#define INT_M68K_LINE2          (INT_IRQ2)
#define INT_M68K_LINE3          (INT_IRQ3)
#define INT_M68K_LINE4          (INT_INT0 | INT_INT1 | INT_INT2 | INT_INT3 | INT_INT4 | INT_INT5 | INT_INT6 | INT_INT7 | \
                                 INT_PWM | INT_KB | INT_RTC | INT_WDT | INT_UART | INT_TIMER2 | INT_SPIM)
#define INT_M68K_LINE5          (INT_PEN)
#define INT_M68K_LINE6          (INT_IRQ6 | INT_TIMER1 | INT_SPIS)
#define INT_M68K_LINE7          (INT_IRQ7)
#define INT_M68K_LINE67         (INT_M68K_LINE6 | INT_M68K_LINE7)
#define INT_M68K_LINE567        (INT_M68K_LINE5 | INT_M68K_LINE6 | INT_M68K_LINE7)
#define INT_M68K_LINE4567       (INT_M68K_LINE4 | INT_M68K_LINE5 | INT_M68K_LINE6 | INT_M68K_LINE7)
#define INT_M68K_LINE34567      (INT_M68K_LINE3 | INT_M68K_LINE4 | INT_M68K_LINE5 | INT_M68K_LINE6 | INT_M68K_LINE7)
#define INT_M68K_LINE234567     (INT_M68K_LINE2 | INT_M68K_LINE3 | INT_M68K_LINE4 | INT_M68K_LINE5 | INT_M68K_LINE6 | INT_M68K_LINE7)

#define INT_IRQ1_SHIFT          0x000001
#define INT_IRQ2_SHIFT          0x000002
#define INT_IRQ3_SHIFT          0x000004
#define INT_IRQ6_SHIFT          0x000008
#define INT_PEN_SHIFT           0x000010
#define INT_SPIS_SHIFT          0x000020
#define INT_TIMER1_SHIFT        0x000040
#define INT_IRQ7_SHIFT          0x000080

#define INT_ACTIVE              1
#define INT_INACTIVE            0

#define GRPBASE_BASE_ADDR       0xfff0
#define GRPBASE_VALID           0x0001

#define GRPMASK_BASE_MASK       0xfff0

#define CSAB_COMPARE            0xff000000
#define CSAB_BSW                0x00010000
#define CSAB_MASK               0x0000ff00
#define CSAB_RO                 0x00000008
#define CSAB_WAIT               0x00000007

#define CSCD_COMPARE            0xfff00000
#define CSCD_BSW                0x00010000
#define CSCD_MASK               0x0000fff0
#define CSCD_RO                 0x00000008
#define CSCD_WAIT               0x00000007

#define PLLCR_PIXCLK_SEL        0x3800
#define PLLCR_PIXCLK_SEL_DIV2       0x0000
#define PLLCR_PIXCLK_SEL_DIV4       0x0800
#define PLLCR_PIXCLK_SEL_DIV8       0x1000
#define PLLCR_PIXCLK_SEL_DIV16      0x1800
#define PLLCR_PIXCLK_SEL_DIV1_0     0x2000
#define PLLCR_PIXCLK_SEL_DIV1_1     0x2800
#define PLLCR_PIXCLK_SEL_DIV1_2     0x3000
#define PLLCR_PIXCLK_SEL_DIV1_3     0x3800
#define PLLCR_SYSCLK_SEL        0x0700
#define PLLCR_SYSCLK_SEL_DIV2       0x0000
#define PLLCR_SYSCLK_SEL_DIV4       0x0100
#define PLLCR_SYSCLK_SEL_DIV8       0x0200
#define PLLCR_SYSCLK_SEL_DIV16      0x0300
#define PLLCR_SYSCLK_SEL_DIV1_0     0x0400
#define PLLCR_SYSCLK_SEL_DIV1_1     0x0500
#define PLLCR_SYSCLK_SEL_DIV1_2     0x0600
#define PLLCR_SYSCLK_SEL_DIV1_3     0x0700
#define PLLCR_CLKEN             0x0010
#define PLLCR_DISPLL            0x0008

#define PLLFSR_CLK32            0x8000
#define PLLFSR_PROT             0x4000
#define PLLFSR_QCNT             0x0f00
#define PLLFSR_PCNT             0x00ff

#define PCTLR_PC_EN             0x80
#define PCTLR_STOP              0x40
#define PCTLR_WIDTH             0x1f

#define CXP_CC                  0xc000
#define CXP_CC_XLU                  0x0000
#define CXP_CC_BLACK                0x4000
#define CXP_CC_INVERSE              0x8000
#define CXP_CC_INVALID              0xc000
#define CXP_MASK                0x03ff

#define CYP_MASK                0x01ff

#define CWCH_CW                 0x1f00
#define CWCH_CH                 0x001f

#define BLKC_BKEN               0x80
#define BLKC_BD                 0x7f

#define LPICF_PBSIZ             0x06
#define LPICF_PBSIZ_1               0x00
#define LPICF_PBSIZ_2               0x02
#define LPICF_PBSIZ_4               0x04
#define LPICF_PBSIZ_INVALID         0x06

#define LPOLCF_LCKPOL           0x08
#define LPOLCF_FLMPOL           0x04
#define LPOLCF_LPPOL            0x02
#define LPOLCF_PIXPOL           0x01

#define LACDRC_MASK             0x0f

#define LPXCD_MASK              0x3f

#define LCKCON_LCDC_EN          0x80
#define LCKCON_LCDON            0x80
#define LCKCON_DMA16            0x40
#define LCKCON_WS               0x30
#define LCKCON_WS_1                 0x00
#define LCKCON_WS_2                 0x10
#define LCKCON_WS_3                 0x20
#define LCKCON_WS_4                 0x30
#define LCKCON_DWIDTH           0x02
#define LCKCON_PCDS             0x01

#define LBAR_MASK               0x7f

#define LPOSR_BOS               0x08
#define LPOSR_POS               0x07

#define LFRCM_XMOD              0xf0
#define LFRCM_YMOD              0x0f

#define LGPMR_PAL1              0x7000
#define LGPMR_PAL0              0x0700
#define LGPMR_PAL3              0x0070
#define LGPMR_PAL2              0x0007

#define RTCHMSR_HOURS           0x1f000000
#define RTCHMSR_MINUTES         0x003f0000
#define RTCHMSR_SECONDS         0x0000003f

#define RTCCTL_38_4             0x0020
#define RTCCTL_ENABLE           0x0080

#define RTCINT_STOPWATCH        0x0001
#define RTCINT_MINUTE           0x0002
#define RTCINT_ALARM            0x0004
#define RTCINT_DAY              0x0008
#define RTCINT_SECOND           0x0010

#define RTCSTPWTCH_MASK         0x003f

#define TCTL_TEN                0x0001
#define TCTL_TEN_ENABLE             0x0001
#define TCTL_CLKSOURCE          0x000e
#define TCTL_CLKSOURCE_STOP         0x0000
#define TCTL_CLKSOURCE_SYSCLK       0x0002
#define TCTL_CLKSOURCE_SYSCLK16     0x0004
#define TCTL_CLKSOURCE_TIN          0x0006
#define TCTL_CLKSOURCE_32KHZ4       0x0008
#define TCTL_CLKSOURCE_32KHZ5       0x000a
#define TCTL_CLKSOURCE_32KHZ6       0x000c
#define TCTL_CLKSOURCE_32KHZ7       0x000e
#define TCTL_IRQEN              0x0010
#define TCTL_IRQEN_ENABLE           0x0010
#define TCTL_OM                 0x0020
#define TCTL_OM_ACTIVELOW           0x0000
#define TCTL_OM_TOGGLE              0x0020
#define TCTL_CAPTURE            0x00c0
#define TCTL_CAPTURE_NOINT          0x0000
#define TCTL_CAPTURE_RISING         0x0040
#define TCTL_CAPTURE_FALLING        0x0080
#define TCTL_CAPTURE_BOTH           0x00c0
#define TCTL_FRR                0x0100
#define TCTL_FRR_RESTART            0x0000
#define TCTL_FRR_FREERUN            0x0100

#define TSTAT_COMP              0x0001
#define TSTAT_CAPT              0x0002

#define WCTLR_WDRST             0x0008
#define WCTLR_LOCK              0x0004
#define WCTLR_FI                0x0002
#define WCTLR_WDEN              0x0001

#define USTCNT_UART_EN          0x8000
#define USTCNT_RX_EN            0x4000
#define USTCNT_TX_EN            0x2000
#define USTCNT_RX_CLK_CONT      0x1000
#define USTCNT_PARITY_EN        0x0800
#define USTCNT_ODD_EVEN         0x0400
#define USTCNT_STOP_BITS        0x0200
#define USTCNT_8_7              0x0100
#define USTCNT_GPIO_DELTA_EN    0x0080
#define USTCNT_CTS_DELTA_EN     0x0040
#define USTCNT_RX_FULL_EN       0x0020
#define USTCNT_RX_HALF_EN       0x0010
#define USTCNT_RX_RDY_EN        0x0008
#define USTCNT_TX_EMPTY_EN      0x0004
#define USTCNT_TX_HALF_EN       0x0002
#define USTCNT_TX_AVAIL_EN      0x0001

#define UBAUD_GPIO_DELTA        0x8000
#define UBAUD_GPIO              0x4000
#define UBAUD_GPIO_DIR          0x2000
#define UBAUD_GPIO_SRC          0x1000
#define UBAUD_BAUD_SRC          0x0800
#define UBAUD_DIVIDE            0x0700
#define UBAUD_DIVIDE_1              0x0000
#define UBAUD_DIVIDE_2              0x0100
#define UBAUD_DIVIDE_4              0x0200
#define UBAUD_DIVIDE_8              0x0300
#define UBAUD_DIVIDE_16             0x0400
#define UBAUD_DIVIDE_32             0x0500
#define UBAUD_DIVIDE_64             0x0600
#define UBAUD_DIVIDE_128            0x0700
#define UBAUD_PRESCALER         0x00ff

#define URX_FIFO_FULL           0x8000
#define URX_FIFO_HALF           0x4000
#define URX_DATA_READY          0x2000
#define URX_OVRUN               0x0800
#define URX_FRAME_ERROR         0x0400
#define URX_BREAK               0x0200
#define URX_PARITY_ERROR        0x0100

#define UTX_FIFO_EMPTY          0x8000
#define UTX_FIFO_HALF           0x4000
#define UTX_TX_AVAIL            0x2000
#define UTX_SEND_BREAK          0x1000
#define UTX_IGNORE_CTS          0x0800
#define UTX_CTS_STATUS          0x0200
#define UTX_CTS_DELTA           0x0100

#define UMISC_CLK_SRC           0x4000
#define UMISC_FORCE_PERR        0x2000
#define UMISC_LOOP              0x1000
#define UMISC_RTS_CONT          0x0080
#define UMISC_RTS               0x0040
#define UMISC_IRDA_ENABLE       0x0020
#define UMISC_IRDA_LOOP         0x0010

#define SPIS_SPIS_IRQ           0x8000
#define SPIS_IRQEN              0x4000
#define SPIS_ENPOL              0x2000
#define SPIS_DATA_RDY           0x1000
#define SPIS_OVRWR              0x0800
#define SPIS_PHA                0x0400
#define SPIS_POL                0x0200
#define SPIS_SPISEN             0x0100

#define SPIM_CLOCK_COUNT        0x000f
#define SPIM_POL                0x0010
#define SPIM_POL_HIGH               0x0000
#define SPIM_POL_LOW                0x0010
#define SPIM_PHA                0x0020
#define SPIM_PHA_NORMAL             0x0000
#define SPIM_PHA_OPPOSITE           0x0020
#define SPIM_IRQEN              0x0040
#define SPIM_SPIMIRQ            0x0080
#define SPIM_XCH                0x0100
#define SPIM_XCH_IDLE               0x0000
#define SPIM_XCH_INIT               0x0100
#define SPIM_SPMEN              0x0200
#define SPIM_SPMEN_DISABLE          0x0000
#define SPIM_SPMEN_ENABLE           0x0200
#define SPIM_RATE               0xe000
#define SPIM_RATE_4                 0x0000
#define SPIM_RATE_8                 0x2000
#define SPIM_RATE_16                0x4000
#define SPIM_RATE_32                0x6000
#define SPIM_RATE_64                0x8000
#define SPIM_RATE_128               0xa000
#define SPIM_RATE_256               0xc000
#define SPIM_RATE_512               0xe000

#define PWMC_PWMIRQ             0x8000
#define PWMC_IRQEN              0x4000
#define PWMC_LOAD               0x0100
#define PWMC_PIN                0x0080
#define PWMC_POL                0x0040
#define PWMC_PWMEN              0x0010
#define PWMC_CLKSEL             0x0007

INLINE mc68328_t* mc68328_get_safe_token( device_t *device )
{
    assert( device != NULL );
    assert( device->type() == MC68328 );
    return (mc68328_t*) downcast<legacy_device_base *>(device)->token();
}

#endif // __MC68328_PRIVATE_H_
