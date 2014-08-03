#include <console.h>
#include <console_struct.h>
#include <console_macros.h>
#include <io.h>
#include <system.h>
#define scr_writew(val, addr) (*(addr) = (val))
#define scr_readw(addr) (*(addr))
#define scr_memcpyw(d, s, c) memcpy(d, s, c)
#define scr_memmovew(d, s, c) memmove(d, s, c)

static void scr_memsetw(u16 *s, u16 c, unsigned int count)
{
	count /= 2;
	while (count--)
		scr_writew(c, s++);
}
#define BLANK 0x0020
#define VIDEO_TYPE_VGAC		0x22
#define VIDEO_TYPE_EGAM		0x20
#define CAN_LOAD_EGA_FONTS	/* undefine if the user must not do this */
#define CAN_LOAD_PALETTE	/* undefine if the user must not do this */

/* You really do _NOT_ want to define this, unless you have buggy
 * Trident VGA which will resize cursor when moving it between column
 * 15 & 16. If you define this and your VGA is OK, inverse bug will
 * appear.
 */
#undef TRIDENT_GLITCH

#define dac_reg		0x3c8
#define dac_val		0x3c9
#define attrib_port	0x3c0
#define seq_port_reg	0x3c4
#define seq_port_val	0x3c5
#define gr_port_reg	0x3ce
#define gr_port_val	0x3cf
#define video_misc_rd	0x3cc
#define video_misc_wr	0x3c2

/*
 *  Interface used by the world
 */

static const char *vgacon_startup(void);
static void vgacon_init(struct vc_data *c, int init);
static void vgacon_deinit(struct vc_data *c);
static void vgacon_cursor(struct vc_data *c, int mode);
static int vgacon_switch(struct vc_data *c);
static int vgacon_blank(struct vc_data *c, int blank);
static int vgacon_font_op(struct vc_data *c, struct console_font_op *op);
static int vgacon_set_palette(struct vc_data *c, unsigned char *table);
static int vgacon_scrolldelta(struct vc_data *c, int lines);
static int vgacon_set_origin(struct vc_data *c);
static void vgacon_save_screen(struct vc_data *c);
static int vgacon_scroll(struct vc_data *c, int t, int b, int dir, int lines);
//static unsigned char vgacon_build_attr(struct vc_data *c, unsigned char color, unsigned char intensity, unsigned char blink, unsigned char underline, unsigned char reverse);
static void vgacon_invert_region(struct vc_data *c, u16 *p, int count);

unsigned int video_scan_lines;

/* Description of the hardware situation */
static unsigned long   vga_vram_base;		/* Base of video memory */
static unsigned long   vga_vram_end;		/* End of video memory */
static u16             vga_video_port_reg;	/* Video register select port */
static u16             vga_video_port_val;	/* Video register value port */
int    vga_video_num_columns;	/* Number of text columns */
static unsigned int    vga_video_num_lines;	/* Number of text lines */
static unsigned long   vga_video_size_row;
static int	       vga_can_do_color = 0;	/* Do we support colors? */

static unsigned char   vga_video_type;		/* Card type */
static unsigned char   vga_hardscroll_enabled;
unsigned int video_font_height;
#ifdef CONFIG_IA64_SOFTSDV_HACKS
/*
 * SoftSDV doesn't have hardware assist VGA scrolling 
 */
static unsigned char   vga_hardscroll_user_enable = 0;
#else
static unsigned char   vga_hardscroll_user_enable = 1;
#endif

static int	       vga_vesa_blanked;
static int	       vga_palette_blanked;
static int	       vga_is_gfx;

static int	       vga_video_font_height;
static unsigned int    vga_rolled_over = 0;


static int no_scroll(char *str)
{
	/*
	 * Disabling scrollback is required for the Braillex ib80-piezo
	 * Braille reader made by F.H. Papenmeier (Germany).
	 * Use the "no-scroll" bootflag.
	 */
	vga_hardscroll_user_enable = vga_hardscroll_enabled = 0;
	return 1;
}



