#include <ledger.h>
#include <string>
using namespace std;

pthread_mutex_t ledger_lock;

list<struct Ledger> ledger;
Bank *bank;

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
	pthread_mutex_init(&ledger_lock, NULL);
	bank->print_account();
	load_ledger(filename);
	pthread_t workers[num_workers];
	int rc;
	for (int i = 0; i < num_workers; i++)
	{
		rc = pthread_create(&(workers[i]), NULL, worker, &i);
		assert(rc == 0);
	}
	for (int j = 0; j < num_workers; j++)
	{
		rc = pthread_join(workers[j], NULL);
		assert(rc == 0);
	}
	bank->print_account();
}

/**
 * @brief Parse a ledger file and store each line into a list
 *
 * @param filename
 */
void load_ledger(char *filename)
{

	ifstream infile(filename);
	int f, t, a, m, ledgerID = 0;
	while (infile >> f >> t >> a >> m)
	{
		struct Ledger l;
		l.from = f;
		l.to = t;
		l.amount = a;
		l.mode = m;
		l.ledgerID = ledgerID++;
		ledger.push_back(l);
	}
}

void *producer(void *workerID)
{
}

void *consumer(void *workerID)
{
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

	while (1)
	{
		pthread_mutex_lock(&ledger_lock);
		if (!ledger.empty())
		{
			Ledger action = ledger.front();
			ledger.pop_front();
			pthread_mutex_unlock(&ledger_lock);
			switch (action.mode)
			{
			case 0:
				bank->deposit(workID - 1, action.ledgerID, action.from, action.amount);
				break;
			case 1:
				bank->withdraw(workID - 1, action.ledgerID, action.from, action.amount);
				break;
			case 2:
				bank->transfer(workID - 1, action.ledgerID, action.from, action.to, action.amount);
				break;
			}
		}
		else
		{
			pthread_mutex_unlock(&ledger_lock);
			break;
		}
	}

	return NULL;
}
