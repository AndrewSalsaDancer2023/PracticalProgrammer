#include <gtest/gtest.h>
#include <pthread.h>

pthread_mutex_t lock; 

struct raimutext {
	raimutext(pthread_mutex_t* theLock)
	:lock_(theLock)
	{
	 pthread_mutex_lock(lock_); 
	}
	~raimutext()
	{
	 pthread_mutex_unlock(lock_); 	
	}
	pthread_mutex_t* lock_{};
};

int var = 0;
void* child_fn ( void* arg ) {
 raimutext mut(&lock);
 var++;
 return NULL;
}

TEST(EqualityTest, Equal) {
   pthread_t child;
   pthread_create(&child, NULL, child_fn, NULL);
   {
   struct raimutext mut(&lock);
   var++;
   }
   pthread_join(child, NULL);
   EXPECT_EQ(var, 2);
}

