#include <linux/usb/ch9.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/android_keyboard.h>


static const char keyboard_shortname[] = "android_keyboard";

struct key_dev{
	struct usb_function function;		/* interface for configuration and endpoint */
	struct usb_composite_dev *cdev;
	spinlock_t		lock;

	struct usb_ep	*ep_in;	/* to host */
	struct usb_request *req;
	atomic_t error;
	
	atomic_t write_excl;
	atomic_t open_excl;

	struct list_head tx_idle;	//list usb_request

	wait_queue_head_t write_wq;
	struct usb_request *rx_req;
	int rx_done;

	wait_queue_head_t read_wq;
	atomic_t count;

	int can_write;
};


struct keyboard_io{
	uint8_t	*buf;
	int	len;
};

/* 
 *	D7: L_Ctrl		D6: L_Alt		D5: L_Shift	D4: L_Windows
 *	D3: R_Ctrl		D2: R_Alt		D1: R_Shift	D0: R_Windows
 */
struct key_report{
	
	unsigned char modifierKeys;
	unsigned char reserved;
	unsigned char keycode[6];
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline struct key_dev *func_to_key(struct usb_function *f)
{
	return container_of(f, struct key_dev, function);
}

static struct usb_request *key_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!req)
		return NULL;

	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static int key_create_interrupt_endpoints(struct key_dev *dev,
				struct usb_endpoint_descriptor *in_desc,struct usb_endpoint_descriptor *out_desc)
{
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_ep *ep;
	
	ep = usb_ep_autoconfig(cdev->gadget, in_desc);
	if (!ep) 
		return -ENODEV;

	ep->driver_data = dev;		/* claim the endpoint */
	dev->ep_in = ep;

	return 0;

}

static void key_request_free(struct usb_request *req, struct usb_ep *ep)
{
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static inline int key_lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void key_unlock(atomic_t *ato)
{
	if(atomic_dec_return(ato) != 0)
		atomic_set(ato,0);
}

/* add a request to the tail of the list */
void key_req_put(struct key_dev *dev,struct list_head *head,
				struct usb_request *req)
{
	unsigned long flags;

	if(!req){
		printk(KERN_INFO "<%s> Add the request to the list error , because of the req is NULL!\n",
			__func__);
	}else{
		spin_lock_irqsave(&dev->lock,flags);
		list_add_tail(&req->list,head);
		spin_unlock_irqrestore(&dev->lock,flags);
	}
}

/* remove a request from the head of a list */
struct usb_request *key_req_get(struct key_dev *dev, struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	spin_lock_irqsave(&dev->lock, flags);
	if (list_empty(head)) {
		req = NULL;
	} else {
		req = list_first_entry(head, struct usb_request, list);
		list_del(&req->list);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return req;
}

static void key_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct key_dev *dev = _key_dev;

	if (req->status != 0)
		atomic_set(&dev->error, 1);

	if(usb_ep_dequeue(ep, req) < 0 )
		atomic_set(&dev->error, 1);

	key_request_free(req,dev->ep_in);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int key_function_bind(struct usb_configuration *c,struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct key_dev	*dev = func_to_key(f);
	int			id;
	int			ret;

	dev->cdev = cdev;

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	key_interface_desc.bInterfaceNumber = id;

	/* allocate endpoints */
	ret = key_create_interrupt_endpoints(dev, &key_input_ep_desc,&key_output_ep_desc);
	if (ret)
		return ret;

	return 0;
}


static void key_function_unbind(struct usb_configuration *c,struct usb_function *f)
{
	//struct key_dev *dev = func_to_key(f);
}


static int key_function_set_alt(struct usb_function *f,unsigned interface, unsigned alt)
{
	struct key_dev	*dev = func_to_key(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	int ret;

	ret = config_ep_by_speed(cdev->gadget, f, dev->ep_in);
	if (ret) {
		dev->ep_in->desc = NULL;
		ERROR(cdev, "config_ep_by_speed failes for ep %s, result %d\n",
				dev->ep_in->name, ret);
		return ret;
	}
	ret = usb_ep_enable(dev->ep_in);
	if (ret) {
		ERROR(cdev, "failed to enable ep %s, result %d\n",
			dev->ep_in->name, ret);
		return ret;
	}
	
	wake_up(&dev->read_wq);
	dev->can_write = 1;
	return 0;
}

static void key_function_disable(struct usb_function *f)
{
	struct key_dev	*dev = func_to_key(f);
	dev->can_write = 0;
	usb_ep_disable(dev->ep_in);

}

static int key_setup(void)
{
	
	struct key_dev *dev;
	int ret;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	spin_lock_init(&dev->lock);

	
	init_waitqueue_head(&dev->read_wq);
	init_waitqueue_head(&dev->write_wq);

	atomic_set(&dev->write_excl,0);
	atomic_set(&dev->count,-1);
	INIT_LIST_HEAD(&dev->tx_idle);

	dev->can_write = 0;
	_key_dev = dev;
	
	ret = misc_register(&keyboard_device);
	if (ret)
		goto err;
	
	return 0;
err:
	kfree(dev);
	printk(KERN_ERR "keyboard gadget driver failed to initialize\n");
	return ret;

}


static int
key_ctrlrequest(struct usb_composite_dev *cdev,
	const struct usb_ctrlrequest *c)
{

	int	value = -EOPNOTSUPP;
	u16 w_length = le16_to_cpu(c->wLength);

	if (value >= 0) {
		int rc;
		cdev->req->zero = value < w_length;
		cdev->req->length = value;
		rc = usb_ep_queue(cdev->gadget->ep0, cdev->req, GFP_ATOMIC);
		if (rc < 0)
			ERROR(cdev, "%s: response queue error\n", __func__);
	}
	return value;
}

static int key_bind_config(struct usb_configuration *c)
{
	struct key_dev *dev = _key_dev;

	dev->cdev = c->cdev;
	dev->function.name = "keyboard";
	dev->function.fs_descriptors = fs_key_descs;
	dev->function.hs_descriptors = fs_key_descs;
	if (gadget_is_superspeed(c->cdev->gadget))
		dev->function.ss_descriptors = fs_key_descs;

	dev->function.strings = key_func_strings;
	dev->function.bind = key_function_bind;
	dev->function.unbind = key_function_unbind;
	dev->function.set_alt = key_function_set_alt;
	dev->function.disable = key_function_disable;
	
	return usb_add_function(c, &dev->function);
}


static void key_cleanup(void)
{
	kfree(_key_dev);
	_key_dev = NULL;
}
