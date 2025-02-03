#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Function to compare two strings for sorting (used in qsort)
int compareStrings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Hashing function to generate a key based on sorted string
unsigned long hashString(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Function to check if two strings are anagrams
bool areAnagrams(const char* str1, const char* str2) {
    if (strlen(str1) != strlen(str2)) return false;

    char sortedStr1[strlen(str1) + 1];
    char sortedStr2[strlen(str2) + 1];

    strcpy(sortedStr1, str1);
    strcpy(sortedStr2, str2);

    qsort(sortedStr1, strlen(sortedStr1), sizeof(char), compareStrings);
    qsort(sortedStr2, strlen(sortedStr2), sizeof(char), compareStrings);

    return strcmp(sortedStr1, sortedStr2) == 0;
}

// Function to find anagrams using a hash map
void findAnagramsUsingHashMap(char* list[], int n) {
    // Map to store sorted string as a key and list of anagrams as values
    unsigned long* hashTable = (unsigned long*)malloc(n * sizeof(unsigned long));
    
    for (int i = 0; i < n; i++) {
        hashTable[i] = hashString(list[i]); // Store hash of sorted string
    }

    // Find anagrams by comparing hashes
    printf("Anagram groups:\n");
    for (int i = 0; i < n; i++) {
        if (i == 0 || hashTable[i] != hashTable[i - 1]) {
            printf("Anagram group for: %s\n", list[i]);
            for (int j = i; j < n; j++) {
                if (hashTable[i] == hashTable[j]) {
                    printf("%s\n", list[j]);
                }
            }
        }
    }

    free(hashTable); // Free memory used for hash table
}

int main() {
    char* list[] = {
        "stop",
        "pots",
        "opt",
        "cat",
        "act",
        "tac",
        "dog",
        "god",
        "supercalifragilistic"
    };

    int n = sizeof(list) / sizeof(list[0]);

    // Find anagrams using hash map
    findAnagramsUsingHashMap(list, n);

    return 0;
}
