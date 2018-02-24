#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h> //for printk
#include <linux/sched.h>
#include <linux/list.h>

struct processInfo
{
  int pid;
  char name[16]; //used task->comm which is 16 char nickname for process
  int number_of_children;
  int first_child_pid;
  char first_child_name[16];
  struct list_head processInfoList;
};

static int hello_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m, "PROCESS REPORTER:\n");
}

static void traverse_processes(struct task_struct *task)
{
  //counters
  int runnable = kmalloc(sizeof(int));
  int unrunnable = kmalloc(sizeof(int));
  int stopped = kmalloc(sizeof(int));
  //iterates though each process
  struct processInfo *procInfo;
  for_each_process(task)
  {
    check_state(runnable, unrunnable, stopped, (int)task->state);
    procInfo == kmalloc(sizeof(struct processInfo));
    procInfo.pid = task->pid;
    procInfo.name = task->comm;
    //will traverse children to get counts and oldest child info (first child)
    traverse_children(task, procInfo);
    //printk("%s [%d]\n",task->comm , task->pid);
  }
}

static void traverse_children(struct task_struct *task, struct processInfo *procInfo)
{
  procInfo->number_of_children = 0;
  struct task_struct *currentChild = task->p_cptr;
  while (currentChild)
  {
    procInfo->number_of_children++;
    if (currentChild->p_osptr == NULL)
    { //checks if older sibling process exists
      //assume null means no older sibling, so currentChild IS oldest
      procInfo->first_child_pid = currentChild->pid;
      procInfo->first_child_name = currentChild->comm;
    }
    currentChild = currentChild->p_osptr;
  }
}

//checks each process state and increments appropiate state counters
static void check_state(int *run, int *unrun, int *stop, int *state)
{
  if (state == 0)
  {
    *run++;
  }
  else if (state > 0)
  {
    *stop++;
  }
  else if (state < 0)
  {
    *unrun++;
  }
}

static int hello_proc_open(struct inode *inode, struct file *filp)
{
  return single_open(filp, hello_proc_show, NULL);
}

static const struct file_operations hello_proc_fops = {
    .owner = THIS_MODULE,
    .open = hello_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init hello_proc_init(void)
{
  printk(KERN_INFO "HI");
  traverseProcesses(&init_task);
  proc_create("proc_report", 0, NULL, &hello_proc_fops);
  return 0;
}
static

    static void __exit
    hello_proc_exit(void)
{
  printk(KERN_INFO "BYE");
  remove_proc_entry("proc_report", NULL);
}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);
