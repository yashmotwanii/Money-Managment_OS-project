typedef struct RequestNode {
    int type; // 1 for post , 0 for get
    
    // The following are used only for post request
    char user1[1024];
    char user2[1024];
    int amount;

    // Assign a priority for SJF/Priority Scheduling
    int priority;

    // Return the http file for whichever request
    char* html_file_name;
} ReqNode;

typedef struct list
{
    ReqNode req;
    struct ListNode *next;
} ListNode;

extern volatile ListNode* req_head;
extern volatile ListNode* req_tail;

extern volatile sem_t mutex_list;
extern volatile sem_t mutex_execute;

extern ReqNode global_req; 