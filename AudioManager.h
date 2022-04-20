
#pragma once
#include "Audio.h"
#include "D3D.h"
#include <Dbt.h>
#include <ksmedia.h>

struct SoundEffectManager
{
	//A bunch of handles for sound effects
	/*
	* SFX0-9 use for one shots
	* SFX10-11 use for looping
	*/
	std::unique_ptr<DirectX::SoundEffect> m_SFX0;
	std::unique_ptr<DirectX::SoundEffect> m_SFX1;
	std::unique_ptr<DirectX::SoundEffect> m_SFX2;
	std::unique_ptr<DirectX::SoundEffect> m_SFX3;
	std::unique_ptr<DirectX::SoundEffect> m_SFX4;
	std::unique_ptr<DirectX::SoundEffect> m_SFX5;
	std::unique_ptr<DirectX::SoundEffect> m_SFX6;
	std::unique_ptr<DirectX::SoundEffect> m_SFX7;
	std::unique_ptr<DirectX::SoundEffect> m_SFX8;
	std::unique_ptr<DirectX::SoundEffect> m_SFX9;
	std::unique_ptr<DirectX::SoundEffect> m_SFX10;
	std::unique_ptr<DirectX::SoundEffect> m_SFX11;

	//A bunch of handles for sound effect instances
	std::unique_ptr<DirectX::SoundEffectInstance> m_SFXInstance0;
	std::unique_ptr<DirectX::SoundEffectInstance> m_SFXInstance1;
	std::unique_ptr<DirectX::SoundEffectInstance> m_SFXInstance2;
	std::unique_ptr<DirectX::SoundEffectInstance> m_SFXInstance3;
};

class AudioManager
{
public:
	//Constructor(s)
	AudioManager();
	//Destructor
	~AudioManager();

	////////////////////////////////
	/// Setup, Update and Check ///
	//////////////////////////////

	//Initialisation of the audio engine
	void InitAudioEngine(HWND hMainWnd);
	//Update for the audio engine/ any looping sounds - one shots dont need updating
	void UpdateAudioEngine(std::unique_ptr<DirectX::SoundEffectInstance>& loopSound);
	//Called when a new audio device is detected
	bool OnNewAudioDevice() { return m_RetryAudio = true; }

	//////////////////////
	/// Functionality ///
	////////////////////

	//A function to load in a sound can be used for both one shots and looping sounds
	//as this won't play anything it is just loading them ready for use elsewhere
	void LoadSound(std::unique_ptr<DirectX::SoundEffect>& sourceSound, const wchar_t* filepath);
	//A function to play one-shot sounds
	void PlayOneShot(std::unique_ptr<DirectX::SoundEffect>& oneShot);
	//A function to play looping sounds
	void PlayLoopingSound(std::unique_ptr<DirectX::SoundEffect>& sourceSound, std::unique_ptr<DirectX::SoundEffectInstance>& loopSound, float loopVolume);
	//Suspend Audio Engine
	void SuspendAudioEngine();
	//////////////////
	/// Getter(s) ///
	////////////////

	DirectX::AudioEngine* GetAudioEngine() { return m_AudioEngine.get(); }
	SoundEffectManager& GetSFXManager() { return m_SFXManager; }
	DirectX::SoundEffect* GetSourceSound(std::unique_ptr<DirectX::SoundEffect>& sourceSound) { return sourceSound.get(); }
	DirectX::SoundEffectInstance* GetSFInstance(std::unique_ptr<DirectX::SoundEffectInstance>& instance) { return instance.get(); }
private:


	//Audio Engine variables used for set up and checking for audio
	//devices
	std::unique_ptr<DirectX::AudioEngine> m_AudioEngine;
	HDEVNOTIFY hNewAudio = nullptr;
	bool m_RetryAudio;
	SoundEffectManager m_SFXManager;


};


