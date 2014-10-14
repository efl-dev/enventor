#ifndef __ENVENTOR_PRIVATE_H__
#define __ENVENTOR_PRIVATE_H__

#define QUOT "\""
#define QUOT_C '\"'
#define QUOT_LEN 1
#define EOL "<br/>"
#define EOL_LEN 5
#define TAB "<tab/>"
#define TAB_LEN 6
#define TAB_SPACE 3
#define REL1_X 0.25f
#define REL1_Y 0.25f
#define REL2_X 0.75f
#define REL2_Y 0.75f
#define VIEW_DATA edj_mgr_view_get(NULL)

extern char EDJE_PATH[PATH_MAX];
extern const char SIG_CURSOR_LINE_CHANGED[];
extern const char SIG_CURSOR_GROUP_CHANGED[];
extern const char SIG_MAX_LINE_CHANGED[];
extern const char SIG_COMPILE_ERROR[];
extern const char SIG_LIVE_VIEW_CURSOR_MOVED[];
extern const char SIG_LIVE_VIEW_RESIZED[];
extern const char SIG_PROGRAM_RUN[];
extern const char SIG_CTXPOPUP_SELECTED[];
extern const char SIG_CTXPOPUP_DISMISSED[];

typedef struct viewer_s view_data;
typedef struct syntax_color_s color_data;
typedef struct parser_s parser_data;
typedef struct attr_value_s attr_value;
typedef struct syntax_helper_s syntax_helper;
typedef struct indent_s indent_data;
typedef struct redoundo_s redoundo_data;
typedef struct editor_s edit_data;

typedef enum attr_value_type
{
   ATTR_VALUE_INTEGER = 1,
   ATTR_VALUE_FLOAT = 2,
   ATTR_VALUE_CONSTANT = 4,
   ATTR_VALUE_PART = 8,
   ATTR_VALUE_STATE = 16,
   ATTR_VALUE_IMAGE = 32,
   ATTR_VALUE_PROGRAM = 64
} attr_value_type;

struct attr_value_s
{
   Eina_Array *strs;
   float min;
   float max;
   attr_value_type type;
   Eina_Bool program : 1;
};

typedef enum {
   TEMPLATE_INSERT_DEFAULT,
   TEMPLATE_INSERT_LIVE_EDIT
} Template_Insert_Type;


/* auto_comp */
void autocomp_init(void);
void autocomp_term(void);
void autocomp_target_set(edit_data *ed);
void autocomp_enabled_set(Eina_Bool enabled);
Eina_Bool autocomp_enabled_get(void);
Eina_Bool autocomp_event_dispatch(const char *key);


/* syntax color */
color_data *color_init(Eina_Strbuf *strbuf);
void color_term(color_data *cd);
const char *color_cancel(color_data *cd, const char *str, int length, int from_pos, int to_pos, char **from, char **to);
const char *color_apply(color_data *cd, const char *str, int length, char *from, char *to);
Eina_Bool color_ready(color_data *cd);


/*parser */
parser_data *parser_init(void);
void parser_term(parser_data *pd);
Eina_Stringshare *parser_first_group_name_get(parser_data *pd, Evas_Object *entry);
void parser_cur_name_get(parser_data *pd, Evas_Object *entry, void (*cb)(void *data, Eina_Stringshare *part_name, Eina_Stringshare *group_name), void *data);
void parser_cur_group_name_get(parser_data *pd, Evas_Object *entry, void (*cb)(void *data, Eina_Stringshare *part_name, Eina_Stringshare *group_name), void *data);
Eina_Stringshare *parser_cur_name_fast_get(Evas_Object *entry, const char *scope);
Eina_Bool parser_type_name_compare(parser_data *pd, const char *str);
attr_value *parser_attribute_get(parser_data *pd, const char *text, const char *cur);
Eina_Stringshare *parser_paragh_name_get(parser_data *pd, Evas_Object *entry);
char *parser_name_get(parser_data *pd, const char *cur);
void parser_cancel(parser_data *pd);
int parser_line_cnt_get(parser_data *pd EINA_UNUSED, const char *src);
Eina_List *parser_states_filtered_name_get(Eina_List *states);
int parser_end_of_parts_block_pos_get(const Evas_Object *entry, const char *group_name);
Eina_Bool parser_images_pos_get(const Evas_Object *entry, int *ret);
Eina_Bool parser_styles_pos_get(const Evas_Object *entry, int *ret);
const char *parser_colon_pos_get(parser_data *pd EINA_UNUSED, const char *cur);


/* syntax helper */
syntax_helper *syntax_init(void);
void syntax_term(syntax_helper *sh);
color_data *syntax_color_data_get(syntax_helper *sh);
indent_data *syntax_indent_data_get(syntax_helper *sh);


/* indent */
indent_data *indent_init(Eina_Strbuf *strbuf);
void indent_term(indent_data *id);
int indent_space_get(indent_data *id, Evas_Object *entry);
void indent_insert_apply(indent_data *id, Evas_Object *entry, const char *insert, int cur_line);
Eina_Bool indent_delete_apply(indent_data *id, Evas_Object *entry, const char *del, int cur_line);


/* build */
void build_edc(void);
void build_init(void);
void build_term(void);
Eina_Bool build_path_set(Enventor_Path_Type type, const Eina_List *pathes);
Eina_List *build_path_get(Enventor_Path_Type type);
void  build_edc_path_set(const char *edc_path);
const char *build_edc_path_get(void);
const char *build_edj_path_get(void);
void build_err_noti_cb_set(void (*cb)(void *data, const char *msg), void *data);

