///////////////////////////////////////////////////////////////////////////////////////////
//
// Mesh is a class used to display meshes in 3d which are converted
// during build-time from fbx and dgsl files.
//
#pragma once
#include "pch.h"

#include "ppltasks_extra.h"
#include "AllStructures.h"
#include "AllResources.h"

#include "../Bullet/src/LinearMath/btConvexHullComputer.h"
#include "btMyMotionState.h"

namespace Game
{
#define NUM_BONE_INFLUENCES 4
	struct SkinningVertex
	{
		UINT boneIndex[NUM_BONE_INFLUENCES];
		float boneWeight[NUM_BONE_INFLUENCES];
	};
	struct SkinningVertexInput
	{
		byte boneIndex[NUM_BONE_INFLUENCES];
		float boneWeight[NUM_BONE_INFLUENCES];
	};

	template <class T> inline LONG SafeAddRef(T* pUnk) { ULONG lr = 0; if (pUnk != nullptr) { lr = pUnk->AddRef(); } return lr; }
	template <class T> inline LONG SafeRelease(T*& pUnk) { ULONG lr = 0; if (pUnk != nullptr) { lr = pUnk->Release(); pUnk = nullptr; } return lr; }

	class Mesh
	{
	public:
		static const UINT MaxTextures = 8;  // 8 unique textures are supported.

		struct SubMesh
		{
			SubMesh() : MaterialIndex(0), IndexBufferIndex(0), VertexBufferIndex(0), StartIndex(0), PrimCount(0) {}

			MaterialBufferType m_material;

			UINT MaterialIndex;
			UINT IndexBufferIndex;
			UINT VertexBufferIndex;
			UINT StartIndex;
			UINT PrimCount;
		};

		struct Material
		{
			Material() { ZeroMemory(this, sizeof(Material)); }
			~Material() {}

			std::wstring Name;

			XMFLOAT4X4 UVTransform;

			int alpha_mode;

			float Ambient[4];
			float Diffuse[4];
			float Specular[4];
			float Emissive[4];
			float SpecularPower;

			float normal_height;

			float emmit_brightness;

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Textures[MaxTextures];
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Normal;
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Emmit;

			Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
			Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
		};

		struct MeshExtents
		{
			float CenterX, CenterY, CenterZ;
			float Radius;

			float MinX, MinY, MinZ;
			float MaxX, MaxY, MaxZ;
		};

		struct Triangle
		{
			XMFLOAT3 points[3];
		};

		struct MeshPoint
		{
			XMFLOAT3 point;
		};

		typedef std::vector<Triangle> TriangleCollection;
		typedef std::vector<MeshPoint> PointCollection;

		struct BoneInfo
		{
			std::wstring Name;
			INT ParentIndex;
			XMFLOAT4X4 InvBindPos;
			XMFLOAT4X4 BindPose;
			XMFLOAT4X4 BoneLocalTransform;
		};

		struct Keyframe
		{
			Keyframe() : BoneIndex(0), Time(0.0f) {}

			UINT BoneIndex;
			float Time;
			XMFLOAT4X4 Transform;
		};

		typedef std::vector<Keyframe> KeyframeArray;

		struct AnimClip
		{
			float StartTime;
			float EndTime;
			KeyframeArray Keyframes;
		};

		typedef std::map<const std::wstring, AnimClip> AnimationClipMap;

		int alpha_mode;

		// Access to mesh data.
		std::vector<SubMesh>& SubMeshes() { return m_submeshes; }
		std::vector<Material>& Materials() { return m_materials; }
		std::vector<ID3D11Buffer*>& VertexBuffers() { return m_vertexBuffers; }
		std::vector<ID3D11Buffer*>& SkinningVertexBuffers() { return m_skinningVertexBuffers; }
		std::vector<ID3D11Buffer*>& IndexBuffers() { return m_indexBuffers; }
		MeshExtents& Extents() { return m_meshExtents; }
		AnimationClipMap& AnimationClips() { return m_animationClips; }
		std::vector<BoneInfo>& BoneInfoCollection() { return m_boneInfo; }
		TriangleCollection& Triangles() { return m_triangles; }
		PointCollection& Points() { return m_points; }
		const wchar_t* Name() const { return m_name.c_str(); }

		void* Tag;

		// Destructor.
		~Mesh()
		{
			for (ID3D11Buffer *ib : m_indexBuffers)
			{
				SafeRelease(ib);
			}

			for (ID3D11Buffer *vb : m_vertexBuffers)
			{
				SafeRelease(vb);
			}

			for (ID3D11Buffer *svb : m_skinningVertexBuffers)
			{
				SafeRelease(svb);
			}

			m_submeshes.clear();
			m_materials.clear();
			m_indexBuffers.clear();
			m_vertexBuffers.clear();
			m_skinningVertexBuffers.clear();
		}

