/*
 * xmodem.c
 *
 * Created: 2020/04/06 22:14:54
 *  Author: kiyot
 */ 

#include <stdint.h>
#include <stddef.h>
#include <lib4legs.h>

#include "xmodem.h"

uint8_t _receive_data_block(XmodemBuf *buf, uint32_t size);
uint8_t _check_data_block(XmodemBuf *buf);

uint8_t xmodem_receive(XmodemContext *ctx, XmodemRxCallback cb)
{
	uint8_t ret = 0;
	uint8_t nak = (uint8_t)0x15;
	uint8_t ack = (uint8_t)0x06;
	uint8_t soh = (uint8_t)0x01;
	uint8_t eot = (uint8_t)0x04;
	
	ctx->received_size = 0;	
	ret = lib4legs_putc((char)nak);
	if (ret != 0) {
		return ret;
	}

	while (1) {
		uint32_t block_size = 128;
		
		ret = _receive_data_block(&ctx->buf, block_size);
		if (ret != 0) {
			return ret;
		}
		else if (soh == ctx->buf.fd.header) {
			uint8_t status = _check_data_block(&ctx->buf);
			uint8_t res = 0;
			
			ctx->received_block_size = block_size;

			ret = cb(status, ctx);		
			res = (ret == 0) ? ack : nak;
			
			ctx->received_size += block_size;
			
			ret = lib4legs_putc((char)res);
			if (ret != 0) {
				return ret;
			}
		}
		else if (eot == ctx->buf.fd.header){
			ret = lib4legs_putc((char)ack);
			break;
		}
		ctx->transfer_size++;
	}

	return 0;
}


uint8_t _receive_data_block(XmodemBuf *buf, uint32_t size)
{
	uint8_t soh = (uint8_t)0x01;
	uint8_t eot = (uint8_t)0x04;
	do {
		lib4legs_rx(&(buf->fd.header), 1, 1);
	} while (buf->fd.header != soh && buf->fd.header != eot);

	if (buf->fd.header == eot) return 0;

	lib4legs_rx(&(buf->fd.index), 1, 1);
	lib4legs_rx(&(buf->fd.index_n), 1, 1);
	
	if ((uint8_t)(buf->fd.index + buf->fd.index_n) != 0xff) {
		return -1; // Index not match.
	}

	for (int i=0 ; i<size ; ++i) {
		lib4legs_rx(&(buf->fd.data[i]), 1, 1);
	}
	lib4legs_rx(&(buf->fd.checksum), 1, 1);

	return 0;	
}

uint8_t _check_data_block(XmodemBuf *buf)
{
	return 0;
}
