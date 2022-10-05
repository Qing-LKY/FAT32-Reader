#include "read.c"

int main() {
    char *s = "a.img";
    int code, x;
    code = load_disk_image(s);
    printf("%d %d\n", code, img_fd);
    code = load_DBR_info();
    printf("%d\n", code);
    code = disp_DBR_info();
    printf("%d\n", code);
    return 0;
}