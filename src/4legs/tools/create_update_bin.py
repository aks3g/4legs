import sys
import struct
import math

FLASH_BLOCK_SIZE = 512

def checksum(byte_arr):
	dword_num = int(len(byte_arr) / 4)
	dword_arr = struct.unpack('<' + 'I'*dword_num, byte_arr)

	sum = 0;
	for d in dword_arr:
		sum = sum + d;

	return sum & 0xffffffff

def main():

	with open(sys.argv[1], 'rb') as f:
		fwbin = f.read()
		print('Fw size = %d[byte]'%(len(fwbin)))
		fwbin = fwbin + b'\xff'*(FLASH_BLOCK_SIZE - (len(fwbin) % FLASH_BLOCK_SIZE))

	if len(sys.argv) >= 4:
		magic = int(sys.argv[3], 16)
	else :
		magic   = 0xaba00101

	version = 0x00000000;
	fwsize  = len(fwbin);
	sum = checksum(fwbin)

	header = struct.pack('<I', magic) + struct.pack('<I', version) +  struct.pack('<I', fwsize) +  struct.pack('<I', sum)

	with open(sys.argv[2], 'wb') as f:
		f.write(header)
		padding_size = 128 - len(header)
		f.write(b'\xff'*padding_size)
		f.write(fwbin)

if __name__ == '__main__':
	main()

