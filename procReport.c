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
  int runnableptr;
  int unrunnableptr;
  int stoppedptr;
  struct processInfo *processInfoListptr;
};

static struct Report myReport;
//function signatures
static void traverse_children(struct task_struct *, struct processInfo *);
static int hello_proc_show(struct seq_file *, void *);
static void traverse_processes(struct task_struct *);
static void check_state(int);
static void freeAll(void);
static int hello_proc_open(struct inode *, struct file *);
static int __init hello_proc_init(void);

//prints report to /proc/proc_report
static int hello_proc_show(struct seq_file *m, void *v)
{
  struct processInfo *scan;
  seq_printf(m, "PROCESS REPORTER:\n");
  list_for_each_entry(scan, &(myReport.processInfoListptr->list), list)
  {

    seq_printf(m, "Process ID=%d Name=%s ", scan->pid, scan->name);
    if (scan->number_of_children == 0)
    {
      seq_printf(m, "*No Children\n");
    }
    else
    {
      seq_printf(m, "number_of_children=%d first_child_pid=%d first_child_name=%s\n",
                 scan->number_of_children, scan->first_child_pid, scan->first_child_name);
    }
  }
  return 0;
}

//traverses each process and its children to create a list of processes and counters needed for myReport
static void traverse_processes(struct task_struct *task)
{
  //iterates though each process
  //struct processInfo procInfoList;
  struct processInfo *procInfo;
  struct list_head *list;

  myReport.processInfoListptr = (struct processInfo *)kmalloc(sizeof(struct processInfo), GFP_KERNEL);
  INIT_LIST_HEAD(&(myReport.processInfoListptr->list));
  list = &(myReport.processInfoListptr->list);
  for_each_process(task)
  {
    procInfo = (struct processInfo *)kmalloc(sizeof(struct processInfo), GFP_KERNEL);
    list_add(&(procInfo->list), list);
    list = &(procInfo->list);
    check_state(task->state);
    procInfo->pid = task->pid;
    strcpy(procInfo->name, task->comm);
    //will traverse children to get counts and oldest child info (first child)
    traverse_children(task, procInfo);
  }
  //setting up content of static report struct for helloProc to access
}

//traverses children of proccess currently being looked at
static void traverse_children(struct task_struct *task, struct processInfo *procInfo)
{
  struct task_struct *currentChild;
  procInfo->number_of_children = 0;
  list_for_each_entry(currentChild, &(task->children), sibling)
  {
    if (procInfo->number_of_children == 0)
    { //takes in first child info
      procInfo->first_child_pid = currentChild->pid;
      strcpy(procInfo->first_child_name, currentChild->comm);
    }
    procInfo->number_of_children++;
  }
}

//checks each process state and increments appropiate state counters for myReport
static void check_state(int state)
{
  if (state == 0)
  {
    myReport.runnableptr++;
  }
  else if (state > 0)
  {
    myReport.stoppedptr++;
  }
  else
  {
    myReport.unrunnableptr++;
  }
}

//frees all unused allocated memory
static void freeAll(void)
{
  struct processInfo *scan;
  struct list_head *pos, *q;
  list_for_each_safe(pos, q, &(myReport.processInfoListptr->list))
  {
    scan = list_entry(pos, struct processInfo, list);
    list_del(pos);
    kfree(scan);
  }
  list_del(&(myReport.processInfoListptr->list));
  kfree(myReport.processInfoListptr);
}
//opens initial seq_file
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
  freeAll();
}

MODULE_LICENSE("GPL");
module_init(hello_proc_init);
module_exit(hello_proc_exit);