#include <console.h>
#include <console_struct.h>
#include <console_macros.h>
#include <system.h>
#include <printk.h>
#include <types.h>
#include <vgacon.h>
#include <heapmngr.h>
const struct consw *conswitchp;

struct vc vc_cons [16];
static int printable;		/* Is console ready for printing? */
int fg_console;
int last_console;
int console_blanked;
int kmsg_redirect;
#define scr_writew(val, addr) (*(addr) = (val))
#define scr_readw(addr) (*(addr))
#define scr_memcpyw(d, s, c) memcpy(d, s, c)
#define scr_memmovew(d, s, c) memmove(d, s, c)
#define update_screen(x) redraw_screen(x, 0)
static int scrollback_delta;
static  void scr_memsetw(u16 *s, u16 c, unsigned int count)
{
	count /= 2;
	while (count--)
		scr_writew(c, s++);
}
/*
 * For each existing display, we have a pointer to console currently visible
 * on that display, allowing consoles other than fg_console to be refreshed
 * appropriately. Unless the low-level driver supplies its own display_fg
 * variable, we use this one for the "master display".
 */
unsigned char color_table[] = { 0, 4, 2, 6, 1, 5, 3, 7,
				       8,12,10,14, 9,13,11,15 }; 
 
static struct vc_data *master_display_fg;
#define IS_FG (currcons == fg_console)
#define IS_VISIBLE CON_IS_VISIBLE(vc_cons[currcons].d)


#define DO_UPDATE IS_VISIBLE


#define video_num_columns	(vc_cons[currcons].d->vc_cols)
#define video_num_lines		(vc_cons[currcons].d->vc_rows)
#define video_size_row		(vc_cons[currcons].d->vc_size_row)
#define can_do_color		(vc_cons[currcons].d->vc_can_do_color)
static int softcursor_original;
static void scrup(int currcons, unsigned int t, unsigned int b, int nr)
{
	unsigned short *d, *s;

	if (t+nr >= b)
		nr = b - t - 1;
	if (b > video_num_lines || t >= b || nr < 1)
		return;
	if (IS_VISIBLE && sw->con_scroll(vc_cons[currcons].d, t, b, SM_UP, nr))
		return;

	d = (unsigned short *) (origin+video_size_row*t);
	s = (unsigned short *) (origin+video_size_row*(t+nr));
	scr_memcpyw(d, s, (b-t-nr) * video_size_row);
	scr_memsetw(d + (b-t-nr) * video_num_columns, video_erase_char, video_size_row*nr);
}

 void lf(int currcons)
{
    	/* don't scroll if above bottom of scrolling region, or
	 * if below scrolling region
	 */
    	if (y+1 == bottom)
		scrup(currcons,top,bottom,1);
	else if (y < video_num_lines-1) {
	    	y++;
		pos += video_size_row;
	}
	need_wrap = 0;
}


static void add_softcursor(int currcons)
{
	int i = scr_readw((u16 *) pos);
	u32 type = cursor_type;

	if (! (type & 0x10)) return;
	if (softcursor_original != -1) return;
	softcursor_original = i;
	i |= ((type >> 8) & 0xff00 );
	i ^= ((type) & 0xff00 );
	if ((type & 0x20) && ((softcursor_original & 0x7000) == (i & 0x7000))) i ^= 0x7000;
	if ((type & 0x40) && ((i & 0x700) == ((i & 0x7000) >> 4))) i ^= 0x0700;
	scr_writew(i, (u16 *) pos);
	if (DO_UPDATE)
		sw->con_putc(vc_cons[currcons].d, i, y, x);
}

static void hide_cursor(int currcons)
{
	//if (currcons == sel_cons)
		//clear_selection();
	if (softcursor_original != -1) {
		scr_writew(softcursor_original,(u16 *) pos);
		if (DO_UPDATE)
			sw->con_putc(vc_cons[currcons].d, softcursor_original, y, x);
		softcursor_original = -1;
	}
	sw->con_cursor(vc_cons[currcons].d,CM_ERASE);
}

static void set_origin(int currcons)
{
	if (!IS_VISIBLE ||
	    !sw->con_set_origin ||
	    !sw->con_set_origin(vc_cons[currcons].d))
		origin = (unsigned long) screenbuf;
	visible_origin = origin;
	scr_end = origin + screenbuf_size;
	pos = origin + video_size_row*y + 2*x;
}

 void save_screen(int currcons)
{
	if (sw->con_save_screen)
		sw->con_save_screen(vc_cons[currcons].d);
}
 void set_cursor(int currcons)
{
    if (!IS_FG || console_blanked /*|| vcmode == KD_GRAPHICS */)
	return;
   // if (deccm) {
	//if (currcons == sel_cons)
		//clear_selection();
	add_softcursor(currcons);
	if ((cursor_type & 0x0f) != 1)
	    sw->con_cursor(vc_cons[currcons].d,CM_DRAW);
    /*} */ else
	hide_cursor(currcons);
}

static inline void cr(int currcons)
{
	pos -= x<<1;
	need_wrap = x = 0;
}

static inline void bs(int currcons)
{
	if (x) {
		pos -= 2;
		x--;
		need_wrap = 0;
	}
}
int vc_cons_allocated(unsigned int i)
{
	return (i < MAX_NR_CONSOLES && vc_cons[i].d);
}
/*
 * gotoxy() must verify all boundaries, because the arguments
 * might also be negative. If the given position is out of
 * bounds, the cursor is placed at the nearest margin.
 */
static void gotoxy(int currcons, int new_x, int new_y)
{
	int min_y, max_y;

	if (new_x < 0)
		x = 0;
	else
		if (new_x >= (int)video_num_columns)
			x = video_num_columns - 1;
		else
			x = new_x;
 	if (decom) {
		min_y = top;
		max_y = bottom;
	} else {
		min_y = 0;
		max_y = video_num_lines;
	}
	if (new_y < min_y)
		y = min_y;
	else if (new_y >= max_y)
		y = max_y - 1;
	else
		y = new_y;
	pos = origin + y*video_size_row + (x<<1);
	need_wrap = 0;
}
static void
scrdown(int currcons, unsigned int t, unsigned int b, int nr)
{
	unsigned short *s;
	unsigned int step;

	if (t+nr >= b)
		nr = b - t - 1;
	if (b > video_num_lines || t >= b || nr < 1)
		return;
	if (IS_VISIBLE && sw->con_scroll(vc_cons[currcons].d, t, b, SM_DOWN, nr))
		return;
	s = (unsigned short *) (origin+video_size_row*t);
	step = video_num_columns * nr;
	scr_memmovew(s + step, s, (b-t-nr)*video_size_row);
	scr_memsetw(s, video_erase_char, 2*step);
}
 void ri(int currcons)
{
    	/* don't scroll if below top of scrolling region, or
	 * if above scrolling region
	 */
	 
	if (y == top)
		scrdown(currcons,top,bottom,1);
	else if (y > 0) {
		y--;
		pos -= video_size_row;
	}
	need_wrap = 0;
}
static inline void scrolldelta(int lines)
{
	scrollback_delta += lines;
//	schedule_console_callback();
}
void scrollback(int lines)
{
	int currcons = fg_console;

	if (!lines)
		lines = video_num_lines/2;
	scrolldelta(-lines);
}

void scrollfront(int lines)
{
	int currcons = fg_console;

	if (!lines)
		lines = video_num_lines/2;
	scrolldelta(lines);
}
/*
 *	Console on virtual terminal
 *
 * The console must be locked when we get here.
 */

void vt_console_print(struct console *co, const char * b, unsigned count)
{
	int currcons = fg_console;
	unsigned char c;
//	static unsigned long printing;

	

	if (!vc_cons_allocated(currcons)) {
		/* impossible */
		 printk("vt_console_print: tty %d not allocated ??\n", currcons+1); 
	 }
//		goto quit;
	//}

	//if (vcmode != KD_TEXT)
		//goto quit;

	/* undraw cursor first */
	if (IS_FG)
		hide_cursor(currcons);

	

	/* Contrived structure to try to emulate original need_wrap behaviour
	 * Problems caused when we have need_wrap set on '\n' character */
	
	const unsigned short *start;
	unsigned short cnt = 0;
	unsigned int myx;
	start = (unsigned short *)pos;
	myx = x;
	while (count--) {
		c = *b++;
		if (c == '\n' || c == '\r' || c == 8 || need_wrap) {
			if (cnt > 0) {
				if (IS_VISIBLE)
					sw->con_putcs(vc_cons[currcons].d, start, cnt, y, x);
				x += cnt;
				if (need_wrap)
					x--;
				cnt = 0;
			}
			if (c == 8) {		/* backspace */
				bs(currcons);
				start = (unsigned short *)pos;
				myx = x;
				continue;
			}
			if (c != '\r')
				lf(currcons); 
			cr(currcons);
			start = (unsigned short *)pos;
			myx = x;
			if (c == '\n' || c == '\r')
				continue;				
		}
		scr_writew((7 << 8) + c, (unsigned short *) pos);
		cnt++;
		if (myx == video_num_columns - 1) {
			need_wrap = 1;
			continue;
		}
		pos+=2;
		myx++;
	}
if (cnt > 0) {
		if (IS_VISIBLE)
			sw->con_putcs(vc_cons[currcons].d, start, cnt, y, x);
		x += cnt;
		if (x == video_num_columns) {
			x--;
			need_wrap = 1;
		}	
	}

	set_cursor(currcons);

	//if (!oops_in_progress)
		//poke_blanked_console();

//quit:
	//clear_bit(0, &printing);
}

