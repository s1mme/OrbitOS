#include <hdregs.h> 
#include <timer.h>
#include <hw_irq.h>
#include <irq.h>
#include <types.h>
#include <hardisk.h>
#include <io.h>
#include <printk.h>
#include <system.h>
#include <heapmngr.h>
#define STAT_OK         (READY_STAT|SEEK_STAT)
#define OK_STATUS(s)    (((s)&(STAT_OK|(BUSY_STAT|WRERR_STAT|ERR_STAT)))==STAT_OK)
struct hd_i_struct hd_info[] = { {0,0,0,0,0,0},{0,0,0,0,0,0} };
//struct hd_struct hd[MAX_BLKDEV<<6]={{0,0},};
static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
                unsigned int head,unsigned int cyl,unsigned int cmd,
                void (*intr_addr)(struct request *info),struct request *info)
{
        unsigned short port;

        outb_p(0/*hd_info[drive].ctl*/,HD_CMD);
        port=HD_DATA;
        outb_p(0/*hd_info[drive].wpcom */>>2,++port);
	  
        outb_p(nsect,++port);        
        outb_p(sect,++port);      
        outb_p(cyl,++port);   
        outb_p(cyl>>8,++port);
        outb_p(0xA0|(drive<<4)|head,++port);  
        outb_p(cmd,++port);
        intr_addr(info);
        
}
 static void fixstring (unsigned char *s, int bytecount)
 {
         unsigned char *p, *end = &s[bytecount &= ~1];   /* bytecount must be even */
 
         /* convert from big-endian to little-endian */
         for (p = end ; p != s;) {
                 unsigned short *pp = (unsigned short *) (p -= 2);
                 *pp = (*pp >> 8) | (*pp << 8);
         }
 
         /* strip leading blanks */
         while (s != end && *s == ' ')
                 ++s;
 
         /* compress internal blanks and strip trailing blanks */
         while (s != end && *s) {
                 if (*s++ != ' ' || (s != end && *s && *s != ' '))
                         *p++ = *(s-1);
         }
 
         /* wipe out trailing garbage */
         while (p != end)
                 *p++ = '\0';
 }

 
static struct hd_driveid *hd_ident_info[16] = {0, };
  void identify_intr(struct request * test)
 {	
		unsigned int dev = 0;
         //unsigned short stat = inb_p(HD_STATUS);
         struct hd_driveid *id = hd_ident_info[0];

                 insw(HD_DATA, id, 256); /* get ID info */

                 fixstring (id->serial_no, sizeof(id->serial_no));
                 fixstring (id->fw_rev, sizeof(id->fw_rev));
                 fixstring (id->model, sizeof(id->model));
                 printk ("  hd%c: %.40s, serial_no %.40s, rev %.40s, %dMB w/%dKB Cache, MaxMult=%d\n",
                         0+'a', id->model,id->serial_no, id->fw_rev, id->cyls*id->heads*id->sectors/2048,
                         id->buf_size/2, id->max_multsect);
                         
                 hd_info[dev].cyl  = id->cur_cyls;
                 hd_info[dev].head = id->cur_heads;
                 hd_info[dev].sect = id->cur_sectors;         
      
 }
