function f (map,x, y)

id = player_get_id()

item = inventory_get_by_name(id,"artefact")

if map == "lake" and item ~= nil then
	map = "magic_lake"
end

character_set_pos(id,map,0,x,y)

end
