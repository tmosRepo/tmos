/*
 * gmenu.cpp
 *
 *  Created on: 03.10.2013
 *      Author: stanly
 */

#include <stdgui.h>

menu_template_t* GMenu::GetItem(int parent_id, int item_id)
{
	menu_template_t* tmp = base;
	if(tmp)
	{
		while(!IsEmpty(tmp))
		{
			if(tmp->parent == parent_id && tmp->item == item_id )
				return tmp;
			tmp++;
		}
	}
	return nullptr;
}

menu_template_t* GMenu::FindItem(int item_id)
{
	menu_template_t* tmp = base;
	if(tmp)
	{
		while(!IsEmpty(tmp))
		{
			if(tmp->item == item_id)
				return tmp;
			tmp++;
		}
	}
	return nullptr;
}

menu_template_t* GMenu::GetMenu(int parent_id, menu_template_t* start)
{
	menu_template_t* tmp = start;
	if(!start)
		tmp = base;
	if(tmp)
	{
		while(!IsEmpty(tmp))
		{
			if(tmp->parent == parent_id)
				return tmp;
			tmp++;
		}
	}
	return nullptr;
}
int GMenu::get_base_size() const
{
	menu_template_t* tmp = base;
	int res = 0;
	if(tmp)
	{
		while(!IsEmpty(tmp))
		{
			++res;
			++tmp;
		}
	}
	return res;
}

int GMenu::GetMenuSize(int menu_id) const
{
	menu_template_t* tmp = base;
	int res = 0;
	if(tmp)
	{
		while(!IsEmpty(tmp))
		{
			if(tmp->parent == menu_id)
				++res;
			++tmp;
		}
	}
	return res;
}

void GMenu::adjust_item_names()
{
	menu_template_t* tmp = menu;
	while(tmp)
	{
		if(tmp && client_rect.width() && !IsEmpty(tmp))
		{
			CSTRING name = tmp->item_name;
			unsigned int pos = remove_amp(name);
			char* ptr = strchr(name.c_str(), '\t');
			if(ptr)
			{
				CSTRING l_str, r_str;
				int max_chars = (client_rect.width() - (text_font->hdistance << 1)) / text_font->hspacing;
				int l_size = ptr - name.c_str()+1;
				l_str.assign(name.c_str(), l_size);

				while(*ptr && IS_SPACE(*ptr))
					ptr++;
				r_str = ptr;
				int r_size = r_str.length();

				if(l_size + r_size >= max_chars)
				{
					if((r_size+1) < max_chars)
					{
						l_size = max_chars - r_size;
						l_str.erase(l_size-1, l_str.length()-l_size);
					}
					else
					{
						r_size = max_chars;
						l_size =0;
						l_str.clear();
					}
				}
				max_chars -= l_size + r_size;
				while(max_chars)
				{
					l_str +=' ';
					max_chars--;
				}
				l_str += r_str;
#if USE_CSTRING
				if(pos)
					l_str.insert('&', pos-1);
#else
				if(pos != string::npos)
					l_str.insert(pos, 1, '&');
#endif				
				tmp->item_name = l_str;
			}
		}
		tmp = GetMenu(tmp->parent, tmp+1);
	}
}

bool GMenu::add_item(int parent_id, int item_id, const CSTRING& name, short unsigned int flg)
{
	menu_template_t * new_base;

	size = get_base_size();
	new_base = (menu_template_t *)tsk_malloc_clear((size +2)*sizeof(menu_template_t));
	if(!new_base)
		return false;
	if(base)
	{
		for(int i=0; i<size; i++)
		{
			new_base[i].parent = base[i].parent;
			new_base[i].item = base[i].item;
			new_base[i].item_name = base[i].item_name;
			new_base[i].flags = base[i].flags;
			base[i].item_name.free();
		}
		tsk_free(base);
	}
	base = new_base;
	base[size].parent = parent_id;
	base[size].item = item_id;
	base[size].item_name = name;
	base[size].flags = flg;

	size++;
//	item = menu = base;
	return true;
}

