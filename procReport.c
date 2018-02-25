#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h> //for printk
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>

struct processInfo
{
  int pid;
  char name[16]; //used task->comm which is 16 char nickname for process
  int number_of_children;
  int first_child_pid;
  char first_child_name[16];
  struct list_head list;
};

struct Report
{
  int runnable;
  int unrunnable;
  int stopped;
  struct processInfo *processInfoList;
};
//function signatures
static void traverse_children(struct task_struct *, struct processInfo *);
static int hello_proc_show(struct seq_file *, void *);
static void traverse_processes(struct task_struct *);
static void check_state(int *, int *, int *, int);
static int hello_proc_open(struct inode *, struct file *);
static int __init hello_proc_init(void);

static int hello_proc_show(struct seq_file *m, void *v)
{
  seq_printf(m, "PROCESS REPORTER:\n");
  return 0;
}

static void traverse_processes(struct task_struct *task)
{
  //counters
  int *runnable = kmalloc(sizeof(int), GFP_KERNEL);
  int *unrunnable = kmalloc(sizeof(int), GFP_KERNEL);
  int *stopped = kmalloc(sizeof(int), GFP_KERNEL);
  //iterates though each process
  struct processInfo procInfoList;
  struct processInfo *procInfo;
  INIT_LIST_HEAD(&procInfoList.list);
  for_each_process(task)
  {
    check_state(runnable, unrunnable, stopped, task->state);
    procInfo = (struct processInfo *)kmalloc(sizeof(struct processInfo), GFP_KERNEL);
    procInfo->pid = task->pid;
    strcpy(procInfo->name, task->comm);
    //will traverse children to get counts and oldest child info (first child)
    traverse_children(task, procInfo);
    list_add(&(procInfo->list), &(procInfoList.list));
    //printk("%s [%d]\n",task->comm , task->pid);
  }
}

static void traverse_children(struct task_struct *task, struct processInfo *procInfo)
{
  struct task_struct *currentChild;
  procInfo->number_of_children = 0;
  currentChild = task->p_cptr;
  while (currentChild)
  {
    procInfo->number_of_children++;
    if (currentChild->p_osptr == NULL)
    { //checks if older sibling process exists
      //assume null means no older sibling, so currentChild IS oldest
      procInfo->first_child_pid = currentChild->pid;
      strcpy(procInfo->first_child_name, currentChild->comm);
    }
    currentChild = currentChild->p_osptr;
  }
}

//checks each process state and increments appropiate state counters
static void check_state(int *run, int *unrun, int *stop, int state)
{
  if (state == 0)
  {
    *run++;
  }
  else if (state > 0)
  {
    *stop++;
  }
  else
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
  traverse_processes(&init_task);
  proc_create("proc_report", 0, NULL, &hello_proc_fops);
  return 0;
}

static void __exit hello_proc_exit(void)
{
  printk(KERN_INFO "BYE");
  remove_proc_entry("proc_report", NULL);
}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);
