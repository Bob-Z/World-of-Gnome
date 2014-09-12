function f (npc,player,keyword)

oracle_name = character_get_name(npc);
player_name = character_get_name(player);

if keyword == nil then
	keyword = "start"
end

if keyword == "start" then
	text = string.format("Hello %s, I am %s. Would you like to help me ?",player_name,oracle_name)
	speak_send(oracle_name,player,text,"","Yes, of course","yes","","No, thanks","bye")
end

if keyword == "yes" then
	text = string.format("Thanks %s, please go fetch the holy item !",player_name)
	speak_send(oracle_name,player,text)
end

if keyword == "bye" then
	speak_send(oracle_name,player,"See you !")
end
end
