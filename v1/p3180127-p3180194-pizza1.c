// We include our header file which contains all our constants and nessecary header files!
#include "p3180127-p3180194-pizza1.h"

// Global variables
int N_cust; // Number of customers (orders).
// Mutexes
pthread_mutex_t lock_sum; // Mutex used while calculating sum.
pthread_mutex_t lock_max; // Mutex used while calculating max.
pthread_mutex_t lock_console; // Mutex used while printing.
pthread_mutex_t lock_order_time; // Mutex used while calculating the time each order takes to complete.
pthread_mutex_t lock_cook; // Mutex used for cooks.
pthread_mutex_t lock_oven; // Mutex used for ovens.
// Condition Variables.
pthread_cond_t  cond_cook; // Condition variale for cooks.
pthread_cond_t  cond_oven; // Condition variale for ovens.

int cooks; // Number of cooks available.
int ovens; // Number of ovens available.
int responce_code; 
int pizzas; // Number of pizzas for each order.
int max;
int sum;

void *pizza(void *id){
	int order_id = *(int *)id; // order_id is set equal to the id of the current thread.
	struct timespec start; // Variable for the start of each order.
	struct timespec finish; // Variable for the completion of each order.
	int order_time;	// Variable used to calculate how long each order took to complete.

	// Start order!
	clock_gettime(CLOCK_REALTIME, &start); // Start timing order...

	// Lock cooks to assign the order to a cook!
	responce_code = pthread_mutex_lock(&lock_cook);
	// No cooks available for this order. Please wait...
	while (cooks == 0){
		responce_code = pthread_cond_wait(&cond_cook, &lock_cook);
	}
	cooks--; // A cook has taken over this order!
	// Unlocking cooks for other orders.
	responce_code = pthread_mutex_unlock(&lock_cook); 

	// The cook is preparing the pizzas!
	sleep(T_prep*pizzas);
	// The pizzas are ready to bake!
	
	// Lock ovens to use one for this order.
	responce_code = pthread_mutex_lock(&lock_oven);
	// No available ovens for this order. Please wait...
	while (ovens == 0){
		responce_code = pthread_cond_wait(&cond_oven, &lock_oven);
	}
	ovens--; // We found an available oven for our pizzas!
	// Unlocking ovens for other orders.
	responce_code = pthread_mutex_unlock(&lock_oven);
	
	// The order is baking...
	sleep(T_bake);
	// The order is baked! 
	
	// Lock ovens!
	responce_code = pthread_mutex_lock(&lock_oven);
	ovens++; // Releasing the oven we used.
	// Wake up threads waiting for an oven.
	responce_code = pthread_cond_signal(&cond_oven);
	// Unlocking ovens for other orders.
    responce_code = pthread_mutex_unlock(&lock_oven); 

	// Lock cooks!
	responce_code = pthread_mutex_lock(&lock_cook);
	cooks++; // The cook can take a different order.
	// Wake up threads waiting for a cook.
    responce_code = pthread_cond_signal(&cond_cook);
	// Unlocking cooks for other orders.
	responce_code = pthread_mutex_unlock(&lock_cook);
	
	// The order is ready! Stop timing it!
	clock_gettime(CLOCK_REALTIME, &finish);

	// Lock order_time to calculate how long it took us to complete the order.
	responce_code = pthread_mutex_lock(&lock_order_time);
	order_time = finish.tv_sec - start.tv_sec;
	responce_code = pthread_mutex_unlock(&lock_order_time); // Unlock order_time.
	
	// Lock max and check if this order took more time than the max value.
	responce_code = pthread_mutex_lock(&lock_max);
	if (order_time > max) 
		max = order_time; // If this order took the longest so far set max equal to the time it took.
	responce_code = pthread_mutex_unlock(&lock_max); // Unlock max.

	// Lock sum in order to add the time it took to complete to the current sum.
	responce_code = pthread_mutex_lock(&lock_sum);
	sum += order_time;
	responce_code = pthread_mutex_unlock(&lock_sum);

	// Lock the console to print that the order is ready as well as the time it took to complete!
	responce_code = pthread_mutex_lock(&lock_console);
	printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %d λεπτά.\n", order_id, order_time);
	responce_code = pthread_mutex_unlock(&lock_console);
	
	pthread_exit(NULL); // Terminate the calling thread.
}

int main(int argc, char *argv[]){
	// Printing error message if we get less or more than the needed arguments
	if (argc != 3) {
		printf("ERROR: Invalid number of arguments\n");
		exit(-1);
	}
	// We initialize max and sum.
	max = 0; // We set max as 0 since there will be on order ready in 0 minutes.
	sum = 0;
	// Variables cooks and ovens will be equal to the constants N_cook and N_oven from our header file.
	cooks = N_cook;
	ovens = N_oven;
	// Initializing mutexes.
	pthread_mutex_init(&lock_order_time, NULL);
	pthread_mutex_init(&lock_console, NULL);
	pthread_mutex_init(&lock_max, NULL);
	pthread_mutex_init(&lock_sum, NULL);
	pthread_mutex_init(&lock_cook, NULL);
	pthread_mutex_init(&lock_oven, NULL);
	// Initializing condition variables.
    pthread_cond_init(&cond_cook, NULL);
	pthread_cond_init(&cond_oven, NULL);

	N_cust = atoi(argv[1]); // N_cust is set equal to the first argument and represents the number of customers (orders) we have. 
	long int seed = atoi(argv[2]); // seed is set equal to the second argument.
	unsigned int new_seed; // new_seed is the seed for function rand_r.

	int id[N_cust]; // Array for the ids
	pthread_t threads[N_cust]; // Array for the threads

	// Creating threads...
	for (int i = 0; i < N_cust; i++) {
		id[i] = i+1; // ids start from 1 to N_cust
		// new_seed has a different value for every thread.
		new_seed = time(&seed)^getpid()^pthread_self();
		// First order starts right away!
		// The other orders arrive at random amount of time..
		if (i!=0){
			// We use time to calculate the amount of time the rest of the orders arrive after using rand_r to generate random numbers in [1,5].
			int time = rand_r(&new_seed)%T_orderhigh + T_orderlow;
			sleep(time); // Waiting for a few seconds..
		}
		// We use pizzas to calculate the amount of pizzas each customer orders.
		pizzas = rand_r(&new_seed)%N_orderhigh + N_orderlow;
		// Main: creating thread i+1
    	responce_code = pthread_create(&threads[i], NULL, pizza, &id[i]);
    }

	// Joining threads...
	for (int i = 0; i < N_cust; i++) {
		pthread_join(threads[i], NULL);
    }
	// Destroying all the mutexes.
	pthread_mutex_destroy(&lock_console);
	pthread_mutex_destroy(&lock_order_time);
	pthread_mutex_destroy(&lock_sum);
	pthread_mutex_destroy(&lock_max);
    pthread_mutex_destroy(&lock_cook);
	pthread_mutex_destroy(&lock_oven);
	// Destroying all the condition variables.
    pthread_cond_destroy(&cond_cook);
	pthread_cond_destroy(&cond_oven);
	
	// Printing the average and maximum time we took to complete the orders.
	printf("Μέσος χρόνος ολοκλήρωσης των παραγγελιών: %d λεπτά.\n", sum/N_cust);
	printf("Μέγιστος χρόνος ολοκλήρωσης των παραγγελιών: %d λεπτά.\n", max);
	return 0;
}
