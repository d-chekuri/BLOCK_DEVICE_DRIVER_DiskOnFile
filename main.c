/* 
* Block device driver module
* DOF of size 512KB
*/

/*
 * Necessary libraries 
 */
#include <linux/module.h>
#include <linux/fs.h>       
#include <linux/errno.h>    
#include <linux/types.h>    
#include <linux/fcntl.h>    
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>  
#include <linux/blk-mq.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/slab.h>
#include <linux/buffer_head.h>

/* 
*  Size of file on disk
*/

#define MEMSIZE 1024 
# define KERNSIZE (512)

/*
* Required MACROs
*/

#define SECT_SIZE 512
#define MBR_SIZE 512
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_DISK_SIGNATURE_SIZE 4
#define PARTITION_TABLE_OFFSET 446
#define PARTITION_ENTRY_SIZE 16 
#define PARTITION_TABLE_SIZE 64 
#define MBR_SIGNATURE_OFFSET 510
#define MBR_SIGNATURE_SIZE 2
#define MBR_SIGNATURE 0xAA55

#define MY_BLOCK_MINOR         2   // minor numbers of block driver
#define MY_BLOCK_MAJOR         240 // major numbe rof the block dirver

static int hardsect_size = 512;
static int nsectors = 1024; 


/* Sector size*/

int  sectsize = 512; 
/********************************************************/
/*
* Partioning and Partition table structs
*/

typedef struct
{
    unsigned char boot_type; // 0x00 - Inactive; 0x80 - Active (Bootable)
    unsigned char start_head;
    unsigned char start_sec:6;
    unsigned char start_cyl_hi:2;
    unsigned char start_cyl;
    unsigned char part_type;
    unsigned char end_head;
    unsigned char end_sec:6;
    unsigned char end_cyl_hi:2;
    unsigned char end_cyl;
    unsigned int abs_start_sec;
    unsigned int sec_in_part;
} PartEntry;

typedef PartEntry PartTable[4];

static PartTable def_part_table =
{
    {
        boot_type: 0x00,
        start_head: 0x00,
        start_sec: 0x2,
        start_cyl: 0x00,
        part_type: 0x83,
        end_head: 0x00,
        end_sec: 0x20,
        end_cyl: 0x09,
        abs_start_sec: 0x00000001,
        sec_in_part: 0x0000013F
    },
    {
        boot_type: 0x00,
        start_head: 0x00,
        start_sec: 0x1,
        start_cyl: 0x14,
        part_type: 0x83,
        end_head: 0x00,
        end_sec: 0x20,
        end_cyl: 0x1F,
        abs_start_sec: 0x00000280,
        sec_in_part: 0x00000180
    },
    {
    },
    {
    }
};

/*
*copying MBR into primary partition
*/

static void copy_mbr(u8 *disk)
{
    memset(disk, 0x0, MBR_SIZE);
    *(unsigned long *)(disk + MBR_DISK_SIGNATURE_OFFSET) = 0x36E5756D;
    memcpy(disk + PARTITION_TABLE_OFFSET, &def_part_table, PARTITION_TABLE_SIZE);
    *(unsigned short *)(disk + MBR_SIGNATURE_OFFSET) = MBR_SIGNATURE;
}

/****************************************************************************************/
/*
* Representation of device
*/
static struct my_block_dev
{
   int size;                      
   u8 *data;
   spinlock_t lock;
   struct blk_mq_tag_set tag_set;
   struct request_queue *queue;
   struct gendisk *gd;
}dev;
/*********************************************************/
/*
* code snippet ensures 
*this module will work on different kernel versions
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
static inline struct request_queue *
blk_generic_alloc_queue(make_request_fn make_request, int node_id)
#else
static inline struct request_queue *
blk_generic_alloc_queue(int node_id)
#endif
{

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
    return (blk_alloc_queue(make_request, node_id));
#else
    return (blk_alloc_queue(node_id));
#endif
}
/***************************************************************/
/*
* Handling IO requests
*/
static void my_block_transfer(struct my_block_dev *dev, unsigned long sector,unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector*SECT_SIZE;
    unsigned long nbytes = nsect*SECT_SIZE;

    if ((offset + nbytes) > dev->size) {
        printk (KERN_NOTICE "Beyond-end write (%ld %ld)\n", offset, nbytes);
        return;
    }
    if (write)
        memcpy(dev->data + offset, buffer, nbytes);
    else
        memcpy(buffer, dev->data + offset, nbytes);
}

