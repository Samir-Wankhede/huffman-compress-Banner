#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_NODES 256

typedef struct Node {
    size_t frequency;
    unsigned char byte;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct minHeap {
    Node **arr;     // change to array of pointers
    size_t size;
    size_t capacity;
} minHeap;

void swap(Node** a, Node** b) {
    Node* temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(minHeap* h, size_t i) {
    size_t l = 2 * i + 1;
    size_t r = 2 * i + 2;
    size_t min = i;

    if (l < h->size && h->arr[min]->frequency > h->arr[l]->frequency) min = l;
    if (r < h->size && h->arr[min]->frequency > h->arr[r]->frequency) min = r;

    if (min != i) {
        swap(&h->arr[i], &h->arr[min]);
        heapify(h, min);
    }
}

minHeap* create_heap(size_t capacity, Node **leaves) {
    minHeap *h = malloc(sizeof(minHeap));
    if (!h) return NULL;

    h->arr = malloc(sizeof(Node*) * capacity);
    if (!h->arr) return NULL;

    h->capacity = capacity;
    h->size = capacity;

    for (size_t i = 0; i < capacity; i++) {
        h->arr[i] = leaves[i];
    }

    for (int j = (int)(h->size / 2) - 1; j >= 0; j--) {
        heapify(h, j);
    }
    return h;
}

Node* extractMin(minHeap* h) {
    if (h->size == 0) return NULL;
    Node* minNode = h->arr[0];
    h->arr[0] = h->arr[h->size - 1];
    h->size--;
    heapify(h, 0);
    return minNode;
}

void insertHelper(minHeap* h, size_t idx) {
    if (idx == 0) return;
    size_t parent = (idx - 1) / 2;
    if (h->arr[parent]->frequency > h->arr[idx]->frequency) {
        swap(&h->arr[parent], &h->arr[idx]);
        insertHelper(h, parent);
    }
}

void insertHeap(minHeap* h, Node* node) {
    if (h->size < h->capacity) {
        h->arr[h->size] = node;
        insertHelper(h, h->size);
        h->size++;
    }
}

void generateCodes(Node* root, char* code, int depth, char* huffmanCode[256]) {
    if (!root) return;
    if (!root->left && !root->right) {
        code[depth] = '\0';
        huffmanCode[root->byte] = strdup(code);
        return;
    }
    code[depth] = '0';
    generateCodes(root->left, code, depth + 1, huffmanCode);
    code[depth] = '1';
    generateCodes(root->right, code, depth + 1, huffmanCode);
}

Node* encode(unsigned int byteFreq[256], char* huffmanCode[256]) {
    Node* leavesArr[256];
    size_t count = 0;

    for (int i = 0; i < 256; i++) {
        if (byteFreq[i] > 0) {
            Node* n = malloc(sizeof(Node));
            n->byte = (unsigned char)i;
            n->frequency = byteFreq[i];
            n->left = NULL;
            n->right = NULL;
            leavesArr[count++] = n;
        }
    }

    if (count == 0) return NULL;

    minHeap* h = create_heap(count, leavesArr);
    if (!h) return NULL;

    while (h->size > 1) {
        Node* left = extractMin(h);
        Node* right = extractMin(h);

        Node* newNode = malloc(sizeof(Node));
        newNode->byte = 0;
        newNode->frequency = left->frequency + right->frequency;
        newNode->left = left;
        newNode->right = right;

        insertHeap(h, newNode);
    }

    Node* root = h->arr[0];
    char code[256];
    generateCodes(root, code, 0, huffmanCode);

    free(h->arr);
    free(h);
    return root;
}

int main() {
    FILE *fptr = fopen("text.txt", "rb");
    if (!fptr) {
        perror("File open failed");
        return 1;
    }

    fseek(fptr, 0L, SEEK_END);
    size_t sz = ftell(fptr);
    fseek(fptr, 0L, SEEK_SET);
    printf("File size: %zu bytes\n", sz);

    unsigned char *buffer = malloc(sz);
    if (!buffer) {
        perror("Malloc failed");
        fclose(fptr);
        return 1;
    }

    fread(buffer, 1, sz, fptr);
    fclose(fptr);

    unsigned int byteFreq[256] = {0};
    char* huffmanCode[256] = {0};

    for (size_t i = 0; i < sz; i++) {
        byteFreq[buffer[i]]++;
    }

    Node* root = encode(byteFreq, huffmanCode);
    if (!root) {
        printf("Failed to build Huffman tree\n");
        free(buffer);
        return 1;
    }

    for (int i = 0; i < 256; i++) {
        if (huffmanCode[i]) {
            printf("%02X: %s\n", i, huffmanCode[i]);
            free(huffmanCode[i]);
        }
    }

    free(buffer);
    return 0;
}
