// twelvedays.c
// Simon Chu
// Sat 2019-10-26 17:03:11 EDT
//
// CS 326 Operating Systems Project 6 - Part II
// Unix Signals Project - The Twelve Days of Christmas
//
// Purpose:
// To print the verses from The Twelve Days of Christmas using concurrent
// processes and signals.
//
// Overall Flow:
// We will first use fork to spawn 12 concurrent executing processes (children).
// The program will then send a signal to the twelfth child, then waits for all
// twelve children to terminate. Each child is going to print out one day of
// The Twelve Days of Christmas.

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// set the number of child processes to be forked
#define DAYS_OF_CHRISTMAS 12

// global variable that stores the number of the current running child.
int globalChildNumber;

// global variable that stores the child PIDs. The index of the array
// corresponds to the child number.
int globalChildPID[DAYS_OF_CHRISTMAS + 1];

const char *times[DAYS_OF_CHRISTMAS] = {
    "twelfth",
    "eleventh",
    "tenth",
    "ninth",
    "eighth",
    "seventh",
    "sixth",
    "fifth",
    "fourth",
    "third",
    "second",
    "first"};

const char *gifts[DAYS_OF_CHRISTMAS] = {
    "Twelve Drummers Drumming",
    "Eleven Pipers Piping",
    "Ten Lords a Leaping",
    "Nine Ladies Dancing",
    "Eight Maids a Milking",
    "Seven Swants a Swimming",
    "Six Geese a Laying",
    "Five Golden Rings",
    "Four Calling Birds",
    "Three French Hens",
    "Two Turtle Doves",
    "A Partridge in a Pear Tree"};

// declare function prototypes
void printInfo();
void signal_handler(int signo);
void child();
int check(int status);

// declare signal masks as global variables
static sigset_t newmask, oldmask, zeromask;

int main(void)
{

  int pid, status;

  // print program information
  printInfo();

  if (signal(SIGUSR1, signal_handler) == SIG_ERR)
  {
    printf("Error Creating Signal Handler\n");
    fflush(stdout);
  }

  sigemptyset(&zeromask);       // intialize empty signal set
  sigemptyset(&newmask);        // initialize empty signal set
  sigaddset(&newmask, SIGUSR1); // add signal to signal set

  // block the SIGUSR1 signal
  check(sigprocmask(SIG_BLOCK, &newmask, &oldmask));

  // fork two child processes from parent process.
  for (int i = 1; i <= DAYS_OF_CHRISTMAS; i++)
  {
    // store the number of the child that is just forked to the global variable
    globalChildNumber = i;

    pid = fork();
    check(pid);

    // execution child process
    if (pid == 0)
    {
      child();
      exit(0);
    }

    // execute parent process
    else
    {
      // store pid of the child forked to the global array
      globalChildPID[i] = pid;

      // parent signal the last child
      if (i == DAYS_OF_CHRISTMAS)
      {
        // prompt user that the parent is signalling the child
        printf("[parent]    Begin Printing The Twelve Days of Christmas\n");
        fflush(stdout);

        // send signal to the second child
        kill(pid, SIGUSR1);
      }
    }
  }

  // parent wait for children to terminate
  for (int i = 1; i <= DAYS_OF_CHRISTMAS; i++)
  {
    check(wait(&status));
  }
}

// prompt information at the beginning of the program.
void printInfo()
{
  printf("\nCreated by Simon Chu\n");
  printf("CS 326 Unix Signals Project - Part II: %s\n",
         "The Twelve Days of Christmas");
  printf("Program to Print \"Twelve Days of Christmas\" Using Concurrent\n");
  printf("Processes and Unix Signal System Call\n\n");
  fflush(stdout);
}

// helper function to check the returned status code.
// exit the program is the status code returned is negative
int check(int status)
{
  if (status < 0)
  {
    printf("Exiting the Program Due to Fatal Error!\n");
    fflush(stdout);
    exit(-1);
  }
  return status;
}

// function that is invoked when a new child process is forked.
// it waits for the signal, and send a signal to the previous child process.
void child()
{
  int index; // stores the index of the verse

  // wait for the signal
  // it will suspend the process until a signal is caught
  sigsuspend(&zeromask);

  // calculate the index of the current verse
  index = globalChildNumber - 1;

  // print the verse
  printf("On the %s day of Christmas\nmy true love sent to me:\n%s\n",
         times[index], gifts[index]);
  fflush(stdout);
  for (int i = globalChildNumber; i < DAYS_OF_CHRISTMAS; i++)
  {
    if (i == DAYS_OF_CHRISTMAS - 1)
    {
      // prepend and for the last verse
      printf("and %s\n", gifts[i]);
      fflush(stdout);
    }
    else
    {
      printf("%s\n", gifts[i]);
      fflush(stdout);
    }
  }

  // if the current child is not the beginning child, signal the previous child
  // otherwise, if the current child is the beginning child, terminate.
  if (globalChildNumber != 1)
  {
    // send signal to the previous child if current child is not the beginning
    // child.
    kill(globalChildPID[globalChildNumber - 1], SIGUSR1);
  }
  else
  {
    // print blank line and terminate if current child is the beginning child.
    printf("\n");
    fflush(stdout);
  }
}

// function to handle signal interrupts
void signal_handler(int signo)
{
  printf("\n\n[child %d is printing]:\n\n", globalChildNumber);
  fflush(stdout);
}
