/*
 * backup_ram.h
 *
 * Created: 2022/02/02 5:40:27
 *  Author: kiyot
 */ 


#ifndef BACKUP_RAM_H_
#define BACKUP_RAM_H_

typedef struct {
	uint32_t update_req;
	uint32_t fw_size;
} LIB4LEGS_BACKUP_RAM;

extern LIB4LEGS_BACKUP_RAM gBackupRam __attribute__ ((section (".bkupram"))) ;

#endif /* BACKUP_RAM_H_ */