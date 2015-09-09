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

#include <app.h>
#include <glib.h>
#include <vconf.h>
#include <Elementary.h>
#include <lbs-setting-common.h>
#include <Elementary.h>
#include <libintl.h>
#include <locations.h>
#include <dlog.h>
#include <app_control.h>
#include <app_control_internal.h>
#include <efl_extension.h>
#include "lbs-setting-window.h"
#include "lbs-setting-help.h"
#include "lbs-setting-common.h"

static lbs_setting_app_data *global_ad;

lbs_setting_app_data *lbs_setting_common_get_app_data(void)
{
	LS_FUNC_ENTER
	return global_ad;
}

void lbs_setting_common_destroy_app_data(void)
{
	LS_FUNC_ENTER
	/*TODO */
	LS_LOGI("Not Implemented");

	return;
}

#if 0
static Eina_Bool _lbs_window_transient_cb(void *data, int type, void *eventinfo)
{
	LS_FUNC_ENTER

	if (eventinfo == NULL) {
		return ECORE_CALLBACK_RENEW;
	}
	Ecore_X_Event_Window_Hide *ev = eventinfo;
	int parent_xwin_id = (int)data;
	if (ev->win == parent_xwin_id) {
		elm_exit();
		return ECORE_CALLBACK_CANCEL;
	}
	return ECORE_CALLBACK_RENEW;
}
#endif

#if 0
static int _get_orientation()
{
	sensor_t sensor;
	sensor_data_t data;
	sensor = sensord_get_sensor(AUTO_ROTATION_SENSOR);
	int handle = sensord_connect(sensor);
	int rotation = 0;

	int ret = sensord_start(handle, 0);
	if (ret < 0) {
		sensord_disconnect(handle);
		LS_LOGE("Fail to auto rotation sensor");
	}

	ret = sensord_get_data(handle, AUTO_ROTATION_BASE_DATA_SET, &data);
	if (ret < 0) {
		LS_LOGE("Fail to get sensor data");
	}

	if (data.value_count > 0) {
		rotation = data.values[0];
	} else {
		LS_LOGE("Fail to auto rotation sensor");
	}

	switch (rotation) {
		case AUTO_ROTATION_DEGREE_UNKNOWN:
		case AUTO_ROTATION_DEGREE_0:
		case AUTO_ROTATION_DEGREE_180:
			rotation = 0;
			break;
		case AUTO_ROTATION_DEGREE_90:
		case AUTO_ROTATION_DEGREE_270:
			rotation = 270;
			break;
		default:
			rotation = 0;
			break;
	}

	sensord_stop(handle);
	sensord_disconnect(handle);

	return rotation;
}
#endif

static bool _app_create_cb(void *user_data)
{
	LS_FUNC_ENTER

	return true;
}

static void _app_terminate_cb(void *user_data)
{
	LS_FUNC_ENTER
}

static void _app_pause_cb(void *user_data)
{
	LS_FUNC_ENTER
	/*Nothing */
}

static void _app_resume_cb(void *user_data)
{
	LS_FUNC_ENTER
	/*Nothing coz thre is nothing when paused */
}

static void _app_control_cb(app_control_h app_control, void *user_data)
{
	LS_FUNC_ENTER

	gboolean ret = FALSE;
	lbs_setting_app_data *ad = (lbs_setting_app_data *) user_data;
	LS_RETURN_IF_FAILED(ad);

	if (ad->win_main) {
		evas_object_del(ad->win_main);
		ad->win_main = NULL;
	}

	bindtextdomain(LBS_SETTING_PKG, "/usr/apps/org.tizen.setting-location/res/locale");

	ad->win_main = create_win(LBS_SETTING_PKG);
	ad->bg = create_bg(ad->win_main);
	ad->conformant = create_conformant(ad->win_main);

	create_indicator_bg(ad->conformant);
	ad->layout_main = create_layout(ad->conformant);
	ret = app_control_clone(&ad->prev_handler, app_control);
	if (FALSE == ret) {
		LS_LOGE("app_control_clone. err=%d", ret);
	}


	char *caller = NULL;
	ret = app_control_get_extra_data(app_control, LOCATION_UG_CALLER, &caller);
	if (FALSE == ret) {
		LS_LOGE("app_control_get_extra_data. err=%d", ret);
	}

	if (caller != NULL && strcmp(caller, "pwlock") == 0) {
		__setting_location_wizard_view(ad);
	} else {
		__setting_location_init(ad);
		__setting_location_create_view(ad);
	}

	if (elm_win_wm_rotation_supported_get(ad->win_main)) {
		int rots[4] = { 0, 90, 180, 270 };	/* rotation value that app may want */
		elm_win_wm_rotation_available_rotations_set(ad->win_main, rots, 4);
	}

#if 0
	unsigned int parent_xwin_id = 0;
	app_control_get_window(app_control, &parent_xwin_id);
	if (parent_xwin_id) {
		ecore_x_icccm_transient_for_set(elm_win_xwindow_get(ad->win_main), parent_xwin_id);
		ecore_x_window_client_manage(parent_xwin_id);
		ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, _lbs_window_transient_cb, (void *)parent_xwin_id);
	}
#endif
	/*LS_FUNC_EXIT */
}

#if 0
static void _app_device_orientation_cb(app_event_info_h event_info, void *user_data)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(event_info);
	LS_RETURN_IF_FAILED(user_data);

	lbs_setting_app_data *ad = (lbs_setting_app_data *)user_data;
	app_device_orientation_e orientation;
	app_event_get_device_orientation(event_info, &orientation);
	if (orientation == APP_DEVICE_ORIENTATION_180) {
		orientation = APP_DEVICE_ORIENTATION_0;
	}
	elm_win_rotation_with_resize_set(ad->win_main, orientation);
}
#endif

static void _app_language_changed_cb(app_event_info_h event_info, void *user_data)
{
	LS_FUNC_ENTER

	char *locale = vconf_get_str(VCONFKEY_LANGSET);
	if (locale) {
		elm_language_set(locale);
	}
}

int main(int argc, char *argv[])
{
	LS_FUNC_ENTER

	int ret = 0;
	lbs_setting_app_data ad = {0,};
	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = _app_create_cb;
	event_callback.terminate = _app_terminate_cb;
	event_callback.app_control = _app_control_cb;
	event_callback.pause = _app_pause_cb;
	event_callback.resume = _app_resume_cb;

	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, NULL, NULL);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _app_language_changed_cb, &ad);

	ret = APP_ERROR_NONE;
	ret = ui_app_main(argc, argv, &event_callback, &ad);

	if (ret != APP_ERROR_NONE) {
		LS_LOGE("ui_app_main() is failed. err=%d", ret);
	}

	LS_FUNC_EXIT
	return 0;
}
