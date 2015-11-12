#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common.h"

typedef struct share_s
{
   Evas_Object *win;
   Evas_Object *base_layout;
   Evas_Object *tags_entry;
   Evas_Object *enventor;
   Evas_Object *error_layout;
   Evas_Object *success_layout;
   Evas_Object *link_entry;
   Evas_Object *error_entry;
   Eina_Stringshare *link;
   Ecore_Event_Handler *event_data_handler;
   Ecore_Event_Handler *event_error_handler;
   Eina_Stringshare *cmd;
} share_data;

static share_data *g_sd = NULL;
static Evas_Coord win_w = DEFAULT_SHARE_WIN_W;
static Evas_Coord win_h = DEFAULT_SHARE_WIN_H;

void
share_close(void)
{
   share_data *sd = g_sd;
   if (!sd) return;
   eina_stringshare_del(sd->cmd);
   evas_object_del(g_sd->win);
   evas_object_del(sd->error_layout);
   evas_object_del(sd->base_layout);

   free(sd);
   g_sd = NULL;

//   tools_share_update(enventor, EINA_FALSE);
}
static Eina_Bool
exe_event_data_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info);

static Eina_Bool
exe_event_error_cb(void *data, int type EINA_UNUSED, void *event_info);

static const char*
share_cmd_get(Evas_Object *enventor, Evas_Object *tags_entry)
{
   const char *edj_path = NULL;
   const Eina_List *pathes = NULL;

   pathes = enventor_object_path_get(enventor, ENVENTOR_PATH_TYPE_EDJ);
   edj_path = eina_list_data_get(pathes);

   if (!edj_path) return NULL;

   const char *tags = elm_entry_entry_get(tags_entry);

   return eina_stringshare_printf("edjshare %s %s", edj_path, tags);
}

static void
share(void)
{
   share_data *sd = g_sd;

   sd->cmd = share_cmd_get(sd->enventor, sd->tags_entry);
   printf("Command is: %s\n", sd->cmd);

   sd->event_data_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DATA,
                                                    exe_event_data_cb, sd);
   sd->event_error_handler = ecore_event_handler_add(ECORE_EXE_EVENT_ERROR,
                                                     exe_event_error_cb, sd);

   Ecore_Exe_Flags flags = (ECORE_EXE_PIPE_READ_LINE_BUFFERED |
                            ECORE_EXE_PIPE_READ |
                            ECORE_EXE_PIPE_ERROR_LINE_BUFFERED |
                            ECORE_EXE_PIPE_ERROR);
   ecore_exe_pipe_run(sd->cmd, flags, NULL);
}

static void
share_btn_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   share();
}

static void
win_delete_request_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   share_close();
}

static void
cancel_btn_cb(void *data EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   share_close();
}

static void
link_copy_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   return;
}

static void
link_open_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ei EINA_UNUSED)
{
   return;
}

