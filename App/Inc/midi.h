#ifndef MIDI_H
#define MIDI_H

#include "mein.h"

extern uint8 midiNoteOnData[3];
extern uint8 midiNoteOffData[3];

void MIDI_SendNoteOn(uint8 note, uint8 channel, uint8 vel, int8 octaveOffset);

void MIDI_SendNoteOff(uint8 note, uint8 channel,  int8 octaveOffset);

void MIDI_SendSustainOn(uint8 channel);

void MIDI_SendSustainOff(uint8 channel);

void MIDI_SendAllNotesOff(uint8 channel);

void MIDI_SendModulation(uint8 channel, uint8 value);

void MIDI_SendControlChange(uint8 channel, uint8 paramNumber, uint8 value);

void MIDI_SendPitchBend(uint8 channel, uint16 value);

#endif 