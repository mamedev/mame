
#include "dview.h"
#ifndef SDLMAME_WIN32
#include <gconf/gconf-client.h>
#endif

G_DEFINE_TYPE(DView, dview, GTK_TYPE_CONTAINER);

static gboolean dview_expose(GtkWidget *wdv, GdkEventExpose *event)
{
	const debug_view_char *viewdata;
	DView *dv = DVIEW(wdv);
	DViewClass *dvc = DVIEW_GET_CLASS(dv);
	debug_view_xy vsize;
	UINT32 i, j, k, l, xx, yy;
	GdkColor bg, fg;
	char s[256];

	vsize = dv->view->visible_size();

	bg.red = bg.green = bg.blue = 0xffff;
	gdk_gc_set_rgb_fg_color(dv->gc, &bg);
	gdk_draw_rectangle(GDK_DRAWABLE(wdv->window), dv->gc, TRUE, 0, 0, wdv->allocation.width - (dv->vs ? dv->vsz : 0), wdv->allocation.height - (dv->hs ? dv->hsz : 0));

	if(dv->hs && dv->vs) {
		gdk_gc_set_foreground(dv->gc, &wdv->style->bg[GTK_STATE_NORMAL]);
		gdk_draw_rectangle(GDK_DRAWABLE(wdv->window), dv->gc, TRUE,
							wdv->allocation.width - dv->vsz, wdv->allocation.height - dv->hsz,
							dv->vsz, dv->hsz);
	}

	viewdata = dv->view->viewdata();

	yy = wdv->style->ythickness;
	for(j=0; j<vsize.y; j++) {
		k = l = 0;
		xx = wdv->style->xthickness;
		for(i=0; i<vsize.x; i++) {
			unsigned char attr = viewdata->attrib;
			unsigned char v = viewdata->byte;

			if(v < 128) {
				s[k++] = v;
			} else {
				s[k++] = 0xc0 | (v>>6);
				s[k++] = 0x80 | (v & 0x3f);
			}
			l++;

			if ( i == 0 || attr != viewdata[-1].attrib ) {
				bg.red = bg.green = bg.blue = 0xffff;
				fg.red = fg.green = fg.blue = 0;

				if(attr & DCA_ANCILLARY)
					bg.red = bg.green = bg.blue = 0xe0e0;
				if(attr & DCA_SELECTED) {
					bg.red = 0xffff;
					bg.green = bg.blue = 0x8080;
				}
				if(attr & DCA_CURRENT) {
					bg.red = bg.green = 0xffff;
					bg.blue = 0;
				}
				if(attr & DCA_CHANGED) {
					fg.red = 0xffff;
					fg.green = fg.blue = 0;
				}
				if(attr & DCA_INVALID) {
					fg.red = fg.green = 0;
					fg.blue = 0xffff;
				}
				if(attr & DCA_DISABLED) {
					fg.red   = (fg.red   + bg.red)   >> 1;
					fg.green = (fg.green + bg.green) >> 1;
					fg.blue  = (fg.blue  + bg.blue)   >> 1;
				}
				if(attr & DCA_COMMENT) {
					fg.red = fg.blue = 0;
					fg.green = 0x8080;
				}
			}

			if ( i == vsize.x - 1 || attr != viewdata[1].attrib || k >= 254 ) {
				s[k++] = 0;
				pango_layout_set_text(dv->playout, s, -1);
				gdk_gc_set_rgb_fg_color(dv->gc, &bg);
				gdk_draw_rectangle(GDK_DRAWABLE(wdv->window), dv->gc, TRUE, xx, yy, l * dvc->fixedfont_width, dvc->fixedfont_height);
				gdk_gc_set_rgb_fg_color(dv->gc, &fg);
				gdk_draw_layout(GDK_DRAWABLE(wdv->window), dv->gc, xx, yy, dv->playout);

				xx += l * dvc->fixedfont_width;
				l = k = 0;
			}
			viewdata++;
		}
		yy += dvc->fixedfont_height;
	}

	gtk_paint_shadow(wdv->style, wdv->window,
						GTK_STATE_NORMAL, GTK_SHADOW_IN,
						&event->area, wdv, "scrolled_window",
						0, 0,
						wdv->allocation.width - (dv->vs ? dv->vsz : 0),
						wdv->allocation.height - (dv->hs ? dv->hsz : 0));

	GTK_WIDGET_CLASS(g_type_class_peek_parent(dvc))->expose_event(wdv, event);

	return FALSE;
}

