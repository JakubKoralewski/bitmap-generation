#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WINDOWS)
#define SLASH "\\"
#else
#define SLASH "/"
#endif

#define BITMAP_LENGTH 14
#define DIB_LENGTH 40
#define PIXEL_AMOUNT 65536 * 4 // 256^2 each 4 bytes

enum Component {
    B, G, R
};

void printHelp() {
    printf(
            "Bitmap generator by Jakub Koralewski for Introduction to CompSci Labs02 Homework.\n\n"
            "Example usages:\n"
            "./bitmap -c R -i 50\n"
            "./bitmap R 50\n\n"
            "Parameters (names should be lowercase):\n"
            "--help\n"
            "--component, -c (R, G, B)\n"
            "--intensity, -i (0-255; base 10)\n"
            "--output, -o name of output bitmap (default \"bitmap.bmp\")\n"
    );
}


void invalidComponent() {
    printf("Invalid component name! (R, G, B; case-insensitive)\n");
    printHelp();
    exit(1);
}

/* Checks for component name. */
enum Component checkColor(char comp[]) {
    if (strcmp("R", comp) == 0 || strcmp("r", comp) == 0) {
        return R;
    } else if (strcmp("G", comp) == 0 || strcmp("g", comp) == 0) {
        return G;
    } else if (strcmp("B", comp) == 0 || strcmp("b", comp) == 0) {
        return B;
    } else {
        invalidComponent();
    }
}

int main(int argc, char* argv[]) {
    unsigned char intensity = 0;
    enum Component component;
    char* file_name = "bitmap.bmp";

    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0) {
            printHelp();
            return 0;
        } else {
            printf("One argument is not enough!\n");
            printHelp();
            return 1;
        }
    } else if (argc == 1) {
        printf("You need to specify component and intensity!\n");
        printHelp();
        return 1;
    } else {
        int i = 1;
        while (i < argc) {
            char* arg = argv[i];
            if (strcmp("-c", arg) == 0 || strcmp("--component", arg) == 0) {
                char* comp = argv[i + 1];
                component = checkColor(comp);
                i += 2;
            } else if (strcmp("-i", arg) == 0 || strcmp("--intensity", arg) == 0) {
                intensity = (unsigned char) strtol(argv[i + 1], NULL, 10);
                i += 2;
            } else if (strcmp("-o", arg) == 0 || strcmp("--output", arg) == 0) {
                file_name = (char*) argv[i + 1];
                i += 2;
            } else {
                // nameless arguments, e.g.: R 50
                component = checkColor(arg);
                intensity = (unsigned char) strtol(argv[i + 1], NULL, 10);
                if (argc == 4) {
                    file_name = argv[i + 2];
                }
                break;
            }
        }
    }

    unsigned char BITMAP_HEADER[BITMAP_LENGTH];
    for (unsigned char i = 0; i < BITMAP_LENGTH; i++) {
        BITMAP_HEADER[i] = 0x00;
    }

    BITMAP_HEADER[0] = 0x42; // "B"
    BITMAP_HEADER[1] = 0x4D; // "M"

    BITMAP_HEADER[2] = 0x36; // file size = (header)0x36 + PIXEL_AMOUNT
    BITMAP_HEADER[4] = 0x04;

    BITMAP_HEADER[10] = 0x36; // offset

    unsigned char DIB_HEADER[DIB_LENGTH];
    for (int j = 0; j < DIB_LENGTH; ++j) {
        DIB_HEADER[j] = 0x00;
    }

    DIB_HEADER[0] = DIB_LENGTH; // length of DIB_HEADER 40 bytes
    DIB_HEADER[5] = 0x01; // width 256
    DIB_HEADER[9] = 0x01; // height 256
    DIB_HEADER[12] = 0x01; // 1 color plane
    DIB_HEADER[14] = 0x18; // 24 bits per pixel
    DIB_HEADER[16] = 0x00; // no compression
    DIB_HEADER[20] = 0x04; // amount of data, PIXEL_AMOUNT in little-endian

    unsigned char PIXELS[PIXEL_AMOUNT];
    unsigned int loc = 0;
    unsigned int padding = 0;
    for (int k = 0xFF; k >= 0; --k) { // start at the bottom
        for (int l = 0; l <= 0xFF; ++l) {
            PIXELS[loc + ((component + 2) % 3)] = k;
            PIXELS[loc + ((component + 1) % 3)] = l;
            PIXELS[loc + component] = intensity; // chosen component with chosen intensity
            loc += 3;
        }
    }

    FILE* fp;
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    file_name = strcat(strcat(cwd, SLASH), file_name);
    fp = fopen(file_name, "w+b");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    fwrite(BITMAP_HEADER, sizeof(char), BITMAP_LENGTH, fp);
    fwrite(DIB_HEADER, sizeof(char), DIB_LENGTH, fp);
    fwrite(PIXELS, sizeof(char), PIXEL_AMOUNT, fp);
    printf("Saved bitmap to: \"%s\"\n", file_name);

    fclose(fp);

    return 0;
}
