/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */
#define LOG_TAG "LCM-icnl9911s-truly-601"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"
#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include "disp_dts_gpio.h"
#endif


#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif


static const unsigned int BL_MIN_LEVEL = 20;
static struct LCM_UTIL_FUNCS lcm_util;
#define MP_LCM_DA (unsigned int)(0x11)
#define MP_LCM_DB (unsigned int)(0x00)
#define MP_LCM_DC (unsigned int)(0x61)
#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define MDELAY(n)       (lcm_util.mdelay(n))
#define UDELAY(n)       (lcm_util.udelay(n))

#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
	lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#define set_gpio_lcd_enp(cmd) \
	lcm_util.set_gpio_lcd_enp_bias(cmd)

#define set_gpio_lcd_enn(cmd) \
	lcm_util.set_gpio_lcd_enn_bias(cmd)
#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
/* #include <linux/jiffies.h> */
/* #include <linux/delay.h> */
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

/* static unsigned char lcd_id_pins_value = 0xFF; */
static const unsigned char LCD_MODULE_ID = 0x01;
#define LCM_DSI_CMD_MODE        0
#define FRAME_WIDTH             720
#define FRAME_HEIGHT            1560

#define LCM_PHYSICAL_WIDTH      64800
#define LCM_PHYSICAL_HEIGHT     140400
#define REGFLAG_DELAY           0xFFFC
#define REGFLAG_UDELAY          0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW       0xFFFE
#define REGFLAG_RESET_HIGH      0xFFFF
#ifndef BUILD_LK
extern int NT50358A_write_byte(unsigned char addr, unsigned char value);
#endif
//static struct LCM_DSI_MODE_SWITCH_CMD lcm_switch_mode_cmd;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x26,0x01,{0x08}},
	{0x28,0x00, {0x00} },
	{REGFLAG_DELAY, 50, {} },
	{0x10, 0x00, {0x00} },
	{REGFLAG_DELAY, 120, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static struct LCM_setting_table lcm_suspend_setting_gesture[] = {
	{0x26,0x01,{0x08}},
	{0x28,0x00, {0x00} },
	{REGFLAG_DELAY, 50, {} },
	{0x10, 0x00, {0x00} },
	{REGFLAG_DELAY, 120, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static struct LCM_setting_table init_setting[] = {
};

static struct LCM_setting_table bl_level[] = {
	{0x51, 0x02,{0x02,0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void push_table(void *cmdq, struct LCM_setting_table *table,
		unsigned int count, unsigned char force_update)
{
	unsigned int i;
	unsigned int cmd;

	for (i = 0; i < count; i++) {
		cmd = table[i].cmd;
		switch (cmd) {
			case REGFLAG_DELAY:
				if (table[i].count <= 10)
					MDELAY(table[i].count);
				else
					MDELAY(table[i].count);
				break;

			case REGFLAG_UDELAY:
				UDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE:
				break;

			default:
				dsi_set_cmdq_V22(cmdq, cmd, table[i].count,
						table[i].para_list, force_update);
		}
	}
}


static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	LCM_LOGI("%s:icnl9911s\n",__func__);
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}


static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
#endif
	LCM_LOGI("lcm_get_params lcm_dsi_mode %d\n", lcm_dsi_mode);
	params->dsi.switch_mode_enable = 0;
	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count = FRAME_WIDTH*3;
	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 12;
	params->dsi.vertical_frontporch = 124;
	params->dsi.vertical_active_line = FRAME_HEIGHT;
	params->dsi.horizontal_sync_active = 52;
	params->dsi.horizontal_backporch = 140;
	params->dsi.horizontal_frontporch = 120;//old is 16,now is 60
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.PLL_CLOCK = 448;    /* FrameRate = 60Hz */ /* this value must be in MTK suggested table */
	params->dsi.ssc_disable = 1;
	//params->dsi.noncont_clock = TRUE; /* Add noncont_clock setting for ESD */
	//params->dsi.noncont_clock_period = 1; /* Add noncont_clock setting for ESD */

	params->dsi.cont_clock = 0;
	params->dsi.clk_lp_per_line_enable = 1;
}



static void lcm_reset(void)
{
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);

	MDELAY(50);


	LCM_LOGI("%s:icnl9911s lcm reset done\n",__func__);
}

extern volatile int gesture_dubbleclick_en;
static void lcm_init_power(void)
{
}

static void lcm_suspend_power(void)
{
}

static void lcm_resume_power(void)
{
}

static void lcm_init(void)
{
	unsigned char cmd = 0x0;
	unsigned char data = 0x12;  //up to +/-5.8V
	int ret = 0;
	LCM_LOGI("%s: icnl9911s start\n",__func__);
	LCM_LOGI("%s: gesture_dubbleclick_en=%d \n",__func__,gesture_dubbleclick_en);
	if (!gesture_dubbleclick_en) {
		MDELAY(15);
		ret = NT50358A_write_byte(cmd, data);
		if (ret < 0)
			LCM_LOGI("%s:----cmd=%0x--i2c write error----\n",__func__, cmd);
		else
			LCM_LOGI("%s:---cmd=%0x--i2c write success----\n",__func__, cmd);
		cmd = 0x01;
		data = 0x12;
		ret = NT50358A_write_byte(cmd, data);
		if (ret < 0)
			LCM_LOGI("%s:--cmd=%0x--i2c write error----\n",__func__, cmd);
		else
			LCM_LOGI("%s:----cmd=%0x--i2c write success----\n",__func__, cmd);
	}

	push_table(NULL, init_setting,
			sizeof(init_setting) / sizeof(struct LCM_setting_table), 1);

	LCM_LOGI("%s: icnl9911s done\n",__func__);
}

static void lcm_suspend(void)
{
	struct LCM_setting_table *setting_table = NULL;
	size_t setting_table_size = 0;
	LCM_LOGI("%s,icnl9911s start\n", __func__);

    if (gesture_dubbleclick_en) {
		setting_table = lcm_suspend_setting_gesture;
		setting_table_size = ARRAY_SIZE(lcm_suspend_setting_gesture);
	} else {
		setting_table = lcm_suspend_setting;
		setting_table_size = ARRAY_SIZE(lcm_suspend_setting);
	}
#ifndef MACH_FPGA
	push_table(NULL, setting_table, setting_table_size, 1);
	MDELAY(10);
#endif
	LCM_LOGI("%s,icnl9911s done\n", __func__);
}

static void lcm_resume(void)
{
	LCM_LOGI("%s,icnl9911s  start\n",__func__);
	lcm_init();
	LCM_LOGI("%s,icnl9911s  done\n",__func__);
}

static unsigned int lcm_ata_check(unsigned char *buffer)
{
	return 0;
}

static void lcm_setbacklight(void *handle, unsigned int level)
{
	if (level > 255)
		level = 255;

	bl_level[0].para_list[0] = (level & 0xF0) >> 4;
	bl_level[0].para_list[1] = (level & 0x0F) << 4;
	LCM_LOGI("%s,backlight set level = %d \n", __func__, level);
	push_table(handle, bl_level, sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);

}

static unsigned int lcm_compare_id(void)
{
#if 0
	return 1;
#else
	unsigned int id_da = 0,id_db = 0,id_dc = 0;
	unsigned char buffer[2];
	unsigned int array[16];

	SET_RESET_PIN(0);
	MDELAY(5);

	SET_RESET_PIN(1);
	MDELAY(20);

	array[0] = 0x00023700;  /* read id return two byte,version and id */
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xDC, buffer, 1);
	id_dc = buffer[0];         /* we only need ID */
	read_reg_v2(0xDB, buffer, 1);
	id_db = buffer[0];         /* we only need ID */
	read_reg_v2(0xDA, buffer, 1);
	id_da = buffer[0];

	LCM_LOGI("%s,icnl9911s vendor DA = 0x%08x,DB = 0x%08x, DC = 0x%08x\n", 
						__func__, id_da, id_db, id_dc);

	if (MP_LCM_DC == id_dc && MP_LCM_DB == id_db && MP_LCM_DA == id_da)
		return 1;
	else
		return 0;
#endif
}



struct LCM_DRIVER icnl9911s_hd_dsi_vdo_hjc_lcm_drv = {
	.name = "icnl9911s_hd_dsi_vdo_hjc_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.set_backlight_cmdq = lcm_setbacklight,
	.ata_check = lcm_ata_check,

};
