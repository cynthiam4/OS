#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/sched/task.h>

static int unrunnable = 0;
static int runnable   = 0;
static int stopped    = 0;

// gets states of the processes, and increment counters
// runs recursivly
void getProcessStates(struct task_struct *task, struct seq_file *m) {
    struct task_struct *child;
    struct list_head *list;

    if (task->state < 0) {
      unrunnable++;
    } else if (task->state == 0) {
      runnable++;
    } else {
      stopped++;
    }
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        getProcessStates(child, m);
    }
}

// For each of the processes running get ID, name, number of  children
void getProcessesInfo(struct task_struct *task, struct seq_file *m) {
    struct task_struct *child;
    struct list_head *list;
    int numberOfChildren = 0;

    seq_printf(m, "Process ID=%d Name=%s ", task->pid, task->comm);

    //increment to get number of children
    list_for_each(list, &task->children) {
        numberOfChildren++;
    }
    
    if (numberOfChildren == 0) {
      seq_printf(m, "*No Children ");
    } else {
      //print first child info
      seq_printf(m, "number_of_children=%d", numberOfChildren);
      child = list_first_entry(list, struct task_struct, sibling);
      seq_printf(m, " first_child_pid=%d first_child_name=%s", child->pid, child->comm);  
    }

    seq_printf(m, "\n");

    //call recursively to print rest
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        getProcessesInfo(child, m);
    }
}


static int proc_report_show(struct seq_file *m, void *v) {
  seq_printf(m, "PROCESS REPORTER:\n");
  getProcessStates(&init_task, m);
  seq_printf(m, "Unrunnable: %d\nRunnable: %d\nStopped: %d\n", unrunnable, runnable, stopped);
  getProcessesInfo(&init_task, m);
  return 0;
}

static int proc_report_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_report_show, NULL);
}

static const struct file_operations hello_proc_fops = {
  .owner = THIS_MODULE,
  .open = proc_report_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};

static int __init proc_report_init(void) {
  proc_create("proc_report", 0, NULL, &hello_proc_fops);
  return 0;
}

static void __exit proc_report_exit(void) {
  remove_proc_entry("proc_report", NULL);
}

MODULE_LICENSE("GPL");
module_init(proc_report_init);
module_exit(proc_report_exit);