int GMenu::get_item_pos(menu_template_t* ptr)
{
	int res=0;
	menu_template_t*  start = GetMenu(ptr->parent);
	while(start)
	{
		if(ptr->item == start->item)
			break;
		++res;
		start = GetMenu(ptr->parent, start +1);
	}
	return res;
}

bool GMenu::SetReplaceItem(int item_id, const CSTRING& item_name, short unsigned int flg)
{
	menu_template_t* ptr;
	ptr = GetItem(0,item_id);
	if(ptr)
	{
		ptr->item_name = item_name;
		ptr->flags = flg;
		return true;
	}
	return	AppendMenu(0, item_id, item_name, flg);
}

bool GMenu::RemoveItem(int item_id)
{
	menu_template_t * new_base;
	menu_template_t* ptr;

	ptr = FindItem(item_id);
	if(!ptr || !base)
		return true;
	if(GetMenu(ptr->item))
		RemoveItem(ptr->item);

	size = get_base_size();
	new_base = (menu_template_t *)tsk_malloc_clear((size +2)*sizeof(menu_template_t));
	if(!new_base)
		return false;

	if(item == ptr)
	{
		menu_template_t* tmp;
		// remove selected
		tmp = GetMenu(item->parent, ptr+1);
		if(tmp)
		{
			item = tmp; // select next
			menu = GetMenu(item->parent);
		}
		else
		{
			if(menu && item && (item != menu))
			{
				menu_template_t* last;
				tmp =menu;
				do
				{
					last = tmp;
					tmp = GetMenu(item->parent, tmp +1);
				} while(tmp && (tmp != item));
				if(tmp)
					item =last;
				else
					item = menu = base;
			}
			else
				item = menu = base;
		}
	}

	for(int i=0, j=0; !IsEmpty(base + i); i++)
	{
		if(base +i == ptr)
		{
			base[i].item_name.free();
			continue;
		}
		new_base[j].parent = base[i].parent;
		new_base[j].item = base[i].item;
		new_base[j].flags = base[i].flags;
		new_base[j].item_name = base[i].item_name;
		base[i].item_name.free();

		if(item == base+i)
			item = new_base +j;
		j++;
	}
	tsk_free(base);
	base = new_base;
	size = GetMenuSize(item->parent);
	menu = GetMenu(item->parent);
	set_scroll();
	return true;
}

bool GMenu::InsertItem(int item_id, int new_item_id, const CSTRING& new_item_name, short unsigned int new_flg)
{
	menu_template_t * new_base;
	menu_template_t* ptr;
	int item_pos;

	ptr = FindItem(item_id);
	if(item)
		item_id = item->item; //keeps the selected , update only members menu/item
	else
		item_id = -1; // no item selected, cannot update menu/item members

	if(!ptr )
	{	// item not found, inserting the new item at topmost
		if(!base)
		{ // if the menu is empty, just add the new item
			if(!add_item(0, new_item_id, new_item_name, new_flg))
				return false;
			item = menu = base;
			return true;
		}
		item_pos = 0;
	}
	else
	{
		if(ptr->item == new_item_id)
			return false;
		item_pos = ptr - base;
/*
 * BUG using pointers of the same type when subtracting
		if(item_pos)
			item_pos /= sizeof(menu_template_t);
*/
	}

	size = get_base_size();
	new_base = (menu_template_t *)tsk_malloc_clear((size +2)*sizeof(menu_template_t));
	if(!new_base)
		return false;
	for(int i=0, j=0; i<size; i++, j++)
	{
		if(item_pos == j)
		{
			// insert new item
			new_base[j].parent = (ptr)?ptr->parent:0;
			new_base[j].item = new_item_id;
			new_base[j].item_name =new_item_name;
			new_base[j].flags = new_flg;
			j++;
		}
		new_base[j].parent = base[i].parent;
		new_base[j].item = base[i].item;
		new_base[j].item_name = base[i].item_name;
		new_base[j].flags = base[i].flags;
		base[i].item_name.free();
	}
	tsk_free(base);
	base = new_base;
	size++;
	if(item_id != -1)
		Select(item_id);
	return true;

}

