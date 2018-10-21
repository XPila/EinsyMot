#ifndef Arduino_h
#define Arduino_h
#include <inttypes.h>
#include <simulator.h>

class Print
{
public:
    virtual size_t write(uint8_t) = 0;
};

class Stream : public Print
{
public:
    virtual int read() = 0;
};

#define SERIAL_8N2 0x0E

class HardwareSerial : public Stream
{
public:
	void begin(unsigned long baud, uint8_t config);
	virtual size_t write(uint8_t);
	virtual int read();
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;


#endif //Arduino_h
