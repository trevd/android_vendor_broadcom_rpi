echo "Patching Build System - Making PI Friendly"
patch  --silent --forward --reject-file=- --strip=0 < vendor/broadcom/rpi/patches/android_build.patch
if [ ! -f vendor/cm/proprietary/Term.apk ] ; then
    ./vendor/cm/get-prebuilts
fi
