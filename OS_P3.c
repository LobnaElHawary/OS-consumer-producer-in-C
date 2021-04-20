#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include<unistd.h>
#include "buffer.h"

#define TRUE 1

/* the buffer */
buffer_item buffer[BUFFER_SIZE]; //buffer size = 5

pthread_mutex_t mutex; //controls buffer access

//counting semaphores that synchronize the consumer and producer
sem_t empty; //counts number of empty slots in buffer, prevents underflow
sem_t full; //counts number of full slots in buffer, prevents overflow

int insert_index = 0;
int remove_index = 0;
int count = 0;

//functions
void initialize();
void *producer(void *param);
void *consumer(void *param);
int insert_item(buffer_item item);
int remove_item(buffer_item *item);


int main(int argc, char *argv[])

{

    // if argument count is not 4 (1.name of executable 2.sleep_time 3.producer_num 4.consumer_num), prompt an error message
    if (argc != 4)
    {
        
        printf("Error in the number of arguments required \n");
        exit(1); //unsuccessful code termination
    }
    
    //convert input from string to long int
    const long int sleep_time = abs(strtol(argv[1], NULL, 0));  //How long to sleep before terminating
    const long int producer_num =abs(strtol(argv[2], NULL, 0)); //The number of producer threads
    const long int consumer_num = abs(strtol(argv[3], NULL, 0)); //The number of consumer threads

    // Initialize buffer related synchronization tools /

    int i = 0;

    srand(time(NULL)); //seed random

    initialize();
    
    pthread_t prod[producer_num]; // Create producer thread identifier
    pthread_t cons[consumer_num]; // Create consumer threads identifier

    // Sleep for user specified time based on the command line input /

    for(i = 0; i < producer_num; i++)
    {
        pthread_create(&prod[i], NULL, producer, &i); //1. thread identifier 2. default attributes 3. names of fn where new thread executes
    }

    for(i = 0; i < consumer_num; i++)
    {
        pthread_create(&cons[i], NULL, consumer, &i); //1. thread identifier 2. default attributes 3. names of fn where new thread executes
    }
    
    sleep(sleep_time); //sleep for specified time

    /* Exit the program */
    return 0;

}
void initialize()
{
    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
}

int insert_item(buffer_item item)
{
    
    int successful=0;
    
    sem_wait(&empty);   //decrements empty semaphore, if value is greater than zero, then the decrement proceeds, else
    //If the semaphore currently has the value zero, then the call blocks until either it becomes possible to perform the decrement
    //wait until buffers are empty
    
    pthread_mutex_lock(&mutex);
    
    //insert item into buffer
    if(count != BUFFER_SIZE)
        
    {
        buffer[insert_index] = item;
        insert_index = (insert_index + 1) % BUFFER_SIZE;
        count++;
    }
    
    //if unsuccessful
    else
    {
        successful=-1;
    }
    
    pthread_mutex_unlock(&mutex);
    sem_post(&full); //increments full semaphore
    
    return (successful);
    
}

int remove_item(buffer_item *item)
{
    
    int successful = 0;
    
    sem_wait(&full);
    pthread_mutex_lock(&mutex);
    
    if(count != 0)
        
    {
        *item=buffer[remove_index];
        remove_index=(remove_index+1) % BUFFER_SIZE;
        count--;
    }
    
    else
        
    {
        successful = -1;
    }
    
    // Release mutex lock and empty semaphore /
    
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);
    
    return (successful);
    
}

void *producer(void *param)

{

    buffer_item item; //is type int

    while(TRUE)
    {
        sleep(rand() % 10 + 1); //sleep from 1 - 10 seconds
        
        item = rand();

        //if insert_item returns -1 (indicating error), the if function will execute
        if(insert_item(item))
        {
            printf("report error conition \n");
        }
        else
        {
            printf("\nProducer %u produced item %d \n", (unsigned int)pthread_self(), item);
        }
    }

}

void *consumer(void *param)
{

    buffer_item item;

    while(TRUE)
    {
        sleep(rand() % 10 + 1); //sleep from 1 - 10 seconds

        //if remove_item returns -1 (indicating error), the if function will execute
        if(remove_item(&item))
        {
            printf("report error conition \n");
        }

        else
        {
            printf("\nConsumer %u consumed item %d \n", (unsigned int)pthread_self(), item);

        }
    }

}
