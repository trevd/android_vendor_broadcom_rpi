#!/bin/sh

LOCAL_MANIFESTS_DIR=.repo/local_manifests

VENDOR_MANIFEST_FILE=broadcom-rpi-vendor.xml
LOCAL_MANIFEST_FILE=$LOCAL_MANIFEST_DIR/$VENDOR_MANIFEST_FILE


rm -rfv $LOCAL_MANIFESTS_FILE
mkdir -pv $LOCAL_MANIFESTS_DIR

repo init -u git://github.com/androidarmv6/android.git -b cm-11.0
curl https://raw.github.com/trevd/android_vendor_broadcom_rpi/cm-11.0/manifests/$VENDOR_MANIFEST_FILE > $LOCAL_MANIFEST_FILE




