//eeprom.h
#ifndef _AVR_EEPROM_H_
#define _AVR_EEPROM_H_

#include <inttypes.h>


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

uint8_t eeprom_read_byte (const uint8_t *__p);
uint8_t eeprom_write_byte (const uint8_t *__p, uint8_t value);
uint8_t eeprom_update_byte (const uint8_t *__p, uint8_t value);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_AVR_EEPROM_H_