		void SetMaterialTexture(std::wstring _name, int tex_slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> p_Tex, int _alpha_mode)
		{
			alpha_mode = _alpha_mode;
			for (int i = 0; i < m_materials.size(); i++)
			{
				if (_name.length() == 0)
				{
					m_materials[i].Textures[tex_slot] = p_Tex;
				}
				else
				{
					if (!wcscmp(_name.c_str(), m_materials[i].Name.c_str()))
					{
						m_materials[i].Textures[tex_slot] = p_Tex;
					}
				}
			}
		}

		void SetMaterialNormal(std::wstring _name, int tex_slot, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> p_Tex, float _intensity)
		{
			for (int i = 0; i < m_materials.size(); i++)
			{
				if (_name.length() == 0)
				{
					m_materials[i].Normal = p_Tex;
					m_materials[i].normal_height = _intensity;
				}
				else
				{
					if (!wcscmp(_name.c_str(), m_materials[i].Name.c_str()))
					{
						m_materials[i].Normal = p_Tex;
						m_materials[i].normal_height = _intensity;
					}
				}
			}
		}

		// Render the mesh to the current render target.
		void Render(AllResources& graphics, bool _render_emmit = false);

		static void LoadFromFile(
			AllResources& graphics,
			const std::wstring& meshFilename,
			const std::wstring& shaderPathLocation,
			const std::wstring& texturePathLocation,
			std::vector<Mesh*>& loadedMeshes,
			float _scale = 1.0f
		);

		static void Load(FILE* fp, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation, Mesh*& outMesh, float _scale);

		// Loads a scene from the specified file, returning a vector of mesh objects.
		static task<void> LoadFromFileAsync(
			AllResources& graphics,
			const std::wstring& meshFilename,
			const std::wstring& shaderPathLocation,
			const std::wstring& texturePathLocation,
			std::vector<Mesh*>& loadedMeshes,
			float _scale = 1.0f
		);

	private:
		Mesh()
		{
			Tag = nullptr;
		}

		static void StripPath(std::wstring& path)
		{
			size_t p = path.rfind(L"\\");
			if (p != std::wstring::npos)
			{
				path = path.substr(p + 1);
			}
		}

		template <typename T>
		static void ReadStruct(Windows::Storage::Streams::DataReader^ reader, T* output, size_t size = sizeof(T))
		{
			reader->ReadBytes(Platform::ArrayReference<BYTE>((BYTE*)output, size));
		}

		// Reads a string converted from an array of wchar_t of given size using a DataReader.
		static void ReadString(Windows::Storage::Streams::DataReader^ reader, std::wstring* output, unsigned int count)
		{
			if (count == 0)
			{
				return;
			}

			std::vector<wchar_t> characters(count);
			ReadStruct(reader, &characters[0], count * sizeof(wchar_t));
			*output = &characters[0];
		}

		// Reads a string converted from an array of wchar_t using a DataReader.
		static void ReadString(Windows::Storage::Streams::DataReader^ reader, std::wstring* output)
		{
			UINT count = reader->ReadUInt32();
			ReadString(reader, output, count);
		}

		static task<Mesh*> ReadAsync(Windows::Storage::Streams::DataReader^ reader, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation, float _scale = 1.0f);

		std::vector<SubMesh> m_submeshes;
		std::vector<Material> m_materials;
		std::vector<ID3D11Buffer*> m_vertexBuffers;
		std::vector<ID3D11Buffer*> m_skinningVertexBuffers;
		std::vector<ID3D11Buffer*> m_indexBuffers;
		TriangleCollection m_triangles;
		PointCollection m_points;

		MeshExtents m_meshExtents;

		btRigidBody* m_rigidbody;

		btConvexHullComputer* convex_hull;

		AnimationClipMap m_animationClips;
		std::vector<BoneInfo> m_boneInfo;

		std::wstring m_name;

		btMyMotionState* m_motion;

	public:
		std::vector<Material> GetMaterials() {
			return m_materials;
		};

		std::wstring* GetName()
		{
			return &m_name;
		}

		btConvexHullComputer* GetConvexHull() {
			return convex_hull;
		}

		std::vector<MeshPoint> GetPoints() {
			return m_points;
		}
		Material* GetMaterial(int index) {
			return &m_materials[index];
		};

		ID3D11ShaderResourceView** GetMaterialTexture(int _mat, int _tex) { return m_materials[_mat].Textures[_tex].GetAddressOf(); };

		void ComputeConvexHull();

		btRigidBody* MakePhysicsBoxFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys);
		btRigidBody* MakePhysicsCylinderFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys);
		btRigidBody* MakePhysicsSphereFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys);
		btRigidBody* MakePhysicsEllipseoidFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys);
		btRigidBody* MakePhysicsConvexHullFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys);
	};
	//
	//
	///////////////////////////////////////////////////////////////////////////////////////////
}