#include <pthread.h>
#include "../../include/middleware/MQueueHandler.h"
#include "../../include/middleware/dDatabase.h"

// Global Queue (Shared Resource)
MQueueHandler* mqueueToDB;

// Wrapper function for pthreads to run the Daemon class method
void* dbDaemonFunc(void* arg) {
    dDatabase* daemon = (dDatabase*)arg;
    daemon->run(); // Enters the infinite loop
    return NULL;
}

int main() {
    // 1. Infrastructure Initialization
    // Create Message Queue first (Dependency for threads)
    mqueueToDB = new MQueueHandler();

    // 2. Initialize Daemon Package
    // Connects to leafsense.db using dbManager internally
    dDatabase* dbDaemon = new dDatabase(mqueueToDB, "database/leafsense.db");

    // 3. Create Daemon Thread
    pthread_t tDatabase;
    if (pthread_create(&tDatabase, NULL, dbDaemonFunc, (void*)dbDaemon) != 0) {
        std::cerr << "Failed to create DB Daemon thread" << std::endl;
        return -1;
    }

    // ... Initialize Sensor Threads (tReadSensors, etc.) ...
    // Example usage from a sensor thread would be:
    // mqueueToDB->sendMessage("SENSOR|23.5|6.2|1100");

    // Wait for threads (Join)
    pthread_join(tDatabase, NULL);

    // Cleanup
    delete dbDaemon;
    delete mqueueToDB;

    return 0;
}