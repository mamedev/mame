#ifndef DS2404_H
#define DS2404_H

void DS2404_init(int ref_year, int ref_month, int ref_day);
void DS2404_load(mame_file *file);
void DS2404_save(mame_file *file);

/* 1-wire interface reset */
WRITE8_HANDLER( DS2404_1W_reset_w );

/* 3-wire interface reset  */
WRITE8_HANDLER( DS2404_3W_reset_w );

READ8_HANDLER( DS2404_data_r );
WRITE8_HANDLER( DS2404_data_w );
WRITE8_HANDLER( DS2404_clk_w );

#endif
