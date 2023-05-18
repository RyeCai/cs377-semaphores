#include <bank.h>

using namespace std;

/**
 * @brief prints account information
 */
void Bank::print_account()
{
  for (int i = 0; i < num; i++)
  {
    sem_wait(&accounts[i].write_lock);
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance
         << endl;
    sem_post(&accounts[i].write_lock);
  }

  sem_wait(&bank_lock);
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl;
  sem_post(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Bank::recordFail(char *message)
{
  sem_wait(&bank_lock);
  cout << message << endl;
  num_fail++;
  sem_post(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
 */
void Bank::recordSucc(char *message)
{
  sem_wait(&bank_lock);
  cout << message << endl;
  num_succ++;
  sem_post(&bank_lock);
}

/**
 * @brief Construct a new Bank:: Bank object.
 *
 * Requirements:
 *  - The function should initialize the private variables.
 *  - Create a new array[N] of type Accounts.
 *  - Initialize each account (HINT: there are three fields to initialize)
 *
 * @param N
 */
Bank::Bank(int N)
{
  sem_init(&bank_lock, 0, 1);
  readers = 0;
  num = N;
  accounts = (Account *)calloc(sizeof(Account), N);

  for (int i = 0; i < N; i++)
  {
    struct Account new_account;
    sem_t acc_write_lock;
    sem_t acc_read_lock;
    sem_init(&acc_write_lock, 0, 1);
    sem_init(&acc_read_lock, 0, 1);
    new_account.accountID = i;
    new_account.balance = 0;
    new_account.write_lock = acc_write_lock;
    new_account.read_lock = acc_read_lock;
    accounts[i] = new_account;
  }
}

/**
 * @brief Destroy the Bank:: Bank object
 *
 * Requirements:
 *  - Make sure to destroy all locks.
 *  - Make sure to free all memory
 *
 */
Bank::~Bank()
{
  sem_destroy(&bank_lock);
  for (int i = 0; i < num; i++)
  {
    sem_destroy(&(accounts[i].write_lock));
    sem_destroy(&(accounts[i].read_lock));
  }
  free(accounts);
  accounts = NULL;
}

/**
 * @brief Logs the current amount of money in an account
 *
 * Requirements:
 *  - Make sure to log in the following format
 *    `Worker [worker_id] completed ledger [ledger_id]: deposit [amount] into account [account]`
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to deposit
 * @return int
 */
int Bank::check_balance(int workerID, int ledgerID, int readerID, int accountID)
{
  sem_wait(&accounts[accountID].read_lock);
  readers++;
  if (readers == 1)
    sem_wait(&accounts[accountID].write_lock);
  sem_post(&accounts[accountID].read_lock);
  // Check and print balance
  char buffer[4096];
  sprintf(buffer, "Worker %d completed ledger %d from reader #%d: account %d has %ld", workerID, ledgerID, readerID, accountID, accounts[accountID].balance);
  recordSucc(buffer);

  sem_wait(&accounts[accountID].read_lock);
  readers--;
  // Releases write lock if there are no readers left
  if (readers == 0)
    sem_post(&accounts[accountID].write_lock);
  sem_post(&accounts[accountID].read_lock);
  return 0;
}

/**
 * @brief Adds money to an account
 *
 * Requirements:
 *  - Make sure to log in the following format
 *    `Worker [worker_id] completed ledger [ledger_id]: deposit [amount] into account [account]`
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to deposit
 * @param amount the amount deposited
 * @return int
 */
int Bank::deposit(int workerID, int ledgerID, int readerID, int accountID, int amount)
{
  sem_wait(&accounts[accountID].write_lock);
  char buffer[4096];
  accounts[accountID].balance += amount;
  sprintf(buffer, "Worker %d completed ledger %d from reader #%d: deposit %d into account %d", workerID, ledgerID, readerID, amount, accountID);
  recordSucc(buffer);
  sem_post(&(accounts[accountID].write_lock));
  return 0;
}

/**
 * @brief Withdraws money from an account
 *
 * Requirements:
 *  - Make sure the account has a large enough balance.
 *    - Case 1: withdraw amount <= balance, log success
 *    - Case 2: log failure
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param accountID the account ID to withdraw
 * @param amount the amount withdrawn
 * @return int 0 on success -1 on failure
 */
int Bank::withdraw(int workerID, int ledgerID, int readerID, int accountID, int amount)
{
  sem_wait(&(accounts[accountID].write_lock));
  char buffer[4096];
  if (accounts[accountID].balance < amount)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d from reader #%d: withdraw %d from account %d", workerID, ledgerID, readerID, amount, accountID);
    recordFail(buffer);
    sem_post(&(accounts[accountID].write_lock));
    return -1;
  }
  accounts[accountID].balance -= amount;
  sprintf(buffer, "Worker %d completed ledger %d from reader #%d: withdraw %d from account %d", workerID, ledgerID, readerID, amount, accountID);
  recordSucc(buffer);
  sem_post(&(accounts[accountID].write_lock));
  return 0;
}

/**
 * @brief Transfer from one account to another
 *
 * Requirements:
 *  - Make sure there is enough money in the FROM account
 *  - Be careful with the locking order
 *
 *
 * @param workerID the ID of the worker (thread)
 * @param ledgerID the ID of the ledger entry
 * @param srcID the account to transfer money out
 * @param destID the account to receive the money
 * @param amount the amount to transfer
 * @return int 0 on success -1 on error
 */
int Bank::transfer(int workerID, int ledgerID, int readerID, int srcID, int destID,
                   unsigned int amount)
{
  sem_wait(&(accounts[srcID].write_lock));
  if (srcID != destID)
    sem_wait(&(accounts[destID].write_lock));
  char buffer[4096];
  if (srcID == destID)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d from reader #%d: transfer %d from account %d to account %d", workerID, ledgerID, readerID, amount, srcID, destID);
    recordFail(buffer);
    sem_post(&(accounts[srcID].write_lock));
    return -1;
  }
  else if (accounts[srcID].balance < amount)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d from reader #%d: transfer %d from account %d to account %d", workerID, ledgerID, readerID, amount, srcID, destID);
    recordFail(buffer);
    sem_post(&(accounts[destID].write_lock));
    sem_post(&(accounts[srcID].write_lock));
    return -1;
  }
  accounts[srcID].balance -= amount;
  accounts[destID].balance += amount;
  sprintf(buffer, "Worker %d completed ledger %d from reader #%d: transfer %d from account %d to account %d", workerID, ledgerID, readerID, amount, srcID, destID);
  recordSucc(buffer);
  sem_post(&(accounts[destID].write_lock));
  sem_post(&(accounts[srcID].write_lock));
  return 0;
}