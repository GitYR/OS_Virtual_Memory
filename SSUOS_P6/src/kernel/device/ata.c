#include <device/ata.h>
#include <ctype.h>
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>
#include <device/block.h>
#include <device/partition.h>
#include <device/io.h>
#include <interrupt.h>
#include <synch.h>
#include <device/console.h>

#define STUDENT_NAME "HeoManwoo"

static void sector_read (struct ata_device*, void *);
static void sector_write (struct ata_device *, const void *);

static void disk_handler (struct intr_frame *iframe);

static char *transform_string (char *, int len);
static void print_hex(char buf[], blk_sec_t ,int size);

static void jump_ata (struct ata_device *device);
static bool delay_ata_for(struct ata_device *device, int what_for);
static void ata_cmd(struct ata_device *n, uint8_t command);
static void ata_io_ready (struct ata_device *d, blk_sec_t sec_num);
static void ata_write (void *dev, blk_sec_t sec_num, const void *buf);
static void ata_read (void *dev, blk_sec_t sec_num, void *buf);


static struct blk_dev_ops ata_ops =
{
	ata_read,
	ata_write
};


unsigned char tmp_block[ATA_SECOR_SIZE] = {0, };


/* Sends an IDENTIFY DEVICE command to disk D and reads the
   response.  Registers the disk with the block device
   layer. */

void clr_tmp_block(void)
{
	for(int i = 0; i < ATA_SECOR_SIZE; i++)
		tmp_block[i] = 0x00;
}


	void
init_ata(void) 
{
		int i;
		blk_cnt_t nblock;

		for(i = 0; i < ATA_DEV_NUM; i++){
			if(i == 0){
				ata.nodes[i].type = ATA_MASTER;			
				ata.nodes[i].port_addr = 0x1f0;
				ata.nodes[i].irq = 5 + 0x20;
			}
			else{
				ata.nodes[i].type = ATA_SLAVE;
				ata.nodes[i].port_addr = 0x170;
				ata.nodes[i].irq = 7 + 0x20;
			
			}
			struct ata_device * device = &ata.nodes[i];

			sema_init (&device->sema, 0);

			reg_handler (device->irq, disk_handler);

			jump_ata (device);
			delay_ata_for (device, ATA_WAIT_IDLE);

			ata_cmd (device, ATA_CMD_IDENTIFY);

			sema_down (&device->sema);
		
			sector_read (device, tmp_block);

			nblock = *(blk_cnt_t *) (tmp_block + 120);
			if(device->type == ATA_MASTER){
				//printk("ata0 : ");
				blk_dev_register (&ata0_blk_dev, BLK_DEV_HDD, 
					transform_string (tmp_block + 20, 20),
					transform_string (tmp_block + 54, 20),
					nblock, BLK_SEC_SIZE, &ata_ops, device);
				clr_tmp_block();
			}else{
				//printk("ata1 : ");
				blk_dev_register (&ata1_blk_dev, BLK_DEV_HDD, 
					transform_string (tmp_block + 20, 20),
					transform_string (tmp_block + 54, 20), 
					nblock, BLK_SEC_SIZE, &ata_ops, device);
				clr_tmp_block();
			}
		}
}

static void
jump_ata (struct ata_device *device) 
{
	uint8_t jmp_hd_reg ;
	if (device->type == ATA_MASTER)
		jmp_hd_reg = ATA_DRHD_MST;
	else 
		jmp_hd_reg = ATA_DRHD_SLV;

	outb (ATA_REG_DEVICE (device->port_addr), jmp_hd_reg );
	inb (ATA_DEV_CTRL_REG (device->port_addr)); 
}
/* Low-level ATA primitives. */
static bool 
delay_ata_for(struct ata_device *device, int what_for)
{
	int i;

	for(i = 0; i < 5000; i++)
	{
		if(what_for == ATA_WAIT_IDLE && 
				(inb (ATA_REG_STATUS (device->port_addr)) & (ATA_SR_BSY | ATA_SR_DRQ)) == 0)
			return true;
		else if(what_for == ATA_WAIT_BUSY && !(inb (ATA_DEV_CTRL_REG (device->port_addr)) & ATA_SR_BSY))
			return (inb (ATA_DEV_CTRL_REG (device->port_addr)) & ATA_SR_DRQ) != 0;
	}
	return false;
}

	static void
