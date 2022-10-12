#include "interact.h"

int main() {
#ifdef SHOW_CLUSTER
    puts("SHOW_CLUSTER ON");
#endif
    while(1) interact_normal();
    return 0;
}