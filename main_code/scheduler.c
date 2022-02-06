#include "scheduler.h"

volatile ListNode* req_head;
volatile ListNode* req_tail;
ReqNode assignPriority(ReqNode req);

volatile sem_t mutex_list;
volatile sem_t mutex_execute;


int getLen(ListNode* head){
    int count = 0;
    while(head != NULL) {
        head = head->next;
        count++;
    }
    return count;
}


void add_request(ReqNode req){
    sem_wait(&mutex_list);
    ListNode* temp;
    temp = malloc(sizeof(ListNode));
    req = assignPriority(req);
    strcpy(temp->req.user1,req.user1);
    strcpy(temp->req.user2,req.user2);
    temp->req.amount=req.amount;
    temp->req.type=req.type;
    temp->next = NULL;

    // temp->req = req;


    if(req_head == NULL){
        req_tail = temp;
        req_head = req_tail; 
    } else {
        req_tail->next = temp;
        req_tail = req_tail->next;
    }
    printf("Length : %d\n",getLen(req_head));
    printf("req : %s %s %d %d END \n",req.user1,req.user2,req.amount,req.type);
    printf("req_head : %s %s %d %d END \n",req_head->req.user1,req_head->req.user2,req_head->req.amount,req_head->req.type);
    printf("req_tail : %s %s %d %d END \n",req_tail->req.user1,req_tail->req.user2,req_tail->req.amount,req_head->req.type);
    
    sem_post(&mutex_list);

    return;
}

void execute_req_type1(ReqNode req){
    printf("Executing Request Type 1\n");
    return;
}

void execute_req_type2(ReqNode req){
    printf("Executing Request Type 2\n");
    return;
}

// Mode FCFS
void wake_up_request_FCFS(){
    int cnt =0;
    while (1)
    {
        sem_wait(&mutex_execute);
        sem_wait(&mutex_list);

        printf("Executor id = %d\n", pthread_self());
        if(req_head != NULL){
            ListNode* prev = req_head;
            printf("%d ",cnt++);
            if(req_head->req.type == 0){

                execute_req_type1(req_head->req);
            } else if(req_head->req.type == 1){
                execute_req_type2(req_head->req);
            } else {
                printf("Unknown type of request");
            }

            req_head = req_head->next;
            free(prev);
        }
        // else{
        //     req_tail = req_head;
        //     printf("No requests present. Sleeping for 10 secs before restart\n");
        //     sleep(10);
        // }
        
        sem_post(&mutex_execute);
        sem_post(&mutex_list);
        // if(req_head == NULL){
        // sleep(1);
        // }
    }
}

ReqNode assignPriority(ReqNode req){
    req.priority = 0;
    if(req.type == 1){
        req.priority = 100;
    } else if(req.type == 0) {
        req.priority = 50;
        // req.priority = value[req.html_file_name];
    } else {
        printf("Unknown Type of Request\n");
        exit(1);
    }
    return req;
}

// Highest Priority First
void wake_up_request_Priority_Scheduling(){
    
    int cnt =0;
    ListNode* reorder = NULL;
    ListNode* reorder_copy = NULL;
    ListNode* curr_head = NULL;
    ListNode* required_prev = NULL;
    ListNode* required_node = NULL;
    ListNode* prev = NULL;
    ListNode* reqs = NULL;
    printf("Starting Infinite While\n");
    while (1){
        sem_wait(&mutex_execute);
        sem_wait(&mutex_list);
        printf("Executor id = %d\n", pthread_self());

        if(req_head != NULL){
            prev = NULL;
            
            reqs = req_head;

            int count = 10;

            while (count-- && req_head != NULL)
            {
                prev = req_head;
                req_head = req_head->next;
            }

            if(prev != NULL) prev->next = NULL;

            if(req_head == NULL) req_tail = req_head;

            printf("Got first 10\n");

            reorder = NULL;
            reorder_copy = reorder;
            curr_head = reqs;
            required_prev = NULL;

            count = 10;
            while (count--)
            {
                int curr_max = -100000;
                curr_head = reqs;
                prev = NULL;
                
                while(curr_head != NULL){
                    if(curr_max < curr_head->req.priority){
                        curr_max = curr_head->req.priority;
                        required_node = curr_head;
                        required_prev = prev;
                    }
                    prev = curr_head;
                    curr_head = curr_head->next;
                }

                if(required_prev != NULL) required_prev->next = required_node->next;
                else reqs = reqs->next;

                if(reorder == NULL) {
                    reorder = required_node;
                    reorder_copy = reorder;
                }
                else {
                    reorder->next = required_node;
                    reorder = reorder->next;
                }
            }

            reorder = reorder_copy;

            printf("Executing First 10\n");
            
            while(reorder != NULL){
                prev = reorder;
                printf("%d ",cnt++);
                if(reorder->req.type == 0){
                    execute_req_type1(reorder->req);
                } else if(reorder->req.type == 1){
                    execute_req_type2(reorder->req);
                    add_entry(reorder->req.user1,reorder->req.user2,reorder->req.amount);
                } else {
                    printf("Unknown type of request\n");
                }

                reorder = reorder->next;
                free(prev);
            }
        }
        sem_post(&mutex_list);
        sem_post(&mutex_execute);
        // sleep(1);
    }

}

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