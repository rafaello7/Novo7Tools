/*
********************************************************************************************************************
*                                              usb controller
*
*                              (c) Copyright 2007-2009,
*										All	Rights Reserved
*
* File Name 	: usbc.c
*
* Author 		: daniel
*
* Version 		: 1.0
*
* Date 			: 2009.09.01
*
* Description 	: ÊÊÓÃÓÚsuniiÆ½Ì¨£¬USB¹«¹²²Ù×÷²¿·Ö
*
* History 		:
*
********************************************************************************************************************
*/

#include  "usbc_i.h"
#include <string.h>


static __u32 usbc_base_address[USBC_MAX_CTL_NUM];       /* usb base address */
static __usbc_otg_t usbc_otg_array[USBC_MAX_OPEN_NUM];  /* usbc ÄÚ²¿Ê¹ÓÃ, ÓÃÀ´¹ÜÀíUSB¶Ë¿Ú */
static __fifo_info_t usbc_info_g;

/*
***********************************************************************************
*                     USBC_GetVbusStatus
*
* Description:
*    »ñµÃµ±Ç°vbusµÄ×´Ì¬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*    ·µ»Øµ±Ç°vbusµÄ×´Ì¬
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_GetVbusStatus(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u8 reg_val = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	reg_val = USBC_Readb(USBC_REG_DEVCTL(usbc_otg->base_addr));
	reg_val = reg_val >> USBC_BP_DEVCTL_VBUS;
    switch(reg_val & 0x03){
		case 0x00:
			return USBC_VBUS_STATUS_BELOW_SESSIONEND;
		//break;

		case 0x01:
			return USBC_VBUS_STATUS_ABOVE_SESSIONEND_BELOW_AVALID;
		//break;

		case 0x02:
			return USBC_VBUS_STATUS_ABOVE_AVALID_BELOW_VBUSVALID;
		//break;

		case 0x03:
			return USBC_VBUS_STATUS_ABOVE_VBUSVALID;
		//break;

		default:
			return USBC_VBUS_STATUS_BELOW_SESSIONEND;
	}
}

/*
***********************************************************************************
*                     USBC_OTG_SelectMode
*
* Description:
*    Ñ¡ÔñÉè±¸µÄÀàÐÍ¡£µ±Ç°Éè±¸ÊÇ×÷device, »¹ÊÇ×÷host
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void USBC_OTG_SelectMode(__hdle hUSB, __u32 mode)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	if(mode == USBC_OTG_HOST){

	}else{

	}
}

/*
***********************************************************************************
*                     USBC_ReadLenFromFifo
*
* Description:
*    ±¾´Îfifo¿ÉÒÔ¶Áµ½µÄÊý¾Ý³¤¶È
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    ep_type  :  input.  epµÄÀàÐÍ, rx »ò tx¡£
* Returns:
*    ·µ»Ø±¾´Îfifo¿ÉÒÔ¶Áµ½µÄÊý¾Ý³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_ReadLenFromFifo(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_type){
		case USBC_EP_TYPE_EP0:
			return USBC_Readw(USBC_REG_COUNT0(usbc_otg->base_addr));
		//break;

		case USBC_EP_TYPE_TX:
			return 0;
		//break;

		case USBC_EP_TYPE_RX:
			return USBC_Readw(USBC_REG_RXCOUNT(usbc_otg->base_addr));
		//break;

		default:
			return 0;
	}
}

/*
***********************************************************************************
*                     USBC_WritePacket
*
* Description:
*    ÍùfifoÀïÃæÐ´Êý¾Ý°ü
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    fifo    :  input.  fifoµØÖ·.
*    cnt     :  input.  Ð´Êý¾Ý³¤¶È
*    buff    :  input.  ´æ·ÅÒªÐ´µÄÊý¾Ý
*
* Returns:
*    ·µ»Ø³É¹¦Ð´ÈëµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_WritePacket(__hdle hUSB, __u32 fifo, __u32 cnt, void *buff)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

	if(usbc_otg == NULL || buff == NULL){
		return 0;
	}

    //--<1>--µ÷ÕûÊý¾Ý
	buf32 = buff;
	len   = cnt;

	i32 = len >> 2;
	i8  = len & 0x03;

    //--<2>--´¦Àí4×Ö½ÚµÄ²¿·Ö
	while (i32--){
		USBC_Writel(*buf32++, fifo);
	}

    //--<3>--´¦Àí·Ç4×Ö½ÚµÄ²¿·Ö
	buf8 = (__u8 *)buf32;
	while (i8--){
		USBC_Writeb(*buf8++, fifo);
	}

	return len;
}

/*
***********************************************************************************
*                     USBC_ReadPacket
*
* Description:
*    ´ÓfifoÀïÃæ¶ÁÊý¾Ý
*
* Arguments:
*    hUSB    :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    fifo    :  input.  fifoµØÖ·.
*    cnt     :  input.  Ð´Êý¾Ý³¤¶È
*    buff    :  input.  ´æ·ÅÒª¶ÁµÄÊý¾Ý
*
* Returns:
*    ·µ»Ø³É¹¦¶ÁµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_ReadPacket(__hdle hUSB, __u32 fifo, __u32 cnt, void *buff)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 len = 0;
	__u32 i32 = 0;
	__u32 i8  = 0;
	__u8  *buf8  = NULL;
	__u32 *buf32 = NULL;

    if(usbc_otg == NULL || buff == NULL){
		return 0;
	}

	//--<1>--µ÷ÕûÊý¾Ý
	buf32 = (__u32 *)buff;
	len   = cnt;

    i32 = len >> 2;
	i8  = len & 0x03;

	//--<2>--´¦Àí4×Ö½ÚµÄ²¿·Ö
	while (i32--){
        *buf32++ = USBC_Readl(fifo);
    }

	//--<3>--´¦Àí·Ç4×Ö½ÚµÄ²¿·Ö
	buf8 = (__u8 *)buf32;
	while (i8--){
        *buf8++ = USBC_Readb(fifo);
    }

	return len;
}

/* Ó³ÉäSRAM D¸øusb fifoÊ¹ÓÃ */
void USBC_ConfigFIFO_Base(__hdle hUSB, __u32 sram_base, __u32 fifo_mode)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __fifo_info_t *usbc_info = &usbc_info_g;
	__u32 reg_value = 0;

	if(usbc_otg == NULL){
		return ;
	}

	if(usbc_otg->port_num == 0){
		reg_value = USBC_Readl(sram_base + 0x04);
		reg_value |= (1 << 0);
		USBC_Writel(reg_value, (sram_base + 0x04));

		usbc_info->port0_fifo_addr = 0x00;
		usbc_info->port0_fifo_size = (8 * 1024);	//8k
    }

	return ;
}

