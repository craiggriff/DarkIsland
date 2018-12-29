#pragma once
#include <fstream>
#include <string>
#include <vector>
//#include "DirectXHelper.h"
//#include "VertexData.h"
//#include "MVPConstantBuffer.h"

#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "../Bullet/src/btBulletDynamicsCommon.h"

#include "AllResources.h"
//#include "DDSTextureLoader.h"
#include "Physics.h"
//#include "Object3D.h"

//using namespace DirectX;
//using namespace DX;
//using namespace Microsoft::WRL;

namespace Game
{
	class Water
	{
	public:
		Water();
		~Water(void);

		// Constant buffer
		btTransform m_initialTransform;
		ModelViewProjectionConstantBuffer	m_constantBufferData;

		AllResources* m_Res;

		bool bPhysicsObject;

		int xcoords, ycoords;

		int xblockpos, yblockpos;

		bool bLoaded;

		bool bUpdateable;

		int start_id, end_id;

		void CreateBuffers();

		bool CreateUpdatebleVertexBuffer();
		bool UpdateUpdatebleVertexBuffer();
		bool LoadTerrainHeight(float** whole_height_map);
		void MakeIndices();

		void GetActiveVerts(bool** water_active_map);

		bool** b_ren;
		float** height_map;
		float** land_dist;
		XMFLOAT3** normals;
		XMFLOAT4** cols;

		XMFLOAT3** tangent;
		XMFLOAT3** binorm;

		float pnscale;
		float x_mom, z_mom, wave_height;

		float cam_check_point;

		float* height_fields;

		void InitWater(AllResources* p_Resources, int xbp, int ybp, int _bUpdateable = 0);

		void UpdateBuffers();

		float zpos;
		float xpos;
		float ypos;

		btRigidBody* m_rigidbody;
		btCollisionShape* m_collisionShape;

		float* cam_z;

		HeightMapInfo* hm_info;

		part_index* p_index;

		float m_scale;

		bool bActive;

		VertexType* m_phy_vertices;
		int no_phy_verticies;
		int count_no_phy_verticies;
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return m_vertexBuffer; }
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return m_indexBuffer; }
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBufferSimple() { return m_indexBuffer_simple; }

		void SetVertexBuffer();
		void SetIndexBuffer();
		void SetIndexBufferSimple();
		void ClearMemory();

		concurrency::task<void> Water::Update(float timeDelta, float timeTotal, float** water_height_map);
		bool SetFlat(int xnum, int ynum, float scale, int xplane);

		void getWorldTransform(btTransform &worldTransform) const;
		void setWorldTransform(const btTransform &worldTransform);

		void SetPosition(float x, float y, float z);

		vector<WaterVertexData> m_vertices;
		vector<unsigned short> m_indices;
		vector<unsigned short> m_indices_simple;

		void AddVertexTexNorm(float x, float y, float z, float u, float v, float nx, float ny, float nz);
		void AddVertexTexNormCol(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a);
		void Render(int complexity);

		void GetMatrix(XMFLOAT4X4* matr);

		void UpdateProjectionMatrix(XMMATRIX *projectionMatrix);
		void Update(XMMATRIX *viewMatrix, float timeTotal);

		//void LoadTexture(Microsoft::WRL::ComPtr<ID3D11Device1> pm_d3dDevice, wchar_t* texture_filename);

		int m_verticesCount;
		int m_indicesCount;
		int m_indicesCount_simple;

		Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;

		// Vertex and index buffers
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer_simple;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
	};
}
