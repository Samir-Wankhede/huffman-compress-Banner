#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fptr = fopen("rickroll.mp4", "rb");
    FILE *fp = fopen("nth_notes.txt", "w");
    FILE *fnewVideo = fopen("nth_notes.mp4", "wb");
    
    if (fptr == NULL || fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    fseek(fptr, 0L, SEEK_END);
    size_t sz = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);

    printf("File size: %zu bytes\n", sz);

    unsigned char *buffer = (unsigned char*) malloc(sz);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(fptr);
        fclose(fp);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, sz, fptr);
    if (read_bytes != sz) {
        fprintf(stderr, "Warning: only read %zu of %zu bytes\n", read_bytes, sz);
    }

    size_t write_bytes = fwrite(buffer, 1, sz, fnewVideo);
    if (write_bytes!=sz){
        fprintf(stderr, "Warning: only wrote %zu of %zu bytes\n", read_bytes, sz);
    }

    for (long long i = 0; i < sz/100; i++) {
        fprintf(fp, "%02X ", buffer[i]);
    }

    for (int i = 0; i < 32 && i < sz; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");

    free(buffer);
    fclose(fptr);
    fclose(fp);

    return 0;
}
