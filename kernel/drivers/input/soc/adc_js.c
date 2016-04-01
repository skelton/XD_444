/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * version V4.3
 * compatible PS3 and XBOX mode
 * change right stick range from -1 to 1
 * fixed self-test suspend release
 * add key and joystick self-test
 * add early suspend 
 * add hot key mirco
 * change to X-input mode
 * add exchange L1/L2 and R1/R2
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/slab.h>
#include "soc.h"


#define CENTER_TRY	5
#define VERTICAL	1
#define HOT		0
#define SELFTEST	1

#define LCD_SCREEN_X	1280
#define LCD_SCREEN_Y 	720
//#define VIEW_CENTREX	(LCD_SCREEN_X / 4 * 3)
//#define VIEW_CENTREY	(LCD_SCREEN_Y / 2)
#define VIEW_CENTREX  	960
#define VIEW_CENTREY	360
#if VERTICAL
//#define LCD_YXFACTOR	LCD_SCREEN_Y / LCD_SCREEN_X;
//#define LCD_XYFACTOR	LCD_SCREEN_X / LCD_SCREEN_Y;
#define LCD_YXFACTOR	9/16
#define LCD_XYFACTOR	16/9
#endif

#define TRACKING_ID	20
#define ADC_STEP	14
//#define ADC_STEP_FACTOR	ADC_STEP * 3 / 2;
#define ADC_STEP_FACTOR	2

#define ADC_MAX		0xFF
//#define HALF_ADC_MAX	(ADC_MAX / 2)
#define HALF_ADC_MAX	0x7F
#define MID_BLIND       16
#define JS_MAGNI_TIMES	68/10

#define	X1		1
#define	X2		2
#define	X3		3
#define	X4		4
#define	X5		5
#define	VX1		6
#define	VX2		4
#define	VX3		3
#define	VX4		2
#define	VX5		1


//#define AXIS_NUM sizeof(axis)/sizeof(axis[0])
//#if XBOX
#define AXIS_NUM 	8
//#else
//#define AXIS_NUM 	6
//#endif

//default mode, 0 is PS3 mode, 1 is XBOX mode
int ioctl_xbox = 1;
//default exchange, 0 is exchange, 1 is default 
int ioctl_exchange = 1;

//1, circle_x, circle_y, r, ax, ay, bx, by, xx, xy, yx, yy, lx, ly, rx, ry, l2x, l2y, r2x, r2y, view，view_x, view_y, leftx, lefty, rightx, righty, upx, upy, downx, downy l3x, l3y, r3x, r3y, selectx, selecty, startx, starty
//2, circle_x, circle_y, r, ax, ay, bx, by, xx, xy, yx, yy, lx, ly, rx, ry, l2x, l2y, r2x, r2y, view_x，view_y, view_r, leftx, lefty, rightx, righty, upx, upy, downx, downy l3x, l3y, r3x, r3y, selectx, selecty, startx, starty
static int key_param[39];

struct joystick_axis {
	unsigned char *name;
	int code;       
	int code2;       
};

struct game_key{
	unsigned char *name;
	int ps3_code;       
	int xbox_code;       
	int value;
	int old_value;
	int flag;
	int num;   	//touch param num
	int id;		//touch point id
	int gpio;
};

struct kp {
	struct input_dev *input_keytouch;
	struct input_dev *ps3_input_joystick;
	struct input_dev *xbox_input_joystick;
	struct work_struct work_update;
	struct timer_list timer;
	struct work_struct work_input_mode;
	struct class *config_class;
	struct device *config_dev;
	int config_major;
	char config_name[20];
	int js_value[AXIS_NUM];
	int js_flag[AXIS_NUM];
	int key_value[AXIS_NUM];
	int key_valid[AXIS_NUM];
	u8 value[AXIS_NUM];
	int stick_flag[2];
	int view_oldx, view_oldy;
	int view_count;
	int view_restart;
	int left_center_count, right_center_count;
	int suspend_flag;
	struct joystick_axis *ps3_axis;
	struct joystick_axis *xbox_axis;
	struct game_key *gamekeys;
	int keynum;
	int xbox;
        int input_mode_old;
};
static struct kp *gp_kp=NULL;

static int release = 1;
static int early_resume = 0;
int brake, gas;
int hatx, haty;

#if SELFTEST
/* selftest = 1 pass, can use key mapping function
 * selftest = 0 fail, cannot use key mapping function
 */
static int keytest = 0;
static int sticktest = 0;
#endif

static struct joystick_axis xbox_axis[] = {
	//name			code		code2
	{"LAS L2R",		ABS_X,		0},
	{"LAS U2D",		ABS_Y,		0},
	{"RAS L2R",		ABS_Z,		0},
	{"left trigger",	ABS_RX,		ABS_BRAKE},
	{"right trigger",	ABS_RY,		ABS_GAS},
	{"RAS U2D",		ABS_RZ,		0},
	{"DPAD L2R",		ABS_HAT0X,	0},
	{"DPAD U2D",		ABS_HAT0Y,	0},
};
/*
static struct joystick_axis xbox_axis[] = {
	//name			code		code2
	{"LAS L2R",		ABS_X,		0},
	{"LAS U2D",		ABS_Y,		0},
	{"RAS L2R",		ABS_RX,		0},
	{"left trigger",	ABS_Z,		0},
	{"right trigger",	ABS_RZ,		0},
	{"RAS U2D",		ABS_RY,		0},
	{"DPAD L2R",		ABS_HAT0X,	0},
	{"DPAD U2D",		ABS_HAT0Y,	0},
};
*/
static struct joystick_axis ps3_axis[] = {
	//name			code		code2
	{"LAS L2R",		ABS_X,		0},
	{"LAS U2D",		ABS_Y,		0},
	{"RAS L2R",		ABS_Z,		0},
	//{"left trigger",	ABS_BRAKE,	0},
	//{"right trigger",	ABS_GAS,	0},
	{"left trigger",	0x30,		0},
	{"right trigger",	0x31,		0},
	{"RAS U2D",		ABS_RZ,		0},
};

