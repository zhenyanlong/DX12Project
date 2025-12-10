#pragma once
class GeneralEvent
{
	bool m_hasBeginPlay = false;
public:
	void BeginPlay()
	{
		if (!m_hasBeginPlay)
		{
			OnBeginPlay();
			m_hasBeginPlay = true;
		}
		
	}
	void Tick(float dt) 
	{
		OnTick(dt);
	}

protected:
	virtual void OnBeginPlay()
	{}
	virtual void OnTick(float dt)
	{}
};

