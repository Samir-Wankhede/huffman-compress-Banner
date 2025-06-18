#include <stdio.h>
#include <stdlib.h>
#define MAX_NODES = 256

typedef struct Node {
    size_t frequency;
    unsigned char byte;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct minHeap {
    Node *arr;
    size_t size;
    size_t capacity;
} minHeap;

minHeap* create_heap(size_t capacity, Node *leaves){
    minHeap *h = (minHeap*)malloc(sizeof(minHeap));
    if (h == NULL){
        printf("Memory error");
        return NULL;
    }
    h->size = 0;
    h->capacity = capacity;
    h->arr = (Node*)malloc(sizeof(Node) * capacity);
    if (h->arr == NULL) {
        printf("Memory error");
        return NULL;
    }
    size_t i;
    for(i=0; i<capacity; i++){
        h->arr[i] = leaves[i];
    }
    h->size = i;
    i = (h->size / 2) - 1;
    while(i>=0){
        heapify(h, i);
        i--;
    }
    return h;
}

void swap(Node* a, Node* b) {
    Node temp = *a;
    *a = *b;
    *b = temp;
}


void heapify(minHeap* h, size_t i){
    size_t l = 2 * i + 1;
    size_t r = 2 * i + 2;
    size_t min = i;
    if (l < h->size && h->arr[min].frequency > h->arr[l].frequency){
        min = l;
    }
    if (r < h->size && h->arr[min].frequency > h->arr[r].frequency){
        min = r;
    }
    if (min != i){
        swap(&h->arr[i], &h->arr[min]);
        heapify(h, min);
    }
}

Node* extractMin(minHeap* h)
{
    Node* deleteItem;
    if (h->size == 0) {
        printf("\nHeap id empty.");
        return NULL;
    }
    deleteItem = &h->arr[0];
    h->arr[0] = h->arr[h->size - 1];
    h->size--;
    heapify(h, 0);
    return deleteItem;
}

void insertHelper(minHeap* h, size_t idx){
    size_t parent = (idx-1)/2;
    if (h->arr[parent].frequency > h->arr[idx].frequency){
        swap(&h->arr[parent], &h->arr[idx]);
        insertHelper(h, parent);
    }
}

void insertHeap(minHeap* h, Node node){
    if (h->size < h->capacity) {
        h->arr[h->size] = node;
        insertHelper(h, h->size);
        h->size++;
    }
}

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
