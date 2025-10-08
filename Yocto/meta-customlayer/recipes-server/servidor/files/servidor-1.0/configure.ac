AC_INIT([servidor], [1.0], [losguapos@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC

# Chequear bibliotecas externas
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([crypto], [HMAC_Init_ex])
AC_CHECK_LIB([ssl], [SSL_new])
AC_CHECK_LIB([gpio], [gpioSetup])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT

