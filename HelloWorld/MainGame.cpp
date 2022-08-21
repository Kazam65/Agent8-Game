#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;
int retries = 0;
int difficulty = 150;

enum Agent8State {
	State_Appear = 0,
	State_Halt,
	State_Play,
	State_Dead,
};


struct GameState
{
	int score = 0;
	Agent8State agentState = State_Appear;
};
GameState gameState;

enum GameObjectType {  //enumeration will automatically assign next number (0 for  agent8) to game objects
	TYPE_NULL = -1,
	Type_Agent8,
	Type_Fan,
	Type_Tool,
	Type_Coin,
	Type_Star,
	Type_Laser,
	Type_Destroyed,
};

void HandlePlayerControls(); //it is necessary to declare functions like this when you want to refer to them before the code for them
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLasers();
void UpdateDestroyed();
void UpdateAgent8();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins(); //makes sprites centre on centre instead of top left
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(Type_Agent8, { 115,0 }, 50, "agent8");
	int id_fan = Play::CreateGameObject(Type_Fan, { 1140,217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0,3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	
	Play::DrawBackground();
	UpdateAgent8();
	
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLasers();
	UpdateDestroyed();

	Play::DrawFontText("64px", "Arrow Keys to move, Space to fire.", { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT - 30 }, Play::CENTRE);
	Play::DrawFontText("132px", "Score: " + std::to_string(gameState.score), { DISPLAY_WIDTH / 2,50 }, Play::CENTRE);
	Play::DrawFontText("132px", "Retries: " + std::to_string(retries), { 3 * DISPLAY_WIDTH / 4,50 }, Play::CENTRE);
	//VOID UPDATEAGENT8 MISSING

	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}


void UpdateAgent8() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(Type_Agent8);
	switch (gameState.agentState) {
		case State_Appear: //Does the intro animation
			obj_agent8.velocity = { 0,12 };
			obj_agent8.acceleration = { 0,0.5f };
			Play::SetSprite(obj_agent8, "agent8_fall", 0);
			obj_agent8.rotation = 0;
			if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3) {
				gameState.agentState = State_Play;
			
			}
			break;

		case State_Halt:
			obj_agent8.velocity *= 0.9f;
			if (Play::IsAnimationComplete(obj_agent8)) {
				gameState.agentState = State_Play;
			}
			break;

		case State_Play:
			HandlePlayerControls();
			break;

		case State_Dead: //restart, maybe add a counter
			obj_agent8.acceleration = { -0.3f,0.5f };
			obj_agent8.rotation += 0.25f;
			if (Play::KeyPressed(VK_SPACE)) {
				gameState.agentState = State_Appear;
				obj_agent8.pos = { 115,0 };
				obj_agent8.velocity = { 0,0 };
				obj_agent8.frame = 0;
				Play::StartAudioLoop("music");
				gameState.score = 0;
				retries += 1;

				for (int objId : Play::CollectGameObjectIDsByType(Type_Tool)) {
					Play::GetGameObject(objId).type = Type_Destroyed;
				}
			}
			break;

	}
	Play::UpdateGameObject(obj_agent8);
	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != State_Dead) {
		obj_agent8.pos = obj_agent8.oldPos;

	}

	Play::DrawLine({ obj_agent8.pos.x,0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);




}


void UpdateFan() {
	GameObject& obj_fan = Play::GetGameObjectByType(Type_Fan);
	if (Play::RandomRoll(difficulty) == 1 && gameState.agentState != State_Dead) {
		int id = Play::CreateGameObject(Type_Tool, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);
		if (difficulty > 30) {
			difficulty -= 1;
		}
		if (Play::RandomRoll(2) == 1) {
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4 - ((100 - difficulty)/35); //speed of tools scales with difficulty capped at a fastest limit
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");

	}

	if (Play::RandomRoll(150) == 50) { //makes coins
		int id = Play::CreateGameObject(Type_Coin, obj_fan.pos, 40, "coin");
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = {-5,0};
		obj_coin.rotSpeed = 0.1f;
		



	}

	Play::UpdateGameObject(obj_fan);

	if (Play::IsLeavingDisplayArea(obj_fan)) {
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;

	}
	
	
	Play::DrawObject(obj_fan);
}

void UpdateTools() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(Type_Agent8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(Type_Tool);

	for (int id : vTools) {
		GameObject& obj_tool = Play::GetGameObject(id);

		if (gameState.agentState!=State_Dead && Play::IsColliding(obj_tool, obj_agent8)){
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			gameState.agentState = State_Dead;
			obj_agent8.pos = { -100, -100 };

		}

		Play::UpdateGameObject(obj_tool);

		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL)) {
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;

		}

		Play::DrawObjectRotated(obj_tool);

		if (!Play::IsVisible(obj_tool)) {
			Play::DestroyGameObject(id);

		}

	}
}

