#!/bin/sh

LOCAL_MANIFEST_DIR=.repo/local_manifests

VENDOR_MANIFEST_FILE=broadcom-rpi-vendor.xml
LOCAL_MANIFEST_FILE=$LOCAL_MANIFEST_DIR/$VENDOR_MANIFEST_FILE


rm -rfv $LOCAL_MANIFEST_FILE
mkdir -pv $LOCAL_MANIFEST_DIR
if [ ! -z $USE_LOCAL_MIRROR ] ; then
  echo "Going Loco in Acapulco"
  repo init -u /src/androidarmv6/android.git -b cm-11.0
else
	echo "Getting Remote"
  repo init -u git://github.com/androidarmv6/android.git -b cm-11.0
fi


curl https://raw.github.com/trevd/android_vendor_broadcom_rpi/cm-11.0/manifests/$VENDOR_MANIFEST_FILE > $LOCAL_MANIFEST_FILE
