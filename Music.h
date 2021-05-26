// Music.h
// This program contains timer0A timer1A ISR for 
// playing your favorite Song.
//
// For use with the TM4C123
// EE319K lab6 
// 3/25/21 

// Description: 
// This file contains the interrupt delays for notes, the samples
// for the sine wave used to make sound, and the data structure
// containing notes for your favorite.
//

#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>
#include "Notes.h"	// Period definitions for notes

#define delay1s 80000000
#define tempo 240	// in bpm (static tempo for multiplying later)
#define Q (delay1s/(tempo/60))	// quarter note
#define H (Q*2)	// half note
#define D_H (Q*3) // dotted-half note
#define W (Q*4)	// whole note
#define Ei (Q/2)	// eighth note
#define D_Q (Ei*3) // dotted-quarter note
#define Sp (2000000) // 25ms delay for note separation
#define Z (2000000) // fixed delay for fast sound effects
struct note{
	uint32_t pitch;	// period in bus cycles or note name from "Notes.h"
	uint32_t duration;	// either Q, H, W, Ei
};
typedef const struct note note_t;
// first 'note' of song array defines the envelope
// song is terminated by {0, 0}

// Song definitions - actual song files are in Music.c
extern const note_t TestScale[];
extern const note_t Chirp[];
extern const note_t SongPlaceholder[];
extern const note_t TestJump[];
extern const note_t Death[];

// Initialize music player
void Music_Init(void);

// Play your favorite song, while button pushed or until end
void Music_PlaySong(note_t* song);

// Stop song
void Music_StopSong(void);

#endif