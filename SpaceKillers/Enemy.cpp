#include "Enemy.hpp"

#include "Game.hpp"

Enemy::Enemy()
	:
	sf::Sprite(),
	mEnemySpeed(75.0f),
	mScoreValue(100)
	{

	}

Enemy::~Enemy()
	{

	}

void Enemy::Update()
	{
	// get time information
	const sf::Time & frameDelta = gpGame->GetFrameDelta();
	const sf::Time & frameStamp = gpGame->GetFrameTimeStamp();

	// get the direction to evade if there is nothing to evade EvadeDir::Null is returned.
	EvadeDir evadeDir = GetEvadeDirection();

	if ( evadeDir != EvadeDir::Null )
		{
		// need to evade in a particular direction
		sf::Vector2f evadeVec(0.0f, mDecidedDirection.y);
		if(evadeDir == EvadeDir::Left)
			{
			evadeVec.x = -1.0f;
			}
		if(evadeDir == EvadeDir::Right)
			{
			evadeVec.x = 1.0f;
			}
		// move in this direction and keep the decided forward speed.
		move( evadeVec * mEnemySpeed * frameDelta.asSeconds() );
		}
	else
		{
		// since there was nothing to evade, do random decisions and keep moving in a general
		// downward direction.
		if ( mTriggerNextDecision.asSeconds() <= frameStamp.asSeconds() )
			{
			mDecidedDirection.x = Random::FloatBetween(-1.0f, 1.0f);
			mDecidedDirection.y = Random::FloatBetween(0.5f, 1.0f);
			mTriggerNextDecision = frameStamp + sf::seconds(Random::FloatBetween(0.5f, 2.0f));
			}
		move(mDecidedDirection * mEnemySpeed * frameDelta.asSeconds());
		}

	// check out of bounds tell enemy to move inwards
	const sf::FloatRect & enemyRect = getGlobalBounds();
	const float minSpeedForEdgeEvoid = .25f;

	if ( enemyRect.left < 0.0f )
		{
		mDecidedDirection.x = 1.0f;
		}
	if ( enemyRect.left + enemyRect.width > gpGame->GetWindow().getSize().y )
		{
		mDecidedDirection.x = -1.0f;
		}

	// shooting code.
	sf::FloatRect areaOfAttack = getGlobalBounds();
	float growWidth = areaOfAttack.width * 2.0f;
	areaOfAttack.width += growWidth;
	areaOfAttack.left -= growWidth / 2.0f;
	areaOfAttack.height = gpGame->GetWindow().getSize().y - areaOfAttack.top;

	if ( areaOfAttack.intersects( gpGame->GetPlayer().getGlobalBounds() ) )
		{
		Shoot();
		mTriggerNextRandomShot = gpGame->GetFrameTimeStamp() + sf::seconds(Random::FloatBetween(.5f, 3.5f));
		}
	else
		{
		if ( mTriggerNextRandomShot <= gpGame->GetFrameTimeStamp() )
			{
			Shoot();
			mTriggerNextRandomShot = gpGame->GetFrameTimeStamp() + sf::seconds( Random::FloatBetween(.5f, 3.5f) );
			}
		}

	}

sf::Time Enemy::GetTriggerNextDecision() const
	{
	return mTriggerNextDecision;
	}

void Enemy::SetTriggerNextDecision( sf::Time timeTrigger )
	{
	mTriggerNextDecision = timeTrigger;
	}

EvadeDir Enemy::GetEvadeDirection() const
	{
	// Get first shot in front of this enemy
	auto lasersPlayer = gpGame->GetLasersPlayer();

	const sf::Vector2f & enemyPos = getPosition();
	const sf::FloatRect & enemyRect = getGlobalBounds();

	// create rect that if the laser's rect touches, that laser should be evaded
	sf::FloatRect evadeAreaOfEffect = enemyRect; // set to the enemy rect initially
	float scaleRatio = 1.0f; // the ratio to scale by
	float widthToAdd = evadeAreaOfEffect.width * scaleRatio; // the calculated width to add
	evadeAreaOfEffect.width += widthToAdd; // add extra width
	evadeAreaOfEffect.left -= widthToAdd / 2.0f; // move rect left by half of added width so it's centered.
	evadeAreaOfEffect.height = gpGame->GetWindow().getSize().y - evadeAreaOfEffect.top; // grow height of rect to bottom of screen.

	for ( auto & laser : lasersPlayer )
		{
		const sf::FloatRect & laserRect( laser.getGlobalBounds() );

		// using intersects to detect collision between the area we have created and the laser rect.
		if ( evadeAreaOfEffect.intersects( laserRect ) )
			{
			// must evade this object

			// determine which direction to evade.
			float enemyMidpointX = enemyRect.left + (enemyRect.width / 2.0f);
			if ( enemyMidpointX <= laserRect.left + (laserRect.width / 2.0f) )
				{
				// laser is on right half, so evade left.
				return EvadeDir::Left;
				}
			else
				{
				// laser is on left half, so evade right.
				return EvadeDir::Right;
				}
			}
		}

	// if we got here no lasers were heading in this enemies direction so keep going
	return EvadeDir::Null;
	}

int Enemy::GetScoreValue() const
	{
	return mScoreValue;
	}

void Enemy::SetScoreValue( const int scoreValue )
	{
	mScoreValue = scoreValue;
	}

void Enemy::Shoot()
	{
	if ( mTriggerCanShoot <= gpGame->GetFrameTimeStamp() )
		{
		gpGame->CreateEnemyLaser(*this);
		mTriggerCanShoot = gpGame->GetFrameTimeStamp() + sf::seconds(Random::FloatBetween(.5f, 1.0f));
		}
	}