bool GMenu::AppendItem(int item_id, int new_item_id, const CSTRING& new_item_name, short unsigned int new_flg)
{
	menu_template_t * new_base;
	menu_template_t* append_to;
	int item_pos;

	append_to = FindItem(item_id);
	if(item)
		item_id = item->item; //keeps the selected , update only members menu/item
	else
		item_id = -1; // no item selected, cannot update menu/item members

	if(!append_to )
	{	// item not found, inserting the new item at top-most
		if(!base)
		{ // if the menu is empty, just add the new item
			if(!add_item(0, new_item_id, new_item_name, new_flg))
				return false;
			item = menu = base;
			return true;
		}
		item_pos = get_base_size();
	}
	else
	{
		while(append_to->parent == (append_to +1)->parent && !IsEmpty(append_to+1))
			append_to++;
		if(append_to->item == new_item_id)
			return false;
		item_pos = append_to - base +1;
	}

	size = get_base_size();
	new_base = (menu_template_t *)tsk_malloc_clear((size +2)*sizeof(menu_template_t));
	if(!new_base)
		return false;
	for(int i=0, j=0; i<size; i++, j++)
	{
		if(item_pos == j)
		{
			// insert new item
			new_base[j].parent = (append_to)?append_to->parent:0;
			new_base[j].item = new_item_id;
			new_base[j].item_name =new_item_name;
			new_base[j].flags = new_flg;
			j++;
		}
		new_base[j].parent = base[i].parent;
		new_base[j].item = base[i].item;
		new_base[j].item_name = base[i].item_name;
		new_base[j].flags = base[i].flags;
		base[i].item_name.free();
	}
	tsk_free(base);
	base = new_base;
	size++;
	if(item_id != -1)
		Select(item_id);
	return true;
}

bool GMenu::Select(int item_id, bool redraw)
{
	menu_template_t* ptr = FindItem(item_id);
	if(ptr)
	{
		item = ptr;
		menu = GetMenu(item->parent);
		size = GetMenuSize(item->parent);
		if(redraw)
		{
			set_scroll();
			send_message(WM_DRAW, 0, 0L, this);
		}
		return true;
	}
	return false;
}

bool GMenu::AppendMenu( int parent_id, int item_id, const CSTRING& item_name, short unsigned int flg)
{
	if(!parent_id)
	{
		if(!base)
		{
			if(!add_item(parent_id, item_id, item_name, flg))
				return false;
		}
		else
		{
			if(GetItem(0, item_id))
				return false;
			if(!add_item(parent_id, item_id, item_name, flg))
				return false;
		}
		item = menu = base; // set at last
		return true;
	}
	else
	{
		if(!menu ||GetItem(parent_id, item_id))
			return false;
		if(add_item(parent_id, item_id, item_name, flg))
		{
			item = menu = base;
			return true;
		}
	}
	return false;
}

bool GMenu::LoadMenu(const MENUTEMPLATE* pat)
{
	menu_template_t* p= (menu_template_t*)pat;
	if(p)
	{
//		while(!p->item_name.empty() && p->item)
		while(!IsEmpty(p))
		{
			if( ! AppendMenu(p->parent, p->item, p->item_name, p->flags) )
				return false;
			p++;
		}
	}
	return true;
}

unsigned int remove_amp(CSTRING& str)
{
	unsigned int pos;

#if USE_CSTRING
	if(str.find(pos, '&'))
	{
		str.erase(pos, 1);
		++pos;
	}
	else
		pos = 0;
#else
	pos = str.find('&');
	if(pos != string::npos)
	{
		str.erase(pos, 1);
	}
#endif				
	return pos;
}

unsigned int GMenu::initialize (GMessage& msg)
{
	GObject::initialize(msg);
	if(title)
		client_rect.y0 += text_font->vspacing + 2*text_font->vdistance;
	size = GetMenuSize(0);
	flags &= ~GO_FLG_VSCROLL; // forced allocation of the scroll bar
	set_scroll();
	if(msg.param)
		send_message(WM_DRAW, 0, 0L, this);
	return 1;
}