/*
 * By replacing the four outb_p with two back to back outw, we can reduce
 * the window of opportunity to see text mislocated to the RHS of the
 * console during heavy scrolling activity. However there is the remote
 * possibility that some pre-dinosaur hardware won't like the back to back
 * I/O. Since the Xservers get away with it, we should be able to as well.
 */
static inline void write_vga(unsigned char reg, unsigned int val)
{
	unsigned int v1, v2;



#ifndef SLOW_VGA
	v1 = reg + (val & 0xff00);
	v2 = reg + 1 + ((val << 8) & 0xff00);
	outw(v1, vga_video_port_reg);
	outw(v2, vga_video_port_reg);
#else
	outb_p(reg, vga_video_port_reg);
	outb_p(val >> 8, vga_video_port_val);
	outb_p(reg+1, vga_video_port_reg);
	outb_p(val & 0xff, vga_video_port_val);
#endif

}

typedef struct ringbuffer_con {
 char con_data[80*25*2*16];
 volatile u16 *bufread;
 volatile u16 *bufptr; 
 volatile u32 counter; 
} ringbuffer_con;

volatile ringbuffer_con *conbuffer;


static const char  *vgacon_startup(void)
{
	const char *display_desc = NULL;
	
	


	vga_video_num_lines = 25;
	vga_video_num_columns = 80;
	vga_video_size_row = 2 * vga_video_num_columns;
	
		vga_can_do_color = 1;
		vga_vram_base = 0xb8000;
		vga_video_port_reg = 0x3d4;
		vga_video_port_val = 0x3d5;
		video_font_height = 16;
		vga_video_font_height = 16;
		
	//	conbuffer = (ringbuffer_con*)malloc_(sizeof(ringbuffer_con));
	

	video_scan_lines =
			vga_video_font_height * vga_video_num_lines;
	return display_desc;
}

static void vgacon_init(struct vc_data *c, int init)
{

	c->vc_cols = vga_video_num_columns;
	c->vc_rows = vga_video_num_lines;
}

static inline void vga_set_mem_top(struct vc_data *c)
{
	write_vga(12, (c->vc_visible_origin-vga_vram_base)/2);
}

static void vgacon_deinit(struct vc_data *c)
{
	/* When closing the last console, reset video origin */
	/*if (!--vgacon_uni_pagedir[1]) {
		c->vc_visible_origin = vga_vram_base;
		vga_set_mem_top(c);
		con_free_unimap(c->vc_num);
	}
	c->vc_uni_pagedir_loc = &c->vc_uni_pagedir;
	con_set_default_unimap(c->vc_num); */
}

static void vgacon_invert_region(struct vc_data *c, u16 *p, int count)
{
	int col = vga_can_do_color;

	while (count--) {
		u16 a = scr_readw(p);
		if (col)
			a = ((a) & 0x88ff) | (((a) & 0x7000) >> 4) | (((a) & 0x0700) << 4);
		else
			a ^= ((a & 0x0700) == 0x0100) ? 0x7000 : 0x7700;
		scr_writew(a, p++);
	}
}

static void vgacon_set_cursor_size(int xpos, int from, int to)
{

	int curs, cure;
	static int lastfrom, lastto;

#ifdef TRIDENT_GLITCH
	if (xpos<16) from--, to--;
#endif

	if ((from == lastfrom) && (to == lastto)) return;
	lastfrom = from; lastto = to;

	//spin_lock_irqsave(&vga_lock, flags);
	outb_p(0x0a, vga_video_port_reg);		/* Cursor start */
	curs = inb_p(vga_video_port_val);
	outb_p(0x0b, vga_video_port_reg);		/* Cursor end */
	cure = inb_p(vga_video_port_val);

	curs = (curs & 0xc0) | from;
	cure = (cure & 0xe0) | to;

	outb_p(0x0a, vga_video_port_reg);		/* Cursor start */
	outb_p(curs, vga_video_port_val);
	outb_p(0x0b, vga_video_port_reg);		/* Cursor end */
	outb_p(cure, vga_video_port_val);
	//spin_unlock_irqrestore(&vga_lock, flags);
}

