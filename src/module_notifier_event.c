#include <linux/notifier.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include "module_notifier_event.h"

int fail(void)
{
	printk(KERN_INFO "Module insertion blocked by CERN's dresden\n");
	
	return -1;
}

static int dresden_module_handler(struct notifier_block *this, unsigned long event, void *pointer) 
{
	unsigned long flags;
	struct module *new_module;
	DEFINE_SPINLOCK(module_event_spinlock);

	spin_lock_irqsave(&module_event_spinlock, flags);

	new_module = pointer;

	switch(new_module->state)
	{
		case(MODULE_STATE_COMING):
#ifndef MODULE_GRSEC
					printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_COMING name: %s init: 0x%p size of init (text + data)\
													0x%x core: 0x%p size of core (text + data) 0x%x\n",
													new_module->name, new_module->init, new_module->init_size,
													new_module->module_core, new_module->core_size);
#else /* MODULE_GRSEC */
					printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_COMING name: %s, int: 0x%p, init_rx: 0x%p (size 0x%x), init_rw: 0x%p (size 0x%x), core_rx: 0x%p (size 0x%x), core_rw: 0x%p (size 0x%x)\n",
													new_module->name, new_module->init,
													new_module->module_init_rx, new_module->init_size_rx,
													new_module->module_init_rw, new_module->init_size_rw,
													new_module->module_core_rx, new_module->core_size_rx,
													new_module->module_core_rw, new_module->core_size_rw);
#endif /* MODULE_GRSEC */
					new_module->init = fail;

					printk(KERN_EMERG MODULE_NAME ": New module name is '%s'. Its functionality will be disabled\n", new_module->name);
			break;
		case(MODULE_STATE_LIVE):
			if(new_module == THIS_MODULE)
			{
				/* Skip reporting of ourselves */

				break;
			}

#ifndef MODULE_GRSEC
                        printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_LIVE name: %s core: 0x%p size of core (text + data) 0x%x\n",
                        										new_module->name, new_module->module_core, new_module->core_size);
#else /* MODULE_GRSEC */
                        printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_LIVE name: %s, core_rx: 0x%p (size 0x%x), core_rw: 0x%p (size 0x%x)\n",
                        										new_module->name,
													new_module->module_core_rx, new_module->core_size_rx,
													new_module->module_core_rw, new_module->core_size_rw);
#endif /* MODULE_GRSEC */
                        break;		
		case(MODULE_STATE_GOING):
#ifndef MODULE_GRSEC
			printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_GOING name: %s core: 0x%p size of core (text + data) 0x%x\n",
													new_module->name, new_module->module_core, new_module->core_size);
#else /* MODULE_GRSEC */
			printk(KERN_EMERG MODULE_NAME ": event: MODULE_STATE_GOING name: %s core: 0x%p (size 0x%x), core_rw: 0x%p (size 0x%x)\n",
													new_module->name,
													new_module->module_core_rx, new_module->core_size_rx,
													new_module->module_core_rw, new_module->core_size_rw);
#endif /* MODULE_GRSEC */
			break;
		default:
			printk(KERN_ERR MODULE_NAME ": ERROR unknown module state in notifier chain for %s module\n", new_module->name);
			break;						
	}

	spin_unlock_irqrestore(&module_event_spinlock, flags);
	return NOTIFY_DONE;
}

static struct notifier_block module_notifier_block = 
{
	.notifier_call = dresden_module_handler,
	.priority      = INT_MAX
};

void register_module_notifier_event_handler(void)
{
	register_module_notifier(&module_notifier_block);
}

