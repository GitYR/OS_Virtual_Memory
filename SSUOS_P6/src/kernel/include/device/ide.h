#include <stdint.h>
#include <device/block.h>
#include <device/partition.h>
#include <device/io.h>
#include <interrupt.h>
#include <synch.h>
typedef uint32_t disk_sector_t;
#define DISK_SECTOR_SIZE 512
struct ata_disk *disk_get (int chan_no, int dev_no) ;
disk_sector_t disk_size (struct ata_disk *);
void disk_read (struct ata_disk *d, disk_sector_t sec_no, void *buffer) ;
void disk_write (struct ata_disk *d, disk_sector_t sec_no, const void *buffer);
/* An ATA device. */
struct ata_disk
  {
    char name[8];               /* Name, e.g. "hda". */
    struct channel *channel;    /* Channel that disk is attached to. */
    int dev_no;                 /* Device 0 or 1 for master or slave. */
    bool is_ata;                /* Is device an ATA disk? */
    disk_sector_t capacity;      //Capacity in sectors (if is_ata). 

    long long read_cnt;         /* Number of sectors read. */
    long long write_cnt;        /* Number of sectors written. */
  };

/* An ATA channel (aka controller).
   Each channel can control up to two disks. */
struct channel
  {
    char name[8];               /* Name, e.g. "ide0". */
    uint16_t reg_base;          /* Base I/O port. */
    uint8_t irq;                /* Interrupt in use. */

    struct lock lock;           /* Must acquire to access the controller. */
    bool expecting_interrupt;   /* True if an interrupt is expected, false if
                                   any interrupt would be spurious. */
    struct semaphore completion_wait;   /* Up'd by interrupt handler. */

    struct ata_disk devices[2];     /* The devices on this channel. */
  };
