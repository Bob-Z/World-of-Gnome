#!/bin/bash

rm -Rf Clothes01 > /dev/null 2>&1
if [ ! -f "Clothes01.zip" ];then
	wget http://opengameart.org/sites/default/files/Clothes01.zip
fi

echo Unzip archive
unzip "Clothes01.zip" > /dev/null

cd Clothes01
rename 's/^/male_/' *.png
cd ..

./cut_image.sh Clothes01

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Clothes01.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/more-lpc-clothes-and-hair >> $LIC_FILE