void UpdateCoinsAndStars() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(Type_Agent8);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(Type_Coin);

	for (int coinId : vCoins) {
		GameObject& obj_coin = Play::GetGameObject(coinId);
		
		bool hasCollided = false;

		if (Play::IsColliding(obj_coin, obj_agent8)) { //checks if coin is collected to increase score
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f) {
				int id = Play::CreateGameObject(Type_Star, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);

			}

			hasCollided = true;
			gameState.score += 500; //increases score when collecting coins
			Play::PlayAudio("collect");

		}

		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);
		if (!Play::IsVisible(obj_coin) || hasCollided) { //removes coin if it goes off screen or gets collected
			Play::DestroyGameObject(coinId);

		}


	}

	std::vector<int> vStars = Play::CollectGameObjectIDsByType(Type_Star);

	for (int id_star : vStars) {
		GameObject& obj_star = Play::GetGameObject(id_star);

		Play::UpdateGameObject(obj_star);
		Play::DrawObjectRotated(obj_star);

		if (!Play::IsVisible(obj_star)) {
			Play::DestroyGameObject(id_star);
		}
	}


}

void UpdateLasers() {
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(Type_Laser);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(Type_Tool);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(Type_Coin);

	for (int laserId : vLasers) {
		GameObject& obj_laser = Play::GetGameObject(laserId);
		bool hasCollided = false;
		for (int toolId : vTools) {
			GameObject& obj_tool = Play::GetGameObject(toolId);
			if (Play::IsColliding(obj_laser, obj_tool)) { //adds score for every tool destroyed
				hasCollided = true;
				obj_tool.type = Type_Destroyed;
				gameState.score += 100;

			}
		}

		for (int coinId : vCoins) {
			GameObject& obj_coin = Play::GetGameObject(coinId);
			if (Play::IsColliding(obj_laser, obj_coin)) {  //subtracts score if the player shoots coins instead of collecting them
				hasCollided = true;
				obj_coin.type = Type_Destroyed;
				Play::PlayAudio("error");
				gameState.score -= 300;
			}
		}

		if (gameState.score < 0) { //stops score being negative
			gameState.score = 0;
		}

		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		if (!Play::IsVisible(obj_laser) || hasCollided) { //remove lasers when they hit things or go off screen
			Play::DestroyGameObject(laserId);

		}

	}
}

void UpdateDestroyed() {
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(Type_Destroyed);

	for (int deadId : vDead) {
		GameObject& obj_dead = Play::GetGameObject(deadId);
		obj_dead.animSpeed = 0.2f;
		Play::UpdateGameObject(obj_dead);

		if (obj_dead.frame % 2) {
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);

		}
		if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10) {
			Play::DestroyGameObject(deadId);
		}
	}


}

void HandlePlayerControls()
{	//this does the controls for the spider
	GameObject& obj_agent8 = Play::GetGameObjectByType(Type_Agent8);
	if (Play::KeyDown(VK_UP)) {
		obj_agent8.velocity = { 0,-4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);

	}
	else if (Play::KeyDown(VK_DOWN)) {
		obj_agent8.acceleration = { 0,0.5f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);

	}
	else {
		if (obj_agent8.velocity.y > 5) {
			gameState.agentState = State_Halt;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0,0 };

		}
		else {
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0,0 };
		}
		
	}

	if (Play::KeyPressed(VK_SPACE)) { //makes lasers shoot when space key pressed
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(Type_Laser, firePos, 30, "laser");
		Play::GetGameObject(id).velocity = { 32,0 };
		Play::PlayAudio("shoot");

	}


	Play::UpdateGameObject(obj_agent8);


	if (Play::IsLeavingDisplayArea(obj_agent8)) { //Stops you going out of bounds
		obj_agent8.pos = obj_agent8.oldPos;

	}

	Play::DrawLine({ obj_agent8.pos.x,0 }, obj_agent8.pos, Play::cWhite); //this makes the web string to top of screen
	Play::DrawObjectRotated(obj_agent8);


}
// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

