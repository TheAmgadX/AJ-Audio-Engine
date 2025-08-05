## 📚 Documentation Overview

This project is actively under development. Below is an overview of the documentation structure to help you navigate the core concepts, systems, and implementation details.

| **File Name**          | **Description**                                                                                                                                                          |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Architecture.md**    | Explains the high-level system architecture, including the major modules, their responsibilities, and how they interact. Useful for understanding the big picture.       |
| **AudioProcessing.md** | Documents the DSP chain, effects system, and audio processing strategies (e.g., echo, reverb, multi-threading).                                                          |
| **benchmarks/**        | Contains benchmark results and visualizations. For example, `plot_echo_benchmarks.png` shows performance comparisons for echo effects.                                   |
| **ErrorHandling.md**   | Outlines the error handling strategy. Describes the flexible error handling system based on the Strategy Pattern, allowing users to plug in their own handling behavior. |
| **FileIO.md**          | Describes the `AudioFile` abstraction and how different formats (e.g., WAV via libsndfile, MP3 via FFmpeg) are loaded, decoded, and written.                             |
| **GettingStarted.md**  | A user guide for setting up the project, building it, and using the core features.                                                                                       |
| **TODO.md**            | A contributor-focused checklist of pending tasks, known issues, and planned features.                                                                                    |
| **UndoSystem.md**      | Documents the undo/redo system for audio operations, the patterns used (e.g., Command Pattern), and outlines future improvements.                                        |

---