// ps3_code from Vendor_054c_Product_0268.kl
static struct game_key gamekeys[] = {
	//name		ps3_code	xbox_code	value	old_value	flag	num	id	gpio
	{"keya",	0x12e,	 	BTN_A,		0,	0,     		0, 	4,	2,	0},
	{"keyb",	0x12d,	 	BTN_B,		0,	0,     		0, 	6,	3,	0},
	{"keyx",	0x12f,	 	BTN_X,		0,	0,     		0, 	8,	4,	0},
	{"keyy",	0x12c,	 	BTN_Y,		0,	0,     		0, 	10,	5,	0},
	{"keyl",	0x12a,	 	BTN_TL,		0,	0,     		0, 	12,	6,	0},
	{"keyr",	0x12b,	 	BTN_TR,		0,	0,     		0, 	14,	7,	0},
	{"keyl2",	0x128,	 	BTN_TL2,	0,	0,     		0, 	16,	8,	0},
	{"keyr2",	0x129,	 	BTN_TR2,	0,	0,     		0, 	18,	9,	0},
	{"left",	0x127,	 	KEY_LEFT,	0,	0,     		0, 	23,	10,	0},
	{"right",	0x125,	 	KEY_RIGHT,	0,	0,     		0, 	25,	11,	0},
	{"up",		0x124,	 	KEY_UP,		0,	0,     		0, 	27,	12,	0},
	{"down",	0x126,	 	KEY_DOWN,	0,	0,     		0, 	29,	13,     0},
	{"keyl3",	0x121,	 	BTN_THUMBL,	0,	0,     		0, 	31,	14,	0},
	{"keyr3",	0x122,	 	BTN_THUMBR,	0,	0,     		0, 	33,	15,	0},
	{"start",	0x123,	 	BTN_START,	0,	0,     		0, 	35,	16,	0},
	{"select",	0x120,	 	BTN_SELECT,	0,	0,     		0, 	37,	17,	0},
	//{"hot_key",	KEY_F12,	KEY_F12	,	0,	0,     		0, 	0,	0,	0},
};

int ps3_input_device();
int xbox_input_device();
int ps3_uninput_device();
int xbox_uninput_device();

static void gpio_init(struct kp *kp, struct device_node *np)
{
	int i,ret;
	enum of_gpio_flags flags;
	
	for (i=0; i<kp->keynum; i++) {
		kp->gamekeys[i].gpio = of_get_named_gpio_flags(np, kp->gamekeys[i].name, 0, &flags);
		if (gpio_is_valid(kp->gamekeys[i].gpio)) {
			ret = gpio_request(kp->gamekeys[i].gpio, NULL);
			if (ret) {
				printk("failed to request GPIO%d for %s\n",kp->gamekeys[i].gpio, kp->gamekeys[i].name);
				continue;
			}
			gpio_direction_input(kp->gamekeys[i].gpio);
			//printk("initial %s success \n", kp->gamekeys[i].name);
		} else {
			kp->gamekeys[i].gpio = 0;
			//printk("initial %s is not use \n", kp->gamekeys[i].name);
		}
	}
}

//extern int select, start;
static void read_keys_value(struct kp *kp)
{
	int i;
	int tmp = 0;

	if (kp->suspend_flag) {
		for (i=0; i<kp->keynum; i++)
			kp->gamekeys[i].value = 0;

	} else {
		for (i=0; i<kp->keynum; i++) {
			if (kp->gamekeys[i].gpio) {
				if (ioctl_exchange != 1) {
					if (i == 4) 
						kp->gamekeys[4].value = gpio_get_value(kp->gamekeys[6].gpio) == 1 ? 0 : 1;
					else if (i == 5)
						kp->gamekeys[5].value = gpio_get_value(kp->gamekeys[7].gpio) == 1 ? 0 : 1;
					else if (i == 6)
						kp->gamekeys[6].value = gpio_get_value(kp->gamekeys[4].gpio) == 1 ? 0 : 1;
					else if (i == 7)
						kp->gamekeys[7].value = gpio_get_value(kp->gamekeys[5].gpio) == 1 ? 0 : 1;
					else
						kp->gamekeys[i].value = gpio_get_value(kp->gamekeys[i].gpio) == 1 ? 0 : 1;
				} else {
					kp->gamekeys[i].value = gpio_get_value(kp->gamekeys[i].gpio) == 1 ? 0 : 1;
				}
			} else {
				kp->gamekeys[i].value = 0;
			}
		}
		//kp->gamekeys[14].value = start;
		//kp->gamekeys[15].value = select;
	}

#if SELFTEST
	for (i=0; i<kp->keynum; i++) {
		if (kp->gamekeys[i].gpio) {
			tmp += kp->gamekeys[i].value;
		}
	}
	//TODO only use extern select and start add
	//tmp += start;
	//tmp += select;
	//end
	if (!tmp) {
		keytest = 1;
	} else {
		keytest = 0;
	}
#endif

}

static void js_report(struct kp *kp, int value, int id)
{
	struct input_dev *input_joystick;
	struct joystick_axis *axis;
	
	/**********************************************************/
	//PS3 and XBOX switch
	if (kp->xbox) {
		input_joystick = kp->xbox_input_joystick;
		axis = kp->xbox_axis;
	} else {
		input_joystick = kp->ps3_input_joystick;
		axis = kp->ps3_axis;
	}
	/**********************************************************/

	if (id >= AXIS_NUM) {
		printk("-------- axis is overflow --------\n");
		return ;
	}

	if (value == 0) {
		if (kp->js_flag[id]) {
			kp->js_flag[id] = 0;
			input_report_abs(input_joystick, axis[id].code, 0);
			if (axis[id].code2)
				input_report_abs(input_joystick, axis[id].code2, 0);
		}
	} else {
		kp->js_flag[id] = 1;
		input_report_abs(input_joystick, axis[id].code, value);
		if (axis[id].code2)
			input_report_abs(input_joystick, axis[id].code2, value);
	}
}

static void vtouch_report(struct kp *kp, long x, long y, int id)
{
#if VERTICAL
	long tmp;
#endif
	if (x != 0 || y != 0) {
#if VERTICAL
		x = LCD_SCREEN_X - x;
		x = x * LCD_YXFACTOR;
		y = y * LCD_XYFACTOR;
		tmp = x;
		x = y;
		y = tmp;
#endif
		input_report_key(kp->input_keytouch, BTN_TOUCH, 1);
		input_report_abs(kp->input_keytouch, ABS_MT_TRACKING_ID, id);
		input_report_abs(kp->input_keytouch, ABS_MT_POSITION_X, x);
		input_report_abs(kp->input_keytouch, ABS_MT_POSITION_Y, y);
		input_mt_sync(kp->input_keytouch);
	}
	release = 1;
}

