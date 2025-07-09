# Basic OS Shell

This C-based OS shell simulates process execution under memory constraints, using demand paging, dynamic scheduling, and Least Recently Used (LRU) page replacement. 
Programs are executed from a shell interface and dynamically mapped into a constrained virtual memory model with support for evictions, page faults, and variable management.

**Supports basic commands:**

help 
- Displays all the commands 
quit 
- Exits / terminates the shell with “Bye!” 
set VAR STRING
- Assigns a value to shell memory
- set VAR STRING first checks to see if VAR already exists.
- If it does exist, STRING overwrites the previous value assigned to VAR.
print VAR
- Displays the STRING assigned to VAR
- Print VAR first checks to see if VAR exists. If it does not exist, then it displays the error “Variable does not exist”.
- If VAR does exist, then it displays the STRING
run SCRIPT.TXT
- Executes the file SCRIPT.TXT
- Assumes that a text file exists with the provided file name, in the current directory.
- Opens that text file and then sends each line one at a time to the interpreter.
- Interpreter treats each line of text as a command.
- At end of script, file is closed, and command line prompt is showed again.
- While script executes command line prompt is not shown.
echo $VAR
- Used for displaying strings passed as arguments to the command line.
- Takes one token string as input
- If string is preceded by $, echo checks shell memory for a alphanumeric string variable following the $ symbol
- If the variable is found, echo displays the value otherwise prints an empty line and returns command line to user
Batch mode
- $ is only displayed in the interactive mode at the user command-line
- Shell always displays the mysh command prompt (entering interactive mode) after running all the instructions in the input file.
_File-system-like commands (my_ls, my_mkdir, my_touch, my_cd)_
my_mkdir dirname
- dirname can be alphanumeric string, possibly preceded by $.
- Without $ creates a new directory named dirname in the current directory.
- If dirname is an alphanumeric string preceded by $, my_mkdir checks the shell memory for a variable first.
- If variable exists and contains a single alphanumeric token, my_mkdir creates a directory using the value associated
- Else, (doesn't exist or not alphanumeric) my_mkdir gives “Bad command: my_mkdir” and returns the command prompt
my_touch filename
- creates a new empty file inside the current directory.
- filename is an alphanumeric string.
my_cd dirname
- changes current directory to directory dirname, inside the current directory.
- If dirname doesn't exist inside the current directory, my_cd displays “Bad command: my_cd” and returns command-line.

_Assumptions:_
You can assume that file/directory names are <100 characters.

**As well as some enhanced operations:**
- set supports multi-token values (up to 5), with proper error handling.
- Adding robust echo functionality with variable expansion.
- Batch mode vs Interactive (user) mode

**Adding memory model:**
- Simulated memory is divided into a frame store and variable store.
- Programs are split into fixed-size pages (3 lines per page).
- Each program maintains a page table for memory address translation.

_Demand paging:_
- Only initial pages are loaded at launch (run / exec commands).
- On page faults, pages are loaded into the next available frame.
- If memory is full, a victim page is evicted using the LRU policy.

_Page replacement with LRU:_
- Full implementation of LRU
- Tracks frame access order and ensures accurate eviction decisions
- Pages are stored in a backing store directory, simulating disk I/O.
- Clean-up operations ensure backing store consistency across runs.

_RR scheduling with paging:_
- Uses Round Robin scheduling with a time slice of 2 instructions.
- Supports executing the same script multiple times via exec.

_Compile-time parameters allow tuning of frame and variable store sizes:_
make mysh framesize=12 varmemsize=8

| **Module/File**                     | **Purpose**                                                                 | **FinTech-Relevant Skills Showcased**                                                                             |
| ----------------------------------- | --------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `shell.c` & `shell.h`               | CLI shell environment for executing programs (`run`, `exec`, `quit`)        | Command interpreter design for transaction replay, scripting batch operations, automating workflows               |
| `scheduler.c` & `scheduler.h`       | Implements **Round Robin (RR)** process scheduling                          | Models time-sliced operations and concurrent task allocation, similar to job queues in banking backends           |
| `pcb.c` & `pcb.h`                   | Manages **Process Control Blocks** tracking program state                   | Ensures reliable execution state across memory faults—critical for transaction and session handling               |
| `pagetbl.c` & `pagetbl.h`           | Implements per-process **page tables**, storing virtual-to-physical mapping | Demonstrates memory modeling and isolation logic, foundational for **risk isolation** and secure sandboxing       |
| `codestore.c` & `codestore.h`       | Handles physical memory frames (pages) and loading logic                    | Encodes low-level memory layout management, analogous to **buffer pool management** in database engines           |
| `varstore.c` & `varstore.h`         | Manages in-memory variable key-value store                                  | Mirrors in-memory cache used for rapid lookup                                           |
| `accessrecord.c` & `accessrecord.h` | Tracks memory/frame access order for **accurate LRU replacement**           | Demonstrates cache policy design & memory access pattern logging, relevant for fraud or anomaly detection systems |
| `readyqueue.c` & `readyqueue.h`     | Implements a queue of ready-to-run processes                                | Queue management logic applies to any batch job processing system (ETL pipelines, compliance checks)              |
| `interpreter.c` & `interpreter.h`   | Parses and executes simple scripting language (commands per line)           | Language parsing and control flow execution, similar to running DSLs in automated report generation               |
| `utiltypes.h`                       | Custom shared data types and constants                                      | Maintains consistency across modules, key in maintaining regulatory compliance and shared interfaces              |
| `limits.h`                          | Defines frame/variable limits; supports compile-time configuration          | Simulates constraint-bound execution—useful for **stress testing and limits modeling** in financial platforms     |

- Demand Paging ~ load code on-demand, trigger page faults, manage evicion.
- LRU Replacement ~ Eviction of the least recently used memory pages.
- Scheduling ~ RR allows independent processes scheduling.
- Page tables ~ Virtual memory abstraction per process.
- Backing store ~ Simulates disk-like structure for on-demand data retrieval.
- Variable store ~ dynamic runtime data handling per process.

**Compile and run shell with custom memory config:**

make mysh framesize=12 varmemsize=8

./mysh

**Example of accepted commands:**

exec script1 script2 RR     # Run multiple paged programs

run script3                 # Run a single paged script

quit                        # Clean shutdown and cleanup
