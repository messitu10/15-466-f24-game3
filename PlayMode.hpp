#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	bool check_hit();
	void fire();
	void set_enemy_spawn_locations();
	void spawn_enemy();	
	void play_enemy_shooting_sound();
	

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, leftMouse;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *gun = nullptr;
	Scene::Transform *crosshair = nullptr;

	Scene::Transform *enemy1 = nullptr;
	Scene::Transform *enemy2 = nullptr;
	Scene::Transform *enemy3 = nullptr;
	Scene::Transform *enemy4 = nullptr;
	Scene::Transform *enemy5 = nullptr;
	Scene::Transform *enemy6 = nullptr;
	Scene::Transform *enemy7 = nullptr;
	Scene::Transform *enemy8 = nullptr;

	Scene::Transform *wall_1 = nullptr;
	Scene::Transform *wall_2 = nullptr;
	Scene::Transform *wall_3 = nullptr;
	Scene::Transform *wall_4 = nullptr;
	Scene::Transform *wall_5 = nullptr;
	Scene::Transform *wall_6 = nullptr;
	Scene::Transform *wall_7 = nullptr;
	Scene::Transform *wall_8 = nullptr;

	float countdown_timer;

	std::vector<Scene::Transform*> enemies;
	uint32_t current_enemy_index;

	glm::vec3 gun_offset;
	float gunshot_gap_time;
	float enemy_gunshot_gap_time;

	uint32_t score;

	std::shared_ptr< Sound::PlayingSample > laser_gun_sound;
	std::shared_ptr< Sound::PlayingSample > laser_hit_sound;
	std::shared_ptr< Sound::PlayingSample > enemy_laser_gun_sound;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