ata_io_ready (struct ata_device *d, blk_sec_t sec_num)
{

	delay_ata_for (d, ATA_WAIT_IDLE);
	jump_ata (d);
	delay_ata_for (d, ATA_WAIT_IDLE);
	outb (ATA_REG_ERR (d->port_addr), 0); 
	outb (ATA_REG_NSECT (d->port_addr), 1);
	outb (ATA_REG_LBAL (d->port_addr), sec_num);
	outb (ATA_REG_LBAM (d->port_addr), sec_num >> 8);
	outb (ATA_REG_LBAH (d->port_addr), (sec_num >> 16));
	outb (ATA_REG_DEVICE (d->port_addr),	ATA_DRHD_LBA | (d->type == ATA_SLAVE ? ATA_DRHD_SLV : 0) | (sec_num >> 24));
}

/* Reads sector SEC_NO from disk D into BUFFER, which must have
   room for ATA_SECOR_SIZE bytes.
   Internally synchronizes accesses to disks, so external
   per-disk locking is unneeded. */

	static void
ata_read (void *dev, blk_sec_t sec_num, void *buf)
{

	struct ata_device *d = dev;
	
	 	ata_io_ready (d, sec_num);
	   ata_cmd (d, ATA_CMD_READ_PIO);
	   delay_ata_for (d, ATA_WAIT_IDLE);
	   sema_down (&d->sema);
	  while(!(inb(ATA_REG_STATUS(d->port_addr)) & 0x08));
	   sector_read (d, buf);
	
}

/* Write sector SEC_NO to disk D from BUFFER, which must contain
   ATA_SECOR_SIZE bytes.  Returns after the disk has
   acknowledged receiving the data.
   Internally synchronizes accesses to disks, so external
   per-disk locking is unneeded. */

	static void
ata_write (void *dev, blk_sec_t sec_num, const void *buf)
{
	struct ata_device *d = dev;
	
	  ata_io_ready (d, sec_num);
	   ata_cmd (d, ATA_CMD_WRITE_PIO);
	   if (!delay_ata_for (d, ATA_WAIT_BUSY));
	   sector_write (d, buf);
	   sema_down (&d->sema);

}





/* Writes COMMAND to controller C and prepares for receiving a
   completion interrupt. */
static void
ata_cmd(struct ata_device *device, uint8_t command)
{
	outb (ATA_REG_CMD (device->port_addr), command);
}

/* Reads a sector from controller C's data register in PIO mode into
   SECTOR, which must have room for ATA_SECOR_SIZE bytes. */
	static void
sector_read (struct ata_device *device, void *sector) 
{
	insw (ATA_REG_DATA (device->port_addr), sector, ATA_SECOR_SIZE / 2);
}

/* Writes SECTOR to controller C's data register in PIO mode.
   SECTOR must contain ATA_SECOR_SIZE bytes. */
	static void
sector_write (struct ata_device *device, const void *sector) 
{
	outsw (ATA_REG_DATA (device->port_addr), sector, ATA_SECOR_SIZE / 2);
}


/*ATA interrupt handler. */
	static void
disk_handler (struct intr_frame *iframe) 
{
	struct ata_device *device;
	int i;
	int intr_num = iframe->vec_no;

	for (i = 0; i < 2; i++){
		device = &ata.nodes[i];
		if (intr_num == device->irq)
		{
			inb (ATA_REG_STATUS (device->port_addr));   
			sema_up (&device->sema);     
			return;
		}
	}
}

static void
 print_hex(char buf[], blk_sec_t sec_num, int len) {

	int i,j;
	uint32_t addr = sec_num * ATA_SECOR_SIZE;
    unsigned char *tmp = buf;
    for(i = 0; i < len; i+= 0x10)
    {   
    	printk("%08x: ", addr + i);
        for(j = 0; j < 0x10; j++)
        {   
            if(i + j >= len)
                printk("  ");
            else
                printk("%02x", tmp[i+j]);
            if(j % 2 == 1)
                printk(" ");
        }   
        printk(" | ");
        for(j = 0; j < 0x10; j++)
        {   
            if(i + j >= len)
                break;
            else
                if(isgraph(tmp[i+j]))
                    printk("%c", tmp[i+j]);
                else if(tmp[i+j] == 0x20)
                		printk(" ");
                else
                    printk(".");
        }   
        printk("\n");
    }   
}
//modify
struct ata_device* get_swapdisk(void)
{
	return &ata.nodes[1];
}
struct blk_dev* get_swapblk(void)
{
	return &ata1_blk_dev; 
}

	static char *
transform_string (char *buf, int len) 
{
	int i;
	char tmp;

	for (i = 0; i+2 <= len; i += 2)
	{
		tmp = buf[i];
		buf[i] = buf[i+1];
		buf[i+1] = tmp;
	}

	while(--len > 0){
		if(buf[len] != 0x20 && buf[len] != 0){
			buf[len+1]=0;
			break;
		}
	}
	
	return buf;
	}
