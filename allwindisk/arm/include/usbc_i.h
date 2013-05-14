/*
********************************************************************************************************************
*                                              usb controller
*
*                              (c) Copyright 2007-2009, daniel.China
*										All	Rights Reserved
*
* File Name 	: usbc_i.h
*
* Author 		: daniel
*
* Version 		: 1.0
*
* Date 			: 2009.09.15
*
* Description 	: ÊÊÓÃÓÚsuniiÆ½Ì¨£¬USB¹«¹²²Ù×÷²¿·Ö
*
* History 		:
*
********************************************************************************************************************
*/
#ifndef  __USBC_I_H__
#define  __USBC_I_H__

#include "usb_bsp.h"

#define  USBC_MAX_OPEN_NUM    8

/* ¼ÇÂ¼USBµÄ¹«¹²ÐÅÏ¢ */
typedef struct __fifo_info{
    __u32 port0_fifo_addr;
	__u32 port0_fifo_size;

    __u32 port1_fifo_addr;
	__u32 port1_fifo_size;

	__u32 port2_fifo_addr;
	__u32 port2_fifo_size;
}__fifo_info_t;

/* ¼ÇÂ¼µ±Ç°USB portËùÓÐµÄÓ²¼þÐÅÏ¢ */
typedef struct __usbc_otg{
    __u32 port_num;
	__u32 base_addr;        /* usb base address 		*/

	__u32 used;             /* ÊÇ·ñÕýÔÚ±»Ê¹ÓÃ   		*/
    __u32 no;               /* ÔÚ¹ÜÀíÊý×éÖÐµÄÎ»ÖÃ 		*/
}__usbc_otg_t;

#endif   //__USBC_I_H__