//static kdev_t vt_console_device(struct console *c)
//{
	//return MKDEV(TTY_MAJOR, c->index ? c->index : fg_console + 1);
//}

struct console vt_console_driver = {
	name:		"tty",
	write:		vt_console_print,
	device:		NULL, /* vt_console_device */
	wait_key:	NULL, /* keyboard_wait_for_keypress */
	unblank:	NULL, /*unblank_screen */
	flags:		CON_PRINTBUFFER,
	index:		-1,
};



static void visual_init(int currcons, int init)
{
    /* ++Geert: sw->con_init determines console size */
    sw = conswitchp;

    cons_num = currcons;
    display_fg = &master_display_fg;
    vc_cons[currcons].d->vc_uni_pagedir_loc = &vc_cons[currcons].d->vc_uni_pagedir;
    vc_cons[currcons].d->vc_uni_pagedir = 0;
    hi_font_mask = 0;
    complement_mask = 0;
    can_do_color = 0;
    sw->con_init(vc_cons[currcons].d, init);
    if (!complement_mask)
        complement_mask = can_do_color ? 0x7700 : 0x0800;
    s_complement_mask = complement_mask;
    video_size_row = video_num_columns<<1;
    screenbuf_size = video_num_lines*video_size_row;
    
    bottom = 25;
    top = 0;
}
static void csi_J(int currcons, int vpar)
{
	unsigned int count;
	unsigned short * start;

	switch (vpar) {
		case 0:	/* erase from cursor to end of display */
			count = (scr_end-pos)>>1;
			start = (unsigned short *) pos;
			if (DO_UPDATE) {
				/* do in two stages */
				sw->con_clear(vc_cons[currcons].d, y, x, 1,
					      video_num_columns-x);
				sw->con_clear(vc_cons[currcons].d, y+1, 0,
					      video_num_lines-y-1,
					      video_num_columns);
			}
			break;
		case 1:	/* erase from start to cursor */
			count = ((pos-origin)>>1)+1;
			start = (unsigned short *) origin;
			if (DO_UPDATE) {
				/* do in two stages */
				sw->con_clear(vc_cons[currcons].d, 0, 0, y,
					      video_num_columns);
				sw->con_clear(vc_cons[currcons].d, y, 0, 1,
					      x + 1);
			}
			break;
		case 2: /* erase whole display */
			count = video_num_columns * video_num_lines;
			start = (unsigned short *) origin;
			if (DO_UPDATE)
				sw->con_clear(vc_cons[currcons].d, 0, 0,
					      video_num_lines,
					      video_num_columns);
			break;
		default:
			return;
	}
	scr_memsetw(start, video_erase_char, 2*count);
	need_wrap = 0;
}
/*
 *	Palettes
 */

void set_palette(int currcons)
{
	//if (vcmode != KD_GRAPHICS)
		sw->con_set_palette(vc_cons[currcons].d, color_table);
}

static void do_update_region(int currcons, unsigned long start, int count)
{

	unsigned int xx, yy, offset;
	u16 *p;

	p = (u16 *) start;
	if (!sw->con_getxy) {
		offset = (start - origin) / 2;
		xx = offset % video_num_columns;
		yy = offset / video_num_columns;
	} else {
		int nxx, nyy;
		start = sw->con_getxy(vc_cons[currcons].d, start, &nxx, &nyy);
		xx = nxx; yy = nyy;
	}
	for(;;) {
		u16 attrib = scr_readw(p) & 0xff00;
		int startx = xx;
		u16 *q = p;
		while (xx < video_num_columns && count) {
			if (attrib != (scr_readw(p) & 0xff00)) {
				if (p > q)
					sw->con_putcs(vc_cons[currcons].d, q, p-q, yy, startx);
				startx = xx;
				q = p;
				attrib = scr_readw(p) & 0xff00;
			}
			p++;
			xx++;
			count--;
		}
		if (p > q)
			sw->con_putcs(vc_cons[currcons].d, q, p-q, yy, startx);
		if (!count)
			break;
		xx = 0;
		yy++;
		if (sw->con_getxy) {
			p = (u16 *)start;
			start = sw->con_getxy(vc_cons[currcons].d, start, NULL, NULL);
		}
	}

}
/*
 *	Redrawing of screen
 */

