#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore_Getopt.h>
#include <Eio.h>
#include "common.h"

typedef struct app_s
{
   Evas_Object *enventor;
   Eina_Bool on_saving : 1;
   Eina_Bool lazy_save : 1;
} app_data;

int main(int argc, char **argv);

void
auto_comp_toggle(app_data *ad)
{
   Eina_Bool toggle = !config_auto_complete_get();
   enventor_object_auto_complete_set(ad->enventor, toggle);
   if (toggle) stats_info_msg_update("Auto Completion Enabled.");
   else stats_info_msg_update("Auto Completion Disabled.");
   config_auto_complete_set(toggle);
}

static void
auto_indent_toggle(app_data *ad)
{
   Eina_Bool toggle = !config_auto_indent_get();
   enventor_object_auto_indent_set(ad->enventor, toggle);
   if (toggle) stats_info_msg_update("Auto Indentation Enabled.");
   else stats_info_msg_update("Auto Indentation Disabled.");
   config_auto_indent_set(toggle);
}

static void
enventor_tools_update(Evas_Object *enventor)
{
   tools_lines_update(enventor, EINA_FALSE);
   tools_highlight_update(enventor, EINA_FALSE);
   tools_swallow_update(enventor, EINA_FALSE);
   tools_status_update(enventor, EINA_FALSE);
}

static void
enventor_common_setup(Evas_Object *enventor)
{
   const char *font_name;
   const char *font_style;
   config_font_get(&font_name, &font_style);
   enventor_object_font_set(enventor, font_name, font_style);
   enventor_object_font_scale_set(enventor, config_font_scale_get());
   enventor_object_live_view_scale_set(enventor, config_view_scale_get());
   enventor_object_auto_indent_set(enventor, config_auto_indent_get());
   enventor_object_auto_complete_set(enventor, config_auto_complete_get());

   Eina_List *list = eina_list_append(NULL, config_output_path_get());
   enventor_object_path_set(enventor, ENVENTOR_PATH_TYPE_EDJ, list);
   eina_list_free(list);

   enventor_object_path_set(enventor, ENVENTOR_PATH_TYPE_IMAGE,
                            config_img_path_list_get());
   enventor_object_path_set(enventor, ENVENTOR_PATH_TYPE_SOUND,
                            config_snd_path_list_get());
   enventor_object_path_set(enventor, ENVENTOR_PATH_TYPE_FONT,
                            config_fnt_path_list_get());
   enventor_object_path_set(enventor, ENVENTOR_PATH_TYPE_DATA,
                            config_dat_path_list_get());
}

static void
syntax_color_update(Evas_Object *enventor)
{
   const char *config_color;
   const char *enventor_color;
   Eina_Bool color_changed = EINA_FALSE;
   Enventor_Syntax_Color_Type color_type;

   color_type = ENVENTOR_SYNTAX_COLOR_STRING;
   for (; color_type < ENVENTOR_SYNTAX_COLOR_LAST; color_type++)
     {
        config_color = config_syntax_color_get(color_type);
        if (config_color)
          {
             enventor_color = enventor_object_syntax_color_get(enventor,
                                                               color_type);
             if (strcmp(config_color, enventor_color))
               {
                  enventor_object_syntax_color_set(enventor, color_type,
                                                   config_color);
                  color_changed = EINA_TRUE;
               }
          }
     }

   if (color_changed)
     enventor_object_syntax_color_full_apply(enventor, EINA_TRUE);
}

static void
syntax_color_init(Evas_Object *enventor)
{
   const char *config_color;
   const char *enventor_color;
   Enventor_Syntax_Color_Type color_type;

   color_type = ENVENTOR_SYNTAX_COLOR_STRING;
   for (; color_type < ENVENTOR_SYNTAX_COLOR_LAST; color_type++)
     {
        config_color = config_syntax_color_get(color_type);
        if (!config_color)
          {
             enventor_color = enventor_object_syntax_color_get(enventor,
                                                               color_type);
             config_syntax_color_set(color_type, enventor_color);
          }
     }
}

