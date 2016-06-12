#!/bin/bash

DIR_LIST=directory_list.txt

DIR=1
echo $DIR- Eyes > $DIR_LIST
mkdir $DIR
mv body/??_*_eyes_* $DIR

DIR=2
echo $DIR- Nose >> $DIR_LIST
mkdir $DIR
mv body/??_*_nose_* $DIR

DIR=3
echo $DIR- Ears >> $DIR_LIST
mkdir $DIR
mv body/??_*_ears_* $DIR

DIR=0
echo $DIR- Body >> $DIR_LIST
mkdir $DIR
mv body/??_* $DIR

DIR=4
echo $DIR- Mustache >> $DIR_LIST
mkdir $DIR
mv facial/??_*stache* $DIR

let "DIR=$DIR+1"
echo $DIR- Beard >> $DIR_LIST
mkdir $DIR
mv facial/??_*_beard* $DIR

let "DIR=$DIR+1"
echo $DIR- Fiveoclock >> $DIR_LIST
mkdir $DIR
mv facial/??_*_fiveoclock* $DIR

let "DIR=$DIR+1"
echo $DIR- Hair >> $DIR_LIST
mkdir $DIR
mv hair/??_*.png $DIR
mv hair/??_*.zip $DIR

let "DIR=$DIR+1"
echo $DIR- Shoes >> $DIR_LIST
mkdir $DIR
mv feet/??_* $DIR

let "DIR=$DIR+1"
echo $DIR- Torso civil >> $DIR_LIST
mkdir $DIR
mv torso/??*_corset_* $DIR
mv torso/??*_dress_* $DIR
mv torso/??*_robes_* $DIR
mv torso/??*sleeve* $DIR
mv formal_male_no_th-sh/??*_vest* $DIR
mv formal_male_no_th-sh/??*_shirt* $DIR

let "DIR=$DIR+1"
echo $DIR- Torso military >> $DIR_LIST
mkdir $DIR
mv torso/??_*chain_mail_* $DIR
mv torso/??_*chest_* $DIR
mv torso/??_*tunics_* $DIR

let "DIR=$DIR+1"
echo $DIR- Arms >> $DIR_LIST
mkdir $DIR
mv torso/??_*arms_* $DIR
mv hands/??_* $DIR

let "DIR=$DIR+1"
echo $DIR- Shoulder >> $DIR_LIST
mkdir $DIR
mv torso/??_*spikes_* $DIR
mv torso/??_*shoulders_* $DIR

let "DIR=$DIR+1"
echo $DIR- Legs civil >> $DIR_LIST
mkdir $DIR
mv formal_male_no_th-sh/??_*pants* $DIR
mv legs/??_*_pants_* $DIR
mv legs/??_*_skirt_* $DIR

let "DIR=$DIR+1"
echo $DIR- Legs military >> $DIR_LIST
mkdir $DIR
mv legs/??_*armor_* $DIR

let "DIR=$DIR+1"
echo $DIR- Belt >> $DIR_LIST
mkdir $DIR
mv belt/??_* $DIR

let "DIR=$DIR+1"
echo $DIR- Tabard >> $DIR_LIST
mkdir $DIR
mv torso/??_*tabard_* $DIR

let "DIR=$DIR+1"
echo $DIR- Neck >> $DIR_LIST
mkdir $DIR
mv accessories/??_* $DIR
mv formal_male_no_th-sh/??_*tie* $DIR

let "DIR=$DIR+1"
echo $DIR- Cape >> $DIR_LIST
mkdir $DIR
mv torso/??*_cape_* $DIR
mv behind_body/??*_cape_* $DIR

let "DIR=$DIR+1"
echo $DIR- Head >> $DIR_LIST
mkdir $DIR
mv head/??_* $DIR

let "DIR=$DIR+1"
echo $DIR- Weapon >> $DIR_LIST
mkdir $DIR
mv weapons/??_* $DIR

let "DIR=$DIR+1"
echo $DIR- Quiver >> $DIR_LIST
mkdir $DIR
mv behind_body/??*_quiver* $DIR

let "DIR=$DIR+1"
echo $DIR- Wings >> $DIR_LIST
mkdir $DIR
mv torso/??*_wings_* $DIR