static void vgacon_cursor(struct vc_data *c, int mode)
{
    if (c->vc_origin != c->vc_visible_origin)
	vgacon_scrolldelta(c, 0);
    switch (mode) {
	case CM_ERASE:
	    write_vga(14, (vga_vram_end - vga_vram_base - 1)/2);
	    break;

	case CM_MOVE:
	case CM_DRAW:
	    write_vga(14, (c->vc_pos-vga_vram_base)/2);
	    switch (c->vc_cursor_type & 0x0f) {
		case CUR_UNDERLINE:
			vgacon_set_cursor_size(c->vc_x, 
					video_font_height - (video_font_height < 10 ? 2 : 3),
					video_font_height - (video_font_height < 10 ? 1 : 2));
			break;
		case CUR_TWO_THIRDS:
			vgacon_set_cursor_size(c->vc_x, 
					 video_font_height / 3,
					 video_font_height - (video_font_height < 10 ? 1 : 2));
			break;
		case CUR_LOWER_THIRD:
			vgacon_set_cursor_size(c->vc_x, 
					 (video_font_height*2) / 3,
					 video_font_height - (video_font_height < 10 ? 1 : 2));
			break;
		case CUR_LOWER_HALF:
			vgacon_set_cursor_size(c->vc_x, 
					 video_font_height / 2,
					 video_font_height - (video_font_height < 10 ? 1 : 2));
			break;
		case CUR_NONE:
			vgacon_set_cursor_size(c->vc_x, 31, 30);
			break;
          	default:
			vgacon_set_cursor_size(c->vc_x, 1, video_font_height);
			break;
		}
	    break;
    }
}

	int nr = 1;
int __cnter = 0;

static int vgacon_switch(struct vc_data *c)
{
	/*
	 * We need to save screen size here as it's the only way
	 * we can spot the screen has been resized and we need to
	 * set size of freshly allocated screens ourselves.
	 */


	
	vga_video_num_columns = c->vc_cols;
	vga_video_num_lines = c->vc_rows;

	

	return 0;	/* Redrawing not needed */
}


static void vgacon_save_screen(struct vc_data *c)
{
	

	//if (!vga_bootup_console) {
		/* This is a gross hack, but here is the only place we can
		 * set bootup console parameters without messing up generic
		 * console initialization routines.
		 */
	//	vga_bootup_console = 1;

	//}
	
}


static void vga_set_palette(struct vc_data *c, unsigned char *table)
{
	int i, j ;

	for (i=j=0; i<16; i++) {
		outb_p (table[i], dac_reg) ;
		outb_p (c->vc_palette[j++]>>2, dac_val) ;
		outb_p (c->vc_palette[j++]>>2, dac_val) ;
		outb_p (c->vc_palette[j++]>>2, dac_val) ;
	}
}

static int vgacon_set_palette(struct vc_data *c, unsigned char *table)
{
#ifdef CAN_LOAD_PALETTE
	if (vga_video_type != VIDEO_TYPE_VGAC || vga_palette_blanked || !CON_IS_VISIBLE(c))
		return -EINVAL;
	vga_set_palette(c, table);
	return 0;
#else
	return -EINVAL;
#endif
}

/* structure holding original VGA register settings */
static struct {
	unsigned char	SeqCtrlIndex;		/* Sequencer Index reg.   */
	unsigned char	CrtCtrlIndex;		/* CRT-Contr. Index reg.  */
	unsigned char	CrtMiscIO;		/* Miscellaneous register */
	unsigned char	HorizontalTotal;	/* CRT-Controller:00h */
	unsigned char	HorizDisplayEnd;	/* CRT-Controller:01h */
	unsigned char	StartHorizRetrace;	/* CRT-Controller:04h */
	unsigned char	EndHorizRetrace;	/* CRT-Controller:05h */
	unsigned char	Overflow;		/* CRT-Controller:07h */
	unsigned char	StartVertRetrace;	/* CRT-Controller:10h */
	unsigned char	EndVertRetrace;		/* CRT-Controller:11h */
	unsigned char	ModeControl;		/* CRT-Controller:17h */
	unsigned char	ClockingMode;		/* Seq-Controller:01h */
} vga_state;

