function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function f (map_name)

tile_type = "grass"
player_id = player_get_id()
map = character_get_map(player_id)

map_w = character_get_map_w(map)
map_h = character_get_map_h(map)

-- determine if the exact name exists
full_name = string.format("%s/image/tile/%s",get_base_directory(),tile_type)
ret = file_exists(full_name)
if ret == true then
	-- actually set tiles
	for x=0,map_w-1 do
		for y=0,map_h-1 do
			tile_name = string.format("tile/%s",tile_type)
			map_set_tile_no_update(map_name,0,tile_name,x,y)
		end
	end
	map_broadcast(map_name);
	return
end

-- determine max number of grass image
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.png",get_base_directory(),tile_type,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1

-- actually set tiles on layer 0
for x=0,map_w-1 do
	for y=0,map_h-1 do
		tile_name = string.format("tile/%s%d.png",tile_type,math.random(1,max_img))
		map_set_tile_no_update(map_name,0,tile_name,x,y)
	end
end

-- determine max number of bush image
tile_type = "grass_bush"
max_img = 1
ret = true
while ret == true do
	max_img = max_img + 1 
	full_name = string.format("%s/image/tile/%s%d.png",get_base_directory(),tile_type,max_img)
	ret = file_exists(full_name)
end
max_img = max_img - 1
bush_density = 100
-- actually set tiles on layer 1
for x=0,map_w-1 do
	for y=0,map_h-1 do
		if math.random(1,bush_density) == 1 then
			print_text_debug("Add bush")
			tile_name = string.format("tile/%s%d.png",tile_type,math.random(1,max_img))
			map_set_tile_no_update(map_name,1,tile_name,x,y)
		end
	end
end

-- let characters walks over grass
-- map_set_character_layer(map_name,1);

map_broadcast(map_name);


end
