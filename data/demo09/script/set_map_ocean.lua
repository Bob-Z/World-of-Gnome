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

-- Layer for player's move
layer = 15
map_add_layer(map_name,layer,map_w,map_h,32,32,"","h")

-- Layer for images
layer = 0
image_w = 324
image_h = 144
layer_w = math.ceil(map_w*32/image_w)
layer_h = math.ceil(map_h*32/image_h)

map_add_layer(map_name,layer,layer_w,layer_h,image_w,image_h,"","h")
tile_array = {}
index=1
tile_name = "tile/ocean.gif"
for x=1,layer_w do
	for y=1,layer_h do
		tile_array[index] = tile_name
		tile_array[index+1] = nil
		index = index + 1
	end
end

map_set_tile_array(map_name,layer,tile_array)

map_broadcast(map_name);

end
