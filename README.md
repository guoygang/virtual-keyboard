# virtual-keyboard
Mobile phone virtual keyboard

键盘的报告描述符：<br>
uint8_t report_descriptor[] = {<br>
<tab>		0x05, 1, 		// Usage Page (1: Generic Desktop) <br>
		0x09, 6, 		// Usage (6: Keyboard)<br>
		0xA1, 1, 		// Collection (1: Application) <br>
		0x05, 7, 		// 	Usage page (7: Key Codes) <br>
		0x19, 224, 	// 	Usage Minimusm (224)<br>
		0x29, 231, 	// 	Usage Maximum (231)<br>
		0x15, 0, 		// 	Logical Minimum (0)<br>
		0x25, 1, 		// 	Logical Maximum (1)<br>
		0x75, 1, 		// 	Report Size (1)<br>
		0x95, 8, 		// 	Report Count (8)<br>
		0x81, 2 ,		// 	Input (Data,Variable,Absolute) <br>
		0x95, 1,		//	Report Count (1)<br>
		0x75, 8,		//	Report Size (8),<br>
		0x81, 1, 		// 	Input (Constant) = Reserved Byte<br>
		0x95, 5,		//	Report Count (5)<br>
		0x75, 1,		//	Report Size (1)<br>
		0x05, 8,		//	Usage Page (Page# for LEDs)<br>
		0x19, 1,		//	Usage Minimum (1)<br>
		0x29, 5,		//	Usage Maximum (5)<br>
		0x91, 2,		//	Output (Data, Variable, Absolute)<br>
		0x95, 1,		//	Report Count (1)<br>
		0x75, 3,		//	Report Size (3)<br>
		0x91, 1,		//	Output (Constant)<br>
		0x95, 6,		//	Report Count (6)<br>
		0x75, 8,		//	Report Size (8)<br>
		0x15, 0,		//	Logical Minimum (0)<br>
		0x25, 101,	//	Logical Maximum (101)<br>
		0x05, 7,		//	Usage Page (7: Key Codes)<br>
		0x19, 0,		//	Usage Minimum (0)<br>
		0x29, 101,	//	Usage Maximum (101)<br>
		0x81, 0,		//	Input (Data, Array)<br>
		0xC0, 		// End_Collection <br>
	};
  
	#define REPORT_DESC_LEN sizeof(hid_report_descriptor)<br>
