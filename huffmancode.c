#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_NODES 65536
#define WIDTH 1584
#define HEIGHT 396

typedef struct Node {
    size_t frequency;
    unsigned short symbol;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct minHeap {
    Node **arr;
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

void generateCodes(Node* root, char* code, int depth, char* huffmanCode[MAX_NODES]) {
    if (!root) return;
    if (!root->left && !root->right) {
        code[depth] = '\0';
        huffmanCode[root->symbol] = strdup(code);
        return;
    }
    code[depth] = '0';
    generateCodes(root->left, code, depth + 1, huffmanCode);
    code[depth] = '1';
    generateCodes(root->right, code, depth + 1, huffmanCode);
}

Node* encode(unsigned int pairFreq[MAX_NODES], char* huffmanCode[MAX_NODES]) {
    Node* leavesArr[MAX_NODES];
    size_t count = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (pairFreq[i] > 0) {
            Node* n = malloc(sizeof(Node));
            n->symbol = (unsigned short)i;
            n->frequency = pairFreq[i];
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
        newNode->symbol = 0;
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

void decode(Node* root, char* encoded_string, unsigned char* output_buffer){
    Node* current = root;
    size_t idx = 0;
    for(size_t i=0; i<strlen(encoded_string); i++){
        if (encoded_string[i]=='1'){
            current = current->right;
        } else {
            current = current->left;
        }
        if (!current->left && !current->right){
            unsigned char first_byte = (current->symbol >> 8) & 0xFF;
            output_buffer[idx++] = first_byte;
            if (i == strlen(encoded_string) - 1){
                output_buffer[idx] = current->symbol & 0xFF;
            }
            current = root;
        }
    }
}

int main() {
    FILE *fptr = fopen("lyrics-rickroll.txt", "rb");
    FILE *optr = fopen("decoded-rickroll.txt", "wb");
    if (!fptr) {
        perror("File open failed");
        return 1;
    }
    if (!optr) {
        perror("Output File open failed");
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
        fclose(optr);
        return 1;
    }

    fread(buffer, 1, sz, fptr);
    fclose(fptr);

    unsigned int pairFreq[MAX_NODES] = {0};
    char* huffmanCode[MAX_NODES] = {0};

    for (size_t i = 0; i < sz - 1; i++) {
        unsigned short pair = (buffer[i] << 8) | buffer[i+1];
        pairFreq[pair]++;
    }

    Node* root = encode(pairFreq, huffmanCode);
    if (!root) {
        printf("Failed to build Huffman tree\n");
        free(buffer);
        return 1;
    }

    for (int i = 0; i < MAX_NODES; i++) {
        if (huffmanCode[i]) {
            printf("%04X: %s\n", i, huffmanCode[i]);
        }
    }

    size_t originalBits = (sz - 1) * 16;
    size_t compressedBits = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (huffmanCode[i]) {
            compressedBits += pairFreq[i] * strlen(huffmanCode[i]);
        }
    }

    printf("\nOriginal size: %zu bits\n", originalBits);
    printf("Compressed size: %zu bits\n", compressedBits);
    printf("Compression ratio: %.2f%%\n", (100.0 * compressedBits / originalBits));
    printf("\n\n");
    
    char *encoded_string = malloc(1);  // Start with empty string
    if (!encoded_string) {
        perror("malloc failed");
        return 1;
    }
    encoded_string[0] = '\0';

    size_t capacity = 1;

    for (size_t i = 0; i < sz - 1; i++){
        unsigned short pair = (buffer[i] << 8) | buffer[i + 1];
        char *code = huffmanCode[pair];
        size_t code_len = strlen(code);
        size_t current_len = strlen(encoded_string);

        if (current_len + code_len + 1 >= capacity){
            capacity = (current_len + code_len + 1) * 2;
            encoded_string = realloc(encoded_string, capacity);
            if (!encoded_string){
                perror("realloc failed");
                return 1;
            }
        }
        strcat(encoded_string, code);
    }
    printf("encoded code: %s\n",encoded_string);
    size_t encoded_length = strlen(encoded_string);
    size_t compressedBytes = (size_t)ceil(encoded_length / 8.0);

    unsigned char* encoded_buffer = (unsigned char*)malloc(compressedBytes);
    if (!encoded_buffer) {
        perror("malloc failed");
        return 1;
    }

    memset(encoded_buffer, 0, compressedBytes);  

    for (size_t i = 0; i < encoded_length; i++) {
        size_t byte_index = i / 8;
        size_t bit_index = 7 - (i % 8); 

        if (encoded_string[i] == '1') {
            encoded_buffer[byte_index] |= (1 << bit_index);
        }
    }
    printf("\n encoded_buffer Encoded: \n");
    for (size_t i =0; i < compressedBytes; i++){
        printf("%02X ",encoded_buffer[i]);
    }
    printf("\n");
    unsigned char* output_buffer = (unsigned char*)malloc(sz);
    decode(root, encoded_string, output_buffer);
    fwrite(output_buffer, 1, sz, optr);

    // image generation part
    FILE* fontFile = fopen("Terminus.ttf", "rb");
    if (!fontFile) {
        perror("Failed to load font");
        return 1;
    }

    fseek(fontFile, 0, SEEK_END);
    size_t fontSize = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);

    unsigned char* fontBuffer = malloc(fontSize);
    fread(fontBuffer, 1, fontSize, fontFile);
    fclose(fontFile);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer, 0)) {
        fprintf(stderr, "Failed to init font\n");
        return 1;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, 24);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    int baseline = (int)(ascent * scale);

    unsigned char* image = calloc(WIDTH * HEIGHT, 1);

    int x = 8, y = 18;

    char hex[3] = {0};
    int bytes_in_row = 0;

    for (size_t i = 0; i < compressedBytes; i++) {
        snprintf(hex, sizeof(hex), "%02X", encoded_buffer[i]);

        for (int j = 0; j < 2; j++) {
            int glyph = stbtt_FindGlyphIndex(&font, hex[j]);
            int w, h, xoff, yoff;
            unsigned char* bitmap = stbtt_GetGlyphBitmap(&font, scale, scale, glyph, &w, &h, &xoff, &yoff);

            for (int row = 0; row < h; row++) {
                for (int col = 0; col < w; col++) {
                    int px = x + col + xoff;
                    int py = y + row + yoff;
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                        image[py * WIDTH + px] = bitmap[row * w + col];
                    }
                }
            }

            stbtt_FreeBitmap(bitmap, NULL);
            x += 12;
        }

        stbtt_GetGlyphHMetrics(&font, stbtt_FindGlyphIndex(&font, ' '), NULL, NULL);
        x += 12; 

        bytes_in_row++;

        if (bytes_in_row == 44) {
            x = 8;
            y += (int)(baseline + lineGap * scale + 4);
            bytes_in_row = 0;
        }
    }

    unsigned char* rgb_image = malloc(WIDTH * HEIGHT * 3);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        rgb_image[i*3+0] = 0;          
        rgb_image[i*3+1] = image[i];   
        rgb_image[i*3+2] = 0;         
    }

    stbi_write_png("green_encoded_map.png", WIDTH, HEIGHT, 3, rgb_image, WIDTH * 3);
    printf("Image written: green_encoded_map.png\n");

    free(fontBuffer);
    free(image);
    free(rgb_image);

    
    free(buffer);
    free(encoded_buffer);
    free(output_buffer);
    return 0;
}
