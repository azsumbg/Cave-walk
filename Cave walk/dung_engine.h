#pragma once

#ifdef DUNGENGINE_EXPORTS
#define DUNGENGINE_API __declspec(dllexport)
#else
#define DUNGENGINE_API __declspec(dllimport)
#endif

#include <random>
#include <vector>

constexpr float map_width{ 1000.0f };
constexpr float map_height{ 800.0f };

constexpr float scr_width{ 800.0f };
constexpr float scr_height{ 600.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 650.0f };

enum class dirs { left = 0, down = 1, up = 2, right = 3, stop = 4 };

// CREATURE FLAGS *************************************

constexpr unsigned char evil1_flag{ 0b00000001 };
constexpr unsigned char evil2_flag{ 0b00000010 };
constexpr unsigned char evil3_flag{ 0b00000100 };
constexpr unsigned char evil4_flag{ 0b00001000 };
constexpr unsigned char evil5_flag{ 0b00010000 };

constexpr unsigned char hero_flag{ 0b10000001 };
constexpr unsigned char hero_club_flag{ 0b10000011 };
constexpr unsigned char hero_axe_flag{ 0b10000101 };
constexpr unsigned char hero_sword_flag{ 0b10001001 };

// OBJECT FLAGS ************************************

constexpr int16_t gold_flag{ 0b0000000000000001 };
constexpr int16_t club_flag{ 0b0000000000000010 };
constexpr int16_t axe_flag{ 0b0000000000000100 };
constexpr int16_t sword_flag{ 0b0000000000001000 };
constexpr int16_t potion_flag{ 0b0000000000010000 };
constexpr int16_t cloak_flag{ 0b0000000000100000 };
constexpr int16_t mail_flag{ 0b0000000001000000 };
constexpr int16_t crystal_flag{ 0b0000000010000000 };

constexpr int16_t stone_brick_flag{ 0b0000000100000000 };
constexpr int16_t red_brick_flag{ 0b0000001000000000 };

// MOVING FLAGS *************************************

constexpr unsigned char left_obst_flag{ 0b00000001 };
constexpr unsigned char right_obst_flag{ 0b00000010 };
constexpr unsigned char up_obst_flag{ 0b00000100 };
constexpr unsigned char down_obst_flag{ 0b00001000 };

constexpr unsigned char up_left_obst_flag{ 0b00000101 };
constexpr unsigned char up_right_obst_flag{ 0b00000110 };
constexpr unsigned char down_left_obst_flag{ 0b00001001 };
constexpr unsigned char down_right_obst_flag{ 0b00001010 };

namespace dll
{
	class DUNGENGINE_API RANDiT
	{
		private:
			std::random_device rd{};
			std::seed_seq* sq{ nullptr };
			std::vector<unsigned int> vSeeds;
			std::mt19937* twister{ nullptr };

		public:
			RANDiT();
			~RANDiT();

			int operator()(int min, int max);
	};

	class DUNGENGINE_API PROTON
	{
		protected:
			float width{ 1.0f };
			float height{ 1.0f };

		public:
			float x{ 0 };
			float y{ 0 };
			float ex{ 0 };
			float ey{ 0 };

			PROTON(float _x = 0, float _y = 0, float _width = 1.0f, float _height = 1.0f);
			virtual ~PROTON() {};

			float GetWidth() const;
			float GetHeight() const;

			void SetWidth(float new_width);
			void SetHeight(float new_height);

			void SetEdges();
			void NewDims(float new_width, float new_height);
	};

	class DUNGENGINE_API PROT_CONTAINER
	{
		private:
			PROTON* m_ptr{ nullptr };
			size_t max_size{ 0 };
			size_t next_element{ 0 };

		public:
			explicit PROT_CONTAINER(size_t lenght);
			~PROT_CONTAINER();

			void push_back(PROTON& element);
			void push_front(PROTON& element);
			bool is_valid() const;
			size_t size() const;

			PROTON operator[](size_t index) const;
			void operator()(size_t index, PROTON& element);
	};

	class DUNGENGINE_API BASE_ASSETS_CLASS :public PROTON
	{
		protected:
			int16_t type_flag{ 0 };

		public:

			BASE_ASSETS_CLASS(int16_t _type, float where_x, float _where_y);
			virtual ~BASE_ASSETS_CLASS() {};

			bool CheckFlag(int16_t which_flag) const;
			void SetFlag(int16_t which_flag);
			void NullFlag(int16_t which_flag);
	};

	class DUNGENGINE_API BASE_CREATURE_CLASS :public PROTON
	{
		protected:
			unsigned char type_flag{ 0 };
			unsigned char obstacle_flag{ 0 };

			float start_x{ 0 };
			float start_y{ 0 };
			float end_x{ 0 };
			float end_y{ 0 };

			float slope{ 0 };
			float intercept{ 0 };

			bool hor_line = false;
			bool vert_line = false;

			float speed{ 1.0f };

			int frame_delay{ 0 };
			int current_frame{ 0 };
			int max_frames{ 0 };

			int strenght{ 0 };
			int attack_delay{ 0 };

			void SetLineInfo(float _end_x, float _end_y);

			BASE_CREATURE_CLASS(unsigned char my_type, float first_x, float first_y);

		public:
			int lifes{ 0 };
			dirs dir{ dirs::stop };

			virtual ~BASE_CREATURE_CLASS() {};

			int GetFrame();
			int Attack();
			float Distance(POINT reference_point, POINT my_point);

			void SetObstacleFlag(unsigned char which_flag);
			unsigned char GetObstacleFlag() const;
			unsigned char Move(float gear, PROT_CONTAINER& Obstacles, bool need_new_path = false,
				float target_x = 0, float target_y = 0);
			
			void Release();
			void Transform(unsigned char to_what);

			friend BASE_CREATURE_CLASS* CreatureFactory(unsigned char what, float _start_x, float _start_y);
	};

	typedef PROT_CONTAINER* Container;
	typedef BASE_ASSETS_CLASS* asset_ptr;
	typedef BASE_CREATURE_CLASS* creature_ptr;

	DUNGENGINE_API asset_ptr AssetFactory(int16_t what, float _start_x, float _start_y);
}