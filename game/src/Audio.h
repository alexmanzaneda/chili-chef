#ifndef _AUDIO_INCLUDE
#define _AUDIO_INCLUDE


// miniaudio is only included by Audio.cpp, so the rest of the game does not
// depend on it. These are its types, declared here to keep pointers to them
struct ma_engine;
struct ma_sound;
typedef struct ma_sound ma_sound_group;


// Short effects. They can overlap: playing one again does not cut the previous one.
// The file of each one is in SOUND_FILES (Audio.cpp)
enum SoundId
{
	SOUND_MENU_MOVE, SOUND_MENU_SELECT, SOUND_PAUSE,
	SOUND_BOMB_PLACE, SOUND_EXPLOSION, SOUND_JUMP, SOUND_POWER_UP,
	SOUND_HEART_LOST, SOUND_CHEF_DEATH, SOUND_ENEMY_DEATH,
	SOUND_DOOR_OPEN, SOUND_LEVEL_CLEAR, SOUND_GAME_OVER, SOUND_VICTORY,
	SOUND_BOSS_ROAR, SOUND_BOSS_HIT, SOUND_BOSS_SHOOT, SOUND_BOSS_SUMMON,
	SOUND_BAR_WARNING, SOUND_BAR_ON,
	SOUND_FIREBALL_HIT, SOUND_BOSS_PHASE, SOUND_BOSS_CHARGE, SOUND_BOSS_DASH, SOUND_BOSS_EXPLODE,
	NUM_SOUNDS
};

// Background tracks. Only one plays at a time, in a loop.
// The file of each one is in MUSIC_FILES (Audio.cpp)
enum MusicId
{
	MUSIC_NONE = -1,
	MUSIC_MENU, MUSIC_KITCHEN, MUSIC_FRIDGE, MUSIC_OVEN,
	NUM_MUSICS
};


// Audio is a singleton that plays the effects and the music of the game.
// If there is no audio device or a file is missing it says so in the console
// and that sound is just not heard: the game never stops because of the audio.


class Audio
{

private:
	Audio();
	~Audio() { free(); }

public:
	static Audio &instance()
	{
		static Audio A;

		return A;
	}

	void init(); // Opens the audio device and loads all the effects
	void free(); // Must be called before the program ends

	void playSound(SoundId sound);

	// Playing the track that is already playing does nothing; if it was paused it goes on
	void playMusic(MusicId music);
	void stopMusic();   // The next playMusic starts the track from the beginning
	void pauseMusic();  // The next playMusic goes on from where it was
	void setSoundVolume(float volume); // 0 (mute) .. 1
	void setMusicVolume(float volume);

private:
	ma_engine *engine;  // NULL if there is no audio
	ma_sound_group *soundGroup, *musicGroup; // Volume of all the effects / of the music
	ma_sound *sounds[NUM_SOUNDS]; // Effects kept in memory, NULL if they could not be loaded
	unsigned long long lastPlayed[NUM_SOUNDS]; // ms (time of the engine) each effect started last
	ma_sound *music;    // Track being played, NULL if none
	MusicId currentMusic;

};


#endif // _AUDIO_INCLUDE
