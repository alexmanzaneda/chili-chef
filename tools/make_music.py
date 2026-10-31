"""Generates the background music of the game, in the style of the Game Boy.

    python tools/make_music.py

writes one .wav per track to game/music/. Like make_sounds.py, nothing is recorded
or downloaded: the tracks are written here as notes and synthesized with the four
voices of the Game Boy (two square waves, a triangle-like wave and noise), so they
are ours. They are meant as placeholders until the real music is composed; to
replace one, just put another .wav with the same name in game/music/.

Every track is a loop: its last sample goes straight into the first one, so the
game plays it in a loop with no jump. The names must match MUSIC_FILES (Audio.cpp).

How a track is written (see SONGS):
  - chords: one per bar ('C', 'Am', 'Bb', 'F#m'...).
  - melody: one string per bar with 16 steps (sixteenth notes): a note ('E5', 'F#5',
    'Bb4'), '-' to hold the previous one or '.' for silence.
  - bass, arp: 16 steps repeated in every bar, built from the chord of the bar:
    'R'/'1' root, '3' third, '5' fifth, '8' octave, '-' hold, '.' silence.
  - drums: 16 steps repeated in every bar: 'k' kick, 's' snare, 'h' hi-hat, '.' nothing.
The whole song is played twice: the second time the melody is thinner and has an
echo, so the loop does not feel so repetitive.
"""

import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(__file__))
from make_sounds import RATE, write_wav

ROOT = os.path.join(os.path.dirname(__file__), '..')
OUT_DIR = os.path.join(ROOT, 'game', 'music')

NOTE_NAMES = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}
CLICK = 0.004  # s of fade at both ends of every note, so no note clicks


SONGS = {
	# Cheerful and bright, C major
	'menu': {
		'bpm': 150,
		'chords': ['C', 'G', 'Am', 'F', 'C', 'G', 'F', 'G'],
		'melody': [
			'E5 - - G5 - - C6 - B5 - G5 - E5 - - -',
			'D5 - - G5 - - B5 - A5 - G5 - D5 - - -',
			'C5 - E5 - A5 - - - G5 - A5 - C6 - - -',
			'A5 - G5 - F5 - - - E5 - F5 - G5 - - -',
			'E5 - - G5 - - C6 - B5 - G5 - E5 - G5 -',
			'D6 - - B5 - - G5 - A5 - B5 - D6 - - -',
			'C6 - A5 - F5 - A5 - G5 - E5 - D5 - - -',
			'G5 - - - F5 - E5 - D5 - - - . . . .',
		],
		'bass': 'R.R.5.R.R.5.8.5.',
		'bass_octave': 3,
		'arp': '1358135813581358',
		'arp_octave': 5,
		'drums': 'k.h.s.h.k.k.s.h.',
	},
	# The kitchen: fast and busy, D minor
	'kitchen': {
		'bpm': 165,
		'chords': ['Dm', 'Dm', 'Bb', 'C', 'Dm', 'Dm', 'Bb', 'A'],
		'melody': [
			'D5 . D5 . F5 - D5 . A5 - G5 - F5 - E5 -',
			'D5 - - - . . A4 - D5 - E5 - F5 - G5 -',
			'F5 . F5 . D5 - F5 . Bb5 - A5 - G5 - F5 -',
			'E5 - - - C5 - E5 - G5 - - - C6 - - -',
			'D5 . D5 . F5 - D5 . A5 - G5 - F5 - E5 -',
			'D5 - - - . . A5 - D6 - C6 - A5 - F5 -',
			'G5 . G5 . F5 - D5 . F5 - G5 - Bb5 - A5 -',
			'A5 - - - C#6 - - - E6 - - - . . . .',
		],
		'bass': 'R.RR5.R.R.RR8.5.',
		'bass_octave': 2,
		'arp': '1.5.8.5.1.5.8.5.',
		'arp_octave': 4,
		'drums': 'k.h.s.hkk.h.s.hh',
	},
	# The fridge: slow and icy, E minor, with a high sparkle
	'fridge': {
		'bpm': 112,
		'chords': ['Em', 'C', 'G', 'D', 'Em', 'C', 'D', 'B'],
		'melody': [
			'B5 - - - - - - - G5 - - - E5 - - -',
			'E5 - - - G5 - - - C6 - - - B5 - - -',
			'D6 - - - - - - - B5 - - - G5 - - -',
			'A5 - - - - - - - F#5 - - - D5 - - -',
			'B5 - - - E6 - - - D6 - - - B5 - - -',
			'C6 - - - B5 - - - G5 - - - E5 - - -',
			'F#5 - - - A5 - - - D6 - - - C6 - - -',
			'B5 - - - - - - - D#5 - - - F#5 - - -',
		],
		'bass': 'R-------5-------',
		'bass_octave': 2,
		'arp': '1358853113588531',
		'arp_octave': 6,
		'drums': 'k...h...s...h..h',
	},
}


# --- Notes and chords ------------------------------------------------------------

def note_frequency(name):
	"""'C5', 'F#4', 'Bb5' -> Hz (A4 = 440)."""
	semitone = NOTE_NAMES[name[0]]
	rest = name[1:]
	if rest[0] == '#':
		semitone, rest = semitone + 1, rest[1:]
	elif rest[0] == 'b':
		semitone, rest = semitone - 1, rest[1:]
	midi = 12 * (int(rest) + 1) + semitone
	return 440.0 * 2 ** ((midi - 69) / 12)


