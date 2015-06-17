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

#include "lbs-setting-view-popup.h"

static void _popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER

	evas_object_del(obj);
	if (data) {
		lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
		LS_RETURN_IF_FAILED(ad->app_popup);
		ad->app_popup = NULL;
	}
}
gboolean lbs_setting_popup_title_2button(char *title, char *body, char *btn1, Evas_Smart_Cb btn1_func, char *btn2, Evas_Smart_Cb btn2_func, void *user_data)
{
	LS_LOGD(">>");
	LS_RETURN_VAL_IF_FAILED(title, FALSE);
	LS_RETURN_VAL_IF_FAILED(body, FALSE);
	LS_RETURN_VAL_IF_FAILED(btn1, FALSE);
	LS_RETURN_VAL_IF_FAILED(btn1_func, FALSE);
	LS_RETURN_VAL_IF_FAILED(btn2, FALSE);
	LS_RETURN_VAL_IF_FAILED(btn2_func, FALSE);

	lbs_setting_app_data *appData = lbs_setting_common_get_app_data();
	LS_RETURN_VAL_IF_FAILED(appData, FALSE);

	if (appData->app_popup) {
		LS_LOGE("critical error : shold be fixed. previous created popup was not destroyed");
	}

	Evas_Object *btn = NULL;
	appData->app_popup = elm_popup_add(appData->win_main);
	elm_popup_align_set(appData->app_popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_size_hint_weight_set(appData->app_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(appData->app_popup, "title,text", title);
	elm_object_text_set(appData->app_popup, body);
	eext_object_event_callback_add(appData->app_popup, EEXT_CALLBACK_BACK, _popup_back_cb, appData);

	btn = elm_button_add(appData->app_popup);
	elm_object_style_set(btn, "popup");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, btn1);
	elm_object_part_content_set(appData->app_popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", btn1_func, appData);

	btn = elm_button_add(appData->app_popup);
	elm_object_style_set(btn, "popup");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(btn, btn2);
	elm_object_part_content_set(appData->app_popup, "button2", btn);
	evas_object_smart_callback_add(btn, "clicked", btn2_func, appData);

	evas_object_show(appData->app_popup);

	return TRUE;
}