void GMenu::draw_this (LCD_MODULE* lcd)
{
	int menu_height;
	int sy, ey;
	int rows;
	int row_height;
	CSTRING str;
	menu_template_t* tmp;

	if(flags & GO_FLG_BORDER)
		draw_border(rect);

	if(item && menu)
	{

		if(scroll)
			scroll->draw_scroll(lcd);

		lcd->allign = TA_CENTER;
		lcd->set_font(text_font);

		// draw title
		if(!item->parent)
			str = title;
		else
		{
			tmp = FindItem(menu->parent);
			if(tmp)
			{
				str = tmp->item_name;
				remove_amp(str);
			}
		}
		menu_height = client_rect.height();// - (2*text_font->vdistance);
		if(!str.empty())
		{
			lcd->pos_x = client_rect.x0 - text_font->hspacing;
			lcd->pos_y = client_rect.y0 - text_font->vspacing;
			RECT_T rc(client_rect);
			client_rect = rect;
			//draw_text(lcd, str.c_str());
			draw_text_line(lcd, str.c_str(), str.length());
			lcd->set_color(PIX_LIGHTBLUE);
			draw_hline(	rect.x0 +rect.width()/8,
						rect.x1 -rect.width()/8,
						lcd->pos_y);
			lcd->set_color(get_fg_color());
			client_rect = rc;
		}

		// draw items

		row_height = text_font->vspacing + text_font->vdistance;
		rows = menu_height / row_height;
		if( !(rows & 1) && rows > size )
			rows--;

		sy = get_item_pos(item);
		if(sy >= rows/2 && rows < size )
		{
			sy -= rows/2;
			if(sy + rows >= size && size >= rows )
			{
				sy = size - rows;
			}
		}
		else
			sy = 0;
		ey = sy + rows;


		tmp = menu;

		GClientLcd dc(this);
		if(dc.CreateLcd(client_rect, lcd))
		{
			lcd->allign = TA_LEFT;
			lcd->pos_x = dc.client_rect.x0;
			lcd->pos_y = dc.client_rect.y0 + (2*text_font->vdistance) + (menu_height - (rows*row_height))/2;

			while(tmp)
			{
				int y = get_item_pos(tmp);
				if( y >= sy && y < ey )
				{
					lcd->set_colors(get_fg_color(), get_bg_color());
					str = tmp->item_name;
					rows = remove_amp(str);
					if(str.length())
					{
						if(tmp->flags & GMENU_FLG_CHECK_ITEM)
						{
							if (tmp->flags & GO_FLG_CHECKED)
							{
								lcd->set_color(PIX_GREEN);
								lcd->draw_icon(GICON_CHECKED_SQUARE);
								lcd->set_color(get_fg_color());
							}
							else
								lcd->draw_icon(GICON_SQUARE);
							lcd->pos_x = /*(client_rect.y1 - client_rect.y0) +*/
									dc.client_rect.x0 + ((lcd->font->hspacing * 4)/3);
						}

						if(tmp==item && ((flags & GO_FLG_SELECTED)||(item->flags & GMENU_FLG_SHOW_SELECTED_ITEM)))
						{
							lcd->set_colors(PIX_BLACK, PIX_LIGHTBLUE);
							lcd->clear_rect(RECT_T(lcd->pos_x, lcd->pos_y-((lcd->pos_y>0)?1:0), client_rect.x1, lcd->pos_y + text_font->vspacing));
							dc.draw_text_line(lcd, str.c_str(), str.length());
						}
						else
							dc.draw_text_line(lcd, str.c_str(), str.length());
						lcd->pos_x = dc.client_rect.x0;
					}
					else
						lcd->pos_y += text_font->vspacing;

					if(rows)
					{
						rows = lcd->pos_x + (rows-1)*text_font->hspacing;
						dc.draw_hline(rows, rows + text_font->hspacing, lcd->pos_y- text_font->vdistance);
					}
					lcd->pos_y += text_font->vdistance;
				}
				tmp = GetMenu(tmp->parent, tmp+1);
			}
			dc.RelaseLcd();
		}
	}
}

