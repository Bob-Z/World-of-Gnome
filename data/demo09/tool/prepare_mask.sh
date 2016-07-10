#!/bin/bash

rm -Rf Masks > /dev/null 2>&1
if [ ! -f "Masks.zip" ];then
	wget http://opengameart.org/sites/default/files/Masks.zip
fi

echo Unzip archive
unzip "Masks.zip" > /dev/null

./cut_image.sh Masks

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Masks.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-masks >> $LIC_FILE
