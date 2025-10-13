// Basic node structure
typedef struct __hoh_node_t {
    int key;
    struct __hoh_node_t *next;
    pthread_mutex_t lock;
} hoh_node_t;

// Basic list structure
typedef struct __hoh_list_t {
    hoh_node_t *head;
} hoh_list_t;

void hoh_list_init(hoh_list_t *L);
void hoh_node_init(hoh_node_t *node, int key, hoh_node_t *next);
void hoh_node_insert(hoh_list_t *L, int key);
int hoh_list_lookup(hoh_list_t *L, int key);
void hoh_Node_Destroy(hoh_node_t *node);
void hoh_List_Destroy(hoh_list_t *L);
