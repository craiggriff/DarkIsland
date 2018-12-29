#pragma once
#include "AllResources.h"
#include "Mesh.h"
#include "btMyMotionState.h"
namespace Game
{
	class MeshModel
	{
	public:
		MeshModel(std::shared_ptr<DX::DeviceResources> pm_deviceResources, Physics* phys);

		void CalcOverallRadius();
		ID3D11ShaderResourceView** GetMaterialTexture(std::wstring _name, int tex_slot, int _alpha_mode);
		ID3D11ShaderResourceView** GetMaterialNormal(std::wstring _name, float _intensity);
		ID3D11ShaderResourceView** GetMaterialEmmit(std::wstring _name, float _intensity);

		void SetMaterialTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, int tex_slot, int _alpha_mode, int specular_power);
		void SetMaterialNormal(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, float _intensity);
		void SetMaterialEmmit(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTex, std::wstring _name, float _intensity);

		void ComputeOverallConvexHull();

		void MakePhysicsMeshes(ObjInfo* ob_info);

		btRigidBody*  MakePhysicsCompoundBoxFromMeshModel(ObjInfo* ob_info, float x, float y, float z, float _scale);
		btRigidBody*  MakePhysicsBoxFromMeshModel(ObjInfo* ob_info, float _scale);
		btRigidBody*  MakePhysicsCylinderFromMeshModel(ObjInfo* ob_info, float _scale);
		btRigidBody*  MakePhysicsSphereFromMeshModel(ObjInfo* ob_info, float _scale);
		btRigidBody*  MakePhysicsEllipseoidFromMeshModel(ObjInfo* ob_info, float _scale);
		btRigidBody*  MakePhysicsConvexHullFromMeshModel(ObjInfo* ob_info, float _scale);

		std::vector<Mesh*> m_mesh;
		std::vector<bool> m_mesh_visible;

		btMyMotionState* m_motion;

		task<void> LoadModel(
			AllResources* p_Resources,
			const std::wstring& meshFilename,
			int phys_shape,
			float _scale = 1.0f
		);

		float OverallRadius;
		float OverallMaxX;
		float OverallMaxY;
		float OverallMaxZ;
		float OverallMinX;
		float OverallMinY;
		float OverallMinZ;

		float OverallCenterX;
		float OverallCenterY;
		float OverallCenterZ;

		int physics_shape;

		std::wstring m_filename;

		Physics* p_Phys;
	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		btConvexHullComputer* hull_comp;
	};
}
