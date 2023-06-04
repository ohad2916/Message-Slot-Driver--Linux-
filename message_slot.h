#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#define BUF_LEN 128
#define MAJOR_NUMBER 235
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUMBER, 0, unsigned long)


typedef struct channel
{
	ssize_t id;
	ssize_t msg_length;
	char* msg;
}Channel;

typedef struct channel_list
{
	Channel value;
	struct channel_list* next;
}ChannelNode;

typedef struct device
{
	long minor;
	ChannelNode* channel_list;
}Device;



static Channel* getChannel(Device* device, ssize_t id)
{
	ChannelNode* node = device->channel_list;
	if(node == NULL)
	{
		node = kmalloc(sizeof(ChannelNode), GFP_KERNEL);
		node->value.id = id;
		node->value.msg = NULL;
		node->value.msg_length = 0;
		node->next = NULL;
		device->channel_list = node;
		return &node->value;
	}

	while (node->next != NULL)
	{
		if (node->value.id == id)
		{
			return &(node->value);
		}
		node = node->next;
	}
	if (node->value.id == id)
		return &(node->value);

	// else create a new one
	node->next = kmalloc(sizeof(ChannelNode), GFP_KERNEL);
	node = node->next;
	// allocating buffer for new channel
	node->value.id = id;
	node->value.msg = NULL;
	node->value.msg_length = 0;
	node->next = NULL;
	return &node->value;
}

static void channelListCleanup(ChannelNode* head)
{
	while(head != NULL)
	{
		ChannelNode* temp = head;
		kfree(temp->value.msg);
		head = head->next;
		kfree(temp);
	}
}

static void deviceCleanup(Device* devices)
{
	int i = 0;
	for(i = 0; i < 256;i++)
	{
		channelListCleanup(devices[i].channel_list);
	}
}



