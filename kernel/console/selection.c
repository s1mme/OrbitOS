/* Variables for selection control. */
/* Use a dynamic buffer, instead of static (Dec 1994) */
       int sel_cons;		/* must not be disallocated */
static volatile int sel_start = -1; 	/* cleared by clear_selection */
static int sel_end;


/* clear_selection, highlight and highlight_pointer can be called
   from interrupt (via scrollback/front) */

/* set reverse video on characters s-e of console with selection. */
inline static void
highlight(const int s, const int e) {
	//invert_screen(sel_cons, s, e-s+2, 1);
}

/* use complementary color to show the pointer */
inline static void
highlight_pointer(const int where) {
	//complement_pos(sel_cons, where);
}


/* remove the current selection highlight, if any,
   from the console holding the selection. */
void
clear_selection(void) {
	highlight_pointer(-1); /* hide the pointer */
	if (sel_start != -1) {
		highlight(sel_start, sel_end);
		sel_start = -1;
	}
}