/* »ñµÃport fifoµÄÆðÊ¼µØÖ· */
__u32 USBC_GetPortFifoStartAddr(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	if(usbc_otg->port_num == 0){
		return usbc_info_g.port0_fifo_addr;
	}else if(usbc_otg->port_num == 1){
	    return usbc_info_g.port1_fifo_addr;
	}else {
	    return usbc_info_g.port2_fifo_addr;
	}
}

/* »ñµÃport fifoµÄ´óÐ¡ */
__u32 USBC_GetPortFifoSize(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	if(usbc_otg->port_num == 0){
		return usbc_info_g.port0_fifo_size;
	}else{
	    return usbc_info_g.port1_fifo_size;
	}
}


/*
***********************************************************************************
*                     USBC_SelectFIFO
*
* Description:
*    Ñ¡ÔñÉè±¸µÄÀàÐÍ¡£µ±Ç°Éè±¸ÊÇ×÷device, »¹ÊÇ×÷host
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    ep_index :  input.  epºÅ¡£ÓÃÀ´Ñ¡ÔñÏàÓ¦µÄfifo
*
* Returns:
*    ·µ»ØÑ¡ÖÐµÄfifo
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
/*
__u32 USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 fifo = 0;

	if(usbc_otg == NULL){
		return 0;
	}

    switch(ep_index){
		case 0:
			fifo = USBC_REG_EPFIFO0(usbc_otg->base_addr);
		break;

		case 1:
			fifo = USBC_REG_EPFIFO1(usbc_otg->base_addr);
		break;

		case 2:
			fifo = USBC_REG_EPFIFO2(usbc_otg->base_addr);
		break;

		case 3:
			fifo = USBC_REG_EPFIFO3(usbc_otg->base_addr);
		break;

		case 4:
			fifo = USBC_REG_EPFIFO4(usbc_otg->base_addr);
		break;

		case 5:
			fifo = USBC_REG_EPFIFO5(usbc_otg->base_addr);
		break;

		default:
			fifo = 0;
	}

	return fifo;
}
*/

__u32 USBC_SelectFIFO(__hdle hUSB, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	return USBC_REG_EPFIFOx(usbc_otg->base_addr, ep_index);
}

static void __USBC_ConfigFifo_TxEp_Default(__u32 usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_TXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_TXFIFOSZ(usbc_base_addr));
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_TxEp
*
* Description:
*    ÅäÖÃtx ep µÄfifoµØÖ·ºÍ´óÐ¡¡£
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    is_double_fifo :  input.  ÊÇ·ñÊ¹ÓÃÓ²¼þË«fifo
*    fifo_size      :  input.  fifo´óÐ¡ = 2µÄfifo_size´Î·½
*    fifo_addr      :  input.  fifoµÄÆðÊ¼µØÖ· = fifo_addr * 8
*
* Returns:
*    ·µ»Ø³É¹¦¶ÁµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void __USBC_ConfigFifo_TxEp(__u32 usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __u32 temp = 0;
    __u32 size = 0;   //fifo_size = (size + 3)µÄ2´Î·½
    __u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>--»»Ëãsz, ²»Âú512£¬ÒÔ512¶ÔÆë
	temp = fifo_size + 511;
	temp &= ~511;  //°Ñ511ºóÃæµÄÇåÁã
	temp >>= 3;
	temp >>= 1;
	while(temp){
		size++;
		temp >>= 1;
	}

	//--<2>--»»Ëãaddr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_TXFIFOAD(usbc_base_addr));

	//--<4>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_TXFIFOSZ(usbc_base_addr));
	if(is_double_fifo){
		USBC_REG_set_bit_b(USBC_BP_TXFIFOSZ_DPB, USBC_REG_TXFIFOSZ(usbc_base_addr));
	}
}

