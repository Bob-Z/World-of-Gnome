#!/bin/bash

ULPC_BASE=Universal-LPC-spritesheet-master
LEGION_BASE=Legion_armor
BANDAGE_BASE=Bandage
MASK_BASE=Masks
HAIR00_BASE=Hair00
LONGEARS_BASE=Long_ears
APRONS_BASE=Aprons
REPTILE_BASE=Reptile
CLOTHES01_BASE=Clothes01
CROWNS_BASE=Crowns
CLOTHES_VARIETY_BASE=lpc_male_clothes_variety_0
LONG_KNIFE_BASE=Long_Knife
PICKAXE_BASE=Pickaxe
WARHAMMER_BASE=Warhammer
AXE_BASE=Axe
KITE_SHIELD_BASE=KiteShield
LIZARD_HEADGEAR_BASE=LizardHeadgear

DEST_DIR=output
mkdir $DEST_DIR > /dev/null 2>&1

COMMAND=cp

DIR_LIST=$DEST_DIR/directory_list.txt

NUM_DIR=0
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Body >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/body/??_*male_light* $DIR
$COMMAND $ULPC_BASE/body/??_*male_dark* $DIR
$COMMAND $ULPC_BASE/body/??_*male_orc* $DIR
$COMMAND $ULPC_BASE/body/??_*male_red_orc* $DIR
$COMMAND $ULPC_BASE/body/??_*male_tanned* $DIR
$COMMAND $ULPC_BASE/body/??_*male_skeleton* $DIR
$COMMAND $REPTILE_BASE/??_*Drake* $DIR
$COMMAND $REPTILE_BASE/??_*Lizard* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Eyes > $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/body/??_*_eyes_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Nose >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/body/??_*_nose_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Ears >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/body/??_*_ears_* $DIR
$COMMAND $LONGEARS_BASE/??_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Mustache >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/facial/??_*stache* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Beard >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/facial/??_*_beard* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Fiveoclock >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/facial/??_*_fiveoclock* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Mask >> $DIR_LIST
mkdir $DIR
$COMMAND $MASK_BASE/??_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Hair >> $DIR_LIST
mkdir $DIR
#$COMMAND $ULPC_BASE/hair/??_*.png $DIR
find $ULPC_BASE/hair -name "??_*.png" -exec $COMMAND {} $DIR \;
#$COMMAND $ULPC_BASE/hair/??_*.zip $DIR
find $ULPC_BASE/hair -name "??_*.zip" -exec $COMMAND {} $DIR \;
$COMMAND $HAIR00_BASE/??_* $DIR
$COMMAND $CLOTHES01_BASE/??_*Messy* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Shoes >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/feet/??_* $DIR
$COMMAND $LEGION_BASE/??_*sandals* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Legs civil >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/formal_male_no_th-sh/??_*pants* $DIR
$COMMAND $ULPC_BASE/legs/??_*magenta_pants_* $DIR
$COMMAND $ULPC_BASE/legs/??_*red_pants_* $DIR
$COMMAND $ULPC_BASE/legs/??_*white_pants_* $DIR
$COMMAND $ULPC_BASE/legs/??_*teal_pants_* $DIR
$COMMAND $ULPC_BASE/legs/??_*_skirt_* $DIR
$COMMAND $ULPC_BASE/torso/??*_overskirt* $DIR
$COMMAND $CLOTHES01_BASE/??_*Pants* $DIR
$COMMAND $CLOTHES_VARIETY_BASE/*_pants_* $DIR
$COMMAND $CLOTHES_VARIETY_BASE/*_robe_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Legs military >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/legs/??_*armor_* $DIR
$COMMAND $LEGION_BASE/??_*Skirt* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Torso civil >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??*_corset_* $DIR
$COMMAND $ULPC_BASE/torso/??_*_vest* $DIR
$COMMAND $ULPC_BASE/torso/??_*_sash_* $DIR
$COMMAND $ULPC_BASE/torso/??_*_tightdress* $DIR
$COMMAND $ULPC_BASE/torso/??_*_underdress* $DIR
$COMMAND $ULPC_BASE/torso/??*_robes_* $DIR
$COMMAND $ULPC_BASE/torso/??*sleeve* $DIR
$COMMAND $ULPC_BASE/formal_male_no_th-sh/??*_vest* $DIR
$COMMAND $ULPC_BASE/formal_male_no_th-sh/??*_shirt* $DIR
$COMMAND $ULPC_BASE/torso/??_*tunics_* $DIR
$COMMAND $CLOTHES01_BASE/??_*sleevless* $DIR
$COMMAND $CLOTHES_VARIETY_BASE/*_shirt_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Torso military >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??_*chain_mail_* $DIR
$COMMAND $ULPC_BASE/torso/??_*chest_* $DIR
$COMMAND $LEGION_BASE/Plate/??_*plate* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Belt >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/belt/??_*cloth* $DIR
$COMMAND $ULPC_BASE/belt/??_*leather* $DIR
$COMMAND $ULPC_BASE/belt/??_*metal* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Buckles >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/belt/??_*buckles* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Arms >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/hands/??_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Shoulder >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??_*shoulders_* $DIR
$COMMAND $ULPC_BASE/torso/??_*arms_* $DIR
$COMMAND $LEGION_BASE/Bauldron/??_*bauldron* $DIR
$COMMAND $CLOTHES01_BASE/??_*Bauldron* $DIR

#gold spikes seems incomplete
rm $ULPC_BASE/torso/??_*spikes_* > /dev/null 2>&1


let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Aprons >> $DIR_LIST
mkdir $DIR
$COMMAND $APRONS_BASE/??_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Tabard >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??_*tabard_* $DIR
$COMMAND $CLOTHES_VARIETY_BASE/*_tabard_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Neck >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/accessories/??_* $DIR
$COMMAND $ULPC_BASE/formal_male_no_th-sh/??_*tie* $DIR
$COMMAND $CLOTHES01_BASE/??_*Scarf* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Bandage >> $DIR_LIST
mkdir $DIR
$COMMAND $BANDAGE_BASE/??_*Bandage* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Cape >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??*_cape_* $DIR
#$COMMAND behind_body/??*_cape_* $DIR
$COMMAND $LEGION_BASE/cape/??_*cape* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Head >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/head/??_* $DIR
$COMMAND $LEGION_BASE/Helmet/??_*helmet* $DIR
$COMMAND $CROWNS_BASE/??_* $DIR
$COMMAND $CLOTHES_VARIETY_BASE/*_hood_* $DIR
$COMMAND $LIZARD_HEADGEAR_BASE/??_* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Weapon >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/weapons/??_* $DIR
find $ULPC_BASE/weapons ! -name "*shield*"  -name "??_*.png" -exec $COMMAND {} $DIR \;
$COMMAND $LONG_KNIFE_BASE/* $DIR
$COMMAND $PICKAXE_BASE/* $DIR
$COMMAND $WARHAMMER_BASE/* $DIR
$COMMAND $AXE_BASE/* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Shield >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/weapons/??*_shield* $DIR
$COMMAND $KITE_SHIELD_BASE/* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Quiver >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/behind_body/??*_quiver* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Tail >> $DIR_LIST
mkdir $DIR
$COMMAND $REPTILE_BASE/??_*TailFront* $DIR

let "NUM_DIR=$NUM_DIR+1"
DIR=$DEST_DIR/$NUM_DIR
echo $NUM_DIR- Wing >> $DIR_LIST
mkdir $DIR
$COMMAND $ULPC_BASE/torso/??*_wings_* $DIR
$COMMAND $REPTILE_BASE/??_*WingFront* $DIR

