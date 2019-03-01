#ifndef _SD_READER_H
#define _SD_READER_H

#include <stdint.h>

typedef struct str_t str;
typedef struct sd_reader_t sd_reader;

sd_reader* sd_reader_open(const char *file);

/**
 * Check for errors
 */
int sd_reader_errno(const sd_reader *reader);

void sd_reader_close(sd_reader *reader);
int sd_reader_ok(const sd_reader *reader);

long sd_reader_pos(sd_reader *reader);
long sd_reader_filesize(const sd_reader *reader);
int sd_reader_set(sd_reader *reader, long pos);

int sd_read_buf(sd_reader *reader, char *buf, int len);
int sd_peek_buf(sd_reader *reader, char *buf, int len);

int sd_read_str_from_buf(sd_reader *reader, str *buf);
int sd_read_str_from_cstr(sd_reader *reader, str *buf);

uint8_t sd_read_ubyte(sd_reader *reader);
uint16_t sd_read_uword(sd_reader *reader);
uint32_t sd_read_udword(sd_reader *reader);
int8_t sd_read_byte(sd_reader *reader);
int16_t sd_read_word(sd_reader *reader);
int32_t sd_read_dword(sd_reader *reader);
float sd_read_float(sd_reader *reader);

uint8_t sd_peek_ubyte(sd_reader *reader);
uint16_t sd_peek_uword(sd_reader *reader);
uint32_t sd_peek_udword(sd_reader *reader);
int8_t sd_peek_byte(sd_reader *reader);
int16_t sd_peek_word(sd_reader *reader);
int32_t sd_peek_dword(sd_reader *reader);
float sd_peek_float(sd_reader *reader);

int sd_read_scan(const sd_reader *reader, const char* format, ...);
int sd_read_line(const sd_reader *reader, char *buffer, int maxlen);

/**
  * Compare following nbytes amount of data and given buffer. Does not advance file pointer.
  */
int sd_match(sd_reader *reader, char *buf, unsigned int nbytes);

/**
  * Skip following nbytes amount of data.
  */
void sd_skip(sd_reader *reader, unsigned int nbytes);

char* sd_read_variable_str(sd_reader *r);

#endif // _SD_READER_H
