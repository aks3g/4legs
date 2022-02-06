/*
 * bootloader_xmegaE5.cpp
 *
 * Created: 2014/11/15 13:23:14
 *  Author: sazae7
 */
#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <libspine.h>

#include "avr_bootloader.h"
#include "flash_operation.h"

/*---------------------------------------------------------------------------*/
// Restart 
typedef void (*application_func_t)(void);
application_func_t application_start = (application_func_t)(0x0000);

// ---------------------------------------------------------------------------
// Work Buffer
static unsigned char buffer[SPM_PAGESIZE];

/*---------------------------------------------------------------------------*/
// I2C Interface
typedef struct {
	uint8_t addr;
	union {
		uint8_t arr[256];
		struct {
			volatile uint8_t  type;
			volatile uint8_t  status;
			volatile uint8_t  restart;
			volatile uint8_t  program;
			volatile uint8_t  debug;
			uint8_t  rsvd[119];
			uint32_t flash_addr;
			uint8_t  flash_block[128];
		} map;
	} buf;
	uint8_t busy;
	uint16_t rx_len;
} I2C_BUF;

static I2C_BUF si2c_buf;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define I2C_IDLE						(0)
#define I2C_MASTER_READ					(1)
#define I2C_MASTER_WRITE				(2)

#define I2C_SLAVE_RESPONCE_NACK			(1)
#define I2C_SLAVE_RESPONCE_ACK			(0)

/*---------------------------------------------------------------------------*/
typedef uint8_t (*i2c_StartCallback)(uint8_t mode);
typedef uint8_t (*i2c_StopCallback)(void);
typedef uint8_t (*i2c_MasterRxCallback)(uint8_t ack);
typedef uint8_t (*i2c_MasterTxCallback)(uint8_t data);

typedef struct
{
	uint8_t ownAddress;
	i2c_StartCallback startCB;
	i2c_StopCallback stopCB;
	i2c_MasterRxCallback masterRxCB;
	i2c_MasterTxCallback masterTxCB;
} I2C_INIT_OPT;
static I2C_INIT_OPT sSlaveContext;

uint8_t _startCb(uint8_t mode)
{
	if (mode == I2C_MASTER_READ) {
		;
	}
	else if (mode == I2C_MASTER_WRITE) {
		si2c_buf.rx_len = 0;
	}
	
	return 0;
}

uint8_t _stopCb(void)
{
	return 0;	
}

uint8_t _tx(uint8_t ack)
{
	return si2c_buf.buf.arr[si2c_buf.addr++];
}

volatile uint8_t val = 0;

uint8_t _rx(uint8_t data)
{
	if (si2c_buf.rx_len == 0) {
		si2c_buf.addr = data;
	}
	else {
		si2c_buf.buf.arr[si2c_buf.addr] = data;
		
		switch (si2c_buf.addr) 
		{
		case SPINE_MMAP_ADDR_TYPE:
		case SPINE_MMAP_ADDR_STATUS:
		case SPINE_MMAP_ADDR_RESTART:
		case SPINE_MMAP_ADDR_PROGRAM:
		case SPINE_MMAP_ADDR_DEBUG:
			si2c_buf.buf.map.status |= SPINE_MMAP_BUSY;
			break;
		case SPINE_MMAP_ADDR_FLASH_ADDR3:
			si2c_buf.addr++;
			val = data;
			break;
		default:
			si2c_buf.addr++;
			break;
		}
	}
	si2c_buf.rx_len++;

	return 0;
}

/*---------------------------------------------------------------------------*/
static uint8_t _sInitialize_i2cSlave(uint8_t saddr)
{
	/*J 18.10.1 CTRL ? Control register */
	TWIC.CTRL = 0x00;
	
	/* 18.12.1 CTRLA ? Control register A */
	TWIC.SLAVE.CTRLA = TWI_SLAVE_INTLVL_MED_gc | TWI_SLAVE_DIEN_bm | TWI_SLAVE_APIEN_bm | TWI_SLAVE_ENABLE_bm | TWI_SLAVE_PIEN_bm;

	/* 18.12.2 CTRLB ? Control register B */
	//J タイムアウトは使わない
	TWIC.SLAVE.CTRLB = 0x00;
	
	/* 18.12.4 ADDR ? Address register */
	TWIC.SLAVE.ADDR = (saddr << 1) & 0xFE;

	/* 18.12.3 STATUS ? Status register */
	TWIC.SLAVE.STATUS;

	return 0;
}

