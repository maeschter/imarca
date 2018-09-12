#!/bin/sh
# $LastChangedDate: 2018-03-01 19:48:32 +0100 (Do, 01. Mär 2018) $
# $Rev: 1478 $
# Write compiletime into ressource file
# Will be displayed in menu Help - Info
# 2014.10.21
# 2016.07.13
# 2017.03.16
# 2018.01.24
# The script must be in dir /kbvCore

date +%y%m%d%H%M > ./res/other/kbv_buildtime

echo compiletime updated