static void keytouch_release(struct kp *kp)
{
	release = 0;
	input_report_key(kp->input_keytouch, BTN_TOUCH, 0);
	input_mt_sync(kp->input_keytouch);
	input_sync(kp->input_keytouch);
	//printk("-------------- release all point -----------------\n");
}

static void scan_joystick(struct kp *kp, int channel)
{
	int js_value;

	js_value = kp->value[channel];
	if (js_value >= 0) {
		if ((js_value >= HALF_ADC_MAX - MID_BLIND) && (js_value <= HALF_ADC_MAX + MID_BLIND)) {
			kp->js_value[channel] = 0;
		} else {
			if (js_value >= HALF_ADC_MAX)
				js_value = (js_value - HALF_ADC_MAX - MID_BLIND) * JS_MAGNI_TIMES;
			else
				js_value = (js_value - HALF_ADC_MAX + MID_BLIND) * JS_MAGNI_TIMES;
			if (js_value > ADC_MAX)
				js_value = ADC_MAX;
			if(js_value < -ADC_MAX)
				js_value = -ADC_MAX;
			kp->js_value[channel] = js_value;
			//printk("---------------------- %i js_value = %d -------------------\n", channel, js_value);
		}
	}
}


static void scan_left_joystick(struct kp *kp)
{
	if (kp->suspend_flag) {
		kp->value[0] = 0x7f;
		kp->value[1] = 0x7f;
	} else {
		//left_h, left_v, right_h, right_v
		soc_value(kp->value);
	}
	scan_joystick(kp, 0);
	scan_joystick(kp, 1);
}

static void scan_right_joystick(struct kp *kp)
{
	if (kp->suspend_flag) {
		kp->value[2] = 0x7f;
		kp->value[3] = 0x7f;
	} else {
		//left_h, left_v, right_h, right_v
		soc_value(kp->value);
	}
	scan_joystick(kp, 2);
	scan_joystick(kp, 3);
}

static void scan_joystick_touchmapping(struct kp *kp, int channel)
{
	int js_value;

	js_value = kp->value[channel];
	if (js_value >= 0) {
		if ((js_value >= HALF_ADC_MAX - MID_BLIND) && (js_value <= HALF_ADC_MAX + MID_BLIND)) {
			kp->key_valid[channel] = 0;
		} else {
			kp->key_valid[channel] = 1;

			if (js_value >= HALF_ADC_MAX)
				js_value = (js_value - HALF_ADC_MAX - MID_BLIND) * JS_MAGNI_TIMES / 2 + HALF_ADC_MAX;
			else
				js_value = (js_value - HALF_ADC_MAX + MID_BLIND) * JS_MAGNI_TIMES / 2 + HALF_ADC_MAX;

			if (js_value > ADC_MAX)
				js_value = ADC_MAX;
			if (js_value < 0)
				js_value = 0;
		}
		kp->key_value[channel] = js_value;
	}
}

static void scan_left_joystick_touchmapping(struct kp *kp)
{
	int tmp;
	if (kp->suspend_flag) {
		kp->value[1] = 0x7f;
		kp->value[0] = 0x7f;
	} else {
		//left_h, left_v, right_h, right_v
		soc_value(kp->value);
	}
	scan_joystick_touchmapping(kp, 1);
	scan_joystick_touchmapping(kp, 0);
#if !VERTICAL
        tmp=kp->key_value[1];
        kp->key_value[1]=kp->key_value[0];
        kp->key_value[0]=tmp; 
#endif

}

static void scan_right_joystick_touchmapping(struct kp *kp)
{
	if (kp->suspend_flag) {
		kp->value[2] = 0x7f;
		kp->value[3] = 0x7f;
	} else {
		//left_h, left_v, right_h, right_v
		soc_value(kp->value);
	}
	scan_joystick_touchmapping(kp, 2);
	scan_joystick_touchmapping(kp, 3);
}

static void ps3_report_joystick_key(struct kp *kp)
{
	int i;

	read_keys_value(kp);
	for (i = 0; i < kp->keynum; i++) {
		if(kp->gamekeys[i].value == kp->gamekeys[i].old_value) {
			if (kp->gamekeys[i].value == kp->gamekeys[i].flag) {
				if(kp->gamekeys[i].value) {
					//printk("%s press\n", kp->gamekeys[i].name);
					input_report_key(kp->ps3_input_joystick, kp->gamekeys[i].ps3_code, 1);
					input_mt_sync(kp->ps3_input_joystick);
					if(i == 6)
						brake = 0xff;
					if(i == 7)
						gas = 0xff;
					kp->gamekeys[i].flag = 0;
				} else {
					//printk("%s release\n", kp->gamekeys[i].name);
					input_report_key(kp->ps3_input_joystick, kp->gamekeys[i].ps3_code, 0);
					input_mt_sync(kp->ps3_input_joystick);
					if(i == 6)
						brake = 0;
					if(i == 7)
						gas = 0;
					kp->gamekeys[i].flag = 1;
				}
			}
		}
		kp->gamekeys[i].old_value = kp->gamekeys[i].value;
	}
}

static void xbox_report_joystick_key(struct kp *kp)
{
	int i;

	read_keys_value(kp);
	for (i = 0; i < kp->keynum; i++) {
		if(kp->gamekeys[i].value == kp->gamekeys[i].old_value) {
			if (kp->gamekeys[i].value == kp->gamekeys[i].flag) {
				if(kp->gamekeys[i].value) {
					//printk("%s press\n", kp->gamekeys[i].name);
					if(i >= 8 && i <= 11) {
						if(i == 8)
							hatx = -0xff;
						else if(i == 9)
							hatx = 0xff;
						if(i == 10)
							haty = -0xff;
						else if(i == 11)
							haty = 0xff;
					} else {
						input_report_key(kp->xbox_input_joystick, kp->gamekeys[i].xbox_code, 1);
						input_mt_sync(kp->xbox_input_joystick);
						if(i == 6)
							brake = 0xff;
						if(i == 7)
							gas = 0xff;
					}
					kp->gamekeys[i].flag = 0;
				} else {
					//printk("%s release\n", kp->gamekeys[i].name);
					if(i >= 8 && i <= 11) {
						if((i == 8) || (i == 9))
							hatx = 0;
						if((i == 10) || (i == 11))
							haty = 0;
					} else {
						input_report_key(kp->xbox_input_joystick, kp->gamekeys[i].xbox_code, 0);
						input_mt_sync(kp->xbox_input_joystick);
						if(i == 6)
							brake = 0;
						if(i == 7)
							gas = 0;
					}
					kp->gamekeys[i].flag = 1;
				}
			}
		}
		kp->gamekeys[i].old_value = kp->gamekeys[i].value;
	}
}

