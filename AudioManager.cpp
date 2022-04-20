#include "AudioManager.h"

AudioManager::AudioManager()
	:m_RetryAudio(false)
{
}

AudioManager::~AudioManager()
{
	
}

void AudioManager::InitAudioEngine(HWND hMainWnd)
{
	//Listen for new audio devices
	DEV_BROADCAST_DEVICEINTERFACE filter = {};
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = KSCATEGORY_AUDIO;

	//Notifys when an audio device has been found
	hNewAudio = RegisterDeviceNotification(hMainWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	DirectX::AUDIO_ENGINE_FLAGS engineFlags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
	engineFlags |= DirectX::AudioEngine_Debug;
#endif // DEBUG
	m_AudioEngine = std::make_unique<DirectX::AudioEngine>(engineFlags);

	if (!m_AudioEngine->IsAudioDevicePresent())
	{
		DBOUT("No audio device, we are in silent mode");
	}
	else if (m_AudioEngine->IsAudioDevicePresent())
	{
		DBOUT("Audio device");
	}
}

void AudioManager::UpdateAudioEngine(std::unique_ptr<DirectX::SoundEffectInstance>& loopSound)
{
	if (hNewAudio)
	{
		UnregisterDeviceNotification(hNewAudio);
		hNewAudio = nullptr;
	}

	if (m_RetryAudio)
	{
		m_RetryAudio = false;
		if (m_AudioEngine->Reset())
		{
			// Restart looped sounds here
			if (loopSound)
			{
				loopSound->Play(true);
			}
		}
	}
	else if (!m_AudioEngine->Update())
	{
		if (m_AudioEngine->IsCriticalError())
		{
			DBOUT("WE LOST THE AUDIO DEVICE");
			m_RetryAudio = true;
		}
	}
}

void AudioManager::LoadSound(std::unique_ptr<DirectX::SoundEffect>& sourceSound, const wchar_t* filepath)
{
	sourceSound = std::make_unique<DirectX::SoundEffect>(m_AudioEngine.get(), filepath);
}

void AudioManager::PlayOneShot(std::unique_ptr<DirectX::SoundEffect>& oneShot)
{
	oneShot->Play();
}

void AudioManager::PlayLoopingSound(std::unique_ptr<DirectX::SoundEffect>& sourceSound, std::unique_ptr<DirectX::SoundEffectInstance>& loopSound, float loopVolume)
{
	loopSound = sourceSound->CreateInstance();
	loopSound->SetVolume(loopVolume);
	loopSound->Play(true);
}

void AudioManager::SuspendAudioEngine()
{
	m_AudioEngine->Suspend();
}


