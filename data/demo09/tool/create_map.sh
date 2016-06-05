#!/bin/bash

MAP_QTY=`ls -d -1 ? | wc -l`
echo Generating $MAP_QTY maps

for NUM in `ls -d ? | sort`;do
	MAP_NAME=C$NUM

	FILE_QTY=`ls -1 $NUM/WC_*_marquee.zip | wc -l`
	echo $FILE_QTY files in map $NUM
	let "COLUMN=($FILE_QTY+1)/2"
	#Add 2 column for map exits
	let "WIDTH=$COLUMN+2"

	FILE=`ls $NUM/WC_*_marquee.zip`

	echo "attribute = {" > $MAP_NAME
        echo "  x = {"	>> $MAP_NAME
        echo "        current = $NUM"	>> $MAP_NAME
	echo "      }"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME
	echo "width = $WIDTH"	>> $MAP_NAME
	echo "height = 3"	>> $MAP_NAME
	echo "tile_width = 64"	>> $MAP_NAME
	echo "tile_height = 64"	>> $MAP_NAME
	echo "warp_x = 0"	>> $MAP_NAME
	echo "warp_y = 0"	>> $MAP_NAME
	echo "layer0 = {"	>> $MAP_NAME
	echo "        set = ["	>> $MAP_NAME
	CURRENT_TILE=0
	echo "\"\"," >> $MAP_NAME #exit column
	for f in $FILE;do
		if [ "$CURRENT_TILE" = "$COLUMN" ];then
			echo "\"\"," >> $MAP_NAME #exit column
			echo "\"tile/exit.gif\"," >> $MAP_NAME #exit tile
			let "W_1=$WIDTH-1"
			for i in `seq 2 ${W_1}`;do
				echo "\"\"," >> $MAP_NAME
			done
			echo "\"tile/exit.gif\"," >> $MAP_NAME #exit tile
			echo "\"\"," >> $MAP_NAME #exit column
		fi
		echo "\"character/$f\"," >> $MAP_NAME
		let "CURRENT_TILE=$CURRENT_TILE+1"
	done
	echo "\"\"" >> $MAP_NAME #exit column

	echo "        ]"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME

	#Events
	echo "event_list: {"	>> $MAP_NAME
	XPOS=1
	YPOS=0
	CURRENT_TILE=0
	for f in $FILE;do
		SHORT_NAME=`echo $f | sed 's/.*WC_//g' | sed 's/_marquee.zip//g'`
		if [ "$CURRENT_TILE" = "$COLUMN" ];then
			XPOS=1
			YPOS=2
		fi
		echo "E$CURRENT_TILE = {" >> $MAP_NAME
		echo "   pos_x = $XPOS" >> $MAP_NAME
		echo "   pos_y = $YPOS" >> $MAP_NAME
		echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
		echo "   param = ( \"$NUM\", \"$SHORT_NAME\" )" >> $MAP_NAME
		echo "}" >> $MAP_NAME

		let "CURRENT_TILE=$CURRENT_TILE+1"
		let "XPOS=$XPOS+1"
	done
	echo "}"	>> $MAP_NAME
done
