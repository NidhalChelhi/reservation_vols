 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 
 #define MAX_BUFFER 1024
 #define SERVER_IP "192.168.229.142" // TODO: Change to the server's IP address
 #define PORT 8080
 
 /**
  * Displays the menu for agency operations
  */
 void displayMenu() {
     printf("\n==== MENU AGENCE DE VOYAGE ====\n");
     printf("1. Consulter la liste des vols\n");
     printf("2. Consulter les informations d'un vol spécifique\n");
     printf("3. Effectuer une réservation\n");
     printf("4. Annuler une réservation\n");
     printf("5. Consulter ma facture\n");
     printf("6. Consulter l'historique des transactions\n");
     printf("7. Quitter\n");
     printf("Choix: ");
 }
 
 /**
  * Main function - Connects to server and handles user input
  */
 int main(int argc, char *argv[]) {
     int sock = 0;
     struct sockaddr_in serv_addr;
     char buffer[MAX_BUFFER] = {0};
     char message[MAX_BUFFER] = {0};
     int agencyRef;
     
     // If agency reference is not provided, prompt for it
     if (argc != 2) {
         printf("Enter agency reference: ");
         scanf("%d", &agencyRef);
         getchar(); // Clear input buffer
     } else {
         agencyRef = atoi(argv[1]);
     }
     
     if (agencyRef <= 0) {
         printf("Agency reference must be a positive integer\n");
         return EXIT_SUCCESS;EXIT_FAILURE;
     }
 
     // Create socket
     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
         printf("Socket creation error\n");
         return EXIT_FAILURE;
     }
 
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_port = htons(PORT);
 
     // Convert IP address from text to binary form
     if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
         printf("Invalid address / Address not supported\n");
         return EXIT_FAILURE;
     }
 
     // Connect to server
     printf("Connecting to server at %s:%d...\n", SERVER_IP, PORT);
     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
         printf("Connection Failed\n");
         return EXIT_FAILURE;
     }
 
     // Send agency reference
     sprintf(message, "AGENCY:%d", agencyRef);
     printf("Sending identification: %s\n", message);
     send(sock, message, strlen(message), 0);
     
     // Receive and display welcome message
     int bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
     if (bytes_received <= 0) {
         printf("Server disconnected or error occurred\n");
         close(sock);
         return EXIT_FAILURE;
     }
     buffer[bytes_received] = '\0';
     printf("Server response: %s\n", buffer);
 
     int choice;
     int running = 1;
 
     while (running) {
         displayMenu();
         scanf("%d", &choice);
         getchar(); // Clear input buffer
 
         switch (choice) {
             case 1: {
                 // Get all flights
                 strcpy(message, "GETFLIGHTS");
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== LISTE DES VOLS ===\n%s\n", buffer);
                 break;
             }
 
             case 2: {
                 // Get specific flight
                 int flightRef;
                 printf("Entrez la référence du vol: ");
                 scanf("%d", &flightRef);
                 getchar(); // Clear input buffer
                 
                 sprintf(message, "GETFLIGHT:%d", flightRef);
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== INFORMATION VOL ===\n%s\n", buffer);
                 break;
             }
 
             case 3: {
                 // Make reservation
                 int flightRef, nbPlaces;
                 printf("Entrez la référence du vol: ");
                 scanf("%d", &flightRef);
                 printf("Entrez le nombre de places à réserver: ");
                 scanf("%d", &nbPlaces);
                 getchar(); // Clear input buffer
                 
                 sprintf(message, "RESERV:%d,%d", flightRef, nbPlaces);
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== RÉSULTAT DE LA RÉSERVATION ===\n%s\n", buffer);
                 break;
             }
 
             case 4: {
                 // Cancel reservation
                 int flightRef, nbPlaces;
                 printf("Entrez la référence du vol: ");
                 scanf("%d", &flightRef);
                 printf("Entrez le nombre de places à annuler: ");
                 scanf("%d", &nbPlaces);
                 getchar(); // Clear input buffer
                 
                 sprintf(message, "CANCEL:%d,%d", flightRef, nbPlaces);
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== RÉSULTAT DE L'ANNULATION ===\n%s\n", buffer);
                 break;
             }
 
             case 5: {
                 // Get invoice
                 strcpy(message, "GETINVOICE");
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== FACTURE ===\n%s\n", buffer);
                 break;
             }
 
             case 6: {
                 // Get transaction history
                 strcpy(message, "GETHISTORY");
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received <= 0) {
                     printf("Server disconnected or error occurred\n");
                     running = 0;
                     break;
                 }
                 buffer[bytes_received] = '\0';
                 printf("\n=== HISTORIQUE DES TRANSACTIONS ===\n%s\n", buffer);
                 break;
             }
 
             case 7: {
                 // Quit
                 strcpy(message, "QUIT");
                 send(sock, message, strlen(message), 0);
                 bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
                 if (bytes_received > 0) {
                     buffer[bytes_received] = '\0';
                     printf("%s\n", buffer);
                 }
                 printf("Disconnecting from server...\n");
                 running = 0;
                 break;
             }
 
             default:
                 printf("Choix invalide. Veuillez réessayer.\n");
         }
     }
 
     close(sock);
     return 0;
}
