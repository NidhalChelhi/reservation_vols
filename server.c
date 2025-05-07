 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 
 #define MAX_BUFFER 1024
 #define PORT 8080
 #define MAX_CLIENTS 10
 
 // Global mutex for file access synchronization
 pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
 
 /**
  * Flight structure to represent flight data
  */
 typedef struct {
     int reference;
     char destination[50];
     int nbPlaces;
     float price;
 } Flight;
 
 /**
  * Transaction structure to represent reservation operations
  */
 typedef struct {
     int flightRef;
     int agencyRef;
     char operation[20];  // "Demande" or "Annulation"
     int value;
     char result[20];     // "succès" or "impossible"
 } Transaction;
 
 /**
  * Invoice structure to represent agency billing
  */
 typedef struct {
     int agencyRef;
     float totalAmount;
 } Invoice;
 
 /**
  * Thread argument structure for passing connection data
  */
 typedef struct {
     int client_socket;
     struct sockaddr_in client_address;
 } ThreadArgs;
 
 /**
  * Initializes flight data file if it doesn't exist
  */
 void initializeFlightsFile() {
     FILE *file = fopen("vols.txt", "r");
     if (file == NULL) {
         file = fopen("vols.txt", "w");
         if (file == NULL) {
             perror("Error creating flights file");
             exit(EXIT_FAILURE);
         }
         fprintf(file, "Référence Vol Destination Nombre Places Prix Place\n");
         fprintf(file, "1000 Paris 20 500\n");
         fprintf(file, "2000 Medina 10 2500\n");
         fprintf(file, "3000 Montréal 40 3500\n");
         fprintf(file, "4000 Dubaï 15 3000\n");
         fclose(file);
     } else {
         fclose(file);
     }
 }
 
 /**
  * Initializes history file if it doesn't exist
  */
 void initializeHistoryFile() {
     FILE *file = fopen("histo.txt", "r");
     if (file == NULL) {
         file = fopen("histo.txt", "w");
         if (file == NULL) {
             perror("Error creating history file");
             exit(EXIT_FAILURE);
         }
         fprintf(file, "Référence Vol Agence Transaction Valeur Résultat\n");
         fclose(file);
     } else {
         fclose(file);
     }
 }
 
 /**
  * Initializes invoice file if it doesn't exist
  */
 void initializeInvoiceFile() {
     FILE *file = fopen("facture.txt", "r");
     if (file == NULL) {
         file = fopen("facture.txt", "w");
         if (file == NULL) {
             perror("Error creating invoice file");
             exit(EXIT_FAILURE);
         }
         fprintf(file, "Référence Agence Somme à payer\n");
         fclose(file);
     } else {
         fclose(file);
     }
 }
 
 /**
  * Gets flight information from the flights file
  */
 Flight getFlightInfo(int flightRef) {
     FILE *file;
     Flight flight = {0};
     char line[256];
     int found = 0;
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("vols.txt", "r");
     if (file == NULL) {
         perror("Error opening flights file");
         pthread_mutex_unlock(&file_mutex);
         return flight;
     }
 
     // Skip header line
     fgets(line, sizeof(line), file);
 
     while (fgets(line, sizeof(line), file) != NULL) {
         if (sscanf(line, "%d %s %d %f", &flight.reference, flight.destination, 
                    &flight.nbPlaces, &flight.price) == 4) {
             if (flight.reference == flightRef) {
                 found = 1;
                 break;
             }
         }
     }
 
     fclose(file);
     pthread_mutex_unlock(&file_mutex);
 
     if (!found) {
         flight.reference = 0; // Indicates flight not found
     }
 
     return flight;
 }
 
 /**
  * Updates flight information in the flights file
  */
 int updateFlightInfo(Flight flight) {
     FILE *file, *tempFile;
     char line[256];
     int found = 0;
     Flight currentFlight;
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("vols.txt", "r");
     if (file == NULL) {
         perror("Error opening flights file for update");
         pthread_mutex_unlock(&file_mutex);
         return 0;
     }
 
     tempFile = fopen("vols_temp.txt", "w");
     if (tempFile == NULL) {
         perror("Error creating temporary file");
         fclose(file);
         pthread_mutex_unlock(&file_mutex);
         return 0;
     }
 
     // Copy header
     fgets(line, sizeof(line), file);
     fprintf(tempFile, "%s", line);
 
     // Copy and update data
     while (fgets(line, sizeof(line), file) != NULL) {
         if (sscanf(line, "%d %s %d %f", &currentFlight.reference, currentFlight.destination, 
                    &currentFlight.nbPlaces, &currentFlight.price) == 4) {
             if (currentFlight.reference == flight.reference) {
                 fprintf(tempFile, "%d %s %d %.2f\n", flight.reference, flight.destination, 
                         flight.nbPlaces, flight.price);
                 found = 1;
             } else {
                 fprintf(tempFile, "%s", line);
             }
         } else {
             fprintf(tempFile, "%s", line);
         }
     }
 
     fclose(file);
     fclose(tempFile);
 
     if (found) {
         // Replace the original file with the updated one
         if (remove("vols.txt") != 0) {
             perror("Error removing original file");
             pthread_mutex_unlock(&file_mutex);
             return 0;
         }
         if (rename("vols_temp.txt", "vols.txt") != 0) {
             perror("Error renaming temporary file");
             pthread_mutex_unlock(&file_mutex);
             return 0;
         }
     } else {
         remove("vols_temp.txt");
     }
 
     pthread_mutex_unlock(&file_mutex);
     return found;
 }
 
 /**
  * Adds a transaction to the history file
  */
 void addTransaction(Transaction transaction) {
     FILE *file;
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("histo.txt", "a");
     if (file == NULL) {
         perror("Error opening history file");
         pthread_mutex_unlock(&file_mutex);
         return;
     }
 
     fprintf(file, "%d %d %s %d %s\n", transaction.flightRef, transaction.agencyRef,
             transaction.operation, transaction.value, transaction.result);
 
     fclose(file);
     pthread_mutex_unlock(&file_mutex);
 }
 
 /**
  * Gets the invoice for an agency
  */
 float getAgencyInvoice(int agencyRef) {
     FILE *file;
     Invoice invoice = {0};
     char line[256];
     int found = 0;
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("facture.txt", "r");
     if (file == NULL) {
         perror("Error opening invoice file");
         pthread_mutex_unlock(&file_mutex);
         return 0.0;
     }
 
     // Skip header line
     fgets(line, sizeof(line), file);
 
     while (fgets(line, sizeof(line), file) != NULL) {
         if (sscanf(line, "%d %f", &invoice.agencyRef, &invoice.totalAmount) == 2) {
             if (invoice.agencyRef == agencyRef) {
                 found = 1;
                 break;
             }
         }
     }
 
     fclose(file);
     pthread_mutex_unlock(&file_mutex);
 
     if (found) {
         return invoice.totalAmount;
     } else {
         return 0.0;
     }
 }
 
 /**
  * Updates or adds an invoice for an agency
  */
 void updateAgencyInvoice(int agencyRef, float amount) {
     FILE *file, *tempFile;
     char line[256];
     int found = 0;
     Invoice invoice;
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("facture.txt", "r");
     if (file == NULL) {
         perror("Error opening invoice file for update");
         pthread_mutex_unlock(&file_mutex);
         return;
     }
 
     tempFile = fopen("facture_temp.txt", "w");
     if (tempFile == NULL) {
         perror("Error creating temporary invoice file");
         fclose(file);
         pthread_mutex_unlock(&file_mutex);
         return;
     }
 
     // Copy header
     fgets(line, sizeof(line), file);
     fprintf(tempFile, "%s", line);
 
     // Copy and update data
     while (fgets(line, sizeof(line), file) != NULL) {
         if (sscanf(line, "%d %f", &invoice.agencyRef, &invoice.totalAmount) == 2) {
             if (invoice.agencyRef == agencyRef) {
                 fprintf(tempFile, "%d %.2f\n", agencyRef, amount);
                 found = 1;
             } else {
                 fprintf(tempFile, "%s", line);
             }
         } else {
             fprintf(tempFile, "%s", line);
         }
     }
 
     if (!found) {
         fprintf(tempFile, "%d %.2f\n", agencyRef, amount);
     }
 
     fclose(file);
     fclose(tempFile);
 
     // Replace the original file with the updated one
     if (remove("facture.txt") != 0) {
         perror("Error removing original invoice file");
         pthread_mutex_unlock(&file_mutex);
         return;
     }
     if (rename("facture_temp.txt", "facture.txt") != 0) {
         perror("Error renaming temporary invoice file");
         pthread_mutex_unlock(&file_mutex);
         return;
     }
 
     pthread_mutex_unlock(&file_mutex);
 }
 
 /**
  * Processes a reservation request
  */
 const char* processReservation(int flightRef, int agencyRef, int nbPlaces) {
     Flight flight = getFlightInfo(flightRef);
     Transaction transaction;
     float currentInvoice, transactionAmount;
 
     if (flight.reference == 0) {
         return "Vol non trouvé";
     }
 
     // Prepare transaction data
     transaction.flightRef = flightRef;
     transaction.agencyRef = agencyRef;
     strcpy(transaction.operation, "Demande");
     transaction.value = nbPlaces;
 
     // Check if enough places are available
     if (flight.nbPlaces >= nbPlaces) {
         // Update flight info
         flight.nbPlaces -= nbPlaces;
         updateFlightInfo(flight);
 
         // Update transaction result
         strcpy(transaction.result, "succès");
         addTransaction(transaction);
 
         // Update invoice
         currentInvoice = getAgencyInvoice(agencyRef);
         transactionAmount = nbPlaces * flight.price;
         updateAgencyInvoice(agencyRef, currentInvoice + transactionAmount);
 
         return "Réservation effectuée avec succès";
     } else {
         // Update transaction result
         strcpy(transaction.result, "impossible");
         addTransaction(transaction);
 
         return "Réservation impossible: pas assez de places disponibles";
     }
 }
 
 /**
  * Processes a cancellation request
  */
 const char* processCancellation(int flightRef, int agencyRef, int nbPlaces) {
     Flight flight = getFlightInfo(flightRef);
     Transaction transaction;
     float currentInvoice, transactionAmount, penaltyAmount;
 
     if (flight.reference == 0) {
         return "Vol non trouvé";
     }
 
     // Prepare transaction data
     transaction.flightRef = flightRef;
     transaction.agencyRef = agencyRef;
     strcpy(transaction.operation, "Annulation");
     transaction.value = nbPlaces;
     strcpy(transaction.result, "succès");
     addTransaction(transaction);
 
     // Update flight info
     flight.nbPlaces += nbPlaces;
     updateFlightInfo(flight);
 
     // Update invoice (deduction with penalty)
     currentInvoice = getAgencyInvoice(agencyRef);
     transactionAmount = nbPlaces * flight.price;
     penaltyAmount = 0.1 * transactionAmount; // 10% penalty
     updateAgencyInvoice(agencyRef, currentInvoice - transactionAmount + penaltyAmount);
 
     return "Annulation effectuée avec succès";
 }
 
 /**
  * Gets flight history as a string
  */
 char* getFlightHistory() {
     FILE *file;
     static char history[4096]; // Large buffer for history
     char line[256];
 
     memset(history, 0, sizeof(history));
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("histo.txt", "r");
     if (file == NULL) {
         perror("Error opening history file");
         pthread_mutex_unlock(&file_mutex);
         strcpy(history, "Erreur lors de la lecture de l'historique");
         return history;
     }
 
     // Copy all lines to the history buffer
     while (fgets(line, sizeof(line), file) != NULL) {
         if (strlen(history) + strlen(line) < sizeof(history) - 1) {
             strcat(history, line);
         } else {
             strcat(history, "...[tronqué]");
             break;
         }
     }
 
     fclose(file);
     pthread_mutex_unlock(&file_mutex);
 
     return history;
 }
 
 /**
  * Gets all flights information as a string
  */
 char* getAllFlights() {
     FILE *file;
     static char flights[4096]; // Large buffer for flights info
     char line[256];
 
     memset(flights, 0, sizeof(flights));
 
     pthread_mutex_lock(&file_mutex);
     file = fopen("vols.txt", "r");
     if (file == NULL) {
         perror("Error opening flights file");
         pthread_mutex_unlock(&file_mutex);
         strcpy(flights, "Erreur lors de la lecture des vols");
         return flights;
     }
 
     // Copy all lines to the flights buffer
     while (fgets(line, sizeof(line), file) != NULL) {
         if (strlen(flights) + strlen(line) < sizeof(flights) - 1) {
             strcat(flights, line);
         } else {
             strcat(flights, "...[tronqué]");
             break;
         }
     }
 
     fclose(file);
     pthread_mutex_unlock(&file_mutex);
 
     return flights;
 }
 
 /**
  * Handler function for client connections (runs in a separate thread)
  */
 void* handle_client(void* arg) {
     ThreadArgs* args = (ThreadArgs*)arg;
     int client_socket = args->client_socket;
     struct sockaddr_in client_address = args->client_address;
     free(args);
 
     char buffer[MAX_BUFFER];
     char response[MAX_BUFFER];
     int agencyRef = 0;
     
     printf("Client connected from %s:%d\n", 
             inet_ntoa(client_address.sin_addr), 
             ntohs(client_address.sin_port));
 
     // First message is expected to be the agency reference
     int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
     if (bytes_received <= 0) {
         close(client_socket);
         pthread_exit(NULL);
     }
     buffer[bytes_received] = '\0';
     
     printf("Received from new client: '%s'\n", buffer);
     
     // Extract agency reference
     if (strncmp(buffer, "AGENCY:", 7) == 0) {
         agencyRef = atoi(buffer + 7);
         sprintf(response, "Bienvenue, Agence %d", agencyRef);
         send(client_socket, response, strlen(response), 0);
         printf("Client identified as Agency %d\n", agencyRef);
     } else {
         strcpy(response, "Format d'identification incorrect. Déconnexion.");
         send(client_socket, response, strlen(response), 0);
         printf("Client identification failed: %s\n", buffer);
         close(client_socket);
         pthread_exit(NULL);
     }
 
     // Process client commands
     while (1) {
         bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
         if (bytes_received <= 0) {
             break;
         }
         buffer[bytes_received] = '\0';
         
         printf("Received from client %d: %s\n", agencyRef, buffer);
 
         // Process commands
         if (strncmp(buffer, "RESERV:", 7) == 0) {
             // Format: RESERV:flightRef,nbPlaces
             int flightRef, nbPlaces;
             if (sscanf(buffer + 7, "%d,%d", &flightRef, &nbPlaces) == 2) {
                 const char* result = processReservation(flightRef, agencyRef, nbPlaces);
                 strcpy(response, result);
             } else {
                 strcpy(response, "Format de réservation incorrect");
             }
         } else if (strncmp(buffer, "CANCEL:", 7) == 0) {
             // Format: CANCEL:flightRef,nbPlaces
             int flightRef, nbPlaces;
             if (sscanf(buffer + 7, "%d,%d", &flightRef, &nbPlaces) == 2) {
                 const char* result = processCancellation(flightRef, agencyRef, nbPlaces);
                 strcpy(response, result);
             } else {
                 strcpy(response, "Format d'annulation incorrect");
             }
         } else if (strcmp(buffer, "GETFLIGHTS") == 0) {
             // Return all flights
             char* flights = getAllFlights();
             strcpy(response, flights);
         } else if (strcmp(buffer, "GETHISTORY") == 0) {
             // Return transaction history
             char* history = getFlightHistory();
             strcpy(response, history);
         } else if (strcmp(buffer, "GETINVOICE") == 0) {
             // Return agency invoice
             float amount = getAgencyInvoice(agencyRef);
             sprintf(response, "Facture de l'agence %d: %.2f", agencyRef, amount);
         } 
         else if (strncmp(buffer, "GETFLIGHT:", 10) == 0) {
             // Return specific flight info
             int flightRef = atoi(buffer + 10);
             Flight flight = getFlightInfo(flightRef);
             if (flight.reference != 0) {
                 sprintf(response, "Vol %d: Destination=%s, Places=%d, Prix=%.2f", 
                         flight.reference, flight.destination, flight.nbPlaces, flight.price);
             } else {
                 sprintf(response, "Vol %d non trouvé", flightRef);
             }
         } else if (strcmp(buffer, "QUIT") == 0) {
             strcpy(response, "Au revoir, Agence");
             send(client_socket, response, strlen(response), 0);
             break;
         } else {
             strcpy(response, "Commande non reconnue");
         }
 
         send(client_socket, response, strlen(response), 0);
     }
 
     printf("Client %d disconnected\n", agencyRef);
     close(client_socket);
     pthread_exit(NULL);
 }
 
 /**
  * Main function - Initializes server and handles connections
  */
 int main() {
     int server_socket, client_socket;
     struct sockaddr_in server_address, client_address;
     pthread_t thread_id;
     socklen_t client_address_len = sizeof(client_address);
     int opt = 1;
 
     // Initialize data files
     initializeFlightsFile();
     initializeHistoryFile();
     initializeInvoiceFile();
 
     // Create socket
     server_socket = socket(AF_INET, SOCK_STREAM, 0);
     if (server_socket < 0) {
         perror("Socket creation failed");
         exit(EXIT_FAILURE);
     }
 
     // Set socket options to reuse address and port
     if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
         perror("Setsockopt failed");
         exit(EXIT_FAILURE);
     }
 
     // Configure server address
     server_address.sin_family = AF_INET;
     server_address.sin_addr.s_addr = INADDR_ANY;
     server_address.sin_port = htons(PORT);
 
     // Bind socket to address
     if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
         perror("Bind failed");
         exit(EXIT_FAILURE);
     }
 
     // Listen for connections
     if (listen(server_socket, MAX_CLIENTS) < 0) {
         perror("Listen failed");
         exit(EXIT_FAILURE);
     }
 
     printf("Server started on port %d\n", PORT);
     printf("Waiting for connections...\n");
 
     // Accept connections and create threads to handle them
     while (1) {
         client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
         if (client_socket < 0) {
             perror("Accept failed");
             continue;
         }
 
         // Create thread arguments
         ThreadArgs* args = malloc(sizeof(ThreadArgs));
         args->client_socket = client_socket;
         args->client_address = client_address;
 
         // Create thread to handle client
         if (pthread_create(&thread_id, NULL, handle_client, (void*)args) != 0) {
             perror("Thread creation failed");
             close(client_socket);
             free(args);
         } else {
             // Detach thread so resources are automatically released when thread terminates
             pthread_detach(thread_id);
         }
     }
 
     close(server_socket);
     return 0;
 }
