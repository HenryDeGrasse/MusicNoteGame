Note Guessing Game
A terminal-based C program that plays a random musical note from a selected octave range, asks the user to guess the note, and tracks the score over multiple rounds. Supports macOS and Linux only.
Prerequisites

C Compiler: GCC or Clang (available on macOS and Linux).
macOS: CoreAudio (included with macOS).
Linux: ALSA library (libasound2-dev on Ubuntu/Debian, alsa-lib-devel on Fedora).

Installation

Linux:Install ALSA development library:sudo apt-get install libasound2-dev  # Ubuntu/Debian
sudo yum install alsa-lib-devel     # Fedora


macOS:No additional libraries needed (CoreAudio is built-in).

Build Instructions

Clone or download the project.
Ensure prerequisites are installed (see above).
Run:make


Execute:./note_game



Troubleshooting

Linux: ALSA errors:Ensure libasound2-dev (Ubuntu/Debian) or alsa-lib-devel (Fedora) is installed.Check ALSA device availability:
aplay -l

If no sound, ensure speakers are connected and not muted.

macOS: No sound:Ensure system audio output is set correctly (System Settings > Sound).Verify gcc or clang is installed via Xcode Command Line Tools:
xcode-select --install



Usage

Select start and end octaves (3–5).
Choose note duration (1–5 seconds).
Specify number of rounds (1–20).
Listen to each note and guess its name (e.g., C4, C#4).
View your score at the end.

Notes

Uses CoreAudio (macOS) or ALSA (Linux) for audio, both GNU-compliant.
Ensure speakers are on and volume is adjusted.
Not supported on Windows.


