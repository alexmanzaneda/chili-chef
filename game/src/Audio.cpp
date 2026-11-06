#include <iostream>
#define MINIAUDIO_IMPLEMENTATION // The code of miniaudio is compiled here, and only here
#include <miniaudio.h>
#include "Audio.h"


using namespace std;


#define SOUND_VOLUME 0.6f
#define MUSIC_VOLUME 0.4f
// The same effect started again before this many ms is skipped: a chain of bombs or 'K'
// would start many copies at once, and together they sound much louder and distorted
#define SOUND_MIN_INTERVAL 50


// Files of the effects, in the order of SoundId (tools/make_sounds.py generates them)
static const char *SOUND_FILES[NUM_SOUNDS] =
{
	"sounds/menu_move.wav", "sounds/menu_select.wav", "sounds/pause.wav",
	"sounds/bomb_place.wav", "sounds/explosion.wav", "sounds/jump.wav", "sounds/power_up.wav",
	"sounds/heart_lost.wav", "sounds/chef_death.wav", "sounds/enemy_death.wav",
	"sounds/door_open.wav", "sounds/level_clear.wav", "sounds/game_over.wav", "sounds/victory.wav",
	"sounds/boss_roar.wav", "sounds/boss_hit.wav", "sounds/boss_shoot.wav", "sounds/boss_summon.wav",
	"sounds/bar_warning.wav", "sounds/bar_on.wav",
	"sounds/fireball_hit.wav", "sounds/boss_phase.wav", "sounds/boss_charge.wav", "sounds/boss_dash.wav", "sounds/boss_explode.wav",
	"sounds/life_lost.wav",
};

// Files of the tracks, in the order of MusicId (tools/make_music.py generates them). They
// are read while they play (not loaded whole), so they can be long. WAV (or FLAC) and not
// MP3: an MP3 has some silence at the start and the end that miniaudio does not remove,
// and the loop would jump there
static const char *MUSIC_FILES[NUM_MUSICS] =
{
	"music/menu.wav", "music/kitchen.wav", "music/fridge.wav", "music/oven.wav",
};


Audio::Audio()
{
	engine = NULL;
	soundGroup = NULL;
	musicGroup = NULL;
	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		sounds[i] = NULL;
		lastPlayed[i] = 0;
	}
	music = NULL;
	currentMusic = MUSIC_NONE;
}

void Audio::init()
{
	engine = new ma_engine;
	if(ma_engine_init(NULL, engine) != MA_SUCCESS)
	{
		cout << "Audio: no audio device, the game runs without sound" << endl;
		delete engine;
		engine = NULL;
		return;
	}

	soundGroup = new ma_sound_group;
	musicGroup = new ma_sound_group;
	ma_sound_group_init(engine, 0, NULL, soundGroup);
	ma_sound_group_init(engine, 0, NULL, musicGroup);
	setSoundVolume(SOUND_VOLUME);
	setMusicVolume(MUSIC_VOLUME);

	// The effects are decoded now and stay in memory, so playing them never waits for the disk.
	// playSound uses the same file name, and miniaudio reuses the data already loaded
	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		sounds[i] = new ma_sound;
		if(ma_sound_init_from_file(engine, SOUND_FILES[i], MA_SOUND_FLAG_DECODE, soundGroup, NULL, sounds[i]) != MA_SUCCESS)
		{
			cout << "Audio: cannot load " << SOUND_FILES[i] << endl;
			delete sounds[i];
			sounds[i] = NULL;
		}
	}
}

void Audio::free()
{
	if(engine == NULL)
		return;

	stopMusic();
	if(music != NULL)
	{
		ma_sound_uninit(music);
		delete music;
		music = NULL;
	}
	currentMusic = MUSIC_NONE;

	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		if(sounds[i] != NULL)
		{
			ma_sound_uninit(sounds[i]);
			delete sounds[i];
			sounds[i] = NULL;
		}
	}

	// The engine goes last: it also frees the copies of the effects still playing
	ma_sound_group_uninit(soundGroup);
	ma_sound_group_uninit(musicGroup);
	ma_engine_uninit(engine);
	delete soundGroup;
	delete musicGroup;
	delete engine;
	soundGroup = musicGroup = NULL;
	engine = NULL;
}

void Audio::playSound(SoundId sound)
{
	if(engine == NULL || sounds[sound] == NULL)
		return;

	unsigned long long now = ma_engine_get_time_in_milliseconds(engine);
	if(lastPlayed[sound] != 0 && now - lastPlayed[sound] < SOUND_MIN_INTERVAL)
		return;
	lastPlayed[sound] = now;

	// A new copy each time, so the same effect can sound several times at once
	ma_engine_play_sound(engine, SOUND_FILES[sound], soundGroup);
}

void Audio::playMusic(MusicId newMusic)
{
	if(engine == NULL || newMusic == MUSIC_NONE)
		return;

	if(newMusic != currentMusic)
	{
		if(music != NULL)
		{
			ma_sound_uninit(music);
			delete music;
			music = NULL;
		}
		currentMusic = newMusic;

		music = new ma_sound;
		if(ma_sound_init_from_file(engine, MUSIC_FILES[newMusic], MA_SOUND_FLAG_STREAM, musicGroup, NULL, music) != MA_SUCCESS)
		{
			cout << "Audio: cannot load " << MUSIC_FILES[newMusic] << endl;
			delete music;
			music = NULL;
			return;
		}
		ma_sound_set_looping(music, MA_TRUE);
	}

	if(music != NULL)
		ma_sound_start(music); // Nothing changes if it was already playing
}

void Audio::stopMusic()
{
	if(music == NULL)
		return;
	ma_sound_stop(music);
	ma_sound_seek_to_pcm_frame(music, 0);
}

void Audio::pauseMusic()
{
	if(music != NULL)
		ma_sound_stop(music); // It keeps its position
}

void Audio::setSoundVolume(float volume)
{
	if(soundGroup != NULL)
		ma_sound_group_set_volume(soundGroup, volume);
}

void Audio::setMusicVolume(float volume)
{
	if(musicGroup != NULL)
		ma_sound_group_set_volume(musicGroup, volume);
}
