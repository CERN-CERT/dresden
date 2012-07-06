#include <linux/kernel.h>
#include <linux/module.h>
#include "module_notifier_event.h"

#define MODULE_NAME "dresden"
#define DRESDEN_OK 0

static int __init dresden_engage(void)
{
	register_module_notifier_event_handler();
	
	/* Hide ourselves */
	
	list_del(&THIS_MODULE->list);	

	printk(KERN_INFO MODULE_NAME ": Engaged\n");
	printk(KERN_INFO MODULE_NAME ":\t[+] Future loading of kernel modules will be prevented\n");
	printk(KERN_INFO MODULE_NAME ":\t[+] Emergency messages will be logged in case of trying to load or unload a module\n");
	printk(KERN_INFO MODULE_NAME ":\t[+] You are not able to remove this module\n");
	
	return DRESDEN_OK;
}

module_init(dresden_engage);

MODULE_AUTHOR("Panos Sakkos <panos.sakkos@cern.ch>");
MODULE_DESCRIPTION("Block incoming modules and log emergency alerts in case of trying to insert\n"
						"\t\t a new module or removing an existing one");
MODULE_LICENSE("GPL");
