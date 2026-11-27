/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "action.h"
#include "const.h"
#include "entry.h"
#include "LockGuard.h"
#include "mutex.h"
#include "network_server.h"
#include "syntax.h"
#include <cstdio>
#include <stdlib.h>

/***************************************************************************
 Add the specified value to the specified attribute
 Check on max and min are done and call to on_* scripts are done if set
 ctx is the context of the source of the attribute change request
 id is the id of the target of the change
 return -1 if fails
 ***************************************************************************/
int attribute_change(Context *context, const char *table, const char *id,
		const char *attribute, int value)
{
	int current;
	int old;
	int min;
	int max;
	char buf[SMALL_BUF];
	bool do_min_action = false;
	bool do_down_action = false;
	bool do_max_action = false;
	bool do_up_action = false;
	std::string action;
	std::string min_action;
	std::string down_action;
	std::string max_action;
	std::string up_action;

	{
		LockGuard guard(attribute_lock);

		if (entry_read_int(table, id, &current, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() }) == false)
		{
			return -1;
		}

		if (entry_read_int(table, id, &min, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_MIN.c_str() }) == false)
		{
			min = -1;
		}

		if (entry_read_int(table, id, &max, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_MAX.c_str() }) == false)
		{
			max = -1;
		}

		old = current;
		current = current + value;
		if (min != -1)
		{
			if (current <= min)
			{
				do_min_action = true;
				current = min;
			}
		}

		if (max != -1)
		{
			if (current >= max)
			{
				do_max_action = true;
				current = max;
			}
		}

		if (entry_write_int(table, id, current, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() }) == false)
		{
			return -1;
		}
		if (entry_write_int(table, id, old, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_PREVIOUS.c_str() }) == false)
		{
			return -1;
		}

		// Check automatic actions
		if (value < 0)
		{
			if (do_min_action == true)
			{
				if (entry_read_string(table, id, action, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_ON_MIN.c_str() }) == false)
				{
					do_min_action = false;
				}
				else
				{
					min_action = action;
				}
			}

			if (entry_read_string(table, id, action, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_ON_DOWN.c_str() }) == true)
			{
				do_down_action = true;
				down_action = action;
			}
		}

		if (value > 0)
		{
			if (do_max_action == true)
			{
				if (entry_read_string(table, id, action, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_ON_MAX.c_str() }) == false)
				{
					do_max_action = false;
				}
				else
				{
					max_action = action;
				}
			}

			if (entry_read_string(table, id, action, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_ON_UP.c_str() }) == true)
			{
				do_up_action = true;
				up_action = action;
			}
		}

	}

	// do automatic actions
	if (do_down_action == true && !down_action.empty())
	{
		action_execute_script(context, std::string(down_action), {});
	}
	if (!down_action.empty())
	{
	}

	if (do_min_action == true && !min_action.empty())
	{
		action_execute_script(context, std::string(min_action), {});
	}
	if (!min_action.empty())
	{
	}

	if (do_up_action == true && !up_action.empty())
	{
		action_execute_script(context, std::string(up_action), {});
	}
	if (!min_action.empty())
	{
	}

	if (do_max_action == true && !max_action.empty())
	{
		action_execute_script(context, std::string(max_action), {});
	}
	if (!max_action.empty())
	{
	}

	sprintf(buf, "%s.%s.%s", ATTRIBUTE_GROUP.c_str(), attribute,
			ATTRIBUTE_CURRENT.c_str());
	network_broadcast_entry_int(table, id, buf, current, true);
	sprintf(buf, "%s.%s.%s", ATTRIBUTE_GROUP.c_str(), attribute,
			ATTRIBUTE_PREVIOUS.c_str());
	network_broadcast_entry_int(table, id, buf, old, true);
	return 0;
}

/****************************************
 get the specified attribute's value
 return -1 if fails
 ****************************************/
int attribute_get(const char *table, const char *id, const char *attribute)
{
	int current = -1;

	LockGuard guard(attribute_lock);

	entry_read_int(table, id, &current, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() });

	return current;
}

/*********************************************************************
 set the specified attribute's value without check of min and max
 return -1 if fails
 *********************************************************************/
int attribute_set(const char *table, const char *id, const char *attribute,
		int value)
{
	LockGuard guard(attribute_lock);

	if (entry_write_int(table, id, value, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() }) == false)
	{
		return -1;
	}

	return 0;
}

/****************************************
 get the specified attribute tag's value
 return nullptr if fails
 returned value MUST be freed
 ****************************************/
std::string attribute_tag_get(const char *table, const char *id,
		const char *attribute)
{
	std::string tag;

	LockGuard guard(attribute_lock);

	entry_read_string(table, id, tag, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() });

	return tag;
}

/*********************************************************************
 set the specified attribute tag's value
 return -1 if fails
 *********************************************************************/
int attribute_tag_set(const char *table, const char *id, const char *attribute,
		const char *value)
{
	LockGuard guard(attribute_lock);

	if (entry_write_string(table, id, value, { ATTRIBUTE_GROUP.c_str(), attribute, ATTRIBUTE_CURRENT.c_str() }) == false)
	{
		return -1;
	}

	return 0;
}
