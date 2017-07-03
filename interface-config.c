/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* String */
#define KEY_FUNC_HID_IDX	0
static struct usb_string key_func_string_defs[] = {
	[KEY_FUNC_HID_IDX].s	= "HID Interface",
	{},			
};

static struct usb_gadget_strings key_func_string_table = {
	.language	= 0x0409,					/* en-US */
	.strings	= key_func_string_defs,
};

static struct usb_gadget_strings *key_func_strings[] = {
	&key_func_string_table,
	NULL,
};

/* Mobile Keyboard HID Descriptor */
#define KEY_HID_DESC_LEN sizeof(key_hid_desc)
static struct hid_descriptor key_hid_desc = {
	.bLength = KEY_HID_DESC_LEN,
	.bDescriptorType = 0x21,					/* ID of hid descriptor type */
	.bcdHID = __constant_cpu_to_le16(0x0100),	/* CPU_TO_LE */
	.bCountryCode=0x00,							/* Country ID. USA: 0x21 */
	.bNumDescriptors=0x01,						/* Sub descriptor's amount */
	.desc[0].bDescriptorType= 0x22,				/* Type of sub descriptor . Report descriptor is 0x22 */
	.desc[0].wDescriptorLength = 0x3F,
};

/* Mobile Keyboard Interface Descriptor */
static struct usb_interface_descriptor key_interface_desc = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE, 		/* Type of Interface Descriptor */
	.bInterfaceNumber =0x00,					/* ID of Interface */
	.bAlternateSetting =0x00,					/* Spare ID */
	.bNumEndpoints =0x01,						/* Amount of Interrupt Endpoints */
	.bInterfaceClass =0x03,						/* Class Belong to */
	.bInterfaceSubClass=0x01,					/* No Subclass:0x00 	Boot Interface Subclass:0x01 */
	.bInterfaceProtocol=0x01,					/* None:0x00		Mouse:0x02   Keyboard:0x01 */
	.iInterface	= 0x00,
};

/* Input Endpoint Descriptor */
static struct usb_endpoint_descriptor key_input_ep_desc = {
	.bLength= USB_DT_ENDPOINT_SIZE,						/* Length of Descriptor */
	.bDescriptorType=USB_DT_ENDPOINT,				
	.bEndpointAddress= USB_DIR_IN,						/* Address of Endpoint */
	.bmAttributes = 0x03,								/* Transfer Type of Endpoint:D1~D0; Interrupter:0x03 */
	.wMaxPacketSize = __constant_cpu_to_le16(0x0008),	/* Max Packet Size */
	.bInterval = 0x0A,									/* Inquire Iinterval Time(10ms) */
};

/* Output Endpoint Descriptor */
static struct usb_endpoint_descriptor key_output_ep_desc = {
	.bLength= USB_DT_ENDPOINT_SIZE,						/* Length of Descriptor */
	.bDescriptorType=USB_DT_ENDPOINT,				
	.bEndpointAddress= 0x01,							/* Address of Endpoint */
	.bmAttributes = 0x03,								/* Transfer Type of Endpoint:D1~D0; Interrupter:0x03 */
	.wMaxPacketSize = __constant_cpu_to_le16(0x0008),	/* Max Packet Size */
	.bInterval = 0x0A,									/* Inquire Iinterval Time(10ms) */
};

static struct usb_descriptor_header *fs_key_descs[] = {
	(struct usb_descriptor_header *) &key_interface_desc,
	(struct usb_descriptor_header *) &key_hid_desc,
	(struct usb_descriptor_header *) &key_input_ep_desc,
	NULL,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int set_modifier(char *modiferKeys,unsigned int index){
	/* set the value */
	if(index >= MODI_MAX || index < 0)
		return -1;
	
	*modiferKeys = *modiferKeys |(0x01 << index);
	return (int)*modiferKeys;
}
