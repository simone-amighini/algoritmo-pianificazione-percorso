#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


// BOOLEAN TYPE
typedef enum{FALSE = 0, TRUE = 1} bool;

// RED-BLACK TREE DATA STRUCTURES:
typedef enum {RED = 0, BLACK = 1} color_t;

typedef struct node {
    int key;                /*
                             * if the node is used for a station: key = station_km
                             * if the node is used for a car: key = car_range
                             */
    
    struct tree *car_fleet; /*
                             * if the node is used for a station: car_fleet -> car fleet's data structure (see "add_car")
                             * if the node is used for a car: car_fleet -> NULL (see "insert_node")
                             */
    
    color_t color;
    struct node *parent;
    struct node *left;      // left child
    struct node *right;     // right child
} node_t;

typedef struct tree {
    struct node *root;
    node_t *nil;
} tree_t;


// FUNCTION DECLARATIONS
void    add_station(tree_t *stations, int station_km, int fleet_size);
void    demolish_station(tree_t *stations, int station_km);
void    add_car(tree_t *stations, int station_km, int new_car_range, bool print_requested);
void    scrap_car(tree_t *stations, int station_km, int old_car_range);
void    plan_route(tree_t *stations, int start_km, int end_km);
bool    evaluate_route_asc(int max_stages[], int index, int length);
bool    evaluate_route_des(int max_stages[], int index, int length);

tree_t* init_tree(void);
void    insert_node(tree_t *T, int key);
void    insert_node_fixup(tree_t *T, node_t *z);
node_t* find_tree_min(tree_t *T, node_t *x);
node_t* find_tree_max(tree_t *T, node_t *x);
node_t* find_previous_node(tree_t *T, node_t *x);
node_t* find_next_node(tree_t *T, node_t *x);
node_t* search_tree(tree_t *T, node_t *x, int key);
void    left_rotate_tree(tree_t *T, node_t *x);
void    right_rotate_tree(tree_t *T, node_t *y);
void    delete_node(tree_t *T, node_t *z);
void    delete_node_fixup(tree_t *T, node_t *x);
void    delete_tree(tree_t *T);
void    delete_tree_aux(tree_t *T, node_t *x);


