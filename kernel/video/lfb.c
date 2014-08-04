#include <system.h>
#include <fs.h>
#include <types.h>
#include <printk.h>
#include <mem.h>
#define PREFERRED_VY 4096
#define PREFERRED_B 32

u16 lfb_resolution_x = 0;
u16 lfb_resolution_y = 0;
u16 lfb_resolution_b = 0;

u8 * lfb_vid_memory = (u8 *)0xE0000000;

static void finalize_graphics(u16 x, u16 y, u16 b) {
	lfb_resolution_x = x;
	lfb_resolution_y = y;
	lfb_resolution_b = b;
}

uintptr_t lfb_get_address(void) {
	return (uintptr_t)lfb_vid_memory;
}

/* Bochs support {{{ */
uintptr_t current_scroll = 0;

void bochs_set_y_offset(u16 y) {
	outports(0x1CE, 0x9);
	outports(0x1CF, y);
	current_scroll = y;
}

u16 bochs_current_scroll(void) {
	return current_scroll;
}

void
graphics_install_bochs(u16 resolution_x, u16 resolution_y) {
	printk( "Setting up BOCHS/QEMU graphics controller...");
	outports(0x1CE, 0x00);
	u16 i = inports(0x1CF);
	if (i < 0xB0C0 || i > 0xB0C6) {
		return;
	}
	outports(0x1CF, 0xB0C4);
	i = inports(0x1CF);
	/* Disable VBE */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x00);
	/* Set X resolution to 1024 */
	outports(0x1CE, 0x01);
	outports(0x1CF, resolution_x);
	/* Set Y resolution to 768 */
	outports(0x1CE, 0x02);
	outports(0x1CF, resolution_y);
	/* Set bpp to 32 */
	outports(0x1CE, 0x03);
	outports(0x1CF, PREFERRED_B);
	/* Set Virtual Height to stuff */
	outports(0x1CE, 0x07);
	outports(0x1CF, PREFERRED_VY);
	/* Re-enable VBE */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x41);

	/* XXX: Massive hack */
	u32 * text_vid_mem = (u32 *)0xA0000;
	text_vid_mem[0] = 0xA5ADFACE;

	for (uintptr_t fb_offset = 0xE0000000; fb_offset < 0xFF000000; fb_offset += 0x01000000) {
		/* Enable the higher memory */
		for (i = fb_offset; i <= fb_offset + 0xFF0000; i += 0x1000) {
			dma_frame( _vmm_get_page_addr(i, 1, kernel_directory), 0, 1, i);
		}

		/* Go find it */
		for (uintptr_t x = fb_offset; x < fb_offset + 0xFF0000; x += 0x1000) {
			if (((uintptr_t *)x)[0] == 0xA5ADFACE) {
				lfb_vid_memory = (u8 *)x;
				goto mem_found;
			}
		}

	}

mem_found:
	finalize_graphics(resolution_x, resolution_y, PREFERRED_B);
}