#if HOT
/*
static void report_hot_key(struct kp *kp)
{
	int i;

	kp->gamekeys[kp->keynum].value = gpio_get_value(RK30_PIN0_PC7) == 1 ? 0 : 1;
	i = kp->keynum;
	if(kp->gamekeys[i].value == kp->gamekeys[i].old_value) {
		if (kp->gamekeys[i].value == kp->gamekeys[i].flag) {
			if(kp->gamekeys[i].value) {
				input_report_key(kp->input_joystick, kp->gamekeys[i].code, 1);
				input_sync(kp->input_joystick);
				kp->gamekeys[i].flag = 0;
				//printk("%s press\n", kp->gamekeys[i].name);
			} else {
				input_report_key(kp->input_joystick, kp->gamekeys[i].code, 0);
				input_sync(kp->input_joystick);
				kp->gamekeys[i].flag = 1;
				//printk("%s release\n", kp->gamekeys[i].name);
			}
		}
	}
	kp->gamekeys[i].old_value = kp->gamekeys[i].value;
}
*/
#endif

static void report_keytouch_key(struct kp *kp)
{
	int i;

	read_keys_value(kp);
	for (i = 0; i < kp->keynum; i++) {
		if(kp->gamekeys[i].value == kp->gamekeys[i].old_value) {
			//if(kp->gamekeys[i].value && kp->gamekeys[i].point) {
			if(kp->gamekeys[i].value) {
				vtouch_report(kp, key_param[kp->gamekeys[i].num], key_param[kp->gamekeys[i].num + 1], kp->gamekeys[i].id);
				kp->gamekeys[i].flag = 0;
			} else {
				kp->gamekeys[i].flag = 1;
			}
		}
		kp->gamekeys[i].old_value = kp->gamekeys[i].value;
	}
}


static ssize_t key_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	char i;
	for (i=0; i<31; i++) {
		printk("key_param[%d] = %d \n", i, key_param[i]);
	}
	return 0;
}

static ssize_t key_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", \
			&key_param[0], &key_param[1], &key_param[2], &key_param[3], \
			&key_param[4], &key_param[5], &key_param[6], &key_param[7], \
			&key_param[8], &key_param[9], &key_param[10], &key_param[11], \
			&key_param[12], &key_param[13], &key_param[14], &key_param[15], \
			&key_param[16], &key_param[17], &key_param[18], &key_param[19], \
			&key_param[20], &key_param[21], &key_param[22], \
			&key_param[23], &key_param[24], &key_param[25], &key_param[26], \
			&key_param[27], &key_param[28], &key_param[29], &key_param[30], \
			&key_param[31], &key_param[32], &key_param[33], &key_param[34], \
			&key_param[35], &key_param[36], &key_param[37], &key_param[38]);
	if (key_param[0] == 1) {
		if (key_param[20] == 0)
			key_param[20] = X3;

		if (key_param[20] == X1)
			key_param[20] = VX1;
		else if (key_param[20] == X2)
			key_param[20] = VX2;
		else if (key_param[20] == X3)
			key_param[20] = VX3;
		else if (key_param[20] == X4)
			key_param[20] = VX4;
		else if (key_param[20] == X5)
			key_param[20] = VX5;
		else
			key_param[20] = VX3;

		if (key_param[21] == 0 || key_param[22] == 0) {
			key_param[21] = VIEW_CENTREX;
			key_param[22] = VIEW_CENTREY;
		}
		gp_kp->view_oldx = VIEW_CENTREX;
		gp_kp->view_oldy = VIEW_CENTREY;
	}

	return count;
}
static DEVICE_ATTR(key, S_IRWXUGO, key_read, key_write);

#if SELFTEST
static void test_stick(void)
{
	int i;
	int tmp = 0;

	//left_h, left_v, right_h, right_v
	soc_value(gp_kp->value);
	for (i=0; i<4; i++) {
		if ((gp_kp->value[i] < HALF_ADC_MAX - MID_BLIND * 2) || (gp_kp->value[i] > HALF_ADC_MAX + MID_BLIND * 2))
			tmp++;
	}

	if (!tmp)
		sticktest = 1;
	else
		sticktest = 0;
}

static ssize_t selftest_sys_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;

	if (gp_kp->suspend_flag) {
		count += sprintf(buf, "%d\n", 0);
	} else {
		test_stick();
		count += sprintf(buf, "%d\n", keytest * sticktest);
		//printk("keytst = %d sticktest = %d \n", keytest, sticktest);
	}

	return count;
}
static DEVICE_ATTR(selftest, S_IRWXUGO, selftest_sys_read, NULL);
#endif

static ssize_t mode_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	count += sprintf(buf, "%d\n", ioctl_xbox);
	return count;
}

static ssize_t mode_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &ioctl_xbox);
	printk("--- ioctl_xbox = %d ---\n", ioctl_xbox);
	return count;
}
static DEVICE_ATTR(mode, S_IRWXUGO, mode_read, mode_write);

static ssize_t exchange_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;
	count += sprintf(buf, "%d\n", ioctl_exchange);
	return count;
}

static ssize_t exchange_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%d", &ioctl_exchange);
	printk("--- ioctl_exchange = %d ---\n", ioctl_exchange);
	return count;
}
static DEVICE_ATTR(exchange, S_IRWXUGO, exchange_read, exchange_write);

static struct attribute *key_attr[] = {
	&dev_attr_key.attr,
	&dev_attr_mode.attr,
	&dev_attr_exchange.attr,
#if SELFTEST
	&dev_attr_selftest.attr,
#endif
	NULL
};
static struct attribute_group key_attr_group = {
	.name = NULL,
	.attrs = key_attr,
};

