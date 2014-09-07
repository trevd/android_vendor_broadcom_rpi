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

#echo "Patching Build System - Making PI Friendly"
#patch  --silent --forward --reject-file=- --strip=0 < vendor/broadcom/rpi/patches/android_build.patch
if [ ! -f vendor/cm/proprietary/Term.apk ] ; then
    ./vendor/cm/get-prebuilts
fi
