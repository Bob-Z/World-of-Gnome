#!/bin/bash

OUTPUT_DIR="/tmp"
TILE_SIZE_X=64
TILE_SIZE_Y=64
TILE_SIZE="${TILE_SIZE_X}x${TILE_SIZE_Y}"
FILL_COLOR="black"

IMAGE_SIZE_X=`identify -format "%[fx:w]" $1`
IMAGE_SIZE_Y=`identify -format "%[fx:h]" $1`

TILE_X=$((IMAGE_SIZE_X / TILE_SIZE_X))
EXACT=$((IMAGE_SIZE_X % TILE_SIZE_X))
if [ EXACT != 0 ];then
	TILE_X=$((TILE_X+1))
fi
TILE_Y=$((IMAGE_SIZE_Y / TILE_SIZE_Y))
EXACT=$((IMAGE_SIZE_Y % TILE_SIZE_Y))
if [ EXACT != 0 ];then
	TILE_Y=$((TILE_Y+1))
fi

FILE_NAME=`basename $1`
MAP_NAME=`echo $FILE_NAME | awk -F. '{ print $1 }'`
EXT_NAME=`echo $FILE_NAME | awk -F. '{ print $2 }'`

mkdir $OUTPUT_DIR/map
mkdir $OUTPUT_DIR/tile
mkdir $OUTPUT_DIR/image

convert $1 -crop $TILE_SIZE -background $FILL_COLOR -extent $TILE_SIZE $OUTPUT_DIR/image/${MAP_NAME}_%05d.$EXT_NAME

OUTPUT_MAP=$OUTPUT_DIR/map/$MAP_NAME

echo "size_x = $TILE_X;" > $OUTPUT_MAP
echo "size_y = $TILE_Y;" >> $OUTPUT_MAP
echo "tile_size_x = $TILE_SIZE_X;" >> $OUTPUT_MAP
echo "tile_size_y = $TILE_SIZE_Y;" >> $OUTPUT_MAP
echo -n "set = [ " >> $OUTPUT_MAP
set=""
for i in `ls  $OUTPUT_DIR/image/${MAP_NAME}_*$EXT_NAME`
do
	filename=`basename $i`
	tilename=`echo $filename | awk -F. '{ print $1 }'`
	echo "image = \"$filename\";" > $OUTPUT_DIR/tile/$tilename
#	echo -n "\"$tilename\"," >> $OUTPUT_MAP
	set=`echo -n "$set\"$tilename\","`

done
#remove trailing coma
echo -n "${set%?}" >> $OUTPUT_MAP
echo -n "];" >> $OUTPUT_MAP
