function f (keyword)

id = player_get_id()

target = character_get_selected_character_id(id)
if target == nil then
	print_text_debug("no id selected")
	print_text_id(id, "Please, select a character to talk to")
	return
end

script = character_get_speak(target);

--loaded_func = loadfile(script);
loaded_func, cError = loadfile("/home/fred/.config/wog/server/demo02/script/npc_speak.lua")

if cError ~= nil then
	error(cError,1)
end

--text = string.format("loaded_func = %s",loaded_func)
--print_text_debug(text)

if keyword == nil then
	keyword = "start"
end

print_text_debug("calling func")
--pcall(loaded_func,target,id,keyword);
pcall(loaded_func,target,id,keyword);

end
