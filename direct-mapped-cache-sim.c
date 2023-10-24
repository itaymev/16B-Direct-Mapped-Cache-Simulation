#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 2

struct Set {
    unsigned char data[BLOCK_SIZE];
    unsigned int tag; // Assume tag is at most 32 bits
    unsigned char valid; // valid bit (0 or 1)
};

struct Cache {
    struct Set *sets;
    int numSets;
};

unsigned int getOffset(unsigned int address) {
    // log2(2) = 1
    // The offset will be contained in the lowest order bit of the address as the sets are only 2 bytes long
    // The mask will return the least significant bit, which represents the offset
    return 0x1 & address;
}
unsigned int getSet(unsigned int address) {
    // log2(8) = 3
    // The set will be the second, third, and fourth bit because there are 8 sets
    // The function extracts the lowest byte of the address and gets rid of the least significant bit ( >> 1) to just get the specified bits
    unsigned int value = (address & 0xF) >> 1;
    return value;
}
unsigned int getTag(unsigned int address) {
    // sizeof(address) - 1 - 3 
    // The tag is the rest of the bits in the address
    // The function extracts the rest of the address, excluding the least significant byte which held information for the set and offset
    // This extracted information is then shifted over 4 because the least significant byte is not a part of the tag in this cache system
    return (0xFFF0 & address) >> 4;
}
struct Cache* mallocCache(int numSets) {
	// Allocating memory here to store the cache pointer by designating memory for the size of a cache*
    struct Cache* cache = malloc(sizeof(struct Cache*));
    // Initializing the numSets variable in the Cache
    cache->numSets = numSets;
    // Allocating memory for the size of a Set multilied by the number of total sets so that sufficient memory is allocated
    cache->sets = malloc(sizeof(struct Set)*numSets);
    // Initializing all valid bits of each set to 0 so that they can be changed later on
    for (int i = 0; i<numSets; i++) {
      cache->sets[i].valid = 0;
    }
    return cache;
}
void freeCache(struct Cache *cache) {
    free(cache->sets);
    free(cache);
}
void printSet(struct Set *set, int setIndex) {
    printf("set: %x - tag: %x - valid: %u - data:", setIndex, set->tag, set->valid);
    unsigned char *data = set->data;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
    printf(" %.2x", data[i]);
    }
    printf("\n");
}
void printCache(struct Cache *cache) {
	// Iterating over the number of sets dictated by the variable in the cache pointer
    for (int i = 0; i < cache->numSets; i++) {
	    // Creating a set pointer at the specific address of the set
      struct Set *set = &cache->sets[i];
      // If the validity bit is 1, print the set using the already created PrintSet() function, else do nothing
      if (set->valid) {
        printSet(set, i);
      }
    }
}
void readValue(struct Cache *cache, unsigned int address) {
	// Intializing variables for the set, offset, and tag of the address
    unsigned int s = getSet(address);
    unsigned int o = getOffset(address);
    unsigned int t = getTag(address);
    // Initializing a pointer to the address given by the set
    struct Set *set = &cache->sets[s];
    // Initializing a character pointer to hold the data stored in the set
    unsigned char *data = set->data;
    // Printing specific syntax required
    printf("looking for set: %X - tag : %X\n", s, t);
    // If validity bit = 1, print the set has been found with specific syntax
    if (set->valid) {
	    // Printing the set was found along with the set number, the set's tag, and the validity bit
      printf("found set: set: %X - tag: %X - valid: %X - data:", s, set->tag, set->valid);
      // Printing the data in the set
      for (int i = 0; i < BLOCK_SIZE; ++i) {
   	 printf(" %.2x", data[i]);
      }
      printf("\n");
      // If the provided input address has the same tag as the set it maps to, pring a hit and the data inside
      if (set->tag == t) {
      	printf("hit %.2x\n", data[o]);
      }
      // If the tags are not equal then it is a conflict miss
      else {
	printf("conflict miss!\n");
      }
    }
    // If not valid, this would create a cold miss as it maps to something out of the cache's scope
    else {
      printf("no valid line found - cold miss!\n");
    }

}
void writeValue(struct Cache *cache, unsigned int address, unsigned char *newData) {
    unsigned int s = getSet(address);
    unsigned int t = getTag(address);
    struct Set *set = &cache->sets[s];
    if (set->valid && set->tag != t) {
    unsigned char *data = set->data;
    printf("evicting line - ");
    printSet(set, s);
    }
    unsigned char *data = set->data;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
    data[i] = newData[i];
    }
    set->tag = t;
    set->valid = 1;
    printf("wrote set: ");
    printSet(set, s);
}
unsigned int readUnsignedIntFromHex() {
    char buffer[10];
    char *p = NULL;
    unsigned int n;
    while (1) {
    fgets(buffer, sizeof(buffer), stdin);
    n = strtoul(buffer, &p, 16);
    if (buffer != p) {
    break;
    }
    printf("Invalid input - try again: ");
    }
    return n;
}
int main() {
    struct Cache *cache = mallocCache(8);
    char buffer[10];
    char c;
    do {
    printf("Enter 'r' for read, 'w' for write, 'p' to print, 'q' to quit: ");
    fgets(buffer, sizeof(buffer), stdin);
    c = buffer[0];
    if (c == 'r') {
    printf("Enter 32-bit unsigned hex address: ");
    unsigned int a = readUnsignedIntFromHex();
    readValue(cache, a);
    } else if (c == 'w') {
    printf("Enter 32-bit unsigned hex address: ");
    unsigned int a = readUnsignedIntFromHex();
    printf("Enter 32-bit unsigned hex value: ");
    unsigned int v = readUnsignedIntFromHex();
    unsigned char *data = (unsigned char *)&v;
    writeValue(cache, a, data);
    } else if (c == 'p') {
    printCache(cache);
    }
    } while (c != 'q');
    freeCache(cache);
}
