#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include <stdlib.h>
#include<semaphore.h>
#include "scheduler.c"
#include "db.c"
#include "server.c"

void test_gen_requests(){
    // printf("1");
    ReqNode req;
    strcpy(req.user1,"a");
    strcpy(req.user2,"b");
    req.amount = 100;
    for(int i=0;i<3;i++){
        int cnt = 100;
        while(cnt--){
            req.type = cnt%2;
            add_request(req);
        }
        sleep(5);
    }
}


int main(void){
    
    create_db();
    clean_table("net_dues");
    clean_table("users");
    clean_table("trans");

    sem_init(&mutex_list, 0, 1);
    sem_init(&mutex_execute, 0, 1);

    req_head = NULL;
    req_tail = req_head;

    pthread_t ptid1;
    pthread_t ptid2;
    pthread_t ptid3;

	pthread_t executers[10];
    int num = 2;

    pthread_create(&ptid1, NULL, &test_gen_requests, NULL);

    // pthread_create(&ptid2, NULL, &wake_up_request_FCFS, NULL);

    // pthread_detach(ptid2);
    // wake_up_request_FCFS();
    // test_gen_requests();

    pthread_create(&ptid3, NULL, &listen_Server, NULL);


    int w;
	for(w=0;w<num;w++)
	{
		pthread_create(&executers[w],NULL,&wake_up_request_Priority_Scheduling,NULL); 
        printf("\nExecutor num %d : %d\n",(w+1),executers[w]);
    }
    for(w=0;w<num;w++)
	{
		// pthread_create(&executers[w],NULL,&wake_up_request_Priority_Scheduling,NULL); 
        pthread_join(executers[w], NULL);
        // printf("Executor num %d : %d\n",(w+1),executers[w]);
    }
    // pthread_join(ptid1, NULL);
    pthread_join(ptid3,NULL);
    // pthread_join(ptid2, NULL);

    return 0;
}