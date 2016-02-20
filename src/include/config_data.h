#define MAX_FONT_SCALE 5.0
#define MIN_FONT_SCALE 0.5
#define MAX_VIEW_SCALE 5.0
#define MIN_VIEW_SCALE 0.1

void config_init(const char *input_path, const char *output_path, Eina_List *img_path, Eina_List *snd_path, Eina_List *fnt_path, Eina_List *dat_path);
void config_term(void);
const char *config_input_path_get(void);
const char *config_output_path_get(void);
const char *config_img_path_get(void);
const char *config_snd_path_get(void);
const char *config_fnt_path_get(void);
const char *config_dat_path_get(void);
void config_img_path_set(const char *img_path);
void config_snd_path_set(const char *snd_path);
void config_fnt_path_set(const char *fnt_path);
void config_dat_path_set(const char *fnt_path);
Eina_List *config_img_path_list_get(void);
Eina_List *config_snd_path_list_get(void);
Eina_List *config_fnt_path_list_get(void);
Eina_List *config_dat_path_list_get(void);
void config_syntax_color_set(Enventor_Syntax_Color_Type color_type, const char *val);
const char *config_syntax_color_get(Enventor_Syntax_Color_Type color_type);
void config_update_cb_set(void (*cb)(void *data), void *data);
void config_stats_bar_set(Eina_Bool enabled);
void config_linenumber_set(Eina_Bool enabled);
Eina_Bool config_stats_bar_get(void);
Eina_Bool config_linenumber_get(void);
void config_apply(void);
void config_input_path_set(const char *input_path);
void config_view_size_get(Evas_Coord *w, Evas_Coord *h);
void config_view_size_set(Evas_Coord w, Evas_Coord h);
Eina_Bool config_part_highlight_get(void);
void config_part_highlight_set(Eina_Bool highlight);
Eina_Bool config_dummy_parts_get(void);
void config_dummy_parts_set(Eina_Bool dummy_parts);
void config_auto_indent_set(Eina_Bool auto_indent);
Eina_Bool config_auto_indent_get(void);
void config_auto_complete_set(Eina_Bool auto_complete);
Eina_Bool config_auto_complete_get(void);
void config_font_set(const char *font_name, const char *font_style);
void config_font_get(const char **font_name, const char **font_style);
void config_font_scale_set(float font_scale);
float config_font_scale_get(void);
void config_view_scale_set(double view_scale);
double config_view_scale_get(void);
Eina_Bool config_tools_get(void);
void config_tools_set(Eina_Bool enabled);
Eina_Bool config_config_get(void);
void config_config_set(Eina_Bool enabled);
double config_console_size_get(void);
void config_console_size_set(double size);
Eina_Bool config_console_get(void);
void config_editor_size_set(double size);
double config_editor_size_get(void);
void config_console_set(Eina_Bool enabled);
void config_win_size_get(Evas_Coord *w, Evas_Coord *h);
void config_win_size_set(Evas_Coord w, Evas_Coord h);
void config_smart_undo_redo_set(Eina_Bool smart_undo_redo);
Eina_Bool config_smart_undo_redo_get(void);
void config_edc_navigator_set(Eina_Bool enabled);
Eina_Bool config_edc_navigator_get(void);
void config_monospace_font_set(Eina_Bool monospace_font);
Eina_Bool config_monospace_font_get(void);
