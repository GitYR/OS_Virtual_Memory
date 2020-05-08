#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stddef.h>
#include <inttypes.h>

/* block device sector size */
#define BLK_SEC_SIZE 512

#define BLK_DEV_ROM 0
#define BLK_DEV_HDD 1
#define BLK_DEV_SSD 2
#define BLK_DEV_USB 3
#define BLK_DEV_CDR 4
#define BLK_DEV_OTH 5

/* sector number */
typedef uint32_t blk_sec_t;

/* block count */
typedef uint32_t blk_cnt_t;

/* block size */
typedef uint32_t blk_size_t;

/* block device type */
typedef uint32_t blk_dev_t;

/* block device. */
struct blk_dev
{
    blk_dev_t type;
    blk_cnt_t blk_count;
    blk_size_t blk_size;                 
    const struct blk_dev_ops *ops;    
    void *device_info;                        /* Extra data owned by driver. */
};

/* block device operations */
void blk_dev_read (struct blk_dev *, blk_sec_t, void *);
void blk_dev_write (struct blk_dev *, blk_sec_t, const void *);

struct blk_dev_ops
{
  void (*read) (void *blk_dev, blk_sec_t, void *buf);
  void (*write) (void *blk_dev, blk_sec_t, const void *buf);
};

// struct blk_dev ata0_blk_dev;
// struct blk_dev ata1_blk_dev;

/* register block device */
void blk_dev_register (struct blk_dev *, blk_dev_t,
  const char *dev, const char *serial, blk_cnt_t count,
  blk_size_t size, const struct blk_dev_ops *, void *device_info);


#endif 