bool GMenu::process_selected()
{
	if(item)
	{
		menu_template_t* next_menu = GetMenu(item->item);
		if( !next_menu )
		{
			send_message(WM_DRAW, 0, client_rect.as_int, this);
			if(!(item->flags & GMENU_FLG_DISABLE_ITEM))
			{
				if(item->flags & GMENU_FLG_CHECK_ITEM)
				{
					item->flags ^= GO_FLG_CHECKED;
				}
				send_message (WM_COMMAND, item->item, 0L, parent);
			}
		}
		else
		{
			menu = next_menu;
			item = next_menu;
			size = GetMenuSize(menu->parent);
			set_scroll();
			send_message(WM_DRAW, 0, 0L, this);
		}
		return true;
	}
	return false;
}

unsigned int GMenu::process_key (GMessage& msg)
{
	menu_template_t* tmp;
	switch (msg.param)
	{
	case KEY_RIGHT:
		tmp = GetMenu(item->item);
		if( tmp )
		{
			menu = tmp;
			item = tmp;
			size = GetMenuSize(item->parent);
			set_scroll();
			send_message(WM_DRAW, 0, 0L, this);
		}
		return 1;

	case KEY_LEFT:
		tmp = FindItem(item->parent);
		if( tmp )
		{
			item = tmp;
			tmp = menu;
			menu = GetMenu(item->parent);
			size = GetMenuSize(item->parent);
			if(set_scroll() || (tmp != menu))
				send_message(WM_DRAW, 0, 0L, this);
		}
		return 1;

	case KEY_UP:
		if(menu && item && (item != menu))
		{
			menu_template_t* last;

			tmp = menu;
			do
			{
				last = tmp;
				tmp = GetMenu(item->parent, tmp +1);
			} while(tmp && (tmp != item));
			if(tmp)
			{
				item =last;
				if(scroll)
					scroll->SetScrollPos(GO_FLG_VSCROLL, get_item_pos(last),true);
				send_message(WM_DRAW, 0, client_rect.as_int, this);
				return 1;
			}
		}
		return 1;

	case KEY_DOWN:
		tmp = GetMenu(item->parent, item +1);
		if( tmp )
		{
			item = tmp;
			if(scroll)
				scroll->SetScrollPos(GO_FLG_VSCROLL, get_item_pos(item),true);
			send_message(WM_DRAW, 0, client_rect.as_int, this);
		}
		return 1;

	case KEY_ENTER:
		return process_selected();

	default:
		{
			char  ch = TranslateKey(msg.param);
			char* hot_pos;
			if(ch && !(msg.param & KEY_ASCII_CODE))
			{
				tmp = menu;
				while(tmp)
				{
					hot_pos = strchr(tmp->item_name.c_str(), '&');
					if(hot_pos && hot_pos[1] == ch)
					{
						item = tmp;
						return process_selected();
					}
					tmp = GetMenu(menu->parent, tmp +1);
				}
			}
		}
		break;
	}
	return 0;
}

bool GMenu::set_scroll(void)
{
	bool res = false;
	if(client_rect)
	{
//		int rows =	(client_rect.height() -
//					((title)?(text_font->vspacing+2*text_font->vdistance):0)) /
//					(text_font->vspacing + text_font->vdistance);

		int rows =	client_rect.height() /
					(text_font->vspacing + text_font->vdistance);

		if( !(rows & 1) && rows > size )
			rows--;

		if(rows < size && !scroll)
		{
			scroll = new GScroll(this);
		}
		if(scroll)
		{
			scroll->SetScrollRange(GO_FLG_VSCROLL, size);
			if(rows < size)
			{
				int tmp = get_item_pos(item);
				scroll->SetScrollPos(GO_FLG_VSCROLL, tmp);
				res = scroll->ShowScroll(GO_FLG_VSCROLL, true);
			}
			else
				res = scroll->ShowScroll(GO_FLG_VSCROLL, false);
		}
		adjust_item_names();
	}
	return res;
}