static void dview_hadj_changed(GtkAdjustment *adj, DView *dv)
{
	debug_view_xy pos;
	UINT32 v = (UINT32)(adj->value);

	pos = dv->view->visible_position();

	if (v != pos.x)
	{
		pos.x = v;
		dv->view->set_visible_position(pos);
		gtk_widget_queue_draw(GTK_WIDGET(dv));
	}
}

static void dview_vadj_changed(GtkAdjustment *adj, DView *dv)
{
	debug_view_xy pos;
	UINT32 v = (UINT32)(adj->value);

	pos = dv->view->visible_position();

	if (v != pos.y)
	{
		pos.y = v;
		dv->view->set_visible_position(pos);
		gtk_widget_queue_draw(GTK_WIDGET(dv));
	}
}

static void dview_realize(GtkWidget *wdv)
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	DView *dv;

	GTK_WIDGET_SET_FLAGS(wdv, GTK_REALIZED | GTK_CAN_FOCUS);
	dv = DVIEW(wdv);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = wdv->allocation.x;
	attributes.y = wdv->allocation.y;
	attributes.width = wdv->allocation.width;
	attributes.height = wdv->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual(wdv);
	attributes.colormap = gtk_widget_get_colormap(wdv);
	attributes.event_mask = gtk_widget_get_events(wdv) | GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK
			| GDK_FOCUS_CHANGE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	wdv->window = gdk_window_new(gtk_widget_get_parent_window(wdv), &attributes, attributes_mask);
	gdk_window_set_user_data(wdv->window, dv);
	dv->gc = gdk_gc_new(GDK_DRAWABLE(wdv->window));
	wdv->style = gtk_style_attach(wdv->style, wdv->window);
}

