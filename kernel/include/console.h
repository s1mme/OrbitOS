/*
 *  linux/include/linux/console.h
 *
 *  Copyright (C) 1993        Hamish Macdonald
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Changed:
 * 10-Mar-94: Arno Griffioen: Conversion for vt100 emulator port from PC LINUX
 */

#ifndef _LINUX_CONSOLE_H_
#define _LINUX_CONSOLE_H_ 
#include <types.h>
#include <console_struct.h>
extern int console_blanked;
extern unsigned char color_table[];

#define ORIG_X			(screen_info.orig_x)

#define MAX_NR_CONSOLES	63
#define MIN_NR_CONSOLES 1 

#define	KERN_EMERG	"<0>"	/* system is unusable			*/
#define	KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define	KERN_CRIT	"<2>"	/* critical conditions			*/
#define	KERN_ERR	"<3>"	/* error conditions			*/
#define	KERN_WARNING	"<4>"	/* warning conditions			*/
#define	KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define	KERN_INFO	"<6>"	/* informational			*/
#define	KERN_DEBUG	"<7>"	/* debug-level messages			*/



#define console_loglevel (console_printk[0])
#define default_message_loglevel (console_printk[1])
#define minimum_console_loglevel (console_printk[2])
#define default_console_loglevel (console_printk[3])


struct console_font_op {
	unsigned int op;	/* operation code KD_FONT_OP_* */
	unsigned int flags;	/* KD_FONT_FLAG_* */
	unsigned int width, height;	/* font size */
	unsigned int charcount;
	unsigned char *data;	/* font data with height fixed to 32 */
};



/*
 * this is what the terminal answers to a ESC-Z or csi0c query.
 */
#define VT100ID "\033[?1;2c"
#define VT102ID "\033[?6c"

struct consw {
	const char *(*con_startup)(void);
	void	(*con_init)(struct vc_data *, int);
	void	(*con_deinit)(struct vc_data *);
	void	(*con_clear)(struct vc_data *, int, int, int, int);
	void	(*con_putc)(struct vc_data *, int, int, int);
	void	(*con_putcs)(struct vc_data *, const unsigned short *, int, int, int);
	void	(*con_cursor)(struct vc_data *, int);
	int	(*con_scroll)(struct vc_data *, int, int, int, int);
	void	(*con_bmove)(struct vc_data *, int, int, int, int, int, int);
	int	(*con_switch)(struct vc_data *);
	int	(*con_blank)(struct vc_data *, int);
	int	(*con_font_op)(struct vc_data *, struct console_font_op *);
	int	(*con_set_palette)(struct vc_data *, unsigned char *);
	int	(*con_scrolldelta)(struct vc_data *, int);
	int	(*con_set_origin)(struct vc_data *);
	void	(*con_save_screen)(struct vc_data *);
	u8	(*con_build_attr)(struct vc_data *, u8, u8, u8, u8, u8);
	void	(*con_invert_region)(struct vc_data *, u16 *, int);
	u16    *(*con_screen_pos)(struct vc_data *, int);
	unsigned long (*con_getxy)(struct vc_data *, unsigned long, int *, int *);
};
/*
extern const struct consw *conswitchp;
extern const struct consw vga_con;	
extern const struct consw dummy_con;
extern const struct consw fb_con;	
extern const struct consw vga_con;
extern const struct consw newport_con;
extern const struct consw prom_con;	
*/
void take_over_console(const struct consw *sw, int first, int last, int deflt);
void give_up_console(const struct consw *sw);

/* scroll */
#define SM_UP       (1)
#define SM_DOWN     (2)

/* cursor */
#define CM_DRAW     (1)
#define CM_ERASE    (2)
#define CM_MOVE     (3)

/*
 *	Array of consoles built from command line options (console=)
 */
struct console_cmdline
{
	char	name[8];			/* Name of the driver	    */
	int	index;				/* Minor dev. to use	    */
	char	*options;			/* Options for the driver   */
};
#define MAX_CMDLINECONSOLES 8
extern struct console_cmdline console_list[MAX_CMDLINECONSOLES];

/*
 *	The interface for a console, or any other device that
 *	wants to capture console messages (printer driver?)
 */

#define CON_PRINTBUFFER	(1)
#define CON_CONSDEV	(2) /* Last on the command line */
#define CON_ENABLED	(4)

struct console
{
	char	name[8];
	void	(*write)(struct console *, const char *, unsigned);
	int	(*read)(struct console *, const char *, unsigned);
	int /*kdev_t*/	(*device)(struct console *);
	int	(*wait_key)(struct console *);
	void	(*unblank)(void);
	int	(*setup)(struct console *, char *);
	short	flags;
	short	index;
	int	cflag;
	struct	 console *next;
};

extern void register_console(struct console *);
extern int unregister_console(struct console *);
extern struct console *console_drivers;
extern void acquire_console_sem(void);
extern void release_console_sem(void);
extern void console_conditional_schedule(void);
extern void console_unblank(void);
extern void con_init(void);
extern  void set_cursor(int currcons);
extern void redraw_screen(int new_console, int is_switch);
/* VESA Blanking Levels */
#define VESA_NO_BLANKING        0
#define VESA_VSYNC_SUSPEND      1
#define VESA_HSYNC_SUSPEND      2
#define VESA_POWERDOWN          3

#endif /* _LINUX_CONSOLE_H */
