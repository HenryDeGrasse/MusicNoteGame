#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <AudioUnit/AudioUnit.h>
#endif
#ifdef __linux__
#include <alsa/asoundlib.h>
#endif

#define PI 3.1415926535f
#define SAMPLE_RATE 44100

// Frequencies for octaves 3, 4, 5 (C3 to B5)
const double frequencies[] = {
    // Octave 3
    130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00, 233.08, 246.94,
    // Octave 4
    261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
    // Octave 5
    523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 880.00, 932.33, 987.77
};
const char *note_names[] = {
    // Octave 3
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
    // Octave 4
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
    // Octave 5
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5"
};
const int notes_per_octave = 12;
const int whole_note_indices[] = {0, 2, 4, 5, 7, 9, 11}; // C, D, E, F, G, A, B

#ifdef __APPLE__
static float phase = 0.0f;
static double target_frequency = 0.0;

OSStatus audio_callback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
                       const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                       UInt32 inNumberFrames, AudioBufferList *ioData) {
    float *buffer = (float *)ioData->mBuffers[0].mData;
    float radians_per_sample = target_frequency * 2.0f * PI / SAMPLE_RATE;
    for (UInt32 i = 0; i < inNumberFrames; i++) {
        buffer[i] = sinf(phase) * 0.5f;
        phase = fmodf(phase + radians_per_sample, 2.0f * PI);
    }
    return noErr;
}

void play_note(double frequency, int duration) {
    AudioComponentDescription desc = {kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0};
    AudioComponent comp = AudioComponentFindNext(NULL, &desc);
    AudioUnit au;
    AudioComponentInstanceNew(comp, &au);

    AudioStreamBasicDescription asbd = {0};
    asbd.mSampleRate = SAMPLE_RATE;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
    asbd.mChannelsPerFrame = 1;
    asbd.mFramesPerPacket = 1;
    asbd.mBitsPerChannel = 32;
    asbd.mBytesPerFrame = 4;
    asbd.mBytesPerPacket = 4;
    AudioUnitSetProperty(au, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &asbd, sizeof(asbd));

    AURenderCallbackStruct callback = {audio_callback, NULL};
    AudioUnitSetProperty(au, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback));

    target_frequency = frequency;
    phase = 0.0f;

    AudioUnitInitialize(au);
    AudioOutputUnitStart(au);
    sleep(duration);
    AudioOutputUnitStop(au);
    AudioUnitUninitialize(au);
    AudioComponentInstanceDispose(au);
}
#endif

#ifdef __linux__
void play_note(double frequency, int duration) {
    snd_pcm_t *pcm;
    int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "ALSA error: %s\n", snd_strerror(err));
        return;
    }

    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm, params);
    snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(pcm, params, 1);
    snd_pcm_hw_params_set_rate(pcm, params, SAMPLE_RATE, 0);
    if ((err = snd_pcm_hw_params(pcm, params)) < 0) {
        fprintf(stderr, "ALSA params error: %s\n", snd_strerror(err));
        snd_pcm_close(pcm);
        return;
    }

    float *buffer = malloc(SAMPLE_RATE * sizeof(float));
    float radians_per_sample = frequency * 2.0f * PI / SAMPLE_RATE;
    float phase = 0.0f;
    for (int i = 0; i < SAMPLE_RATE; i++) {
        buffer[i] = sinf(phase) * 0.5f;
        phase = fmodf(phase + radians_per_sample, 2.0f * PI);
    }

    for (int i = 0; i < duration; i++) {
        if ((err = snd_pcm_writei(pcm, buffer, SAMPLE_RATE)) < 0) {
            fprintf(stderr, "ALSA write error: %s\n", snd_strerror(err));
        }
    }

    free(buffer);
    snd_pcm_drop(pcm);
    snd_pcm_close(pcm);
}
#endif