void probe_ide(struct request *info)
{
	hd_out(0,0,0,0,0,WIN_IDENTIFY,&identify_intr,info);
 
}
static int hd_error = 0;
static void dump_status (const char *msg, unsigned int stat, struct request *info)
{
        char devc;

        devc = 0;
     
        sti();
        printk("hd%c: %s: status=0x%02x { ", devc, msg, stat & 0xff);
        if (stat & BUSY_STAT)   printk("Busy ");
        if (stat & READY_STAT)  printk("DriveReady ");
        if (stat & WRERR_STAT)  printk("WriteFault ");
        if (stat & SEEK_STAT)   printk("SeekComplete ");
        if (stat & DRQ_STAT)    printk("DataRequest ");
        if (stat & ECC_STAT)    printk("CorrectedError ");
        if (stat & INDEX_STAT)  printk("Index ");
        if (stat & ERR_STAT)    printk("Error ");
        printk("}\n");
        if ((stat & ERR_STAT) == 0) {
                hd_error = 0;
        } else {
                hd_error = inb(HD_ERROR);
                printk("hd%c: %s: error=0x%02x { ", devc, msg, hd_error & 0xff);
                if (hd_error & BBD_ERR)         printk("BadSector ");
                if (hd_error & ECC_ERR)         printk("UncorrectableError ");
                if (hd_error & ID_ERR)          printk("SectorIdNotFound ");
                if (hd_error & ABRT_ERR)        printk("DriveStatusError ");
                if (hd_error & TRK0_ERR)        printk("TrackZeroNotFound ");
                if (hd_error & MARK_ERR)        printk("AddrMarkNotFound ");
                printk("}");
                if (hd_error & (BBD_ERR|ECC_ERR|ID_ERR|MARK_ERR)) {
                        printk(", CHS=%d/%d/%d", (inb(HD_HCYL)<<8) + inb(HD_LCYL),
                                inb(HD_CURRENT) & 0xf, inb(HD_SECTOR));
                        if (info)
                                printk(", sector=%ld", info->sector);
                }
                printk("\n");
        }
      
}
static int drive_busy(struct request *info)
{
        unsigned int i;
        unsigned char c;

        for (i = 0; i < 500000 ; i++) {
                c = inb_p(HD_STATUS);
                if ((c & (BUSY_STAT | READY_STAT | SEEK_STAT)) == STAT_OK)
                        return 0;
        }
        dump_status("reset timed out", c,info);
        return 1;
}


void check_status(struct request *info)
{
        int i = inb_p(HD_STATUS);

        if (!OK_STATUS(i)) {
                dump_status("check_status", i, info);
        }
}
static unsigned int mult_count  [16] = {0,}; /* currently enabled MultMode count */
static void read_intr(struct request *info)
{

        int i, retries = 100, msect = mult_count[0],  nsect;

        do {
                i = (unsigned) inb_p(HD_STATUS);
                if (i & BUSY_STAT)
                        continue;
                if (!OK_STATUS(i))
                        break;
                if (i & DRQ_STAT)
                        goto ok_to_read;
        } while (--retries > 0);
        dump_status("read_intr", i,info);
        return;
ok_to_read:
       if (msect) {
		   
                if ((nsect = info->current_nr_sectors) > msect)
                        nsect = msect;
                msect -= nsect;
        } else 
                nsect = 1;
        insw(HD_DATA,info->buffer,nsect<<8);
        info->sector += nsect;
        info->buffer += nsect<<9;
        info->errors = 0;
        i = (info->nr_sectors -= nsect);

#ifdef DEBUG
    //    printk("hd%c: read: sectors(%ld-%ld), remaining=%ld, buffer=0x%08lx\n",
      //          dev+'a', info->sector, info->sector+nsect,
        //        info->nr_sectors, (unsigned long) info->buffer+(nsect<<9));
#endif

        if (i > 1) {
                if (msect)
                {
                        goto ok_to_read;
					}
                return;
        } 
      (void) inb_p(HD_STATUS);
      
        return;
}int NR_HD = 0;
 #define barrier() __asm__("": : :"memory")
static void reset_controller(struct request *info)
{
        int     i;

        outb_p(4,HD_CMD);
        for(i = 0; i < 1000; i++) barrier();
        outb_p(hd_info[0].ctl & 0x0f,HD_CMD);
        for(i = 0; i < 1000; i++) barrier();
        if (drive_busy(info))
                printk("hd: controller still busy\n");
        else if ((hd_error = inb(HD_ERROR)) != 1)
                printk("hd: controller reset failed: %02x\n",hd_error);
}
int reset = 1;
static void reset_hd(struct request *info)
{

repeat:
        if (reset) {
                reset = 0;

                reset_controller(info);
        } else {
                check_status(info);
                if (reset)
                        goto repeat;
        }
       
                hd_request(info);
}

