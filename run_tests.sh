#!/bin/sh
rm -f ./_composition_test
cc -Wall -s -O3 -o _composition_test -I . -I ../str2num -I ../callback_printf ../str2num/str2num.c ../callback_printf/callback_printf.c  ../callback_printf/sfprintf.c iniparse.c composition_test.c
./_composition_test
exit $?
