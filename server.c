#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_CLIENTS 256
#define PORT 8080
#define BUFF_SIZE 4096

typedef enum{ // Client states
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} state_c;

typedef struct{ // Struct to store clients
    int fd;
    state_c state;
    char buffer[4096];
} client_state_t;

client_state_t client_states[MAX_CLIENTS];

void init_clients(){ // Initialize client struct array
    int i;

    for(i = 0; i < MAX_CLIENTS; i++){
        client_states[i].fd = -1;
        client_states[i].state = STATE_NEW;
        memset(&client_states[i].buffer, '\0', BUFF_SIZE);
    }
}

int find_free_slot(){ // Finding free slot in the client struct array
    int i;

    for(i = 0; i < MAX_CLIENTS; i++){
        if(client_states[i].fd == -1){
            return i;
        }
    }
    return -1;
}

int find_slot_by_fd(int fd){ // Finding given fd's slot
    int i;

    for(i = 0; i < MAX_CLIENTS; i++){
        if(client_states[i].fd == fd){
            return i;
        }
    }
    return -1;
}

int main(){
    int listen_fd, conn_fd, free_slot; // Fds to store listening socket and connection socket
    struct sockaddr_in server_addr, client_addr; // Structs to store adresses
    socklen_t client_len = sizeof(client_addr);

    struct pollfd fds[MAX_CLIENTS + 1]; // Pollfd array to use with poll
    int nfds; // Count of clients we need to handle
    int opt = 1;
    int i, j;
    int fd, slot;
    ssize_t bytes_read; // Storing data read from client buffer

    init_clients(); // Initializing client_states array

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){ // Creating a listening socket
        perror("socket");
        return -1;
    }

    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){ // Changing a socket option to debug faster (not necessary)
        perror("setsocketopt");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr)); // Zeroing server address variable

    server_addr.sin_family = AF_INET; // Going to use IPv4 format as adressing
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accepting all connections
    server_addr.sin_port = htons(PORT); // Packaging port in Network Byte Order

    if(bind(listen_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){ // Binding listening socket
        perror("bind");
        return -1;
    }

    if(listen(listen_fd, 10) == -1){
        perror("listen");
        return -1;
    }

    printf("Server listening on port %d\n", PORT);

    memset(fds, 0, sizeof(fds)); // Zeroing pollfd array
    fds[0].fd = listen_fd; // First element of pollfd array is our listening socket
    fds[0].events = POLLIN;
    nfds = 1;

    while(1){

        j = 1;
        for(i = 0; i < MAX_CLIENTS; i++){ // Adding active clients to pollfd array
            if(client_states[i].fd != -1){
                fds[j].fd = client_states[i].fd;
                fds[j].events = POLLIN;
                j++;
            }
        }

        int n_events = poll(fds, nfds, -1); // Waiting for any event from any client
        if(n_events == -1){
            perror("poll");
            return -1;
        }

        if(fds[0].revents & POLLIN){ // If there is an event in listening socket, accepting connection
            if((conn_fd = accept(listen_fd, (struct sockaddr*) &client_addr, &client_len)) == -1){
                perror("accept");
            } else{
                  printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                  free_slot = find_free_slot();

                  if(free_slot == -1){
                    printf("Server is full, closing new connection\n");
                    close(conn_fd);
                  } else{
                    client_states[free_slot].fd = conn_fd; // Placing new client to pollfd array
                    client_states[free_slot].state = STATE_CONNECTED;
                    nfds++;
                    printf("Slot %d has been occupied by fd %d\n", free_slot, client_states[free_slot].fd);
                  }
            }
            n_events--;
        }

        for(i = 0; (nfds >= i) && (n_events > 0); i++){ // Finally checking clients for any event
            if(fds[i].revents & POLLIN){
                n_events--;

                fd = fds[i].fd;
                slot = find_slot_by_fd(fd);
                bytes_read = read(fd, &client_states[slot].buffer, sizeof(client_states[slot].buffer)); // Reading client buffer

                if(bytes_read <= 0){
                    close(fd);

                    if(slot == -1){
                        printf("Tried to close an fd that does not exist\n");
                    }
                    else{
                        client_states[slot].fd = -1;
                        client_states[slot].state = STATE_DISCONNECTED;
                        printf("Client disconnected or error occured\n");
                        nfds--;
                    }
                }
                else{
                    printf("Received data from client %d : %s\n", fd, client_states[slot].buffer);
                }
            }
        }
    }

    return 0;
}