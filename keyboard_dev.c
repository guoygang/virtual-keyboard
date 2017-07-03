////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int set_modifier(char *modiferKeys,unsigned int index){
	/* set the value */
	if(index >= MODI_MAX || index < 0)
		return -1;
	
	*modiferKeys = *modiferKeys |(0x01 << index);
	return (int)*modiferKeys;
}

static struct key_report *generate_report(struct key_report *report,char input)
{
	if(!report)
		return NULL;
	
	memset(report, 0, sizeof(*report));

	if(input >= 'a' && input <='z'){
		report->keycode[0] = 0x04 + (input - 'a');		/* The base of a~z is 0x04~0x1D */
	}
	
	if(input >= 'A' && input <='Z'){
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x04 + (input - 'A');
	}

	if(input >='0' && input <= '9'){
		if(input == '0')
			report->keycode[0] = 0x27;					/* the Base of 0 is 0x27 */
		else
			report->keycode[0] = 0x1E + (input -'1'); 	/* The base of 1 is 0x1E */
	}

	switch(input){
	case '!':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x1E;break;
	case '@':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x1F;break;
	case '#':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x20;break;
	case '$':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x21;break;
	case '%':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x22;break;
	case '^':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x23;break;
	case '&':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x24;break;
	case '*':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x25;break;
	case '(': 
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x26;break;
	case ')':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x27;break;
	case '_':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x2D;break;
	case '+':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x2E;break;
	case '\n':report->keycode[0] = 0x28;break;	/* Enter Key */
	case '	':report->keycode[0] = 0x2B;break;	/* 0x2A ->backspace*/
	case ' ':report->keycode[0] = 0x2C;break;
	case '-':report->keycode[0] = 0x2D;break;
	case '=':report->keycode[0] = 0x2E;break;
	case '[':report->keycode[0] = 0x2F;break;
	case '{':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x2F;break;
	case ']':report->keycode[0] = 0x30;break;
	case '}':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x30;break;
	case '\\':report->keycode[0] = 0x31;break;
	case '|':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x31;break;
	case ';':report->keycode[0] = 0x33;break;
	case ':':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x33;break;
	case '\'':report->keycode[0] = 0x34;break;
	case '\"':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x34;break;
	case ',':report->keycode[0] = 0x36;break;
	case '<':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x36;
		break;
	case '.':report->keycode[0] = 0x37;break;
	case '>':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x37;
		break;
	case '/':report->keycode[0] = 0x38;break;
	case '?':
		set_modifier(&report->modifierKeys,LEFT_SHIFT);
		report->keycode[0] = 0x38;break;
	/* 	
		0x39 is Caps Locl 		0x3A ~ 0x45 : F1 ~ F12 		0x46 : PrintScreen 		0x47 : Scroll Lock 		0x48 : Pause 
	 	0x49 : Insert 			0x4A : Home 				0x4B : PageUp 			0x4C : Delete 			0x4D : End 
	 	0x4E : pageDown			0x4F : RightArrow			0x50 : LeftArrow		0x51 : DownArrow 		0x52 : UpArrow
	 	0x53 : Num Lock
	*/
	case 200:/* Window + r */
		set_modifier(&report->modifierKeys,LEFT_WINDOWS);
		report->keycode[0] = 0x15;break;
	case 201:	/* Window + F10 */
		set_modifier(&report->modifierKeys,LEFT_WINDOWS);
		report->keycode[0] = 0x44;break;
	case 202:
		set_modifier(&report->modifierKeys,LEFT_WINDOWS);
		report->keycode[0] = 0x85;break;
	default:
		// Not support
		break;
	}
	return report;
}

static void clear_report(struct key_report *report)
{
	memset(report, 0 ,sizeof(*report));
}

