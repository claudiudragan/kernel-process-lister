#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<asm/io.h>
#include<asm/uaccess.h>
#include<linux/buffer_head.h>
#include<linux/tty.h>


#define KBD_IRQ 1
#define KBD_DATA_REG 0x60
#define KBD_SCANCODE_MASK 0x7f
#define KBD_STATUS_MASK 0x80
#define BUF_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Silyon");
MODULE_DESCRIPTION("A module which dumps running process info into a file.");

struct tty_struct* my_tty = NULL;
static char buf[BUF_SIZE];

static void print_string(char* msg){
	if(my_tty != NULL){
		my_tty->driver->ops->write(my_tty, msg, strlen(msg));
		my_tty->driver->ops->write(my_tty, "\r\n", 2);
	}
}

void print_proccess(void){
	int count = 0;
	struct task_struct *task_list;

	for_each_process(task_list){
		snprintf(buf, BUF_SIZE, "pid: %d - %s", task_list->pid, task_list->comm);
		print_string(buf);
		count++;
	}

	snprintf(buf, BUF_SIZE, "Procs: %d", count);
	print_string(buf);
}

static irqreturn_t kbd2_isr(int irq, void *dev_id){
	char scancode;

	scancode = inb(KBD_DATA_REG);

	if((scancode & KBD_SCANCODE_MASK) == 0x10){
		pr_info("Scan Code %x %s\n", scancode & KBD_SCANCODE_MASK, scancode & KBD_STATUS_MASK ? "Released" : "Pressed");
		print_proccess();
	}
		
	

	return IRQ_HANDLED;
}


static int __init kbd2_init(void){
	my_tty = current->signal->tty;

	return request_irq(KBD_IRQ, kbd2_isr, IRQF_SHARED, "kbd2", (void*)kbd2_isr);
}

static void __exit kbd2_cleanup(void){
	pr_info("Syslist has been deleted.");
	free_irq(KBD_IRQ, (void*)kbd2_isr);
}

module_init(kbd2_init);
module_exit(kbd2_cleanup);
