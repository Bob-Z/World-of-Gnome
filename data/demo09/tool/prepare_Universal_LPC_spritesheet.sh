#!/bin/bash

rm -Rf Universal-LPC-spritesheet-master
if [ ! -f master.zip ];then
	wget https://github.com/jrconway3/Universal-LPC-spritesheet/archive/master.zip
fi
echo Unzip archive
unzip master.zip > /dev/null

./cut_image.sh Universal-LPC-spritesheet-master

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Universal-LPC-spritesheet.lic
echo Author: jrconway3 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo Source: https://github.com/jrconway3/Universal-LPC-spritesheet >> $LIC_FILE
echo Source: http://gaurav.munjal.us/Universal-LPC-Spritesheet-Character-Generator/# >> $LIC_FILE
