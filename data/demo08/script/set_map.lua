function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function f (tile_type)

player_id = player_get_id()
map_w = character_get_map_w(player_id)
map_h = character_get_map_h(player_id)

-- determine if the exact name exists
full_name = string.format("%s/image/tile/%s",get_base_directory(),tile_type)
ret = file_exists(full_name)
if ret == true then
	-- actually set tiles
	for x=0,map_w-1 do
		for y=0,map_h-1 do
			tile_name = string.format("tile/%s",tile_type)
			map_set_tile(map_name,0,tile_name,x,y)
		end
	end
	return
end

-- determine max number of image of the same type
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.png",get_base_directory(),tile_type,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1

-- actually set tiles
for x=0,map_w-1 do
	for y=0,map_h-1 do
		tile_name = string.format("tile/%s%d.png",tile_type,math.random(1,max_img))
		map_set_tile(map_name,0,tile_name,x,y)
	end
end

end
