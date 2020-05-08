#ifndef	   __ATA_H__
#define	   __ATA_H__

#include <synch.h>
#include <device/block.h>

#define ATA_REG_DATA(base) (base + 0x00)
#define ATA_REG_ERR(base) (base + 0x01)
#define ATA_REG_NSECT(base) (base + 0x02)
#define ATA_REG_LBAL(base) (base + 0x03)
#define ATA_REG_LBAM(base) (base + 0x04)
#define ATA_REG_LBAH(base) (base + 0x05)
#define ATA_REG_DEVICE(base) (base + 0x06)
#define ATA_REG_STATUS(base) (base + 0x07)
#define ATA_REG_CMD(base) (base + 0x07)
#define ATA_ALT_ST_REG(base) (base + 0x206)
#define ATA_DEV_CTRL_REG(base) (base + 0x206)

#define ATA_MASTER 0x01
#define ATA_SLAVE 0x02

#define ATA_SR_BSY  0x80 /* Busy */
#define ATA_SR_DRDY 0x40 /* Drive ready */
#define ATA_SR_DF   0x20 /* Drive write fault */
#define ATA_SR_DSC  0x10 /* Drive seek complete */
#define ATA_SR_DRQ  0x08 /* Data request ready */
#define ATA_SR_CORR 0x04 /* Corrected data */
#define ATA_SR_IDX  0x02 /* inlex */
#define ATA_SR_ERR  0x01 /* Error */

#define ATA_DRHD_DEF 0xa0
#define ATA_DRHD_MST (0x00 | ATA_DRHD_DEF)
#define ATA_DRHD_SLV (0x10 | ATA_DRHD_DEF)
#define ATA_DRHD_LBA (0x40 | ATA_DRHD_DEF)   

#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

#define ATA_WAIT_BUSY 0x01
#define ATA_WAIT_IDLE 0x02

#define ATA_SECOR_SIZE 512


#define ATA_DEV_NUM 2

struct ata_device
{
  uint8_t type;
  uint16_t port_addr;
  uint8_t irq;
  struct semaphore sema;
};

struct ata_interface
{
  struct ata_device nodes[ATA_DEV_NUM];
};

struct ata_interface ata;
struct blk_dev ata0_blk_dev;
struct blk_dev ata1_blk_dev;
struct ata_device* get_swapdisk(void); 
struct blk_dev* get_swapblk(void);
void init_ata(void);

#endif
