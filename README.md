Note Guessing Game
A terminal-based C program that plays a random musical note from a selected octave range, asks the user to guess the note, and tracks the score over multiple rounds. Supports macOS and Linux only.
Prerequisites

C Compiler: GCC or Clang (available on macOS and Linux).
macOS: CoreAudio (included with macOS).
Linux: ALSA library (alsa-lib on Artix/Arch, libasound2-dev on Ubuntu/Debian, alsa-lib-devel on Fedora).

Installation

Linux:Install ALSA development library:sudo pacman -S alsa-lib  # Artix/Arch
sudo apt-get install libasound2-dev  # Ubuntu/Debian
sudo yum install alsa-lib-devel     # Fedora


macOS:No additional libraries needed (CoreAudio is built-in).

Build Instructions

Clone or download the project.
Ensure prerequisites are installed (see above).
Run:make


Execute:./note_game



Usage

Opening Menu:

1. Default: Whole notes only (C, D, E, F, G, A, B) in octave 4, 2-second duration, 5 rounds.
2. Harder: All notes (including sharps/flats) in octave 4, 2-second duration, 5 rounds.
3. Custom: Choose start/end octaves (3–5), duration (1–5 seconds), rounds (1–20), and whether to include sharps/flats (0 = whole notes only, 1 = include sharps).


Gameplay:

Listen to a note and guess its name.
Input Format: Enter notes as C4, C#4, D4, etc. (case-sensitive, e.g., C4 not c4).
Whole notes: C, D, E, F, G, A, B followed by octave number (e.g., C4).
Sharps: Use # (e.g., C#4 for C-sharp in octave 4).


After each round, see if your guess was correct.
At the end, view your score (correct guesses / total rounds).



Troubleshooting

Linux: High-frequency noise:If a high-pitched noise occurs after playing a note, ensure ALSA is configured correctly:
aplay -l

Check for pulseaudio or pipewire conflicts (common on Artix). Restart ALSA:
sudo alsa force-reload  # Artix/Arch

Ensure speakers are connected and not muted. If noise persists, try a different ALSA device (e.g., hw:0,0 instead of default in code).

Linux: ALSA errors:Ensure alsa-lib (Artix/Arch) or libasound2-dev (Ubuntu/Debian) or alsa-lib-devel (Fedora) is installed.Verify ALSA device availability:
aplay -l


macOS: No sound:Ensure system audio output is set correctly (System Settings > Sound).Verify gcc or clang is installed via Xcode Command Line Tools:
xcode-select --install



Notes

Uses CoreAudio (macOS) or ALSA (Linux) for audio, both GNU-compliant.
Ensure speakers are on and volume is adjusted.
Not supported on Windows.


