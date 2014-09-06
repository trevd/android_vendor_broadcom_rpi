#!/bin/sh
#
# Copyright (C) 2014 Trevor Drake <trevd1234@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License v2.0 Only as 
# published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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


curl https://raw.githubusercontent.com/trevd/android_vendor_broadcom_rpi/development/manifests/broadcom-rpi-vendor.xml > $LOCAL_MANIFEST_FILE
