// Part 3d: pet list operations

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include "pet_list.h"

// static for encapsulation
static const int pet_weights[NUM_PET_TYPES] = { 3, 14, 10, 16 };
static const char pet_letters[NUM_PET_TYPES] = { 'C', 'P', 'H', 'D' };

static bool valid_floor(int floor)
{
    return floor >= 1 && floor <= NUM_FLOORS;
}

void building_init(struct building *b)
{
    int i;

    // An empty kernel list points at itself. Using one before this crashes.
    for (i = 0; i < NUM_FLOORS; i++)
        INIT_LIST_HEAD(&b->floors[i]);
    INIT_LIST_HEAD(&b->elevator);

    b->num_waiting = 0;
    b->num_on_board = 0;
    b->load = 0;
    b->num_serviced = 0;
}

// Returns 0 on success, 1 for an invalid request, and -ENOMEM if kmalloc fails
int add_pet(struct building *b, int start_floor, int dest_floor, int type)
{
    struct pet *p;

    if (!valid_floor(start_floor) || !valid_floor(dest_floor))
        return 1;
    if (start_floor == dest_floor)
        return 1;
    if (type < 0 || type >= NUM_PET_TYPES)
        return 1;

    p = kmalloc(sizeof(*p), GFP_KERNEL);
    if (!p)
        return -ENOMEM;

    p->type = type;
    p->start_floor = start_floor;
    p->dest_floor = dest_floor;
    p->weight = pet_weights[type];

    // Tail insert keeps each floor for FIFO order
    list_add_tail(&p->list, &b->floors[start_floor - 1]);
    b->num_waiting++;
    return 0;
}

// boards pets from the front of the floor's line until one doesn't fit.
// returns how many boarded.
int load_pets(struct building *b, int floor)
{
    struct pet *p, *next;
    int boarded = 0;

    if (!valid_floor(floor))
        return 0;

    // macro for cleaner code
    list_for_each_entry_safe(p, next, &b->floors[floor - 1], list) {
        // FIFO: stop at the first pet that doesn't fit, nobody cuts in line
        if (b->num_on_board >= MAX_PETS || b->load + p->weight > MAX_WEIGHT)
            break;

        list_move_tail(&p->list, &b->elevator);
        b->num_waiting--;
        b->num_on_board++;
        b->load += p->weight;
        boarded++;
    }
    return boarded;
}

// Removes and frees every pet on board whose destination is this floor.
// Returns how many got off.
int unload_pets(struct building *b, int floor)
{
    struct pet *p, *next;
    int unloaded = 0;

    // _safe because p is freed inside the loop
    list_for_each_entry_safe(p, next, &b->elevator, list) {
        if (p->dest_floor != floor)
            continue;

        list_del(&p->list);
        b->num_on_board--;
        b->load -= p->weight;
        b->num_serviced++;
        kfree(p);
        unloaded++;
    }
    return unloaded;
}

// Number of pets waiting on this floor.
int floor_count(struct building *b, int floor)
{
    if (!valid_floor(floor))
        return 0;
    return list_count_nodes(&b->floors[floor - 1]);
}

// True if the pet at the front of this floor's line would fit right now.
bool next_pet_fits(struct building *b, int floor)
{
    struct pet *p;

    if (!valid_floor(floor))
        return false;
    p = list_first_entry_or_null(&b->floors[floor - 1], struct pet, list);
    return p && b->num_on_board < MAX_PETS && b->load + p->weight <= MAX_WEIGHT;
}

// True if anyone on board wants to get off at this floor.
bool has_dropoff(struct building *b, int floor)
{
    struct pet *p;

    list_for_each_entry(p, &b->elevator, list) {
        if (p->dest_floor == floor)
            return true;
    }
    return false;
}

// Writes " C5 D2 P4" (type letter + destination) into buf. scnprintf returns
// what it actually wrote, so len can never run past the end of buf.
static int print_list(struct list_head *pets, char *buf, size_t size)
{
    struct pet *p;
    int len = 0;

    if (size == 0)
        return 0;
    buf[0] = '\0';
    list_for_each_entry(p, pets, list) {
        len += scnprintf(buf + len, size - len, " %c%d",
                         pet_letters[p->type], p->dest_floor);
    }
    return len;
}

int print_floor(struct building *b, int floor, char *buf, size_t size)
{
    if (!valid_floor(floor)) {
        if (size > 0)
            buf[0] = '\0';
        return 0;
    }
    return print_list(&b->floors[floor - 1], buf, size);
}

int print_elevator(struct building *b, char *buf, size_t size)
{
    return print_list(&b->elevator, buf, size);
}

static void free_list(struct list_head *pets)
{
    struct pet *p, *next;

    list_for_each_entry_safe(p, next, pets, list) {
        list_del(&p->list);
        kfree(p);
    }
}

// Frees every pet on every list. num_serviced is kept since it's a running total.
void free_all_pets(struct building *b)
{
    int i;

    for (i = 0; i < NUM_FLOORS; i++)
        free_list(&b->floors[i]);
    free_list(&b->elevator);

    b->num_waiting = 0;
    b->num_on_board = 0;
    b->load = 0;
}
