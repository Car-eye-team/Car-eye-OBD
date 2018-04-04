#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern void des(unsigned char *plain_strng, unsigned char *key, unsigned char d, unsigned char *ciph_strng);
/*握手过程的加密算法
*/
unsigned char paramcodeLink(unsigned char *data, unsigned char datalen, unsigned char *id)
{
	unsigned char u8temp;	
	unsigned char in[8];
	unsigned char out[8];
	unsigned char key[8];
	
	if(datalen != 16)return 1;
	for(u8temp = 0; u8temp < datalen; u8temp ++)
	{
		*(data + u8temp) = (*(data +u8temp)) ^ (*(id + u8temp));
	}
	key[0] = 0x33;
	key[1] = 0xda;
	key[2] = 0x32;
	key[3] = 0x10;
	key[4] = 0x1a;
	key[5] = 0xcc;
	key[6] = 0xa3;
	key[7] = 0xaf;
	memcpy(in, data, 8);
	des(in, key, 0, out);
	memcpy(data, out, 8);
	memcpy(in, data + 8, 8);
	des(in, key, 0, out);
	memcpy(data + 8, out, 8);
	
	return 0;
}

