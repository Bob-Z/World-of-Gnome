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

#include "log.h"
#include <bits/types/FILE.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

/***************************************************************************
 Return a string representing the checksum of the file + a bool to true on success
 filename is the directory + name
 ***************************************************************************/
std::pair<bool, std::string> checksum_file(const std::string & file_name)
{
	FILE *fp = nullptr;

	unsigned long long checksum = 5381;

	std::pair<bool, std::string> result
	{ false, "0" };

	fp = fopen(file_name.c_str(), "r"); // read mode

	if (fp == nullptr)
	{
		return result;
	}

	int ch = 0;
	while ((ch = fgetc(fp)) != EOF)
	{
		checksum = checksum * 33 ^ ch;
	}

	fclose(fp);

	char text[128];
	snprintf(text, sizeof(text), "%llu", checksum);

	result.first = true;
	result.second = text;

	//LOG("Checksum for " + std::string(file_name) + " is " + std::string(result.second));

	return result;
}
