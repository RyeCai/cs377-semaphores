#include <bank.h>

using namespace std;

/**
 * @brief prints account information
 */
void Bank::print_account()
{
  for (int i = 0; i < num; i++)
  {
    pthread_mutex_lock(&accounts[i].lock);
    cout << "ID# " << accounts[i].accountID << " | " << accounts[i].balance
         << endl;
    pthread_mutex_unlock(&accounts[i].lock);
  }

  pthread_mutex_lock(&bank_lock);
  cout << "Success: " << num_succ << " Fails: " << num_fail << endl;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_fail` and log
 *        message.
 *
 * @param message
 */
void Bank::recordFail(char *message)
{
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_fail++;
  pthread_mutex_unlock(&bank_lock);
}

/**
 * @brief helper function to increment the bank variable `num_succ` and log
 *        message.
 *
 * @param message
 */
void Bank::recordSucc(char *message)
{
  pthread_mutex_lock(&bank_lock);
  cout << message << endl;
  num_succ++;
  pthread_mutex_unlock(&bank_lock);
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
  pthread_mutex_init(&bank_lock, NULL);
  num = N;
  accounts = (Account *)calloc(sizeof(Account), N);

  for (int i = 0; i < N; i++)
  {
    struct Account new_account;
    pthread_mutex_t acc_lock;
    pthread_mutex_init(&acc_lock, NULL);
    new_account.accountID = i;
    new_account.balance = 0;
    new_account.lock = acc_lock;
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
  pthread_mutex_destroy(&bank_lock);
  for (int i = 0; i < num; i++)
  {
    pthread_mutex_destroy(&(accounts[i].lock));
  }
  free(accounts);
  accounts = NULL;
  // free(&num);
  // free(&num_succ);
  // free(&num_fail);
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
int Bank::deposit(int workerID, int ledgerID, int accountID, int amount)
{
  pthread_mutex_lock(&accounts[accountID].lock);
  char buffer[4096];
  accounts[accountID].balance += amount;
  sprintf(buffer, "Worker %d completed ledger %d: deposit %d into account %d", workerID, ledgerID, amount, accountID);
  recordSucc(buffer);
  pthread_mutex_unlock(&(accounts[accountID].lock));
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
int Bank::withdraw(int workerID, int ledgerID, int accountID, int amount)
{
  pthread_mutex_lock(&(accounts[accountID].lock));
  char buffer[4096];
  if (accounts[accountID].balance < amount)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d: withdraw %d from account %d", workerID, ledgerID, amount, accountID);
    recordFail(buffer);
    pthread_mutex_unlock(&(accounts[accountID].lock));
    return -1;
  }
  accounts[accountID].balance -= amount;
  sprintf(buffer, "Worker %d completed ledger %d: withdraw %d from account %d", workerID, ledgerID, amount, accountID);
  recordSucc(buffer);
  pthread_mutex_unlock(&(accounts[accountID].lock));
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
int Bank::transfer(int workerID, int ledgerID, int srcID, int destID,
                   unsigned int amount)
{
  pthread_mutex_lock(&(accounts[srcID].lock));
  if (srcID != destID)
    pthread_mutex_lock(&(accounts[destID].lock));
  char buffer[4096];
  if (srcID == destID)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d: transfer %d from account %d to account %d", workerID, ledgerID, amount, srcID, destID);
    recordFail(buffer);
    pthread_mutex_unlock(&(accounts[srcID].lock));
    return -1;
  }
  else if (accounts[srcID].balance < amount)
  {
    sprintf(buffer, "Worker %d failed to complete ledger %d: transfer %d from account %d to account %d", workerID, ledgerID, amount, srcID, destID);
    recordFail(buffer);
    pthread_mutex_unlock(&(accounts[destID].lock));
    pthread_mutex_unlock(&(accounts[srcID].lock));
    return -1;
  }
  accounts[srcID].balance -= amount;
  // pthread_mutex_lock(&(accounts[destID].lock));
  accounts[destID].balance += amount;
  sprintf(buffer, "Worker %d completed ledger %d: transfer %d from account %d to account %d", workerID, ledgerID, amount, srcID, destID);
  recordSucc(buffer);
  pthread_mutex_unlock(&(accounts[destID].lock));
  pthread_mutex_unlock(&(accounts[srcID].lock));
  return 0;
}