void redraw_screen(int new_console, int is_switch)
{
	int redraw = 1;
	int currcons, old_console;

	if (!vc_cons_allocated(new_console)) {
		/* strange ... */
		printk("redraw_screen: tty %d not allocated ??\n", new_console+1); 
		return;
	}

	if (is_switch) {
		currcons = fg_console;
		hide_cursor(currcons);
		if (fg_console != new_console) {
			struct vc_data **display = vc_cons[new_console].d->vc_display_fg;
			old_console = (*display) ? (*display)->vc_num : fg_console;
			*display = vc_cons[new_console].d;
			fg_console = new_console;
			currcons = old_console;
			if (!IS_VISIBLE) {
				save_screen(currcons);
				set_origin(currcons);
			}
			currcons = new_console;
			if (old_console == new_console)
				redraw = 0;
		}
	} else {
		currcons = new_console;
		hide_cursor(currcons);
	}

	if (redraw) {
		int update;
		set_origin(currcons);
		update = sw->con_switch(vc_cons[currcons].d);
		set_palette(currcons);
		if (update)
			do_update_region(currcons, origin, screenbuf_size/2);
	}
	set_cursor(currcons);
	//if (is_switch) {
	//	set_leds();
	//	compute_shiftstate();
	//}
}

/* the default colour table, for VGA+ colour systems */
int default_red[] = {0x00,0xaa,0x00,0xaa,0x00,0xaa,0x00,0xaa,
    0x55,0xff,0x55,0xff,0x55,0xff,0x55,0xff};
int default_grn[] = {0x00,0x00,0xaa,0x55,0x00,0x00,0xaa,0xaa,
    0x55,0x55,0xff,0xff,0x55,0x55,0xff,0xff};
int default_blu[] = {0x00,0x00,0x00,0x00,0xaa,0xaa,0xaa,0xaa,
    0x55,0x55,0x55,0x55,0xff,0xff,0xff,0xff};

static void vc_init(unsigned int currcons, unsigned int rows, unsigned int cols, int do_clear)
{
	int j, k ;

	video_num_columns = cols;
	video_num_lines = rows;
	video_size_row = cols<<1;
	screenbuf_size = video_num_lines * video_size_row;

	set_origin(currcons);
	pos = origin;
	//reset_vc(currcons);
	for (j=k=0; j<16; j++) {
		vc_cons[currcons].d->vc_palette[k++] = default_red[j] ;
		vc_cons[currcons].d->vc_palette[k++] = default_grn[j] ;
		vc_cons[currcons].d->vc_palette[k++] = default_blu[j] ;
	}
	def_color       = 0x07;   /* white */
	ulcolor		= 0x0f;   /* bold white */
	halfcolor       = 0x08;   /* grey */
	//init_waitqueue_head(&vt_cons[currcons]->paste_wait);
	//reset_terminal(currcons, do_clear);
}


void con_init(void)
{
	
	
	conswitchp = &vga_con; //VGA

	const char *display_desc = NULL;
	unsigned int currcons = 0;

	if (conswitchp)		//VGA
		display_desc = conswitchp->con_startup();
	/*
	 * kmalloc is not running yet - we use the bootmem allocator.
	 */
	for (currcons = 0; currcons < MIN_NR_CONSOLES; currcons++) {
		vc_cons[currcons].d = (struct vc_data *)
				malloc_(sizeof(struct vc_data)); //alloc_bootmem
	//	vt_cons[currcons] = (struct vt_struct *)
		//		alloc_bootmem(sizeof(struct vt_struct)); //kan kanske använda "min" kmalloc istället
	visual_init(currcons, 1);
	screenbuf = (unsigned short *) malloc_(screenbuf_size); //alloc_bootmem
	kmalloced = 0;
	vc_init(currcons, video_num_lines, video_num_columns, 
			currcons || !sw->con_save_screen);
	}
	currcons = fg_console = 0;
	master_display_fg = vc_cons[currcons].d;
	set_origin(currcons);
	save_screen(currcons);
	gotoxy(currcons,x,y);
	csi_J(currcons, 0);
	update_screen(fg_console);

	printable = 1;
	//printk("\n");
//	conswitchp->con_clear(vc_cons[0].d ,0,0,1,80*25*2);
//	conswitchp->con_putcs(vc_cons[1].d ,"test",5,24, 50);
	
	
	register_console(&vt_console_driver);


	//vt_console_print(vc_cons[1].d ,"eklund\n",7);
	printk("\n screenbuf_size : %d\n ", screenbuf_size);


	printk("\n Console : %s \n%s         %dx%d     ",
		can_do_color ? "colour" : "mono",
		display_desc, video_num_columns, video_num_lines);
		//unregister_console(&vt_console_driver);
		save_screen(currcons);
		

}