static void
config_update_cb(void *data)
{
   app_data *ad = data;
   Evas_Object *enventor = ad->enventor;

   enventor_common_setup(enventor);
   enventor_tools_update(enventor);

   syntax_color_update(enventor);

   //Live View Size
   Evas_Coord w, h;
   if (config_view_size_configurable_get())
     config_view_size_get(&w, &h);
   else
     {
        w = 0;
        h = 0;
     }
   enventor_object_live_view_size_set(enventor, w, h);

   base_tools_toggle(EINA_FALSE);
   base_console_auto_hide();
}

static Eina_Bool
main_mouse_wheel_cb(void *data, int type EINA_UNUSED, void *ev)
{
   Ecore_Event_Mouse_Wheel *event = ev;
   app_data *ad = data;
   Evas_Coord x, y, w, h;

   if (!EVENT_KEY_MODIFIER_CHECK(CTRL, event->modifiers))
      return ECORE_CALLBACK_PASS_ON;

   //View Scale
   Evas_Object *view = enventor_object_live_view_get(ad->enventor);
   evas_object_geometry_get(view, &x, &y, &w, &h);

   if ((event->x >= x) && (event->x <= (x + w)) &&
       (event->y >= y) && (event->y <= (y + h)))
     {
        double scale = config_view_scale_get();

        if (event->z < 0) scale += 0.1;
        else scale -= 0.1;

        config_view_scale_set(scale);
        scale = config_view_scale_get();
        enventor_object_live_view_scale_set(ad->enventor, scale);

        //Just in live edit mode case.
        live_edit_update();

        char buf[256];
        snprintf(buf, sizeof(buf), "Live View Scale: %2.2fx", scale);
        stats_info_msg_update(buf);

        return ECORE_CALLBACK_PASS_ON;
     }

   //Font Size
   evas_object_geometry_get(ad->enventor, &x, &y, &w, &h);

   if ((event->x >= x) && (event->x <= (x + w)) &&
       (event->y >= y) && (event->y <= (y + h)))
     {
        if (event->z < 0)
          {
             config_font_scale_set(config_font_scale_get() + 0.1f);
             enventor_object_font_scale_set(ad->enventor,
                                            config_font_scale_get());
          }
        else
          {
             config_font_scale_set(config_font_scale_get() - 0.1f);
             enventor_object_font_scale_set(ad->enventor,
                                            config_font_scale_get());
          }

        char buf[128];
        snprintf(buf, sizeof(buf), "Font Size: %1.1fx",
                 config_font_scale_get());
        stats_info_msg_update(buf);

        return ECORE_CALLBACK_PASS_ON;
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Evas_Object *
tools_set(Evas_Object *enventor)
{
   Evas_Object *tools = tools_init(base_layout_get(), enventor);
   base_tools_set(tools);
   enventor_tools_update(enventor);

   return tools;
}

static void
args_dispatch(int argc, char **argv, char *edc_path, char *edj_path,
              Eina_List **img_path, Eina_List **snd_path,
              Eina_List **fnt_path, Eina_List **dat_path,
              Eina_Bool *default_edc, Eina_Bool *template)
{

   Eina_List *id = NULL;
   Eina_List *fd = NULL;
   Eina_List *sd = NULL;
   Eina_List *dd = NULL;

   Eina_Bool quit = EINA_FALSE;
   Eina_Bool help = EINA_FALSE;

   //No arguments. set defaults
   if (argc == 1) goto defaults;

   static const Ecore_Getopt optdesc = {
     PACKAGE_NAME,
     ENVENTOR_USAGE,
     VERSION,
     ENVENTOR_COPYRIGHT,
     ENVENTOR_LICENSE,
     ENVENTOR_INFO,
     EINA_TRUE,
       {
          ECORE_GETOPT_STORE_TRUE('t', "to", "Open template menu"),
          ECORE_GETOPT_APPEND_METAVAR('i', "id", "Images path",
                                      "path", ECORE_GETOPT_TYPE_STR),
          ECORE_GETOPT_APPEND_METAVAR('s', "sd", "Sounds path",
                                       "path", ECORE_GETOPT_TYPE_STR),
          ECORE_GETOPT_APPEND_METAVAR('f', "fd", "Fonts path",
                                      "path", ECORE_GETOPT_TYPE_STR),
          ECORE_GETOPT_APPEND_METAVAR('d', "dd", "Data path",
                                      "path", ECORE_GETOPT_TYPE_STR),
          ECORE_GETOPT_VERSION('v', "version"),
          ECORE_GETOPT_COPYRIGHT('c', "copyright"),
          ECORE_GETOPT_LICENSE('l', "license"),
          ECORE_GETOPT_HELP('h', "help"),
          ECORE_GETOPT_SENTINEL
       }
   };

   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_BOOL(*template),
      ECORE_GETOPT_VALUE_LIST(id),
      ECORE_GETOPT_VALUE_LIST(sd),
      ECORE_GETOPT_VALUE_LIST(fd),
      ECORE_GETOPT_VALUE_LIST(dd),
      ECORE_GETOPT_VALUE_BOOL(quit),
      ECORE_GETOPT_VALUE_BOOL(quit),
      ECORE_GETOPT_VALUE_BOOL(quit),
      ECORE_GETOPT_VALUE_BOOL(help),
      ECORE_GETOPT_VALUE_NONE
   };

   //edc path
   int i = 0;
   for (; i < argc; i++)
     {
        if (strstr(argv[i], ".edc"))
          {
             sprintf(edc_path, "%s", argv[i]);
             *default_edc = EINA_FALSE;
          }
        else if (strstr(argv[i], ".edj"))
          {
             sprintf(edj_path, "%s", argv[i]);
          }
     }

   if ((ecore_getopt_parse(&optdesc, values, argc, argv) < 0) || quit)
        exit(0);
   if (help)
     {
        fprintf(stderr, ENVENTOR_HELP_EXAMPLES);
        exit(0);
     }

defaults:
   if (*default_edc)
     {
        Eina_Tmpstr *tmp_path;
        eina_file_mkstemp(DEFAULT_EDC_FORMAT, &tmp_path);
        sprintf(edc_path, "%s", (const char *)tmp_path);
        eina_tmpstr_del(tmp_path);
     }

     char *s = NULL;
     EINA_LIST_FREE(id, s)
       {
          *img_path = eina_list_append(*img_path, eina_stringshare_add(s));
          free(s);
       }
     id = NULL;
     EINA_LIST_FREE(sd, s)
       {
          *snd_path = eina_list_append(*snd_path, eina_stringshare_add(s));
          free(s);
       }
     sd = NULL;
     EINA_LIST_FREE(fd, s)
       {
          *fnt_path = eina_list_append(*fnt_path, eina_stringshare_add(s));
          free(s);
       }
     fd = NULL;
     EINA_LIST_FREE(dd, s)
       {
          *dat_path = eina_list_append(*dat_path, eina_stringshare_add(s));
          free(s);
       }

   ecore_getopt_list_free(id);
   ecore_getopt_list_free(fd);
   ecore_getopt_list_free(sd);
   ecore_getopt_list_free(dd);
}

static void
config_data_set(app_data *ad, int argc, char **argv, Eina_Bool *default_edc,
                Eina_Bool *template)
{
   char edc_path[PATH_MAX] = { 0, };
   char edj_path[PATH_MAX] = { 0, };
   Eina_List *img_path = NULL;
   Eina_List *snd_path = NULL;
   Eina_List *fnt_path = NULL;
   Eina_List *dat_path = NULL;

   args_dispatch(argc, argv, edc_path, edj_path, &img_path, &snd_path,
                 &fnt_path, &dat_path, default_edc, template);
   config_init(edc_path, edj_path, img_path, snd_path, fnt_path, dat_path);
   config_update_cb_set(config_update_cb, ad);
}

static void
elm_setup()
{
   elm_need_efreet();
   elm_config_profile_set("standard");

   /* Recover the scale since it will be reset by
      elm_config_profile_set() */
   char *scale = getenv("ELM_SCALE");
   if (scale) elm_config_scale_set(atof(scale));

   /* Recover the current engine since it will be reset by
      elm_config_profiel_set() */
   char *engine = getenv("ELM_ENGINE");
   if (engine && !strncmp(engine, "gl", strlen("gl")))
     elm_config_accel_preference_set("3d");

   elm_config_focus_highlight_clip_disabled_set(EINA_FALSE);
   elm_config_scroll_bounce_enabled_set(EINA_FALSE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_compile_bin_dir_set(PACKAGE_BIN_DIR);
   elm_app_compile_data_dir_set(PACKAGE_DATA_DIR);
   elm_app_compile_lib_dir_set(PACKAGE_LIB_DIR);
   elm_app_info_set(main, "enventor", "images/logo.png");

   snprintf(EDJE_PATH, sizeof(EDJE_PATH), "%s/themes/enventor.edj",
            elm_app_data_dir_get());
   elm_theme_extension_add(NULL, EDJE_PATH);
}

static void
enventor_cursor_line_changed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                                void *event_info)
{
   Enventor_Cursor_Line *cur_line = (Enventor_Cursor_Line *)event_info;
   stats_line_num_update(cur_line->line, enventor_object_max_line_get(obj));
}

static void
enventor_cursor_group_changed_cb(void *data EINA_UNUSED,
                                 Evas_Object *obj EINA_UNUSED,
                                 void *event_info)
{
   const char *group_name = event_info;
   stats_edc_group_update(group_name);
}

static void
enventor_compile_error_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                          void *event_info)
{
   const char *msg = event_info;
   base_error_msg_set(msg);
}

