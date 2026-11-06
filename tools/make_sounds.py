"""Generates the sound effects of the game, in the style of the Game Boy.

    python tools/make_sounds.py

writes one .wav per effect to game/sounds/. Nothing is recorded or downloaded:
every effect is synthesized here (square waves and noise, like the sound chip of
the Game Boy), so they are ours and can be published without licensing issues.

The names must match SOUND_FILES (game/src/Audio.cpp), in the order of SoundId
(Audio.h). To add an effect: add its function to SOUNDS, its file to SOUND_FILES
and its id to SoundId. To tweak one, change its function and run this again.

Only the standard library is needed.
"""

import math
import os
import random
import struct
import wave

ROOT = os.path.join(os.path.dirname(__file__), '..')
OUT_DIR = os.path.join(ROOT, 'game', 'sounds')

RATE = 22050  # Samples per second: enough for 8-bit style sounds
VOLUME = 0.5  # Peak of every effect; Audio.cpp sets the final volume

C5, D5, E5, F5, G5, A5, B5 = 523.25, 587.33, 659.26, 698.46, 783.99, 880.00, 987.77
C6, E6, G6 = 1046.50, 1318.51, 1567.98


# --- Waves: each one returns the samples (-1..1) of a sound of 'seconds' ---------

def square(seconds, freq_from, freq_to=None, duty=0.5):
	"""Square wave whose pitch slides from freq_from to freq_to."""
	freq_to = freq_from if freq_to is None else freq_to
	n = int(seconds * RATE)
	samples, phase = [], 0.0
	for i in range(n):
		freq = freq_from + (freq_to - freq_from) * i / n
		phase = (phase + freq / RATE) % 1.0
		samples.append(1.0 if phase < duty else -1.0)
	return samples


def noise(seconds, period_from=1, period_to=None):
	"""White noise. The longer the period (samples each value lasts), the lower it sounds."""
	period_to = period_from if period_to is None else period_to
	n = int(seconds * RATE)
	samples, value, left = [], 0.0, 0
	for i in range(n):
		if left <= 0:
			value = random.uniform(-1.0, 1.0)
			left = period_from + (period_to - period_from) * i / n
		left -= 1
		samples.append(value)
	return samples


def silence(seconds):
	return [0.0] * int(seconds * RATE)


# --- Shaping ---------------------------------------------------------------------

def envelope(samples, attack=0.005, release=None):
	"""Short fade in (no click at the start) and linear fade out until the end."""
	n = len(samples)
	a = max(1, int(attack * RATE))
	r = n if release is None else max(1, int(release * RATE))
	out = []
	for i, s in enumerate(samples):
		gain = min(1.0, i / a) * min(1.0, (n - i) / r)
		out.append(s * gain)
	return out


def notes(sequence, duty=0.5, gap=0.0):
	"""Melody: [(frequency, seconds), ...]. Each note with its own little envelope."""
	out = []
	for freq, seconds in sequence:
		out += envelope(square(seconds, freq, duty=duty), release=seconds * 0.6) + silence(gap)
	return out


def mix(*tracks):
	n = max(len(t) for t in tracks)
	return [sum(t[i] for t in tracks if i < len(t)) / len(tracks) for i in range(n)]


def vibrato(samples, depth=0.4, speed=30.0):
	"""Volume wobble, so a slide sounds sad or shaky."""
	return [s * (1.0 - depth * (0.5 + 0.5 * math.sin(2 * math.pi * speed * i / RATE))) for i, s in enumerate(samples)]


# --- Effects, by name of the file --------------------------------------------------

def menu_move():
	return envelope(square(0.04, 1200, duty=0.25))


def menu_select():
	return notes([(E6, 0.05), (G6, 0.09)], duty=0.25)


def pause():
	return notes([(B5, 0.06), (E5, 0.06), (B5, 0.06), (E5, 0.1)], duty=0.25)


def bomb_place():
	return envelope(square(0.09, 180, 90), release=0.08)


def explosion():
	# Noise that gets lower and fades slowly, with a low thump at the start
	boom = envelope(noise(0.6, 1, 14), release=0.55)
	thump = envelope(square(0.15, 90, 40), release=0.15)
	return mix(boom, boom, thump)


def jump():
	return envelope(square(0.14, 320, 760, duty=0.25), release=0.1)


def power_up():
	return notes([(C5, 0.05), (E5, 0.05), (G5, 0.05), (C6, 0.05), (E6, 0.05), (G6, 0.12)], duty=0.25)


def heart_lost():
	return vibrato(envelope(square(0.35, 700, 220, duty=0.25), release=0.3))


def chef_death():
	# A short "hit" and then a long fall
	hit = envelope(noise(0.08, 2), release=0.08)
	fall = vibrato(envelope(square(0.9, 880, 110, duty=0.5), release=0.5), depth=0.5, speed=12)
	return hit + fall


