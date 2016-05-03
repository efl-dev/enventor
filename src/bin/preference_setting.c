#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "common.h"

typedef struct preference_setting_s
{
   Evas_Object *scroller;
   Evas_Object *slider_view;
   Evas_Object *view_size_w_entry;
   Evas_Object *view_size_h_entry;
   Evas_Object *toggle_highlight;
   Evas_Object *toggle_swallow;
   Evas_Object *toggle_stats;
   Evas_Object *toggle_file_browser;
   Evas_Object *toggle_edc_navigator;
   Evas_Object *toggle_tools;
   Evas_Object *toggle_console;

} preference_setting_data;

/*****************************************************************************/
/* Internal method implementation                                            */
/*****************************************************************************/
static Evas_Object *
label_create(Evas_Object *parent, const char *text)
{
   Evas_Object *label = elm_label_add(parent);
   elm_object_text_set(label, text);
   evas_object_show(label);

   return label;
}

static Evas_Object *
toggle_create(Evas_Object *parent, const char *text, Eina_Bool state)
{
   Evas_Object *toggle = elm_check_add(parent);
   elm_object_style_set(toggle, "toggle");
   elm_check_state_set(toggle, state);
   evas_object_size_hint_weight_set(toggle, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(toggle, EVAS_HINT_FILL, 0);
   elm_object_text_set(toggle, text);
   evas_object_show(toggle);

   return toggle;
}

static Evas_Object *
entry_create(Evas_Object *parent)
{
   Evas_Object *entry = elm_entry_add(parent);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   evas_object_show(entry);

   return entry;
}

/*****************************************************************************/
/* Externally accessible calls                                               */
/*****************************************************************************/
void
preference_setting_focus_set(preference_setting_data *psd)
{
   if (!psd) return;
   elm_object_focus_set(psd->slider_view, EINA_TRUE);
}

void
preference_setting_config_set(preference_setting_data *psd)
{
   if (!psd) return;

   config_view_scale_set(elm_slider_value_get(psd->slider_view));
   config_tools_set(elm_check_state_get(psd->toggle_tools));
   config_console_set(elm_check_state_get(psd->toggle_console));
   config_stats_bar_set(elm_check_state_get(psd->toggle_stats));
   config_part_highlight_set(elm_check_state_get(psd->toggle_highlight));
   config_dummy_parts_set(elm_check_state_get(psd->toggle_swallow));
   config_file_browser_set(elm_check_state_get(psd->toggle_file_browser));
   config_edc_navigator_set(elm_check_state_get(psd->toggle_edc_navigator));

   Evas_Coord w = 0;
   Evas_Coord h = 0;
   const char *w_entry = elm_entry_entry_get(psd->view_size_w_entry);
   if (w_entry) w = (Evas_Coord)atoi(w_entry);
   const char *h_entry = elm_entry_entry_get(psd->view_size_h_entry);
   if (h_entry) h = (Evas_Coord)atoi(h_entry);
   config_view_size_set(w, h);
}

void
preference_setting_reset(preference_setting_data *psd)
{
   if (!psd) return;

   elm_slider_value_set(psd->slider_view, (double) config_view_scale_get());
   elm_check_state_set(psd->toggle_console, config_console_get());
   elm_check_state_set(psd->toggle_tools, config_tools_get());
   elm_check_state_set(psd->toggle_stats, config_stats_bar_get());
   elm_check_state_set(psd->toggle_highlight, config_part_highlight_get());
   elm_check_state_set(psd->toggle_swallow, config_dummy_parts_get());
   elm_check_state_set(psd->toggle_file_browser, config_file_browser_get());
   elm_check_state_set(psd->toggle_edc_navigator, config_edc_navigator_get());

   //Reset view scale
   int view_size_w, view_size_h;
   config_view_size_get(&view_size_w, &view_size_h);
   char buf[10];
   snprintf(buf, sizeof(buf), "%d", view_size_w);
   elm_entry_entry_set(psd->view_size_w_entry, buf);
   snprintf(buf, sizeof(buf), "%d", view_size_h);
   elm_entry_entry_set(psd->view_size_h_entry, buf);
}

Evas_Object *
preference_setting_content_get(preference_setting_data *psd,
                               Evas_Object *parent)
{
   static Elm_Entry_Filter_Accept_Set digits_filter_data;
   static Elm_Entry_Filter_Limit_Size limit_filter_data;

   if (!psd) return NULL;
   if (psd->scroller) return psd->scroller;

   //Preference
   Evas_Object *scroller = elm_scroller_add(parent);

   //Box
   Evas_Object *box = elm_box_add(scroller);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);

   elm_object_content_set(scroller, box);

   //Live View Label
   Evas_Object *live_view_label = elm_label_add(box);
   elm_object_scale_set(live_view_label, 1.1);
   elm_object_text_set(live_view_label, "<b>Live View:");
   evas_object_size_hint_align_set(live_view_label, 0, 0);
   evas_object_show(live_view_label);
   elm_box_pack_end(box, live_view_label);

   Evas_Object *box2;
   Evas_Object *layout_padding3;

   //Box for View Scale
   box2  = elm_box_add(box);
   elm_box_horizontal_set(box2, EINA_TRUE);
   elm_box_padding_set(box2, 5 * elm_config_scale_get(), 0);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, 0);
   evas_object_show(box2);

   elm_box_pack_end(box, box2);

   /* This layout is intended to put the label aligned to left side
      far from 3 pixels. */
   layout_padding3 = elm_layout_add(box2);
   elm_layout_file_set(layout_padding3, EDJE_PATH, "padding3_layout");
   evas_object_show(layout_padding3);

   elm_box_pack_end(box2, layout_padding3);

   Evas_Object *label_view_scale = label_create(layout_padding3,
                                               _("View Scale"));
   elm_object_part_content_set(layout_padding3, "elm.swallow.content",
                               label_view_scale);

   //View Scale (Slider)
   Evas_Object *slider_view = elm_slider_add(box);
   evas_object_size_hint_weight_set(slider_view, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(slider_view, EVAS_HINT_FILL, 0);
   elm_slider_span_size_set(slider_view, 190);
   elm_slider_indicator_show_set(slider_view, EINA_FALSE);
   elm_slider_unit_format_set(slider_view, "%1.2fx");
   double step = 0.01 / (double) (MAX_VIEW_SCALE - MIN_VIEW_SCALE);
   elm_slider_step_set(slider_view, step);
   elm_slider_min_max_set(slider_view, MIN_VIEW_SCALE, MAX_VIEW_SCALE);
   elm_slider_value_set(slider_view, (double) config_view_scale_get());
   evas_object_show(slider_view);

   elm_box_pack_end(box2, slider_view);

   //View Size

   //Box for View Size
   box2 = elm_box_add(box);
   elm_box_horizontal_set(box2, EINA_TRUE);
   elm_box_padding_set(box2, 5 * elm_config_scale_get(), 0);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, 0);
   evas_object_show(box2);

   elm_box_pack_end(box, box2);

   //Label (View Size)

   /* This layout is intended to put the label aligned to left side
      far from 3 pixels. */
   layout_padding3 = elm_layout_add(box2);
   elm_layout_file_set(layout_padding3, EDJE_PATH, "padding3_layout");
   evas_object_show(layout_padding3);

   elm_box_pack_end(box2, layout_padding3);

   Evas_Object *label_view_size = label_create(layout_padding3,
                                               _("View Size"));
   elm_object_part_content_set(layout_padding3, "elm.swallow.content",
                               label_view_size);

   //Spacer
   Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(box2));
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box2, rect);

   Evas_Coord w, h;
   char w_str[5], h_str[5];
   config_view_size_get(&w, &h);
   snprintf(w_str, sizeof(w_str), "%d", w);
   snprintf(h_str, sizeof(h_str), "%d", h);

   //Entry (View Width)
   Evas_Object *entry_view_size_w = entry_create(box2);
   evas_object_size_hint_weight_set(entry_view_size_w, 0.15, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry_view_size_w, EVAS_HINT_FILL, EVAS_HINT_FILL);

   digits_filter_data.accepted = "0123456789";
   digits_filter_data.rejected = NULL;
   elm_entry_markup_filter_append(entry_view_size_w,
                                  elm_entry_filter_accept_set,
                                  &digits_filter_data);
   limit_filter_data.max_char_count = 4;
   limit_filter_data.max_byte_count = 0;
   elm_entry_markup_filter_append(entry_view_size_w,
                                  elm_entry_filter_limit_size,
                                  &limit_filter_data);

   elm_object_text_set(entry_view_size_w, w_str);
   elm_box_pack_end(box2, entry_view_size_w);

   //Label (X)
   Evas_Object *label_view_size_x = label_create(box2, "X");
   elm_box_pack_end(box2, label_view_size_x);

   //Entry (View Height)
   Evas_Object *entry_view_size_h = entry_create(box2);
   evas_object_size_hint_weight_set(entry_view_size_h, 0.15, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry_view_size_h, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_entry_markup_filter_append(entry_view_size_h,
                                  elm_entry_filter_accept_set,
                                  &digits_filter_data);
   elm_entry_markup_filter_append(entry_view_size_h,
                                  elm_entry_filter_limit_size,
                                  &limit_filter_data);

   elm_object_text_set(entry_view_size_h, h_str);
   elm_box_pack_end(box2, entry_view_size_h);

   //Toggle (Part Highlighting)
   Evas_Object *toggle_highlight = toggle_create(box, _("Part Highlighting"),
                                                 config_part_highlight_get());
   elm_box_pack_end(box, toggle_highlight);

   //Toggle (Dummy Swallow)
   Evas_Object *toggle_swallow = toggle_create(box, _("Dummy Parts"),
                                               config_dummy_parts_get());
   elm_box_pack_end(box, toggle_swallow);

   //Separator
   Evas_Object *separator = elm_separator_add(box);
   evas_object_size_hint_weight_set(separator, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(separator, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_separator_horizontal_set(separator, EINA_TRUE);
   evas_object_show(separator);
   elm_box_pack_end(box, separator);

   //GUIs
   Evas_Object *gui_label = elm_label_add(box);
   elm_object_scale_set(gui_label, 1.1);
   evas_object_size_hint_align_set(gui_label, 0, 0);
   elm_object_text_set(gui_label, "<b>GUI Preference:");
   evas_object_show(gui_label);
   elm_box_pack_end(box, gui_label);

   //Toggle (Status)
   Evas_Object *toggle_stats = toggle_create(box, _("Status"),
                                             config_stats_bar_get());
   elm_box_pack_end(box, toggle_stats);

   //Toggle (File Browser)
   Evas_Object *toggle_file_browser = toggle_create(box, _("File Browser"),
                                                    config_file_browser_get());
   elm_box_pack_end(box, toggle_file_browser);

   //Toggle (EDC Navigator)
   Evas_Object *toggle_edc_navigator = toggle_create(box, _("EDC Navigator"),
                                               config_edc_navigator_get());
   elm_box_pack_end(box, toggle_edc_navigator);

   //Toggle (Tools)
   Evas_Object *toggle_tools = toggle_create(box, _("Tools"),
                                             config_tools_get());
   elm_box_pack_end(box, toggle_tools);

   //Toggle (Console)
   Evas_Object *toggle_console = toggle_create(box, _("Auto Hiding Console"),
                                               config_console_get());
   evas_object_size_hint_weight_set(toggle_console, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(toggle_console, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, toggle_console);

   psd->scroller = scroller;
   psd->slider_view = slider_view;
   psd->view_size_w_entry = entry_view_size_w;
   psd->view_size_h_entry = entry_view_size_h;
   psd->toggle_highlight = toggle_highlight;
   psd->toggle_swallow = toggle_swallow;
   psd->toggle_stats = toggle_stats;
   psd->toggle_file_browser = toggle_file_browser;
   psd->toggle_edc_navigator = toggle_edc_navigator;
   psd->toggle_tools = toggle_tools;
   psd->toggle_console = toggle_console;

   return scroller;
}

preference_setting_data *
preference_setting_init(void)
{
   preference_setting_data *psd = calloc(1, sizeof(preference_setting_data));
   if (!psd)
     {
        EINA_LOG_ERR(_("Failed to allocate Memory!"));
        return NULL;
     }
   return psd;
}

void
preference_setting_term(preference_setting_data *psd)
{
   if (!psd) return;
   evas_object_del(psd->scroller);
   free(psd);
}