static void dview_size_allocate(GtkWidget *wdv, GtkAllocation *allocation)
{
	DView *dv = DVIEW(wdv);
	DViewClass *dvc = DVIEW_GET_CLASS(dv);
	UINT32 ah   = allocation->height-2*wdv->style->ythickness;
	UINT32 aw   = allocation->width-2*wdv->style->xthickness;
	int ohs = dv->hs;
	int ovs = dv->vs;
	debug_view_xy size, pos, col, vsize;

	pos = dv->view->visible_position();
	size = dv->view->total_size();

	dv->tr = size.y;
	dv->tc = size.x;

	dv->hs = (size.x*dvc->fixedfont_width > aw ? 1 : 0);
	dv->vs = (size.y*dvc->fixedfont_height > ah ? 1 : 0);

	if(dv->hs)
		ah -= dv->hsz;
	if(dv->vs)
		aw -= dv->vsz;

	dv->hs = (size.x*dvc->fixedfont_width > aw ? 1 : 0);
	dv->vs = (size.y*dvc->fixedfont_height > ah ? 1 : 0);

	ah = allocation->height - (dv->hs ? dv->hsz : 0);
	aw = allocation->width  - (dv->vs ? dv->vsz : 0);

	col.y = (ah-2*wdv->style->ythickness+dvc->fixedfont_height-1) / dvc->fixedfont_height;
	col.x = (aw-2*wdv->style->xthickness+dvc->fixedfont_width-1) / dvc->fixedfont_width;

	wdv->allocation = *allocation;

	vsize.y = size.y-pos.y;
	vsize.x = size.x-pos.x;
	if(vsize.y > col.y)
		vsize.y = col.y;
	else if(vsize.y < col.y) {
		pos.y = size.y-col.y;
		if(pos.y < 0)
			pos.y = 0;
		vsize.y = size.y-pos.y;
	}
	if(vsize.x > col.x)
		vsize.x = col.x;
	else if(vsize.x < col.x) {
		pos.x = size.x-col.x;
		if(pos.x < 0)
			pos.x = 0;
		vsize.x = size.x-pos.x;
	}

	/* FIXME: This does not really work */
	{
		GdkGeometry x;
		x.max_width = size.x*dvc->fixedfont_width;
		x.max_height = -1;
		if (wdv->window)
		{
			gdk_window_set_geometry_hints( wdv->window,&x, GDK_HINT_MAX_SIZE  );
		}
	}
	dv->view->set_visible_position(pos);
	dv->view->set_visible_size(vsize);

#ifdef GTK_WIDGET_REALIZED
	if(GTK_WIDGET_REALIZED(wdv))
#else
	if(gtk_widget_get_realized(wdv))
#endif
		gdk_window_move_resize(wdv->window,
								allocation->x, allocation->y,
								allocation->width, allocation->height);

	if(dv->hs) {
		GtkAllocation al;
		int span = (aw-2*wdv->style->xthickness) / dvc->fixedfont_width;

		if(!ohs)
			gtk_widget_show(dv->hscrollbar);
		al.x = 0;
		al.y = ah;
		al.width = aw;
		al.height = dv->hsz;
		gtk_widget_size_allocate(dv->hscrollbar, &al);
		if(pos.x+span > size.x)
			pos.x = size.x-span;
		if(pos.x < 0)
			pos.x = 0;
		dv->hadj->lower = 0;
		dv->hadj->upper = size.x;
		dv->hadj->value = pos.x;
		dv->hadj->step_increment = 1;
		dv->hadj->page_increment = span;
		dv->hadj->page_size = span;
		gtk_adjustment_changed(dv->hadj);
		dv->view->set_visible_position(pos);
	} else {
		if(ohs)
			gtk_widget_hide(dv->hscrollbar);
	}

	if(dv->vs) {
		GtkAllocation al;
		int span = (ah-2*wdv->style->ythickness) / dvc->fixedfont_height;

		if(!ovs)
			gtk_widget_show(dv->vscrollbar);
		al.x = aw;
		al.y = 0;
		al.width = dv->vsz;
		al.height = ah;
		gtk_widget_size_allocate(dv->vscrollbar, &al);
		if(pos.y+span > size.y)
			pos.y = size.y-span;
		if(pos.y < 0)
			pos.y = 0;
		dv->vadj->lower = 0;
		dv->vadj->upper = size.y;
		dv->vadj->value = pos.y;
		dv->vadj->step_increment = 1;
		dv->vadj->page_increment = span;
		dv->vadj->page_size = span;
		gtk_adjustment_changed(dv->vadj);
		dv->view->set_visible_position(pos);
	} else {
		if(ovs)
			gtk_widget_hide(dv->vscrollbar);
	}
}

static void dview_size_request(GtkWidget *wdv, GtkRequisition *req)
{
	GtkRequisition req2;
	DView *dv = DVIEW(wdv);
	DViewClass *dvc = DVIEW_GET_CLASS(dv);
	int vs = 0, hs = 0;
	debug_view_xy size;

	size = dv->view->total_size();

	if(size.x > 50) {
		size.x = 50;
		hs = 1;
	}
	if(size.y > 5) {
		size.y = 5;
		vs = 1;
	}
	req->width = size.x*dvc->fixedfont_width+2*wdv->style->xthickness;
	req->height = size.y*dvc->fixedfont_height+2*wdv->style->ythickness;

	gtk_widget_size_request(dv->hscrollbar, &req2);
	dv->hsz = req2.height;

	gtk_widget_size_request(dv->vscrollbar, &req2);
	dv->vsz = req2.width;

	if(hs)
		req->height += dv->hsz;
	if(vs)
		req->width += dv->vsz;
}