static void vga_vesa_blank(int mode)
{
	/* save original values of VGA controller registers */
	if(!vga_vesa_blanked) {
		//spin_lock_irq(&vga_lock);
		vga_state.SeqCtrlIndex = inb_p(seq_port_reg);
		vga_state.CrtCtrlIndex = inb_p(vga_video_port_reg);
		vga_state.CrtMiscIO = inb_p(video_misc_rd);
		//spin_unlock_irq(&vga_lock);

		outb_p(0x00,vga_video_port_reg);	/* HorizontalTotal */
		vga_state.HorizontalTotal = inb_p(vga_video_port_val);
		outb_p(0x01,vga_video_port_reg);	/* HorizDisplayEnd */
		vga_state.HorizDisplayEnd = inb_p(vga_video_port_val);
		outb_p(0x04,vga_video_port_reg);	/* StartHorizRetrace */
		vga_state.StartHorizRetrace = inb_p(vga_video_port_val);
		outb_p(0x05,vga_video_port_reg);	/* EndHorizRetrace */
		vga_state.EndHorizRetrace = inb_p(vga_video_port_val);
		outb_p(0x07,vga_video_port_reg);	/* Overflow */
		vga_state.Overflow = inb_p(vga_video_port_val);
		outb_p(0x10,vga_video_port_reg);	/* StartVertRetrace */
		vga_state.StartVertRetrace = inb_p(vga_video_port_val);
		outb_p(0x11,vga_video_port_reg);	/* EndVertRetrace */
		vga_state.EndVertRetrace = inb_p(vga_video_port_val);
		outb_p(0x17,vga_video_port_reg);	/* ModeControl */
		vga_state.ModeControl = inb_p(vga_video_port_val);
		outb_p(0x01,seq_port_reg);		/* ClockingMode */
		vga_state.ClockingMode = inb_p(seq_port_val);
	}

	/* assure that video is enabled */
	/* "0x20" is VIDEO_ENABLE_bit in register 01 of sequencer */
	//spin_lock_irq(&vga_lock);
	outb_p(0x01,seq_port_reg);
	outb_p(vga_state.ClockingMode | 0x20,seq_port_val);

	/* test for vertical retrace in process.... */
	if ((vga_state.CrtMiscIO & 0x80) == 0x80)
		outb_p(vga_state.CrtMiscIO & 0xef,video_misc_wr);

	/*
	 * Set <End of vertical retrace> to minimum (0) and
	 * <Start of vertical Retrace> to maximum (incl. overflow)
	 * Result: turn off vertical sync (VSync) pulse.
	 */
	if (mode & VESA_VSYNC_SUSPEND) {
		outb_p(0x10,vga_video_port_reg);	/* StartVertRetrace */
		outb_p(0xff,vga_video_port_val); 	/* maximum value */
		outb_p(0x11,vga_video_port_reg);	/* EndVertRetrace */
		outb_p(0x40,vga_video_port_val);	/* minimum (bits 0..3)  */
		outb_p(0x07,vga_video_port_reg);	/* Overflow */
		outb_p(vga_state.Overflow | 0x84,vga_video_port_val); /* bits 9,10 of vert. retrace */
	}

	if (mode & VESA_HSYNC_SUSPEND) {
		/*
		 * Set <End of horizontal retrace> to minimum (0) and
		 *  <Start of horizontal Retrace> to maximum
		 * Result: turn off horizontal sync (HSync) pulse.
		 */
		outb_p(0x04,vga_video_port_reg);	/* StartHorizRetrace */
		outb_p(0xff,vga_video_port_val);	/* maximum */
		outb_p(0x05,vga_video_port_reg);	/* EndHorizRetrace */
		outb_p(0x00,vga_video_port_val);	/* minimum (0) */
	}

	/* restore both index registers */
	outb_p(vga_state.SeqCtrlIndex,seq_port_reg);
	outb_p(vga_state.CrtCtrlIndex,vga_video_port_reg);
	//spin_unlock_irq(&vga_lock);
}

