
edit_data *edit_init(Evas_Object *win, stats_data *sd, config_data *cd);
void edit_term(edit_data *ed);
void edit_edc_read(edit_data *ed, const char *file_path);
void edit_focus_set(edit_data *ed);
Evas_Object *edit_obj_get(edit_data *ed);
Eina_Bool edit_changed_get(edit_data *ed);
void edit_changed_set(edit_data *ed, Eina_Bool changed);
void edit_line_number_toggle(edit_data *ed);
void edit_editable_set(edit_data *ed, Eina_Bool editable);
Eina_Bool edit_save(edit_data *ed);
const char *edit_group_name_get(edit_data *ed);
void edit_new(edit_data* ed);
void edit_part_changed_cb_set(edit_data *ed, void (*cb)(void *data, const char *part_name), void *data);
void edit_cur_part_update(edit_data *ed);
void edit_font_size_update(edit_data *ed, Eina_Bool msg);
void edit_template_insert(edit_data *ed);
void edit_template_part_insert(edit_data *ed, Edje_Part_Type type);
void edit_vd_set(edit_data *ed, view_data *vd);

