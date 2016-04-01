/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *	note: only support mulititouch	Wenfs 2010-10-01
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/input/mt.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/fb.h>
#include <linux/backlight.h>


#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif


#if 0
#define fts_dbg(dev, format, arg...)		\
	dev_printk(KERN_INFO , dev , format , ## arg)
#else
#define fts_dbg(dev, format, arg...)
#endif

#define FT_RESET_DLY		20
#define FTS_NAME		"ft5x0x_ts"
#define FTS_I2C_RATE		(200*1000)
#define MAX_POINT		5
#define PRESS_MAX		255
#define UPDATE			0

#define FTS_REPORT_RATE		0x0c //0x06 // report_rate = (FTS_REPORT_RATE)*(10Hz)
#define FTS_VALID_THRES		0x0a // valid_tresshold = 0x0a * 4

#define FTS_REG_THRES		0x80         /* Thresshold, the threshold be low, the sensitivy will be high */
#define FTS_REG_REPORT_RATE	0x88         /* **************report rate, in unit of 10Hz **************/
#define FTS_REG_PMODE		0xA5         /* Power Consume Mode 0 -- active, 1 -- monitor, 3 -- sleep */    
#define FS_REG_FIRMID		0xA6         /* ***************firmware version **********************/
#define FS_REG_NOISE_MODE	0xb2         /* to enable or disable power noise, 1 -- enable, 0 -- disable */

//FT5X0X_REG_PMODE
enum {
	FTS_PMODE_ACTIVE  = 0x00,
	FTS_PMODE_MONITOR,
	FTS_PMODE_STANDBY,
	FTS_PMODE_HIBERNATE,
};

struct fts_event{
	u16 x;
	u16 y;
	u16 pressure;
	s16 id;
	u8 flag;
};
struct fts_data {
	int irq;
	int x_max;
    int y_max;
	int reset_gpio;
	u8 touch_point;
	u16 down_count[MAX_POINT];
	u8 flags[MAX_POINT];

	struct i2c_client *client;
	struct device *dev;
	struct input_dev *input_dev;
	int (*platform_sleep)(void);
    	int (*platform_wakeup)(void);
	int (*platform_init_hw)(void);
    	void (*platform_deinit_hw)(void);
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend;
#endif
};

struct fts_data *fb_ts;

static int fts_i2c_read(struct fts_data *ts, const u8 reg, u8 *buf, int len)
{
	int ret = 0;
	
    struct i2c_msg xfer_msg[2];

    xfer_msg[0].addr = ts->client->addr;
    xfer_msg[0].len = 1;
    xfer_msg[0].flags = ts->client->flags & I2C_M_TEN;
    xfer_msg[0].buf = &reg;
    xfer_msg[0].scl_rate = FTS_I2C_RATE; 

    xfer_msg[1].addr = ts->client->addr;
    xfer_msg[1].len = len;
    xfer_msg[1].flags |= I2C_M_RD;
    xfer_msg[1].buf = buf;
    xfer_msg[1].scl_rate = FTS_I2C_RATE; 

	ret = i2c_transfer(ts->client->adapter, xfer_msg, ARRAY_SIZE(xfer_msg));

	if(ret < 0)
		fts_dbg(ts->dev, "%s error, reg: 0x%x\n", __func__, reg);

	ret = (ret < 0)?ret:0;

	return ret;
}
static int fts_i2c_write(struct fts_data *ts, const u8 reg,  u8 *buf, int len)
{
	int ret  = 0;
    struct i2c_msg xfer_msg[2];
    buf[0] = reg;

    xfer_msg[0].addr = ts->client->addr;
    xfer_msg[0].len = len + 1;
    xfer_msg[0].flags = ts->client->flags & I2C_M_TEN;
    xfer_msg[0].buf = buf;
    xfer_msg[0].scl_rate = FTS_I2C_RATE; 
	
	ret = i2c_transfer(ts->client->adapter, xfer_msg, 1);
	if(ret < 0)
		fts_dbg(ts->dev, "%s error, reg: 0x%x\n", __func__, reg);

	ret = (ret < 0)?ret:0;

	return ret;
}
#define    FTS_PACKET_LENGTH        128
static u8 CTPM_FW[]=
{
#include "ft_app_5306.i"
};

typedef enum
{
	ERR_OK,
	ERR_MODE,
	ERR_READID,
	ERR_ERASE,
	ERR_STATUS,
	ERR_ECC,
	ERR_DL_ERASE_FAIL,
	ERR_DL_PROGRAM_FAIL,
	ERR_DL_VERIFY_FAIL,
	ERR_RESET_FM
}E_UPGRADE_ERR_TYPE;

static int fts_ctpm_read(struct fts_data *ts, char *buf, int len)
{
	int ret  = 0;

	ret = i2c_master_recv(ts->client,  buf, len );
	
	if(ret < 0)
		fts_dbg(ts->dev, "%s error\n", __func__);

	ret = (ret < 0)?ret:0;

	return ret;
}
static int fts_ctpm_write(struct fts_data *ts, char *buf, int len)
{
	int ret  = 0;

	ret = i2c_master_send(ts->client,  buf, len );
	
	if(ret < 0)
		fts_dbg(ts->dev, "%s error\n", __func__);

	ret = (ret < 0)?ret:0;

	return ret;
}

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(struct fts_data *ts, u8* pbt_buf, int dw_lenth)
{
	char val[4];
	int retry = 10, ret = 0, tmp;
	int  i, j, i_is_new_protocol = 0, packet_number;
	u8  packet_buf[FTS_PACKET_LENGTH + 6];
	u8  bt_ecc = 0;

	val[0] = 0xaa;
	fts_i2c_write(ts, 0xfc, val, 1);
	mdelay(50);
	val[0] = 0x55;
	fts_i2c_write(ts, 0xfc, val, 1);
	fts_dbg(ts->dev, "[TSP] Step 1: Reset CTPM\n");
	mdelay(10);
	fts_dbg(ts->dev, "[TSP] Step 2:enter new update mode\n");

	val[0] = 0x55;
	val[1] = 0xaa;
	do{
		ret = fts_ctpm_write(ts, val, 2);
	}while(retry-- && (ret < 0));

	if (retry > 0)
        	i_is_new_protocol = 1;

	val[0] = 0x90;
	val[1] = 0x00;
	val[2] = 0x00;
	val[3] = 0x00;
	fts_ctpm_write(ts, val, 4);

	fts_ctpm_read(ts, val, 2);
	fts_dbg(ts->dev, "[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",val[0],val[1]);
	if(val[0] != 0x79 || val[1] != 0x03)
		return -ERR_READID;
	
	if(i_is_new_protocol)
		val[0] = 0x61;
	else
		val[0] = 0x60;
	
	val[1] = 0x00;
	val[2] = 0x00;
	val[3] = 0x00;
	fts_ctpm_write(ts, val, 1);

	mdelay(1500);
	fts_dbg(ts->dev,"[TSP] Step 4: erase. \n");

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	fts_dbg(ts->dev,"[TSP] Step 5: start upgrade, packet_number = %d\n", packet_number);

	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;

	for(i = 0; i < packet_number; i++){
		tmp = i * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(tmp >> 8);
		packet_buf[3] = (u8)tmp ;
		tmp = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(tmp >> 8);
		packet_buf[5] = (u8)(tmp);

		for(j = 0; j < FTS_PACKET_LENGTH; j++){
			packet_buf[6+j] = pbt_buf[i*FTS_PACKET_LENGTH + j];
			bt_ecc ^= packet_buf[6+j];
		}
		fts_ctpm_write(ts, packet_buf, FTS_PACKET_LENGTH + 6);
		mdelay(FTS_PACKET_LENGTH/6 + 1);

		if ((i * FTS_PACKET_LENGTH % 1024) == 0)
        	{
              		fts_dbg(ts->dev, "[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)i) * FTS_PACKET_LENGTH);
        	}
	}
	
	if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
	{
		tmp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(tmp>>8);
		packet_buf[3] = (u8)tmp;
		
		tmp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(tmp>>8);
		packet_buf[5] = (u8)tmp;

		for (i=0;i<tmp; i++)
		{
			packet_buf[6+i] = pbt_buf[packet_number*FTS_PACKET_LENGTH + i]; 
			bt_ecc ^= packet_buf[6+i];
		}

		fts_ctpm_write(ts, packet_buf, tmp + 6);    
		mdelay(20);
	}

	for (i = 0; i<6; i++)
	{
		tmp = 0x6ffa + i;
		packet_buf[2] = (u8)(tmp>>8);
		packet_buf[3] = (u8)tmp;
        	packet_buf[4] = (u8)(1>>8);
        	packet_buf[5] = (u8)1;
        	packet_buf[6] = pbt_buf[dw_lenth + i]; 
        	bt_ecc ^= packet_buf[6];

        	fts_ctpm_write(ts, packet_buf,7);  
        	mdelay(20);
    	}

	val[0] = 0xcc;
	val[1] = 0x00;
	val[2] = 0x00;
	val[3] = 0x00;
	fts_ctpm_write(ts, val, 1);

	fts_ctpm_read(ts, val, 1);
	fts_dbg(ts->dev,"[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", val[0], bt_ecc);

	if(val[0] != bt_ecc)
		return -ERR_ECC;

	val[0] = 0x07;
	val[1] = 0x00;
	val[2] = 0x00;
	val[3] = 0x00;
	fts_ctpm_write(ts, val, 1);
	
	mdelay(100);//100ms
	fts_dbg(ts->dev, "[TSP] Step 1: Reset new FM\n");
	fts_i2c_read(ts, 0xfc, val, 1);
	if(val[0] == 0x01){
		val[0] = 0x04;
		fts_i2c_write(ts, 0xfc, val, 1);
		mdelay(2500);//2500ms
		do{
			fts_i2c_read(ts, 0xfc, val, 1);
			mdelay(100);//100ms
		}while(retry-- && val[0] != 1);

		if(retry <= 0)
			return -ERR_RESET_FM;
	}
	
	fts_dbg(ts->dev, "[TSP] %s ok\n", __func__);	
	return ERR_OK;
}
int fts_ctpm_fw_upgrade_with_i_file(struct fts_data *ts)
{
	u8* pbt_buf = 0;
	int ret;
    
	pbt_buf = CTPM_FW;
	ret =  fts_ctpm_fw_upgrade(ts, pbt_buf, sizeof(CTPM_FW));
   
	return ret;
}
unsigned char fts_ctpm_get_upg_ver(void)
{
	unsigned int ui_sz;
	
	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2)
	{
		return CTPM_FW[ui_sz - 2];
	}
	else
		return 0xff; 
 
}
static int fts_update_config(struct fts_data *ts)
{
	char old_ver, new_ver;
	int ret = 0;

	ret = fts_i2c_read(ts, FS_REG_FIRMID, &old_ver, 1);
	if(ret < 0){
		dev_err(ts->dev, "%s: i2c read version error\n", __func__);
		return ret;
	}

	if(fts_ctpm_get_upg_ver() != old_ver){
		msleep(200);
		ret =  fts_ctpm_fw_upgrade_with_i_file(ts);
		if(ret < 0){
			dev_err(ts->dev, "%s: failed to ugrade\n", __func__);
			return ret;
		}
		msleep(200);
		fts_i2c_read(ts, FS_REG_FIRMID, &new_ver, 1);
		dev_info(ts->dev, "Update from old version[0x%2x] to new version[0x%2x]\n", old_ver, new_ver);
		msleep(4000);
	}
	return 0;
}


static void fts_report_value(struct fts_data *ts)
{
	char buf[6*MAX_POINT+1], reg;
	struct fts_event events[MAX_POINT], ev;
	int ret = 0, i, off;

	reg = 0x00;
	ret = fts_i2c_read(ts, reg, buf, 6*MAX_POINT+1);

	if (ret < 0) {
		dev_err(ts->dev, "fts_i2c_write error:%d!\n",ret);
		return;
	}
	memset(events, 0, sizeof(struct fts_event) * MAX_POINT);
	ts->touch_point = buf[2]&0x07;

	if(ts->touch_point == 0){
		for(i = 0; i < MAX_POINT; i++){
			if(ts->flags[i] != 0){
				fts_dbg(ts->dev, "Point UP: id: %d, down_count: %d\n",i, ts->down_count[i]);
				ts->flags[i] = 0;
				ts->down_count[i] = 0;
				input_mt_slot(ts->input_dev, i);
				input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
			}
		}
		input_sync(ts->input_dev);
		return;
	}

	for(i = 0; i < ts->touch_point; i++){
		off = i*6+3;
		ev.id = (s16)(buf[off+2] & 0xF0)>>4;
		if(unlikely((ev.id >= MAX_POINT) || (ev.id < 0))){
			dev_err(ts->dev, "read touch_id[%d] error\n", ev.id);
			return;
		}
		ev.x = ((buf[off+0] & 0x0F)<<8) | buf[off+1];
		ev.y = ((buf[off+2] & 0x0F)<<8) | buf[off+3];
		if(unlikely(ev.x > ts->x_max || ev.y > (ts->y_max + 60))){
			dev_err(ts->dev, "read pos[x:%d, y:%d] error\n", ev.x, ev.y);
			continue;
		}
		ev.flag = (buf[off+0] & 0xc0) >> 6;
		if(ev.flag){
			events[ev.id].x = ev.x;
			events[ev.id].y = ev.y;
			events[ev.id].flag = ev.flag;
			events[ev.id].pressure = 200;
		}
		fts_dbg(ts->dev, "get event: id: %d, x: %d, y: %d, flag:%d\n",ev.id, ev.x, ev.y, ev.flag);
	}
	for(i = 0; i < MAX_POINT; i++){
		if((events[i].flag == 0) && (ts->flags[i] != 0)){
			fts_dbg(ts->dev, "Point UP: id: %d, x: %d, y: %d, down_count: %d\n",
				i, events[i].x, events[i].y, ts->down_count[i]);
			ts->down_count[i] = 0;
			input_mt_slot(ts->input_dev, i);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
		}else if(events[i].flag != 0){
			fts_dbg(ts->dev, "Point DOWN: id: %d, x: %d, y: %d\n",i, events[i].x, events[i].y);
			ts->down_count[i]++;
			input_mt_slot(ts->input_dev, i);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
			//input_report_abs(data->input_dev, ABS_MT_PRESSURE, event.pressure);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X,  events[i].x);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,  events[i].y);
		}
		ts->flags[i] = events[i].flag;
	}
	input_sync(ts->input_dev);
	return;
}

static irqreturn_t fts_interrupt(int irq, void *dev_id)
{
	struct fts_data *ts = dev_id;
	
	disable_irq_nosync(ts->irq);
	fts_report_value(ts);
	enable_irq(ts->irq);
	return IRQ_HANDLED;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
void fts_early_suspend(struct early_suspend *h)
{
	struct fts_data *ts = container_of(h, struct fts_data, suspend);

	char buf = FTS_PMODE_HIBERNATE;
	int ret = 0;
	
	disable_irq(ts->irq);
	ret = fts_i2c_write(ts, FTS_REG_PMODE, &buf, 1);

	if(ret < 0)
		dev_err(ts->dev, "%s: i2c_write error, reg = 0x%x\n", __func__, FTS_REG_PMODE);
	return;
}
void fts_early_resume(struct early_suspend *h)
{
	struct fts_data *ts = container_of(h, struct fts_data, suspend);
	
	enable_irq(ts->irq);
	return;
}
#endif

static int ft5x06_ts_suspend(void)
{
	struct fts_data *ts = fb_ts;
	char buf = FTS_PMODE_HIBERNATE;
	int i;

	disable_irq(ts->irq);
	if (gpio_is_valid(ts->reset_gpio)) {
		fts_i2c_write(ts, FTS_REG_PMODE, &buf, 1);
		gpio_set_value(ts->reset_gpio, 0);
		msleep(FT_RESET_DLY);
	}
	for(i = 0; i < MAX_POINT; i++){
		if(ts->flags[i] != 0){
			ts->down_count[i] = 0;
			input_mt_slot(ts->input_dev, i);
			input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
		}
		ts->flags[i] = 0;
	}
	input_sync(ts->input_dev);
	
	return 0;
}

static int ft5x06_ts_resume(void)
{
	struct fts_data *ts = fb_ts;

	if (gpio_is_valid(ts->reset_gpio)) {
		gpio_set_value(ts->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value(ts->reset_gpio, 1);
	}
	enable_irq(ts->irq);

	return 0;
}

static int early_resume = 0;
static int ft5x06_fb_event_notify(struct notifier_block *self, unsigned long action, void *data)
{
        struct fb_event *event = data;
        int blank_mode = *((int *)event->data);

        if (action == FB_EARLY_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
                                break;
                        default:
                                ft5x06_ts_suspend();
                                break;
                }
        }
        else if (action == FB_EVENT_BLANK) {
                switch (blank_mode) {
                        case FB_BLANK_UNBLANK:
								if(early_resume)
									ft5x06_ts_resume();
								else
									early_resume = 1;
                                break;
                        default:
                                break;
                }
        }
        return NOTIFY_OK;
}

static struct notifier_block ft5x06_fb_notifier = {
        .notifier_call = ft5x06_fb_event_notify,
};


static int  fts_probe(struct i2c_client *client ,const struct i2c_device_id *id)
{
	struct fts_data *ts;

	char buf[2];
	int retrys = 5, ret = 0;
	
	struct device_node *np = client->dev.of_node;
    enum of_gpio_flags rst_flags;
    unsigned long irq_flags, irq_gpio;
	int val;


	
	ts = devm_kzalloc(&client->dev, sizeof(struct fts_data), GFP_KERNEL);	
	if (!ts){
		dev_err(&client->dev, "No memory for fts\n");
		return -ENOMEM;
	}
	
	irq_gpio = of_get_named_gpio_flags(np, "touch-gpio", 0, (enum of_gpio_flags *)&irq_flags);
    ts->reset_gpio = of_get_named_gpio_flags(np, "reset-gpio", 0, &rst_flags);
	of_property_read_u32(np, "max-x", &val);
	ts->x_max = val;
    of_property_read_u32(np, "max-y", &val);
	ts->y_max = val;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
		dev_err(&client->dev, "Must have I2C_FUNC_I2C.\n");
		return -ENODEV;
	}
	
	if (gpio_is_valid(irq_gpio)) {
		ret = gpio_request(irq_gpio, "ft5x06_irq_gpio");
		if (ret < 0) {
			dev_err(&client->dev, "irq gpio request failed");
		}
		ret = gpio_direction_input(irq_gpio);
		if (ret < 0) {
			dev_err(&client->dev,
				"set_direction for irq gpio failed\n");
		}
	}
	
	ts->irq = gpio_to_irq(irq_gpio);

	if (gpio_is_valid(ts->reset_gpio)) {
		ret = gpio_request(ts->reset_gpio, "ft5x06_reset_gpio");
		if (ret < 0) {
			dev_err(&client->dev, "reset gpio request failed");
		}

		ret = gpio_direction_output(ts->reset_gpio, 0);
		if (ret < 0) {
			dev_err(&client->dev,
				"set_direction for reset gpio failed\n");
		}
		msleep(FT_RESET_DLY);
		gpio_set_value(ts->reset_gpio, 1);
		msleep(FT_RESET_DLY);
	}
	
	ts->client = client;
	ts->dev = &client->dev;
	
	i2c_set_clientdata(client, ts);

	buf[0] = FTS_PMODE_MONITOR;
	while(retrys--){
		ret = fts_i2c_write(ts, FTS_REG_PMODE, buf, 1);
		if(ret == 0)
			break;
	}
	
	if(ret < 0){
		dev_err(ts->dev, "fts_i2c_write error, reg: 0x%x\n", FTS_REG_PMODE);
		goto err_i2c_write;
	}
	
	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		ret = -ENOMEM;
		dev_err(ts->dev, "failed to allocate input device\n");
		goto err_input_allocate_device;
	}

	__set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);
	__set_bit(EV_ABS, ts->input_dev->evbit);	

	input_mt_init_slots(ts->input_dev, MAX_POINT,0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);	
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts->x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts->y_max, 0, 0);

	ts->input_dev->name = "ft5x0x_ts-touchscreen";		//dev_name(&client->dev)
	ret = input_register_device(ts->input_dev);
	if (ret) {
		dev_err(ts->dev,"failed to register input device: \n");
		goto err_input_register_device;
	}

