#ifndef SNESDSP4_H
#define SNESDSP4_H

#define FALSE 0
#define TRUE 1


struct DSP4_t
{
  UINT8 waiting4command;
  UINT8 half_command;
  UINT16 command;
  UINT32 in_count;
  UINT32 in_index;
  UINT32 out_count;
  UINT32 out_index;
  UINT8 parameters[512];
  UINT8 output[512];
};

extern struct DSP4_t DSP4;

struct DSP4_vars_t
{
  // op control
  INT8 DSP4_Logic;            // controls op flow


  // projection format
  INT16 lcv;                  // loop-control variable
  INT16 distance;             // z-position INTo virtual world
  INT16 raster;               // current raster line
  INT16 segments;             // number of raster lines drawn

  // 1.15.16 or 1.15.0 [sign, INTeger, fraction]
  INT32 world_x;              // line of x-projection in world
  INT32 world_y;              // line of y-projection in world
  INT32 world_dx;             // projection line x-delta
  INT32 world_dy;             // projection line y-delta
  INT16 world_ddx;            // x-delta increment
  INT16 world_ddy;            // y-delta increment
  INT32 world_xenv;           // world x-shaping factor
  INT16 world_yofs;           // world y-vertical scroll

  INT16 view_x1;              // current viewer-x
  INT16 view_y1;              // current viewer-y
  INT16 view_x2;              // future viewer-x
  INT16 view_y2;              // future viewer-y
  INT16 view_dx;              // view x-delta factor
  INT16 view_dy;              // view y-delta factor
  INT16 view_xofs1;           // current viewer x-vertical scroll
  INT16 view_yofs1;           // current viewer y-vertical scroll
  INT16 view_xofs2;           // future viewer x-vertical scroll
  INT16 view_yofs2;           // future viewer y-vertical scroll
  INT16 view_yofsenv;         // y-scroll shaping factor
  INT16 view_turnoff_x;       // road turnoff data
  INT16 view_turnoff_dx;      // road turnoff delta factor


  // drawing area

  INT16 viewport_cx;          // x-center of viewport window
  INT16 viewport_cy;          // y-center of render window
  INT16 viewport_left;        // x-left of viewport
  INT16 viewport_right;       // x-right of viewport
  INT16 viewport_top;         // y-top of viewport
  INT16 viewport_bottom;      // y-bottom of viewport


  // sprite structure

  INT16 sprite_x;             // projected x-pos of sprite
  INT16 sprite_y;             // projected y-pos of sprite
  INT16 sprite_attr;          // obj attributes
  UINT8 sprite_size;          // sprite size: 8x8 or 16x16
  INT16 sprite_clipy;         // visible line to clip pixels off
  INT16 sprite_count;

  // generic projection variables designed for
  // two solid polygons + two polygon sides

  INT16 poly_clipLf[2][2];    // left clip boundary
  INT16 poly_clipRt[2][2];    // right clip boundary
  INT16 poly_ptr[2][2];       // HDMA structure poINTers
  INT16 poly_raster[2][2];    // current raster line below horizon
  INT16 poly_top[2][2];       // top clip boundary
  INT16 poly_bottom[2][2];    // bottom clip boundary
  INT16 poly_cx[2][2];        // center for left/right poINTs
  INT16 poly_start[2];        // current projection poINTs
  INT16 poly_plane[2];        // previous z-plane distance


  // OAM
  INT16 OAM_attr[16];         // OAM (size,MSB) data
  INT16 OAM_index;            // index INTo OAM table
  INT16 OAM_bits;             // offset INTo OAM table

  INT16 OAM_RowMax;           // maximum number of tiles per 8 aligned pixels (row)
  INT16 OAM_Row[32];          // current number of tiles per row
};

extern struct DSP4_vars_t DSP4_vars;

#endif
