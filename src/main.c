#define TIMER_READY 1
#define TIMER_REACT 2
#define TIMER_EARLY 3
#define TIMER_DEBOUNCE 4 
#define DEFAULT_MIN_DELAY 1000
#define DEFAULT_MAX_DELAY 3000
#define DEFAULT_EARLY_RESET_DELAY 3000
#define DEFAULT_VIRTUAL_DEBOUNCE 50
#define DEFAULT_AVG_TRIALS 5
#define DEFAULT_TOTAL_TRIALS 1000
#define DEFAULT_RAWKEYBOARDENABLE 1
#define DEFAULT_RAWMOUSEENABLE 1

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

void PrimaryLogic();
void DisplayLogic();
void GameStateLogic();
int  GenerateRandomDelay(int min, int max);
void AllocateMemoryForReactionTimes();
void HandleInput();
void UpdateKeyState();
void ResetLogic();
void HandleReactClick();
void HandleEarlyClick();
void SetTimer();
void KillTimer();

// ##REVIEW## Might want to break this stuff up, structs aren't going to be that important/useful
typedef struct {
    // Game Options
    int averaging_trials;
    int total_trials;
    unsigned int min_delay;
    unsigned int max_delay;
    unsigned int early_reset_delay;
} Configuration;

// Program State and Data
typedef struct {
    enum {
        STATE_READY,
        STATE_REACT,
        STATE_EARLY,
        STATE_RESULT
    } game_state;
    int current_attempt;
    int trial_iteration;
    double reaction_times[512];
    int start_time;
    int end_time;
    int frequency;
    bool input_active;
    bool debounce_active;
} ProgramState;

// Declare structs, pointers, and variables ##REVIEW## I would prefer this to be in my main scope
Configuration config;
ProgramState program_state = {.game_state = STATE_READY, .reaction_times = {0}};

int main() {
    program_state.game_state = STATE_READY;
    srand((unsigned)time(NULL));
    SetTimer(TIMER_READY, GenerateRandomDelay(config.min_delay, config.max_delay)); // ##REVIEW## Need a timer function. HW specific?
    
    return 0;
}

void PrimaryLogic() { // Should this be moved into a while(1) in main? Switch case not needed?
    switch() {
    case WM_TIMER:
        GameStateLogic();
        break;

    case WM_INPUT:
        HandleInput();
        break;
    }
}

void DisplayLogic() { // ##REVIEW## Needs a complete rewite to send i2c text. Does it need to be a continous i2c signal?
        // Display text
        wchar_t buffer[100] = {0};

        if (program_state.game_state == STATE_RESULT) { // ##REVIEW## This is probably the hardest to follow code in the script
            program_state.trial_iteration++;

            if (program_state.current_attempt < config.averaging_trials) {
                swprintf_s(buffer, 100, L"Last: %.2lfms\nComplete %d trials for average.\nTrials so far: %d",
                    program_state.reaction_times[(program_state.current_attempt - 1 + config.averaging_trials) % config.averaging_trials],
                         config.averaging_trials, program_state.trial_iteration);
            }
            else {
                double total = 0;
                for (int i = 0; i < config.averaging_trials; i++) {
                    total += program_state.reaction_times[i];
                }
                double average = total / config.averaging_trials;
                swprintf_s(buffer, 100, L"Last: %.2lfms\nAverage (last %d): %.2lfms\nTrials so far: %d",
                    program_state.reaction_times[(program_state.current_attempt - 1) % config.averaging_trials], config.averaging_trials, average, program_state.trial_iteration);
            }
        }
        else if (program_state.game_state == STATE_EARLY) {
            swprintf_s(buffer, 100, L"Too early!\nTrials so far: %d", program_state.trial_iteration);
        }

}

void GameStateLogic() { // ##REVIEW## Needs major rewriting
        switch (YET TO BE NAMED TIMER VARIABLE???) {
        case TIMER_READY:
            program_state.game_state = STATE_READY;
            KillTimer(TIMER_READY);
            SetTimer(TIMER_REACT, GenerateRandomDelay(config.min_delay, config.max_delay));
            break;

        case TIMER_REACT:
            if (program_state.game_state == STATE_READY) {
                program_state.game_state = STATE_REACT;
                QueryPerformanceCounter(&program_state.start_time); // Being to measure reaction
            }
            break;

        case TIMER_EARLY:
            ResetLogic(); // Reset the game after showing the "too early" screen
            break;

        case TIMER_DEBOUNCE:  // Debounce reset
            program_state.debounce_active = false;
            break;

        }
}

int GenerateRandomDelay(int min, int max) { // Rejection Sampling RNG
    int range = max - min + 1;
    int buckets = RAND_MAX / range;
    int limit = buckets * range;

    int r;
    do {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void HandleInput() {   // Primary input logic is done here
    if (!program_state.input_active) { // ##REVIEW## Should I be passing input_active as a variable and then handle it locally?
        return;
    }
    if (program_state.game_state == STATE_REACT) {
        HandleReactClick();
    }
    else if ((program_state.game_state == STATE_EARLY) || (program_state.game_state == STATE_RESULT)) {
        if ((program_state.game_state == STATE_RESULT) && program_state.current_attempt == config.averaging_trials) { // ##REVIEW## Double check this
            program_state.current_attempt = 0;
            for (int i = 0; i < config.averaging_trials; i++) {
                program_state.reaction_times[i] = 0;
            }
        }
        ResetLogic();
    }
    else {
        HandleEarlyClick();
    }
    if (program_state.input_active) { // Activate debounce
        program_state.debounce_active = true;
        SetTimer(TIMER_DEBOUNCE, );
    }
}

// Main application logic functions
void ResetLogic() {
    KillTimer(TIMER_READY);
    KillTimer(TIMER_REACT);
    KillTimer(TIMER_EARLY);
    program_state.game_state = STATE_READY;  
    SetTimer(TIMER_READY, GenerateRandomDelay(config.min_delay, config.max_delay));
}

void HandleReactClick() {
    double time_taken = ((double)(program_state.end_time - program_state.start_time) / program_state.frequency) * 1000; // ##REVIEW## Probably won't work
    
    program_state.reaction_times[program_state.current_attempt % config.averaging_trials] = time_taken;
    program_state.current_attempt++;

    program_state.game_state = STATE_RESULT;
}

void HandleEarlyClick() {
    program_state.game_state = STATE_EARLY;
    SetTimer(TIMER_EARLY, config.early_reset_delay); // Early state eventually resets back to Ready state regardless of user input
}

void SetTimer() { // Just supply timer name and time
    
}

void KillTimer() { // End timer if it is running

}
