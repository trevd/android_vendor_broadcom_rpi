LOCAL_STEM := rpi/device-partial.mk

$(call inherit-product-if-exists, vendor/broadcom/$(LOCAL_STEM))

PRODUCT_COPY_FILES +=  vendor/broadcom/rpi/proprietary/LICENCE.broadcom:obj/NOTICE_FILES/src/LICENCE.broadcom
