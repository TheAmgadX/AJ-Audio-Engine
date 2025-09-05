#include <iostream>
#include <thread>
#include <cassert>
#include <chrono>

#include "core/buffer_pool.h"
#include "core/thread_pool.h"
#include "core/error_handler.h"
#include "core/engine_resources.h"
#include "audio_io/audio_io_manager.h"
#include "core/event_handler.h"   // IEventHandler + ConsoleRecordHandler

class AudioIOManagerRecordTests {
public:
    static void run_all() {
        std::cout << "\nRunning AudioIOManager Record Tests\n";
        std::cout << "---------------------------------------------\n";

        // test_invalid_record();
        test_valid_record();

        std::cout << "All AudioIOManager Record Tests Completed Successfully.\n";
    }

private:
    static void test_invalid_record() {
        std::cout << "\nTest: Invalid Record Setup\n";

        AJ::error::ConsoleErrorHandler errHandler;
        AJ::utils::ConsoleRecordHandler evHandler;

        // Invalid resources
        std::shared_ptr<AJ::EngineResources> brokenResources = nullptr;
        std::string sessionDir = "/tmp/session_invalid";

        AJ::io::audio_io_manager::RecordHandlers recHandlers{errHandler, evHandler};
        AJ::io::audio_io_manager::PlayHandlers playHandlers{errHandler, evHandler};


        AJ::io::audio_io_manager::AudioIOManager mgr(brokenResources, sessionDir,
                                                    recHandlers, playHandlers);
        bool failed = mgr.record();
        
        assert(!failed && "Invalid record setup did not fail as expected");
        std::cout << "  ✓ Invalid record setup correctly failed\n";
    }

    static void test_valid_record() {
        std::cout << "\nTest: Valid Record Setup\n";
        std::cout << "NOTE: This test is interactive. Press Enter to stop recording.\n";

        AJ::error::ConsoleErrorHandler errHandler;
        AJ::utils::ConsoleRecordHandler evHandler;

        // Engine resources with valid pools/queue/threadpool

        auto engineResources = std::make_shared<AJ::EngineResources>(errHandler);

        std::string sessionDir = "/home/aj-e/Programming Codes/C++/AJ-Audio-Engine/build/build/bin/session";

        AJ::io::audio_io_manager::RecordHandlers recHandlers = AJ::io::audio_io_manager::RecordHandlers(errHandler, evHandler);
        
        AJ::io::audio_io_manager::PlayHandlers playHandlers = AJ::io::audio_io_manager::PlayHandlers(errHandler, evHandler);

        
        AJ::io::audio_io_manager::AudioIOManager manager(engineResources, sessionDir, recHandlers, playHandlers);

        bool ok = manager.record();
        assert(ok && "Record should succeed with valid setup");

        std::cout << "  ✓ Valid record setup succeeded\n";
    }
};