static void vga_vesa_unblank(void)
{
	/* restore original values of VGA controller registers */
	//spin_lock_irq(&vga_lock);
	outb_p(vga_state.CrtMiscIO,video_misc_wr);

	outb_p(0x00,vga_video_port_reg);		/* HorizontalTotal */
	outb_p(vga_state.HorizontalTotal,vga_video_port_val);
	outb_p(0x01,vga_video_port_reg);		/* HorizDisplayEnd */
	outb_p(vga_state.HorizDisplayEnd,vga_video_port_val);
	outb_p(0x04,vga_video_port_reg);		/* StartHorizRetrace */
	outb_p(vga_state.StartHorizRetrace,vga_video_port_val);
	outb_p(0x05,vga_video_port_reg);		/* EndHorizRetrace */
	outb_p(vga_state.EndHorizRetrace,vga_video_port_val);
	outb_p(0x07,vga_video_port_reg);		/* Overflow */
	outb_p(vga_state.Overflow,vga_video_port_val);
	outb_p(0x10,vga_video_port_reg);		/* StartVertRetrace */
	outb_p(vga_state.StartVertRetrace,vga_video_port_val);
	outb_p(0x11,vga_video_port_reg);		/* EndVertRetrace */
	outb_p(vga_state.EndVertRetrace,vga_video_port_val);
	outb_p(0x17,vga_video_port_reg);		/* ModeControl */
	outb_p(vga_state.ModeControl,vga_video_port_val);
	outb_p(0x01,seq_port_reg);		/* ClockingMode */
	outb_p(vga_state.ClockingMode,seq_port_val);

	/* restore index/control registers */
	outb_p(vga_state.SeqCtrlIndex,seq_port_reg);
	outb_p(vga_state.CrtCtrlIndex,vga_video_port_reg);
	//spin_unlock_irq(&vga_lock);
}

static void vga_pal_blank(void)
{
	int i;

	for (i=0; i<16; i++) {
		outb_p (i, dac_reg) ;
		outb_p (0, dac_val) ;
		outb_p (0, dac_val) ;
		outb_p (0, dac_val) ;
	}
}

static int vgacon_blank(struct vc_data *c, int blank)
{
	switch (blank) {
	case 0:				/* Unblank */
		if (vga_vesa_blanked) {
			vga_vesa_unblank();
			vga_vesa_blanked = 0;
		}
		if (vga_palette_blanked) {
			vga_set_palette(c, color_table);
			vga_palette_blanked = 0;
			return 0;
		}
		vga_is_gfx = 0;
		/* Tell console.c that it has to restore the screen itself */
		return 1;
	case 1:				/* Normal blanking */
		if (vga_video_type == VIDEO_TYPE_VGAC) {
			vga_pal_blank();
			vga_palette_blanked = 1;
			return 0;
		}
		vgacon_set_origin(c);
		scr_memsetw((void *)vga_vram_base, BLANK, c->vc_screenbuf_size);
		return 1;
	case -1:			/* Entering graphic mode */
		scr_memsetw((void *)vga_vram_base, BLANK, c->vc_screenbuf_size);
		vga_is_gfx = 1;
		return 1;
	default:			/* VESA blanking */
		if (vga_video_type == VIDEO_TYPE_VGAC) {
			vga_vesa_blank(blank-1);
			vga_vesa_blanked = blank;
		}
		return 0;
	}
}

