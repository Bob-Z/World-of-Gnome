price_list = { sword = 200, shield = 150, bag = 25, magic = 500 }

function set_player_portrait(player_id,mood)
		player_type = character_get_type(player_id)
		filename = string.format("portrait/%s_%s.gif",player_type,mood)
		character_set_portrait(player_id,filename)
end

function trade (player_id,item)
	gold = inventory_get_by_name(player_id,"gold")
	if gold == nil then
		text = string.format("You don't have gold, get out of here !")
		portrait = "portrait/old_man_angry.gif"

		set_player_portrait(player_id,"angry");

		return text,portrait
	end

	gold_qty = resource_get_quantity(gold)
	price = price_list[item]
	if gold_qty >= price then
		resource_set_quantity(gold,gold_qty-price)
		new_item = item_create_from_template(item)
		inventory_add(player_id,new_item)
		text = string.format("You got a new %s. What do you need ?",item)
		portrait = "portrait/old_man_happy.gif"
		set_player_portrait(player_id,"happy");
		return text,portrait
	end

	text = string.format("You don't have enough gold, get out !")
	portrait = "portrait/old_man_angry.gif"
	set_player_portrait(player_id,"angry");
	return text,portrait
end

function talk (npc,portrait,player_id,text)
	func_param = {}
	for k, v in pairs(price_list) do
		keyword = k
		price = v
		speech = string.format("item/%s.gif",keyword)
		table.insert(func_param, speech)
		speech = string.format("Buy a %s (%d gold)",keyword,price)
		table.insert(func_param, speech)
		table.insert(func_param, keyword)
	end
	table.insert(func_param,"")
	table.insert(func_param,"Bye !")
	table.insert(func_param,"speak_end")

	speak_send(npc,portrait,player_id,text, unpack(func_param))
end

function f (npc,player_id,keyword)

	if keyword == nil then
		keyword = "start"
	end

	if keyword == "speak_end" then
		speak_send("","",player_id,"")
		return
	end

	if keyword == "start" then
		text = string.format("Hello %s. What do you need ?",character_get_name(player_id))
		portrait = "portrait/old_man.gif"
	else
		text,portrait = trade(player_id,keyword)
	end


	talk(npc,portrait,player_id,text)
end
