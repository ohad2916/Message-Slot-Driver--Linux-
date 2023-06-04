
#include "message_slot.h"
MODULE_LICENSE("GPL");

#define DEVICE_NAME "message_slot"


static Device devices[256];


static int device_open(struct inode* inode, struct file* file)
{
    int minor;
    unsigned long channel_id;
    minor = iminor(inode);

    channel_id = (unsigned long)file->private_data;

    printk("Invoking device open!(%p)minor:%d\nlast channel used:%lu", file,minor,channel_id);
    if(devices[minor].minor < 0)
    {
        printk("Initiliazing MAJOR %d,MINOR %d", MAJOR_NUMBER, minor);

        // initiliazing minor
        devices[minor].minor = minor;
    }
    return 0;
}

static int device_release(struct inode* inode, struct file* file)
{
    int minor;
    minor = iminor(inode);
    printk("Invoking device close!(%p,minor: %d,channel:%lu)\n", file,minor,(unsigned long)file->private_data);
	return 0;
}


static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
    unsigned long channel_id;
    int err, minor;
    Channel* channel;

    // Error checking
    if (file->private_data == NULL)
    {
        return -EINVAL;
    }
    if (length > BUF_LEN || length <= 0)
    {
        return -EMSGSIZE;
    }
    channel_id = (unsigned long)file->private_data;

    minor = iminor(file->f_inode);
    // this either gets an existing one or creates a new one.
    channel = getChannel(&devices[minor], channel_id);
    if(!(channel->msg = kmalloc(BUF_LEN+1,GFP_KERNEL)))
    {
        return -EFAULT;
    }
    memset(channel->msg, '\0', (BUF_LEN+1) * sizeof(char));
	printk("Invoking device write!(file:%p,message:%s(%lu),minor:%d,channel:%ld)\n"
				,file, buffer, length,minor,channel_id);

    if ((err = copy_from_user(channel->msg, buffer, length)))
    {
        return -EFAULT;
    }
    channel->msg_length = length;

    return length;
}

static ssize_t device_read(struct file* file, char __user* buffer, 
							size_t length, loff_t* offset)
{
    unsigned long channel_id;
    int err, minor;
    Channel* channel;
    if (file->private_data == NULL)
    {
        return -EINVAL;
    }
    channel_id = (unsigned long)file->private_data;
    minor = iminor(file->f_inode);
    if (!channel_id)
        return -EINVAL;

    //find the channel buffer
    channel = getChannel(&devices[minor], channel_id);
	printk("Invoking device_read(%p,%ld,%s)\n", file, length,channel->msg);
    if(channel->msg_length <=0)
    {
        return -EWOULDBLOCK;
    }
    if(channel->msg_length > BUF_LEN)
    {
        return -ENOSPC;
    }
    err = copy_to_user(buffer, channel->msg, channel->msg_length);

    kfree(channel->msg);
    channel->msg = NULL;
    channel->msg_length = 0;

    return err;
}

static long device_ioctl(struct file* file,unsigned int ioctl_command_id,unsigned long ioctl_param)
{

    if (MSG_SLOT_CHANNEL != ioctl_command_id || ioctl_param == 0)
    {
		return -EINVAL;
    }
    file->private_data = (void*)ioctl_param;
    printk("Invoking device ioctl!: channel_id:%lu\n", ioctl_param);
    return 0;

}



static struct file_operations fops = {
    .owner = THIS_MODULE,
	.read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};

static int __init message_slot_init(void)
{
    int i;

    if (register_chrdev(235, DEVICE_NAME, &fops) < 0)
    {
        printk("Failed to register character device!\n");
        return -1;
    }
    // Initilize device array
    for (i = 0; i < 256;i++)
        devices[i].minor = -1;
    printk("Loaded message Slot module!\n");

    return 0;
}

static void __exit message_slot_cleanup(void)
{
    deviceCleanup(devices);
    unregister_chrdev(MAJOR_NUMBER, DEVICE_NAME);
	printk("Unloaded message Slot module!\n");
}

module_init(message_slot_init);
module_exit(message_slot_cleanup);
