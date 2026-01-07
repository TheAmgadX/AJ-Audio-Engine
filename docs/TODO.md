## TODO / Roadmap

- Integrate the custom memory manager across the entire engine.
- Audit all `std::vector` usage and eliminate unnecessary copies and moves.
- Implement playback functionality and the Audio I/O management layer.
- Design and implement the session manager and audio mixer.
- Complete the `read` implementation in `file_streamer`.
- Design and implement the undo/redo system.
- Finalize all `AJ_Engine` public APIs and resolve remaining TODOs.
- Add extensive unit, integration, and stress testing.
- Implement audio resampling using FFmpeg.
- Detect and trace memory leaks, invalid accesses, and failures using the memory manager.
- Fully document the engine architecture with proper UML diagrams.
- Apply memory alignment strategies and low-level memory optimizations.
