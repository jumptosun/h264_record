#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>

int trim_annexb_prefix(uint8_t*& p, int& len);

int get_nal_type(uint8_t* p, int len);

int get_vol_type(uint8_t* p, int len);

#endif