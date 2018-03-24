//以下是des.c文件全部：
//密钥：       B4 31 5B 86 9D 7D FA A2
//数据：       1F AD 61 A5 F7 19 77 14
//DES加密结果：4C 78 E9 1A F2 DA 9C D3
#include <stdlib.H>
#include <stdio.h>
#include <string.h>
const unsigned char initial_tr[64] =
{
 57, 49, 41, 33, 25, 17,  9,  1,
 59, 51, 43, 35, 27, 19, 11,  3,
 61, 53, 45, 37, 29, 21, 13,  5,
 63, 55, 47, 39, 31, 23, 15,  7,
 56, 48, 40, 32, 24, 16,  8,  0,
 58, 50, 42, 34, 26, 18, 10,  2,
 60, 52, 44, 36, 28, 20, 12,  4,
 62, 54, 46, 38, 30, 22, 14,  6
};
const unsigned char final_tr[64] =
{
 39,  7, 47, 15, 55, 23, 63, 31,
 38,  6, 46, 14, 54, 22, 62, 30,
 37,  5, 45, 13, 53, 21, 61, 29,
 36,  4, 44, 12, 52, 20, 60, 28,
 35,  3, 43, 11, 51, 19, 59, 27,
 34,  2, 42, 10, 50, 18, 58, 26,
 33,  1, 41,  9, 49, 17, 57, 25,
 32,  0, 40,  8, 48, 16, 56, 24
};
const unsigned char swap[64] =
{
 33, 34, 35, 36, 37, 38, 39, 40,
 41, 42, 43, 44, 45, 46, 47, 48,
 49, 50, 51, 52, 53, 54, 55, 56,
 57, 58, 59, 60, 61, 62, 63, 64,
  1,  2,  3,  4,  5,  6,  7,  8,
  9, 10, 11, 12, 13, 14, 15, 16,
 17, 18, 19, 20, 21, 22, 23, 24,
 25, 26, 27, 28, 29, 30, 31, 32
};
const unsigned char key_tr1[56] =
{
 56, 48, 40, 32, 24, 16,  8,
  0, 57, 49, 41, 33, 25, 17,
  9,  1, 58, 50, 42, 34, 26,
 18, 10,  2, 59, 51, 43, 35,
 62, 54, 46, 38, 30, 22, 14,
  6, 61, 53, 45, 37, 29, 21,
 13,  5, 60, 52, 44, 36, 28,
 20, 12,  4, 27, 19, 11,  3
};
const unsigned char key_tr2[64] =
{
 0,  0, 13,  4, 16, 10, 23,  0,
 0,  0,  2,  9, 27, 14,  5, 20,
 0,  0, 22,  7, 18, 11,  3, 25,
 0,  0, 15,  1,  6, 26, 19, 12,
 0,  0, 40, 54, 51, 30, 36, 46,
 0,  0, 29, 47, 39, 50, 44, 32,
 0,  0, 43, 52, 48, 38, 55, 33,
 0,  0, 45, 31, 41, 49, 35, 28
};
const unsigned char etr[64] =
{
 0,  0, 31,  4,  0,  1,  2,  3,
 0,  0,  3,  8,  4,  5,  6,  7,
 0,  0,  7, 12,  8,  9, 10, 11,
 0,  0, 11, 16, 12, 13, 14, 15,
 0,  0, 15, 20, 16, 17, 18, 19,
 0,  0, 19, 24, 20, 21, 22, 23,
 0,  0, 23, 28, 24, 25, 26, 27,
 0,  0, 27,  0, 28, 29, 30, 31
};
const unsigned char ptr[32] =
{
 31, 14, 39, 44, 60, 23, 55, 36,
  4, 30, 46, 53, 12, 37, 62, 21,
  5, 15, 47, 29, 63, 54,  6, 20,
 38, 28, 61, 13, 45, 22,  7, 52
};
const unsigned char s[8][64] =
{
 {
  14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
   0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
   4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
  15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
 },
 {
  15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
   3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
   0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
  13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
 },
 {
  10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
  13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
  13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
   1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
 },
 {
   7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
  13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
  10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
   3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
 },
 {
   2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
  14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
   4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
  11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
 },
 {
  12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
  10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
   9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
   4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7, 6,  0,  8,  13
 },
 {
   4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
  13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
   1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
   6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
 },
 {
  13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
   1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
   7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
   2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
 }
};
const unsigned char rots[16] =
{
 1,  1,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  1
};
const unsigned char bit_msk[8] =
{
 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
unsigned char DES_Encrypt_key[8];
unsigned char DES_Decrypt_key[8];
unsigned char sub_keys[16][8]; //sub_keys[16][8]
unsigned char main_key[8];
        void    des(unsigned char*, unsigned char*, unsigned char, unsigned char*);
  void  FLASH_Read_KEYS(unsigned char key_index);
static  void    transpose (unsigned char*, unsigned char*, const unsigned char*, unsigned char);
static  void    rotate_l (unsigned char*);
static  void    compute_subkeys (unsigned char*);
static  void    f (unsigned char*, unsigned char*, unsigned char*);
/************************************************************************/
/*                                                                      */
/*      Functional Description: Encipher and decipher 64 bits string    */
/*                              according to a 64 bits key string       */
/*                              The string format is shown below        */
/*                                                                      */
/*      input parameter 1:      pointer to 64 bits input string         */
/*                      2:      pointer to 64 bits key string           */
/*                      3:      boolean if false indicating enciphering */
/*                              if true dechiphering                    */
/*                      4:      pointer to a 64 bits output string      */
/************************************************************************/
/*                                                                      */
/*                      msb                     lsb                     */
/*                      bit                     bit                     */
/*                      +-- -- -- -- -- -- -- --+                       */
/*              addr    !1st                 8th!                       */
/*                      +-- -- -- -- -- -- -- --+                       */
/*              addr+1  !9th                16th!                       */
/*                      +-- -- -- -- -- -- -- --+                       */
/*                      :                       :                       */
/*                      :                       :                       */
/*                      +-- -- -- -- -- -- -- --+                       */
/*              addr+7  !57th               64th!                       */
/*                      +-- -- -- -- -- -- -- --+                       */
/*                                                                      */
/************************************************************************/
void des(unsigned char *plain_strng, unsigned char *key, unsigned char d, unsigned char *ciph_strng)
{
  unsigned char a_str[8], b_str[8], x_str[8];
  unsigned char i, j, *pkey, temp;
    for (i = 0; i < 8 ; ++i)
 {
  if (key[i] != main_key[i])
        {
   compute_subkeys(key);
   i = 7;
        }
 }
 
    transpose(plain_strng, a_str, initial_tr, 64);
    for (i=1; i < 17; ++i)
    {
  for (j=0; j < 8; ++j){b_str[j] = a_str[j];}
  
  if (!d)           /*enchipher*/
   pkey = &sub_keys[i-1][0];
  else                /*dechipher*/
   pkey = &sub_keys[16-i][0];
  
  for (j=0; j < 4; ++j){a_str[j] = b_str[j+4];}
   
  f(pkey, a_str, x_str);
   
  for (j=0; j < 4; ++j) {a_str[j+4] = b_str[j] ^ x_str[j];}
    }
    
    temp = a_str[0]; a_str[0] = a_str[4]; a_str[4] = temp;
    temp = a_str[1]; a_str[1] = a_str[5]; a_str[5] = temp;
    temp = a_str[2]; a_str[2] = a_str[6]; a_str[6] = temp;
    temp = a_str[3]; a_str[3] = a_str[7]; a_str[7] = temp;
    transpose(a_str, ciph_strng, final_tr, 64);
}
/************************************************************************/
/*                                                                      */
/*      Functional Description: Permute n bits in a string, according   */
/*                              to a table describing the new order.    */
/*                              0 < n <= 64                             */
/*                                                                      */
/*      input parameter 1:      pointer to first byte in input string   */
/*                      2:      pointer to first byte in output string  */
/*                      3:      pointer to table describing new order   */
/*                      4:      number of bits to be permuted           */
/************************************************************************/
void transpose(unsigned char *idata, unsigned char *odata, const unsigned char *tbl, unsigned char n)
{
 const unsigned char *tab_adr;
 int i, bi_idx;
 tab_adr = &bit_msk[0];
 i = 0;
 
 do
    {odata[i++] = 0;}
 while (i < 8);
 i = 0;
 do
 {
        bi_idx = *tbl++;
        if (idata[bi_idx>>3] & tab_adr[bi_idx & 7])
  {
   odata[i>>3] |= tab_adr[i & 7];
  }
 }
 while (++i < n);
}
/************************************************************************/
/*                                                                      */
/*      Functional Description: rotate 2 concatenated strings of 28     */
/*                              bits one position to the left.          */
/*                                                                      */
/*      input parameter 1:      pointer to first byte in key string     */
/*                                                                      */
/************************************************************************/
void rotate_l(unsigned char *key)
{
  unsigned char str_x[8];
  unsigned char i;
      for (i=0; i < 8; ++i) str_x[i] = key[i];
      for (i=0; i < 7; ++i)
      {
        key[i] = (key[i] << 1);
        if ((i < 6) && ((str_x[i+1] & 128) == 128))
          key[i] |= 1;
      }
      if (str_x[0] & 0x80 )
        key[3] |= 0x10;
      else
        key[3] &= ~0x10;
      if (str_x[3] & 0x08 )
        key[6] |= 0x01;
      else
        key[6] &= ~0x01;
}
/************************************************************************/
/*                                                                      */
/*      Functional Description: Computes the 16 sub keys for use in the */
/*                              DES algorithm                           */
/*                                                                      */
/*      input parameter 1:      pointer to first byte in key string     */
/*      output           :      fills the array sub_keys[16][8] with    */
/*                              sub keys and stores the input key in    */
/*                              main_key[8]                             */
/************************************************************************/
void compute_subkeys(unsigned char *key)
{
 unsigned char i, j, ikey[8], okey[8];
 for (i=0; i < 8; ++i)
 {
  main_key[i] = key[i];
 }
 
 transpose(key, ikey, key_tr1, 56);
 
 for (i=0; i < 16; ++i)
    {
  for (j=0; j < rots[i]; ++j) {rotate_l(ikey);}
   transpose(ikey, okey, key_tr2,  64);
  for (j=0; j < 8; ++j)
  { sub_keys[i][j] = okey[j];}
    }
}
/************************************************************************/
/*                                                                      */
/*      Functional Description: The chipher function                    */
/*                                                                      */
/*      input parameter 1:      pointer to first byte in key string     */
/*                      2:      pointer to a 32 bit input string        */
/*                      3:      pointer to a 32 bit output string       */
/************************************************************************/
void f(unsigned char *skey, unsigned char *a_str, unsigned char *x_str)
{
 unsigned char e_str[8], y_str[8], z_str[8];
 unsigned char k;
 transpose(a_str, e_str, etr, 64);
 for (k=0; k < 8; ++k)
 {
        y_str[k] = (e_str[k] ^ skey[k]) & 63;
        z_str[k] = s[k] [y_str[k]];
 }
 transpose(z_str, x_str, ptr, 32);
}

