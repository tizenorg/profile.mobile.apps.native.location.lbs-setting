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

#ifndef LBS_SETTING_WINDOW_H_
#define LBS_SETTING_WINDOW_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOCATION_SETUP_WIZARD_HELP_1 S_("IDS_ST_POP_YOUR_LOCATION_DATA_INCLUDING_GPS_DATA_WILL_BE_USED_BY_RELEVANT_APPLICATIONS")
#define LOCATION_SETUP_WIZARD_HELP_2 S_("IDS_STU_BODY_USE_LOCATION_EXPLANATION_MSG_3")
#define LOCATION_SETUP_WIZARD_HELP_3 P_("IDS_STU_BODY_SAMSUNG_APPLICATIONS_MAY_USE_COLLECT_TRANSFER_AND_OR_TRANSMIT_THE_USERS_LOCATION_INFORMATION_IF_ALLOWED")
enum {
	LOCATION_POPUP_GPS,
	LOCATION_POPUP_WIFI,
	LOCATION_POPUP_USEMYLOCATION,
};

#define REPLY_KEY_GPS		"gps_state"
#define REPLY_KEY_WIFI		"wifi_state"
#define REPLY_VALUE_ON		"on"
#define REPLY_VALUE_OFF		"off"

#define LOCATION_MAIN_VIEW		0
#define LOCATION_HELP_VIEW		1
#define CURRENT_SCREEN_TYPE_ID	"screen_type_id_key"
#define LOCATION_WIZARD_VIEW	4
#define LOCATION_UG_CALLER		"caller"
#define SETUP_INFO_MAX_LEN		1024

#define VCONF_LBS_SETTING_IS_SHOW_GPS_POPUP "db/location/setting/GpsPopup"

void lbs_setting_win_show(lbs_setting_app_data *ad);
void lbs_setting_win_activate(lbs_setting_app_data *ad);
void lbs_setting_win_deactivate(lbs_setting_app_data *ad);

Evas_Object *create_win(const char *name);
Evas_Object *create_layout(Evas_Object *parent);
Evas_Object *create_bg(Evas_Object *parent);
Evas_Object *create_conformant(Evas_Object *parent);
Evas_Object *create_indicator_bg(Evas_Object *parent);
void __setting_location_create_view(lbs_setting_app_data *ad);
int __setting_location_init(lbs_setting_app_data *ad);

Evas_Object *__setting_location_wizard_view(lbs_setting_app_data *ad);


#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* LBS_SETTING_WINDOW_H_ */