static void
enventor_live_view_resized_cb(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info)
{
   if (!config_stats_bar_get()) return;
   Enventor_Live_View_Size *size = event_info;
   stats_view_size_update(size->w, size->h);

   if (!config_view_size_configurable_get())
     config_view_size_set(size->w, size->h);
}

static void
enventor_live_view_loaded_cb(void *data EINA_UNUSED, Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   if (!config_view_size_configurable_get()) return;

   Evas_Coord w, h;
   config_view_size_get(&w, &h);
   enventor_object_live_view_size_set(obj, w, h);
}

static void
enventor_live_view_cursor_moved_cb(void *data EINA_UNUSED,
                                   Evas_Object *obj EINA_UNUSED,
                                   void *event_info)
{
   if (!config_stats_bar_get()) return;
   Enventor_Live_View_Cursor *cursor = event_info;
   stats_cursor_pos_update(cursor->x, cursor->y, cursor->relx, cursor->rely);
}

static void
enventor_program_run_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                        void *event_info)
{
   if (!config_stats_bar_get()) return;
   const char *program = event_info;
   char buf[256];
   snprintf(buf, sizeof(buf), "Program Run: \"%s\"", program);
   stats_info_msg_update(buf);
}

static void
enventor_ctxpopup_changed_cb(void *data, Evas_Object *obj,
                             void *event_info EINA_UNUSED)
{
   app_data *ad = data;
   if (!enventor_object_modified_get(obj)) return;
   if (ad->on_saving)
     {
        ad->lazy_save = EINA_TRUE;
        return;
     }
   ad->on_saving = EINA_TRUE;
   enventor_object_save(obj, config_input_path_get());
}

