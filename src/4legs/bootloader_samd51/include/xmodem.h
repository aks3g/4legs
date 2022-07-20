/*
 * xmodem.h
 *
 * Created: 2020/04/06 22:15:11
 *  Author: kiyot
 */ 

typedef union XmodemBuf_t
{
	struct{
		uint8_t header;
		uint8_t index;
		uint8_t index_n;
		uint8_t data[128];
		uint8_t checksum;			
	} fd;
	uint8_t arr[128 + 4];
} XmodemBuf;

typedef struct {
	XmodemBuf buf;
	uint32_t  transfer_size;
	uint32_t  received_size;
	
	uint32_t  received_block_size;
} XmodemContext;

typedef uint8_t (*XmodemRxCallback)(uint8_t status, const XmodemContext *ctx);

uint8_t xmodem_receive(XmodemContext *buf, XmodemRxCallback cb);

