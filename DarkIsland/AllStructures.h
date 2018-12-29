#pragma once
#include "pch.h"
#include "../Bullet/src/btBulletDynamicsCommon.h"

//#include "DefParticle.h"
#include "DefShader.h"

namespace Game
{
	typedef struct
	{
		float impulse;
		int model_index;
		X3DAUDIO_LISTENER Listener;// = { 0 };
		X3DAUDIO_EMITTER Emitter;// = { 0 };
		X3DAUDIO_DSP_SETTINGS DSPSettings;
	} stuffplaysound_t;

	struct CB_FXAA
	{
		XMFLOAT4 m_fxaa;
	};

	typedef struct
	{
		XMFLOAT3 pos;
		XMFLOAT3 dir; // camera point direction
	} PathPoint;

	typedef struct
	{
		int spot_index;
		XMFLOAT4X4 dirView; // Directional Light View
		XMFLOAT4X4 dirProj; // Directional Light Projection
	} SpotShadow;

	typedef struct
	{
		int path_type;
		std::vector<PathPoint> path_points;
	} TerrainPath;

	typedef struct
	{
		XMFLOAT3 eye;
		XMFLOAT3 at;
	} CameraPathPoint;

	struct ScreenSizeBufferType
	{
		float width_size;
		float height_size;
		XMFLOAT2 padding;
	};

	struct SkyBufferType
	{
		float translationx;
		float translationz;
		float scale;
		float brightness;
	};

	struct CG_POINT_LIGHT
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT3 pos;

		float _specular_power;
		float fire_scale;
		float dist; // distance from camera
		float radius; // radius of light