void __USBC_ConfigFifo_RxEp_Default(__u32 usbc_base_addr)
{
	USBC_Writew(0x00, USBC_REG_RXFIFOAD(usbc_base_addr));
	USBC_Writeb(0x00, USBC_REG_RXFIFOSZ(usbc_base_addr));
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_RxEp
*
* Description:
*    ÅäÖÃtx ep µÄfifoµØÖ·ºÍ´óÐ¡¡£
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    is_double_fifo :  input.  ÊÇ·ñÊ¹ÓÃÓ²¼þË«fifo
*    fifo_size      :  input.  fifo´óÐ¡ = 2µÄfifo_size´Î·½
*    fifo_addr      :  input.  fifoµÄÆðÊ¼µØÖ· = fifo_addr * 8
*
* Returns:
*    ·µ»Ø³É¹¦¶ÁµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void __USBC_ConfigFifo_RxEp(__u32 usbc_base_addr, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __u32 temp = 0;
    __u32 size = 0;   //fifo_size = (size + 3)µÄ2´Î·½
    __u32 addr = 0;   //fifo_addr = addr * 8

	//--<1>--¼ÆËãsz, ²»Âú512£¬ÒÔ512¶ÔÆë
	temp = fifo_size + 511;
	temp &= ~511;  //°Ñ511ºóÃæµÄÇåÁã
	temp >>= 3;
	temp >>= 1;
	while(temp){
		size++;
		temp >>= 1;
	}

	//--<2>--»»Ëãaddr
	addr = fifo_addr >> 3;

	//--<3>--config fifo addr
	USBC_Writew(addr, USBC_REG_RXFIFOAD(usbc_base_addr));

	//--<2>--config fifo size
	USBC_Writeb((size & 0x0f), USBC_REG_RXFIFOSZ(usbc_base_addr));
	if(is_double_fifo){
		USBC_REG_set_bit_b(USBC_BP_RXFIFOSZ_DPB, USBC_REG_RXFIFOSZ(usbc_base_addr));
	}
}

/*
***********************************************************************************
*                     USBC_ConfigFifo_Default
*
* Description:
*    ÅäÖÃep µÄfifoµØÖ·ºÍ´óÐ¡¡£
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*	 ep_type		:  input.  epµÄÀàÐÍ
*
* Returns:
*    ·µ»Ø³É¹¦¶ÁµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void USBC_ConfigFifo_Default(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support
		break;

		case USBC_EP_TYPE_TX:
			__USBC_ConfigFifo_TxEp_Default(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_ConfigFifo_RxEp_Default(usbc_otg->base_addr);
		break;

		default:
		break;
	}
}

/*
***********************************************************************************
*                     USBC_ConfigFifo
*
* Description:
*    ÅäÖÃep µÄfifoµØÖ·ºÍ´óÐ¡¡£
*
* Arguments:
*    hUSB           :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*	 ep_type		:  input.  epµÄÀàÐÍ
*    is_double_fifo :  input.  ÊÇ·ñÊ¹ÓÃÓ²¼þË«fifo
*    fifo_size      :  input.  fifo´óÐ¡ = 2µÄfifo_size´Î·½
*    fifo_addr      :  input.  fifoµÄÆðÊ¼µØÖ· = fifo_addr * 8
*
* Returns:
*    ·µ»Ø³É¹¦¶ÁµÄ³¤¶È
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void USBC_ConfigFifo(__hdle hUSB, __u32 ep_type, __u32 is_double_fifo, __u32 fifo_size, __u32 fifo_addr)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
			//not support
		break;

		case USBC_EP_TYPE_TX:
			__USBC_ConfigFifo_TxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;

		case USBC_EP_TYPE_RX:
			__USBC_ConfigFifo_RxEp(usbc_otg->base_addr, is_double_fifo, fifo_size, fifo_addr);
		break;

		default:
		break;
	}
}

/*
***********************************************************************************
*                     USBC_GetLastFrameNumber
*
* Description:
*    »ñµÃ×îºóÒ»Ö¡µÄÖ¡ºÅ
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_GetLastFrameNumber(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return USBC_Readl(USBC_REG_FRNUM(usbc_otg->base_addr));
}

/*
***********************************************************************************
*                     USBC_GetStatus_Dp
*
* Description:
*    »ñµÃdpµÄ×´Ì¬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/

__u32 USBC_GetStatus_Dp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;

    return temp;
}



/*
***********************************************************************************
*                     USBC_GetStatus_Dm
*
* Description:
*    »ñµÃdmµÄ×´Ì¬
*
* Arguments:
*    hUSB :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_GetStatus_Dm(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;

	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	temp = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;

    return temp;
}


/*
***********************************************************************************
*                     USBC_GetStatus_Dp
*
* Description:
*    »ñµÃdpµÄ×´Ì¬
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/

__u32 USBC_GetStatus_DpDm(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 temp = 0;
	__u32 dp = 0;
	__u32 dm = 0;


	if(usbc_otg == NULL){
		return 0;
	}

	temp = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	dp = (temp >> USBC_BP_ISCR_EXT_DP_STATUS) & 0x01;
	dm = (temp >> USBC_BP_ISCR_EXT_DM_STATUS) & 0x01;
	return ((dp << 1) | dm);

}

/*
***********************************************************************************
*                     USBC_GetOtgMode_Form_ID
*
* Description:
*    ´Óvendor0 µÄ id »ñµÃµ±Ç°OTGµÄÄ£Ê½
*
* Arguments:
*    hUSB :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*    USBC_OTG_DEVICE / USBC_OTG_HOST
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_GetOtgMode_Form_ID(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if(usbc_otg == NULL){
		return USBC_OTG_DEVICE;
	}

    mode = USBC_REG_test_bit_l(USBC_BP_ISCR_MERGED_ID_STATUS, USBC_REG_ISCR(usbc_otg->base_addr));
    if(mode){
		return USBC_OTG_DEVICE;
	}else{
	    return USBC_OTG_HOST;
	}
}

/*
***********************************************************************************
*                     USBC_GetOtgMode_Form_BDevice
*
* Description:
*    ´Ó OTG Device µÄ B-Device »ñµÃµ±Ç°OTGµÄÄ£Ê½
*
* Arguments:
*    hUSB :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*    USBC_OTG_DEVICE / USBC_OTG_HOST
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__u32 USBC_GetOtgMode_Form_BDevice(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 mode = 0;

	if(usbc_otg == NULL){
		return USBC_OTG_DEVICE;
	}

    mode = USBC_REG_test_bit_b(USBC_BP_DEVCTL_B_DEVICE, USBC_REG_DEVCTL(usbc_otg->base_addr));
    if(mode){
		return USBC_OTG_DEVICE;
	}else{
	    return USBC_OTG_HOST;
	}
}

/*
***********************************************************************************
*                     USBC_SelectBus
*
* Description:
*    Ñ¡ÔñÊý¾Ý´«ÊäµÄ×ÜÏß·½Ê½
*
* Arguments:
*    hUSB     :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*    io_type  :  input.  ×ÜÏß·½Ê½, pio»¹ÊÇdma.
*    ep_type  :  input.  epµÄÀàÐÍ, rx »ò tx¡£
*    ep_index :  input.  epºÅ
*
* Returns:
*
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
void USBC_SelectBus(__hdle hUSB, __u32 io_type, __u32 ep_type, __u32 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

    reg_val = USBC_Readb(USBC_REG_VEND0(usbc_otg->base_addr));
    if(io_type == USBC_IO_TYPE_DMA){
		if(ep_type == USBC_EP_TYPE_TX){
			reg_val |= ((ep_index - 0x01) << 1) << USBC_BP_VEND0_DRQ_SEL;  //drq_sel
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;   //io_dma
		}else{
		    reg_val |= ((ep_index << 1) - 0x01) << USBC_BP_VEND0_DRQ_SEL;
			reg_val |= 0x1<<USBC_BP_VEND0_BUS_SEL;
		}
	}else{
	    //reg_val &= ~(0x1 << USBC_BP_VEND0_DRQ_SEL);  //Çå³ýdrq_sel, Ñ¡Ôñpio
	    reg_val &= 0x00;  //Çå³ýdrq_sel, Ñ¡Ôñpio
	}

	USBC_Writeb(reg_val, USBC_REG_VEND0(usbc_otg->base_addr));
}

/* »ñµÃtx epÖÐ¶Ï±êÖ¾Î» */
static __u32 __USBC_INT_TxPending(__u32 usbc_base_addr)
{
    return (USBC_Readw(USBC_REG_INTTx(usbc_base_addr)));
}

/* Çå³ýtx epÖÐ¶Ï±êÖ¾Î» */
static void __USBC_INT_ClearTxPending(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_Writew((1 << ep_index), USBC_REG_INTTx(usbc_base_addr));
}

/* Çå³ýËùÓÐtx epÖÐ¶Ï±êÖ¾Î» */
static void __USBC_INT_ClearTxPendingAll(__u32 usbc_base_addr)
{
    USBC_Writew(0xffff, USBC_REG_INTTx(usbc_base_addr));
}

/* »ñµÃrx epÖÐ¶Ï±êÖ¾Î» */
static __u32 __USBC_INT_RxPending(__u32 usbc_base_addr)
{
    return (USBC_Readw(USBC_REG_INTRx(usbc_base_addr)));
}

/* Çå³ýrx epÖÐ¶Ï±êÖ¾Î» */
static void __USBC_INT_ClearRxPending(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_Writew((1 << ep_index), USBC_REG_INTRx(usbc_base_addr));
}

/* Çå³ýrx epÖÐ¶Ï±êÖ¾Î» */
static void __USBC_INT_ClearRxPendingAll(__u32 usbc_base_addr)
{
    USBC_Writew(0xffff, USBC_REG_INTRx(usbc_base_addr));
}

/* ¿ªÄ³Ò»¸ötx epµÄÖÐ¶Ï */
static void __USBC_INT_EnableTxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_set_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* ¿ªÄ³Ò»¸örx epµÄÖÐ¶Ï */
static void __USBC_INT_EnableRxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_set_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* ¹ØÄ³Ò»¸ötx epµÄÖÐ¶Ï */
static void __USBC_INT_DisableTxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_clear_bit_w(ep_index, USBC_REG_INTTxE(usbc_base_addr));
}