/*
 * PIO_FONT support.
 *
 * The font loading code goes back to the codepage package by
 * Joel Hoffman (joel@wam.umd.edu). (He reports that the original
 * reference is: "From: p. 307 of _Programmer's Guide to PC & PS/2
 * Video Systems_ by Richard Wilton. 1987.  Microsoft Press".)
 *
 * Change for certain monochrome monitors by Yury Shevchuck
 * (sizif@botik.yaroslavl.su).
 */

#ifdef CAN_LOAD_EGA_FONTS

#define colourmap 0xa0000
/* Pauline Middelink <middelin@polyware.iaf.nl> reports that we
   should use 0xA0000 for the bwmap as well.. */
#define blackwmap 0xa0000
#define cmapsz 8192

#define KD_FONT_OP_SET		0	/* Set font */
#define KD_FONT_FLAG_DONT_RECALC 	1
#define KD_FONT_OP_GET		1	

#else

static int vgacon_font_op(struct vc_data *c, struct console_font_op *op)
{
	return -ENOSYS;
}

#endif

static int vgacon_scrolldelta(struct vc_data *c, int lines)
{
	if (!lines)			/* Turn scrollback off */
		c->vc_visible_origin = c->vc_origin;
	else {

		int vram_size = vga_vram_end - vga_vram_base;
		int margin = c->vc_size_row * 4;
		int ul, we, p, st;

		if (vga_rolled_over > (c->vc_scr_end - vga_vram_base) + margin) {
			ul = c->vc_scr_end - vga_vram_base;
			we = vga_rolled_over + c->vc_size_row;
		} else {
			ul = 0;
			we = vram_size;
		}
		p = (c->vc_visible_origin - vga_vram_base - ul + we) % we + lines * c->vc_size_row;
		st = (c->vc_origin - vga_vram_base - ul + we) % we;
		if (p < margin)
			p = 0;
		if (p > st - margin)
			p = st;
		c->vc_visible_origin = vga_vram_base + (p + ul) % we;
	}
	vga_set_mem_top(c);
	return 1;
}

static int vgacon_set_origin(struct vc_data *c)
{
	if (vga_is_gfx ||	/* We don't play origin tricks in graphic modes */
	    (console_blanked && !vga_palette_blanked))	/* Nor we write to blanked screens */
		return 0;
	c->vc_origin = c->vc_visible_origin = vga_vram_base;
	vga_set_mem_top(c);
	vga_rolled_over = 0;
	return 1;
}








/* dir: do we want to scroll up or down? */
static int vgacon_scroll(struct vc_data *c, int t, int b, int dir, int lines)
{
	unsigned long oldo;
	unsigned int delta;
	
	if (t || b != (int)c->vc_rows || vga_is_gfx)
		return 0;

	if (c->vc_origin != c->vc_visible_origin)
		vgacon_scrolldelta(c, 0);

	if (!vga_hardscroll_enabled || lines >= (int)c->vc_rows/2)
		return 0;

	oldo = c->vc_origin;
	delta = lines * c->vc_size_row;
	if (dir == SM_UP) {
		if (c->vc_scr_end + delta >= vga_vram_end) {
			scr_memcpyw((u16 *)vga_vram_base,
				    (u16 *)(oldo + delta),
				    c->vc_screenbuf_size - delta);
			c->vc_origin = vga_vram_base;
			vga_rolled_over = oldo - vga_vram_base;
		} else
			c->vc_origin += delta;
		scr_memsetw((u16 *)(c->vc_origin + c->vc_screenbuf_size - delta), c->vc_video_erase_char, delta);
	} else {
		if (oldo - delta < vga_vram_base) {
			scr_memmovew((u16 *)(vga_vram_end - c->vc_screenbuf_size + delta),
				     (u16 *)oldo,
				     c->vc_screenbuf_size - delta);
			c->vc_origin = vga_vram_end - c->vc_screenbuf_size;
			vga_rolled_over = 0;
		} else
			c->vc_origin -= delta;
		c->vc_scr_end = c->vc_origin + c->vc_screenbuf_size;
		scr_memsetw((u16 *)(c->vc_origin), c->vc_video_erase_char, delta);
	}
	c->vc_scr_end = c->vc_origin + c->vc_screenbuf_size;
	c->vc_visible_origin = c->vc_origin;
	vga_set_mem_top(c);
	c->vc_pos = (c->vc_pos - oldo) + c->vc_origin;
	return 1;
}


