#pragma once

//#include "Coin.h"

#include "Terrain.h"
#include "TerrainPaths.h"
#include "Water.h"
#include "Camera.h"

#include "AllResources.h"
#include "../DirectXTK/Inc/DDSTextureLoader.h"

/*
#pragma pack(2) // Add this

typedef struct
{
unsigned short bfType;
unsigned int   bfSize;
unsigned short bfReserved1;
unsigned short bfReserved2;
unsigned int   bfOffBits;
} BITMAPFILEHEADER;

#pragma pack() // and this
*/

namespace Game
{
	class Level
	{
	public:
		Level(AllResources* p_Resources);
		~Level(void);

		int total_planes;

		int left_wall_id;
		int right_wall_id;
		int front_wall_id;
		int back_wall_id;

		float scale_height;

		float** stack_height; // used when objects are stacked
		float** shadows;		// used to work out shadows on terrain

								/*
								float3 player_pos;
								*/

		float*** tex_blend;

		void ClearBlendEdit(int _blendnum);
		void SetAllLevelTexture(int tex);

		void SaturateTexBlend(int pointy, int pointx);

		void CalculateRoofHeightmap();

		btRigidBody* m_FloorPlane;
		btRigidBody* m_GroundPlane;

		btRigidBody* m_FrontPlane;
		btRigidBody* m_BackPlane;
		btRigidBody* m_LeftPlane;
		btRigidBody* m_RightPlane;

		int map_x_size, map_y_size;

		int total_platforms;

		float left_right_walls;
		float front_back_walls;

		float back_position;

		float* cam_z;

		float** height_map_to; // pinch height map
		float** height_map;

		float** roof_height_map;

		float** wave_height_map; // water waves
		bool** wave_active_map; // water waves

		float** steepness;

		norm_t** normals;
		XMFLOAT4** colours;

		bool** bPlaneUpdate;

		float highest_point;
		float lowest_point;

		int total_x_points;
		int total_y_points;

		char ground1[30];
		char ground2[30];
		float noise_scale;

		float local_terrain_scale;

		int x_planes;// = ((map_x_size) / 9);
		int y_planes;// = ((map_y_size) / 9);

		int lev_num;

		float flerp(float a, float b, float f);

		int clampB(int pX, int pMax);

		void UpdateVertexBuffers(bool _update_all);

		void CreateNormals(bool _update_all);

		void UpdateWaveheightmap(float timeDelta, float timeTotal);

		void MakeNewTerrain(float val1, float val2, float val3, float val4);
		void FlattenTerrain();

		void InitializeGeneratedTerrain(int level, float height_scale, float noise_z);

		void UpdateGeneratedTerrainPinch(int level, float noise_z, float xPos, float zPos, float height, float width, bool fixed_height = false);
		void UpdateGeneratedTerrainTex(float xPos, float zPos, float width, int tex_num);
		void UpdateGeneratedTerrainPath(float xPos, float yPos, float zPos, float width, int tex_num, bool _dip_path = false);
		void BakePinch(bool _bakeall);

		void ClearPlaneUpdate();

		void InitializeTerrainVariables(int x_nodes, int y_nodes, float terrain_scale);

		void Update(XMMATRIX *viewMatrix, float timeTotal);

		void UpdatePhysics();

		void Render();
		void RenderWater();

		void BakeAll();

		void SetWaterActive();
		Concurrency::task<void> Level::Update(float timeDelta, float timeTotal);
		float GetTerrainHeight(float xPos, float zPos);
		float GetRoofHeight(float zPos, float xPos);

		float GetWaterHeight(float zPos, float xPos);

		void LoadBinary(int level);
		bool SaveBinary(int level);

		//void UpdateTerrain(bool _update_all = false);
		concurrency::task<void> Level::UpdateTerrain(bool _update_all);

		void RemovePhysics();
		void MakePhysics(bool _makewalls = true, bool _bMakeAll = true);

		btVector3 Level::GetNormal(float xPos, float zPos);

		void SetFriction(float fric);
		void SetRestitution(float rest);

		concurrency::task<void> LoadLevelTextures();

		concurrency::task<void> LoadWaterTextures();

		void ClearMemory();

		std::vector<Terrain*> rm_Terrain;

		std::vector<Terrain> m_Terrain;
		std::vector<Water> m_Water;
		//Terrain* m_Terrain[1024];
		//Water* m_Water[1024];

		TerrainPaths* m_Paths;

		int current_location;
		bool bLoadedShadows;

		HeightMapInfo m_hminfo;

		Physics* m_phys;
		float m_terrain_scale;

		int fin_state;

		MaterialBufferType mat_buffer;

		ID3D11ShaderResourceView* m_TextureArray[8];
		ID3D11ShaderResourceView* m_NormalArray[8];

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Water_Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Water_Texture_2;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Water_Texture_3;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Water_Norm;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_Water_Norm2;
		/*
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return m_vertexBuffer; }
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return m_indexBuffer; }

		void SetVertexBuffer();
		void SetIndexBuffer();

		vector<GroundVertexData> m_vertices;
		vector<unsigned short> m_indices;
		*/

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		AllResources* m_Res;
	};
}