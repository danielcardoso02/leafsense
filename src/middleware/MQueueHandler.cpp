/**
 * @file MQueueHandler.cpp
 * @brief Implementation of the Thread-safe Message Queue
 */

#include "../../include/middleware/MQueueHandler.h"

// Constructor
MQueueHandler::MQueueHandler() {
    // Initialize Mutex (Default attributes)
    if (pthread_mutex_init(&queueMutex, NULL) != 0) {
        std::cerr << "[MQueue Error] Mutex init failed" << std::endl;
    }

    // Initialize Condition Variable (Default attributes)
    if (pthread_cond_init(&queueCondition, NULL) != 0) {
        std::cerr << "[MQueue Error] Condition variable init failed" << std::endl;
    }
}

// Destructor
MQueueHandler::~MQueueHandler() {
    // Clean up POSIX resources
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCondition);
    
    // Clear any remaining messages
    while (!messageQueue.empty()) {
        messageQueue.pop();
    }
}

// Producer: sendMessage
// Logic defined in Section 4.5.6: Lock -> Push -> Signal -> Unlock
void MQueueHandler::sendMessage(std::string message) {
    // 1. Acquire Lock
    pthread_mutex_lock(&queueMutex);

    // 2. Push Message
    messageQueue.push(message);

    // 3. Signal the Condition Variable (Wake up the consumer/Daemon)
    pthread_cond_signal(&queueCondition);

    // 4. Release Lock
    pthread_mutex_unlock(&queueMutex);
}

// Consumer: receiveMessage
// Logic defined in Section 4.5.6: Lock -> Wait(if empty) -> Pop -> Unlock
std::string MQueueHandler::receiveMessage() {
    std::string message = "";

    // 1. Acquire Lock
    pthread_mutex_lock(&queueMutex);

    // 2. Wait while queue is empty (Handling spurious wakeups)
    while (messageQueue.empty()) {
        // Blocks here, atomically releasing mutex, until signaled by sendMessage
        pthread_cond_wait(&queueCondition, &queueMutex);
    }

    // 3. Retrieve and Pop
    if (!messageQueue.empty()) {
        message = messageQueue.front();
        messageQueue.pop();
    }

    // 4. Release Lock
    pthread_mutex_unlock(&queueMutex);

    return message;
}

// Helper: Check if empty
bool MQueueHandler::isEmpty() {
    bool empty;
    pthread_mutex_lock(&queueMutex);
    empty = messageQueue.empty();
    pthread_mutex_unlock(&queueMutex);
    return empty;
}

// Helper: Clear queue
void MQueueHandler::clear() {
    pthread_mutex_lock(&queueMutex);
    while (!messageQueue.empty()) {
        messageQueue.pop();
    }
    pthread_mutex_unlock(&queueMutex);
}