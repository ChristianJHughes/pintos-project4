#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>

#define WIKI_ARRAY_SIZE 50 /* The number of wiki entries. Should be 1000000 for production. */
#define WIKI_STRING_SIZE 2003 /* The number of characters in each wiki entry. Must account for newline and terminating characters. */
#define WORDS_ARRAY_SIZE 75 /* The number of words being searched for in each wiki entry. Should be 50000 for production. */
#define WORDS_STRING_SIZE 11 /* The number of characters taken up by a word. Must account for newline and terminating characters. */

/* All of the wiki entries. */
char wiki_array[WIKI_ARRAY_SIZE][WIKI_STRING_SIZE];

/* All of the words being searched for. */
char words_array[WORDS_ARRAY_SIZE][WORDS_STRING_SIZE];

/* Results of the word search*/
char results_array[WORDS_ARRAY_SIZE][WIKI_ARRAY_SIZE];
char local_results_array[WORDS_ARRAY_SIZE][WIKI_ARRAY_SIZE];

int num_threads;

/* Initialize the results array to all zero */
void init_array()
{
  int i, j;

  for(i = 0; i < WORDS_ARRAY_SIZE; i++)
  {
    for(j= 0; j < WIKI_ARRAY_SIZE; j++)
    {
      results_array[i][j] = 'f';
      local_results_array[i][j] = 'f';
    }
  }
}

/* Read all of the wiki entries and words into local data structures from thier resprctive files. */
int read_to_memory()
{
  /* Read the wiki article into memory line by line. */
  FILE *file = fopen("/homes/cjhughes255/project4/wiki.50short", "r");

  if(file == NULL)
  {
    printf("fail");
    return -1;
  }

  /* Read each wiki line into memory. */
  int line_num = 0;
  char line[WIKI_STRING_SIZE];
  while(fgets(line, WIKI_STRING_SIZE, file) != NULL)
  {
    strcpy(wiki_array[line_num], line);
    line_num++;
  }
  fclose(file);

  /* Read the words list to memory line by line. */
  file = fopen("/homes/cjhughes255/project4/words_4-8chars75", "r");
  
  if(file == NULL)
  {
    printf("fail2");
    return -1;
  }

  /* Read each word line into memory. */
  line_num = 0;
  char line2[WORDS_STRING_SIZE];
  while (fgets(line2, WORDS_STRING_SIZE, file) != NULL)
  {
    line2[strcspn(line2, "\n")] = 0;
    strcpy(words_array[line_num], line2);
    line_num++;
  }
  fclose(file);
  return 0;
}

/* If a given word is present in 1 or more wiki articles, make note of that in results_array */
void *find_word_in_wiki(void *rank)
{
  int i, j;
  int myID = *((int *) rank);

  int startPos = ((long) myID) * (WIKI_ARRAY_SIZE / num_threads);
  int endPos = startPos + (WIKI_ARRAY_SIZE / num_threads);

  
  if(myID == num_threads-1)
  {
    endPos = WIKI_ARRAY_SIZE;
  }

  for(i = 0; i < WORDS_ARRAY_SIZE; i++)
  {
    for(j = startPos; j < endPos; j++)
    {
      char *p = strstr(wiki_array[j], words_array[i]);
      if(p)
      {
        local_results_array[i][j] = 't';
      }
    }
  }
}

/* Print out found words and their associated articles */
void print_results()
{
  int i, j, found_word;

  for(i = 0; i < WORDS_ARRAY_SIZE; i++)
  {
    found_word = 0;
    for(j= 0; j < WIKI_ARRAY_SIZE; j++)
    {
      if(results_array[i][j] == 't')
      {
        // If this is the first time that the word has been found...
        if (found_word == 0)
	{
	  // Set found_word to true. Print out the word alongside its line number.
	  found_word = 1;
	  printf("%s: %d", words_array[i], j + 1);
	}
        // Else, the word has been found before. Append it to the existing string.
        else
	{
	  printf(", %d", j + 1);
	}
      }
    }
    if(found_word == 1)
    {
      printf("\n");
    }
  }
}

int main(int argc, char* argv[]) {
  /* For measuring performance. */
  struct timeval t1, t2, t3, t4, t5;
  double elapsedTime;
  int numSlots, myVersion = 4; //base = 1, pthreads = 2, openmp = 3, mpi = 4

  int i, rc;

  int num_tasks, rank;
  MPI_Status status;

  /* Initialize and set thread detached attribute */
  rc = MPI_Init(&argc, &argv);

  if(rc != MPI_SUCCESS)
  {
    printf("error starting MPI program.");
    MPI_Abort(MPI_COMM_WORLD, rc);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  num_threads = num_tasks;
  
  gettimeofday(&t1, NULL);
  if(rank == 0)
  {
    init_array();
<<<<<<< HEAD
  }
  gettimeofday(&t2, NULL);
  
  if(read_to_memory() == 0)
  {
=======
    gettimeofday(&t2, NULL);
    read_to_memory();
  }

>>>>>>> 75b7374bf3009e45a76c1b5ab6a6927162442560
    gettimeofday(&t3, NULL);

    MPI_Bcast(wiki_array, WIKI_ARRAY_SIZE*WIKI_STRING_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    find_word_in_wiki(&rank);
    gettimeofday(&t4, NULL);

    MPI_Reduce(local_results_array, results_array, WIKI_ARRAY_SIZE*WORDS_ARRAY_SIZE, MPI_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank == 0)
    {
      print_results();

      gettimeofday(&t5, NULL);
      
      elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
      elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
      printf("Time to Init Array: %f\n", elapsedTime);
      
      elapsedTime = (t3.tv_sec - t2.tv_sec) * 1000.0; //sec to ms
      elapsedTime += (t3.tv_usec - t2.tv_usec) / 1000.0; // us to ms
      printf("Time to read: %f\n", elapsedTime);
      
      elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0; //sec to ms
      elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0; // us to ms
      printf("Time to search: %f\n", elapsedTime);
      
      elapsedTime = (t5.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
      elapsedTime += (t5.tv_usec - t1.tv_usec) / 1000.0; // us to ms
      printf("DATA, %d, %s, %f, %d\n", myVersion, getenv("NSLOTS"), elapsedTime, num_threads);
      
      printf("Main: program completed. Exiting.\n");
    }
<<<<<<< HEAD
  }
  else
  {
    MPI_Finalize();
    return -1;
  }
  MPI_Finalize();
  return 0;
=======
    MPI_Finalize();
  gettimeofday(&t5, NULL);
 
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
  printf("Time to Init Array: %f\n", elapsedTime);

  elapsedTime = (t3.tv_sec - t2.tv_sec) * 1000.0; //sec to ms
  elapsedTime += (t3.tv_usec - t2.tv_usec) / 1000.0; // us to ms
  printf("Time to read: %f\n", elapsedTime);

  elapsedTime = (t4.tv_sec - t3.tv_sec) * 1000.0; //sec to ms
  elapsedTime += (t4.tv_usec - t3.tv_usec) / 1000.0; // us to ms
  printf("Time to search: %f\n", elapsedTime);

  elapsedTime = (t5.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
  elapsedTime += (t5.tv_usec - t1.tv_usec) / 1000.0; // us to ms
  printf("DATA, %d, %s, %f, %d\n", myVersion, getenv("NSLOTS"),  elapsedTime, num_threads);
  
  printf("Main: program completed. Exiting.\n");
>>>>>>> 75b7374bf3009e45a76c1b5ab6a6927162442560
}