static long int x, y;
static void view_move(struct kp *kp)
{
	int tmp_x = 0;
	int tmp_y = 0;

	if(kp->view_count < key_param[20]) {
		x += kp->key_value[2];
		y += kp->key_value[3];
		kp->key_value[2] = kp->view_oldx;
		kp->key_value[3] = kp->view_oldy;
		kp->view_count++;
		return;
	} else {
		x /= key_param[20];	
		y /= key_param[20];	
		kp->view_count = 0;
	}
	
	tmp_x = (x - HALF_ADC_MAX) / ADC_STEP_FACTOR;
	tmp_y = (y - HALF_ADC_MAX) / ADC_STEP_FACTOR;

	if((tmp_x > 57) || (tmp_x < -57))
		tmp_x = tmp_x * 3 / 10;
	else
		tmp_x = tmp_x / 10;
	if((tmp_y > 50) || (tmp_y < -50))
		tmp_y = tmp_y * 3 / 10;
	else
		tmp_y = tmp_y / 10;

	tmp_x += kp->view_oldx;
	tmp_y += kp->view_oldy;

	if ((tmp_x > LCD_SCREEN_X) || (tmp_x < (LCD_SCREEN_X / 2 + 30)) || (tmp_y > LCD_SCREEN_Y) || (tmp_y < 0)) {
		gp_kp->view_restart = 1;
		kp->key_value[2] = 0;
		kp->key_value[3] = 0;
		kp->view_count = 0;
		x = 0;
		y = 0;
		return;
	}
	/*
	if (tmp_x > LCD_SCREEN_X) {
		tmp_x = LCD_SCREEN_X;
		tmp_y = kp->view_oldy;
	}
	if (tmp_x < (LCD_SCREEN_X / 2 + 30)) {
		tmp_x = (LCD_SCREEN_X / 2 + 30);
		tmp_y = kp->view_oldy;
	}
	if (tmp_y > LCD_SCREEN_Y) {
		tmp_y = LCD_SCREEN_Y;
		tmp_x = kp->view_oldx;
	}
	if (tmp_y < 0) {
		tmp_y = 0;
		tmp_x = kp->view_oldx;
	}
	*/

	kp->key_value[2] = tmp_x;
	kp->key_value[3] = tmp_y;
	kp->view_oldx = tmp_x;
	kp->view_oldy = tmp_y;
	x = 0;
	y = 0;
}

static void kp_timer_sr(unsigned long data)
{
	struct kp *kp_data=(struct kp *)data;
	schedule_work(&(kp_data->work_update));
	mod_timer(&kp_data->timer,jiffies+msecs_to_jiffies(10));
}

static void update_work_func(struct work_struct *work)
{
	struct kp *kp = container_of(work, struct kp, work_update);
	int i;
	int tmp = 1;
	long int tmpx, tmpy;

	struct input_dev *input_joystick;
	
	if (ioctl_xbox != kp->input_mode_old){
		if (ioctl_xbox) {
			xbox_input_device(kp);
			input_joystick = kp->xbox_input_joystick;
			kp->xbox = 1;
			ps3_uninput_device(kp);
		} else {
			ps3_input_device(kp);
			input_joystick = kp->ps3_input_joystick;
			kp->xbox = 0;
			xbox_uninput_device(kp);
		}
		kp->input_mode_old = kp->xbox;
	} else {
		if (ioctl_xbox) {
			input_joystick = kp->xbox_input_joystick;
			kp->xbox = 1;
		} else {
			input_joystick = kp->ps3_input_joystick;
			kp->xbox = 0;
		}
	}


#if HOT
	//report_hot_key(kp);
#endif

	/****************************** report joystick ********************************/
	//report left stick
	if (key_param[0] == 0 || key_param[1] < 0 || key_param[2] < 0) {
		scan_left_joystick(kp);
#if VERTICAL
		js_report(kp, kp->js_value[0], 0); //left
		js_report(kp, kp->js_value[1], 1); //left
#else
		js_report(kp, kp->js_value[1], 0); //left
		js_report(kp, kp->js_value[0], 1); //left
#endif
		input_sync(input_joystick);
	}
	//report right stick
	if (key_param[0] == 0) {
		scan_right_joystick(kp);
		js_report(kp, kp->js_value[3], 5); //right
		js_report(kp, kp->js_value[2], 2); //right
		input_sync(input_joystick);
	} else if (key_param[0] == 1 && (key_param[21] < 0 || key_param[22] < 0)) {
		scan_right_joystick(kp);
		js_report(kp, kp->js_value[3], 5); //right
		js_report(kp, kp->js_value[2], 2); //right
		input_sync(input_joystick);
	} else if (key_param[0] == 2 && (key_param[20] < 0 || key_param[21] < 0)) {
		scan_right_joystick(kp);
		js_report(kp, kp->js_value[3], 5); //right
		js_report(kp, kp->js_value[2], 2); //right
		input_sync(input_joystick);
	}
	//report keys
	if (key_param[0] == 0) {
		if (kp->xbox)
			xbox_report_joystick_key(kp);
		else
			ps3_report_joystick_key(kp);
		input_sync(input_joystick);

		js_report(kp, brake, 3); //brake
		js_report(kp, gas, 4); //gas
		if (kp->xbox) {
			js_report(kp, hatx, 6); //hatx, only for xbox mode
			js_report(kp, haty, 7); //haty, only for xbox mode
		}
		input_sync(input_joystick);
	}
	//end
	/*******************************************************************************/


	/*******************************************************************************/
	//report key mapping
	//left joystick
	if ((key_param[0] == 1 || key_param[0] == 2) && key_param[1] >= 0 && key_param[2] >= 0) {
		scan_left_joystick_touchmapping(kp);
		if ((kp->key_valid[0] == 1) || (kp->key_valid[1] == 1)) {
			kp->stick_flag[0] = 1;
			if(kp->left_center_count < CENTER_TRY) {
				if(kp->left_center_count == 0)
					vtouch_report(kp, key_param[1], key_param[2], 0);
				if(kp->left_center_count == 1)
					vtouch_report(kp, key_param[1] + 1, key_param[2], 0);
				if(kp->left_center_count == 2)
					vtouch_report(kp, key_param[1], key_param[2] + 1, 0);
				if(kp->left_center_count == 3)
					vtouch_report(kp, key_param[1] - 1, key_param[2], 0);
				if(kp->left_center_count == 4)
					vtouch_report(kp, key_param[1], key_param[2] - 1, 0);
				kp->left_center_count++;
			} else {
				tmpx = key_param[1] + (kp->key_value[0] - HALF_ADC_MAX) * (long int)key_param[3] / HALF_ADC_MAX;
				tmpy = key_param[2] + (kp->key_value[1] - HALF_ADC_MAX) * (long int)key_param[3] / HALF_ADC_MAX;
				vtouch_report(kp, tmpx, tmpy, 0);
			}
		} else if (kp->stick_flag[0] == 1) {
			kp->stick_flag[0] = 0;
			kp->left_center_count = 0;
		}
	} else if (key_param[0] == 1 || key_param[0] == 2) {
		kp->stick_flag[0] = 0;
		kp->left_center_count = 0;
	}


	//right joystick
	if (key_param[0] == 1 && key_param[21] >= 0 && key_param[22] >= 0) { //mode 1
		if (kp->view_restart == 1) {
			kp->view_restart = 0;
			kp->key_valid[2] = 0;
			kp->key_valid[3] = 0;
		} else {
			scan_right_joystick_touchmapping(kp);
		}
		if ((kp->key_valid[2] == 1) || (kp->key_valid[3] == 1)) {
			kp->stick_flag[1] = 1;
			if(!kp->right_center_count) {
				vtouch_report(kp, VIEW_CENTREX, VIEW_CENTREY, 1);
				kp->right_center_count = 1;
			} else {
				view_move(kp);
				vtouch_report(kp, kp->key_value[2], kp->key_value[3], 1);
			}
		} else if (kp->stick_flag[1] == 1) {
			kp->stick_flag[1] = 0;
			kp->right_center_count = 0;
			kp->view_oldx = VIEW_CENTREX;
			kp->view_oldy = VIEW_CENTREY;
		}
	} else if (key_param[0] == 2 && key_param[20] >= 0 && key_param[21] >= 0) { //mode 2
		scan_right_joystick_touchmapping(kp);
		if ((kp->key_valid[2] == 1) || (kp->key_valid[3] == 1)) {
			kp->stick_flag[1] = 1;
			if(kp->right_center_count < CENTER_TRY) {
				if(kp->right_center_count == 0)
					vtouch_report(kp, key_param[20], key_param[21], 1);
				if(kp->right_center_count == 1)
					vtouch_report(kp, key_param[20] + 1, key_param[21], 1);
				if(kp->right_center_count == 2)
					vtouch_report(kp, key_param[20], key_param[21] + 1, 1);
				if(kp->right_center_count == 3)
					vtouch_report(kp, key_param[20] - 1, key_param[21], 1);
				if(kp->right_center_count == 4)
					vtouch_report(kp, key_param[20], key_param[21] - 1, 1);
				kp->right_center_count++;
			} else {
				tmpx = key_param[20] +  (kp->key_value[2] - HALF_ADC_MAX) * (long int)key_param[22] / HALF_ADC_MAX;
				tmpy = key_param[21] +  (kp->key_value[3] - HALF_ADC_MAX) * (long int)key_param[22] / HALF_ADC_MAX;
				vtouch_report(kp, tmpx, tmpy, 1);
			}
		} else if (kp->stick_flag[1] == 1) {
			kp->stick_flag[1] = 0;
			kp->right_center_count = 0;
		}
	} else if (key_param[0] == 1 || key_param[0] == 2) {
		kp->stick_flag[1] = 0;
		kp->right_center_count = 0;
	}
	//end
	
	if ((key_param[0] == 1 || key_param[0] == 2)) {
		report_keytouch_key(kp);
		input_sync(kp->input_keytouch);
		if (release && (kp->stick_flag[0] == 0) && (kp->stick_flag[1] ==0)) {
			for (i = 0; i < kp->keynum; i++) {
				tmp = (tmp * kp->gamekeys[i].flag);
			}
			if (tmp)
				keytouch_release(kp);
		}
	}
	/*******************************************************************************/
}

