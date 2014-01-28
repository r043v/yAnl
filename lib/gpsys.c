#include "gpsys.h"

#define GPOS_RESID_MEM	0
#define GPOS_RESID_NET	1
#define GPOS_RESID_FIO	2
#define GPOS_RESID_GDI	3

#define FIO_ENTERCRITICAL()		_gp_os_res_lock(GPOS_RESID_FIO)
#define FIO_EXITCRITICAL()		_gp_os_res_unlock(GPOS_RESID_FIO)

extern void (*sm_write_gp32) (unsigned char data);
extern unsigned char (*sm_read_gp32) (void);
extern void (*sm_busy_gp32) (void);
extern void (*sm_read_en_gp32) (void);
extern void (*sm_read_dis_gp32) (void);
//extern int _gp_e2prom_read(ushort pos, ushort count, uchar*dst);
//extern int _gp_e2prom_write(short pos, short count, uchar*dst);
/*extern int (*_gp_e2prom_read)(int pos, int count, uchar*dst);
extern int (*_gp_e2prom_write)(int pos, int count, uchar*dst);
extern int (*_gp_os_res_lock)(int dtc) ;
extern int (*_gp_os_res_unlock)(int dtc) ;
extern int (*plop)(int dtc) ;
*/
/*
#define SM_ALE_EN(drv_no)				(rPDATE |= 0x10)    //PE4
#define SM_ALE_DIS(drv_no)				(rPDATE &= (~0x10))
#define SM_CLE_EN(drv_no)				(rPDATE |= 0x20)    //PE5
#define SM_CLE_DIS(drv_no)				(rPDATE &= (~0x20))
#define SM_CHIP_EN(drv_no)				(rPDATD &= (~0x80)) //PD7
#define SM_CHIP_DIS(drv_no)				(rPDATD |= 0x80)
#define SM_WP_DIS()						(rPDATD |= 0x40)    //PD6
*/
extern void (*SM_ALE_EN) (int drv_no);
extern void (*SM_ALE_DIS) (int drv_no);
extern void (*SM_CLE_EN) (int drv_no);
extern void (*SM_CLE_DIS) (int drv_no);
extern void (*SM_CHIP_EN) (int drv_no);
extern void (*SM_CHIP_DIS) (int drv_no);
extern void (*SM_WP_DIS) (void);
extern void (*SM_WP_EN) (void);

#define SM_WRITE_EN(drv_no)				sm_read_dis_gp32()
#define SM_WRITE_DIS(drv_no)			sm_read_en_gp32()	//no_oper
#define SM_READ_EN(drv_no)				sm_read_en_gp32()
#define SM_READ_DIS(drv_no)				sm_read_dis_gp32()

#define SM_WRITE_CMD(drv_no, cmd)		sm_write_gp32(cmd)
#define SM_WRITE_ADDR(drv_no, addr)		sm_write_gp32(addr)
#define SM_WRITE_DATA(drv_no, data)		sm_write_gp32(data)
#define SM_READ_DATA(drv_no)			sm_read_gp32()

#define SM_WAIT_TR(drv_no)				sm_busy_gp32();

