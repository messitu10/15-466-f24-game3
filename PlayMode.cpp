#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <cmath>

GLuint gun_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > gun_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("gun.pnct"));
	gun_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > gun_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("gun.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = gun_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = gun_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > laser_shoot_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Laser_shoot.wav"));
});

Load< Sound::Sample > laser_hit_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Hit_hurt.wav"));
});

Load< Sound::Sample > enemy_laser_shoot_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("Laser_shoot_enemy.wav"));
});

PlayMode::PlayMode() : scene(*gun_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Gun") gun = &transform;
		else if (transform.name == "Crosshair") crosshair = &transform;
		else if (transform.name == "Enemy1") enemy1 = &transform;
		else if (transform.name == "Enemy2") enemy2 = &transform;
		else if (transform.name == "Enemy3") enemy3 = &transform;
		else if (transform.name == "Enemy4") enemy4 = &transform;
		else if (transform.name == "Enemy5") enemy5 = &transform;
		else if (transform.name == "Enemy6") enemy6 = &transform;
		else if (transform.name == "Enemy7") enemy7 = &transform;
		else if (transform.name == "Enemy8") enemy8 = &transform;
		else if (transform.name == "Wall1") wall_1 = &transform;
		else if (transform.name == "Wall2") wall_2 = &transform;
		else if (transform.name == "Wall3") wall_3 = &transform;
		else if (transform.name == "Wall4") wall_4 = &transform;
		else if (transform.name == "Wall5") wall_5 = &transform;
		else if (transform.name == "Wall6") wall_6 = &transform;
		else if (transform.name == "Wall7") wall_7 = &transform;
		else if (transform.name == "Wall8") wall_8 = &transform;

	}
	if (gun == nullptr) throw std::runtime_error("Gun not found.");
	if (crosshair == nullptr) throw std::runtime_error("Crosshair not found.");

	if (enemy1 == nullptr) throw std::runtime_error("Enemy1 not found.");
	if (enemy2 == nullptr) throw std::runtime_error("Enemy2 not found.");
	if (enemy3 == nullptr) throw std::runtime_error("Enemy3 not found.");
	if (enemy4 == nullptr) throw std::runtime_error("Enemy4 not found.");
	if (enemy5 == nullptr) throw std::runtime_error("Enemy5 not found.");
	if (enemy6 == nullptr) throw std::runtime_error("Enemy6 not found.");
	if (enemy7 == nullptr) throw std::runtime_error("Enemy7 not found.");
	if (enemy8 == nullptr) throw std::runtime_error("Enemy8 not found.");
	
	if (wall_1 == nullptr) throw std::runtime_error("Wall1 not found.");
	if (wall_2 == nullptr) throw std::runtime_error("Wall2 not found.");
	if (wall_3 == nullptr) throw std::runtime_error("Wall3 not found.");
	if (wall_4 == nullptr) throw std::runtime_error("Wall4 not found.");
	if (wall_5 == nullptr) throw std::runtime_error("Wall5 not found.");
	if (wall_6 == nullptr) throw std::runtime_error("Wall6 not found.");
	if (wall_7 == nullptr) throw std::runtime_error("Wall7 not found.");
	if (wall_8 == nullptr) throw std::runtime_error("Wall8 not found.");

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	countdown_timer = 60.0f;

	gun_offset = glm::vec3(-0.02f, -0.5f, 0.7f);
	gun->position += gun_offset;

	set_enemy_spawn_locations();

	gunshot_gap_time = 0.0f;
	enemy_gunshot_gap_time = 3.0f;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		leftMouse.downs += 1;
		leftMouse.pressed = true;
		return true;
	} else if (evt.type == SDL_MOUSEBUTTONUP) {
		leftMouse.pressed = false;
		return true;
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);

			//I asked chatGPT how to avoid camera tilting. The codes about camera rotations here are instructed by chatGPT.
			glm::quat yaw_rotation = glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 right = camera->transform->rotation * glm::vec3(1.0f, 0.0f, 0.0f);
			glm::quat pitch_rotation = glm::angleAxis(motion.y * camera->fovy, right);
			camera->transform->rotation = glm::normalize(yaw_rotation * pitch_rotation) * camera->transform->rotation;
			return true;
		}
	}

	return false;
}

void PlayMode::set_enemy_spawn_locations(){
	enemies.emplace_back(enemy1);
	enemies.emplace_back(enemy2);
	enemies.emplace_back(enemy3);
	enemies.emplace_back(enemy4);
	enemies.emplace_back(enemy5);
	enemies.emplace_back(enemy6);
	enemies.emplace_back(enemy7);
	enemies.emplace_back(enemy8);

	spawn_enemy();
}

void PlayMode::fire() {
	if(gunshot_gap_time <= 0.0f){
		laser_gun_sound = Sound::play_3D(*laser_shoot_sample, 1.0f, gun->position, 20.0f);
		if(check_hit()){
			score += 1;
			spawn_enemy();
		}
		gunshot_gap_time += 0.5f;
	}
}

bool PlayMode::check_hit(){
	glm::mat4x3 crosshair_local_to_world_matrix = crosshair->make_local_to_world();
	glm::vec3 crosshair_position = glm::vec3(crosshair_local_to_world_matrix[3][0], crosshair_local_to_world_matrix[3][1], crosshair_local_to_world_matrix[3][2]);

	float x_or_y = std::max(std::abs(crosshair_position.x), std::abs(crosshair_position.y));

	if(x_or_y > 477.10f && x_or_y < 477.70f){
		laser_hit_sound = Sound::play_3D(*laser_hit_sample, 1.0f, enemies[current_enemy_index]->position, 200.0f);
	 	return true;
	}

	return false;
}

void PlayMode::spawn_enemy(){
	std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(0, enemies.size() - 1); 

	uint32_t new_index = distrib(gen);

	while (new_index == current_enemy_index)
	{
		new_index = distrib(gen);
	}
	
    current_enemy_index = distrib(gen);

    for (size_t i = 0; i < enemies.size(); ++i) {
        if (i == current_enemy_index) {
            enemies[i]->scale = glm::vec3(1.0f);
			play_enemy_shooting_sound();
        } else {
            enemies[i]->scale = glm::vec3(0.0f);
        }
    }
}

void PlayMode::play_enemy_shooting_sound(){
	enemy_laser_gun_sound = Sound::play_3D(*enemy_laser_shoot_sample, 1.0f, enemies[current_enemy_index]->position, 200.0f);
	enemy_gunshot_gap_time = 3.0f;
}

void PlayMode::update(float elapsed) {
	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	if (countdown_timer > 0.0f) {
        countdown_timer -= elapsed; 
        if (countdown_timer <= 0.0f) {
            countdown_timer = 0.0f; 
            std::cout << "Time's up!" << std::endl; 
        }
    }

	if(gunshot_gap_time > 0.0f){
		gunshot_gap_time -= elapsed;
	}

	if(gunshot_gap_time <= 0.0f && leftMouse.pressed){
		fire();
	}

	if(enemy_gunshot_gap_time > 0.0f){
		enemy_gunshot_gap_time -= elapsed;
	}
	else{
		play_enemy_shooting_sound();
	}

	//reset button press counters:
	leftMouse.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		std::string score_text = "Time: " + std::to_string((int)countdown_timer) + "     Score: " + std::to_string(score);

		
		constexpr float H = 0.2f;
		glm::vec3 top_left_position(-aspect + 0.1f * H, 0.7f - 0.1f * H, 0.0f);

		lines.draw_text(score_text,
			top_left_position,
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(score_text,
			top_left_position + ofs,
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