static int selftest_open(struct inode *inode, struct file *file)
{
	return 0;
}
#if SELFTEST
static ssize_t selftest_read(struct file *file, char __user *buf, size_t count,loff_t *ppos)
{
	char *rbuf;

	test_stick();
	rbuf = kzalloc(count, GFP_KERNEL);
	rbuf[0] = keytest * sticktest;
	if (copy_to_user(buf, rbuf, count)) {
		return -EFAULT;
	}

	return 0;
}
#endif
static int selftest_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations keypad_fops = {
	.owner = THIS_MODULE,
	.open = selftest_open,
#if SELFTEST
	.read = selftest_read,
#endif
	.release = selftest_release,
};

static int register_keypad_dev(struct kp  *kp)
{
	int ret=0;
	strcpy(kp->config_name,"selftest");
	ret=register_chrdev(0, kp->config_name, &keypad_fops);
	if (ret<=0) {
		printk("register char device error\r\n");
		return  ret ;
	}
	kp->config_major=ret;
	//printk("adc keypad major:%d\r\n",ret);
	kp->config_class=class_create(THIS_MODULE,kp->config_name);
	kp->config_dev=device_create(kp->config_class, NULL, MKDEV(kp->config_major,0), NULL, kp->config_name);

	return ret;
}

static int adc_early_suspend(struct kp *kp)
{
	printk("%s\n",__func__);
	
	del_timer_sync(&kp->timer);
	//keytouch_release(kp);
	//input_sync(kp->input_joystick);
	kp->suspend_flag = 1;
	return 0;
}

static int adc_early_resume(struct kp *kp)
{
	printk("%s\n",__func__);
	if (early_resume) {
		setup_timer(&kp->timer, kp_timer_sr, kp) ;
		mod_timer(&kp->timer, jiffies+msecs_to_jiffies(100));
		kp->suspend_flag = 0;
	} else {
		early_resume = 1;
	}

	return 0;
}

static int joystick_fb_event_notify(struct notifier_block *self, unsigned long action, void *data)
{
        struct fb_event *event = data;
        int blank_mode = *((int *)event->data);

        if (action == FB_EARLY_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
                                break;
                        default:
                                adc_early_suspend(gp_kp);
                                break;
                }
        }
        else if (action == FB_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
                                adc_early_resume(gp_kp);
                                break;
                        default:
                                break;
                }
        }
        return NOTIFY_OK;
}

static struct notifier_block joystick_fb_notifier = {
        .notifier_call = joystick_fb_event_notify,
};

