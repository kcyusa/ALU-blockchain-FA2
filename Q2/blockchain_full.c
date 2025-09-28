#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_BLOCKS 100

// --- Block Structure ---
typedef struct Block {
    int index;
    char timestamp[30];
    char data[256];          // Simplified transactions
    char previousHash[65];   // SHA-256 hash (64 chars + null terminator)
    char hash[65];           // Current block hash
    int nonce;
} Block;

// --- Blockchain ---
Block blockchain[MAX_BLOCKS];
int blockchainSize = 0;

// --- Helper: SHA-256 Function ---
void calculateHash(Block *block, char outputHash[65]) {
    char input[512];
    snprintf(input, sizeof(input), "%d%s%s%s%d",
             block->index, block->timestamp, block->data,
             block->previousHash, block->nonce);

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)input, strlen(input), hash);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(outputHash + (i * 2), "%02x", hash[i]);
    outputHash[64] = '\0';
}

// --- Proof of Work ---
void proofOfWork(Block *block, int difficulty) {
    char target[difficulty + 1];
    for (int i = 0; i < difficulty; i++)
        target[i] = '0';
    target[difficulty] = '\0';

    while (1) {
        calculateHash(block, block->hash);
        if (strncmp(block->hash, target, difficulty) == 0)
            break; // Hash meets difficulty
        block->nonce++;
    }
}

// --- Create a New Block ---
Block createBlock(const char *data, int difficulty) {
    Block newBlock;
    newBlock.index = blockchainSize;
    snprintf(newBlock.timestamp, sizeof(newBlock.timestamp), "%ld", time(NULL));
    strncpy(newBlock.data, data, sizeof(newBlock.data) - 1);

    if (blockchainSize == 0)
        strncpy(newBlock.previousHash, "0", sizeof(newBlock.previousHash) - 1);
    else
        strncpy(newBlock.previousHash, blockchain[blockchainSize - 1].hash,
                sizeof(newBlock.previousHash) - 1);

    newBlock.nonce = 0;

    // Mine the block (PoW)
    proofOfWork(&newBlock, difficulty);

    // Add to blockchain
    blockchain[blockchainSize++] = newBlock;

    return newBlock;
}

// --- Print Block Info ---
void printBlock(Block block) {
    printf("\n--- Block %d ---\n", block.index);
    printf("Timestamp     : %s\n", block.timestamp);
    printf("Data          : %s\n", block.data);
    printf("Previous Hash : %s\n", block.previousHash);
    printf("Hash          : %s\n", block.hash);
    printf("Nonce         : %d\n", block.nonce);
}

// --- Main Function ---
int main() {
    int difficulty;
    printf("Enter mining difficulty (number of leading zeros): ");
    scanf("%d", &difficulty);

    clock_t start, end;
    double cpu_time_used;

    // Task 1 + Task 3: Create Genesis Block
    printf("\nMining Genesis Block...\n");
    start = clock();
    Block genesis = createBlock("Genesis Block", difficulty);
    end = clock();
    printBlock(genesis);
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %.2f seconds\n", cpu_time_used);

    // Task 3: Mine additional blocks
    const char *transactions[] = {
        "Alice -> Bob 10 BTC",
        "Bob -> Charlie 5 BTC",
        "Charlie -> Dave 2 BTC"
    };

    for (int i = 0; i < 3; i++) {
        printf("\nMining Block %d...\n", i + 1);
        start = clock();
        Block newBlock = createBlock(transactions[i], difficulty);
        end = clock();
        printBlock(newBlock);
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Time taken: %.2f seconds\n", cpu_time_used);
    }

    return 0;
}