int main(char* argv[], int argc) {
    char command[20];   // a command is always shorter than 20 chars
    int station_km, fleet_size, new_car_range, old_car_range, start_km, end_km;   // commands' parameters
    
    tree_t *stations = init_tree(); // stations' data stucture
    
    // scan the input until the end of the file
    while (!feof(stdin)) {
        // scan the command
        if (fscanf(stdin,"%s", command) != 1) {
            if (feof(stdin)) {
                break;
            } else {
                fprintf(stderr, "Errata lettura del comando in input\n");
                exit(1);
            }
        }
        
        // recognise and execute the command
        if (strncmp(command, "aggiungi-stazione", 17) == 0) {
            if (scanf("%d %d", &station_km, &fleet_size) == 2) {
                add_station(stations, station_km, fleet_size);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti di aggiungi-stazione\n");
                exit(1);
            }
        } else if (strncmp(command, "demolisci-stazione", 18) == 0) {
            if (scanf("%d", &station_km) == 1) {
                demolish_station(stations, station_km);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti di demolisci-stazione\n");
                exit(1);
            }
        } else if (strncmp(command, "aggiungi-auto", 13) == 0) {
            if (scanf("%d %d", &station_km, &new_car_range) == 2) {
                add_car(stations, station_km, new_car_range, TRUE);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti di aggiungi-auto\n");
                exit(1);
            }
        } else if (strncmp(command, "rottama-auto", 12) == 0) {
            if (scanf("%d %d", &station_km, &old_car_range) == 2) {
                scrap_car(stations, station_km, old_car_range);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti rottama-auto\n");
                exit(1);
            }
        } else if (strcmp(command, "pianifica-percorso") == 0) {
            if (scanf("%d %d", &start_km, &end_km) == 2) {
                plan_route(stations, start_km, end_km);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti di pianifica-percorso\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Comando non trovato\n");
            exit(1);
        }
    }
    
    // free the memory
    delete_tree(stations);
    
    return 0;
}


void add_station(tree_t *stations, int station_km, int fleet_size) {
    // add the station if it doesn't exist
    if (search_tree(stations, stations->root, station_km) == NULL) {
        insert_node(stations, station_km);
        printf("aggiunta\n");
        
        // add the cars
        int new_car_range;
        
        for (int i = 0; i < fleet_size; i++) {
            if (scanf("%d", &new_car_range) == 1) {
                add_car(stations, station_km, new_car_range, FALSE);
            } else {
                fprintf(stderr, "Errata lettura degli argomenti di aggiungi-auto all'interno della funzione aggiungi-stazione\n");
                exit(1);
            }
        }
    } else {
        printf("non aggiunta\n");
        
        // clear unused parameters from stdin
        int junk;
        
        for (int i = 0; i < fleet_size; i++) {
            if (scanf("%d", &junk) != 1) {
                fprintf(stderr, "Errata lettura degli argomenti di aggiungi-auto all'interno della funzione aggiungi-stazione\n");
                exit(1);
            }
        }
    }
}

void demolish_station(tree_t *stations, int station_km) {
    node_t *target_station = search_tree(stations, stations->root, station_km);
    
    // check if the station exists
    if (target_station != NULL) {
        delete_tree(target_station->car_fleet); // scrap all the cars in the station
        delete_node(stations, target_station);  // demolish the station
        printf("demolita\n");
    } else {
        printf("non demolita\n");
    }
}

void add_car(tree_t *stations, int station_km, int new_car_range, bool print_requested) {
    node_t *target_station = search_tree(stations, stations->root, station_km);
    
    // check if the station exists
    if (target_station != NULL) {
        // initialise the car fleet if it hasn't been created yet       
        if (target_station->car_fleet == NULL) {
            target_station->car_fleet = init_tree();
        }
        
        // add the car
        insert_node(target_station->car_fleet, new_car_range);
        
        if (print_requested) {
            printf("aggiunta\n");
        }
    } else {
        if (print_requested) {
            printf("non aggiunta\n");
        }
    }
}

void scrap_car(tree_t *stations, int station_km, int old_car_range) {
    node_t *target_station = search_tree(stations, stations->root, station_km);
    
    // check if station exists
    if (target_station != NULL) {
        node_t *old_car = search_tree(target_station->car_fleet, target_station->car_fleet->root, old_car_range);
        
        if (old_car != NULL) {
            delete_node(target_station->car_fleet, old_car);
            printf("rottamata\n");
        } else {
            printf("non rottamata\n");
        }
    }
    else {
        printf("non rottamata\n");
    }
}

void plan_route(tree_t *stations, int start_km, int end_km) {
    node_t *start_station = search_tree(stations, stations->root, start_km);
    node_t *end_station = search_tree(stations, stations->root, end_km);
    
    if ((start_station != NULL) && (end_station != NULL)) {
        if (start_station == end_station) {
            printf("%d\n", start_km);
        } else {
            // find the direction to establish which function to use
            node_t* (*find_next_station)(tree_t *, node_t *);
            bool (*evaluate_route) (int *, int, int);
            
            if (start_station->key < end_station->key) {
                find_next_station = find_next_node;
                evaluate_route = evaluate_route_asc;
            } else {
                find_next_station = find_previous_node;
                evaluate_route = evaluate_route_des;
            }
            
            // prepare array of intermediate stations
            node_t *current_station = start_station;
            int length = 1;
            
            while (current_station != end_station) {
                // count the number of stages between start and end (both included)
                current_station = find_next_station(stations, current_station);
                length++;
            }
            
            node_t *inter_stations[length];     // allocate array of intermediate stations
            
            current_station = start_station;    // restart from the beginning
            
            // fill the array with pointers to intermediate stations
            for (int i = 0; i < length; i++) {
                inter_stations[i] = current_station;
                current_station = find_next_station(stations, current_station);
            }
            
            // calculate the maximum number of stations reachable from the i-th one and put the result into max_stages[i]
            int dst;
            int max_range;
            node_t *max_range_car = NULL;
            int max_stages[length];
            
            for (int i = 0; i < length; i++) {
                // check if car fleet has been initialised
                if (inter_stations[i]->car_fleet != NULL) {
                    max_range_car = find_tree_max(inter_stations[i]->car_fleet, inter_stations[i]->car_fleet->root);
                    
                    // check if car fleet is not empty
                    if (max_range_car != inter_stations[i]->car_fleet->nil) {
                        max_range = max_range_car->key;
                    } else {
                        max_range = 0;
                    }
                } else {
                    max_range = 0;
                }
                
                max_stages[i] = 0;
                for (int j = i + 1; j < length; j++) {
                    dst = abs(inter_stations[j]->key - inter_stations[i]->key);
                    if (dst <= max_range) {
                        max_stages[i]++;
                    } else {
                        break;
                    }
                }
            }
            
            // if there's an available route, print the best one
            if (evaluate_route(max_stages, length - 1, length) == TRUE) {
                int i = 0;
                
                while (max_stages[i] != 0) {
                    printf("%d ", inter_stations[i]->key);
                    i = i + max_stages[i];
                }
                
                printf("%d\n", inter_stations[i]->key);   // print end station
            } else {
                printf("nessun percorso\n");
            }
        }
    } else {
        printf("nessun percorso\n");
    }
}

bool evaluate_route_asc(int max_stages[], int index, int length) {
    int best_number = 0;
    int sum = INT_MAX;
    int new_sum;
    int i;
    bool passthrough;
    
    while (max_stages[index] > 0) {
        new_sum = 1;
        i = index;
        passthrough = FALSE;
        
        while ((max_stages[i] != 0) && (i != length - 1)) {
            i = i + max_stages[i];
            new_sum++;
            
            if (new_sum > sum) {
                passthrough = TRUE;
                break;
            }
        }
        
        // <= : in case of equal number of stages, choose the smallest jump
        if ((new_sum <= sum) && (i == length - 1) && (passthrough == FALSE)) {
            sum = new_sum;
            best_number = max_stages[index];
        }
        
        max_stages[index]--;
    }
    
    max_stages[index] = best_number;
        
    if (index > 0) {
        return evaluate_route_asc(max_stages, index - 1, length);
    } else {
        i = 0;
        while (max_stages[i] != 0) {
            i = i + max_stages[i];
        }
        
        if (i == length - 1) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

bool evaluate_route_des(int max_stages[], int index, int length) {
    int best_number = 0;
    int sum = INT_MAX;
    int new_sum;
    int i;
    int j;
    int i_old;
    int j_old;
    bool passthrough;
    
    while (max_stages[index] > 0) {
        new_sum = 1;
        i = index;
        passthrough = FALSE;
        
        while ((max_stages[i] != 0) && (i != length - 1)) {
            i = i + max_stages[i];
            new_sum++;
            
            if (new_sum > sum) {
                passthrough = TRUE;
                break;
            }
        }
        
        if ((i == length - 1) && (passthrough == FALSE)) {
            if (new_sum < sum) {
                sum = new_sum;
                best_number = max_stages[index];
            } else if (new_sum == sum) {
                // in case of equal number of stages, look at the following stages to decide
                i = index + max_stages[index];  // index for proposed new value for best_number
                j = index + best_number;        // index for saved value of best_number
                
                i_old = i;
                j_old = j;
                
                // find the best alternative between best_number and max_stages[i] (= proposed new value for best_number)
                while (i != length - 1) {
                    if (i == j) {
                        // exit from loop when the last common stage has been found...
                        break;
                    }
                    
                    i_old = i;
                    j_old = j;
                    
                    i = i + max_stages[i];
                    j = j + max_stages[j];
                }
                
                if (i_old > j_old) {
                    sum = new_sum;
                    best_number = max_stages[index];
                }
            }
        }
        
        max_stages[index]--;
    }

    max_stages[index] = best_number;
        
    if (index > 0) {
        return evaluate_route_des(max_stages, index - 1, length);
    } else {
        i = 0;
        while (max_stages[i] != 0) {
            i = i + max_stages[i];
        }
        
        if (i == length - 1) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

tree_t* init_tree(void) {
    // initialising the tree and T->nil node
    tree_t *T = (tree_t *) malloc(sizeof(tree_t));
    T->nil = (node_t *) malloc(sizeof(node_t));
    T->root = T->nil;
    
    // initialising T->nil's fields
    T->nil->left = T->nil;
    T->nil->right = T->nil;
    T->nil->parent = T->nil;
    T->nil->color = BLACK;
    T->nil->car_fleet = NULL;
    T->nil->key = 0;
    
    return T;
}

// ALGORITHM ADAPTED FROM BOOK
void insert_node(tree_t *T, int key) {
    node_t *z = (node_t *) malloc(sizeof(node_t)); 
    node_t *y = T->nil;     // y will be parent of z
    node_t *x = T->root;    // node being compared with z
    
    z->key = key;
    
    // descend until reaching the sentinel
    while (x != T->nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    
    z->parent = y;          // found the location: insert z with parent y
    
    if (y == T->nil) {
        T->root = z;            // tree T was empty
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }
    
    z->left = T->nil;       // both of z's chidren are the sentinel
    z->right = T->nil;
    z->color = RED;         // the new node starts out RED
    z->car_fleet = NULL;
    insert_node_fixup(T, z); // correct any violations of rb tree properties
}

// ALGORITHM ADAPTED FROM BOOK
void insert_node_fixup(tree_t *T, node_t *z) {
    node_t *y = NULL;
    node_t *x = NULL;
    
    if (z == T->root) {
        T->root->color = BLACK;
    } else {
        x = z->parent;
        if (x->color == RED) {
            if (x == x->parent->left) {
                y = x->parent->right;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->parent->color = RED;
                    insert_node_fixup(T, x->parent);
                } else {
                    if (z == x->right) {
                        z = x;
                        left_rotate_tree(T, z);
                        x = z->parent;
                    }
                    x->color = BLACK;
                    x->parent->color = RED;
                    right_rotate_tree(T, x->parent);
                }
            } else {
                y = x->parent->left;
                if (y->color == RED) {
                    x->color = BLACK;
                    y->color = BLACK;
                    x->parent->color = RED;
                    insert_node_fixup(T, x->parent);
                } else {
                    if (z == x->left) {
                        z = x;
                        right_rotate_tree(T, z);
                        x = z->parent;
                    }
                    x->color = BLACK;
                    x->parent->color = RED;
                    left_rotate_tree(T, x->parent);
                }
            }
        } 
    }
}

// ALGORITHM ADAPTED FROM BOOK
node_t* find_tree_min(tree_t *T, node_t *x) {
    while (x->left != T->nil) {
        x = x->left;
    }
    return x;
}

// ALGORITHM ADAPTED FROM BOOK
node_t* find_tree_max(tree_t *T, node_t *x) {
    while (x->right != T->nil) {
        x = x->right;
    }
    return x;
}

// ALGORITHM ADAPTED FROM BOOK
node_t* find_previous_node(tree_t *T, node_t *x) {
    node_t *y = NULL;
    
    if (x->left != T->nil) {
        return find_tree_max(T, x->left);
    } else {
        y = x->parent;
        while ((y != T->nil) && (x == y->left)) {
            x = y;
            y = y->parent;
        }
        return y;
    }
}

// ALGORITHM ADAPTED FROM BOOK
node_t* find_next_node(tree_t *T, node_t *x) {
    node_t *y = NULL;
    
    if (x->right != T->nil) {
        return find_tree_min(T, x->right);  // leftmost node in right subtree
    } else {
        // find the lowest ancestor of x whose left child is an ancestor of x
        y = x->parent;
        while ((y != T->nil) && (x == y->right)) {
            x = y;
            y = y->parent;
        }
        return y;
    }
}

node_t* search_tree(tree_t *T, node_t *x, int key) {
    // search the (sub)tree starting from the node x
    while (TRUE) {
        if (x == T->nil) {
            // empty (sub)tree
            return NULL;
        } else if (key == x->key) {
            // key found in (sub)tree
            return x;
        } else {
            // key not found: search continues in left or right subtree
            if (key < x->key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }
    }
}

// ALGORITHM ADAPTED FROM BOOK
void left_rotate_tree(tree_t *T, node_t *x) {
    node_t *y = x->right;
    
    x->right = y->left;
    
    if (y->left != T->nil) {
        y->left->parent = x;
    }
    
    y->parent = x->parent;
    
    if (x->parent == T->nil) {
        T->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    y->left = x;
    x->parent = y;
}

// ALGORITHM ADAPTED FROM BOOK
void right_rotate_tree(tree_t *T, node_t *y) {
    node_t *x = y->left;
    
    y->left = x->right;
    
    if (x->right != T->nil) {
        x->right->parent = y;
    }
    
    x->parent = y->parent;
    
    if (y->parent == T->nil) {
        T->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    
    x->right = y;
    y->parent = x;
}

// ALGORITHM ADAPTED FROM BOOK
void delete_node(tree_t *T, node_t *z) {
    node_t *y = NULL;
    node_t *x = NULL;
    
    if ((z->left == T->nil) || (z->right == T->nil)) {
        y = z;
    } else {
        y = find_next_node(T, z);
    }
    
    if (y->left != T->nil) {
        x = y->left;
    } else {
        x = y->right;
    }
    
    x->parent = y->parent;
    
    if (y->parent == T->nil) {
        T->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    
    if (y != z) {
        z->key = y->key;
        z->car_fleet = y->car_fleet;
    }
    
    if (y->color == BLACK) {
        // correct any violations of rb tree properties
        delete_node_fixup(T, x);
    }
    
    // free the memory
    free(y);
}

// correct any violations of rb tree properties
void delete_node_fixup(tree_t *T, node_t *x) {
    node_t *w = NULL;
    
    if ((x->color == RED) || (x->parent == T->nil)) {
        x->color = BLACK;
    } else if (x == x->parent->left) {
        w = x->parent->right;
        if (w->color == RED) {
            w->color = BLACK;
            x->parent->color = RED;
            left_rotate_tree(T, x->parent);
            w = x->parent->right;
        }
        if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
            w->color = RED;
            delete_node_fixup(T, x->parent);
        } else {
            if (w->right->color == BLACK) {
                w->left->color = BLACK;
                w->color = RED;
                right_rotate_tree(T, w);
                w = x->parent->right;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->right->color = BLACK;
            left_rotate_tree(T, x->parent);
        }
    } else {
        w = x->parent->left;
        if (w->color == RED) {
            w->color = BLACK;
            x->parent->color = RED;
            right_rotate_tree(T, x->parent);
            w = x->parent->left;
        }
        if ((w->right->color == BLACK) && (w->left->color == BLACK)) {
            w->color = RED;
            delete_node_fixup(T, x->parent);
        } else {
            if (w->left->color == BLACK) {
                w->right->color = BLACK;
                w->color = RED;
                left_rotate_tree(T, w);
                w = x->parent->left;
            }
            w->color = x->parent->color;
            x->parent->color = BLACK;
            w->left->color = BLACK;
            right_rotate_tree(T, x->parent);
        }
    }
}

void delete_tree(tree_t *T) {
    node_t *x = T->root;
    
    delete_tree_aux(T, x);
    free(T->nil);
    free(T);
}

void delete_tree_aux(tree_t *T, node_t *x) {
    if (x != T->nil) {
        delete_tree_aux(T, x->left);
        delete_tree_aux(T, x->right);
        free(x);
    }
}