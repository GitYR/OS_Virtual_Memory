#include <device/block.h>
#include <device/ata.h>
#include <device/console.h>

/* read sector block from device */
void
blk_dev_read (struct blk_dev *blk_dev, blk_sec_t sector, void *buf)
{
  blk_dev->ops->read (blk_dev->device_info, sector, buf);
}

/* write sector block from device */
void
blk_dev_write (struct blk_dev *blk_dev, blk_sec_t sector, const void *buf)
{
  blk_dev->ops->write (blk_dev->device_info, sector, buf);
}

/* Registers a new block device with the given NAME.  If
   EXTRA_INFO is non-null, it is printed as part of a user
   message.  The block device's SIZE in sectors and its TYPE must
   be provided, as well as the it operation functions OPS, which
   will be passed device_info in each function call. */

void blk_dev_register (struct blk_dev * blk_dev, blk_dev_t type,
  const char *dev, const char *serial, blk_cnt_t count,
  blk_size_t size, const struct blk_dev_ops *ops, void *device_info)
{

  blk_dev->type = type;
  blk_dev->blk_count = count;
  blk_dev->blk_size = size;
  blk_dev->ops = ops;
  blk_dev->device_info = device_info;


}