/* ¹ØÄ³Ò»¸örx epµÄÖÐ¶Ï */
static void __USBC_INT_DisableRxEp(__u32 usbc_base_addr, __u8 ep_index)
{
    USBC_REG_clear_bit_w(ep_index, USBC_REG_INTRxE(usbc_base_addr));
}

/* ¹ØËùÓÐµÄtx epÖÐ¶Ï */
static void __USBC_INT_DisableTxAll(__u32 usbc_base_addr)
{
    USBC_Writew(0, USBC_REG_INTTxE(usbc_base_addr));
}

/* ¹ØËùÓÐµÄrx epÖÐ¶Ï */
static void __USBC_INT_DisableRxAll(__u32 usbc_base_addr)
{
    USBC_Writew(0, USBC_REG_INTRxE(usbc_base_addr));
}

/* »ñµÃepÖÐ¶Ï±êÖ¾Î» */
__u32 USBC_INT_EpPending(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    return __USBC_INT_TxPending(usbc_otg->base_addr);

		case USBC_EP_TYPE_RX:
		    return __USBC_INT_RxPending(usbc_otg->base_addr);

		default:
			return 0;
	}
}

/* Çå³ýepÖÐ¶Ï±êÖ¾Î» */
void USBC_INT_ClearEpPending(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    __USBC_INT_ClearTxPending(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_ClearRxPending(usbc_otg->base_addr, ep_index);
		break;

		default:
			break;
	}

	return ;
}

/* Çå³ýepÖÐ¶Ï±êÖ¾Î» */
void USBC_INT_ClearEpPendingAll(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_EP0:
		case USBC_EP_TYPE_TX:
		    __USBC_INT_ClearTxPendingAll(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_ClearRxPendingAll(usbc_otg->base_addr);
		break;

		default:
			break;
	}

	return ;
}

/* »ñµÃusb miscÖÐ¶Ï±êÖ¾Î» */
__u32 USBC_INT_MiscPending(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return (USBC_Readb(USBC_REG_INTUSB(usbc_otg->base_addr)));
}

