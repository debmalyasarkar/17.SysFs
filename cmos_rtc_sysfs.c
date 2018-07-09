#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include "cmos_rtc.h"

static DEFINE_MUTEX(mymtx);

static struct kobject *rtc_obj;

/* Function to Get any RTC Parameter */
uint8_t get_rtc_val(uint8_t param)
{
  uint8_t val;
  int ret; 
  
  ret = mutex_lock_killable(&mymtx);
  if (ret < 0) 
  {
    pr_err("Interrupted while waiting for mutex\r\n");
    return -1;
  }
  outb(param, ADDRESS_REG);
  val = inb(DATA_REG);
  mutex_unlock(&mymtx);
  return val;
}

/* Function to Set any RTC Parameter */
void set_rtc_val(uint8_t param, uint8_t setVal)
{
  int ret;

  ret = mutex_lock_killable(&mymtx);
  if (ret < 0) 
  {
    pr_err("Interrupted while waiting for mutex\r\n");
    return;
  }
  outb(param, ADDRESS_REG);
  outb(setVal, DATA_REG);
  mutex_unlock(&mymtx);
}

/* Function to be invoked when '/sys/cmos_rtc/time' file is accessed for reading using 'cat' */
static ssize_t tm_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
  int retval;
  rtc_t myrtc = {0};

  pr_info("Invoked %s\r\n",__func__);
  /* Get the current CMOS RTC values */
  myrtc.hour   = get_rtc_val(HOUR);
  myrtc.minute = get_rtc_val(MINUTE);
  myrtc.second = get_rtc_val(SECOND);

  /* Write the RTC Values to the buffer which is accessed from user space */
  retval = sprintf(buf, "%02x:%02x:%02x\n", myrtc.hour, myrtc.minute, myrtc.second);

  return retval;
}

/* Function to be invoked when '/sys/cmos_rtc/time' file is accessed for writing using 'echo' */
static ssize_t tm_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
  rtc_t myrtc = {0};

  pr_info("Invoked %s\r\n",__func__);
  /* Some data validation on the buffer can be done here to prevent potential 
     crashes if user writes data in a different format other than the expected 
     one */

  /* Extract the RTC values from the buffer which is accessed from user space */
  count = sscanf(buf, "%02x:%02x:%02x", &myrtc.hour, &myrtc.minute, &myrtc.second);

  /* Update the entered RTC values to the CMOS RTC */
  set_rtc_val(HOUR,    myrtc.hour);
  set_rtc_val(MINUTE,  myrtc.minute);
  set_rtc_val(SECOND,  myrtc.second);
  
  return count;
}

/* Function to be invoked when '/sys/cmos_rtc/date' file is accessed for reading using 'cat' */
static ssize_t dt_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
  int retval;
  rtc_t myrtc = {0};

  pr_info("Invoked %s\r\n",__func__);
  /* Get the current CMOS RTC values */
  myrtc.day   = get_rtc_val(DAY);
  myrtc.month = get_rtc_val(MONTH);
  myrtc.year  = get_rtc_val(YEAR);

  /* Write the RTC Values to the buffer which is accessed from user space */
  retval = sprintf(buf, "%02x/%02x/%02x\n", myrtc.day, myrtc.month, myrtc.year);

  return retval;
}

/* Function to be invoked when '/sys/cmos_rtc/date' file is accessed for writing using 'echo' */
static ssize_t dt_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
  rtc_t myrtc = {0};

  pr_info("Invoked %s\r\n",__func__);
  /* Some data validation on the buffer can be done here to prevent potential 
     crashes if user writes data in a different format other than the expected 
     one */

  /* Extract the RTC values from the buffer which is accessed from user space */
  count = sscanf(buf, "%02x/%02x/%02x", &myrtc.day, &myrtc.month, &myrtc.year);

  /* Update the entered RTC values to the CMOS RTC */
  set_rtc_val(DAY,   myrtc.day);
  set_rtc_val(MONTH, myrtc.month);
  set_rtc_val(YEAR,  myrtc.year);
  
  return count;
}

/* Linking routines to particular entry */
/* Use __ATTR family to ensure that naming convention */

/* In the ATTR Macro the arguments are :
   Arg1 = Name of Entry
   Arg2 = Permissions
   Arg3 = Show function pointer
   Arg4 = Store function pointer
*/
/* Each subfile to be created inside the /sys entry must be defined here */
/* We may also define a common store and show functions which are used by all 
   sub files. There inside the store and show functions we resolve the name entry 
   to find out in which file context the function has been invoked */
static struct kobj_attribute tm_attribute = __ATTR(time, 0660, tm_show, tm_store);
static struct kobj_attribute dt_attribute = __ATTR(date, 0660, dt_show, dt_store);

/* Create the attribute structure for all sub files */
static struct attribute *attrs[] = {
  &tm_attribute.attr,
  &dt_attribute.attr,
  NULL,                /* NULL termination required for the list of attributes */
};

/* Initialize the attribute group for the /sys entry */
static struct attribute_group attr_group = {
  .attrs = attrs,
};

static int __init cmos_rtc_init(void)
{
  int retval;

  pr_info("Invoked %s\r\n",__func__);
  /* Create a struct kobject dynamically and register it with sysfs */
  /* Increments ref Count in kobject structure by 1 to denote that the current 
     module is loaded in memory and it is in use */
  rtc_obj = kobject_create_and_add("cmos_rtc",NULL);
  if(!rtc_obj)
    return -ENOMEM;

  /* Create the files associated with this kobject */
  retval = sysfs_create_group(rtc_obj, &attr_group);
  if(retval)
    kobject_put(rtc_obj);
  
  return retval;
}

static void __exit cmos_rtc_exit(void)
{  
  pr_info("Invoked %s\r\n",__func__);

  /* This function decrements the ref count of kobject by 1 */
  /* When ref count reaches 0,the resources associated with kobject are freed */
  /* If other modules are using this module then ref count will be more han 1 and 
     in that case only doing kobject_put might not delete the kobject */
  /* Instead of kobject_put we can also use kobject_del */

  /* Remove sysfs entry */
  kobject_put(rtc_obj);
}

/* Register the names of custom entry and exit routines */
module_init(cmos_rtc_init);
module_exit(cmos_rtc_exit);

/* Comments which are retained in code */
MODULE_AUTHOR("debmalyasarkar1@gmail.com");
MODULE_DESCRIPTION("Creating Files in sysfs for CMOS RTC");
MODULE_LICENSE("GPL");


/* From UserSpace Command Fomat:
Reading - $cat /sys/cmos_rtc/date
Writing - $echo "01/01/18" > /sys/cmos_rtc/date
          &
Reading - $cat /sys/cmos_rtc/time
Writing - $echo "01:01:01" > /sys/cmos_rtc/time
*/