int main() {
#if !defined(__APPLE__) && !defined(__linux__)
    printf("This program only supports macOS and Linux.\n");
    return 1;
#endif

    srand(time(NULL));
    int start_octave, end_octave, duration, rounds, include_sharps, mode;
    int note_indices[36]; // Max size for 3 octaves
    int num_notes;

    // Opening menu
    printf("Note Guessing Game\n");
    printf("1. Default (whole notes, octave 4, 2s duration, 5 rounds)\n");
    printf("2. Harder (all notes, octave 4, 2s duration, 5 rounds)\n");
    printf("3. Custom (choose octaves, duration, rounds, include sharps)\n");
    printf("Select mode (1-3): ");
    scanf("%d", &mode);
    while (getchar() != '\n'); // Clear input buffer

    if (mode == 1) {
        start_octave = 4;
        end_octave = 4;
        duration = 2;
        rounds = 5;
        include_sharps = 0;
        num_notes = 7;
        for (int i = 0; i < num_notes; i++) {
            note_indices[i] = (start_octave - 3) * notes_per_octave + whole_note_indices[i];
        }
    } else if (mode == 2) {
        start_octave = 4;
        end_octave = 4;
        duration = 2;
        rounds = 5;
        include_sharps = 1;
        num_notes = notes_per_octave;
        for (int i = 0; i < num_notes; i++) {
            note_indices[i] = (start_octave - 3) * notes_per_octave + i;
        }
    } else if (mode == 3) {
        printf("Select start octave (3-5): ");
        scanf("%d", &start_octave);
        while (getchar() != '\n');
        if (start_octave < 3 || start_octave > 5) {
            printf("Invalid octave. Defaulting to 3.\n");
            start_octave = 3;
        }
        printf("Select end octave (%d-5): ", start_octave);
        scanf("%d", &end_octave);
        while (getchar() != '\n');
        if (end_octave < start_octave || end_octave > 5) {
            printf("Invalid octave. Defaulting to %d.\n", start_octave);
            end_octave = start_octave;
        }
        printf("Note duration (1-5 seconds): ");
        scanf("%d", &duration);
        while (getchar() != '\n');
        if (duration < 1 || duration > 5) {
            printf("Invalid duration. Defaulting to 2 seconds.\n");
            duration = 2;
        }
        printf("Number of rounds (1-20): ");
        scanf("%d", &rounds);
        while (getchar() != '\n');
        if (rounds < 1 || rounds > 20) {
            printf("Invalid rounds. Defaulting to 5.\n");
            rounds = 5;
        }
        printf("Include sharps/flats? (0 = whole notes only, 1 = include sharps): ");
        scanf("%d", &include_sharps);
        while (getchar() != '\n');
        if (include_sharps != 0 && include_sharps != 1) {
            printf("Invalid choice. Defaulting to whole notes only.\n");
            include_sharps = 0;
        }

        if (include_sharps) {
            num_notes = (end_octave - start_octave + 1) * notes_per_octave;
            for (int i = 0; i < num_notes; i++) {
                note_indices[i] = (start_octave - 3) * notes_per_octave + i;
            }
        } else {
            num_notes = (end_octave - start_octave + 1) * 7;
            for (int i = 0; i < end_octave - start_octave + 1; i++) {
                for (int j = 0; j < 7; j++) {
                    note_indices[i * 7 + j] = (start_octave - 3 + i) * notes_per_octave + whole_note_indices[j];
                }
            }
        }
    } else {
        printf("Invalid mode. Defaulting to mode 1.\n");
        start_octave = 4;
        end_octave = 4;
        duration = 2;
        rounds = 5;
        include_sharps = 0;
        num_notes = 7;
        for (int i = 0; i < num_notes; i++) {
            note_indices[i] = (start_octave - 3) * notes_per_octave + whole_note_indices[i];
        }
    }

    int score = 0;
    for (int i = 0; i < rounds; i++) {
        int random_idx = note_indices[rand() % num_notes];
        double selected_freq = frequencies[random_idx];
        const char *correct_note = note_names[random_idx];

        printf("\nRound %d: Listen and guess the note (e.g., C4, C#4):\n", i + 1);
        play_note(selected_freq, duration);

        char user_guess[10];
        printf("Your guess: ");
        fgets(user_guess, sizeof(user_guess), stdin);
        user_guess[strcspn(user_guess, "\n")] = '\0';

        if (strcmp(user_guess, correct_note) == 0) {
            printf("Correct! The note was %s.\n", correct_note);
            score++;
        } else {
            printf("Wrong. The note was %s. You guessed %s.\n", correct_note, user_guess);
        }
    }

    printf("\nGame Over! Score: %d/%d (%.2f%%)\n", score, rounds, (float)score / rounds * 100);
    return 0;
}