#if UPDATE
	ret = fts_update_config(ts);
	if(ret < 0){
		dev_err(ts->dev, "failed to fts_update_config\n");
		goto err_fts_update_config;
	}
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->suspend.suspend =fts_early_suspend;
	ts->suspend.resume =fts_early_resume;
	ts->suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;;
	register_early_suspend(&ts->suspend);
#endif

	buf[0] = FTS_REPORT_RATE;
	ret = fts_i2c_write(ts, FTS_REG_REPORT_RATE, &buf[0], 1);
	if(ret == 0)
		ret = fts_i2c_read(ts, FTS_REG_REPORT_RATE, &buf[0], 1);
	if(ret < 0){
		dev_err(&client->dev, "fts_i2c_read error, reg: 0x%x\n", FTS_REG_REPORT_RATE);
		goto err_request_irq;
	}
	
	buf[1] = FTS_VALID_THRES;
	ret = fts_i2c_write(ts, FTS_REG_THRES, &buf[1], 1);
	if(ret == 0)
		ret = fts_i2c_read(ts, FTS_REG_THRES, &buf[1], 1);
	if(ret < 0){
		dev_err(&client->dev, "fts_i2c_read error, reg: 0x%x\n", FTS_REG_THRES);
		goto err_request_irq;
	}
	ret = devm_request_threaded_irq(&client->dev, ts->irq, NULL, fts_interrupt, irq_flags | IRQF_ONESHOT, client->dev.driver->name, ts);
	if (ret < 0) {
		dev_err(&client->dev, "irq %d busy?\n", ts->irq);
		goto err_request_irq;
	}
	
	dev_info(ts->dev, "%s ok, i2c addr: 0x%x, report rate: %dHz, active thres: %d\n", 
		__func__, ts->client->addr, buf[0]*10, buf[1]*4);
		
	fb_ts = ts;
	fb_register_client(&ft5x06_fb_notifier);
	
	return 0;
	
