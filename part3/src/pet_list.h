// Part 3d: linked lists for pets waiting on floors and riding the elevator 
// Each pet is in exactly one list - its start floor -> elevator
// None of these functions lock (no synchronization)

#ifndef PET_LIST_H
#define PET_LIST_H

#include <linux/list.h>
#include <linux/types.h>

#define NUM_FLOORS  5
#define MAX_PETS    5
#define MAX_WEIGHT  50

enum pet_type { CHIHUAHUA, PUG, PUGHUAHUA, DACHSHUND, NUM_PET_TYPES };

struct pet {
    int type;
    int start_floor;
    int dest_floor;
    int weight;
    struct list_head list;
};

struct building {
    struct list_head floors[NUM_FLOORS]; // floors[0] is floor 1
    struct list_head elevator;
    int num_waiting;
    int num_on_board;
    int load;
    int num_serviced;
};

void building_init(struct building *b);
int add_pet(struct building *b, int start_floor, int dest_floor, int type);
int load_pets(struct building *b, int floor);
int unload_pets(struct building *b, int floor);
int floor_count(struct building *b, int floor);
bool next_pet_fits(struct building *b, int floor);
bool has_dropoff(struct building *b, int floor);
int print_floor(struct building *b, int floor, char *buf, size_t size);
int print_elevator(struct building *b, char *buf, size_t size);
void free_all_pets(struct building *b);

#endif
