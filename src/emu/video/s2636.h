extern UINT8 *s2636_1_ram;
extern UINT8 *s2636_2_ram;
extern UINT8 *s2636_3_ram;

extern mame_bitmap *s2636_1_bitmap;
extern mame_bitmap *s2636_2_bitmap;
extern mame_bitmap *s2636_3_bitmap;

extern UINT8 s2636_1_dirty[4];
extern UINT8 s2636_2_dirty[4];
extern UINT8 s2636_3_dirty[4];

extern int s2636_x_offset;
extern int s2636_y_offset;

void s2636_w(UINT8 *workram,int offset,int data,UINT8 *dirty);
void s2636_update_bitmap(running_machine *machine,mame_bitmap *bitmap,UINT8 *workram,UINT8 *dirty,int Graphics_Bank,mame_bitmap *collision_bitmap);

