/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2012 by GSport contributors
 
 Based on the KEGS emulator written by and Copyright (C) 2003 Kent Dickey

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or (at your 
 option) any later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


/* adb.c */
void adb_init(void);
void adb_shut(void); // OG Added adb_shut()
void adb_reset(void);
void adb_log(word32 addr, int val);
void show_adb_log(void);
void adb_error(void);
void adb_add_kbd_srq(void);
void adb_clear_kbd_srq(void);
void adb_add_data_int(void);
void adb_add_mouse_int(void);
void adb_clear_data_int(void);
void adb_clear_mouse_int(void);
void adb_send_bytes(int num_bytes, word32 val0, word32 val1, word32 val2);
void adb_send_1byte(word32 val);
void adb_response_packet(int num_bytes, word32 val);
void adb_kbd_reg0_data(int a2code, int is_up);
void adb_kbd_talk_reg0(void);
void adb_set_config(word32 val0, word32 val1, word32 val2);
void adb_set_new_mode(word32 val);
int adb_read_c026(void);
void adb_write_c026(int val);
void do_adb_cmd(void);
int adb_read_c027(void);
void adb_write_c027(int val);
int read_adb_ram(word32 addr);
void write_adb_ram(word32 addr, int val);
int adb_get_keypad_xy(int get_y);
int update_mouse(int x, int y, int button_states, int buttons_valid);
int mouse_read_c024(double dcycs);
void mouse_compress_fifo(double dcycs);
void adb_key_event(int a2code, int is_up);
word32 adb_read_c000(void);
word32 adb_access_c010(void);
word32 adb_read_c025(void);
int adb_is_cmd_key_down(void);
int adb_is_option_key_down(void);
void adb_increment_speed(void);
void adb_physical_key_update(int a2code, int is_up);
void adb_virtual_key_update(int a2code, int is_up);
void adb_all_keys_up(void);
void adb_kbd_repeat_off(void);
