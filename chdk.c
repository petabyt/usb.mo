#include <stdio.h>
#include <dryos.h>
#include <module.h>
#include <menu.h>
#include <config.h>
#include <bmp.h>
#include <console.h>
#include <math.h>
#include <ptp.h>
#include <imgconv.h>
#include <mem.h>
#include <exmem.h>
#include "main.h"

int recv_ptp_data(struct ptp_context *data, char *buf, int size)
	// repeated calls per transaction are ok
{
	while ( size >= BUF_SIZE )
	{
		data->recv_data(data->handle,buf,BUF_SIZE,0,0);
		// XXX check for success??

		size -= BUF_SIZE;
		buf += BUF_SIZE;
	}
	if ( size != 0 )
	{
		data->recv_data(data->handle,buf,size,0,0);
		// XXX check for success??
	}

	return 1;
}

int send_ptp_data(struct ptp_context *data, const char *buf, int size)
	// repeated calls per transaction are *not* ok
{
	int tmpsize;

	tmpsize = size;
	while ( size >= BUF_SIZE )
	{
		if ( data->send_data(data->handle,(void *)buf,BUF_SIZE,tmpsize,0,0,0) )
		{
			return 0;
		}

		tmpsize = 0;
		size -= BUF_SIZE;
		buf += BUF_SIZE;
	}
	if ( size != 0 )
	{
		if ( data->send_data(data->handle,(void *)buf,size,tmpsize,0,0,0) )
		{
			return 0;
		}
	}

	return 1;
}

int ptp_chdk_upload_file(struct ptp_context *context) {
    FILE *f;
    int s,fn_len;
    char *buf, *fn;

    s = context->get_data_size(context->handle);

    recv_ptp_data(context,(char *) &fn_len,4);
    s -= 4;

    fn = (char *) fio_malloc(fn_len+1);
    if ( fn == NULL )
    {
        return PTP_RC_GeneralError;
    }
    fn[fn_len] = '\0';

    recv_ptp_data(context,fn,fn_len);
    s -= fn_len;

    bmp_printf(FONT_LARGE, 0, 0, "UL '%s' %db", fn, s);

    FIO_RemoveFile(fn);
    f = FIO_CreateFile(fn);
    if (!f)
    {
        fio_free(fn);
        return PTP_RC_GeneralError;
    }
    fio_free(fn);

    buf = (char *) fio_malloc(BUF_SIZE);
    if ( buf == NULL )
    {
        return PTP_RC_GeneralError;
    }
    while ( s > 0 )
    {
        if ( s >= BUF_SIZE )
        {
            recv_ptp_data(context,buf,BUF_SIZE);
            FIO_WriteFile(f, buf, BUF_SIZE);
            s -= BUF_SIZE;
        } else {
            recv_ptp_data(context,buf,s);
            FIO_WriteFile(f, buf, s);
            s = 0;
        }
    }

    FIO_CloseFile(f);

    fio_free(buf);

    return 0;
}
