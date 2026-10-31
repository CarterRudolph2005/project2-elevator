# Elevator

A pet elevator implemented as a Linux kernel module. Three system calls (548-550) added to
kernel 6.16.12 let user programs start the elevator, issue pet requests, and stop it. A kthread
moves the elevator between floors 1-5 and a mutex protects the shared floor and elevator lists.
Status is shown in `/proc/elevator`.

## Group Members
- **Carter Rudolph**: [email]@fsu.edu
- **Terryon Larkins**: [email]@fsu.edu
- **Willa Gutowski**: [email]@fsu.edu

## Division of Labor

### Part 1: System Call Tracing
- **Responsibilities**: Write an empty C program and a copy of it with exactly five added system
  calls, then use `strace` to show the copy makes five more system calls than the empty one.
- **Assigned to**: Willa Gutowski

### Part 2: Timer Kernel Module
- **Responsibilities**: Write the `my_timer` kernel module, which creates `/proc/timer` and prints
  the current time and the time elapsed since the last read.
- **Assigned to**: Willa Gutowski

### Part 3a: Adding System Calls
- **Responsibilities**: Add system calls 548-550 (`start_elevator`, `issue_request`,
  `stop_elevator`) to the kernel. Each one forwards to a function pointer that the elevator
  module sets when it loads.
- **Assigned to**: Carter Rudolph

### Part 3b: Kernel Compilation
- **Responsibilities**: Compile and install kernel 6.16.12 with the new system calls on the group
  VM and confirm they exist.
- **Assigned to**: Carter Rudolph

### Part 3c: Threads
- **Responsibilities**: Use a kthread to run the elevator, moving it between floors and loading
  and unloading pets with the required delays.
- **Assigned to**: Terryon Larkins

### Part 3d: Linked List
- **Responsibilities**: Store waiting pets on each floor and the pets on the elevator in kernel
  linked lists. Pets are allocated with `kmalloc`, board in FIFO order, and are freed when they
  get off or when the module is removed.
- **Assigned to**: Carter Rudolph

### Part 3e: Mutexes
- **Responsibilities**: Protect the shared floor and elevator data with a mutex so the kthread,
  the system calls, and `/proc/elevator` never access it at the same time.
- **Assigned to**: Terryon Larkins

### Part 3f: Scheduling Algorithm
- **Responsibilities**: Decide which floor the elevator goes to next and when pets load and
  unload, within the 5 pet / 50 lb limits.
- **Assigned to**: Terryon Larkins

## File Listing
```
elevator/
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   └── Makefile
├── part3/
│   ├── kernel/
│   │   ├── setup.sh
│   │   └── install.sh
│   ├── src/
│   │   ├── pet_list.h
│   │   └── pet_list.c
│   ├── tests/
│   ├── Makefile
│   └── syscalls.c
├── Makefile
└── README.md
```
# How to Compile & Execute

### Requirements
- **Compiler**: `gcc`, `make`
- **Kernel**: Linux 6.16.12 with system calls 548-550 installed

## Part 1

### Compilation
```bash
make
```
This will build the `empty` and `part1` executables in `part1/`.
### Execution
```bash
make run
```
This will run both programs under `strace` and write `empty.trace` and `part1.trace`.

## Part 2

### Compilation
```bash
make
```
This will build the `my_timer.ko` kernel module in `part2/`.
### Execution
```bash
sudo insmod my_timer.ko
cat /proc/timer
sudo rmmod my_timer
```
This will load the module, print the current time and the time elapsed since the last read, and
remove the module.


## Part 3

### Compilation
```bash
make
```
This will build the `elevator.ko` kernel module in `part3/`.
### Execution
```bash
sudo insmod elevator.ko
watch -n 1 cat /proc/elevator
sudo rmmod elevator
```
This will load the module and show the elevator's state, current floor, load, and waiting pets,
updated every second. Pets are added and the elevator is started and stopped with the provided
`producer` and `consumer` programs.

## Development Log
Each member records their contributions here.

### Carter Rudolph

| Date       | Work Completed / Notes |
|------------|------------------------|
| 2026-10-28 | Reassigned the division of labor so no member depends on another's work. Reviewed the starter files. |
| 2026-10-28 | Part 3a: wrote `syscalls.c` and the kernel setup script that adds syscalls 548-550 to the table, `syscalls.h`, and the kernel Makefile. |
| 2026-10-28 | Part 3b: configured (`localmodconfig`, no debug info), compiled, and installed kernel 6.16.12 on the group VM. Set it as the GRUB default. `syscheck` test passed for all three syscalls. |
| 2026-10-29 | Part 3d: wrote `pet_list.h` and `pet_list.c` (pet creation and validation, FIFO loading, unloading, counts, print helpers, cleanup) and a standalone test module. All checks passed on the VM with no leaks after `rmmod`. |

**Kernel build notes**
- `part3/kernel/setup.sh` adds the system calls and builds the kernel. `install.sh` installs it
  and sets 6.16.12 as the GRUB default, since the VM's stock kernel (7.0) is newer and would
  boot first.
- In 6.16 the syscall table is at `arch/x86/entry/syscalls/syscall_64.tbl`, not the path in the
  spec.

**Use of AI**
- Used Claude to optimize `syscalls.c`, the kernel `setup.sh` and `install.sh` scripts,
  `pet_list.c`, and the pet list test module, and to debug kernel build errors. I wrote
  `pet_list.h`, went through `pet_list.c` function by function, ran and verified every step on
  the VM (`uname -r`, `syscheck`, the pet list test), and reviewed the code to make sure I
  understand it.

### Terryon Larkins

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |

### Willa Gutowski

| Date       | Work Completed / Notes |
|------------|------------------------|
| YYYY-MM-DD | [Description of task]  |


## Meetings
Document in-person meetings, their purpose, and what was discussed.

| Date       | Attendees            | Topics Discussed | Outcomes / Decisions |
|------------|----------------------|------------------|-----------------------|
| YYYY-MM-DD | [Names]              | [Agenda items]   | [Actions/Next steps]  |



## Bugs
- No known bugs in Parts 3a, 3b, or 3d.

## Considerations
