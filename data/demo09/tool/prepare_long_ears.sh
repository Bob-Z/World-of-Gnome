#!/bin/bash

rm -Rf Long_ears > /dev/null 2>&1
if [ ! -f "Long ears.zip" ];then
	wget http://opengameart.org/sites/default/files/Long%20ears.zip
fi

echo Unzip archive
unzip "Long ears.zip" > /dev/null

mv "Long ears" "Long_ears"

./cut_image.sh Long_ears

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Long_ears.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/lpc-long-ears >> $LIC_FILE
