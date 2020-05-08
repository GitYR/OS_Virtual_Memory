
#include <stddef.h>
#include <device/ata.h>
#define PGBITS  12                         /* Number of offset bits. */
#define PGSIZE  (1 << PGBITS)              /* Bytes in a page. */
#define DISK_SECTOR_SIZE 512

#define DISK_SECTORS_PER_PAGE   (PGSIZE / DISK_SECTOR_SIZE)

 void init_swap (void);
// size_t swap_out_page (const void *kpage);
// void swap_in_page (size_t idx, void *kpage);
// void swap_test(void );