static Eina_Bool
exe_event_data_cb(void *data EINA_UNUSED,
                  int type EINA_UNUSED,
                  void *event_info)
{
   Ecore_Exe_Event_Data *ev = event_info;
   Ecore_Exe_Event_Data_Line *el = NULL;
   int i = 0;

   share_data *sd = (share_data *)data;
   evas_object_hide(sd->base_layout);
   evas_object_hide(sd->error_layout);

   if (!sd->success_layout)
   {
      sd->success_layout = elm_layout_add(sd->win);
      elm_layout_file_set(sd->success_layout, EDJE_PATH, "share_success_layout");
      evas_object_size_hint_weight_set(sd->success_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_show(sd->success_layout);
      elm_win_resize_object_add(sd->win, sd->success_layout);

      sd->link_entry = elm_entry_add(sd->success_layout);
      elm_entry_scrollable_set(sd->link_entry, EINA_TRUE);
      elm_entry_editable_set(sd->link_entry, EINA_FALSE);
      elm_entry_single_line_set(sd->link_entry, EINA_FALSE);
      evas_object_size_hint_weight_set(sd->link_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set (sd->link_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
      evas_object_show(sd->link_entry);
      elm_object_part_content_set(sd->success_layout, "elm.swallow.entry", sd->link_entry);

      Evas_Object *button = elm_button_add(sd->success_layout);
      elm_object_text_set(button, "Close");
      elm_object_part_content_set(sd->success_layout, "elm.swallow.close", button);
      evas_object_smart_callback_add(button, "clicked", cancel_btn_cb, sd);
      evas_object_show(button);

      Evas_Object *icon = elm_icon_add(sd->success_layout);
      elm_image_file_set(icon, EDJE_PATH, "save");
      evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      button = elm_button_add(sd->success_layout);
      elm_object_part_content_set(button, "icon", icon);
      evas_object_smart_callback_add(button, "clicked", link_copy_cb, sd);
      evas_object_show(button);
      elm_object_part_content_set(sd->success_layout, "elm.swallow.copy", button);
      elm_object_disabled_set(button, EINA_TRUE);

      icon = elm_icon_add(sd->success_layout);
      elm_image_file_set(icon, EDJE_PATH, "goto");
      evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
      button = elm_button_add(sd->success_layout);
      elm_object_part_content_set(button, "icon", icon);
      evas_object_smart_callback_add(button, "clicked", link_open_cb, sd);
      evas_object_show(button);
      elm_object_part_content_set(sd->success_layout, "elm.swallow.open", button);
      elm_object_disabled_set(button, EINA_TRUE);

   }
   evas_object_show(sd->success_layout);
   elm_entry_entry_insert(sd->link_entry, "");

   fprintf(stdout, "Data:\n");
   for (el = ev->lines; el && el->line; el++, i++)
     {
        elm_entry_entry_append(sd->link_entry, el->line);
        elm_entry_entry_append(sd->link_entry, "\n");
        fprintf(stdout, "\t%s\n", el->line);
     }
   return EINA_TRUE;
}

static Eina_Bool
exe_event_error_cb(void *data, int type EINA_UNUSED, void *event_info)
{
   share_data *sd = (share_data *) data;
   Ecore_Exe_Event_Data *ev = event_info;
   Ecore_Exe_Event_Data_Line *el = NULL;
   int i = 0;

   evas_object_hide(sd->base_layout);
   evas_object_hide(sd->success_layout);

   if (!sd->error_layout)
   {
      sd->error_layout = elm_layout_add(sd->win);
      evas_object_hide(sd->base_layout);
      elm_layout_file_set(sd->error_layout, EDJE_PATH, "share_error_layout");
      evas_object_size_hint_weight_set(sd->error_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_show(sd->error_layout);
      elm_win_resize_object_add(sd->win, sd->error_layout);

      sd->error_entry = elm_entry_add(sd->error_layout);
      elm_entry_single_line_set(sd->error_entry, EINA_FALSE);
      elm_entry_scrollable_set(sd->error_entry, EINA_TRUE);
      elm_entry_editable_set(sd->error_entry, EINA_FALSE);
      evas_object_size_hint_weight_set(sd->error_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
      evas_object_size_hint_align_set (sd->error_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
      evas_object_show(sd->error_entry);
      elm_object_part_content_set(sd->error_layout, "elm.swallow.entry", sd->error_entry);

      Evas_Object *button = elm_button_add(sd->error_layout);
      elm_object_text_set(button, "Cancel");
      elm_object_part_content_set(sd->error_layout, "elm.swallow.cancel", button);
      evas_object_smart_callback_add(button, "clicked", cancel_btn_cb, sd);
      evas_object_show(button);

      button = elm_button_add(sd->error_layout);
      elm_object_text_set(button, "Try again");
      elm_object_part_content_set(sd->error_layout, "elm.swallow.share", button);
      evas_object_smart_callback_add(button, "clicked", share_btn_cb, sd);
      evas_object_show(button);
   }

   evas_object_show(sd->error_layout);
   elm_entry_entry_set(sd->error_entry, "");

   fprintf(stdout, "Error:\n");
   for (el = ev->lines; el && el->line; el++, i++)
   {
      if (strcmp(el->line, "FAIL"))
        {
          elm_entry_entry_append(sd->error_entry, el->line);
          elm_entry_entry_append(sd->error_entry, "</br>");
        }
       fprintf(stdout, "\t%s\n", el->line);
   }
   return EINA_TRUE;
}

void
share_open(Evas_Object *enventor)
{
   share_data *sd = g_sd;

   if (sd)
     {
        share_close();
        return;
     }

   search_close();
   goto_close();

   sd = calloc(1, sizeof(share_data));
   if (!sd)
     {
        EINA_LOG_ERR("Failed to allocate Memory!");
        return;
     }
   g_sd = sd;

   Evas_Object *win = elm_win_add(base_win_get(), "Share edj file",
                                  ELM_WIN_DIALOG_BASIC);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_title_set(win, "Share edj file");
   evas_object_smart_callback_add(win, "delete,request", win_delete_request_cb, sd);

   Evas_Object *bg = elm_bg_add(win);
   evas_object_show(bg);
   elm_win_resize_object_add(win, bg);

   Evas_Object *base_layout = elm_layout_add(win);
   elm_layout_file_set(base_layout, EDJE_PATH, "share_base_layout");
   evas_object_size_hint_weight_set(base_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(base_layout);
   elm_win_resize_object_add(win, base_layout);

   Evas_Object *tags_entry = elm_entry_add(base_layout);
   elm_entry_single_line_set(tags_entry, EINA_FALSE);
   elm_entry_scrollable_set(tags_entry, EINA_TRUE);
   evas_object_size_hint_weight_set(tags_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tags_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_content_set(base_layout, "elm.swallow.tags", tags_entry);
   evas_object_show(tags_entry);
   elm_object_focus_set(tags_entry, EINA_TRUE);
   elm_entry_entry_insert(tags_entry, "enventor");

   Evas_Object *button_share = elm_button_add(base_layout);
   elm_object_text_set(button_share, "Share");
   evas_object_smart_callback_add(button_share, "clicked", share_btn_cb, sd);
   evas_object_show(button_share);
   elm_object_part_content_set(base_layout, "elm.swallow.share", button_share);

   Evas_Object *button_cancel = elm_button_add(base_layout);
   elm_object_text_set(button_cancel, "Cancel");
   evas_object_smart_callback_add(button_cancel, "clicked", cancel_btn_cb, sd);
   evas_object_show(button_cancel);
   elm_object_part_content_set(base_layout, "elm.swallow.cancel", button_cancel);

   win_w = (Evas_Coord) ((double) win_w * elm_config_scale_get());
   win_h = (Evas_Coord) ((double) win_h * elm_config_scale_get());
   evas_object_resize(win, win_w, win_h);
   evas_object_show(win);

   sd->win = win;
   sd->base_layout = base_layout;
   sd->tags_entry = tags_entry;
   sd->enventor = enventor;
}

