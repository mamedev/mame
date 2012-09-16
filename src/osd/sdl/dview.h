#ifndef DVIEW_H
#define DVIEW_H

#include <gtk/gtk.h>

#include "emu.h"
#include "video.h"
#include "osdepend.h"

#include "debug/debugvw.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"

GType dview_get_type(void);

#define DVIEW_TYPE           (dview_get_type())
#define DVIEW(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), DVIEW_TYPE, DView))
#define DVIEW_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST((obj), DVIEW,  DViewClass))
#define IS_DVIEW(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), DVIEW_TYPE))
#define IS_DVIEW_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((obj), DVIEW_TYPE))
#define DVIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), DVIEW_TYPE, DViewClass))


struct  DViewClass;
struct  DView;

struct DViewClass
{
  GtkContainerClass parent_class;
  PangoFontDescription *fixedfont;
  int fixedfont_width, fixedfont_height;
};

struct DView
{
  GtkContainer parent;
  GtkAdjustment *hadj, *vadj;
  GtkWidget *hscrollbar, *vscrollbar;
  int hsz, vsz;
  int hs, vs;
  int tr, tc;
  gchar *name;
  PangoLayout *playout;
  GdkGC *gc;
  debug_view *view;
  int dv_type;
};


GtkWidget *dview_new(const gchar *widget_name, const gchar *string1, const gchar *string2, gint int1, gint int2);
void dview_set_debug_view(DView *dv, running_machine &machine, debug_view_type type);

#endif