def enemy_death():
	pop = envelope(square(0.12, 900, 200, duty=0.25), release=0.1)
	crackle = envelope(noise(0.2, 2, 6), release=0.2)
	return mix(pop, crackle)


def door_open():
	return notes([(G5, 0.08), (C6, 0.08), (E6, 0.08), (G6, 0.2)], duty=0.5)


def level_clear():
	return notes([(C5, 0.1), (E5, 0.1), (G5, 0.1), (C6, 0.2), (G5, 0.1), (C6, 0.45)], duty=0.5, gap=0.02)


def game_over():
	return notes([(G5, 0.25), (F5, 0.25), (E5, 0.25), (D5, 0.3), (C5, 0.7)], duty=0.5, gap=0.04)


def life_lost():
	# Like game_over but lighter: shorter, thinner (duty 0.25) and ending on D instead of the
	# tonic, so it sounds like "oops" and not like the end. It lasts as long as the counter blinks
	return notes([(E5, 0.12), (C5, 0.12), (D5, 0.4)], duty=0.25, gap=0.03)


def victory():
	return notes([(C5, 0.12), (C5, 0.12), (C5, 0.12), (C6, 0.4), (A5, 0.2), (B5, 0.2), (C6, 0.7)], duty=0.5, gap=0.03)


def boss_roar():
	# A low growl that rises and falls
	growl = vibrato(envelope(square(0.7, 70, 140), release=0.3), depth=0.3, speed=18)
	return mix(growl, envelope(noise(0.7, 6, 10), release=0.4))


def boss_hit():
	# Metal being hit: a high clang over a thump
	clang = envelope(square(0.18, 1400, 900, duty=0.125), release=0.15)
	thump = envelope(square(0.2, 150, 60), release=0.2)
	return mix(clang, thump)


def boss_shoot():
	return mix(envelope(noise(0.22, 3, 1), release=0.15), envelope(square(0.18, 500, 200, duty=0.25), release=0.12))


def boss_summon():
	return vibrato(envelope(square(0.4, 200, 800, duty=0.25), release=0.2), depth=0.5, speed=20)


def bar_warning():
	# An electric buzz that grows, while the bar blinks
	return vibrato(envelope(square(0.6, 55, 110, duty=0.25), attack=0.4, release=0.1), depth=0.2, speed=40)


def bar_on():
	# The bar catches: a sharp crackle and a hiss that fades
	crack = envelope(noise(0.06, 1), release=0.05)
	hiss = envelope(noise(0.45, 2, 4), release=0.4)
	return crack + hiss


def fireball_hit():
	# A short soft puff, lower than the shot so they do not get mixed up
	puff = envelope(noise(0.2, 2, 8), release=0.18)
	thump = envelope(square(0.1, 160, 70, duty=0.25), release=0.1)
	return mix(puff, puff, thump)


def boss_phase():
	# The lamps of the oven sound an alarm over a deeper growl: now it is angrier
	alarm = notes([(A5, 0.09), (E5, 0.09)] * 3, duty=0.25, gap=0.01)
	growl = vibrato(envelope(square(len(alarm) / RATE, 55, 45), release=0.3), depth=0.4, speed=14)
	return mix(alarm, growl, growl)


def boss_charge():
	# Its motor revs up while it shakes, before the charge
	return vibrato(envelope(square(0.8, 50, 220, duty=0.25), attack=0.3, release=0.08), depth=0.3, speed=45)


def boss_dash():
	# The charge: a whoosh of air that goes from high to low
	return envelope(noise(0.35, 1, 6), attack=0.03, release=0.3)


def boss_explode():
	# The end of the boss: a long rumble with a deep thump and crackles over it
	boom = envelope(noise(1.3, 2, 24), release=1.2)
	thump = envelope(square(0.5, 90, 28), release=0.5)
	crackle = envelope(noise(0.6, 1, 3), release=0.6)
	return mix(boom, boom, thump, crackle)


SOUNDS = [
	menu_move, menu_select, pause,
	bomb_place, explosion, jump, power_up,
	heart_lost, chef_death, enemy_death,
	door_open, level_clear, game_over, victory,
	boss_roar, boss_hit, boss_shoot, boss_summon,
	bar_warning, bar_on,
	fireball_hit, boss_phase, boss_charge, boss_dash, boss_explode,
	life_lost,
]


def write_wav(path, samples):
	with wave.open(path, 'wb') as f:
		f.setnchannels(1)
		f.setsampwidth(2)  # 16 bits
		f.setframerate(RATE)
		f.writeframes(b''.join(struct.pack('<h', int(max(-1.0, min(1.0, s)) * VOLUME * 32767)) for s in samples))


def main():
	random.seed(2026)  # The same noise every time, so the files only change when an effect does
	os.makedirs(OUT_DIR, exist_ok=True)
	for sound in SOUNDS:
		path = os.path.join(OUT_DIR, sound.__name__ + '.wav')
		write_wav(path, sound())
		print('Written', os.path.relpath(path, ROOT))


if __name__ == '__main__':
	main()
