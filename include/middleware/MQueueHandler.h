/**
 * @file MQueueHandler.h
 * @brief Thread-safe Message Queue for Inter-Process/Thread Communication
 * @layer Middleware
 */

#ifndef MQUEUEHANDLER_H
#define MQUEUEHANDLER_H

#include <queue>
#include <string>
#include <pthread.h>
#include <iostream>

class MQueueHandler {
private:
    // The underlying container for messages
    std::queue<std::string> messageQueue;

    // POSIX synchronization primitives
    pthread_mutex_t queueMutex;
    pthread_cond_t queueCondition;

public:
    /**
     * @brief Constructor: Initializes POSIX synchronization primitives
     */
    MQueueHandler();

    /**
     * @brief Destructor: Destroys primitives and cleans up resources
     */
    ~MQueueHandler();

    /**
     * @brief Producer method: Adds a message to the queue safely
     * @param message The data payload to be processed (e.g., SQL command or log entry)
     */
    void sendMessage(std::string message);

    /**
     * @brief Consumer method: Retrieves the next message
     * Blocking call - waits if queue is empty
     * @return The message string
     */
    std::string receiveMessage();

    /**
     * @brief Utility to check if queue is empty (Thread-safe)
     */
    bool isEmpty();

    /**
     * @brief Clears the queue (Thread-safe)
     */
    void clear();
};

#endif // MQUEUEHANDLER_H