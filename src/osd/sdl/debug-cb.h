#include <gtk/gtk.h>

void
on_new_mem_activate                    (GtkWidget *win);

void
on_new_disasm_activate                (GtkWidget *win);

void
on_run_activate                       (GtkWidget *win);

void
on_run_h_activate                     (GtkWidget *win);

void
on_run_cpu_activate                   (GtkWidget *win);

void
on_run_irq_activate                   (GtkWidget *win);

void
on_run_vbl_activate                   (GtkWidget *win);

void
on_step_into_activate                 (GtkWidget *win);

void
on_step_over_activate                 (GtkWidget *win);

void
on_step_out_activate                  (GtkWidget *win);

GtkWidget*
debug_view_new (gchar *widget_name, gchar *string1, gchar *string2,
				gint int1, gint int2);

GtkWidget*
dview_new (const gchar *widget_name, const gchar *string1, const gchar *string2,
				gint int1, gint int2);

void
on_raw_opcodes_activate               (GtkWidget *win);

void
on_enc_opcodes_activate               (GtkWidget *win);

void
on_comments_activate                  (GtkWidget *win);

void
on_new_errorlog_activate              (GtkWidget *win);

void
on_soft_reset_activate                (GtkWidget *win);

void
on_hard_reset_activate                (GtkWidget *win);

void
on_exit_activate                      (GtkWidget *win);

void
on_chunks_1_activate                  (GtkWidget *win);

void
on_chunks_2_activate                  (GtkWidget *win);

void
on_chunks_4_activate                  (GtkWidget *win);

void
on_reverse_activate                   (GtkWidget *win);

void
on_ibpl_activate                      (GtkWidget *win);

void
on_dbpl_activate                      (GtkWidget *win);

GtkWidget*
dview_new (const gchar *widget_name, const gchar *string1, const gchar *string2,
				gint int1, gint int2);

void
on_run_to_cursor_activate             (GtkWidget *win);

void
on_set_breakpoint_at_cursor_activate             (GtkWidget *win);

gboolean
on_disasm_button_press_event           (GtkWidget       *widget,
										GdkEventButton  *event,
										gpointer         user_data);

gboolean
on_memoryview_button_press_event           (GtkWidget       *widget,
										GdkEventButton  *event,
										gpointer         user_data);

gboolean
on_memoryview_key_press_event             (GtkWidget   *widget,
														GdkEventKey *event,
														gpointer     user_data);

void
on_logical_addresses_group_changed     (GtkWidget *win);

void
on_physical_addresses_group_changed    (GtkWidget *win);
