/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

// file will be included in all cpu variants
// timing value should move to separate array

static void PREFIX186(_pusha)(void);
static void PREFIX186(_popa)(void);
static void PREFIX186(_bound)(void);
static void PREFIX186(_push_d16)(void);
static void PREFIX186(_imul_d16)(void);
static void PREFIX186(_push_d8)(void);
static void PREFIX186(_imul_d8)(void);
static void PREFIX186(_rotshft_bd8)(void);
static void PREFIX186(_rotshft_wd8)(void);
static void PREFIX186(_enter)(void);
static void PREFIX186(_leave)(void);
static void PREFIX186(_insb)(void);
static void PREFIX186(_insw)(void);
static void PREFIX186(_outsb)(void);
static void PREFIX186(_outsw)(void);

/* changed instructions */
static void PREFIX186(_repne)(void);
static void PREFIX186(_repe)(void);
