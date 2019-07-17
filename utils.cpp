#include "utils.hpp"

int trim_annexb_prefix(uint8_t*& p, int& len)
{
    if (len < 4)
        return -1;

    uint32_t prefix = *((uint32_t*)p);
    if ((prefix & 0x00FFFFFF) == 0x010000) {
        p += 3;
        len -= 3;

    } else if (prefix == 0x01000000) {
        p += 4;
        len -= 4;

    } else {
        return -1;
    }

    return 0;
}

int get_nal_type(uint8_t* p, int len)
{
    if (!p || 5 >= len)
        return -1;

    char type;
    uint32_t prefix = *((uint32_t*)p);

    if ((prefix & 0x00FFFFFF) == 0x010000) {
        type = ((char*)p)[3];

    } else if (prefix == 0x01000000) {
        type = ((char*)p)[4];

    } else {
        return -1;
    }

    return (type & 0x1f);
}

// < 0 = error
// 0 = I-Frame
// 1 = P-Frame
// 2 = B-Frame
// 3 = S-Frame
int get_vol_type(uint8_t* p, int len)
{
    if (!p || 6 >= len)
        return -1;

    // return p[0] & 0x1f;

    uint8_t* pos;
    uint32_t prefix = *((uint32_t*)p);

    if ((prefix & 0x00FFFFFF) == 0x010000) {
        pos = p + 3;

    } else if (prefix == 0x01000000) {
        pos = p + 4;
    }

    // Verify VOP id
    if (0xb6 == *pos) {
        pos++;
        return (*pos & 0xc0) >> 6;
    } // end if

    switch ((*pos) & 0x1f) {
        // case 0x05 :
    case 0x07:
        return 0;

    case 0x01:
        return 1;

    default:
        break;
    } // end switch

    return -1;
}
