


// basic node structure
typedef struct __node_t {
    int key;
    struct __node_t *next;
} node_t;

// basic list structure (one used per list)
typedef struct __list_t {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

void list_init(list_t *L);
void list_insert(list_t *L, int key);
int list_lookup(list_t *L, int key);
void list_destroy(list_t *L);