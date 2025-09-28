UTXO Simulation - README
========================
How this code works
-------------------
- The program keeps a simple in-memory UTXO pool (array of UTXO structs).
- Each UTXO has:
    - txid (unique integer)
    - receiver (address string)
    - amount (integer)
    - spent flag (boolean)
- Transactions:
    - The sender's unspent UTXOs are selected (smallest-first) until the requested amount is covered.
    - Selected input UTXOs are marked as spent.
    - A new UTXO is created for the receiver with the requested amount.
    - If the inputs exceed the amount, a change UTXO is created for the sender.
- Demo:
    - The program seeds the pool with initial UTXOs for Alice, Bob, and Charlie.
    - It runs a pre-defined demo sequence of 5 transactions, printing the UTXO pool after each transaction.
    - You can then optionally run an interactive mode to create your own transactions.

Compile and run
---------------
1) Compile:
   gcc -o utxo_simulation utxo_simulation.c

2) Run:
   ./utxo_simulation

Sample output (abbreviated)
--------------------------
- The program prints the initial unspent UTXO pool, then after each transaction prints details:
  e.g.
  Alice -> Bob : 30
   Inputs: 1  (or selected txids)
   Outputs:
    -> Bob : 30 (new txid 5)
    -> Alice : 20 (change txid 6)

 - After every transaction the "Unspent UTXO Pool" is printed showing txid, receiver, amount, spent.


Account Model Simulation - README
=================================

Code Summary
------------
1. Data Structures:
   - `Account`: struct with `name` (string) and `balance` (integer).
   - Accounts are stored in a global array.

2. Functions:
   - `addAccount(name, balance)`:
       Adds a new account with the given starting balance.
   - `printAccounts()`:
       Displays all accounts and balances.
   - `findAccountIndex(name)`:
       Finds the index of an account by its name.
   - `transferFunds(sender, receiver, amount)`:
       Validates the transaction:
         - Sender and receiver must exist.
         - Sender must have sufficient balance.
       Updates balances if valid.
   - `demoTransactions()`:
       Runs a sequence of sample transfers (successful and failed).

3. Main Flow:
   - Initializes 3 accounts: Alice, Bob, Charlie.
   - Runs the demo transaction sequence.
   - Prints account balances before and after each transfer.

Expected Behavior
-----------------
1) Start with:
   - Alice: 100
   - Bob: 50
   - Charlie: 75

2) Alice sends 40 to Bob:
   - Alice: 60
   - Bob: 90
   - Charlie: 75

3) Bob sends 30 to Charlie:
   - Alice: 60
   - Bob: 60
   - Charlie: 105

4) Charlie tries to send 200 to Alice (FAIL: insufficient funds).

5) Alice sends 50 to Charlie:
   - Alice: 10
   - Bob: 60
   - Charlie: 155

Compile & Run
-------------
1) Compile:
   gcc -o account_model_simulation account_model_simulation.c

2) Run:
   ./account_model_simulation

