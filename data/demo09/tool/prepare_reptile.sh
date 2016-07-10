#!/bin/bash

rm -Rf Reptile > /dev/null 2>&1
if [ ! -f "Reptile.zip" ];then
	wget http://opengameart.org/sites/default/files/Reptile.zip
fi

echo Unzip archive
unzip "Reptile.zip" > /dev/null

cd Reptile
rename 's/fem/female/' *.png
rename 's/Fem/female/' *.png
cd ..

./cut_image.sh Reptile

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

LIC_FILE=output/Reptile.lic
echo Author: Nila122 > $LIC_FILE
echo License: CC-BY-SA 3.0 >> $LIC_FILE
echo License: GPL 3.0 >> $LIC_FILE
echo License: GPL 2.0 >> $LIC_FILE
echo Source: http://opengameart.org/content/drakes-and-lizardfolk >> $LIC_FILE