static void dview_forall(GtkContainer *dvc, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
	if(include_internals) {
		DView *dv = DVIEW(dvc);
		callback(dv->hscrollbar, callback_data);
		callback(dv->vscrollbar, callback_data);
	}
}


static void dview_class_init(DViewClass *dvc)
{
#ifndef SDLMAME_WIN32
	GConfClient *conf = gconf_client_get_default();
	char *name = 0;
	dvc->fixedfont = 0;

	if(conf)
		name = gconf_client_get_string(conf, "/desktop/gnome/interface/monospace_font_name", 0);

	if(name) {
		dvc->fixedfont = pango_font_description_from_string(name);
		g_free(name);
	}

	if(!dvc->fixedfont)
#endif
		dvc->fixedfont = pango_font_description_from_string("Monospace 10");

	if(!dvc->fixedfont) {
		mame_printf_error("Couldn't find a monospace font, aborting\n");
		abort();
	}

	GTK_CONTAINER_CLASS(dvc)->forall = dview_forall;
	GTK_WIDGET_CLASS(dvc)->expose_event = dview_expose;
	GTK_WIDGET_CLASS(dvc)->realize = dview_realize;
	GTK_WIDGET_CLASS(dvc)->size_request = dview_size_request;
	GTK_WIDGET_CLASS(dvc)->size_allocate = dview_size_allocate;
}


static void dview_init(DView *dv)
{
	DViewClass *dvc;

	dvc = DVIEW_GET_CLASS(dv);

	if(!dvc->fixedfont_width) {
		PangoFontMetrics *metrics;
		metrics = pango_context_get_metrics(gtk_widget_get_pango_context(GTK_WIDGET(dv)), dvc->fixedfont, 0);

		dvc->fixedfont_width = PANGO_PIXELS(pango_font_metrics_get_approximate_char_width(metrics));
		dvc->fixedfont_height = PANGO_PIXELS(pango_font_metrics_get_ascent(metrics) +
												pango_font_metrics_get_descent(metrics));
	}

	dv->view = 0;
	gtk_widget_modify_font(GTK_WIDGET(dv), dvc->fixedfont);
	dv->playout = gtk_widget_create_pango_layout(GTK_WIDGET(dv), 0);
	pango_layout_set_font_description(dv->playout, dvc->fixedfont);
	dv->gc = 0;

	dv->hadj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 1, 1));
	dv->hscrollbar = gtk_hscrollbar_new(dv->hadj);
	gtk_widget_set_parent(dv->hscrollbar, GTK_WIDGET(dv));
	g_object_ref(dv->hscrollbar);
	g_signal_connect(dv->hadj, "value-changed", G_CALLBACK(dview_hadj_changed), dv);

	dv->vadj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 1, 1));
	dv->vscrollbar = gtk_vscrollbar_new(dv->vadj);
	gtk_widget_set_parent(dv->vscrollbar, GTK_WIDGET(dv));
	g_object_ref(dv->vscrollbar);
	g_signal_connect(dv->vadj, "value-changed", G_CALLBACK(dview_vadj_changed), dv);
}

static void dview_update(debug_view &dw, void *osdprivate)
{
	DView *dv = (DView *) osdprivate;
	debug_view_xy size = dw.total_size();

	if((dv->tr != size.y) || (dv->tc != size.x))
		gtk_widget_queue_resize(GTK_WIDGET(dv));
	else
		gtk_widget_queue_draw(GTK_WIDGET(dv));
}

GtkWidget *dview_new(const gchar *widget_name, const gchar *string1, const gchar *string2, gint int1, gint int2)
{
	GtkWidget *wdv = (GtkWidget *) g_object_new(DVIEW_TYPE, NULL);
	DView *dv = DVIEW(wdv);
	dv->name = (gchar *) widget_name;
	return wdv;
}

void dview_set_debug_view(DView *dv, running_machine &machine, debug_view_type type)
{
	dv->view = machine.debug_view().alloc_view(type, dview_update, dv);
	dv->dv_type = type;
}
