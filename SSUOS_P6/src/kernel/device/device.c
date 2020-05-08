#include <device/console.h>
#include <device/device.h>
#include <device/io.h>


void dev_shutdown(void)
{
	const char s[] = "Shutdown";
	const char *p;

	//kill process
	//flush files

	printk ("Powering off...\n");
	//flush serial & screen

	/* This is a special power-off sequence supported by Bochs and
	   QEMU, but not by physical hardware. */
	for (p = s; *p != '\0'; p++)
		outb (0x8900, *p);

	/* This will power off a VMware VM if "gui.exitOnCLIHLT = TRUE"
	   is set in its configuration file.  (The "ssuos" script does
	   that automatically.)  */
	asm volatile ("cli; hlt" : : : "memory");

	/* None of those worked. */
	printk ("still running...\n");
	for (;;);
}
