function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function f ()

player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

-- dirt layer --
-- determine max number of image
tile_type = "dirt"
layer = 0
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.gif",get_base_directory(),tile_type,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1

-- actually set tiles on layer 0
for x=0,map_w-1 do
	for y=0,map_h-1 do
		tile_name = string.format("tile/%s%d.gif",tile_type,math.random(1,max_img))
		map_set_tile_no_update(map_name,layer,tile_name,x,y)
	end
end


-- water layer --
-- determine max number of image
tile_type = "water"
layer = 1
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.png",get_base_directory(),tile_type,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1

-- actually set tiles on layer 1
for x=0,map_w-1 do
	for y=0,map_h-1 do
		tile_name = string.format("tile/%s%d.png",tile_type,math.random(1,max_img))
		map_set_tile_no_update(map_name,layer,tile_name,x,y)
	end
end

-- let characters walks under water
-- map_set_character_layer(map_name,0);

map_broadcast(map_name);


end
