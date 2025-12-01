/**
 * @file main.cpp
 * @brief LeafSense Application Entry Point
 * 
 * Initializes the complete LeafSense hydroponic monitoring system:
 * - Message queue for thread communication
 * - Database daemon for persistent storage
 * - Master controller for sensor/actuator management
 * - Qt GUI with login and main window
 * 
 * @author LeafSense Team
 * @version 1.0
 */

#include <QApplication>
#include <QDialog>
#include <QDebug>
#include <pthread.h>

#include "../include/middleware/MQueueHandler.h"
#include "../include/application/gui/mainwindow.h"
#include "../include/application/gui/login_dialog.h"
#include "../include/application/gui/theme/theme_manager.h"
#include "../include/middleware/MQueueHandler.h"
#include "../include/middleware/dDatabase.h"
#include "../include/middleware/Master.h"

/* ============================================================================
 * Global System Components
 * ============================================================================ */

Master* systemMaster = nullptr;         ///< Main system controller
dDatabase* dbDaemon = nullptr;          ///< Database daemon
MQueueHandler* mqueueToDB = nullptr;    ///< Message queue handler
pthread_t tDatabase;                    ///< Database thread

/* ============================================================================
 * Thread Entry Points
 * ============================================================================ */

/**
 * @brief Database daemon thread function
 * @param arg Pointer to dDatabase instance
 * @return NULL on completion
 */
void* dbDaemonFunc(void* arg) 
{
    ((dDatabase*)arg)->run();
    return NULL;
}

/* ============================================================================
 * System Cleanup
 * ============================================================================ */

/**
 * @brief Performs graceful shutdown of all system components
 * 
 * Stops the master controller and database daemon, joins threads,
 * and releases all allocated resources.
 */
void performCleanup() 
{
    qDebug() << "[System] Stopping backend services...";
    
    if (systemMaster) systemMaster->stop();
    if (dbDaemon) dbDaemon->stop();
    
    pthread_join(tDatabase, NULL);
    
    delete systemMaster; 
    delete dbDaemon; 
    delete mqueueToDB;
    
    qDebug() << "[System] Cleanup done.";
}

/* ============================================================================
 * Application Entry Point
 * ============================================================================ */

/**
 * @brief Main function - Application entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 = success)
 */
int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);
    app.setApplicationName("LeafSense");

    // -------------------------------------------------------------------------
    // 1. Initialize Backend Services
    // -------------------------------------------------------------------------
    
    // Create message queue for inter-thread communication
    mqueueToDB = new MQueueHandler();
    
    // Start database daemon thread
    dbDaemon = new dDatabase(mqueueToDB, "leafsense.db"); 
    pthread_create(&tDatabase, NULL, dbDaemonFunc, (void*)dbDaemon);
    
    // Start master controller (manages sensors and actuators)
    systemMaster = new Master(mqueueToDB);
    systemMaster->start(); 

    // -------------------------------------------------------------------------
    // 2. Initialize GUI
    // -------------------------------------------------------------------------
    
    ThemeManager::instance().set_theme(ThemeMode::LIGHT);

    // Main application loop with login/logout support
    while (true) {
        LoginDialog login;
        
        if (login.exec() == QDialog::Accepted) {
            // User logged in successfully
            MainWindow* w = new MainWindow();
            w->set_logged_in_user(login.get_username());
            w->set_login_time(login.get_login_time());
            
            // Default plant selection
            Plant p; 
            p.id = 1; 
            p.name = "Lettuce"; 
            w->set_selected_plant(p);
            w->show();
            
            int res = app.exec();
            delete w;
            
            // Exit if not returning to login
            if (res != 0) { 
                performCleanup(); 
                return res; 
            }
        } else {
            // User cancelled login
            performCleanup(); 
            return 0;
        }
    }
}