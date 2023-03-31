/*
 * gobject.h
 *
 *  Created on: 21.08.2013
 *      Author: edguest
 */

#ifndef GOBJECT_H_
#define GOBJECT_H_
#include <tmos.h>
#include <stdgui.h>
#include <message.h>
#include <fonts.h>

struct LCD_MODULE;
struct GContainer;

enum object_type:unsigned char
{
	OBJECT_OBJECT=0,
	OBJECT_CONTAINER,
	OBJECT_BUTTON,
	OBJECT_EDIT,
	OBJECT_FBUTTON,
	OBJECT_FTEXT,
	OBJECT_LISTBOX,
	OBJECT_MENU,
	OBJECT_RADIO,
	OBJECT_TEXT,
// window objects
	OBJECT_WINDOW,
	OBJECT_DIALOG,
	OBJECT_MESSAGEBOX,
	OBJECT_DOWAIT,
	OBJECT_DISPLAY,
	OBJECT_DISPLAY_MULTIPLEXER,
	OBJECT_CPU_USAGE
};

struct GTimer
{
	static GTimer* base_timer;
	GObject* object;
	GId timer_id;
	unsigned int time_out;
	GTimer* next;

	GTimer(GId id, GObject* obj) :
			object(obj), timer_id(id), time_out(0), next(nullptr)
	{
		;
	}
	~GTimer();

	bool TimerProc(void);
	void RegisterTimer(void);
};

bool process_timers(void);

struct text_metrics_t
{
	union
	{
		struct
		{
			short int width, height;
		};
		int as_int;
	};
	text_metrics_t() :
			as_int(0)
	{
	}
	text_metrics_t(int val) :
			as_int(val)
	{
	}
	text_metrics_t(int sx, int sy) :
			width(sx), height(sy)
	{
	}
};

struct GObject
{
	RECT_T rect;
	RECT_T client_rect;
	GContainer* parent;
	GObject* nextObj;
	struct
	{
		GId id;
		GFlags flags;
		uint8_t ref_cnt;
		GFlags displays;
	}__attribute__((packed));

	static void* lastAllocated;
	static uint8_t	invalidate_cnt;

	GObject() :
			parent(nullptr), nextObj(nullptr), id(0), flags(0), ref_cnt(1), displays(1)
	{
		if(lastAllocated && lastAllocated == this)
		{
			ref_cnt = 0;
			lastAllocated = nullptr;
		}
	}
	GObject(GId id_t, const RECT_T& rect_t, GFlags flags_t = GO_FLG_DEFAULT) :
			rect(rect_t), parent(nullptr), nextObj(nullptr), id(id_t), flags(flags_t),
			ref_cnt(1), displays(1)
	{
		if(lastAllocated && lastAllocated == this)
		{
			ref_cnt = 0;
			lastAllocated = nullptr;
		}
	}

	virtual ~GObject();

	GUI_GET_OBJECT_TYPE(OBJECT_OBJECT);

	void* operator new(size_t size)
	{
		void *p;
		uint32_t tmp = CURRENT_TIME;

		// Do not return nullptr !!!
		while ((p = tsk_malloc(size)) == nullptr)
		{
			on_out_of_memory(size);
			tsk_sleep(1);
			if (seconds_since(tmp) > 15)
			{
				SYST->SYST_CSR = 0;
				LowLevelReboot();
			}
		}

		while(locked_set_if_null(&lastAllocated, p))
			tsk_sleep(1);

		return p;
	}

	void* operator new(size_t size, void *p )
	{
		return p;
	}

	// Timer methods
	bool SetTimer(GId event, unsigned int elapse);
	void KillTimer(GId event);
	bool IsActiveTimer(GId event);
	bool StopTimer(GId event);
	// size and position methods
	virtual void LPtoDP(POINT_T& size, unsigned char lcd_index =0);
	virtual void DPtoLP(POINT_T& size, unsigned char lcd_index =0);

	// Draw methods

	void clear_rect(const RECT_T& area);
	bool cut_hline(int& x0, int& x1, int& y) const;
	bool cut_vline(int& y0, int& y1, int& x) const;
	void set_xy_all(LCD_MODULE* lcd, unsigned int xy, unsigned int all) const;
	int  draw_text(LCD_MODULE* lcd, const char* txt);
	void draw_text_line(LCD_MODULE* lcd, const char* txt, unsigned int len);
	void draw_line(int x0, int y0, int x1, int y1);
	void draw_line(POINT_T a, POINT_T b){draw_line(a.x, a.y, b.x, b.y);}
	void draw_circle(int x0, int y0, int radius, int sectors = 0xFF);
	void draw_circle(POINT_T center, int r, int sectors = 0xFF)
		{draw_circle(center.x, center.y, r, sectors);}
	void draw_ellipse (int x0, int y0, int width, int height, int sectors = 0xFF);
	void draw_ellipse (POINT_T center, int width, int height, int sectors = 0xFF)
		{draw_ellipse(center.x, center.y, width, height, sectors);}
	void fill_circle(int x0, int y0, int radius);
	void fill_circle(POINT_T center, int r) {fill_circle(center.x, center.y, r);}
	void draw_poligon(const RECT_T& frame, bool fill = false);
	void draw_rectangle(const RECT_T& frame, bool fill = false);
	// Control methods
	virtual bool close();
	bool remove();
	virtual bool get_focus(bool notify_msg = true);	//sets the parent focus on this
	// Redraw methods
	virtual void draw(LCD_MODULE* lcd, RECT_T area);
	virtual void draw_this(LCD_MODULE* lcd) = 0;		// called to self redraw
	virtual void invalidate(GObject* object, RECT_T area);
	// virtual draw methods ( they are used from the module )
	virtual void allocate_border(void);
	virtual POINT_T get_border_size(void);
	virtual void draw_border(RECT_T& frame);

	virtual void draw_point(int x, int y);
	virtual void draw_hline(int x0, int x1, int y);
	virtual void draw_bline(int x0, int x1, int y);
	virtual void draw_vline(int y0, int y1, int x);
	virtual void invert_vline(int y0, int y1, int x);
	virtual void invert_hline(int x0, int x1, int y);
	virtual void draw_bitmap(int x0, int y0, const char* src, int width,
			int rows)
	{
		;
	}
	virtual int overlapped(GObject* obj,  RECT_T& frame);
	// queue message
	unsigned int message(GMessage& msg);

	virtual bool set_flag(GFlags val);
	virtual bool clr_flag(GFlags val);

	virtual GObject* get_object(GId xid);

	virtual bool is_available();
	virtual void move(int x, int y);

protected:
	friend struct GContainer;
	friend struct GFloating_Button;

	virtual unsigned int initialize(GMessage& msg);	//proceeds items with WM_INIT code
	virtual unsigned int process_key(GMessage& msg)	//proceeds items with WM_KEY code
	{
		return 0;
	}
	virtual unsigned int process_default(GMessage& msg);//proceeds items with unknown code
	virtual unsigned int process_command(GMessage& msg)	//proceeds items with WM_COMMAND code
	{
		return 0;
	}
	virtual unsigned int process_idle(GMessage& msg)//proceeds items with WM_IDLE code
	{
		return 0;
	}

	virtual unsigned int process_destroy(GMessage& msg); //The WM_DESTROY message is sent when a window is being destroyed.
	virtual void redraw_screen(GObject* object, RECT_T area);
private:
	// Timer methods
	GTimer* FindTimer(GId event);
	void KillObjectTimers(void) const;
};

// Draw utility
text_metrics_t get_text_metrics(const char* text, short int x_size,
		const RENDER_MODE* font);

#endif /* GOBJECT_H_ */