/* dummy_obj */
void dummy_obj_new(Evas_Object *layout);
void dummy_obj_del(Evas_Object *layout);
void dummy_obj_update(Evas_Object *layout);


/* edj_mgr */
void edj_mgr_init(Evas_Object *enventor);
void edj_mgr_term(void);
view_data * edj_mgr_view_new(const char *group);
view_data *edj_mgr_view_get(Eina_Stringshare *group);
Evas_Object * edj_mgr_obj_get(void);
view_data *edj_mgr_view_switch_to(view_data *vd);
void edj_mgr_view_del(view_data *vd);
void edj_mgr_reload_need_set(Eina_Bool reload);
Eina_Bool edj_mgr_reload_need_get(void);
void edj_mgr_clear(void);
void edj_mgr_view_scale_set(double scale);
double edj_mgr_view_scale_get(void);


/* redoundo */
redoundo_data *redoundo_init(Evas_Object *entry);
void redoundo_term(redoundo_data *rd);
void redoundo_clear(redoundo_data *rd);
void redoundo_text_push(redoundo_data *rd, const char *text, int pos, int length, Eina_Bool insert);
void redoundo_text_relative_push(redoundo_data *rd, const char *text);
void redoundo_entry_region_push(redoundo_data *rd, int cursor_pos, int cursor_pos2);
int redoundo_undo(redoundo_data *rd, Eina_Bool *changed);
int redoundo_redo(redoundo_data *rd, Eina_Bool *changed);


/* edj_viewer */
view_data * view_init(Evas_Object *enventor, const char *group, void (*del_cb)(void *data), void *data);
void view_term(view_data *vd);
Evas_Object *view_obj_get(view_data *vd);
void view_new(view_data *vd, const char *group);
void view_part_highlight_set(view_data *vd, const char *part_name);
void view_dummy_set(view_data *vd, Eina_Bool dummy_on);
Eina_Bool view_dummy_get(view_data *vd);
void view_program_run(view_data *vd, const char *program);
Eina_Stringshare *view_group_name_get(view_data *vd);
void *view_data_get(view_data *vd);
void view_scale_set(view_data *vd, double scale);
Eina_List *view_parts_list_get(view_data *vd);
Eina_List *view_images_list_get(view_data *vd);
Eina_List *view_programs_list_get(view_data *vd);
Eina_List *view_part_states_list_get(view_data *vd, const char *part);
Eina_List *view_program_targets_get(view_data *vd, const char *prog);
void view_string_list_free(Eina_List *list);


/* template */
Eina_Bool template_part_insert(edit_data *ed, Edje_Part_Type part_type, Template_Insert_Type insert_type, float rel1_x, float rel1_y, float rel2_x, float rel2_y, const Eina_Stringshare *group_name, char *syntax, size_t n);
Eina_Bool template_insert(edit_data *ed, Template_Insert_Type insert_type, char *syntax, size_t n);


/* ctxpopup */
Evas_Object *ctxpopup_candidate_list_create(edit_data *ed, attr_value *attr, double slider_val, Evas_Smart_Cb ctxpopup_dismiss_cb, Evas_Smart_Cb ctxpopup_selected_cb);
Evas_Object *ctxpopup_img_preview_create(edit_data*ed, const char *imgpath, Evas_Smart_Cb ctxpopup_dismiss_cb, Evas_Smart_Cb ctxpopup_relay_cb);


/* edc_editor */
edit_data *edit_init(Evas_Object *enventor);
void edit_term(edit_data *ed);
Evas_Object *edit_obj_get(edit_data *ed);
Eina_Bool edit_changed_get(edit_data *ed);
void edit_changed_set(edit_data *ed, Eina_Bool changed);
void edit_linenumber_set(edit_data *ed, Eina_Bool linenumber);
Eina_Bool edit_linenumber_get(edit_data *ed);
Eina_Bool edit_save(edit_data *ed, const char *file);
void edit_new(edit_data* ed);
void edit_view_sync_cb_set(edit_data *ed, void (*cb)(void *data, Eina_Stringshare *part_name, Eina_Stringshare *group_name), void *data);
void edit_view_sync(edit_data *ed);
void edit_font_scale_set(edit_data *ed, double font_scale);
double edit_font_scale_get(edit_data *ed);
void edit_part_highlight_toggle(edit_data *ed, Eina_Bool msg);
void edit_line_delete(edit_data *ed);
Eina_Stringshare *edit_cur_prog_name_get(edit_data *ed);
Eina_Stringshare *edit_cur_part_name_get(edit_data *ed);
Eina_Stringshare *edit_cur_paragh_get(edit_data *ed);
int edit_max_line_get(edit_data *ed);
void edit_goto(edit_data *ed, int line);
void edit_syntax_color_full_apply(edit_data *ed, Eina_Bool force);
void edit_syntax_color_partial_apply(edit_data *ed, double interval);
Evas_Object *edit_entry_get(edit_data *ed);
void edit_line_increase(edit_data *ed, int cnt);
void edit_line_decrease(edit_data *ed, int cnt);
int edit_cur_indent_depth_get(edit_data *ed);
void edit_redoundo_region_push(edit_data *ed, int cursor_pos1, int cursor_pos2);
void edit_auto_indent_set(edit_data *ed, Eina_Bool auto_indent);
Eina_Bool edit_auto_indent_get(edit_data *ed);
void edit_part_highlight_set(edit_data *ed, Eina_Bool part_highlight);
Eina_Bool edit_part_highlight_get(edit_data *ed);
void edit_ctxpopup_set(edit_data *ed, Eina_Bool ctxpopup);
Eina_Bool edit_ctxpopup_get(edit_data *ed);
Eina_Bool edit_load(edit_data *ed, const char *edc_path);

#endif