int ps3_input_device(struct kp *kp)
{
	int i, ret;

	//register joystick
	kp->ps3_input_joystick = input_allocate_device();
	if (!kp->ps3_input_joystick) {
		printk("---------- allocate ps3_input_joystick fail ------------\n");
		kfree(kp);
		input_free_device(kp->ps3_input_joystick);
		return -ENOMEM;
	}

	for (i = 0; i < kp->keynum; i++)
		set_bit(kp->gamekeys[i].ps3_code, kp->ps3_input_joystick->keybit);
	//for hot key
	set_bit(kp->gamekeys[kp->keynum].ps3_code, kp->ps3_input_joystick->keybit);

	set_bit(EV_REP, kp->ps3_input_joystick->evbit);
	set_bit(EV_KEY, kp->ps3_input_joystick->evbit);
	set_bit(EV_ABS, kp->ps3_input_joystick->evbit);
	set_bit(EV_SYN, kp->ps3_input_joystick->evbit);

	//PS3 joystick mode
	//kp->input_joystick->name = "Sony PLAYSTATION(R)3 Controller";
	kp->ps3_input_joystick->name = "PLAYSTATION(R)3";
	kp->ps3_input_joystick->id.vendor = 0x054C;
	kp->ps3_input_joystick->id.product = 0x0268;
	input_set_abs_params(kp->ps3_input_joystick, ABS_X, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->ps3_input_joystick, ABS_Y, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->ps3_input_joystick, ABS_Z, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->ps3_input_joystick, ABS_RZ, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->ps3_input_joystick, 0x30, 0, 0xFF, 0, 0);
	input_set_abs_params(kp->ps3_input_joystick, 0x31, 0, 0xFF, 0, 0);
	kp->ps3_input_joystick->id.bustype = BUS_USB;

	kp->ps3_input_joystick->id.version = 0x001;
	kp->ps3_input_joystick->rep[REP_DELAY]=0xffffffff;
	kp->ps3_input_joystick->rep[REP_PERIOD]=0xffffffff;
	kp->ps3_input_joystick->keycodesize = sizeof(unsigned short);
	kp->ps3_input_joystick->keycodemax = 0x1ff;
	ret = input_register_device(kp->ps3_input_joystick);
	if (ret < 0) {
		printk(KERN_ERR "register ps3_input_joystick device fail\n");
		kfree(kp);
		input_free_device(kp->ps3_input_joystick);
		return -EINVAL;
	}

	return 0;
}

int xbox_input_device(struct kp *kp)
{
	int i, ret;

	//register joystick
	kp->xbox_input_joystick = input_allocate_device();
	if (!kp->xbox_input_joystick) {
		printk("---------- allocate xbox_input_joystick fail ------------\n");
		kfree(kp);
		input_free_device(kp->xbox_input_joystick);
		return -ENOMEM;
	}

	for (i = 0; i < kp->keynum; i++)
		set_bit(kp->gamekeys[i].xbox_code, kp->xbox_input_joystick->keybit);
	//for hot key
	set_bit(kp->gamekeys[kp->keynum].xbox_code, kp->xbox_input_joystick->keybit);

	set_bit(EV_REP, kp->xbox_input_joystick->evbit);
	set_bit(EV_KEY, kp->xbox_input_joystick->evbit);
	set_bit(EV_ABS, kp->xbox_input_joystick->evbit);
	set_bit(EV_SYN, kp->xbox_input_joystick->evbit);

	//360 joystick mode
	kp->xbox_input_joystick->name = "XBOX";
	kp->xbox_input_joystick->id.vendor = 0x02D6;
	kp->xbox_input_joystick->id.product = 0x89E5;
	input_set_abs_params(kp->xbox_input_joystick, ABS_X, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_Y, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_Z, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RZ, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RX, 0, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RY, 0, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_BRAKE, 0, ADC_MAX, 0, 0);
        input_set_abs_params(kp->xbox_input_joystick, ABS_GAS, 0, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_HAT0X, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_HAT0Y, -ADC_MAX, ADC_MAX, 0, 0);
	kp->xbox_input_joystick->id.bustype = BUS_USB;

	kp->xbox_input_joystick->id.version = 0x001;
	kp->xbox_input_joystick->rep[REP_DELAY]=0xffffffff;
	kp->xbox_input_joystick->rep[REP_PERIOD]=0xffffffff;
	kp->xbox_input_joystick->keycodesize = sizeof(unsigned short);
	kp->xbox_input_joystick->keycodemax = 0x1ff;
	ret = input_register_device(kp->xbox_input_joystick);
	if (ret < 0) {
		printk(KERN_ERR "register xbox_input_joystick device fail\n");
		kfree(kp);
		input_free_device(kp->xbox_input_joystick);
		return -EINVAL;
	}

	return 0;
}

/*
int xbox_input_device(struct kp *kp)
{
	int i, ret;

	//register joystick
	kp->xbox_input_joystick = input_allocate_device();
	if (!kp->xbox_input_joystick) {
		printk("---------- allocate xbox_input_joystick fail ------------\n");
		kfree(kp);
		input_free_device(kp->xbox_input_joystick);
		return -ENOMEM;
	}

	for (i = 0; i < kp->keynum; i++)
		set_bit(kp->gamekeys[i].xbox_code, kp->xbox_input_joystick->keybit);
	//for hot key
	set_bit(kp->gamekeys[kp->keynum].xbox_code, kp->xbox_input_joystick->keybit);

	set_bit(EV_REP, kp->xbox_input_joystick->evbit);
	set_bit(EV_KEY, kp->xbox_input_joystick->evbit);
	set_bit(EV_ABS, kp->xbox_input_joystick->evbit);
	set_bit(EV_SYN, kp->xbox_input_joystick->evbit);

	//360 joystick mode
	kp->xbox_input_joystick->name = "Microsoft X-Box 360";
	kp->xbox_input_joystick->id.vendor = 0x045E;
	kp->xbox_input_joystick->id.product = 0x028E;
	input_set_abs_params(kp->xbox_input_joystick, ABS_X, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_Y, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_Z, 0, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RZ, 0, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RX, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_RY, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_HAT0X, -ADC_MAX, ADC_MAX, 0, 0);
	input_set_abs_params(kp->xbox_input_joystick, ABS_HAT0Y, -ADC_MAX, ADC_MAX, 0, 0);
	kp->xbox_input_joystick->id.bustype = BUS_USB;

	kp->xbox_input_joystick->id.version = 0x001;
	kp->xbox_input_joystick->rep[REP_DELAY]=0xffffffff;
	kp->xbox_input_joystick->rep[REP_PERIOD]=0xffffffff;
	kp->xbox_input_joystick->keycodesize = sizeof(unsigned short);
	kp->xbox_input_joystick->keycodemax = 0x1ff;
	ret = input_register_device(kp->xbox_input_joystick);
	if (ret < 0) {
		printk(KERN_ERR "register xbox_input_joystick device fail\n");
		kfree(kp);
		input_free_device(kp->xbox_input_joystick);
		return -EINVAL;
	}

	return 0;
}
*/

