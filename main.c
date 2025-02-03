#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>

#define bufferSize 10000
#define bufferSizeTask 1000
#define bufferSizeAnagrams 1000000


// mpicc -o output ./main.c -lm

#define MAX_ANAGRAMS 500   // Limit the number of anagrams to avoid excessive memory usage
MPI_Datatype result_type;
int anagramsNumber;

typedef struct {
    int index;
    char** anagrams;  // Pointer to array of strings (anagrams)
} AnagramData;

typedef struct{
    int primesNumber;
    int primesDiv;
    int anagramsNumber;
    char anagrams[bufferSizeAnagrams];
    char extraInfo[bufferSizeTask];  // New field to store additional information
}Result;

// Check if a number is prime
int isPrime(int n) {
    if (n < 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// Count primes up to N
int countPrimes(int N) {
    int count = 0;
    for (int i = 2; i <= N; i++) {
        if (isPrime(i)) count++;
    }
    return count;
}

void initAnagramData(AnagramData* data) {
    data->index = 0;
    data->anagrams = NULL;  // Initially set to NULL
}

void swap(char* a, char* b) {
    char temp = *a;
    *a = *b;
    *b = temp;
}

bool shouldSwap(char* str, int start, int curr) {
    for (int i = start; i < curr; i++) {
        if (str[i] == str[curr]) {
            return false;
        }
    }
    return true;
}

void permuteRec(char* str, int idx, int n, Result* data) {
    if (idx == n - 1) {
        if (data->anagramsNumber >= MAX_ANAGRAMS) {
            return;
        }

        int currentLength = strlen(data->anagrams);
        int newAnagramLength = strlen(str) + 1; 
        
        if (currentLength + newAnagramLength >= bufferSizeAnagrams) {
            return; 
        }

        strcat(data->anagrams, str);
        strcat(data->anagrams, "\n");
        data->anagramsNumber++;
        return;
    }

    for (int i = idx; i < n; i++) {
        if (!shouldSwap(str, idx, i)) {
            continue;
        }

        swap(&str[idx], &str[i]);

        permuteRec(str, idx + 1, n, data);

        swap(&str[idx], &str[i]);
    }
}

void permute(char* str, Result* data) {

    int n = strlen(str);
    data->anagrams[0] = '\0'; // Inițializează buffer-ul pentru anagrame
    data->anagramsNumber = 0;
    permuteRec(str, 0, n, data);
}

// Count prime divisors of N
int countPrimeDivisors(int N) {
    int count = 0;
    for (int i = 1; i <= N; i++) {
        if (N % i == 0 && isPrime(i)) {
            count++;
        }
    }
    return count;
}

void initResults(Result* results){
    results->primesDiv = -1;
    results->primesNumber = -1;
    // AnagramData ad;
    // initAnagramData(&ad);
    results->anagramsNumber = -1;
}

void createResultType() {
    int blockcounts[5] = {1, 1, 1, bufferSizeAnagrams, bufferSizeTask};  // One block for each field, and bufferSizeTask for extraInfo
    MPI_Aint offsets[5];
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};  // Same types but different handling for extraInfo

    // Field 1: primesNumber
    offsets[0] = offsetof(Result, primesNumber);
    
    // Field 2: primesDiv
    offsets[1] = offsetof(Result, primesDiv);
    
    // Field 3: anagramData (this is a complex struct, so we need to handle it separately)
    offsets[2] = offsetof(Result, anagramsNumber);
    
    // Field 3: anagramData (this is a complex struct, so we need to handle it separately)
    offsets[3] = offsetof(Result, anagrams);
    
    // Field 5: extraInfo
    offsets[4] = offsetof(Result, extraInfo);
    
    // Create the new MPI datatype
    MPI_Type_create_struct(5, blockcounts, offsets, types, &result_type);
    MPI_Type_commit(&result_type);
}

void trimEnd(char* str) {
    if (str == NULL) return;

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

void handleTask(char* task, char* input,char* client , int rank) {
    Result results;
    results.primesNumber = -1;
    results.primesDiv = -1;
    results.anagramsNumber = -1;
    results.extraInfo[0] = '\0';
    results.anagrams[0] = '\0';

    if (strcmp(task, "PRIMES") == 0) {
        int N = atoi(input);
        int result = countPrimes(N);
        // printf("Worker %d: PRIMES %d -> %d primes found.", rank, N, result);
        results.primesNumber = result;
    } else if (strcmp(task, "PRIMEDIVISORS") == 0) {
        int N = atoi(input);
        int result = countPrimeDivisors(N);
        // printf("Worker %d: PRIMEDIVISORS %d -> %d prime divisors.", rank, N, result);
        results.primesDiv = result;
    } else if (strcmp(task, "ANAGRAMS") == 0) {
        trimEnd(input);
        permute(input, &results);
    } else {
        printf("Worker %d: Unknown task '%s'.", rank, task);
        fflush(stdout);
    }
    sprintf(results.extraInfo, "%s %s %s", client, task, input);
    results.extraInfo[bufferSizeTask - 1] = '\0';  
    //finish time

    MPI_Send(&results, 1, result_type, 0, 0, MPI_COMM_WORLD);
}

void getFormattedTime(char* buffer, size_t buffer_size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);  // Get the current time

    // Convert the time into hours, minutes, seconds, and milliseconds
    int hours = tv.tv_sec / 3600;
    int minutes = (tv.tv_sec % 3600) / 60;
    int seconds = tv.tv_sec % 60;
    int milliseconds = tv.tv_usec / 1000;  // Convert microseconds to milliseconds

    // Format the time as hh:mm:ss:ms
    snprintf(buffer, buffer_size, "%02d:%02d:%02d:%03d", hours, minutes, seconds, milliseconds);
}

// Function to log the time in hh:mm:ss:ms format to a file
void logTime(FILE* logFile, const char* message) {
    char timeBuffer[16];  // Buffer to hold the formatted time
    getFormattedTime(timeBuffer, sizeof(timeBuffer));  // Get the formatted time

    // Log the message with the current time into the log file
    fprintf(logFile, "%s: %s\n", message, timeBuffer);
    fflush(logFile);
}

int main(int argc, char** argv) {
    int serversCount, rank;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &serversCount);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    createResultType();  // Create the custom MPI datatype for Result

    if (rank == 0) {
        FILE* file = fopen("commandFile", "r");
        if (!file) {
            perror("Error opening command file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        FILE* logFile = fopen("log.txt", "w");
        if (!logFile) {
            perror("Error opening log file");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char line[bufferSize];
        char resultMessage[bufferSize];

        while (fgets(line, bufferSize, file) != NULL) {
            if (strlen(line) <= 2) {
                continue;
            }

            //receive time
            logTime(logFile,"MASTER: Command received");

            if (strstr(line, "WAIT")) {
                int waitTime;
                sscanf(line, "WAIT %d", &waitTime);
                printf("MASTER: Waiting for %d seconds...\n", waitTime);
                fflush(stdout);
                sleep(waitTime);
                continue;
            }

            char receivedMessage[bufferSize];
            MPI_Recv(receivedMessage, bufferSize, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            //dispatch
            logTime(logFile,"MASTER: Command dispatched");
            MPI_Send(line, strlen(line) + 1, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);

            Result result;
            MPI_Recv(&result, 1, result_type, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
            logTime(logFile,"MASTER: Command finished");
            printf("MASTER received result from worker %d:\n", status.MPI_SOURCE);
            printf("Received extraInfo: %s\n", result.extraInfo);
            fflush(stdout);
            if (result.primesNumber > 0) {
                printf("Primes Count: %d\n", result.primesNumber);
                fflush(stdout);
            }

            if (result.primesDiv > 0) {
                printf("Prime Divisors Count: %d\n", result.primesDiv);
                fflush(stdout);
            }

            if (result.anagramsNumber> 0) {
                printf("Anagram Count: %d\n", result.anagramsNumber);
                fflush(stdout);
                // printf("HH:%s\n",result.anagrams);
            }
            char* client = strtok(strdup(result.extraInfo), " ");
            char* task = strtok(NULL, " ");
            char* input = strtok(NULL, "\n");
            char filename[256];
            snprintf(filename, sizeof(filename), "%s.txt", client); 

            FILE* outputFile = fopen(filename, "w");
            if (outputFile == NULL) {
                perror("Error opening file");
                continue;
            }

            fprintf(outputFile,"%s\n",result.extraInfo);

            if (result.primesNumber > 0) {
                fprintf(outputFile, "Primes Count: %d\n", result.primesNumber);
            }

            if (result.primesDiv > 0) {
                fprintf(outputFile, "Prime Divisors Count: %d\n", result.primesDiv);
            }

            if (result.anagramsNumber > 0) {
                fprintf(outputFile, "%s", result.anagrams);
            }

            fclose(outputFile);
        }

        for (int i = 1; i < serversCount; i++) {
            MPI_Send("STOP", 5, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }

        fclose(file);
    } else {
        char* receivedMessage = (char*)malloc(bufferSize * sizeof(char));
        if (receivedMessage == NULL) {
            perror("Failed to allocate memory for received message");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char message[bufferSize];

        while (1) {
            MPI_Send(message, strlen(message) + 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

            MPI_Recv(receivedMessage, bufferSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

            if (strcmp(receivedMessage, "STOP") == 0) {
                free(receivedMessage);
                break;
            }

            char* client = strtok(receivedMessage, " ");
            char* task = strtok(NULL, " ");
            char* input = strtok(NULL, "\n");

            if (task && input) {
                handleTask(task, input, client, rank);
            } else {
                printf("Worker %d received invalid task format.\n", rank);
                fflush(stdout);
            }

            sleep(2); 
        }
    }

    MPI_Type_free(&result_type);
    MPI_Finalize();
    return 0;
}
