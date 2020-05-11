/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2019 carabobz@gmail.com

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef CLIENT_UI_CHATBOX_H_
#define CLIENT_UI_CHATBOX_H_

#include "ChatEditBox.h"
#include "ChatHistoryBox.h"

class SdlItem;

class ChatBox
{
public:
	ChatBox(TTF_Font * const font, std::function<void(std::string)> editCallBack);
	virtual ~ChatBox();

	void compose(std::vector<SdlItem *> & itemArray, const int upperBorderHeight, const int lowerBorderHeight);

private:
	ChatEditBox m_chatEditBox;
	ChatHistoryBox m_chatHistoryBox;
};

#endif /* CLIENT_UI_CHATBOX_H_ */