int ps3_uninput_device(struct kp *kp)
{
	input_unregister_device(kp->ps3_input_joystick);
	input_free_device(kp->ps3_input_joystick);
}

int xbox_uninput_device(struct kp *kp)
{
	input_unregister_device(kp->xbox_input_joystick);
	input_free_device(kp->xbox_input_joystick);
}

static int adc_probe(struct platform_device *pdev)
{
	struct kp *kp;
	struct device_node *np = pdev->dev.of_node;
	int i, ret;
	s8 phys[32];

	kp = kzalloc(sizeof(struct kp), GFP_KERNEL);
	if (!kp) {
		kfree(kp);
		return -ENOMEM;
	}
	gp_kp=kp;

	kp->stick_flag[0] = 0;
	kp->stick_flag[1] = 0;
	kp->view_oldx = 0;
	kp->view_oldy = 0;
	kp->left_center_count = 0;
	kp->right_center_count = 0;
	kp->view_restart = 0;
	for (i=0; i<AXIS_NUM; i++) {
		kp->js_value[i] = 0;
		kp->js_flag[i] = 0;
		kp->key_value[i] = 0;
		kp->key_valid[i] = 0;
	}

	/************************************************************************************/
	//register keytouch
	kp->input_keytouch = input_allocate_device();
	if (!kp->input_keytouch) {
		printk("---------- allocate input_keytouch fail ------------\n");
		kfree(kp);
		input_free_device(kp->input_keytouch);
		return -ENOMEM;
	}

	set_bit(BTN_TOUCH, kp->input_keytouch->keybit);
	set_bit(EV_REP, kp->input_keytouch->evbit);
	set_bit(EV_KEY, kp->input_keytouch->evbit);
	set_bit(EV_ABS, kp->input_keytouch->evbit);
	set_bit(EV_SYN, kp->input_keytouch->evbit);
	set_bit(ABS_MT_TOUCH_MAJOR, kp->input_keytouch->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, kp->input_keytouch->absbit);
	set_bit(ABS_MT_POSITION_X, kp->input_keytouch->absbit);
	set_bit(ABS_MT_POSITION_Y, kp->input_keytouch->absbit);
	set_bit(ABS_MT_TRACKING_ID, kp->input_keytouch->absbit);
	input_set_abs_params(kp->input_keytouch, ABS_MT_POSITION_X, 0, LCD_SCREEN_X, 0, 0);
	input_set_abs_params(kp->input_keytouch, ABS_MT_POSITION_Y, 0, LCD_SCREEN_Y, 0, 0);
	input_set_abs_params(kp->input_keytouch, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(kp->input_keytouch, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(kp->input_keytouch, ABS_MT_TRACKING_ID, 0, TRACKING_ID, 0, 0);

	sprintf(phys, "input/ts");
	kp->input_keytouch->name = "ADC keytouch";
	kp->input_keytouch->phys = phys;
	kp->input_keytouch->dev.parent = &pdev->dev;
	kp->input_keytouch->id.bustype = BUS_ISA;
	kp->input_keytouch->id.vendor = 0x0001;
	kp->input_keytouch->id.product = 0x0001;
	kp->input_keytouch->id.version = 0x100;
	kp->input_keytouch->rep[REP_DELAY]=0xffffffff;
	kp->input_keytouch->rep[REP_PERIOD]=0xffffffff;
	kp->input_keytouch->keycodesize = sizeof(unsigned short);
	kp->input_keytouch->keycodemax = 0x1ff;

	ret = input_register_device(kp->input_keytouch);
	if (ret < 0) {
		printk(KERN_ERR "register input_keytouch device fail\n");
		kfree(kp);
		input_free_device(kp->input_keytouch);
		return -EINVAL;
	}
	/************************************************************************************/
	struct device *dev = &pdev->dev;
	sysfs_create_group(&dev->kobj, &key_attr_group);
	platform_set_drvdata(pdev, kp);
	register_keypad_dev(kp);
	fb_register_client(&joystick_fb_notifier);

	kp->xbox = ioctl_xbox;
        kp->input_mode_old = kp->xbox;
	kp->keynum = sizeof(gamekeys)/sizeof(gamekeys[0]);
	kp->gamekeys = gamekeys;

	kp->xbox_axis = xbox_axis;
	kp->ps3_axis = ps3_axis;
	if (ioctl_xbox) {
		ret = xbox_input_device(kp);
		if (ret != 0) {
			printk("register XBOX input device error!");
			return -EINVAL;
		}
	} else {
		ret = ps3_input_device(kp);
		if (ret != 0) {
			printk("register PS3 input device error!");
			return -EINVAL;
		}
	}

	gpio_init(kp, np);

	INIT_WORK(&(kp->work_update), update_work_func);
	setup_timer(&kp->timer, kp_timer_sr, kp) ;
	mod_timer(&kp->timer, jiffies+msecs_to_jiffies(20));

	return 0;
}

static int adc_remove(struct platform_device *pdev)
{
	struct kp *kp = platform_get_drvdata(pdev);

	input_unregister_device(kp->input_keytouch);
	input_unregister_device(kp->ps3_input_joystick);
	input_unregister_device(kp->xbox_input_joystick);
	input_free_device(kp->input_keytouch);
	input_free_device(kp->ps3_input_joystick);
	input_free_device(kp->xbox_input_joystick);

	unregister_chrdev(kp->config_major,kp->config_name);
	if(kp->config_class)
	{
		if(kp->config_dev)
			device_destroy(kp->config_class,MKDEV(kp->config_major,0));
		class_destroy(kp->config_class);
	}

	gp_kp = NULL;
	kfree(kp);

	return 0;
}


static struct of_device_id adcjs_joystick_ids[] = {
        { .compatible = "adcjs" },
};

static struct platform_driver adc_driver = {
	.probe      = adc_probe,
	.remove     = adc_remove,
	.driver     = {
		.name   = "mx-adcjs",
		.of_match_table = of_match_ptr(adcjs_joystick_ids),
	},
};

static int __init adc_init(void)
{
	return platform_driver_register(&adc_driver);
}

static void __exit adc_exit(void)
{
	platform_driver_unregister(&adc_driver);
}

//module_init(adc_init);
late_initcall(adc_init);
module_exit(adc_exit);

MODULE_AUTHOR("Samty");
MODULE_DESCRIPTION("ADC Joystick Driver");
MODULE_LICENSE("GPL");

