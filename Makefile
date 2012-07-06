obj-m += dresden.o
dresden-objs := dresden_core.o module_notifier_event.o

all:
	$(MAKE) -C /lib/modules/`uname -r`/build M=`pwd` modules
clean:
	$(MAKE) -C /lib/modules/`uname -r`/build M=`pwd` clean
	$(RM) Module.markers modules.order