static long do_key_write(struct key_report *report)
{
	struct usb_request *req;
	int ret;
	struct key_dev *dev = _key_dev;

	if(!dev)
		return -ENODEV;

	if(!key_lock(&dev->write_excl))
		return -EBUSY;
	
	req = key_request_new(dev->ep_in, 16);
	if (!req)
		return -EFAULT;

	memcpy(req->buf,report,8);
	req->length = 8;
	req->status   = 0;
	req->zero     = 0;
	req->no_interrupt = 0;
	req->complete = key_complete;
	if(!dev->can_write)
		return 0;
	
	ret = usb_ep_queue(dev->ep_in, req, GFP_ATOMIC);
	if (ret < 0) {
		pr_debug("adb_write: xfer error %d\n", ret);
		atomic_set(&dev->error, 1);
	}
	
	key_unlock(&dev->write_excl);
	return 0;
}


static int input_letter(struct key_report *k_report,char letter)
{
	int count;
	int ret;

	KEYBOARD_SLEEP;
	count = 0;
letter_down:
	clear_report(k_report);
	generate_report(k_report,letter);
	ret = do_key_write(k_report);
	if(ret != 0){
		count++;
		if(count < TRY_TIMES)
			goto letter_down;
	}
	if(ret != 0)
		return LETTER_DOWN_ERR;

	
	KEYBOARD_SLEEP;
	count = 0;
letter_up:
	clear_report(k_report);
	k_report->keycode[0] = 0x00;
	ret = do_key_write(k_report);
	if(ret != 0){
		count++;
		if(count < TRY_TIMES)
			goto letter_up;
	}
	if(ret != 0)
		return LETTER_UP_ERR;

	return LETTER_OK;
}


static int input_windows(char *input_string){
	int ret = -1;
	struct key_report k_report;
	struct key_dev *dev = _key_dev;
	char *strptr = input_string;

	if(!dev)
		return -ENODEV;

	while(*strptr){
		ret = input_letter(&k_report,*strptr++);	// Enter 
		if(ret != LETTER_OK)
			return -1;
	}
	
	return 0;

}

static ssize_t keyboard_write (struct file * filp, const char __user * buf, size_t count, loff_t *pos)
{
	int ret = -1;
	struct key_dev *dev;
	struct android_keyboard_info key_info;
	char input_str[ANDROID_KEYBOARD_MAX_LEN];


	dev = filp->private_data;
	if(!dev)
		return -ENODEV;

	if(key_lock(&dev->write_excl))
		return -EBUSY;

	memset(input_str, 0, ANDROID_KEYBOARD_MAX_LEN);
	if(copy_from_user((void *)&key_info,buf,sizeof(struct android_keyboard_info))){
		ret = -EFAULT;
		goto failed;
	}
	
	if(key_info.str_len <= 0)
		return 0;
	
	if(key_info.str_len >= ANDROID_KEYBOARD_MAX_LEN)
		key_info.str_len = ANDROID_KEYBOARD_MAX_LEN - 1;

	if(copy_from_user((void *)input_str,key_info.input_string,key_info.str_len)){
		ret = -EFAULT;
		goto failed;
	}

	
	switch(key_info.mode){
	case KEY_WINDOWS: 
		ret = input_windows(input_str);break;
	case KEY_LINUX: 
		ret = 0;break;
	case KEY_MAC: 
		ret = 0;break;
	default: 
		ret = 0; break;
	}
	
failed:
	key_unlock(&dev->write_excl);
	return ret;
}


static int keyboard_open (struct inode *node, struct file *filp)
{
	if (!_key_dev)
		return -ENODEV;
	
	if (key_lock(&_key_dev->open_excl))
		return -EBUSY;

	filp->private_data = _key_dev;

	/* clear the error latch */
	atomic_set(&_key_dev->error, 0);
	return 0;
}

int keyboard_release(struct inode *node, struct file *filp)
{
	struct key_dev *dev = filp->private_data;

	if(!dev)
		return -ENODEV;

	key_unlock(&dev->open_excl);
	return 0;
}

/* file operations */
static const struct file_operations keyboard_fops = {
	.owner 		= THIS_MODULE,
	.write 		= keyboard_write,
	.open 		= keyboard_open,
	.release 	= keyboard_release,
};

static struct miscdevice keyboard_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = keyboard_shortname,
	.fops = &keyboard_fops,
};