err_request_irq:
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->suspend);
#endif 	
err_fts_update_config:	
	input_unregister_device(ts->input_dev);
err_input_register_device:
	input_free_device(ts->input_dev);
err_input_allocate_device:
err_i2c_write:
	i2c_set_clientdata(client, NULL);
	kfree(ts);
	return 0;
	
}
static int fts_remove(struct i2c_client *client)
{
	struct fts_data *ts = i2c_get_clientdata(client);

	free_irq(ts->irq,ts);
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->suspend);
#endif 
	input_unregister_device(ts->input_dev);
	input_free_device(ts->input_dev);
	i2c_set_clientdata(client, NULL);
	kfree(ts);
	if(ts->platform_deinit_hw)
		ts->platform_deinit_hw();
	return 0;
}

static struct of_device_id ft5x06_dt_ids[] = {
        { .compatible = "ft5x06" },
        { }
};

static struct i2c_device_id fts_idtable[] = {
	{ FTS_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, fts_idtable);

static struct i2c_driver fts_driver  = {
	.probe		= fts_probe,
	.remove 	= fts_remove,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= FTS_NAME,
		.of_match_table = of_match_ptr(ft5x06_dt_ids),
	},
	.id_table	= fts_idtable,
};

static int __init fts_ts_init(void)
{
	return i2c_add_driver(&fts_driver);
}

static void __exit fts_ts_exit(void)
{
	i2c_del_driver(&fts_driver);
}

module_init(fts_ts_init);
module_exit(fts_ts_exit);

MODULE_DESCRIPTION("FocalTech ft5x06 TouchScreen driver");
MODULE_LICENSE("GPL v2");
