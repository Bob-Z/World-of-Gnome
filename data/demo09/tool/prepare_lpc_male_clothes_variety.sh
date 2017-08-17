#!/bin/bash

rm -Rf lpc_male_clothes_variety_0 > /dev/null 2>&1
if [ ! -f "lpc_male_clothes_variety_0.zip" ];then
	wget https://opengameart.org/sites/default/files/lpc_male_clothes_variety_0.zip
fi

echo Unzip archive
unzip -d lpc_male_clothes_variety_0 "lpc_male_clothes_variety_0.zip" > /dev/null

#cd lpc_male_clothes_variety_0
#rename 's/^/male_/' *.png
#cd ..

./cut_image.sh lpc_male_clothes_variety_0

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/lpc_male_clothes_variety_0.lic
echo Author: Boom Shaka > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo Source: https://opengameart.org/content/lpc-male-clothes-variety-pack >> $LIC_FILE