		bool bCastShadows;
	};
	struct CG_SPOT_LIGHT
	{
		float radius;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT3 pos;
		XMFLOAT3 dir;
		XMFLOAT3 up;
		int lightmap;
		float spot;

		float _specular_power;
		bool bCastShadows;
	};

	struct PointLightType
	{
		float radius;
		XMFLOAT3 pos;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT3 dir;
		float spot;
		float specular_power;
		unsigned int shadow_index;
		float spare;
		int lightmap;
		XMFLOAT4X4 dirView; // Directional Light View
		XMFLOAT4X4 dirProj; // Directional Light Projection
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float specularPower;
		XMFLOAT4 specularColor;
		XMFLOAT4 fogColor;
		XMFLOAT4 skyColor;

		XMFLOAT3 eye_position;
		unsigned int numPointLights;
		unsigned int numSpotLights;
		float lightning;
		XMFLOAT3 lightning_dir;
		XMFLOAT3 pos;
		float screen_width;
		float screen_height;
		float texel_width;
		float texel_height;
	};

	struct Vertex
	{
		float x, y, z;
		float nx, ny, nz;
		float tx, ty, tz, tw;
		UINT color;
		float u, v;
	};

	struct VertexData
	{
	public:
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT4 tan;
		UINT color;
		XMFLOAT2 tex;
		//XMFLOAT3 bin; // binormal

		void SetNormal(float x, float y, float z) { norm = XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = XMFLOAT2(u, v); }

		void SetTangent(float x, float y, float z, float w) { tan = XMFLOAT4(x, y, z, w); }
		void SetBinormal(float x, float y, float z) {  }

		void SetColor(XMFLOAT4 c) {/* col = c;*/ }
	};

	struct GroundVertexData
	{
	public:
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT2 tex;
		float blend;
		float blend_2;
		float blend_3;
		float blend_4;
		float blend_5;
		float blend_6;
		float blend_7;
		float blend_8;

		XMFLOAT4 col;
		XMFLOAT3 tan;
		XMFLOAT3 bin; // binormal

		void SetNormal(float x, float y, float z) { norm = XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = XMFLOAT2(u, v); }

		void SetBlend(float b) { blend = b; }
		void SetBlend2(float b) { blend_2 = b; }
		void SetBlend3(float b) { blend_3 = b; }
		void SetBlend4(float b) { blend_4 = b; }
		void SetBlend5(float b) { blend_5 = b; }
		void SetBlend6(float b) { blend_6 = b; }
		void SetBlend7(float b) { blend_7 = b; }
		void SetBlend8(float b) { blend_8 = b; }

		void SetColor(float r, float g, float b, float a) { col.x = r; col.y = g; col.z = b; col.w = a; }

		void SetTan(float x, float y, float z) { tan = XMFLOAT3(x, y, z); }
		void SetBin(float x, float y, float z) { bin = XMFLOAT3(x, y, z); }
	};

	struct WaterVertexData
	{
	public:
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT2 tex;
		float blend;
		float blend_2;
		XMFLOAT4 col;
		XMFLOAT3 tan;
		XMFLOAT3 bin; // binormal

		void SetNormal(float x, float y, float z) { norm = XMFLOAT3(x, y, z); }
		void SetPosition(float x, float y, float z) { pos = XMFLOAT3(x, y, z); }
		void SetTex(float u, float v) { tex = XMFLOAT2(u, v); }

		void SetBlend(float b) { blend = b; }
		void SetBlend2(float b) { blend_2 = b; }
		void SetColor(float r, float g, float b, float a) { col.x = r; col.y = g; col.z = b; col.w = a; }
		void SetTan(float x, float y, float z) { tan = XMFLOAT3(x, y, z); }
		void SetBin(float x, float y, float z) { bin = XMFLOAT3(x, y, z); }
	};

	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		XMFLOAT4X4 model;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 mvp; // pre calculated
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		XMFLOAT3 pos;
		XMFLOAT3 color;
	};

	struct HeightMapInfo {				// Heightmap structure
		int terrainWidth;				// Width of heightmap
		int terrainHeight;				// Height (Length) of heightmap
		XMFLOAT3 *heightMap;	// Array to store terrain's vertex positions
		XMFLOAT3 *normal;		// Array to store terrain's vertex positions
		XMFLOAT4 *colour;		// Array to store terrain's vertex positions
	};

	struct VertexPositionTex
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};

	struct VertexPositionTexCol
	{
		XMFLOAT3 pos;
		XMFLOAT4 col;
		XMFLOAT2 tex;
		uint tex_num;
		XMFLOAT2 padding;
	};

	struct ParticleInstance
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
		float padding_b;
	};

	typedef struct
	{
		float x, y, z;
	}VertexType;

	typedef struct
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	}FaceType;

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	struct NoiseBufferType
	{
		float frameTime;
		XMFLOAT3 scrollSpeeds;
		XMFLOAT3 scales;
		float padding;
	};

	struct DistortionBufferType
	{
		XMFLOAT2 distortion1;
		XMFLOAT2 distortion2;
		XMFLOAT2 distortion3;
		float distortionScale;
		float distortionBias;
	};

	struct MaterialBufferType
	{
		MaterialBufferType()
		{
			static_assert((sizeof(MaterialBufferType) % 16) == 0, "CB must be 16-byte aligned");
			Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
			Specular = XMFLOAT4(0.0f, 0.3f, 0.0f, 1.0f);
			Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			SpecularPower = 30.0f;
			normal_height = 0.0f;
			emmit_brightness = 0.0f;
			Padding2 = 0.0f;
		}

		XMFLOAT4   Ambient;
		XMFLOAT4   Diffuse;
		XMFLOAT4   Specular;
		XMFLOAT4   Emissive;
		float               SpecularPower;
		float               normal_height;
		float               emmit_brightness;
		float               Padding2;
	};

	typedef struct
	{
		int part_no;
		int tail_no;
		float dist;
	} part_index;

	typedef struct
	{
		float x;
		float y;
		float z;
	}norm_t;

	struct ObjInfo
	{
		XMFLOAT3 pos;
		XMFLOAT3 dir; // yaw pitch roll or vector
		XMFLOAT3 mrf; // mass restitution friction
		int item_id;
		int group;
		int mask;
	};

	struct LevelInfo
	{
		int db_rec_id;
		int level;
		char terrain_tex1[40];
		char terrain_tex2[40];
		char terrain_tex3[40];
		char terrain_tex4[40];
		char terrain_tex5[40];
		char terrain_tex6[40];
		char terrain_tex7[40];
		char terrain_tex8[40];
		char terrain_normal1[40];
		char terrain_normal2[40];
		char terrain_normal3[40];
		char terrain_normal4[40];
		char terrain_normal5[40];
		char terrain_normal6[40];
		char terrain_normal7[40];
		char terrain_normal8[40];
		char skybox[40];
		XMFLOAT3 diff_dir; // diffuse light direction
		XMFLOAT4 diffuse_col; // diffuse light col
		XMFLOAT4 ambient_col; // diffuse light col
		XMFLOAT4 specular_col; // diffuse light col
		XMFLOAT4 fog_col; // diffuse light col
		XMFLOAT4 dust_col; // dust light col
		float specular_power;
		float ground_friction;
		float ground_restitution;
		float player_start_x;
		float player_start_y;
		float player_start_z;
		float player_start_angle;
		int bRain;
		int bSnow;
		int bFog;
		float ground_steepness_blend;
		XMFLOAT4 wind; // wind - x & z = dir and y = strength & w = wave scale
		float flag_angle;
		char music_track[40];

		float spot_distance;
		float point_distance;
		float sky_brightness;
		float sky_ambient;
		float sky_diffuse;
		int start_sound;
		int start_music;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
}