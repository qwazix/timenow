TEMPLATE = subdirs

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog

SUBDIRS += \
    timenowd

#while [ true ]; do chmod +x /tmp/qtc_packaging_timenow/timenow/debian/rules; done
