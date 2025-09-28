#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 10
#define NAME_LEN 32

/* --- Data structure for account --- */
typedef struct {
    char name[NAME_LEN];
    int balance;  /* integer units (no decimals for simplicity) */
} Account;

/* Global array of accounts */
Account accounts[MAX_ACCOUNTS];
int account_count = 0;

/* Utility: add a new account */
void addAccount(const char *name, int balance) {
    if (account_count >= MAX_ACCOUNTS) {
        printf("Cannot add more accounts.\n");
        return;
    }
    strncpy(accounts[account_count].name, name, NAME_LEN - 1);
    accounts[account_count].name[NAME_LEN - 1] = '\0';
    accounts[account_count].balance = balance;
    account_count++;
}

/* Utility: print all accounts and balances */
void printAccounts() {
    printf("\n=== Account Balances ===\n");
    printf("%-10s | %-6s\n", "Name", "Balance");
    printf("------------------------\n");
    for (int i = 0; i < account_count; i++) {
        printf("%-10s | %6d\n", accounts[i].name, accounts[i].balance);
    }
    printf("========================\n");
}

/* Find account index by name */
int findAccountIndex(const char *name) {
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].name, name) == 0) return i;
    }
    return -1; /* not found */
}

/* Transfer function:
 * - Checks sender and receiver exist.
 * - Checks sufficient balance.
 * - Updates balances.
 */
bool transferFunds(const char *sender, const char *receiver, int amount) {
    int senderIdx = findAccountIndex(sender);
    int receiverIdx = findAccountIndex(receiver);

    if (senderIdx == -1 || receiverIdx == -1) {
        printf("Error: Sender or receiver not found.\n");
        return false;
    }
    if (accounts[senderIdx].balance < amount) {
        printf("Error: Insufficient balance for %s.\n", sender);
        return false;
    }

    /* Deduct from sender and add to receiver */
    accounts[senderIdx].balance -= amount;
    accounts[receiverIdx].balance += amount;

    printf("Transaction successful: %s sent %d to %s.\n", sender, amount, receiver);
    return true;
}

/* Demo sequence */
void demoTransactions() {
    printf("\n--- Starting Demo Transactions ---\n");
    printAccounts();

    /* 1) Alice sends 40 to Bob */
    transferFunds("Alice", "Bob", 40);
    printAccounts();

    /* 2) Bob sends 30 to Charlie */
    transferFunds("Bob", "Charlie", 30);
    printAccounts();

    /* 3) Charlie attempts to send 200 to Alice (should fail) */
    transferFunds("Charlie", "Alice", 200);
    printAccounts();

    /* 4) Alice sends 50 to Charlie */
    transferFunds("Alice", "Charlie", 50);
    printAccounts();

    printf("\n--- Demo Complete ---\n");
}

int main() {
    printf("=== Account/Balance Model Simulation ===\n");

    /* Initialize accounts */
    addAccount("Alice", 100);
    addAccount("Bob", 50);
    addAccount("Charlie", 75);

    /* Run demo transactions */
    demoTransactions();

    return 0;
}
