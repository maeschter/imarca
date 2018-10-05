#!/bin/sh
# LastChangedDate: 2018-09-20
# Write compiletime into ressource file
# Will be displayed in menu Help - Info
# 2014.10.21
# 2016.07.13
# 2017.03.16
# 2018.01.24
# The script must be in dir /kbvCore

date +%y%m%d%H%M > ./res/other/kbv_buildtime

echo compiletime updated