static blk_status_t my_block_request(struct blk_mq_hw_ctx *hctx,const struct blk_mq_queue_data *bd)
{
    struct request *rq = bd->rq;
    struct my_block_dev *dev=rq->rq_disk->private_data;
    struct req_iterator iter;
    struct bio_vec bvec;
    sector_t pos_sector = blk_rq_pos(rq);
    void *buffer;
    
    blk_mq_start_request(rq);

    if (blk_rq_is_passthrough(rq)) {
        printk (KERN_NOTICE "Skip non-fs request\n");
        blk_mq_end_request(rq, BLK_STS_IOERR);
        goto out;
    }

 
        rq_for_each_segment(bvec, rq, iter)
    {
        size_t num_sector = blk_rq_cur_sectors(rq);
        printk (KERN_NOTICE "dir %d sec %lld, nr %ld\n",
                        rq_data_dir(rq),
                        pos_sector, num_sector);
        buffer = page_address(bvec.bv_page) + bvec.bv_offset;
        my_block_transfer(dev, pos_sector, num_sector,
                buffer, rq_data_dir(rq) == WRITE);
        pos_sector += num_sector;
    }

    blk_mq_end_request(rq, BLK_STS_OK);

out:
    return BLK_STS_OK;
}

/********************************************************/
/*
*/
static struct blk_mq_ops my_queue_ops = {
   .queue_rq = my_block_request,
};

static int my_block_open(struct block_device *bdev, fmode_t mode)
{
    printk(KERN_INFO "mydiskdrive : open \n");
        return 0;
}

static void my_block_release(struct gendisk *gd, fmode_t mode)
{
    printk(KERN_INFO "mydiskdrive : closed \n");
   
}

struct block_device_operations my_block_ops = {
    .owner = THIS_MODULE,
    .open = my_block_open,
    .release = my_block_release
};

//***************************************************/

/* 
*creating and initiliazing block device dirver
*/
static int create_block_device(struct my_block_dev *dev)
{

    dev->size = nsectors*hardsect_size;
    dev->data = vmalloc(dev->size);
    copy_mbr(dev->data);
    spin_lock_init(&dev->lock);     
    dev->queue = blk_mq_init_sq_queue(&dev->tag_set, &my_queue_ops, 128, BLK_MQ_F_SHOULD_MERGE);
    blk_queue_logical_block_size(dev->queue, hardsect_size);
    (dev->queue)->queuedata = dev;
    
   dev->gd=alloc_disk(MY_BLOCK_MINOR);
   if(!dev->gd)
   {
      printk(KERN_NOTICE "failed alloc_disk");
      return -ENOMEM;
   }
    
   dev->gd->major=MY_BLOCK_MAJOR;
   dev->gd->first_minor=0;
   dev->gd->minors=MY_BLOCK_MINOR;
   dev->gd->fops= &my_block_ops;
   dev->gd->queue=dev->queue;
   dev->gd->private_data=dev;
   snprintf(dev->gd->disk_name,32,"dof"); 
   set_capacity(dev->gd, nsectors*(hardsect_size/SECT_SIZE));
   
   add_disk(dev->gd);
   return 0;
}

static int my_block_init(void)
{
    int status;

    status = register_blkdev(MY_BLOCK_MAJOR, "dof");
    if (status < 0) 
     {
             printk(KERN_ERR "unable to register dof block device\n");
             return -EBUSY;
     }
   
     status=create_block_device(&dev);
     if(status<0)
     return status;
     
     return 0;
}
/*****************************************************/
/*
* Deregistering and deleting the block device driver
*/
static void delete_block_device(struct my_block_dev *dev)
{
   if(dev->gd)
   del_gendisk(dev->gd);
 
   blk_cleanup_queue(dev->queue);
}


static void my_block_exit(void)
{
     delete_block_device(&dev);
     put_disk(dev.gd);
     vfree(dev.data);
     spin_unlock(&dev.lock);
 
     unregister_blkdev(MY_BLOCK_MAJOR, "dof");
}
/*************************************************/
module_init(my_block_init);
module_exit(my_block_exit);
MODULE_AUTHOR("Deepak_Chekuri");
MODULE_DESCRIPTION("BLOCK DRIVER");

/*********** THE END **************************/