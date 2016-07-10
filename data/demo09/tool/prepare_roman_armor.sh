#!/bin/bash

rm -Rf Legion_armor > /dev/null 2>&1
if [ ! -f "Legion armor.zip" ];then
	wget http://opengameart.org/sites/default/files/Legion%20armor.zip
fi
echo Unzip archive
unzip "Legion armor.zip" > /dev/null
mv "Legion armor" Legion_armor

./cut_image.sh Legion_armor

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Legion_armor.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-roman-armor >> $LIC_FILE
