function f (listner_id, keyword)

id = player_get_id()

action = character_get_speak(listner_id);

if keyword == nil then
	keyword = "start"
end

call_action(action,listner_id,id,keyword);

end
