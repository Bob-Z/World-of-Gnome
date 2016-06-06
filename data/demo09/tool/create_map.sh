#!/bin/bash

MAP_QTY=`ls -d -1 ? | wc -l`
echo Generating $MAP_QTY maps

for NUM in `ls -d [0-9]* | sort -h`;do
	MAP_NAME=C$NUM

	FILE_QTY=`ls -1 $NUM/WC_*_marquee.zip | wc -l`
	echo $FILE_QTY files in map $NUM
	let "COLUMN=($FILE_QTY+1)/2"
	let "ADD_TILE=$FILE_QTY%2"
	#Add 2 column for map exits and 2 for cancel tiles
	let "WIDTH=$COLUMN+2+2"
	FIRST_TILE=2
	let "LAST_TILE=$COLUMN+2-1"

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
	echo "\"\"," >> $MAP_NAME #left exit column
	echo "\"tile/fog.png\"," >> $MAP_NAME #left cancel tile
	for f in $FILE;do
		echo "\"character/$f\"," >> $MAP_NAME
		let "CURRENT_TILE=$CURRENT_TILE+1"
		if [ "$CURRENT_TILE" = "$COLUMN" ];then
			echo "\"tile/fog.png\"," >> $MAP_NAME #right cancel tile
			echo "\"\"," >> $MAP_NAME #right exit column
			echo "\"tile/exit.gif\"," >> $MAP_NAME #left exit tile
			echo "\"\"," >> $MAP_NAME #left cancel column
			for i in `seq $FIRST_TILE $LAST_TILE`;do
				echo "\"\"," >> $MAP_NAME
			done
			echo "\"\"," >> $MAP_NAME #right cancel column
			echo "\"tile/exit.gif\"," >> $MAP_NAME #exit tile
			echo "\"\"," >> $MAP_NAME #left exit column
			echo "\"tile/fog.png\"," >> $MAP_NAME #left cancel tile
		fi
	done
	if [ "$ADD_TILE" = 1 ];then
		echo "\"\"," >> $MAP_NAME
	fi
	echo "\"tile/fog.png\"," >> $MAP_NAME #right cancel tile
	echo "\"\"" >> $MAP_NAME #right exit column

	echo "        ]"	>> $MAP_NAME
	echo "}"	>> $MAP_NAME

	#Events
	echo "event_list: {"	>> $MAP_NAME

	XPOS=2
	YPOS=0
	CURRENT_EVENT=0
	for f in $FILE;do
		SHORT_NAME=`echo $f | sed 's/.*WC_//g' | sed 's/_marquee.zip//g'`
		if [ "$CURRENT_EVENT" = "$COLUMN" ];then
			XPOS=2
			YPOS=2
		fi
		echo "E$CURRENT_EVENT = {" >> $MAP_NAME
		echo "   pos_x = $XPOS" >> $MAP_NAME
		echo "   pos_y = $YPOS" >> $MAP_NAME
		echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
		echo "   param = ( \"$NUM\", \"$SHORT_NAME\" )" >> $MAP_NAME
		echo "}" >> $MAP_NAME

		let "CURRENT_EVENT=$CURRENT_EVENT+1"
		let "XPOS=$XPOS+1"
	done

	#Left exit event
	let "PREV_NUM=$NUM-1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = 0" >> $MAP_NAME
	echo "   pos_y = 1" >> $MAP_NAME
	echo "   script = \"goto.lua\"" >> $MAP_NAME
	echo "   param = ( \"C$PREV_NUM\", \"1\", \"1\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right exit event
	let "RIGHT_X=$WIDTH-1"
	let "NEXT_NUM=$NUM+1"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $RIGHT_X" >> $MAP_NAME
	echo "   pos_y = 1" >> $MAP_NAME
	echo "   script = \"goto.lua\"" >> $MAP_NAME
	echo "   param = ( \"C$NEXT_NUM\", \"1\", \"1\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Left cancel events
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = 1" >> $MAP_NAME
	echo "   pos_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = 1" >> $MAP_NAME
	echo "   pos_y = 2" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	#Right cancel events
	let "RIGHT_X=$WIDTH-2"
	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $RIGHT_X" >> $MAP_NAME
	echo "   pos_y = 0" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "E$CURRENT_EVENT = {" >> $MAP_NAME
	echo "   pos_x = $RIGHT_X" >> $MAP_NAME
	echo "   pos_y = 2" >> $MAP_NAME
	echo "   script = \"set_sprite.lua\"" >> $MAP_NAME
	echo "   param = ( \"$NUM\", \"\" )" >> $MAP_NAME
	echo "}" >> $MAP_NAME

	let "CURRENT_EVENT=$CURRENT_EVENT+1"

	echo "}"	>> $MAP_NAME
done
