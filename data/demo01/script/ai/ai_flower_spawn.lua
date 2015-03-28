function f ()

id = player_get_id()

-- avoid inifnite loop when a flower is created
bloom = character_attribute_get(id,"bloom")

if bloom == -1 then
	character_attribute_set(id,"bloom",1)
	return 5000
end

-- random new position
map = character_get_map(id)
pos_x = character_get_x(id)
pos_y = character_get_y(id)

new_pos_x = pos_x
new_pos_y = pos_y
rand = math.random(-1,1)
if math.random(0,1) == 0 then
	new_pos_x = pos_x + rand
else
	new_pos_y = pos_y + rand
end

-- do not spawn if there is a flower already
new_pos_character = {map_get_character(map,0,new_pos_x,new_pos_y)}

i=1
while new_pos_character[i] ~= nil do
	new_type = character_get_type(new_pos_character[i])
	if new_type == "flower1" then
		return 3333
	end
	if new_type == "flower2" then
		return 4444
	end
	if new_type == "flower3" then
		return 5555
	end
	i = i+1
end

--spawn
r = math.random(1,3)
if r == 1 then
	new_id = character_create_from_template("flower1",map,0,new_pos_x,new_pos_y)
elseif r == 2 then
	new_id = character_create_from_template("flower2",map,0,new_pos_x,new_pos_y)
else
	new_id = character_create_from_template("flower3",map,0,new_pos_x,new_pos_y)
end

if new_id == nil then
	return 5000
end

character_set_npc(new_id,1)

text = string.format("****** creating %s from %s at %d %d (%d %d)",new_id,id,new_pos_x,new_pos_y,pos_x,pos_y)
print_text_debug(text)
-- return the time in ms before the next NPC AI action
return 6666

end