/* Çå³ýusb miscÖÐ¶Ï±êÖ¾Î» */
void USBC_INT_ClearMiscPending(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(mask, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* Çå³ýËùÓÐusb miscÖÐ¶Ï±êÖ¾Î» */
void USBC_INT_ClearMiscPendingAll(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(0xff, USBC_REG_INTUSB(usbc_otg->base_addr));
}

/* ¿ªÄ³Ò»¸öepÖÐ¶Ï */
void USBC_INT_EnableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_EnableTxEp(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_EnableRxEp(usbc_otg->base_addr, ep_index);
		break;

		default:
        break;
	}

	return ;
}

/* ¿ªÄ³Ò»¸öusb miscÖÐ¶Ï */
void USBC_INT_EnableUsbMiscUint(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val |= mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* ¹ØÄ³tx epµÄÖÐ¶Ï */
void USBC_INT_DisableEp(__hdle hUSB, __u32 ep_type, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_DisableTxEp(usbc_otg->base_addr, ep_index);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_DisableRxEp(usbc_otg->base_addr, ep_index);
		break;

		default:
        break;
	}

	return;
}

/* ¹ØÄ³Ò»¸öusb miscÖÐ¶Ï */
void USBC_INT_DisableUsbMiscUint(__hdle hUSB, __u32 mask)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return ;
	}

	reg_val = USBC_Readb(USBC_REG_INTUSBE(usbc_otg->base_addr));
	reg_val &= ~mask;
	USBC_Writeb(reg_val, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* ¹ØËùÓÐµÄepÖÐ¶Ï */
void USBC_INT_DisableEpAll(__hdle hUSB, __u32 ep_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	switch(ep_type){
		case USBC_EP_TYPE_TX:
		    __USBC_INT_DisableTxAll(usbc_otg->base_addr);
		break;

		case USBC_EP_TYPE_RX:
		    __USBC_INT_DisableRxAll(usbc_otg->base_addr);
		break;

		default:
        break;
	}

	return;
}

/* ¹ØËùÓÐµÄusb miscÖÐ¶Ï */
void USBC_INT_DisableUsbMiscAll(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_Writeb(0, USBC_REG_INTUSBE(usbc_otg->base_addr));
}

/* »ñµÃµ±Ç°»î¶¯µÄep */
__u32 USBC_GetActiveEp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return 0;
	}

    return USBC_Readb(USBC_REG_EPIND(usbc_otg->base_addr));
}

/* ÅäÖÃµ±Ç°»î¶¯ep */
void USBC_SelectActiveEp(__hdle hUSB, __u8 ep_index)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	USBC_Writeb(ep_index, USBC_REG_EPIND(usbc_otg->base_addr));
}

/* ¼ÓÇ¿usb´«ÊäÐÅºÅ */
void USBC_EnhanceSignal(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

	return;
}

