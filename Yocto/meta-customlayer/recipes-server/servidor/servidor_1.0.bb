SUMMARY = "Servidor TCP que usa GPIO y OpenSSL"
DESCRIPTION = "Servidor TCP que usa GPIO y OpenSSL para control remoto en Raspberry Pi"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://servidor-1.0.tar.gz"
S = "${WORKDIR}/servidor-1.0"

DEPENDS += "openssl libgpio"

inherit autotools

do_install() {
    install -d ${D}${bindir}
    install -m 0755 src/servidor ${D}${bindir}
}

FILES:${PN} += "${bindir}/servidor"

