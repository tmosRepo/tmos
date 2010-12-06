/*
 * cache.cpp
 *
 *  Created on: 2010-8-4
 *      Author: Miroslav Kostadinov
 */

#include <tmos.h>
#include <str_list.h>
#include <cache.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+			CCache
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//*----------------------------------------------------------------------------
//								getc()
//								---------------
//	Return:
//		error: 0, success - symbol
//
//*----------------------------------------------------------------------------
char CCache::getc()
{
	char c;

	if (pos)
	{
		c = *pos++;
		if (c || ((unsigned) (pos - buf.c_str()) <= buf.length()))
			return c;
	}

	if (load() == RES_OK)
		return *pos++;
	return 0;
}

RES_CODE CCache::getc(char& c)
{
	RES_CODE res;

	if (pos)
	{
		if ((c = *pos++) || ((unsigned) (pos - buf.c_str()) <= buf.length()))
			return RES_OK;
	}

	res = load();
	if (res == RES_OK)
		return c = *pos++;
	return res;
}

RES_CODE CCache::get_pc(char&c)
{
	RES_CODE res;

	while (1)
	{
		res = getc(c);
		if (res != RES_OK)
			break;

		if (c >= ' ')
			break;
	}

	return res;
}

RES_CODE CCache::skip_ws()
{
	RES_CODE res;

	do
	{
		if (pos)
		{
			do
			{
				if (*pos > ' ')
					return RES_OK;
				pos++;
			} while ((unsigned) (pos - buf.c_str()) <= buf.length());
		}

		res = load();
	} while (res == RES_OK);

	return res;

}

//*----------------------------------------------------------------------------
//								ungetc()
//								---------------
//	Return:
//		error: 0, success - symbol
//
//*----------------------------------------------------------------------------
bool CCache::ungetc()
{
	if (pos > buf.c_str())
	{
		pos--;
		return true;
	}

	return false;
}

//*----------------------------------------------------------------------------
//*			read line
//*----------------------------------------------------------------------------
RES_CODE CCache::readline(CSTRING& var)
{
	unsigned int res;
	int i;

	while (1)
	{
		if (pos)
		{
			res = buf.c_str() + buf.length() - pos; //max size
			i = 0;
			while (res--)
			{
				if (!pos[i] || pos[i] == '\n')
				{
					//we found \n or \0
					var.append(pos, i);
					pos += i + 1;
					return RES_OK;
				}
				i++;
			}
			var.append(pos, i);
		}
		res = load();
		if (res != RES_OK)
			return res;

	};
}

//*----------------------------------------------------------------------------
//*			read line timed
//*----------------------------------------------------------------------------
RES_CODE CCache::readline(CSTRING& str, unsigned int time)
{
	CURRENT_TASK->time = CURRENT_TIME + time;
	return readline(str);
}

RES_CODE CCache::load()
{
	pos = NULL;

	if (next.empty())
		return RES_EOF;

	buf = next;
	next.clear();

	pos = buf.c_str();
	return RES_OK;
}

//*----------------------------------------------------------------------------
//			get_name()
//								---------------
//	Return: RES_OK if the char was skipped, RES_EOF if no more tags, errors..
//
//	Description:
//		Name	   		::=   	NameStartChar (NameChar)*
//		NameStartChar	::=   	":" | [A-Z] | "_" | [a-z]	//add "?"
//		NameChar	   	::=   	NameStartChar | "-" | "." | [0-9]
//*----------------------------------------------------------------------------
RES_CODE CCache::get_name(CSTRING& var)
{
	char ch;
	RES_CODE res;

	res = get_pc(ch);
	if (res == RES_OK)
	{
		if (ch == ':' || ch == '_' || ch == '?' || IS_ALPHA(ch))
		{
			var += ch;

			while (!var.empty())
			{
				res = getc(ch);
				switch (res)
				{
				case RES_OK:
					if (ch == ':' || ch == '_' || ch == '-' || ch == '.'
							|| IS_ALPHANUM(ch))
					{
						var += ch;
					}
					else
					{
						ungetc();
						return RES_OK;
					}
					break;

				case RES_EOF:
					return RES_OK;

				default:
					return res;
				}

			}
			return RES_OUT_OF_MEMORY;
		}
		ungetc();
		res = RES_INVALID_DATA;
	}
	return res;
}

//*----------------------------------------------------------------------------
//*			buffer
//*----------------------------------------------------------------------------
RES_CODE CCache::buffer(CSTRING &var)
{
	unsigned int len;

	if (!var.empty())
	{
		if (pos)
		{
			len = buf.length() - (pos - buf.c_str());
			if (len)
			{
				CSTRING s(pos, len);
				if (next.empty())
					next = s;
				else
					next = s + next;
			}
		}
		buf = var;
		pos = (char*) buf.c_str();
	}
	return RES_OK;
}

//*----------------------------------------------------------------------------
//			match()
//								---------------
//	Description:
//		Strips the white space and then tries to read and match.
//	Return: RES_OK if str matches. Otherwise the cache is restored
//
//*----------------------------------------------------------------------------
RES_CODE CCache::match(const char* str)
{
	RES_CODE res;

	res = skip_ws();

	if (res == RES_OK)
	{
		const char* ptr = str;

		if (ptr)
		{
			while (*ptr)
			{
				char c;

				res = getc(c);
				if (res != RES_OK)
					break;

				if (c != *ptr++)
				{
					for (unsigned int i = ptr - str; i; i--)
						if (!ungetc())
						{
							CSTRING s(str, i);
							buffer(s);
						}
					res = RES_INVALID_DATA;
					break;
				}

			}
		}

	}

	return res;
}

RES_CODE CCache::match_name(const char* str)
{
	RES_CODE res;

	res = match(str);

	if (res == RES_OK)
	{
		char c;
		res = getc(c);

		if (res == RES_OK)
		{
			ungetc();
			if (IS_ALPHANUM(c) || c == ':' || c == '_' || c == '-' || c == '.')
			{
				//this is not a name
				unsigned int len = strlen(str);

				while (len)
				{
					if (!ungetc())
					{
						CSTRING s(str, len);
						buffer(s);
						break;
					}
					len--;

				}
				res = RES_INVALID_DATA;
			}
		}
		else if (res == RES_EOF)
			res = RES_OK;
	}

	return res;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+			CCachedHandle
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


RES_CODE CCachedHandle::load()
{
	unsigned int len;

	if (!next.empty())
	{

		buf = next;
		next.clear();

		pos = buf.c_str();
		return RES_OK;
	}

	len = buf.reserve(size);
	if (len)
	{
		pos = (char*) buf.c_str();
		hnd->tsk_resume_read((void*) pos, len);
		len -= hnd->len;
		buf.storage.ram->len = len;
		buf.storage.ram->buf[len] = 0;
		return hnd->res;
	}

	return RES_OUT_OF_MEMORY;
}

