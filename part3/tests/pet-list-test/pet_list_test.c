// Standalone test for Part 3d (pet lists). Runs on insmod, frees on rmmod.
// make && sudo insmod pet_list_test.ko && sudo rmmod pet_list_test
// sudo dmesg | grep pet_list_test

#include <linux/module.h>
#include <linux/init.h>
#include "../../src/pet_list.c"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Standalone test for the pet lists (Part 3d)");

#define BUF_SIZE 256

static struct building b;
static char buf[BUF_SIZE];
static int failures;

static void check(bool ok, const char *what)
{
    if (!ok)
        failures++;
    pr_info("pet_list_test: %s: %s\n", ok ? "PASS" : "FAIL", what);
}

// Prints the building the same way /proc/elevator will
static void show(const char *label)
{
    int f;

    pr_info("pet_list_test: --- %s ---\n", label);
    print_elevator(&b, buf, BUF_SIZE);
    pr_info("pet_list_test: elevator:%s (%d pets, %d lbs)\n",
            buf, b.num_on_board, b.load);
    for (f = NUM_FLOORS; f >= 1; f--) {
        print_floor(&b, f, buf, BUF_SIZE);
        pr_info("pet_list_test: floor %d: %d%s\n", f, floor_count(&b, f), buf);
    }
    pr_info("pet_list_test: waiting %d, serviced %d\n", b.num_waiting, b.num_serviced);
}

static int __init pet_list_test_init(void)
{
    building_init(&b);

    // Bad requests return 1 and allocate nothing
    check(add_pet(&b, 6, 2, PUG) == 1, "floor 6 rejected (producer sends 1-6)");
    check(add_pet(&b, 1, 0, PUG) == 1, "floor 0 rejected");
    check(add_pet(&b, 3, 3, PUG) == 1, "same start and dest rejected");
    check(add_pet(&b, 1, 2, 4) == 1, "type 4 rejected");
    check(add_pet(&b, 1, 2, -1) == 1, "type -1 rejected");
    check(b.num_waiting == 0, "bad requests added nothing");

    // Floor 1 line: D5 P3 D2 H4 C5 (59 lbs total)
    add_pet(&b, 1, 5, DACHSHUND);
    add_pet(&b, 1, 3, PUG);
    add_pet(&b, 1, 2, DACHSHUND);
    add_pet(&b, 1, 4, PUGHUAHUA);
    add_pet(&b, 1, 5, CHIHUAHUA);
    check(floor_count(&b, 1) == 5 && b.num_waiting == 5, "5 pets waiting on floor 1");
    show("5 pets added on floor 1");

    // D5 + P3 + D2 = 46 lbs. H4 would make 56, so loading stops and C5 can't cut.
    check(load_pets(&b, 1) == 3, "3 pets board");
    check(b.load == 46, "load is 46 lbs");
    check(floor_count(&b, 1) == 2, "H4 and C5 still waiting (FIFO)");
    check(!next_pet_fits(&b, 1), "next pet (H, 10 lbs) does not fit");
    show("loaded on floor 1");

    // Floor 2: D2 gets off
    check(has_dropoff(&b, 2), "someone wants floor 2");
    check(!has_dropoff(&b, 4), "nobody on board wants floor 4");
    check(unload_pets(&b, 2) == 1, "1 pet gets off on floor 2");
    check(b.load == 30 && b.num_serviced == 1, "load 30 lbs, 1 serviced");

    // Back on floor 1: H4 (40 lbs) and C5 (43 lbs) both fit now
    check(next_pet_fits(&b, 1), "next pet fits now");
    check(load_pets(&b, 1) == 2, "H4 and C5 board");
    check(b.num_on_board == 4 && b.load == 43, "4 pets, 43 lbs");

    // Pet limit: two chihuahuas on floor 3, only one fits before the 5-pet max
    add_pet(&b, 3, 1, CHIHUAHUA);
    add_pet(&b, 3, 1, CHIHUAHUA);
    check(load_pets(&b, 3) == 1, "only 1 boards (5 pet max)");
    check(b.num_on_board == MAX_PETS, "elevator full at 5 pets");
    show("pet limit reached");

    // Floor 5: D5 and C5 get off
    check(unload_pets(&b, 5) == 2, "2 pets get off on floor 5");
    show("unloaded on floor 5");

    pr_info("pet_list_test: %d failures. 4 pets left for rmmod to free.\n", failures);
    return 0;
}

static void __exit pet_list_test_exit(void)
{
    free_all_pets(&b);
    check(b.num_waiting == 0 && b.num_on_board == 0 && b.load == 0, "counters reset");
    check(list_empty(&b.elevator) && floor_count(&b, 3) == 0, "lists empty after free");
}

module_init(pet_list_test_init);
module_exit(pet_list_test_exit);
