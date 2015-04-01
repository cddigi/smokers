/*
	Cornelius Donley
	Andrew Bockus

	March 5, 2015
*/


/* Shell Author: S. Renk */
/* CSC 420/520 */
/* Parallel Cigarette-Smoker Problem */

// include headers
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

void agent();
void smoker(int);

// shared vars for the smokers
enum iState { EMPTY, FULL };
enum sState { GONE, PRESENT, CRAVING, SMOKING, DONE };
int item[3] = { EMPTY }; /* holds iState */
int smokerCount = 0;
int smokerState[3] = { GONE };
int numTimesSmoked[3] = { 0, 0, 0 };
const int NUM_TO_SMOKE = 100; // Number of times to smoke

// locks to create mutual exclusion
HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
// any others you need

// ********** Create 3 smokers and an agent & set them smoking ****************
void main()
{ // Set up 4 threads
	HANDLE sThread[4];                  // 4 smokers
	DWORD  sThreadID[4];                // PID of thread
	DWORD WINAPI proc(LPVOID);          // code for the 4 smokers

	// seed rand # generator
	srand ( time(NULL) );   // uncomment this when you have it running
	// start 4 threads
	for (int smokerNbr = 0; smokerNbr < 4; smokerNbr++)
	{
		sThread[smokerNbr] = CreateThread(NULL, 0, proc, NULL, 0, &sThreadID[smokerNbr]);
	}

	WaitForMultipleObjects(4, sThread, true, INFINITE); // wait for threads to finish
	cout << "press CR to end."; while (_getch() != '\r');
	return;
}



DWORD WINAPI proc(LPVOID)    // forman/worker solution rather than peer2peer
{
	int myID;

	WaitForSingleObject(mutex, INFINITE);   // lock the lock
	myID = smokerCount++;
	ReleaseMutex(mutex);                    // unlock the lock
	smokerState[myID] = PRESENT;

	if (myID == 3)
	{
		WaitForSingleObject(mutex, INFINITE);
		cout << "Calling Agent \n";
		ReleaseMutex(mutex);
		agent();
	}
	else
	{
		WaitForSingleObject(mutex, INFINITE);
		cout << "Calling Smoker " << myID << endl;
		ReleaseMutex(mutex);
		smokerState[myID] = PRESENT;
		smoker(myID);
	}
	return 0;
}


void agent()
{
	/* wait for all three smokers to report */
	while (smokerCount < 3) Sleep(0);

	/* The agent continues to deal items as long as all smokers have not reached there smoke limit and left the table */
	while (numTimesSmoked[0] < NUM_TO_SMOKE || numTimesSmoked[1] < NUM_TO_SMOKE || numTimesSmoked[2] < NUM_TO_SMOKE)
	{
		/* The agent chooses a random number between 0 and 2 that will match up with a smoker */
		int smoker = rand() % 3;

		/* If the smoker has already left the table, another is chosen*/
		while (numTimesSmoked[smoker] >= NUM_TO_SMOKE)
			smoker = rand() % 3;

		/* The items the smoker needs are placed on the table */
		WaitForSingleObject(mutex, INFINITE);
		item[(smoker + 1) % 3]++;
		item[(smoker + 2) % 3]++;
		//cout << "Paper: " << item[0] << "\tMatches: " << item[1] << "\tTobacco: " << item[2] << endl << endl;
		ReleaseMutex(mutex);

		// Wait for the smoker to grab the items
		while (item[0] == FULL || item[1] == FULL || item[2] == FULL) Sleep(0);
	}
}

void smoker(int myID)
{	
	/* While the smoker has not had his fill, he stays at the table*/
	while (numTimesSmoked[myID] < NUM_TO_SMOKE)
	{
		smokerState[myID] = CRAVING;

		/* If the smokers items are on the table, he grabs them and enters CS */
		if (item[(myID + 1) % 3] == FULL && item[(myID + 2) % 3] == FULL)
		{
			/* The smoker takes his items */
			WaitForSingleObject(mutex, INFINITE);
			item[(myID + 1) % 3]--;
			item[(myID + 2) % 3]--;
			numTimesSmoked[myID]++;
			ReleaseMutex(mutex);			

			/* The smoker now smokes */
			smokerState[myID] = SMOKING;
			/*WaitForSingleObject(mutex, INFINITE);
			cout << "Smoker " << myID << " smoking " << numTimesSmoked[myID] << endl << endl;
			ReleaseMutex(mutex);*/

			Sleep(rand() % 10);
		}
		else Sleep(0);
	}

	/* The smoker has had his fill and waits for the others to finish */
	smokerState[myID] = DONE;
	WaitForSingleObject(mutex, INFINITE);
	cout << "Smoker " << myID << " done." << endl;
	ReleaseMutex(mutex);
}

