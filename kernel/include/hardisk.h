#ifndef HARDISK_H
#define HARDISK_H
 struct request {
        volatile int rq_status; // should split this into a few status bits 
		#define RQ_INACTIVE             (-1)
		#define RQ_ACTIVE               1
		#define RQ_SCSI_BUSY            0xffff
		#define RQ_SCSI_DONE            0xfffe
		#define RQ_SCSI_DISCONNECTING   0xffe0

        int cmd;                // READ or WRITE 
        int errors;
        unsigned long sector;
        signed long nr_sectors;
        unsigned long nr_segments;
        unsigned long current_nr_sectors;
        char * buffer;
        struct request * next;
};
 static void reset_hd(struct request *info);
static void hd_request(struct request *info);
 

 #define MAX_BLKDEV 16

 #define WIN_IDENTIFY            0xEC  
 struct blk_dev_struct {
          void (*request_fn)(void);
          struct request * current_request;
          struct request   plug;
  };
  struct blk_dev_struct blk_dev[MAX_BLKDEV];
 #define CURRENT (blk_dev[0].current_request)
 
 struct hd_i_struct {
          unsigned int head,sect,cyl,wpcom,lzone,ctl;
          };
//struct hd_i_struct hd_info[] = { {0,0,0,0,0,0},{0,0,0,0,0,0} };
 struct hd_struct {
          long start_sect;
          long nr_sects;
  };
extern char * read_disc_sector(u32 sector, u32 nr_sectors, void *buf);
extern void hd_init_hook_(void); 
extern void probe_ide(struct request *info);
#endif