/*---------------------------------------------------------------------------*/
static void _initialize_gpio(void){
	PORTC.DIRSET = (1 << 7) | (1 << 6);
	PORTC.DIRCLR = (1 << 5) | (1 << 4);

	PORTC.PIN4CTRL = (2 << PORT_OPC_gp); // Pull down
	PORTC.PIN5CTRL = (2 << PORT_OPC_gp); // Pull down

	PORTC.OUTCLR = (1 << 7);
	PORTC.OUTSET = (1 << 6);

	return;
} 

/*---------------------------------------------------------------------------*/
static uint8_t _check_id(void) {
	return (PORTC.IN & 0x30) >> 4;
}

/*---------------------------------------------------------------------------*/
static void _swap_vector(int boot_sect)
{
	cli();
	if (boot_sect) {
		CPU_CCP    = CCP_IOREG_gc;  // Unlock reg.
		PMIC_CTRL  = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm | PMIC_IVSEL_bm; // Bootloader vector
	}	
	else {
		CPU_CCP    = CCP_IOREG_gc;  // Unlock reg.
		PMIC_CTRL  = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; // Application vector
	}	
	sei();
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int main(void)
{

	//J クロック設定
	OSC.CTRL |= OSC_RC32MEN_bm; // turn on 32 MHz oscillator
	while (!(OSC.STATUS & OSC_RC32MRDY_bm)) { }; // wait for it to start
	CCP = CCP_IOREG_gc;
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc;

//		CPU_CCP    = CCP_IOREG_gc;  // Unlock reg.
//		PMIC_CTRL  = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm | PMIC_IVSEL_bm; // Bootloader vector

	_initialize_gpio();
	_swap_vector(1);
	
	//J I2CをSlaveモードで初期化
	sSlaveContext.startCB    = _startCb;
	sSlaveContext.stopCB     = _stopCb;
	sSlaveContext.masterRxCB = _tx;
	sSlaveContext.masterTxCB = _rx;
	_sInitialize_i2cSlave(0x30 + _check_id());
	
	sei();
	
	uint8_t inBootloader = 1;

	//J UARTからデータ読み込んでは書く　の繰り返し
    while (inBootloader) {
		if(si2c_buf.buf.map.status == 0) {
			continue;
		}

		if (si2c_buf.addr == SPINE_MMAP_ADDR_RESTART) {
			if (si2c_buf.buf.map.restart == SPINE_RESTART_KEY) {
				break; // Restart.
			}
		}
		else if(si2c_buf.addr == SPINE_MMAP_ADDR_PROGRAM) {
			if (si2c_buf.buf.map.program == SPINE_PROGRAM_KEY) {
				programFlashPageWithErase(si2c_buf.buf.map.flash_addr, si2c_buf.buf.map.flash_block);
			}
		}
		else if(si2c_buf.addr == SPINE_MMAP_ADDR_DEBUG) {
			if (si2c_buf.buf.map.debug) {
				PORTC.OUTCLR = (1 << 6);
			}
			else {
				PORTC.OUTSET = (1 << 6);
			}
		}
		
		si2c_buf.buf.map.status = 0;
	}
	// END

	_swap_vector(0);
	application_start();
	while(1); //J Goto application.

	return 1;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define I2C_IS_NACK()				(TWIC.MASTER.STATUS & TWI_MASTER_RXACK_bm)
#define I2C_WAIT_FOR_TX_DONE()		{ while (!(TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm)); }
#define I2C_WAIT_FOR_RX_DONE()		{ while (!(TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm)); }
#define I2C_STOP_CONDITION()		(TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc)
#define I2C_RESTART()				(TWIC.MASTER.CTRLC = TWI_MASTER_CMD_REPSTART_gc)

#define ENABLE_I2C_MASTER_INTERRUPT()	{TWIC.MASTER.CTRLA |=  (TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm);}
#define DISABLE_I2C_MASTER_INTERRUPT()	{TWIC.MASTER.CTRLA &= ~(TWI_MASTER_INTLVL_MED_gc | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm);}

/*---------------------------------------------------------------------------*/
#define I2C_SLAVE_CMD_COMPLETE(ack)	(TWIC.SLAVE.CTRLB = (TWI_SLAVE_CMD_COMPTRANS_gc | ((ack) ? TWI_MASTER_ACKACT_bm : 0)))
#define I2C_SLAVE_CMD_RESPONSE(ack)	(TWIC.SLAVE.CTRLB = (TWI_SLAVE_CMD_RESPONSE_gc  | ((ack) ? TWI_MASTER_ACKACT_bm : 0)))

/*---------------------------------------------------------------------------*/
//J I2C Slaveの割込み処理
/*---------------------------------------------------------------------------*/
static volatile uint8_t sI2cSlaveMode;
static volatile uint8_t sIsFirstByte;

ISR(TWIC_TWIS_vect)
{
	//J 割込みステータスを取得
	uint8_t status = TWIC.SLAVE.STATUS;

	/* Bit 6 ? APIF: Address/Stop Interrupt Flag */
	//J Slave Addressが設定したモノと一致したら立つ
	//J CtrlAのPIENが立っていれば、STOP Conditionでも立つ
	if (TWIC.SLAVE.STATUS & TWI_SLAVE_APIF_bm) {
		/* Bit 0 ? AP: Slave Address or Stop */
		//J 1: Slave Address was Hit
		if (TWIC.SLAVE.STATUS & TWI_SLAVE_AP_bm) {
			/* Bit 1 ? DIR: Read/Write Direction */
			// Master Read Operation
			if (TWIC.SLAVE.STATUS & TWI_SLAVE_DIR_bm) {
				sIsFirstByte = 1;
				sI2cSlaveMode = I2C_MASTER_READ;
			}
			// Master Write Operation
			else {
				sI2cSlaveMode = I2C_MASTER_WRITE;
			}

			//J Callback to Applicatioin
			if (sSlaveContext.startCB != NULL) {
				(void) sSlaveContext.startCB(sI2cSlaveMode);
			}

			I2C_SLAVE_CMD_RESPONSE(I2C_SLAVE_RESPONCE_ACK);
		}
		//J 0: Stop Condition
		else {
			//J Callback to Applicatioin
			if (sSlaveContext.stopCB != NULL) {
				sSlaveContext.stopCB();
			}

			// Clear Flag
			TWIC.SLAVE.STATUS = TWI_SLAVE_APIF_bm;

			sI2cSlaveMode = I2C_IDLE;
		}
	}

	/* Bit 7 ? DIF: Data Interrupt Flag */
	//J データ受信完了割込
	//J 1を書くか、CtrlBのCMDに適当なコマンドを書き込んだらフラグクリア
	//J クリアされるまで、SCLはホールドされる
	else if (status & TWI_SLAVE_DIF_bm) {
		uint8_t ack = I2C_SLAVE_RESPONCE_NACK;

		//J Master Read Operation
		if (sI2cSlaveMode == I2C_MASTER_READ) {
			/* Bit 4 ? RXACK: Received Acknowledge */
			//J NACKの場合、もう通信は終わる
			if (status & TWI_SLAVE_RXACK_bm && !sIsFirstByte) {
				//J Nackだからと言って、Slave側は何もすることは無い。死ぬだけや
				if (sSlaveContext.masterRxCB != NULL) {
					sSlaveContext.masterRxCB(I2C_SLAVE_RESPONCE_NACK);
				}

				I2C_SLAVE_CMD_COMPLETE(I2C_SLAVE_RESPONCE_ACK);
			}
			//J ACKの場合、次のデータをデータレジスタに入れてレスポンスを返す
			else {
				sIsFirstByte = 0;

				if (sSlaveContext.masterRxCB != NULL) {
					TWIC.SLAVE.DATA = sSlaveContext.masterRxCB(I2C_SLAVE_RESPONCE_ACK);
					} else {
					TWIC.SLAVE.DATA = 0x00;
				}

				I2C_SLAVE_CMD_RESPONSE(I2C_SLAVE_RESPONCE_ACK); // Clear DIF bit
			}
		}
		//J Master Write Operation
		else if (sI2cSlaveMode == I2C_MASTER_WRITE) {
			if (sSlaveContext.masterTxCB != NULL) {
				ack = sSlaveContext.masterTxCB(TWIC.SLAVE.DATA);
			}

			I2C_SLAVE_CMD_RESPONSE(ack); // Clear DIF bit
		}
	}

	return;
}



