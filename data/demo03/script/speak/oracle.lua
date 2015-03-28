function f (keyword,player)

if player == nil then
	player = player_get_id()
end
player_portrait = character_get_portrait(player)
player_name = character_get_name(player)

if keyword == nil then
	keyword = "start"
end

text = string.format("keyword = %s",keyword)
print_text_debug(text);

if keyword == "start" then
	text = string.format("Yezul: Hello %s, I am Yezul. Would you like to help me ?",player_name)
	popup_send(player,
		"image","portrait/oracle.png",
		"text",text,
		"eop",
		"image",player_portrait,
		"action","speak/oracle","yes",
		"text","Yes, of course   ",
		"action","speak/oracle","bye",
		"text","   No, thanks")
end

if keyword == "yes" then
	character_attribute_set(player,"quest_given",0)

	item = item_create_from_template("artefact")
	map_add_item("cliff",0,item,5,5);
	text = string.format("Thanks %s, please go fetch the holy artefact !",player_name)
	popup_send(player,
		"image","portrait/oracle.png",
		"text",text,
		"eop",
		"image",player_portrait,
		"action", "popup_end", "",
		"text","Let's go !")
end

if keyword == "bye" then
	character_attribute_set(player,"quest_given",0)

	popup_send(player,
		"image","portrait/oracle.png",
		"text","See you !",
		"eop",
		"image",player_portrait,
		"action", "popup_end", "",
		"text","Bye !")
end

end
