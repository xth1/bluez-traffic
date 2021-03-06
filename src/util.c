/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Thiago da Silva Arruda <thiago.xth1@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <glib.h>
#include <string.h>
#include <stdio.h>
#define BUFF_SIZE 32

char *make_str(const char c_str[])
{
	char *str;
	int sz = strlen(c_str);

	str = (char *) malloc((sz + 1) * sizeof(char));

	strcpy(str, c_str);

	return str;
}

char *to_str(int d)
{
	char buff[BUFF_SIZE];
	
	sprintf(buff,"%d",d);
	
	return make_str(buff);
}


gboolean has_key(GHashTable *table, gpointer key)
{
	gpointer pointer;
	
	pointer = g_hash_table_lookup(table, key);
	
	if(pointer != NULL)
		return TRUE;
	return FALSE;
}
