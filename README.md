# JobDispatcher-server-clients-

# Assignment3 - Variant: Job Dispatcher

## Cerințe pentru finalizare

This variant has a maximum number of 40 points.

### Problem description

Servers are often implemented as server clusters: a group of computer servers, in order to supply server functionality beyond the capability of a single machine. In this assignment, you simulate the functionality of such a server cluster.

The main server acts as a job dispatcher. It continuously receives commands and dispatches the commands to worker servers out of his cluster. If there are no free servers at the moment, then it waits until one of them finishes its current work.

The worker servers execute the commands and send results back to the main server. The main server writes the results in files, having a file for each client. The main server also produces a log where it records the time moment when each command has been received, when it has been dispatched to a worker server, and when it has been finished.

The commands arriving at the server are simulated by a command file. A command file contains several lines, where each line may contain either a request from a client to execute a specific command, or an indication that the server must wait a period of time before reading the next request line. This wait is used to simulate a bursty arrival of client requests. The client requests can be:

- `PRIMES N` - find out how many primes there are in the first `N` natural numbers
- `PRIMEDIVISORS N` - find out how many prime divisors has the number `N`
- `ANAGRAMS name` - generates all anagrams (permutations) of `name`. LaterUpdate: it is ok to consider only names with up to 8 characters.

Commands may come in any order and in any combination. For example, a command file can be:

CLI0 PRIMEDIVISORS 452876 CLI1 ANAGRAMS tralala CLI2 PRIMEDIVISORS 129072 WAIT 2 CLI3 PRIMES 2908764 WAIT 1 CLI4 PRIMES 10000987 CLI5 PRIMES 12043876 CLI6 PRIMES 20876 CLI7 ANAGRAMS supercalifragilistic WAIT 3 CLI8 ANAGRAMS caterpillar CLI9 ANAGRAMS rainbow CLI10 PRIMES 3451629


### Requirements

Choose a good architecture for your server cluster, think very well the details of the Main server that must perform different concurrent activities, and its protocol of interactions with the other servers.

Use MPI for the implementation.

Make sure that your application works well even if it is deployed with all processes on a single machine or with processes distributed across multiple machines.

### Standard requirements: (20 points)

Implement the server cluster with job dispatcher described above. The deadline for this part is week 11. Execute performance measurements and report your results: for a long list of random commands, without `WAIT` commands inserted between them, compare the total time needed to complete the list of commands executed serially with the total time needed by executing them on the server cluster.

### Bonus features: (additional 20 points)

The deadline for this part is extended to week 13.

Some types of commands need more computational power and they will be assigned to a group of free servers to be executed in parallel. For example, MATRIX operations must be assigned to groups of servers if they operate on sizes bigger than some threshold. Extend the job dispatcher and its protocol of interaction with the other servers in order to handle this case.

Following MATRIX operations must be added to the set of supported commands:

- `MATRIXADD N filename1 filename2` - execute matrix addition of two matrices of size `N*N`, with their elements given in files
- `MATRIXMULT N filename1 filename2` - execute matrix multiplication of two matrices of size `N*N`, with their elements given in files

The filenames in these commands refer to files which are available in the local filesystem of the Main server. The results of matrix operations will be saved in files in the local filesystem of the Main server.
