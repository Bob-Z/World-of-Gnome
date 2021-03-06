function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function f (map_name)

player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

-- Clean-up
for i=0,20 do
	map_delete_layer(map_name,i)
end

-- dirt layer --
-- determine max number of image
tile_name_root = "dirt"
layer = 10
map_add_layer(map_name,layer,map_w,map_h,32,32,"","d")
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.gif",get_base_directory(),tile_name_root,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1

tile_array = {}
index=1
for x=1,map_w do
	for y=1,map_h do
		tile_name = string.format("tile/%s%d.gif",tile_name_root,math.random(1,max_img))
		tile_array[index] = tile_name
		tile_array[index+1] = nil
		index = index + 1
	end
end
map_set_tile_array(map_name,layer,tile_array)

map_broadcast(map_name);

end
