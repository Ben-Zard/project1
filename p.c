
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 256

typedef struct {
    int sender;
    int receiver;
    char content[MAX_MSG_LEN];
} Message;

// void sendMessage(int sender, int receiver, char* content, int pipeOut[][2], int k) {
//     Message msg;
//     msg.sender = sender;
//     msg.receiver = receiver;
//     strncpy(msg.content, content, MAX_MSG_LEN);
//     msg.content[MAX_MSG_LEN - 1] = '\0';
//     write(pipeOut[receiver][1], &msg, sizeof(Message));
//     printf("Process %d sent a message to process %d: '%s'\n", sender, receiver, content);
//     for (int i = 0; i < k; i++) {
//         if (i != sender && i != receiver) {
//             char buffer[MAX_MSG_LEN];
//             sprintf(buffer, "Process %d received a message from process %d intended for process %d but forwarding to process %d", i, sender, receiver, (i + 1) % k);
//             printf("%s\n", buffer);
//             write(pipeOut[(i + 1) % k][1], &msg, sizeof(Message));
//         }
//     }
// }
void sendMessage(int sender, int receiver, char* content, int pipeOut[][2], int k) {
   Message msg;
   msg.sender = sender;
   msg.receiver = receiver;
   strncpy(msg.content, content, MAX_MSG_LEN);
   msg.content[MAX_MSG_LEN - 1] = '\0';
   write(pipeOut[receiver][1], &msg, sizeof(Message));
   printf("Process %d sent a message to process %d: '%s'\n", sender, receiver, content);
   printf("Message content: '%s'\n", content);
   for (int i = 0; i < k; i++) {
       if (i != sender && i != receiver) {
           char buffer[MAX_MSG_LEN];
           sprintf(buffer, "Process %d received a message from process %d intended for process %d but forwarding to process %d\n", i, sender, receiver, (i + 1) % k);
           printf("%s\n", buffer);
           printf("Message content: '%s'\n", content); //right spot? 
           write(pipeOut[(i + 1) % k][1], &msg, sizeof(Message));
       }
   }
}



void receiveMessage(int pid, int pipeIn[][2], int k) {
    Message msg;
    read(pipeIn[pid][0], &msg, sizeof(Message));
    if (pid == msg.receiver) {
        printf("Process %d received a message: '%s'\n", pid, msg.content);
    } else {
        char buffer[MAX_MSG_LEN];
        sprintf(buffer, "Process %d received a message from process %d intended for process %d but forwarding to process %d\n", pid, msg.sender, msg.receiver, (pid + 1) % k);
        printf("%s\n", buffer);
        printf("Message content: '%s'\n", msg.content);
        write(pipeIn[(pid + 1) % k][1], &msg, sizeof(Message));
    }
}


// void badApple(int pid, int pipeIn[][2], int pipeOut[][2], int k) {
//     srand(time(NULL) + pid);
//     while (1) {
//         Message msg;
//         read(pipeIn[pid][0], &msg, sizeof(Message));
//         if (pid == msg.receiver) {
//             printf("Process %d received a message: '%s'\n", pid, msg.content);
//             if (rand() % 3 == 0) {
//                 printf("Process %d is a bad apple and modified the message.\n", pid);
//                 strcpy(msg.content, "I'm a bad apple!");
//             }
//             msg.receiver = (pid + 1) % k;
//             printf("Forwarding message to process %d...\n", msg.receiver);
//             printf("Message content: '%s'\n", msg.content);
//             write(pipeOut[(pid + 1) % k][1], &msg, sizeof(Message));
//         } else {
//             char buffer[MAX_MSG_LEN];
//             sprintf(buffer, "Process %d received a message from process %d intended for process %d but forwarding to process %d", pid, msg.sender, msg.receiver, (pid + 1) % k);
//             printf("%s\n", buffer);
//             printf("Message content: '%s'\n", msg.content);
//             write(pipeOut[(pid + 1) % k][1], &msg, sizeof(Message));
//         }
//     }
// }
void badApple(int pid, int pipeIn[][2], int pipeOut[][2], int k) {
   srand(time(NULL) + pid);
   while (1) {
       Message msg;
       read(pipeIn[pid][0], &msg, sizeof(Message));
       if (pid == msg.receiver) {
           printf("Process %d received a message: '%s'\n", pid, msg.content);
           if (rand() % 3 == 0) {
               printf("Process %d is a bad apple and modified the message.\n", pid);
               msg.content[0] = 'I';
               msg.content[1] = '\'';
               msg.content[2] = 'm';
               msg.content[3] = ' ';
               msg.content[4] = 'a';
               msg.content[5] = ' ';
               msg.content[6] = 'b';
               msg.content[7] = 'a';
               msg.content[8] = 'd';
               msg.content[9] = ' ';
               msg.content[10] = 'a';
               msg.content[11] = 'p';
               msg.content[12] = 'p';
               msg.content[13] = 'l';
               msg.content[14] = 'e';
               msg.content[15] = '!';
               msg.content[16] = '\0';
           }
           msg.receiver = (pid + 1) % k;
           printf("Forwarding message to process %d...\n", msg.receiver);
           printf("Message content: '%s'\n", msg.content);
           write(pipeOut[(pid + 1) % k][1], &msg, sizeof(Message));
       } else {
           char buffer[MAX_MSG_LEN];
           sprintf(buffer, "Process %d received a message from process %d intended for process %d but forwarding to process %d\n", pid, msg.sender, msg.receiver, (pid + 1) % k);
           printf("%s\n", buffer);
           printf("Message content: '%s'\n", msg.content);
           write(pipeOut[(pid + 1) % k][1], &msg, sizeof(Message));
       }
   }
}


void handler(int sig) {
    printf("Caught signal %d, shutting down...\n", sig);
    exit(0);
}

int main() {
   int k;
   printf("Enter the number of processes (k): ");
   scanf("%d", &k);
   int pipeIn[k][2], pipeOut[k][2];
   for (int i = 0; i < k; i++) {
       if (pipe(pipeIn[i]) == -1 || pipe(pipeOut[i]) == -1) {
           perror("pipe");
           exit(1);
       }

       pid_t pid = fork();
       if (pid == -1) {
           perror("fork");
           exit(1);
       } else if (pid == 0) {
           // child process
           if (i == 0) {
               // leader process
               signal(SIGINT, handler);
               printf("Process %d (the leader) is ready to send messages.\n", i);
               while (1) {
                   int receiver;
                   char content[MAX_MSG_LEN];
                   printf("Enter the ID of the receiver and the message: ");
                   scanf("%d %[^\n]", &receiver, content);
                   sendMessage(i, receiver, content, pipeOut, k);
               }
           } else {
               // regular process
               if (i == rand() % k) { // assign a node to be a bad apple
                   printf("Process %d is a bad apple.\n", i);
                   badApple(i, pipeIn, pipeOut, k);
               } else {
                   receiveMessage(i, pipeIn, k);
               }
           }
       } else {
           // parent process
           if (i == 0) {
               // leader process
               close(pipeIn[i][0]);
               close(pipeOut[i][1]);
           } else if (i == k-1) {
               // last process
               close(pipeIn[i][1]);
               close(pipeOut[i][0]);
           } else {
               // intermediate process
               close(pipeIn[i][1]);
               close(pipeOut[i][0]);
               close(pipeIn[i][0]);
               close(pipeOut[i][1]);
           }
       }
   }

   // leader process waits for Ctrl-C signal
   while (1) {
       sleep(1);
   }

   return 0;
}