/*
 *  The console `switch' structure for the VGA based console
 */

static int vgacon_dummy(struct vc_data *c)
{
	return 0;
}
void vgacon_clear(struct vc_data *conp, int sy, int sx, int height,
			      int width)
{
    int rows;
    unsigned long dest;

 

    dest = vga_vram_base + sy*vga_video_size_row + sx*2;
    if (sx == 0 && width == 80)      
	scr_memsetw((void *)dest, conp->vc_video_erase_char, height * width);
    else
        for (rows = height; rows-- ; dest += vga_video_size_row)
	    scr_memsetw((void *)dest, conp->vc_video_erase_char, width);

}


void vgacon_putc(struct vc_data *conp, int c, int ypos, int xpos)
{
    unsigned short *p;



    p = (unsigned short *)(vga_vram_base+ypos*vga_video_size_row+xpos*2);
    scr_writew(7 << 8 | c, p);
  
}

void vgacon_putcs(struct vc_data *conp, const unsigned short *s, int count,
		       int ypos, int xpos)
{
    unsigned short *p;
    unsigned short sattr;



    p = (unsigned short *)(vga_vram_base+ypos*vga_video_size_row+xpos*2);
    sattr = 7 << 8;
    while (count--)
	scr_writew(sattr | *s++, p++);

}

void vgacon_bmove(struct vc_data *conp, int sy, int sx, int dy, int dx,
		       int height, int width)
{
    unsigned long src, dst;
    int rows;



    if (sx == 0 && dx == 0 && width == 80) {
	src = vga_vram_base + sy * vga_video_size_row;
	dst = vga_vram_base + dy * vga_video_size_row;
	scr_memmovew((unsigned short *)dst, (unsigned short *)src,
		     height * width);
    } else if (dy < sy || (dy == sy && dx < sx)) {
	src = vga_vram_base + sy * vga_video_size_row + sx * 2;
	dst = vga_vram_base + dy * vga_video_size_row + dx * 2;
	for (rows = height; rows-- ;) {
	    scr_memmovew((unsigned short *)dst, (unsigned short *)src, width);
	    src += vga_video_size_row;
	    dst += vga_video_size_row;
	}
    } else {
	src = vga_vram_base + (sy+height-1) * vga_video_size_row + sx * 2;
	dst = vga_vram_base + (dy+height-1) * vga_video_size_row + dx * 2;
	for (rows = height; rows-- ;) {
	    scr_memmovew((unsigned short *)dst, (unsigned short *)src, width);
	    src -= vga_video_size_row;
	    dst -= vga_video_size_row;
	}
    }

}
#define DUMMY (void *) vgacon_dummy

const struct consw vga_con = {
	con_startup:		vgacon_startup,
	con_init:		vgacon_init,
	con_deinit:		vgacon_deinit,
	con_clear:		vgacon_clear,
	con_putc:		vgacon_putc,
	con_putcs:		vgacon_putcs,
	con_cursor:		vgacon_cursor,
	con_scroll:		vgacon_scroll,
	con_bmove:		vgacon_bmove,
	con_switch:		vgacon_switch,
	con_blank:		vgacon_blank,
	con_font_op:		DUMMY, /* vgacon_font_op */
	con_set_palette:	vgacon_set_palette,
	con_scrolldelta:	vgacon_scrolldelta,
	con_set_origin:		vgacon_set_origin,
	con_save_screen:	vgacon_save_screen,
	con_build_attr:		DUMMY, /* vgacon_build_attr */
	con_invert_region:	vgacon_invert_region,
};

