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

#ifndef __GPS_POPUP_H_
#define __GPS_POPUP_H_

#include <dlog.h>
#include <Elementary.h>
#include <bundle_internal.h>
#include <syspopup.h>

#define PACKAGE "gps-syspopup"
#define GPS_EDJ "/usr/apps/com.samsung.gps-syspopup/res/edje/gps.edj"

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "org.tizen.gps-syspopup"
#endif

#ifndef PREFIX
#define PREFIX "/usr/apps/"PACKAGE_NAME
#endif

#define GPSPOPUP_COMMON_RES		PREFIX"/res/locale"
#define GPS_POPUP_TAG			"GPS_SYSPOPUP"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG GPS_POPUP_TAG
#endif

#define GPS_POPUP_LOG(fmt,args...) LOGD(fmt, ##args)

#define GPS_POPUP_RETURN_IF_FAILED(point) do { \
		if (point == NULL) { \
			GPS_POPUP_LOG("GPS_POPUP_RETURN_IF_FAILED"); \
			return; \
		} \
	} while (0)

#define GPS_POPUP_RETURN_VAL_IF_FAILED(point, val) do { \
		if (point == NULL) { \
			GPS_POPUP_LOG("GPS_POPUP_RETURN_VAL_IS_FAILED"); \
			return val; \
		} \
	} while (0)

#define P_(s)			dgettext(PACKAGE_NAME, s)

#define S_(s)			dgettext("sys_string", s)
#define dgettext_noop(s)	(s)
#undef N_
#define N_(s)			dgettext_noop(s)

/**
* @struct gps_popup_appdata
* @brief This structure defines gps popup main info.
*/
struct gps_popup_appdata {
	Evas_Object *win_main;				/**< win */
	Evas_Object *conform;
	Evas_Object *layout;

	Evas_Object *gps_popup;				/**< gps_popup */
	Evas_Object *wireless_popup_setting;/**< gps_popup */
	Evas_Object *gps_popup_setting;
	bundle *b;
	E_DBus_Connection *dbus_connection;

	Evas_Object *ask_check;
};

#endif /*__GPS_POPUP_H_*/
