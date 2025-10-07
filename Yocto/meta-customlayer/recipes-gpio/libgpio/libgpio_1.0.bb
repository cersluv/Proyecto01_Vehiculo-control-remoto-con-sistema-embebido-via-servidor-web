SUMMARY = "Biblioteca para GPIO en Raspberry 4"
DESCRIPTION = "Biblioteca de control de gpio creada para el Taller 7 Yocto II"


SECTION = "libs"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=1f6f1c0be32491a0c8d2915607a28f36"


SRC_URI = "file://libgpio-1.0.tar.gz"

S = "${WORKDIR}/${PN}-${PV}"

EXTRA_OECONF += "--enable-shared"

inherit autotools

PROVIDES += "${PN}-staticdev"

FILES_${PN} = "${libdir}/*.so* ${bindir}/*"
FILES_${PN}-staticdev = "${includedir} ${datadir}"
