
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_UTXOS 100
#define MAX_ADDR_LEN 64

/* --- Data structures --- */

/* UTXO structure */
typedef struct {
    int txid;                            /* unique tx id */
    char receiver[MAX_ADDR_LEN];         /* receiver address */
    int amount;                          /* amount in integer units (e.g., satoshis or cents) */
    bool spent;                          /* whether this UTXO has been spent */
} UTXO;

/* Transaction input: references an existing UTXO by txid */
typedef struct {
    int input_txid;
} TxInput;

/* Transaction output: new UTXO to be created */
typedef struct {
    char to[MAX_ADDR_LEN];
    int amount;
} TxOutput;

/* Transaction structure (for logging) */
typedef struct {
    int tx_number;
    char from[MAX_ADDR_LEN];
    char to[MAX_ADDR_LEN];
    int amount;
    int num_inputs;
    TxInput inputs[10];
    int num_outputs;
    TxOutput outputs[10];
} Transaction;

/* --- Global UTXO pool (simple array) --- */
UTXO utxo_pool[MAX_UTXOS];
int utxo_count = 0;

/* txid generator */
int next_txid = 1;
int next_tx_number = 1;

/* Utility: print all unspent UTXOs */
void print_unspent_utxos() {
    printf("\n=== Unspent UTXO Pool ===\n");
    printf("%-6s | %-12s | %-6s | %-6s\n", "TXID", "RECEIVER", "AMOUNT", "SPENT");
    printf("--------------------------------\n");
    for (int i = 0; i < utxo_count; ++i) {
        UTXO *u = &utxo_pool[i];
        printf("%-6d | %-12s | %-6d | %-6s\n", u->txid, u->receiver, u->amount, u->spent ? "YES" : "NO");
    }
    printf("===========================\n");
}

/* Add a UTXO to the pool */
int add_utxo(const char *receiver, int amount) {
    if (utxo_count >= MAX_UTXOS) {
        fprintf(stderr, "UTXO pool full\n");
        return -1;
    }
    int txid = next_txid++;
    UTXO u;
    u.txid = txid;
    strncpy(u.receiver, receiver, MAX_ADDR_LEN - 1);
    u.receiver[MAX_ADDR_LEN - 1] = '\0';
    u.amount = amount;
    u.spent = false;
    utxo_pool[utxo_count++] = u;
    return txid;
}

/* Find index of UTXO by txid; returns -1 if not found */
int find_utxo_index_by_txid(int txid) {
    for (int i = 0; i < utxo_count; ++i) {
        if (utxo_pool[i].txid == txid) return i;
    }
    return -1;
}

/* Find all unspent UTXOs for an address and store txids in out_txids (caller provides array).
 * Returns number found.
 */
int find_unspent_for_address(const char *address, int *out_txids, int max_out) {
    int found = 0;
    for (int i = 0; i < utxo_count && found < max_out; ++i) {
        if (!utxo_pool[i].spent && strcmp(utxo_pool[i].receiver, address) == 0) {
            out_txids[found++] = utxo_pool[i].txid;
        }
    }
    return found;
}

/* Simple UTXO selection: pick unspent UTXOs for sender, smallest-first, until amount covered.
 * Populates selected_txids array and returns number selected; returns -1 if insufficient funds.
 */
int select_utxos_for_amount(const char *sender, int amount, int *selected_txids, int max_selected) {
    /* First gather unspent UTXOs for sender */
    int candidates[50];
    int cand_count = find_unspent_for_address(sender, candidates, 50);
    if (cand_count == 0) return -1;

    /* Build an array of (amount, txid) for sorting */
    int idxs[50];
    for (int i = 0; i < cand_count; ++i) idxs[i] = candidates[i];

    /* Simple selection: sort candidates by amount ascending to reduce change fragmentation (bubble sort ok for small n) */
    for (int i = 0; i < cand_count - 1; ++i) {
        for (int j = 0; j < cand_count - 1 - i; ++j) {
            int a_idx = find_utxo_index_by_txid(idxs[j]);
            int b_idx = find_utxo_index_by_txid(idxs[j+1]);
            if (a_idx == -1 || b_idx == -1) continue;
            if (utxo_pool[a_idx].amount > utxo_pool[b_idx].amount) {
                int tmp = idxs[j]; idxs[j] = idxs[j+1]; idxs[j+1] = tmp;
            }
        }
    }

    int total = 0;
    int sel_count = 0;
    for (int i = 0; i < cand_count && sel_count < max_selected; ++i) {
        int utxo_index = find_utxo_index_by_txid(idxs[i]);
        if (utxo_index == -1) continue;
        selected_txids[sel_count++] = utxo_pool[utxo_index].txid;
        total += utxo_pool[utxo_index].amount;
        if (total >= amount) return sel_count;
    }

    return -1; /* insufficient funds */
}

/* Execute a transaction from 'from' to 'to' of 'amount' units.
 * Returns 0 on success, -1 on failure.
 * Prints details of transaction and updates UTXO pool.
 */