def chord_tones(chord, octave):
	"""Frequencies of the degrees of a chord: '1' root, '3' third, '5' fifth, '8' octave."""
	root = chord[0]
	rest = chord[1:]
	if rest[:1] in ('#', 'b'):
		root, rest = root + rest[0], rest[1:]
	third = 3 if rest == 'm' else 4
	base = note_frequency(root + str(octave))
	semitones = {'1': 0, 'R': 0, '3': third, '5': 7, '8': 12}
	return {degree: base * 2 ** (s / 12) for degree, s in semitones.items()}


def steps_to_notes(tokens, frequency_of):
	"""[(first step, number of steps, frequency)] from the tokens of a pattern."""
	notes = []
	for step, token in enumerate(tokens):
		if token == '-':
			if notes and notes[-1][0] + notes[-1][1] == step:
				start, length, freq = notes[-1]
				notes[-1] = (start, length + 1, freq)
		elif token != '.':
			notes.append((step, 1, frequency_of(token)))
	return notes


# --- Voices: each one adds a note to the buffer -----------------------------------

def add(buffer, start, samples):
	"""What goes past the end (the echo of the last bar) comes in at the start, as it
	sounds when the game repeats the loop."""
	for i, s in enumerate(samples):
		buffer[(start + i) % len(buffer)] += s


def shape(samples, decay=0.0):
	"""Fade at both ends (no clicks) and an optional decay of the volume during the note."""
	n = len(samples)
	edge = max(1, int(CLICK * RATE))
	out = []
	for i, s in enumerate(samples):
		gain = min(1.0, i / edge, (n - i) / edge) * (1.0 - decay * i / n)
		out.append(s * gain)
	return out


def pulse(n, freq, duty, volume, vibrato=0.0):
	samples, phase = [], 0.0
	for i in range(n):
		# The vibrato only starts after a while, like a singer holding a note
		wobble = 1.0 + vibrato * max(0.0, i / RATE - 0.15) * math.sin(2 * math.pi * 6 * i / RATE)
		phase = (phase + freq * wobble / RATE) % 1.0
		samples.append(volume if phase < duty else -volume)
	return samples


def triangle(n, freq, volume):
	"""The wave channel of the Game Boy, used for the bass: 4-bit steps of a triangle."""
	samples, phase = [], 0.0
	for _ in range(n):
		phase = (phase + freq / RATE) % 1.0
		tri = 4 * phase - 1 if phase < 0.5 else 3 - 4 * phase
		samples.append(round(tri * 7.5) / 7.5 * volume)
	return samples


def drum(kind):
	if kind == 'k':  # Kick: a square that falls fast in pitch
		n, samples, phase = int(0.09 * RATE), [], 0.0
		for i in range(n):
			phase = (phase + (160 - 120 * i / n) / RATE) % 1.0
			samples.append(0.5 if phase < 0.5 else -0.5)
		return shape(samples, decay=1.0)
	if kind == 's':  # Snare: noise
		return shape([random.uniform(-0.35, 0.35) for _ in range(int(0.11 * RATE))], decay=1.0)
	# Hi-hat: very short noise
	return shape([random.uniform(-0.12, 0.12) for _ in range(int(0.03 * RATE))], decay=1.0)


# --- Song --------------------------------------------------------------------------

def check(name, song):
	"""A bar with a step too many or too few would shift everything after it."""
	if len(song['melody']) != len(song['chords']):
		sys.exit('%s: %d bars of melody and %d chords' % (name, len(song['melody']), len(song['chords'])))
	for bar, tokens in enumerate(song['melody']):
		if len(tokens.split()) != 16:
			sys.exit('%s: bar %d of the melody has %d steps, not 16' % (name, bar + 1, len(tokens.split())))
	for key in ('bass', 'arp', 'drums'):
		if len(song[key]) != 16:
			sys.exit('%s: %s has %d steps, not 16' % (name, key, len(song[key])))


def render(song):
	step =int(RATE * 60 / song['bpm'] / 4)  # Samples of a sixteenth note
	bars = len(song['chords'])
	passes = 2
	total = step * 16 * bars * passes
	buffer = [0.0] * total

	for p in range(passes):
		for bar in range(bars):
			bar_start = (p * bars + bar) * 16 * step
			chord = song['chords'][bar]

			# Melody: the first time fuller, the second thinner and with an echo
			duty = 0.5 if p == 0 else 0.25
			for start, length, freq in steps_to_notes(song['melody'][bar].split(), note_frequency):
				n = length * step
				add(buffer, bar_start + start * step, shape(pulse(n, freq, duty, 0.30, vibrato=0.01), decay=0.3))
				if p == 1:
					echo = shape(pulse(n, freq, duty, 0.10), decay=0.6)
					add(buffer, bar_start + (start + 3) * step, echo)

			bass = chord_tones(chord, song['bass_octave'])
			for start, length, freq in steps_to_notes(song['bass'], bass.get):
				add(buffer, bar_start + start * step, shape(triangle(length * step, freq, 0.45)))

			arp = chord_tones(chord, song['arp_octave'])
			for start, length, freq in steps_to_notes(song['arp'], arp.get):
				add(buffer, bar_start + start * step, shape(pulse(length * step, freq, 0.125, 0.08), decay=0.7))

			for s, kind in enumerate(song['drums']):
				if kind != '.':
					add(buffer, bar_start + s * step, drum(kind))

	peak = max(abs(s) for s in buffer)
	return [s / peak * 0.9 for s in buffer]


def main():
	random.seed(2026)  # The same noise every time, so the files only change when a song does
	os.makedirs(OUT_DIR, exist_ok=True)
	for name, song in SONGS.items():
		check(name, song)
		path = os.path.join(OUT_DIR, name + '.wav')
		samples = render(song)
		write_wav(path, samples)
		print('Written', os.path.relpath(path, ROOT), '(%.1f s)' % (len(samples) / RATE))


if __name__ == '__main__':
	main()
