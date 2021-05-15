/*
   Copyright (C) 2020-2021 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include <unistd.h>
#include <vector>

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <init/DeviceLibinit.h>
#include "property_service.h"
#include "vendor_init.h"

namespace android {
namespace init {

static const char* BUILD_FINGERPRINT = "google/coral/coral:11/RQ2A.210405.005/7181113:user/release-keys";
static const char* BUILD_DESCRIPTION = "coral-user 11 RQ2A.210405.005 7181113 release-keys";
std::string bootsku;

using android::base::GetProperty;
using android::base::SetProperty;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
};

int property_set(const char *key, const char *value) {
	return __system_property_set(key, value);
}

void property_override(char const prop[], char const value[], bool add = true)
{
	prop_info *pi;

	pi = (prop_info *) __system_property_find(prop);
	if (pi)
		__system_property_update(pi, value, strlen(value));
	else if (add)
		__system_property_add(prop, strlen(prop), value, strlen(value));
}

/* From Magisk@jni/magiskhide/hide_utils.c */
static const char *snet_prop_key[] = {
	"ro.boot.vbmeta.device_state",
	"ro.boot.flash.locked",
	"ro.boot.veritymode",
	"ro.boot.warranty_bit",
	"ro.warranty_bit",
	NULL
};

static const char *snet_prop_value[] = {
	"locked",
	"1",
	"enforcing",
	"0",
	"0",
	NULL
};

static void workaround_snet_properties() {
	// Hide all sensitive props
	for (int i = 0; snet_prop_key[i]; ++i) {
		property_override(snet_prop_key[i], snet_prop_value[i]);
	}
}

const auto set_ro_product_prop = [](const std::string &source,
		const std::string &prop, const std::string &value) {
	auto prop_name = "ro.product." + source + prop;
	property_override(prop_name.c_str(), value.c_str(), false);
};

const auto set_ro_build_prop = [](const std::string &source,
		const std::string &prop, const std::string &value) {
	auto prop_name = "ro." + source + "build." + prop;
	property_override(prop_name.c_str(), value.c_str(), false);
};

void tmobileProps() {
	system("echo 'init_channel: T-Mobile detected! Setting stock props!' | tee /dev/kmsg");
	system("echo 'init_channel: Safetynet may fail!' | tee /dev/kmsg");
	property_override("ro.build.description", "channel_revvl-user 10 QPY30.85-18 6572f release-keys");
	property_override("persist.vendor.radio.customer_mbns", "tmo_usa_ims_default.mbn;sprint_usa_ims.mbn");
	property_override("persist.vendor.radio.data_con_rprt", "1");
	property_override("persist.vendor.ims.playout_delay", "10");
	property_override("persist.vendor.ims.cam_sensor_delay", "20");
	property_override("persist.vendor.ims.display_delay", "40");
	for (const auto &source : ro_props_default_source_order) {
		set_ro_build_prop(source, "fingerprint", "motorola/channel_revvl/channel:10/QPY30.85-18/6572f:user/release-keys");
		set_ro_product_prop(source, "device", "channel");
		set_ro_product_prop(source, "model", "REVVLRY");
		set_ro_product_prop(source, "name", "channel_revvl");
	}
}

void globalProps() {
	system("echo 'init_channel: Global detected! Lets set our own fingerprint and description' | tee /dev/kmsg");
	system("echo 'init_channel: Safetynet may pass!' | tee /dev/kmsg");
	property_override("ro.build.description", BUILD_DESCRIPTION);
	property_override("ro.build.fingerprint", BUILD_FINGERPRINT);
	for (const auto &source : ro_props_default_source_order) {
		set_ro_build_prop(source, "fingerprint", BUILD_FINGERPRINT);
		set_ro_product_prop(source, "device", "channel");
		set_ro_product_prop(source, "model", "moto g(7) play");
	}
}

void vendor_load_properties() {
	property_override("ro.boot.verifiedbootstate", "green");
	workaround_snet_properties();
	bootsku = GetProperty("ro.boot.hardware.sku", "");
	if (bootsku == "XT1952-T") {
		tmobileProps(); // T-Mobile Model
	} else {
		globalProps(); // Every other model
	}
}

}  // namespace init
}  // namespace android
