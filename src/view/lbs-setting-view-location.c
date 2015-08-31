/*
 *  lbs-setting
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Kyoungjun Sung <kj7.sung@samsung.com>, Young-Ae Kang <youngae.kang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <efl_assist.h>
#include <vconf.h>
#include "lbs-setting-string.h"
#include "lbs-setting-view-location.h"
#include "lbs-setting-view-popup.h"

static Evas_Object *g_genlist = NULL;
static Eina_Bool is_checked = FALSE;

Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	LS_FUNC_ENTER

	return EINA_TRUE;
}

char *_gl_label_get(void *data, Evas_Object *obj, const char *part)
{
	LS_FUNC_ENTER
	char *label = NULL;

	LS_RETURN_VAL_IF_FAILED(part, "NULL");

	if (!strcmp(part, "elm.text")) {
		label = STR_LOCATION;
	} else if (!strcmp(part, "elm.text.sub")) {
		gboolean is_location_on = FALSE;

		vconf_get_int(VCONFKEY_LOCATION_ENABLED, &is_location_on);
		if (is_location_on) {
			label = STR_ON;
		} else {
			label = STR_OFF;
		}
	}

	LS_LOGI("part [%s], label[%s]", part, label ? label : "NULL");

	return label = g_strdup(label ? label : "NULL");
}

Evas_Object *_gl_check_get(void *data, Evas_Object *obj, const char *part)
{
	LS_FUNC_ENTER

	Evas_Object *check = NULL;
	gboolean is_location_on = FALSE;

	if (!strcmp(part, "elm.swallow.icon.1")) {
		check = elm_check_add(obj);
		elm_object_style_set(check, "list");
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_propagate_events_set(check, EINA_FALSE);
		evas_object_repeat_events_set(check, EINA_TRUE);

		vconf_get_int(VCONFKEY_LOCATION_ENABLED, &is_location_on);
		is_checked = is_location_on;
		elm_check_state_set(check, (is_checked) ? EINA_TRUE : EINA_FALSE);
	}

	return check;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	LS_FUNC_ENTER
}

void _popup_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	LS_RETURN_IF_FAILED(ad);
	LS_RETURN_IF_FAILED(ad->app_popup);

	evas_object_del(ad->app_popup);
	ad->app_popup = NULL;
}

void _popup_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	is_checked = !is_checked;
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	LS_RETURN_IF_FAILED(ad);
	LS_RETURN_IF_FAILED(ad->app_popup);

	LS_LOGI("is_checked [%s]", is_checked ? "TRUE" : "FALSE");

	vconf_set_int(VCONFKEY_LOCATION_ENABLED, is_checked);

	view_location_update();

	evas_object_del(ad->app_popup);
	ad->app_popup = NULL;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);

	if (!is_checked) {
		lbs_setting_popup_title_2button(STR_USER_CONSENT
										, STR_USER_CONSENT_MSG
										, STR_DISAGREE, _popup_disagree_cb
										, STR_AGREE, _popup_agree_cb
										, (void *)event_info);
	} else {
		is_checked = !is_checked;

		LS_LOGI("is_checked [%s]", is_checked ? "TRUE" : "FALSE");

		vconf_set_int(VCONFKEY_LOCATION_ENABLED, is_checked);

		elm_genlist_item_update(it);
	}
}

gboolean view_location_update(void)
{
	LS_FUNC_ENTER
	lbs_setting_app_data *ad = lbs_setting_common_get_app_data();
	LS_RETURN_VAL_IF_FAILED(ad, FALSE);

	Evas_Object *nf = ad->nf;
	LS_RETURN_VAL_IF_FAILED(nf, FALSE);

	elm_genlist_clear(g_genlist);

	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	LS_RETURN_VAL_IF_FAILED(itc, FALSE);

	itc->item_style = "multiline";
	itc->func.text_get = _gl_label_get;
	itc->func.content_get = _gl_check_get;
	itc->func.del = _gl_del;

	Elm_Object_Item *item = NULL;
	item = elm_genlist_item_append(
				g_genlist,
				itc,
				(void *)1,
				NULL,
				ELM_GENLIST_ITEM_NONE,
				_gl_sel,
				(void *)1);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DEFAULT);

	elm_genlist_item_class_free(itc);
	return TRUE;
}