static void
enventor_live_view_updated_cb(void *data, Evas_Object *obj,
                              void *event_info EINA_UNUSED)
{
   app_data *ad = data;

   if (ad->lazy_save && enventor_object_modified_get(obj))
     {
        enventor_object_save(obj, config_input_path_get());
        ad->lazy_save = EINA_FALSE;
     }
   else
     {
        ad->lazy_save = EINA_FALSE;
        ad->on_saving = EINA_FALSE;
     }
}

static void
enventor_ctxpopup_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                               void *event_info EINA_UNUSED)
{
   if (menu_activated_get() > 0)
     enventor_object_focus_set(obj, EINA_FALSE);
}

static void
enventor_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   if (file_mgr_edc_modified_get()) file_mgr_warning_open();
}

static void
enventor_setup(app_data *ad)
{
   Evas_Object *enventor = enventor_object_add(base_layout_get());
   evas_object_smart_callback_add(enventor, "cursor,line,changed",
                                  enventor_cursor_line_changed_cb, ad);
   evas_object_smart_callback_add(enventor, "cursor,group,changed",
                                  enventor_cursor_group_changed_cb, ad);
   evas_object_smart_callback_add(enventor, "compile,error",
                                  enventor_compile_error_cb, ad);
   evas_object_smart_callback_add(enventor, "live_view,loaded",
                                  enventor_live_view_loaded_cb, ad);
   evas_object_smart_callback_add(enventor, "live_view,cursor,moved",
                                  enventor_live_view_cursor_moved_cb, ad);
   evas_object_smart_callback_add(enventor, "live_view,resized",
                                  enventor_live_view_resized_cb, ad);
   evas_object_smart_callback_add(enventor, "live_view,updated",
                                  enventor_live_view_updated_cb, ad);
   evas_object_smart_callback_add(enventor, "program,run",
                                  enventor_program_run_cb, ad);
   evas_object_smart_callback_add(enventor, "ctxpopup,changed",
                                  enventor_ctxpopup_changed_cb, ad);
   evas_object_smart_callback_add(enventor, "ctxpopup,dismissed",
                                  enventor_ctxpopup_dismissed_cb, ad);
   evas_object_smart_callback_add(enventor, "focused",
                                  enventor_focused_cb, ad);

   evas_object_size_hint_expand_set(enventor, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(enventor, EVAS_HINT_FILL, EVAS_HINT_FILL);

   enventor_common_setup(enventor);

   enventor_object_file_set(enventor, config_input_path_get());

   base_enventor_set(enventor);
   base_title_set(config_input_path_get());
   base_live_view_set(enventor_object_live_view_get(enventor));

   ad->enventor = enventor;
}

static void
default_template_insert(app_data *ad)
{
   if (live_edit_get())
     {
        stats_info_msg_update("Insertion of template code is disabled "
                              "while in Live Edit mode");
        return;
     }

   char syntax[12];
   if (enventor_object_template_insert(ad->enventor, ENVENTOR_TEMPLATE_INSERT_DEFAULT, syntax, sizeof(syntax)))
     {
        char msg[64];
        snprintf(msg, sizeof(msg), "Template code inserted, (%s)", syntax);
        stats_info_msg_update(msg);
        enventor_object_save(ad->enventor, config_input_path_get());
     }
   else
     {
        stats_info_msg_update("Can't insert template code here. Move the "
                              "cursor inside the \"Collections,Images,Parts,"
                              "Part,Programs\" scope.");
     }
}

static Eina_Bool
alt_func(Ecore_Event_Key *event)
{
   if (!EVENT_KEY_MODIFIER_CHECK(ALT, event->modifiers))
       return EINA_FALSE;

   //Full Edit View
   if (!strcmp(event->key, "Left"))
     {
        base_live_view_full_view();
        return EINA_TRUE;
     }
   //Full Live View
   if (!strcmp(event->key, "Right"))
     {
        base_enventor_full_view();
        return EINA_TRUE;
     }
   //Full Console View
   if (!strcmp(event->key, "Up"))
     {
        base_console_full_view();
        return EINA_TRUE;
     }
   //Full Editors View
   if (!strcmp(event->key, "Down"))
     {
        base_editors_full_view();
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
ctrl_func(app_data *ad, Ecore_Event_Key *event)
{
   if (!EVENT_KEY_MODIFIER_CHECK(CTRL, event->modifiers))
         return EINA_FALSE;
   //Save
   if (!strcmp(event->key, "s") || !strcmp(event->key, "S"))
     {
        file_mgr_edc_save();
        return EINA_TRUE;
     }
  //Delete Line
   if (!strcmp(event->key, "d") || !strcmp(event->key, "D"))
     {
        enventor_object_line_delete(ad->enventor);
        return EINA_TRUE;
     }
   //Find/Replace
   if (!strcmp(event->key, "f") || !strcmp(event->key, "F"))
     {
        live_edit_cancel();
        search_open(ad->enventor);
        return EINA_TRUE;
     }
   //Goto Line
   if (!strcmp(event->key, "l") || !strcmp(event->key, "L"))
     {
        live_edit_cancel();
        goto_open(ad->enventor);
        return EINA_TRUE;
     }
   //Part Highlight
   if (!strcmp(event->key, "h") || !strcmp(event->key, "H"))
     {
        tools_highlight_update(ad->enventor, EINA_TRUE);
        return EINA_TRUE;
     }
   //Swallow Dummy Object
   if (!strcmp(event->key, "w") || !strcmp(event->key, "W"))
     {
        tools_swallow_update(ad->enventor, EINA_TRUE);
        return EINA_TRUE;
     }
   //Template Code
   if (!strcmp(event->key, "t") || !strcmp(event->key, "T"))
     {
        default_template_insert(ad);
        return EINA_TRUE;
     }
   //Auto Indentation
   if (!strcmp(event->key, "i") || !strcmp(event->key, "I"))
     {
        auto_indent_toggle(ad);
        return EINA_TRUE;
     }
   //Auto Completion
   if (!strcmp(event->key, "o") || !strcmp(event->key, "O"))
     {
        auto_comp_toggle(ad);
        return EINA_TRUE;
     }

   if (!strcmp(event->key, "space"))
     {
        enventor_object_auto_complete_list_show(ad->enventor);
        return EINA_TRUE;
     }
   //Live Edit
   if (!strcmp(event->key, "e") || !strcmp(event->key, "E"))
     {
        live_edit_toggle();
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
main_key_down_cb(void *data, int type EINA_UNUSED, void *ev)
{
   Ecore_Event_Key *event = ev;
   app_data *ad = data;

   //Main Menu
   if (!strcmp(event->key, "Escape"))
     {
        if (goto_is_opened())
          {
             goto_close();
             enventor_object_focus_set(ad->enventor, EINA_TRUE);
             return ECORE_CALLBACK_DONE;
          }
        if (search_is_opened())
          {
             search_close();
             enventor_object_focus_set(ad->enventor, EINA_TRUE);
             return ECORE_CALLBACK_DONE;
          }
        if (live_edit_get())
          {
             live_edit_cancel();
             enventor_object_focus_set(ad->enventor, EINA_TRUE);
             return ECORE_CALLBACK_DONE;
          }
        if (file_mgr_warning_is_opened())
          {
             file_mgr_warning_close();
             return ECORE_CALLBACK_DONE;
          }
        if (enventor_object_ctxpopup_visible_get(ad->enventor))
          {
             enventor_object_ctxpopup_dismiss(ad->enventor);
             return ECORE_CALLBACK_DONE;
          }

        menu_toggle();
        return ECORE_CALLBACK_DONE;
     }

   if (menu_activated_get() > 0) return ECORE_CALLBACK_PASS_ON;
   if (file_mgr_warning_is_opened()) return ECORE_CALLBACK_PASS_ON;

   if (ctrl_func(ad, event)) return ECORE_CALLBACK_DONE;
   if (alt_func(event)) return ECORE_CALLBACK_DONE;

   //Control Key
   if (!strcmp("Control_L", event->key))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        return ECORE_CALLBACK_PASS_ON;
     }

   //Alt Key
   if (!strcmp("Alt_L", event->key))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        return ECORE_CALLBACK_PASS_ON;
     }

   //README
   if (!strcmp(event->key, "F1"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        live_edit_cancel();
        menu_about();
        return ECORE_CALLBACK_DONE;
     }
   //New
   if (!strcmp(event->key, "F2"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        live_edit_cancel();
        menu_edc_new(EINA_FALSE);
        return ECORE_CALLBACK_DONE;
     }
   //Save
   if (!strcmp(event->key, "F3"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        live_edit_cancel();
        menu_edc_save();
        return ECORE_CALLBACK_DONE;
     }
   //Load
   if (!strcmp(event->key, "F4"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        live_edit_cancel();
        menu_edc_load();
        return ECORE_CALLBACK_DONE;
     }
   //Line Number
   if (!strcmp(event->key, "F5"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        tools_lines_update(ad->enventor, EINA_TRUE);
        return ECORE_CALLBACK_DONE;
     }
   //Tools
   if (!strcmp(event->key, "F9"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        base_tools_toggle(EINA_TRUE);
        return ECORE_CALLBACK_DONE;
     }
   //Console
   if (!strcmp(event->key, "F10"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        base_console_toggle();
        return ECORE_CALLBACK_DONE;
     }
   //Statusbar
   if (!strcmp(event->key, "F11"))
     {
        enventor_object_ctxpopup_dismiss(ad->enventor);
        tools_status_update(NULL, EINA_TRUE);
        return ECORE_CALLBACK_DONE;
     }
   //Setting
   if (!strcmp(event->key, "F12"))
     {
        live_edit_cancel();
        enventor_object_ctxpopup_dismiss(ad->enventor);
        menu_setting();
        return ECORE_CALLBACK_DONE;
     }

   return ECORE_CALLBACK_PASS_ON;
}

static void
statusbar_set()
{
   Evas_Object *obj = stats_init(base_layout_get());
   elm_object_part_content_set(base_layout_get(), "elm.swallow.statusbar", obj);
   tools_status_update(NULL, EINA_FALSE);
}

static void
live_edit_set(Evas_Object *enventor, Evas_Object *tools)
{
   Evas_Object *trigger = tools_live_edit_get(tools);
   live_edit_init(enventor, trigger);
}

static Eina_Bool
init(app_data *ad, int argc, char **argv)
{
   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, main_key_down_cb, ad);
   ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL, main_mouse_wheel_cb, ad);

   elm_setup();

   enventor_init(argc, argv);

   Eina_Bool template = EINA_FALSE;
   Eina_Bool default_edc = EINA_TRUE;
   config_data_set(ad, argc, argv, &default_edc, &template);
   newfile_default_set(default_edc);
   base_gui_init();
   statusbar_set();
   enventor_setup(ad);
   file_mgr_init(ad->enventor);
   Evas_Object *tools = tools_set(ad->enventor);
   live_edit_set(ad->enventor, tools);

   base_gui_show();

   //Guarantee Enventor editor has focus.
   enventor_object_focus_set(ad->enventor, EINA_TRUE);

   menu_init(ad->enventor);

   if (template) menu_edc_new(EINA_TRUE);

   //Initialize syntax color.
   syntax_color_init(ad->enventor);
   syntax_color_update(ad->enventor);

   return EINA_TRUE;
}

static void
term(app_data *ad EINA_UNUSED)
{
   menu_term();
   live_edit_term();
   stats_term();
   tools_term();
   base_gui_term();
   file_mgr_term();
   config_term();
   enventor_shutdown();
}

EAPI_MAIN
int elm_main(int argc, char **argv)
{
   app_data ad;
   memset(&ad, 0x00, sizeof(ad));

   if (!init(&ad, argc, argv))
     {
        term(&ad);
        return 0;
     }

   elm_run();

   term(&ad);

   elm_shutdown();

   return 0;
}
ELM_MAIN()