static void write_intr(struct request *info)
{
        int i;
        int retries = 10;

        do {
                i = (unsigned) inb_p(HD_STATUS);
                if (i & BUSY_STAT)
                        continue;
                if (!OK_STATUS(i))
                        break;
                if ((info->nr_sectors <= 1) || (i & DRQ_STAT))
                        goto ok_to_write;
        } while (--retries > 0);
        dump_status("write_intr", i,info);
        hd_request(info);
        return;
ok_to_write:
        info->sector++;
        i = --info->nr_sectors;
        --info->current_nr_sectors;
        info->buffer += 512;
        if (i > 0) {
                u16 *words = (u16 *)info->buffer;
				u16 port = HD_DATA;

				for (i=0; i < 256 *  (int)info->sector; i++) {
				outw(port, words[i]);
			} 
        } else {
                hd_request(info);
        }
        return;
}

static void recal_intr(struct request *info)
{
        check_status(info);
#if (HD_DELAY > 0)
        last_req = read_timer();
#endif
        hd_request(info);
}
static void hd_request(struct request *info)
{
        unsigned int dev, block , track, head, cyl;
 signed int sec;
 signed int nsect;

repeat:

        dev = 0; 
        block = info->sector;
        nsect = info->nr_sectors;

        block += 0; 
        dev >>= 6;

        sec   = block % hd_info[dev].sect + 1;
        track = block / hd_info[dev].sect;
        head  = track % hd_info[dev].head;
        cyl   = track / hd_info[dev].head;
#ifdef DEBUG_
        printk("hd%c: %sing: CHS=%d/%d/%d, sectors=%d, buffer=0x%08lx\n",
                dev+'a', (info->cmd == READ)?"read":"writ",
                cyl, head, sec, nsect, (unsigned long) info->buffer);
#endif

        if (info->cmd == READ) {
                unsigned int cmd = WIN_READ; //mult_count[dev] > 1 ? WIN_MULTREAD : WIN_READ;
                hd_out(0,nsect,sec,head,cyl,cmd,&read_intr,info);
                if ( info->nr_sectors != -1 )
                {
                      goto repeat;
				  }
                return;
        }
        if (info->cmd == WRITE) {
                        hd_out(0 /*dev */,nsect,sec,head,cyl,WIN_MULTWRITE,&write_intr,info);
           if ( info->nr_sectors  < 2 )
					return;
				else
                      goto repeat;
 
                return;
        }
        printk("unknown hd-command");  //panic
}

void do_hd_request (struct request *info)
{
	    reset_controller(info);		
        hd_request(info);
        reset_controller(info);		
}

char * read_disc_sector(u32 sector, u32 nr_sectors, void *buf)
{
	
struct request  info = {0,0,0,0,0,0,0,0,0};
info.cmd = 0;
info.sector = sector;
info.nr_sectors = nr_sectors;

info.buffer = (char *)buf;

memset(info.buffer, 0x0,512*nr_sectors);
do_hd_request(&info);

return buf ;
}
/*
void write_disc_sector(u32 sector, u32 nr_sectors, void *data)
{
	
	CURRENT->cmd = 1;
	CURRENT->sector = sector;
    CURRENT->nr_sectors = nr_sectors;
	char * buf = malloc_(512*nr_sectors);
    CURRENT->buffer = buf;
	int i = 0;
 
	CURRENT->buffer = data;

		
	do_hd_request();
	free(buf);
} */
irqreturn_t hd_interrupt(u32 esp);
void hd_init_hook_(void)
{
	struct irqaction *hd;
	hd = malloc_(sizeof(struct irqaction));
	hd->handler = hd_interrupt;
	hd->flags = 0;
	hd->mask = 0;
	hd->name = "hardisk";
	hd->dev_id = NULL;
	hd->next = 0;
	hd->irq = 14;

	
	printk("irq: %d\n", hd->irq);
	printk("name: %s\n", hd->name);


	irq_desc[14].action = hd;
	//irq_desc[1].action = &irq1;
	setup_irq(14, irq_desc[14].action ); 
	
}
irqreturn_t hd_interrupt(u32 esp)
 {

	printk("::hd interrupt::\n");
    return esp;
 }
