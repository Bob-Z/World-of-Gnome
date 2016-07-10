#!/bin/bash

rm -Rf Hair00 > /dev/null 2>&1
if [ ! -f "Hair00.zip" ];then
	wget http://opengameart.org/sites/default/files/Hair00.zip
fi

echo Unzip archive
unzip "Hair00.zip" > /dev/null

./cut_image.sh Hair00

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Hair00.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/3-hairs-for-lpc >> $LIC_FILE
