/*
    rtc65271.h: include file for rtc65271.c
*/

extern int rtc65271_file_load(running_machine *machine, mame_file *file);
extern int rtc65271_file_save(mame_file *file);
extern void rtc65271_init(running_machine *machine, UINT8 *xram, void (*interrupt_callback)(running_machine *machine, int state));
extern UINT8 rtc65271_r(int xramsel, offs_t offset);
extern void rtc65271_w(int xramsel, offs_t offset, UINT8 data);
extern READ8_HANDLER( rtc65271_rtc_r );
extern READ8_HANDLER( rtc65271_xram_r );
extern WRITE8_HANDLER( rtc65271_rtc_w );
extern WRITE8_HANDLER( rtc65271_xram_w );
