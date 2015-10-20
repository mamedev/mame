// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII RAM related tasks
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

#if (ALTO2_CRAM_CONFIG==1)
#define ALTO2_UCODE_ROM_PAGES   1           //!< number of microcode ROM pages
#define ALTO2_UCODE_RAM_PAGES   1           //!< number of microcode RAM pages
#elif (ALTO2_CRAM_CONFIG==2)
#define ALTO2_UCODE_ROM_PAGES   2           //!< number of microcode ROM pages
#define ALTO2_UCODE_RAM_PAGES   1           //!< number of microcode RAM pages
#elif (ALTO2_CRAM_CONFIG==3)
#define ALTO2_UCODE_ROM_PAGES   1           //!< number of microcode ROM pages
#define ALTO2_UCODE_RAM_PAGES   3           //!< number of microcode RAM pages
#else
#error "Undefined CROM/CRAM configuration"
#endif

/**
 * \brief number of S register banks
 * This depends on the number of RAM pages
 *   8 pages in 3K CRAM configuration
 *   1 page in 1K CRAM configurations
 */
#if (ALTO2_UCODE_RAM_PAGES == 3)
#define ALTO2_SREG_BANKS    8
#else
#define ALTO2_SREG_BANKS    1
#endif

#define ALTO2_UCODE_PAGE_SIZE   02000                       //!< number of words of microcode
#define ALTO2_UCODE_PAGE_MASK   (ALTO2_UCODE_PAGE_SIZE-1)   //!< mask for microcode ROM/RAM address
#define ALTO2_UCODE_SIZE        ((ALTO2_UCODE_ROM_PAGES + ALTO2_UCODE_RAM_PAGES) * ALTO2_UCODE_PAGE_SIZE)   //!< total number of words of microcode
#define ALTO2_UCODE_RAM_BASE    (ALTO2_UCODE_ROM_PAGES * ALTO2_UCODE_PAGE_SIZE) //!< base offset for the RAM page(s)

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2RAM_H_
#define _A2RAM_H_
//! BUS source for RAM related tasks
enum {
	bs_ram_read_slocation= bs_task_3,           //!< ram related: read S register
	bs_ram_load_slocation= bs_task_4            //!< ram related: load S register
};

//!< F1 functions for RAM related tasks
enum {
	f1_ram_swmode       = f1_task_10,           //!< f1 10: switch mode to CROM/CRAM in same page
	f1_ram_wrtram       = f1_task_11,           //!< f1 11: start WRTRAM cycle
	f1_ram_rdram        = f1_task_12,           //!< f1 12: start RDRAM cycle
#if (ALTO2_UCODE_RAM_PAGES == 3)
	f1_ram_load_rmr     = f1_task_13,           //!< f1 13: load the reset mode register
#else   // ALTO2_UCODE_RAM_PAGES != 3
	f1_ram_load_srb     = f1_task_13            //!< f1 14: load the S register bank from BUS[12-14]
#endif
};

void bs_early_read_sreg();                      //!< bus source: drive bus by S register or M (MYL), if rsel is = 0
void bs_early_load_sreg();                      //!< bus source: load S register puts garbage on the bus
void bs_late_load_sreg();                       //!< bus source: load S register from M
void branch_ROM(const char *from, int page);    //!< branch to ROM page
void branch_RAM(const char *from, int page);    //!< branch to RAM page
void f1_late_swmode();                          //!< F1 func: switch to micro program counter BUS[6-15] in other bank
void f1_late_wrtram();                          //!< F1 func: start WRTRAM cycle
void f1_late_rdram();                           //!< F1 func: start RDRAM cycle
#if (ALTO2_UCODE_RAM_PAGES == 3)
void f1_late_load_rmr();                        //!< F1 func: load the reset mode register
#else   // ALTO2_UCODE_RAM_PAGES != 3
void f1_late_load_srb();                        //!< F1 func: load the S register bank from BUS[12-14]
#endif
void init_ram(int task);                        //!< called by RAM related tasks
void exit_ram();                                //!< deinitialize the RAM related tasks
void reset_ram();                               //!< reset the RAM related tasks
#endif // A2RAM_H
#endif  // ALTO2_DEFINE_CONSTANTS
