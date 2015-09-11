function f ()

id = player_get_id()
target = character_attribute_tag_get(id,"aggro_target")

id_map = character_get_map(id)
id_x = character_get_x(id);
id_y = character_get_y(id);

target_map = character_get_map(target)
target_x = character_get_x(target);
target_y = character_get_y(target);

-- Do not follow across map
if id_map ~= target_map then
	character_attribute_tag_set(id,"aggro_target","")
        character_set_ai_script(id,"ai/ai_random.lua")
	return
end

if target_x > id_x then
	id_x = id_x +1
end
if target_x < id_x then
	id_x = id_x -1
end
if target_y > id_y then
	id_y = id_y +1
end
if target_y < id_y then
	id_y = id_y -1
end

character_set_pos(id,id_map,id_x,id_y)

-- return the time in ms before the next NPC AI action
return 500
end
