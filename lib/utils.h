
#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
struct bridge_ipinfo {
    uint32_t ip; //������
    uint32_t prefix; //��������ǰ׺ 16 24��
};
struct bridge_ipinfo get_brip(const char *br);
void new_containerip(char *ipaddr, int len);

#endif

