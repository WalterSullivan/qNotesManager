#ifndef CRC32_H
#define CRC32_H

/*
**  CRC.H - header file for SNIPPETS CRC and checksum functions
*/

#include <stdlib.h>           /* For size_t                 */
#include <QtGlobal>

quint32 updateCRC32(unsigned char ch, quint32 crc);
quint32 crc32buf(const char *buf, size_t len);



#endif // CRC32_H
