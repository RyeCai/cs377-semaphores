#include <ledger.h>
#include <string>
#include <semaphore.h>
using namespace std;

sem_t ledger_lock;
ifstream infile;
int ledger_id;
Bank *bank;

// Producer/Consumer Variables
struct Ledger buffer[MAX];
int use;
int fill_amount;
sem_t full;
sem_t empty;
// Number of times a consumer checks the buffer
int loops;

/**
 * @brief creates a new bank object and sets up workers
 *
 * Requirements:
 *  - Create a new Bank object class with 10 accounts.
 *  - Load the ledger into a list
 *  - Set up the worker threads.
 *
 * @param num_workers
 * @param filename
 */
void InitBank(int num_workers, char *filename)
{
	bank = new Bank(10);
	bank->print_account();
	sem_init(&ledger_lock, 0, 1);
	pthread_t readers[num_workers];
	pthread_t workers[num_workers];
	infile.open(filename);
	ledger_id = 0;
	use = 0;
	fill_amount = 0;
	sem_init(&empty, 0, MAX);
	sem_init(&full, 0, 0);
	loops = MAX * 100;
	int rc;
	for (int i = 0; i < num_workers; i++)
	{
		rc = pthread_create(&(readers[i]), NULL, reader, &i);
		assert(rc == 0);
		rc = pthread_create(&(workers[i]), NULL, worker, &i);
		assert(rc == 0);
	}
	for (int j = 0; j < num_workers; j++)
	{
		rc = pthread_join(readers[j], NULL);
		assert(rc == 0);
		rc = pthread_join(workers[j], NULL);
		assert(rc == 0);
	}
	bank->print_account();
}

void put(Ledger l)
{
	buffer[fill_amount] = l;
	fill_amount = (fill_amount + 1) % MAX;
}

Ledger get()
{
	Ledger tmp = buffer[use];
	use = (use + 1) % MAX;
	return tmp;
}

/**
 * @brief Parses a ledger file and adds them to the buffer for the workers.
 *
 * @param workerID
 * @return void*
 */
void *reader(void *workerID)
{
	int workID = ((int *)(workerID))[0];
	int f, t, a, m;
	for (int i = 0; i < loops; i++)
	{
		sem_wait(&empty);
		sem_wait(&ledger_lock);
		if (infile >> f >> t >> a >> m)
		{
			struct Ledger l;
			l.from = f;
			l.to = t;
			l.amount = a;
			l.mode = m;
			l.ledgerID = ledger_id++;
			l.reader = workID;
			// cout << "Reader " << workID << " produced ledger " << l.ledgerID << endl;
			put(l);
		}
		sem_post(&ledger_lock);
		sem_post(&full);
	}
	return NULL;
}

/**
 * @brief Remove items from the list and execute the instruction.
 *
 * @param workerID
 * @return void*
 */
void *worker(void *workerID)
{
	int workID = ((int *)(workerID))[0];

	for (int i = 0; i < loops; i++)
	{
		sem_wait(&full);
		sem_wait(&ledger_lock);
		Ledger action = buffer[use];

		if (action.ledgerID != -1)
		{
			// prevent worker from redoing tasks in the buffer
			buffer[use].ledgerID = -1;
			use = (use + 1) % MAX;
			sem_post(&ledger_lock);
			sem_post(&empty);
			switch (action.mode)
			{
			case 0:
				bank->deposit(workID - 1, action.ledgerID, action.reader, action.from, action.amount);
				break;
			case 1:
				bank->withdraw(workID - 1, action.ledgerID, action.reader, action.from, action.amount);
				break;
			case 2:
				bank->transfer(workID - 1, action.ledgerID, action.reader, action.from, action.to, action.amount);
				break;
			case 3:
				bank->check_balance(workID - 1, action.ledgerID, action.reader, action.from);
				break;
			}
		}
		else
		{
			sem_post(&ledger_lock);
			sem_post(&empty);
		}
	}

	return NULL;
}
