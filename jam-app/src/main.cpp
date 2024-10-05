#include "logging.h"
#include "utils.h"

#include <chrono>
#include <thread>

#include "Game_App.h"

extern Game_App g_game;

int main(int argc, char** argv) {
    start_logging(LOG_LEVEL_ENUM::LOG_LEVEL_DEBUG);

    if (!g_game.Run()) {
        log_error("Game_App failed on Run().");
    } else {
        log_info("Clean shutdown.");
    }

	// End the program
    stop_logging();
    return 0;
}