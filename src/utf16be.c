#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf16be_to_utf16le(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  size_t bytes_to_write;
  int ret = -1;

  bom = UTF16LE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);
  read_to_bigendian(infile, &(buf.upper_bytes), 2); /*garbage*/
  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    bytes_to_write = 2; /* utf-32 future compatibility */
    reverse_bytes(&(buf.upper_bytes), 2);
    if(is_lower_surrogate_pair(buf)) {
      if((bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2) > 0)) {
        break;
      }
      reverse_bytes(&(buf.lower_bytes), 2);
      bytes_to_write += 2;
    }
    write_to_bigendian(outfile, &buf, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}

int
from_utf16be_to_utf8(int infile, int outfile)
{

  int bom;
  utf8_glyph_t buf;
  ssize_t bytes_read;
  //size_t remaining_bytes;
  size_t size_of_glyph;
  code_point_t code_point;
  code_point_t code_point_temp;
  utf16_glyph_t utf16_buf;
  utf16_glyph_t utf16_buf2;
  int ret = 0;

  bom = UTF8;
  //bom = 0xEFBBBF;
  //reverse_bytes(&bom, 3);

  write_to_bigendian(outfile, &bom, 3);

  read_to_bigendian(infile, &utf16_buf.lower_bytes, 2);/*garbage */
  while((bytes_read = read_to_bigendian(infile, &utf16_buf.upper_bytes, 1) + read_to_bigendian(infile, &utf16_buf.lower_bytes, 1)) > 0) {
    code_point = ((utf16_buf.upper_bytes << 8) & 0xFF00) + ((utf16_buf.lower_bytes) & 0x00FF);
    utf16_buf.upper_bytes = code_point;
    utf16_buf.lower_bytes = code_point;
    if(is_upper_surrogate_pair(utf16_buf)){
        (bytes_read = read_to_bigendian(infile, &utf16_buf2.upper_bytes, 1) + read_to_bigendian(infile, &utf16_buf2.lower_bytes, 1));

        code_point_temp = ((utf16_buf2.upper_bytes << 8) & 0xFF00) + ((utf16_buf2.lower_bytes) & 0x00FF);
        utf16_buf2.upper_bytes = code_point_temp;
        utf16_buf2.lower_bytes = code_point_temp;
        if(!is_lower_surrogate_pair(utf16_buf2)){
          return -1;
        }
        code_point -= 0xD800;
        code_point <<= 10;
        code_point = code_point & 0xFFC00;
        code_point_temp -= 0xDC00;
        code_point_temp = code_point_temp & 0x003FF;

      //code_point = decode_both_utf16_bytes(utf16_buf, utf16_buf2);

  code_point = code_point + code_point_temp + 0x10000;

  }
    if(code_point != 0xBFBBEF && code_point != 0xEFBBBF){
        buf = code_point_to_utf8(code_point, &size_of_glyph);
        write_to_bigendian(outfile, &buf, size_of_glyph);
  }
  }
  ret = bytes_read;


  return ret;
}

utf16_glyph_t
code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;

  memeset(&ret, 0, sizeof ret);
  if(is_code_point_surrogate(code_point)) {
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
    reverse_bytes(&ret.lower_bytes, 2);
  #endif
    *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
  #endif
    *size_of_glyph = 2;
  }
  return ret;
}
