#! /bin/sh

autoheader
aclocal
autoconf
mkdir -p config
automake --add-missing