static ERR_CODE _GpSmcIDGet(unsigned char *data)
{
	char drv_no;

	ubyte temp_buf[3];
	ubyte uniqe_id[128];
	ubyte compl_id[128];
	ubyte zcompl[128];
	ubyte spare[8];
	int i, j, k;
	int err;
	char have_id = 0;

	i = j = k = 0;
	err = 0;
	drv_no = 0;

	// SMC reset
	if (GpSMCDetected() != SM_OK)
		return ERR_CARD_NOT_DETECTED;

	SM_CHIP_EN(drv_no);
	SM_CLE_EN(drv_no);
	//SM_ALE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_CMD(drv_no, 0xff);
	SM_WRITE_DIS(drv_no);
	SM_CLE_DIS(drv_no);
	SM_WAIT_TR(drv_no);

	//ID read
	SM_CHIP_EN(drv_no);

	SM_CLE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_CMD(drv_no, 0x90);	//0x90
	SM_WRITE_DIS(drv_no);
	SM_CLE_DIS(drv_no);

	SM_ALE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_ADDR(drv_no, 0);	//0x00
	SM_WRITE_DIS(drv_no);
	SM_ALE_DIS(drv_no);

	SM_READ_EN(drv_no);
	temp_buf[0] = SM_READ_DATA(drv_no);
	temp_buf[1] = SM_READ_DATA(drv_no);
	temp_buf[2] = SM_READ_DATA(drv_no);
	SM_READ_DIS(drv_no);

	//if((temp_buf[0]!=0xec)||(temp_buf[2]!=0xa5))
	//  return 0;
	if (temp_buf[2] != 0xa5)	// NO unique id
	{
		return ERR_ID_NOT_FOUND;
	}

	if ((temp_buf[0] != SMC_FAC_SAMSUNG) && (temp_buf[0] != SMC_FAC_TOSHIBA))	//0xec : samsung, 0x98 : toshiba
	{
		return ERR_ID_INVALID;
	}

	SM_CHIP_DIS(drv_no);

	//Redundant Block Access Mode
	SM_CHIP_EN(drv_no);

	SM_CLE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	if (temp_buf[0] == SMC_FAC_SAMSUNG)	//if samsung
	{
		SM_WRITE_CMD(drv_no, 0x30);
		SM_WRITE_DIS(drv_no);
		SM_WRITE_EN(drv_no);
		SM_WRITE_CMD(drv_no, 0x65);
	}
	else						//if toshiba
	{
		SM_WRITE_CMD(drv_no, 0x5a);
		SM_WRITE_DIS(drv_no);
		SM_WRITE_EN(drv_no);
		SM_WRITE_CMD(drv_no, 0xb5);
	}
	SM_WRITE_DIS(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_CMD(drv_no, 0);
	SM_WRITE_DIS(drv_no);
	SM_CLE_DIS(drv_no);

	SM_ALE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_ADDR(drv_no, 0);	//0x00
	SM_WRITE_DIS(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_ADDR(drv_no, 0);	//0x00
	SM_WRITE_DIS(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_ADDR(drv_no, 0);	//0x00
	SM_WRITE_DIS(drv_no);
	if ((temp_buf[1] == SMC_DEV_64M) || (temp_buf[1] == SMC_DEV_128M))
	{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//by achi 2002.1.3
//add
		SM_WRITE_EN(drv_no);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		SM_WRITE_ADDR(drv_no, 0);	//0x00
		SM_WRITE_DIS(drv_no);
	}
	SM_ALE_DIS(drv_no);

	SM_WAIT_TR(drv_no);

	SM_READ_EN(drv_no);
	for (i = 0; i < 256; i++)
	{
		if (i % 32 < 16)
		{
			uniqe_id[j] = SM_READ_DATA(drv_no);
			j++;
		}
		else
		{
			compl_id[k] = SM_READ_DATA(drv_no);
			k++;
		}
	}
	SM_READ_DIS(drv_no);

	SM_CHIP_DIS(drv_no);

// spare area read
	SM_CHIP_EN(drv_no);

	SM_CLE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_CMD(drv_no, 0x50);
	SM_WRITE_DIS(drv_no);
	SM_CLE_DIS(drv_no);

	SM_ALE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_ADDR(drv_no, 0);
	SM_WRITE_ADDR(drv_no, 0);
	SM_WRITE_ADDR(drv_no, 0);
	if ((temp_buf[1] == SMC_DEV_64M) || (temp_buf[1] == SMC_DEV_128M))
	{
		SM_WRITE_ADDR(drv_no, 0);	//0x00
	}
	SM_WRITE_DIS(drv_no);
	SM_ALE_DIS(drv_no);

	SM_WAIT_TR(drv_no);

	SM_READ_EN(drv_no);

	for (i = 0; i < 8; ++i)
	{
		spare[i] = SM_READ_DATA(drv_no);
	}

	SM_READ_DIS(drv_no);

	SM_CHIP_DIS(drv_no);

	for (i = 0; i < 4; i++)
	{
		if ((spare[i] == 0x55) && (spare[i + 1] == 0xaa))
			have_id++;
	}
	if (!have_id)
	{
		return ERR_ID_INVALID;
	}

	for (i = 0; i < 8; i++)
	{
		err = 0;
		for (j = 0; j < 16; j++)
		{
			zcompl[j + i * 16] = ~compl_id[j + i * 16];
			if (uniqe_id[j + i * 16] != zcompl[j + i * 16])
				err++;
		}
		if (!err)
			break;
	}

	if (err)
	{
		return ERR_ID_INVALID;
	}
	else
	{
		for (j = 0; j < 16; j++)
			*data++ = uniqe_id[i * 16 + j];
	}

	SM_CHIP_EN(drv_no);
	SM_CLE_EN(drv_no);
	//SM_ALE_EN(drv_no);
	SM_WRITE_EN(drv_no);
	SM_WRITE_CMD(drv_no, 0xff);
	SM_WRITE_DIS(drv_no);
	SM_CLE_DIS(drv_no);
	SM_WAIT_TR(drv_no);
	//SM_CHIP_DIS(drv_no);

	return SM_OK;
}


/*
ERR_CODE GpE2PROMRead(int pos, int count, unsigned char *dest)
{
	int ret;

	//FIO_ENTERCRITICAL();
	ret = _gp_e2prom_read(pos, count, dest);
	//FIO_EXITCRITICAL();
	return (ERR_CODE) ret;
}

ERR_CODE GpE2PROMWrite(int pos, int count, unsigned char *src)
{
	int ret;

	FIO_ENTERCRITICAL();
	ret = _gp_e2prom_write(pos, count, src);
	FIO_EXITCRITICAL();
	return (ERR_CODE) ret;
}
*/
ERR_CODE GpSmcIDGet(unsigned char *data)
{
	int ret;

	//FIO_ENTERCRITICAL();
	ret = _GpSmcIDGet(data);
	//FIO_EXITCRITICAL();
	return (ERR_CODE) ret;
}

#define DEVICE_SN_POS	16
#define DEVICE_SN_LEN	16
#define DEVICE_UID_POS	36

/*ERR_CODE GpDeviceIDGet(unsigned char *data)
{
	char *sn_encode_stream = (char*)"SANGHYUK";
	int i, j;
	ERR_CODE err = GpE2PROMRead(DEVICE_SN_POS, DEVICE_SN_LEN, data);

	if (err == SM_OK)
	{
		j = 0;
		for (i = 0; i < DEVICE_SN_LEN; i++)
		{
			data[i] ^= sn_encode_stream[j++];
			j %= sizeof(sn_encode_stream);
		}
	}
	return err;
}

int GpDevUserInfoGet(int mode, unsigned char *data)
{
	char *sn_encode_stream = (char*)"SANGHYUK";
	int i, j;
	int err = 0;

	if (mode == 1)
		err = GpE2PROMRead(DEVICE_UID_POS, DEVICE_SN_LEN, data);
	else if (mode == 2)
		err = GpE2PROMRead(DEVICE_UID_POS + 16, DEVICE_SN_LEN, data);
	if (err == SM_OK)
	{
		j = 0;
		for (i = 0; i < DEVICE_SN_LEN; i++)
		{
			data[i] ^= sn_encode_stream[j++];
			j %= sizeof(sn_encode_stream);
		}
	}
	return err;
}

int GpDevUserInfoSet(int mode, unsigned char *data)
{
	char *sn_encode_stream = (char*)"SANGHYUK";
	int i, j;
	int err = 0;

	j = 0;
	for (i = 0; i < DEVICE_SN_LEN; i++)
	{
		data[i] ^= sn_encode_stream[j++];
		j %= sizeof(sn_encode_stream);
	}

	if (mode == 1)
		err = GpE2PROMWrite(DEVICE_UID_POS, DEVICE_SN_LEN, data);
	else if (mode == 2)
		err = GpE2PROMWrite(DEVICE_UID_POS + 16, DEVICE_SN_LEN, data);

	return err;

}
*/