/* ½øÈë TestPacket Ä£Ê½ */
void USBC_EnterMode_TestPacket(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ½øÈë Test_K Ä£Ê½ */
void USBC_EnterMode_Test_K(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ½øÈë Test_J Ä£Ê½ */
void USBC_EnterMode_Test_J(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* ½øÈë Test_SE0_NAK Ä£Ê½ */
void USBC_EnterMode_Test_SE0_NAK(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_set_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* Çå³ýËùÓÐ²âÊÔÄ£Ê½ */
void USBC_EnterMode_Idle(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return ;
	}

    USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_PACKET, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_K, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_J, USBC_REG_TMCTL(usbc_otg->base_addr));
	USBC_REG_clear_bit_b(USBC_BP_TMCTL_TEST_SE0_NAK, USBC_REG_TMCTL(usbc_otg->base_addr));
}

/* vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î» */
static __u32 __USBC_WakeUp_ClearChangeDetect(__u32 reg_val)
{
    __u32 temp = reg_val;

	temp &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	return temp;
}

void USBC_SetWakeUp_Default(__hdle hUSB)
{

	return;
}

void USBC_EnableIdPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

    //vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î»
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableIdPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

	//vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î»
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_EnableDpDmPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

    //vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î»
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val |= (1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

void USBC_DisableDpDmPullUp(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
    __u32 reg_val = 0;

	//vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î»
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
	reg_val &= ~(1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

static void __USBC_ForceIdDisable(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//vbus, id, dpdm±ä»¯Î»ÊÇÐ´1ÇåÁã, Òò´ËÎÒÃÇÔÚ²Ù×÷ÆäËûbitµÄÊ±ºòÇå³ýÕâÐ©Î»
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToLow(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//ÏÈÐ´00£¬ºóÐ´10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceIdToHigh(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//ÏÈÐ´00£¬ºóÐ´10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force id to (id_type) */
void USBC_ForceId(__hdle hUSB, __u32 id_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

    switch(id_type){
		case USBC_ID_TYPE_HOST:
			__USBC_ForceIdToLow(usbc_otg->base_addr);
		break;

		case USBC_ID_TYPE_DEVICE:
			__USBC_ForceIdToHigh(usbc_otg->base_addr);
		break;

		default:
			__USBC_ForceIdDisable(usbc_otg->base_addr);
	}
}

static void __USBC_ForceVbusValidDisable(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//ÏÈÐ´00£¬ºóÐ´10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToLow(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//ÏÈÐ´00£¬ºóÐ´10
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

static void __USBC_ForceVbusValidToHigh(__u32 usbc_base_addr)
{
	__u32 reg_val = 0;

	//ÏÈÐ´00£¬ºóÐ´11
	reg_val = USBC_Readl(USBC_REG_ISCR(usbc_base_addr));
	//reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_base_addr));
}

/* force vbus valid to (id_type) */
void USBC_ForceVbusValid(__hdle hUSB, __u32 vbus_type)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

    switch(vbus_type){
		case USBC_VBUS_TYPE_LOW:
			__USBC_ForceVbusValidToLow(usbc_otg->base_addr);
		break;

		case USBC_VBUS_TYPE_HIGH:
			__USBC_ForceVbusValidToHigh(usbc_otg->base_addr);
		break;

		default:
			__USBC_ForceVbusValidDisable(usbc_otg->base_addr);
	}
}

void USBC_A_valid_InputSelect(__hdle hUSB, __u32 source)
{

    return;
}

void USBC_EnableUsbLineStateBypass(__hdle hUSB)
{

    return;
}

void USBC_DisableUsbLineStateBypass(__hdle hUSB)
{

    return;
}

void USBC_EnableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_HOSC_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ½ûÓÃHosc */
void USBC_DisableHosc(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_HOSC_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ²éÑ¯ÊÇ·ñ²úÉú vbus ÖÐ¶Ï */
__u32 USBC_IsVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //¶ÁÈ¡±ä»¯Î»µÄÍ¬Ê±, Ð´1Çå³ý¸ÃÎ»
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ²éÑ¯ÊÇ·ñ²úÉú id ÖÐ¶Ï */
__u32 USBC_IsIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //¶ÁÈ¡±ä»¯Î»µÄÍ¬Ê±, Ð´1Çå³ý¸ÃÎ»
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_ID_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ²éÑ¯ÊÇ·ñ²úÉú dpdm ÖÐ¶Ï */
__u32 USBC_IsDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;
	__u32 temp = 0;

    //¶ÁÈ¡±ä»¯Î»µÄÍ¬Ê±, Ð´1Çå³ý¸ÃÎ»
    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));

	temp = reg_val & (1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
    reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT;
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));

	return temp;
}

/* ½ûÓÃ wake ÖÐ¶Ï */
void USBC_DisableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_IRQ_ENABLE);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ½ûÓÃ vbus ÖÐ¶Ï */
void USBC_DisableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ½ûÓÃ id ÖÐ¶Ï */
void USBC_DisableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ½ûÓÃ dpdm ÖÐ¶Ï */
void USBC_DisableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN);
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* Ê¹ÄÜ wake ÖÐ¶Ï */
void USBC_EnableWakeIrq(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_IRQ_ENABLE;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* Ê¹ÄÜ vbus ±ä»¯ÖÐ¶Ï */
void USBC_EnableVbusChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* Ê¹ÄÜid±ä»¯ÖÐ¶Ï */
void USBC_EnableIdChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_ID_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* Ê¹ÄÜdmdp±ä»¯ÖÐ¶Ï */
void USBC_EnableDpDmChange(__hdle hUSB)
{
	__usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

    reg_val = USBC_Readl(USBC_REG_ISCR(usbc_otg->base_addr));
    reg_val |= 1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN;
	reg_val = __USBC_WakeUp_ClearChangeDetect(reg_val);
	USBC_Writel(reg_val, USBC_REG_ISCR(usbc_otg->base_addr));
}

/* ²âÊÔÄ£Ê½, »ñµÃ¼Ä´æÆ÷µÄÖµ */
__u32 USBC_TestMode_ReadReg(__hdle hUSB, __u32 offset, __u32 reg_width)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;
	__u32 reg_val = 0;

	if(usbc_otg == NULL){
		return reg_val;
	}

    if(reg_width == 8){
		reg_val = USBC_Readb(usbc_otg->base_addr + offset);
	}else if(reg_width == 16){
	    reg_val = USBC_Readw(usbc_otg->base_addr + offset);
	}else if(reg_width == 32){
	    reg_val = USBC_Readl(usbc_otg->base_addr + offset);
	}else{
	    reg_val = 0;
	}

	return reg_val;
}

/*
*******************************************************************************
*                     print_usb_reg_by_ep
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void USBC_PrintEPReg(__u32 usbc_base, __s32 ep_index, char *str, USBC_PRINT print)
{
	__u32 old_ep_index = 0;

    print("\n");
    print("--------------------------ep%d: %s--------------------------\n", ep_index, str);

	if(ep_index >= 0){
		old_ep_index = USBC_Readw(usbc_base + USBC_REG_o_EPIND);
		USBC_Writew(ep_index, (usbc_base + USBC_REG_o_EPIND));
		print("old_ep_index = %d, ep_index = %d\n", old_ep_index, ep_index);
	}

    print("USBC_REG_o_FADDR         = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_FADDR));
    print("USBC_REG_o_PCTL          = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_PCTL));
    print("USBC_REG_o_INTTx         = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_INTTx));
    print("USBC_REG_o_INTRx         = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_INTRx));
    print("USBC_REG_o_INTTxE        = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_INTTxE));
    print("USBC_REG_o_INTRxE        = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_INTRxE));
    print("USBC_REG_o_INTUSB        = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_INTUSB));
    print("USBC_REG_o_INTUSBE       = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_INTUSBE));
    print("USBC_REG_o_EPIND         = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_EPIND));
    print("USBC_REG_o_TXMAXP        = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_TXMAXP));
    print("USBC_REG_o_CSR0          = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_CSR0));
    print("USBC_REG_o_TXCSR         = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_TXCSR));
    print("USBC_REG_o_RXMAXP        = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_RXMAXP));
    print("USBC_REG_o_RXCSR         = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_RXCSR));

    print("USBC_REG_o_COUNT0        = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_COUNT0));
    print("USBC_REG_o_RXCOUNT       = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_RXCOUNT));
    print("USBC_REG_o_TXTYPE        = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_TXTYPE));
    print("USBC_REG_o_NAKLIMIT0     = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_NAKLIMIT0));
    print("USBC_REG_o_TXINTERVAL    = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_TXINTERVAL));
    print("USBC_REG_o_RXTYPE        = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_RXTYPE));
    print("USBC_REG_o_RXINTERVAL    = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_RXINTERVAL));
    print("USBC_REG_o_CONFIGDATA    = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_CONFIGDATA));

    print("USBC_REG_o_DEVCTL        = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_DEVCTL));
    print("USBC_REG_o_TXFIFOSZ      = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_TXFIFOSZ));
    print("USBC_REG_o_RXFIFOSZ      = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_RXFIFOSZ));
    print("USBC_REG_o_TXFIFOAD      = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_TXFIFOAD));
    print("USBC_REG_o_RXFIFOAD      = 0x%x\n", USBC_Readw(usbc_base + USBC_REG_o_RXFIFOAD));
    print("USBC_REG_o_VEND0         = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_VEND0));
    print("USBC_REG_o_VEND1         = 0x%x\n", USBC_Readb(usbc_base + USBC_REG_o_VEND1));

	print("TXFADDRx(%d)             = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_TXFADDRx));
	print("TXHADDRx(%d              = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_TXHADDRx));
	print("TXHPORTx(%d)             = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_TXHPORTx));
	print("RXFADDRx(%d)             = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_RXFADDRx));
	print("RXHADDRx(%d)             = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_RXHADDRx));
	print("RXHPORTx(%d)             = 0x%x\n", ep_index, USBC_Readb(usbc_base + USBC_REG_o_RXHPORTx));
	print("RPCOUNTx(%d)             = 0x%x\n", ep_index, (u32)USBC_Readw(usbc_base + USBC_REG_o_RPCOUNT));

    print("USBC_REG_o_ISCR          = 0x%x\n", (u32)USBC_Readl(usbc_base + USBC_REG_o_ISCR));
    print("USBC_REG_o_PHYCTL        = 0x%x\n", (u32)USBC_Readl(usbc_base + USBC_REG_o_PHYCTL));
    print("USBC_REG_o_PHYBIST       = 0x%x\n", (u32)USBC_Readl(usbc_base + USBC_REG_o_PHYBIST));

	if(ep_index >= 0){
		USBC_Writew(old_ep_index, (usbc_base + USBC_REG_o_EPIND));
	}

	print("---------------------------------------------------------------------------\n");
	print("\n");

    return;
}

/*
*******************************************************************************
*                     print_all_usb_reg
*
* Description:
*    void
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void USBC_PrintAllReg(__u32 usbc_base, __s32 ep_start, __u32 ep_end, char *str, USBC_PRINT print)
{
	__u32 i = 0;
	__u32 old_ep_index = 0;

    print("\n");
    print("-------------------print_all_usb_reg: %s----------------\n", str);

	old_ep_index = USBC_Readw(usbc_base + USBC_REG_o_EPIND);

	for(i = ep_start; i <= ep_end; i++){
		USBC_PrintEPReg(usbc_base, i, str, print);
	}

	USBC_Writew(old_ep_index, (usbc_base + USBC_REG_o_EPIND));

	print("---------------------------------------------------------------------------\n");
	print("\n");

    return;
}


/*
***********************************************************************************
*                     USBC_open_otg
*
* Description:
*    ÏòbspÉêÇë»ñµÃ¶Ë¿ÚºÅÎªotg_noµÄtogÊ¹ÓÃÈ¨
*
* Arguments:
*    otg_no  :  input.  ÐèÒªÊ¹ÓÃµÄTOG¶Ë¿ÚºÅ, ·¶Î§Îª: 0 ~ USBC_MAX_CTL_NUM
*
* Returns:
*    ³É¹¦, ·µ»Øusbc_otg¾ä±ú¡£Ê§°Ü, ·µ»ØNULL
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
#if 0
__hdle USBC_open_otg(__u32 otg_no)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;
	__u32 i = 0;

    //--<1>--otg_no²»ÄÜ³¬¹ýËùÄÜÖ§³ÖµÄ·¶Î§
    if(otg_no >= USBC_MAX_CTL_NUM){
		return 0;
	}

    //--<2>--ÔÚ¹ÜÀíÊý×éÀïÕÒÒ»¸ö¿ÕÎ», ×î´óÖ§³ÖÍ¬Ê±´ò¿ª8´Î
    for(i = 0; i < USBC_MAX_OPEN_NUM; i++){
		if(usbc_otg[i].used == 0){
			usbc_otg[i].used      = 1;
			usbc_otg[i].no        = i;
			usbc_otg[i].port_num  = otg_no;
			usbc_otg[i].base_addr = usbc_base_address[otg_no];

			return (__hdle)(&(usbc_otg[i]));
		}
	}

    return 0;
}
#else
__hdle USBC_open_otg(__u32 otg_no)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;

    //--<1>--otg_no²»ÄÜ³¬¹ýËùÄÜÖ§³ÖµÄ·¶Î§
    if(otg_no >= USBC_MAX_CTL_NUM){
		return 0;
	}

    //--<2>--ÔÚ¹ÜÀíÊý×éÀïÕÒÒ»¸ö¿ÕÎ», ×î´óÖ§³ÖÍ¬Ê±´ò¿ª8´Î
	usbc_otg[otg_no].used      = 1;
	usbc_otg[otg_no].no        = otg_no;
	usbc_otg[otg_no].port_num  = otg_no;
	usbc_otg[otg_no].base_addr = usbc_base_address[otg_no];

	return (__hdle)(&(usbc_otg[otg_no]));
}

#endif

/*
***********************************************************************************
*                     USBC_close_otg
*
* Description:
*    ÊÍ·ÅtogµÄÊ¹ÓÃÈ¨
*
* Arguments:
*    hUSB  :  input.  USBC_open_otg»ñµÃµÄ¾ä±ú, ¼ÇÂ¼ÁËUSBCËùÐèÒªµÄÒ»Ð©¹Ø¼üÊý¾Ý
*
* Returns:
*    0  :  ³É¹¦
*   !0  :  Ê§°Ü
*
* note:
*    ÎÞ
*
***********************************************************************************
*/
__s32  USBC_close_otg(__hdle hUSB)
{
    __usbc_otg_t *usbc_otg = (__usbc_otg_t *)hUSB;

	if(usbc_otg == NULL){
		return -1;
	}

	memset(usbc_otg, 0, sizeof(__usbc_otg_t));

	return 0;
}


/*
***********************************************************************************
*                           USBC_init
*
* Description:
*
*
* Arguments:
*
*
* Returns:
*    0  :  ³É¹¦
*   !0  :  Ê§°Ü
*
* note:
*
*
***********************************************************************************
*/
__s32 USBC_init(bsp_usbc_t *usbc)
{
 //   __usbc_otg_t *usbc_otg = usbc_otg_array;
    __u32 i = 0;

//    memset(usbc_base_address, 0, sizeof(usbc_base_address));
//    memset(&usbc_info_g, 0, sizeof(__fifo_info_t));
//    memset(usbc_otg, 0, (USBC_MAX_OPEN_NUM * sizeof(__usbc_otg_t)));

    /* ±£´æ driver ´«½øÀ´µÄ usb ¿ØÖÆÆ÷µÄ»ùÖ· */
/*
    for(i = 0; i < USBC_MAX_CTL_NUM; i++){
        __u32 port_num = 0;

        port_num = usbc->usbc_info[i].num;
        usbc_base_address[i] = usbc->usbc_info[i].base;
    }
*/
    for(i = 0; i < USBC_MAX_CTL_NUM; i++){
        __u32 port_num = 0;

		if(usbc->usbc_info[i].base){
	        port_num = usbc->usbc_info[i].num;
	        usbc_base_address[i] = usbc->usbc_info[i].base;
		}
    }

    return 0;
}

/*
***********************************************************************************
*                            USBC_exit
*
* Description:
*
*
* Arguments:
*
*
* Returns:
*    0  :  ³É¹¦
*   !0  :  Ê§°Ü
*
* note:
*
*
***********************************************************************************
*/
__s32 USBC_exit(bsp_usbc_t *usbc)
{
    __usbc_otg_t *usbc_otg = usbc_otg_array;

    memset(&usbc_info_g, 0, sizeof(__fifo_info_t));
    memset(usbc_otg, 0, (USBC_MAX_OPEN_NUM * sizeof(__usbc_otg_t)));
    memset(usbc_base_address, 0, sizeof(usbc_base_address));

    return 0;
}

///* USB´«ÊäÀàÐÍÑ¡Ôñ, ¶ÁÐ´Êý¾ÝµÈ */
//EXPORT_SYMBOL(USBC_OTG_SelectMode);
//
//EXPORT_SYMBOL(USBC_ReadLenFromFifo);
//EXPORT_SYMBOL(USBC_WritePacket);
//EXPORT_SYMBOL(USBC_ReadPacket);
//
//EXPORT_SYMBOL(USBC_ConfigFIFO_Base);
//EXPORT_SYMBOL(USBC_GetPortFifoStartAddr);
//EXPORT_SYMBOL(USBC_GetPortFifoSize);
//EXPORT_SYMBOL(USBC_SelectFIFO);
//EXPORT_SYMBOL(USBC_ConfigFifo_Default);
//EXPORT_SYMBOL(USBC_ConfigFifo);
//
//EXPORT_SYMBOL(USBC_SelectBus);
//
//EXPORT_SYMBOL(USBC_GetActiveEp);
//EXPORT_SYMBOL(USBC_SelectActiveEp);
//
//EXPORT_SYMBOL(USBC_EnhanceSignal);
//
//EXPORT_SYMBOL(USBC_GetLastFrameNumber);
//
//
///* usb ÖÐ¶Ï²Ù×÷²¿·Ö */
//EXPORT_SYMBOL(USBC_INT_EpPending);
//EXPORT_SYMBOL(USBC_INT_MiscPending);
//EXPORT_SYMBOL(USBC_INT_ClearEpPending);
//EXPORT_SYMBOL(USBC_INT_ClearMiscPending);
//EXPORT_SYMBOL(USBC_INT_ClearEpPendingAll);
//EXPORT_SYMBOL(USBC_INT_ClearMiscPendingAll);
//
//EXPORT_SYMBOL(USBC_INT_EnableEp);
//EXPORT_SYMBOL(USBC_INT_EnableUsbMiscUint);
//
//EXPORT_SYMBOL(USBC_INT_DisableEp);
//EXPORT_SYMBOL(USBC_INT_DisableUsbMiscUint);
//
//EXPORT_SYMBOL(USBC_INT_DisableEpAll);
//EXPORT_SYMBOL(USBC_INT_DisableUsbMiscAll);
//
//
///* usb ¿ØÖÆ²Ù×÷²¿·Ö */
//EXPORT_SYMBOL(USBC_GetVbusStatus);
//EXPORT_SYMBOL(USBC_GetStatus_Dp);
//EXPORT_SYMBOL(USBC_GetStatus_Dm);
//EXPORT_SYMBOL(USBC_GetStatus_DpDm);
//
//EXPORT_SYMBOL(USBC_GetOtgMode_Form_ID);
//EXPORT_SYMBOL(USBC_GetOtgMode_Form_BDevice);
//
//EXPORT_SYMBOL(USBC_SetWakeUp_Default);
//
//EXPORT_SYMBOL(USBC_EnableIdPullUp);
//EXPORT_SYMBOL(USBC_DisableIdPullUp);
//EXPORT_SYMBOL(USBC_EnableDpDmPullUp);
//EXPORT_SYMBOL(USBC_DisableDpDmPullUp);
//
//EXPORT_SYMBOL(USBC_ForceId);
//EXPORT_SYMBOL(USBC_ForceVbusValid);
//
//EXPORT_SYMBOL(USBC_A_valid_InputSelect);
//
//EXPORT_SYMBOL(USBC_EnableUsbLineStateBypass);
//EXPORT_SYMBOL(USBC_DisableUsbLineStateBypass);
//EXPORT_SYMBOL(USBC_EnableHosc);
//EXPORT_SYMBOL(USBC_DisableHosc);
//
//EXPORT_SYMBOL(USBC_IsVbusChange);
//EXPORT_SYMBOL(USBC_IsIdChange);
//EXPORT_SYMBOL(USBC_IsDpDmChange);
//
//EXPORT_SYMBOL(USBC_DisableWakeIrq);
//EXPORT_SYMBOL(USBC_DisableVbusChange);
//EXPORT_SYMBOL(USBC_DisableIdChange);
//EXPORT_SYMBOL(USBC_DisableDpDmChange);
//
//EXPORT_SYMBOL(USBC_EnableWakeIrq);
//EXPORT_SYMBOL(USBC_EnableVbusChange);
//EXPORT_SYMBOL(USBC_EnableIdChange);
//EXPORT_SYMBOL(USBC_EnableDpDmChange);
//
///* usb ²âÊÔÄ£Ê½ */
//EXPORT_SYMBOL(USBC_EnterMode_TestPacket);
//EXPORT_SYMBOL(USBC_EnterMode_Test_K);
//EXPORT_SYMBOL(USBC_EnterMode_Test_J);
//EXPORT_SYMBOL(USBC_EnterMode_Test_SE0_NAK);
//EXPORT_SYMBOL(USBC_EnterMode_Idle);
//
//EXPORT_SYMBOL(USBC_TestMode_ReadReg);
//
//EXPORT_SYMBOL(USBC_open_otg);
//EXPORT_SYMBOL(USBC_close_otg);
//EXPORT_SYMBOL(USBC_init);
//EXPORT_SYMBOL(USBC_exit);
//


