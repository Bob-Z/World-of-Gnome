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

#include "ChatHistoryBox.h"

#include "SdlItemCore.h"
#include "textview.h"

static constexpr Uint32 BACKGROUND_COLOR = 0xFFFFFF40;

static constexpr Uint32 TEXT_TIMEOUT_MS = 5000;

/*****************************************************************************/
ChatHistoryBox::ChatHistoryBox(TTF_Font * const font) :
		m_font(font)
{
}

/*****************************************************************************/
ChatHistoryBox::~ChatHistoryBox()
{
}

/*****************************************************************************/
void ChatHistoryBox::compose(std::vector<SdlItem *> & itemArray, const int upperBorderHeight, const int lowerBorderHeight)
{
	const history_entry_t * history = nullptr;
	history_entry_t * hist = nullptr;

	history = textview_get_history();

	if (history == nullptr)
	{
		return;
	}

	hist = (history_entry_t*) history;
	Uint32 timeStamp = SDL_GetTicks();
	int historyBoxY = lowerBorderHeight;

	while (hist != nullptr)
	{
		if (timeStamp > hist->time + TEXT_TIMEOUT_MS)
		{
			return;
		}

		SdlItem * item = new SdlItem;
		itemArray.push_back(item);

		item->setOverlay(true);
		item->setText(hist->text);
		item->setBackGroudColor(BACKGROUND_COLOR);
		item->setFont(m_font);

		int textWidth = 0;
		int textHeight = 0;
		sdl_get_string_size(m_font, hist->text, &textWidth, &textHeight);

		item->setPos(0, historyBoxY - textHeight);
		item->setShape(textWidth, textHeight);
		historyBoxY -= textHeight;
		if (upperBorderHeight > historyBoxY)
		{
			return;
		}

		hist = hist->next;
	}
}
