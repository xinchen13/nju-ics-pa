#include <utils.h>

iRingBuffer iringbuffer;

void init_buffer(iRingBuffer buffer) {
    for (int i = 0; i < IRINGBUF_SIZE; i++) {
        for (int j = 0; j < LOG_LEHGTH; j++) {
            buffer.data[i][j] = '\0';
        }
    }
    buffer.write_ptr = 0;
}

void write_buffer(iRingBuffer buffer, const char *log_buf) {
    int index = (buffer.write_ptr + 1) % IRINGBUF_SIZE;
    // buffer.write_ptr = (buffer.write_ptr + 1) % IRINGBUF_SIZE;
    strcpy(buffer.data[index], log_buf);
    printf("%s\n", log_buf);
    printf("position: %d\n", index);
    buffer.write_ptr = index;
}

void print_buffer(iRingBuffer buffer) {
    for (int current_ptr = 0; current_ptr < IRINGBUF_SIZE; current_ptr++) {
        if (current_ptr == buffer.write_ptr) {
            printf(" --> %s\n", buffer.data[current_ptr]);
        }
        else {
            printf("     %s\n", buffer.data[current_ptr]);
        }         
    }
}