int create_transaction(const char *from, const char *to, int amount) {
    printf("\n--- Creating transaction #%d: %s -> %s : %d units ---\n", next_tx_number, from, to, amount);

    /* Select inputs */
    int selected[10];
    int num_selected = select_utxos_for_amount(from, amount, selected, 10);
    if (num_selected < 0) {
        printf("Insufficient funds for %s to send %d units. Transaction aborted.\n", from, amount);
        return -1;
    }

    /* Calculate total input amount */
    int total_in = 0;
    for (int i = 0; i < num_selected; ++i) {
        int idx = find_utxo_index_by_txid(selected[i]);
        total_in += utxo_pool[idx].amount;
    }

    /* Mark selected inputs as spent */
    for (int i = 0; i < num_selected; ++i) {
        int idx = find_utxo_index_by_txid(selected[i]);
        utxo_pool[idx].spent = true;
    }

    /* Create outputs: receiver gets 'amount', sender gets change (if any) */
    int change = total_in - amount;

    int out_txid_receiver = add_utxo(to, amount);
    if (out_txid_receiver < 0) {
        fprintf(stderr, "Failed to create receiver UTXO (pool full)\n");
        return -1;
    }

    int out_txid_change = -1;
    if (change > 0) {
        out_txid_change = add_utxo(from, change);
        if (out_txid_change < 0) {
            fprintf(stderr, "Failed to create change UTXO (pool full)\n");
            return -1;
        }
    }

    /* Create a transaction log (local struct) for printing */
    Transaction tx;
    tx.tx_number = next_tx_number++;
    strncpy(tx.from, from, MAX_ADDR_LEN-1);
    tx.from[MAX_ADDR_LEN-1] = '\0';
    strncpy(tx.to, to, MAX_ADDR_LEN-1);
    tx.to[MAX_ADDR_LEN-1] = '\0';
    tx.amount = amount;
    tx.num_inputs = num_selected;
    for (int i = 0; i < num_selected; ++i) tx.inputs[i].input_txid = selected[i];
    tx.num_outputs = 0;
    strncpy(tx.outputs[tx.num_outputs].to, to, MAX_ADDR_LEN-1);
    tx.outputs[tx.num_outputs].amount = amount;
    tx.num_outputs++;
    if (change > 0) {
        strncpy(tx.outputs[tx.num_outputs].to, from, MAX_ADDR_LEN-1);
        tx.outputs[tx.num_outputs].amount = change;
        tx.num_outputs++;
    }

    /* Print transaction summary */
    printf("Transaction #%d created:\n", tx.tx_number);
    printf(" Inputs: ");
    for (int i = 0; i < tx.num_inputs; ++i) {
        printf("%d ", tx.inputs[i].input_txid);
    }
    printf("\n Outputs:\n");
    for (int i = 0; i < tx.num_outputs; ++i) {
        printf("  -> %s : %d (new txid %d)\n", tx.outputs[i].to, tx.outputs[i].amount,
               (i == 0 ? out_txid_receiver : out_txid_change));
    }

    print_unspent_utxos();

    return 0;
}

/* Seed the UTXO pool with some initial funds for demonstration:
 * - Alice: 100
 * - Bob: 50
 * - Charlie: 75
 */
void seed_initial_utxos() {
    add_utxo("Alice", 50);   /* txid 1 */
    add_utxo("Alice", 50);   /* txid 2 => Alice total 100 */
    add_utxo("Bob", 50);     /* txid 3 */
    add_utxo("Charlie", 75); /* txid 4 */
}

/* Simple interactive demo sequence with a few transactions */
void demo_sequence() {
    printf("Starting demo sequence with initial UTXOs...\n");
    print_unspent_utxos();

    /* 1) Alice sends 30 to Bob */
    create_transaction("Alice", "Bob", 30);

    /* 2) Bob sends 60 to Charlie (requires aggregating inputs; Bob currently has initial 50 + received 30 = 80) */
    create_transaction("Bob", "Charlie", 60);

    /* 3) Charlie sends 40 to Alice */
    create_transaction("Charlie", "Alice", 40);

    /* 4) Alice attempts to send 150 to Bob (should fail - insufficient funds) */
    create_transaction("Alice", "Bob", 150);

    /* 5) Alice sends 20 to Bob */
    create_transaction("Alice", "Bob", 20);

    printf("\nDemo complete.\n");
}

/* Optional interactive mode allowing user to create custom transactions */
void interactive_mode() {
    char sender[MAX_ADDR_LEN];
    char receiver[MAX_ADDR_LEN];
    int amount;
    char choice[8];

    while (1) {
        print_unspent_utxos();
        printf("\nDo you want to create a transaction? (yes/no): ");
        if (scanf("%7s", choice) != 1) break;
        if (strcmp(choice, "no") == 0) break;
        if (strcmp(choice, "yes") != 0) continue;

        printf("Sender address: ");
        scanf("%63s", sender);
        printf("Receiver address: ");
        scanf("%63s", receiver);
        printf("Amount (integer units): ");
        if (scanf("%d", &amount) != 1) { printf("Invalid amount\n"); break; }

        create_transaction(sender, receiver, amount);
    }
}

int main() {
    printf("=== UTXO Simulation ===\n\n");
    seed_initial_utxos();

    /* Run demo sequence showing at least two users (Alice, Bob, Charlie) transacting */
    demo_sequence();

    /* Offer an interactive mode for the user */
    printf("\nYou can also try interactive mode to create your own transactions.\n");
    printf("Enter 'yes' to proceed to interactive mode, or 'no' to exit: ");
    char resp[8];
    if (scanf("%7s", resp) == 1 && strcmp(resp, "yes") == 0) {
        interactive_mode();
    } else {
        printf("Exiting.\n");
    }

    return 0;
}
