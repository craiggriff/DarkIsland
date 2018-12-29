#include "pch.h"

#include "Mesh.h"

using namespace Game;

void Mesh::Render(AllResources& graphics, bool _render_emmit)
{
	ID3D11DeviceContext* deviceContext = graphics.m_deviceResources->GetD3DDeviceContext();

	if (GetName()->compare(L"PHY_BOX") == 0 ||
		GetName()->compare(L"PHY_CYLINDER") == 0 ||
		GetName()->compare(L"PHY_SPHERE") == 0 ||
		GetName()->compare(L"PHY_ELLIPSEOID") == 0 ||
		GetName()->compare(L"PHY_CONVEXHULL") == 0 ||
		GetName()->compare(L"LIGHT_POS") == 0 ||
		GetName()->compare(L"LIGHT_POINT") == 0)
	{
		return;
	}

	for (SubMesh& submesh : m_submeshes)
	{
		Material& material = m_materials[submesh.MaterialIndex];

		if (_render_emmit == true && material.emmit_brightness == 0.0f)
			continue;

		if (submesh.IndexBufferIndex < m_indexBuffers.size() &&
			submesh.VertexBufferIndex < m_vertexBuffers.size())
		{
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffers[submesh.VertexBufferIndex], &stride, &offset);
			deviceContext->IASetIndexBuffer(m_indexBuffers[submesh.IndexBufferIndex], DXGI_FORMAT_R16_UINT, 0);
		}

		if (submesh.MaterialIndex < m_materials.size())
		{
			MaterialBufferType mat_buffer;

			// Update the material constant buffer.
			memcpy(&mat_buffer.Ambient, material.Ambient, sizeof(material.Ambient));
			memcpy(&mat_buffer.Diffuse, material.Diffuse, sizeof(material.Diffuse));
			memcpy(&mat_buffer.Specular, material.Specular, sizeof(material.Specular));
			memcpy(&mat_buffer.Emissive, material.Emissive, sizeof(material.Emissive));
			mat_buffer.SpecularPower = material.SpecularPower;
			mat_buffer.normal_height = material.normal_height;
			mat_buffer.emmit_brightness = material.emmit_brightness;
			graphics.UpdateMaterialBuffer(&mat_buffer);

			for (UINT tex = 0; tex < 1; tex++)
			{
				//ID3D11ShaderResourceView* texture = material.Textures[tex].Get();
				
				if (_render_emmit == false)
					//if (false)//(_render_emmit==false) // cool
				{
					deviceContext->PSSetShaderResources(0, 1, material.Textures[0].GetAddressOf());

					if (material.normal_height > 0.0f)
					{
						deviceContext->PSSetShaderResources(3, 1, material.Normal.GetAddressOf());
					}
				}
				else
				{
					deviceContext->PSSetShaderResources(4, 1, material.Emmit.GetAddressOf());
				}
			}

			deviceContext->DrawIndexed(submesh.PrimCount * 3, submesh.StartIndex, 0);
		}
	}
}

void Mesh::LoadFromFile(
	AllResources& graphics,
	const std::wstring& meshFilename,
	const std::wstring& shaderPathLocation,
	const std::wstring& texturePathLocation,
	std::vector<Mesh*>& loadedMeshes,
	float _scale
)
{
	//
	// clear output vector
	//
	if (true)
	{
		loadedMeshes.clear();
	}

	//
	// open the mesh file
	//
	FILE* fp = nullptr;
	_wfopen_s(&fp, meshFilename.c_str(), L"rb");
	if (fp == nullptr)
	{
		//std::wstring error = L"Mesh file could not be opened " + meshFilename + L"\n";
		//OutputDebugString(error.c_str());
	}
	else
	{
		//
		// read how many meshes are part of the scene
		//
		UINT meshCount = 0;
		fread(&meshCount, sizeof(meshCount), 1, fp);

		//
		// for each mesh in the scene, load it from the file
		//
		for (UINT i = 0; i < meshCount; i++)
		{
			Mesh* mesh = nullptr;
			Mesh::Load(fp, graphics, shaderPathLocation, texturePathLocation, mesh, _scale);
			if (mesh != nullptr)
			{
				loadedMeshes.push_back(mesh);
			}
		}
	}
}

void Mesh::Load(FILE* fp, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation, Mesh*& outMesh, float _scale)
{
	UNREFERENCED_PARAMETER(texturePathLocation);

	//
	// initialize output mesh
	//
	outMesh = nullptr;
	if (fp != nullptr)
	{
		Mesh* mesh = new Mesh();

		UINT nameLen = 0;
		fread(&nameLen, sizeof(nameLen), 1, fp);
		if (nameLen > 0)
		{
			std::vector<wchar_t> objName(nameLen);
			fread(&objName[0], sizeof(wchar_t), nameLen, fp);
			mesh->m_name = &objName[0];
		}

		//
		// read material count
		//
		UINT numMaterials = 0;
		fread(&numMaterials, sizeof(UINT), 1, fp);
		mesh->m_materials.resize(numMaterials);

		//
		// load each material
		//
		for (UINT i = 0; i < numMaterials; i++)
		{
			Material& material = mesh->m_materials[i];

			//
			// read material name
			//
			UINT stringLen = 0;
			fread(&stringLen, sizeof(stringLen), 1, fp);
			if (stringLen > 0)
			{
				std::vector<wchar_t> matName(stringLen);
				fread(&matName[0], sizeof(wchar_t), stringLen, fp);
				material.Name = &matName[0];
			}

			//
			// read ambient and diffuse properties of material
			//
			fread(material.Ambient, sizeof(material.Ambient), 1, fp);
			fread(material.Diffuse, sizeof(material.Diffuse), 1, fp);
			fread(material.Specular, sizeof(material.Specular), 1, fp);
			fread(&material.SpecularPower, sizeof(material.SpecularPower), 1, fp);
			fread(material.Emissive, sizeof(material.Emissive), 1, fp);
			fread(&material.UVTransform, sizeof(material.UVTransform), 1, fp);

			//material.Diffuse[0] = (rand() % 100)*0.01f;
			//material.Diffuse[1] = (rand() % 100)*0.01f;
			//material.Diffuse[2] = (rand() % 100)*0.01f;
			//material.Diffuse[3] = (rand() % 100)*0.01f;
			//
			// assign vertex shader and sampler state
			//
			//material.VertexShader = graphics.GetVertexShader();

			//material.SamplerState = graphics.GetSamplerState();

			//
			// read name of the pixel shader
			//
			stringLen = 0;
			fread(&stringLen, sizeof(stringLen), 1, fp);
			if (stringLen > 0)
			{
				//
				// read the pixel shader name
				//
				std::vector<wchar_t> pixelShaderName(stringLen);
				fread(&pixelShaderName[0], sizeof(wchar_t), stringLen, fp);
				std::wstring sourceFile = &pixelShaderName[0];

				//
				// continue loading pixel shader if name is not empty
				//
				if (!sourceFile.empty())
				{
					//
					// create well-formed file name for the pixel shader
					//
					Mesh::StripPath(sourceFile);

					//
					// use fallback shader if Pixel Shader Model 4.0 is not supported
					//
					/*
					if (graphics.GetDeviceFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
					{
					//
					// this device is not compatible with Pixel Shader Model 4.0
					// try to fall back to a shader with the same name but compiled from HLSL
					//
					size_t lastUnderline = sourceFile.find_last_of('_');
					size_t firstDotAfterLastUnderline = sourceFile.find_first_of('.', lastUnderline);
					sourceFile = sourceFile.substr(lastUnderline + 1, firstDotAfterLastUnderline - lastUnderline) + L"cso";
					}

					//
					// append path
					//
					sourceFile = shaderPathLocation + sourceFile;

					//
					// get or create pixel shader
					//
					ID3D11PixelShader* materialPixelShader = graphics.GetOrCreatePixelShader(sourceFile);
					material.PixelShader = materialPixelShader;
					*/
				}
			}

			//
			// load textures
			//
			for (int t = 0; t < MaxTextures; t++)
			{
				//
				// read name of texture
				//
				stringLen = 0;
				fread(&stringLen, sizeof(stringLen), 1, fp);
				if (stringLen > 0)
				{
					std::vector<wchar_t> textureFilename(stringLen);
					fread(&textureFilename[0], sizeof(wchar_t), stringLen, fp);
					std::wstring sourceFile = &textureFilename[0];

					//
					// get or create texture
					//
					//ID3D11ShaderResourceView* textureResource = graphics.GetOrCreateTexture(sourceFile);
					//material.Textures[t] = textureResource;
				}
			}
		}

		//
		// does this object contain skeletal animation?
		//
		BYTE isSkeletalDataPresent = FALSE;
		fread(&isSkeletalDataPresent, sizeof(BYTE), 1, fp);

		//
		// read submesh info
		//
		UINT numSubmeshes = 0;
		fread(&numSubmeshes, sizeof(UINT), 1, fp);
		mesh->m_submeshes.resize(numSubmeshes);
		for (UINT i = 0; i < numSubmeshes; i++)
		{
			SubMesh& submesh = mesh->m_submeshes[i];

			//fread(&(mesh->m_submeshes[i]), sizeof(SubMesh), 1, fp);

			fread(&(submesh.MaterialIndex), sizeof(UINT), 1, fp);
			fread(&(submesh.IndexBufferIndex), sizeof(UINT), 1, fp);
			fread(&(submesh.VertexBufferIndex), sizeof(UINT), 1, fp);
			fread(&(submesh.StartIndex), sizeof(UINT), 1, fp);
			fread(&(submesh.PrimCount), sizeof(UINT), 1, fp);
		}

		//
		// read index buffers
		//
		UINT numIndexBuffers = 0;
		fread(&numIndexBuffers, sizeof(UINT), 1, fp);
		mesh->m_indexBuffers.resize(numIndexBuffers);

		std::vector<std::vector<USHORT>> indexBuffers(numIndexBuffers);

		for (UINT i = 0; i < numIndexBuffers; i++)
		{
			UINT ibCount = 0;
			fread(&ibCount, sizeof(UINT), 1, fp);
			if (ibCount > 0)
			{
				indexBuffers[i].resize(ibCount);

				//
				// read in the index data
				//
				fread(&indexBuffers[i][0], sizeof(USHORT), ibCount, fp);

				//
				// create an index buffer for this data
				//
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(USHORT) * ibCount;
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData;
				ZeroMemory(&initData, sizeof(initData));
				initData.pSysMem = &indexBuffers[i][0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_indexBuffers[i]);
			}
		}

		//
		// read vertex buffers
		//
		UINT numVertexBuffers = 0;
		fread(&numVertexBuffers, sizeof(UINT), 1, fp);
		mesh->m_vertexBuffers.resize(numVertexBuffers);

		std::vector<std::vector<Vertex>> vertexBuffers(numVertexBuffers);

		for (UINT i = 0; i < numVertexBuffers; i++)
		{
			UINT vbCount = 0;
			fread(&vbCount, sizeof(UINT), 1, fp);
			if (vbCount > 0)
			{
				vertexBuffers[i].resize(vbCount);

				//
				// read in the vertex data
				//
				fread(&vertexBuffers[i][0], sizeof(Vertex), vbCount, fp);

				for (int z = 0; z < vbCount; z++)
				{
					vertexBuffers[i][z].v = 1.0f - vertexBuffers[i][z].v;
				}

				for (int z = 0; z < vbCount; z++)
				{
					//awwwwvertexBuffers[i][z].v = 0.0f - vertexBuffers[i][z].v;
					MeshPoint point;
					if (true)// (isSkeletalDataPresent == false)
					{
						//vertexBuffers[i][z].u *= 0.01f;
						//vertexBuffers[i][z].v *= 0.01f;
						vertexBuffers[i][z].x *= _scale;
						vertexBuffers[i][z].y *= _scale;
						vertexBuffers[i][z].z *= _scale;

						point.point.x = vertexBuffers[i][z].x;
						point.point.y = vertexBuffers[i][z].y;
						point.point.z = vertexBuffers[i][z].z;
					}

					mesh->m_points.push_back(point);
				}

				//
				// create a vertex buffer for this data
				//
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(Vertex) * vbCount;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData;
				ZeroMemory(&initData, sizeof(initData));
				initData.pSysMem = &vertexBuffers[i][0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_vertexBuffers[i]);
			}
		}

		for (SubMesh& subMesh : mesh->m_submeshes)
		{
			std::vector<USHORT>& ib = indexBuffers[subMesh.IndexBufferIndex];
			std::vector<Vertex>& vb = vertexBuffers[subMesh.VertexBufferIndex];

			for (UINT j = 0; j < ib.size(); j += 3)
			{
				Vertex& v0 = vb[ib[j]];
				Vertex& v1 = vb[ib[j + 1]];
				Vertex& v2 = vb[ib[j + 2]];

				Triangle tri;
				tri.points[0].x = v0.x;
				tri.points[0].y = v0.y;
				tri.points[0].z = v0.z;

				tri.points[1].x = v1.x;
				tri.points[1].y = v1.y;
				tri.points[1].z = v1.z;

				tri.points[2].x = v2.x;
				tri.points[2].y = v2.y;
				tri.points[2].z = v2.z;

				mesh->m_triangles.push_back(tri);
			}
		}

		// done with temp buffers
		vertexBuffers.clear();
		indexBuffers.clear();

		//
		// read skinning vertex buffers
		//
		UINT numSkinningVertexBuffers = 0;
		fread(&numSkinningVertexBuffers, sizeof(UINT), 1, fp);
		mesh->m_skinningVertexBuffers.resize(numSkinningVertexBuffers);
		for (UINT i = 0; i < numSkinningVertexBuffers; i++)
		{
			UINT vbCount = 0;
			fread(&vbCount, sizeof(UINT), 1, fp);
			if (vbCount > 0)
			{
				std::vector<SkinningVertex> verts(vbCount);
				std::vector<SkinningVertexInput> input(vbCount);

				//
				// read in the vertex data
				//
				fread(&verts[0], sizeof(SkinningVertex), vbCount, fp);

				//
				// convert indices to byte (to support D3D Feature Level 9)
				//
				for (UINT j = 0; j < vbCount; j++)
				{
					for (int k = 0; k < NUM_BONE_INFLUENCES; k++)
					{
						input[j].boneIndex[k] = (byte)verts[j].boneIndex[k];
						input[j].boneWeight[k] = verts[j].boneWeight[k];
					}
				}

				//
				// create a vertex buffer for this data
				//
				D3D11_BUFFER_DESC bd;
				ZeroMemory(&bd, sizeof(bd));
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.ByteWidth = sizeof(SkinningVertexInput) * vbCount;
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA initData;
				ZeroMemory(&initData, sizeof(initData));
				initData.pSysMem = &input[0];

				graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_skinningVertexBuffers[i]);
			}
		}

		//
		// read extents
		//
		fread(&mesh->m_meshExtents, sizeof(MeshExtents), 1, fp);

		mesh->m_meshExtents.CenterX *= _scale;
		mesh->m_meshExtents.CenterY *= _scale;
		mesh->m_meshExtents.CenterZ *= _scale;
		mesh->m_meshExtents.MaxX *= _scale;
		mesh->m_meshExtents.MaxY *= _scale;
		mesh->m_meshExtents.MaxZ *= _scale;
		mesh->m_meshExtents.MinX *= _scale;
		mesh->m_meshExtents.MinY *= _scale;
		mesh->m_meshExtents.MinZ *= _scale;

		// Recalculate max points
		mesh->m_meshExtents.MaxX = 0.0f;
		mesh->m_meshExtents.MaxY = 0.0f;
		mesh->m_meshExtents.MaxZ = 0.0f;

		for (int z = 0; z < mesh->m_points.size(); z++)
		{
			if (mesh->m_points[z].point.x > mesh->m_meshExtents.MaxX)
				mesh->m_meshExtents.MaxX = mesh->m_points[z].point.x;

			if (mesh->m_points[z].point.y > mesh->m_meshExtents.MaxY)
				mesh->m_meshExtents.MaxX = mesh->m_points[z].point.y;

			if (mesh->m_points[z].point.z > mesh->m_meshExtents.MaxZ)
				mesh->m_meshExtents.MaxX = mesh->m_points[z].point.z;
		}

		float dist1 = mesh->m_meshExtents.MaxX +
			mesh->m_meshExtents.MaxY +
			mesh->m_meshExtents.MaxZ;

		XMStoreFloat(&dist1, XMVectorSqrt(XMLoadFloat(&dist1)));

		mesh->m_meshExtents.Radius = dist1;// *_scale;;

		if (isSkeletalDataPresent)
		{
			//
			// read bones
			//
			UINT boneCount = 0;
			fread(&boneCount, sizeof(UINT), 1, fp);

			mesh->m_boneInfo.resize(boneCount);

			for (UINT b = 0; b < boneCount; b++)
			{
				// read the bone name (length, then chars)
				UINT nameLength = 0;
				fread(&nameLength, sizeof(UINT), 1, fp);

				if (nameLength > 0)
				{
					std::vector<wchar_t> nameVec(nameLength);
					fread(&nameVec[0], sizeof(wchar_t), nameLength, fp);

					mesh->m_boneInfo[b].Name = &nameVec[0];
				}

				// read the transforms
				fread(&mesh->m_boneInfo[b].ParentIndex, sizeof(INT), 1, fp);
				fread(&mesh->m_boneInfo[b].InvBindPos, sizeof(XMFLOAT4X4), 1, fp);
				fread(&mesh->m_boneInfo[b].BindPose, sizeof(XMFLOAT4X4), 1, fp);
				fread(&mesh->m_boneInfo[b].BoneLocalTransform, sizeof(XMFLOAT4X4), 1, fp);
			}

			//
			// read animation clips
			//
			UINT clipCount = 0;
			fread(&clipCount, sizeof(UINT), 1, fp);

			for (UINT j = 0; j < clipCount; j++)
			{
				// read clip name
				UINT len = 0;
				fread(&len, sizeof(UINT), 1, fp);

				std::wstring clipName;
				if (len > 0)
				{
					std::vector<wchar_t> clipNameVec(len);
					fread(&clipNameVec[0], sizeof(wchar_t), len, fp);

					clipName = &clipNameVec[0];
				}

				fread(&mesh->m_animationClips[clipName].StartTime, sizeof(float), 1, fp);
				fread(&mesh->m_animationClips[clipName].EndTime, sizeof(float), 1, fp);

				KeyframeArray& keyframes = mesh->m_animationClips[clipName].Keyframes;

				// read keyframecount
				UINT kfCount = 0;
				fread(&kfCount, sizeof(UINT), 1, fp);

				// preallocate the memory
				keyframes.reserve(kfCount);

				// read each keyframe
				for (UINT k = 0; k < kfCount; k++)
				{
					Keyframe kf;

					// read the bone
					fread(&kf.BoneIndex, sizeof(UINT), 1, fp);

					// read the time
					fread(&kf.Time, sizeof(UINT), 1, fp);

					// read the transform
					fread(&kf.Transform, sizeof(XMFLOAT4X4), 1, fp);

					// add to collection
					keyframes.push_back(kf);
				}
			}
		}

		//
		// set the output mesh
		//
		outMesh = mesh;
	}
}

task<void> Mesh::LoadFromFileAsync(
	AllResources& graphics,
	const std::wstring& meshFilename,
	const std::wstring& shaderPathLocation,
	const std::wstring& texturePathLocation,
	std::vector<Mesh*>& loadedMeshes,
	float _scale)
{
	
	// Clear the output vector.
	if (true)
	{
		loadedMeshes.clear();
	}

	auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
	//folder->ToString()
	// Start by loading the file.
	return create_task(folder->GetFileAsync(Platform::StringReference(meshFilename.c_str())))
		// Then open the file.
		.then([](Windows::Storage::StorageFile^ file)
	{
		return Windows::Storage::FileIO::ReadBufferAsync(file);
	})
		// Then read the meshes.
		.then([&graphics, _scale, shaderPathLocation, texturePathLocation, &loadedMeshes](Windows::Storage::Streams::IBuffer^ buffer)
	{
		auto reader = Windows::Storage::Streams::DataReader::FromBuffer(buffer);
		reader->ByteOrder = Windows::Storage::Streams::ByteOrder::LittleEndian;
		reader->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

		// Read how many meshes are part of the scene.
		std::shared_ptr<UINT> remainingMeshesToRead = std::make_shared<UINT>(reader->ReadUInt32());

		if (*remainingMeshesToRead == 0)
		{
			return create_task([]() {});
		}

		// For each mesh in the scene, load it from the file.

		return extras::create_iterative_task([reader, _scale, &graphics, shaderPathLocation, texturePathLocation, &loadedMeshes, remainingMeshesToRead]()
		{
			return Mesh::ReadAsync(reader, graphics, shaderPathLocation, texturePathLocation, _scale)
				.then([&loadedMeshes, remainingMeshesToRead](Mesh* mesh) -> bool
			{
				if (mesh != nullptr)
				{
					loadedMeshes.push_back(mesh);
				}

				// Return true to continue iterating, false to stop.

				*remainingMeshesToRead = *remainingMeshesToRead - 1;

				return *remainingMeshesToRead > 0;
			});
		});
	});
}

task<Mesh*> Mesh::ReadAsync(Windows::Storage::Streams::DataReader^ reader, AllResources& graphics, const std::wstring& shaderPathLocation, const std::wstring& texturePathLocation, float _scale)
{
	std::vector<task<void>> innerTasks;

	// Initialize output mesh.
	if (reader == nullptr)
	{
		return extras::create_value_task<Mesh*>(nullptr);
	}

	Mesh* mesh = new Mesh();

	ReadString(reader, &mesh->m_name);

	// Read material count.
	UINT numMaterials = reader->ReadUInt32();
	mesh->m_materials.resize(numMaterials);

	size_t firstDotAfterLastUnderlineName = mesh->m_name.find_first_of('.');
	if (firstDotAfterLastUnderlineName > 0)
	{
		mesh->m_name = mesh->m_name.substr(0, firstDotAfterLastUnderlineName);
	}

	//if (!mesh->m_name.compare(L"PHY_BOX"))
	//{
	//	mesh->m_materials.resize(numMaterials);
	//}
	// Load each material.
	for (UINT i = 0; i < numMaterials; i++)
	{
		Material& material = mesh->m_materials[i];
		//material.
		// Read the material name.
		ReadString(reader, &material.Name);

		// Read the ambient and diffuse properties of material.

		ReadStruct(reader, &material.Ambient, sizeof(material.Ambient));
		ReadStruct(reader, &material.Diffuse, sizeof(material.Diffuse));
		ReadStruct(reader, &material.Specular, sizeof(material.Specular));
		ReadStruct(reader, &material.SpecularPower);
		ReadStruct(reader, &material.Emissive, sizeof(material.Emissive));
		ReadStruct(reader, &material.UVTransform);

		size_t firstDotAfterLastUnderline = mesh->m_materials[i].Name.find_first_of('.');
		if (firstDotAfterLastUnderline > 0)
		{
			mesh->m_materials[i].Name = mesh->m_materials[i].Name.substr(0, firstDotAfterLastUnderline);
		}

		/*
		if (mesh->m_materials[i].Name == L"Walls")
		{
			OutputDebugStringW(material.Name.c_str());
		}

		if (mesh->m_materials[i].Name == L"GothicChurch")
		{
			OutputDebugStringW(material.Name.c_str());
		}
		*/

		/*
		if (mesh->m_name == L"Plankfloor")
		{
		OutputDebugStringW(mesh->m_name.c_str());
		}

		if (mesh->m_materials[i].Name == L"Wooden")
		{
		OutputDebugStringW(mesh->m_name.c_str());
		}
		*/
		//material.Diffuse[0] = (rand() % 100)*0.01f;
		//material.Diffuse[1] = (rand() % 100)*0.01f;
		//material.Diffuse[2] = (rand() % 100)*0.01f;
		//material.Diffuse[3] = (rand() % 100)*0.01f;

		material.normal_height = 0.0;
		material.emmit_brightness = 0.0;

		material.SpecularPower *= 2.0f;
		//if (material.Name == L"lamp_glass")
		//{
		material.Ambient[0] = 0.5f;
		material.Ambient[1] = 0.5f;
		material.Ambient[2] = 0.5f;
		material.Ambient[3] = 1.0f;
		//}

		if (material.Specular[0] == 0 || material.Specular[1] == 0 || material.Specular[2] == 0)
		{
			material.Specular[3] = 0.0f;
		}

		// Assign the vertex shader and sampler state.
		//material.VertexShader = graphics.GetVertexShader();

		//material.SamplerState = graphics.GetSamplerState();

		// Read the size of the name of the pixel shader.
		UINT stringLen = reader->ReadUInt32();
		if (stringLen > 0)
		{
			// Read the pixel shader name.
			std::wstring sourceFile;
			ReadString(reader, &sourceFile, stringLen);

			// Continue loading pixel shader if name is not empty.
			if (false)//(!sourceFile.empty())
			{
				// Create well-formed file name for the pixel shader.
				Mesh::StripPath(sourceFile);

				// Use fallback shader if Pixel Shader Model 4.0 is not supported.
				if (true) //(graphics.GetDeviceFeatureLevel() < D3D_FEATURE_LEVEL_10_0)
				{
					// This device is not compatible with Pixel Shader Model 4.0.
					// Try to fall back to a shader with the same name but compiled from HLSL.
					size_t lastUnderline = sourceFile.find_last_of('_');
					size_t firstDotAfterLastUnderline = sourceFile.find_first_of('.', lastUnderline);
					sourceFile = sourceFile.substr(lastUnderline + 1, firstDotAfterLastUnderline - lastUnderline) + L"cso";
				}

				// Append the base path.
				sourceFile = shaderPathLocation + sourceFile;

				// Get or create the pixel shader.
				//innerTasks.push_back(graphics.GetOrCreatePixelShaderAsync(sourceFile).then([&material](ID3D11PixelShader* materialPixelShader)
				//{
				//	material.PixelShader = materialPixelShader;
				//}));
			}
		}

		// Load the textures.
		//std::wstring PathLocation = Windows::ApplicationModel::Package::Current->InstalledLocation->DisplayName->Data();

		for (int t = 0; t < MaxTextures; t++)
		{
			// Read the size of the name of the texture.
			stringLen = reader->ReadUInt32();
			if (stringLen > 0)
			{
				// Read the texture name.
				std::wstring sourceFile;
				ReadString(reader, &sourceFile, stringLen);

				// Append the base path.

				size_t firstDotAfterLastUnderline = sourceFile.find_first_of('.');
				sourceFile = sourceFile.substr(0, firstDotAfterLastUnderline) + L"dds";

				//sourceFile = PathLocation + sourceFile;

				std::wstring compStr;//
				compStr = L"trees_bark_002_col.dds";
				/*
				if (true)//!sourceFile.compare(compStr))
				{
				innerTasks.push_back(graphics.GetOrCreateTextureAsync(compStr).then([&material, t](ID3D11ShaderResourceView* textureResource)
				{
				material.Textures[t] = textureResource;
				}));
				}
				*/
				// Get or create the texture.
				//innerTasks.push_back(graphics.GetOrCreateTextureAsync(sourceFile).then([&material, t](ID3D11ShaderResourceView* textureResource)
				//{
				//	material.Textures[t] = textureResource;
				//}));
			}
		}
	}

	// Does this object contain skeletal animation?
	BYTE isSkeletalDataPresent = reader->ReadByte();

	// Read the submesh information.
	UINT numSubmeshes = reader->ReadUInt32();
	mesh->m_submeshes.resize(numSubmeshes);
	for (UINT i = 0; i < numSubmeshes; i++)
	{
		SubMesh& submesh = mesh->m_submeshes[i];
		submesh.MaterialIndex = reader->ReadUInt32();
		submesh.IndexBufferIndex = reader->ReadUInt32();
		submesh.VertexBufferIndex = reader->ReadUInt32();
		submesh.StartIndex = reader->ReadUInt32();
		submesh.PrimCount = reader->ReadUInt32();
	}

	// Read the index buffers.
	UINT numIndexBuffers = reader->ReadUInt32();
	mesh->m_indexBuffers.resize(numIndexBuffers);

	std::vector<std::vector<USHORT>> indexBuffers(numIndexBuffers);

	for (UINT i = 0; i < numIndexBuffers; i++)
	{
		UINT ibCount = reader->ReadUInt32();
		if (ibCount > 0)
		{
			indexBuffers[i].resize(ibCount);

			// Read in the index data.
			for (USHORT& component : indexBuffers[i])
			{
				component = reader->ReadUInt16();
			}

			// Create an index buffer for this data.
			D3D11_BUFFER_DESC bd = { 0 };
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(USHORT)* ibCount;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = &indexBuffers[i][0];

			graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_indexBuffers[i]);
		}
	}

	// Read the vertex buffers.
	UINT numVertexBuffers = reader->ReadUInt32();
	mesh->m_vertexBuffers.resize(numVertexBuffers);

	std::vector<std::vector<Vertex>> vertexBuffers(numVertexBuffers);

	for (UINT i = 0; i < numVertexBuffers; i++)
	{
		UINT vbCount = reader->ReadUInt32();
		if (vbCount > 0)
		{
			vertexBuffers[i].resize(vbCount);

			// Read in the vertex data.
			ReadStruct(reader, &vertexBuffers[i][0], vbCount * sizeof(Vertex));

			//&vertexBuffers[i][0].
			// Flip the u
			for (int z = 0; z < vbCount; z++)
			{
				vertexBuffers[i][z].v = 0.0f - vertexBuffers[i][z].v;

				MeshPoint point;
				if (true)// (isSkeletalDataPresent == false)
				{
					//vertexBuffers[i][z].u *= 0.01f;
					//vertexBuffers[i][z].v *= 0.01f;
					point.point.x = vertexBuffers[i][z].x *= _scale;
					point.point.y = vertexBuffers[i][z].y *= _scale;
					point.point.z = vertexBuffers[i][z].z *= _scale;

					mesh->m_points.push_back(point);
				}
			}

			// Create a vertex buffer for this data.
			D3D11_BUFFER_DESC bd = { 0 };
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(Vertex)* vbCount;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = &vertexBuffers[i][0];

			graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_vertexBuffers[i]);
		}
	}

	// Create the triangle array for each submesh.
	for (SubMesh& subMesh : mesh->m_submeshes)
	{
		std::vector<USHORT>& ib = indexBuffers[subMesh.IndexBufferIndex];
		std::vector<Vertex>& vb = vertexBuffers[subMesh.VertexBufferIndex];

		for (UINT j = 0; j < ib.size(); j += 3)
		{
			Vertex& v0 = vb[ib[j]];
			Vertex& v1 = vb[ib[j + 1]];
			Vertex& v2 = vb[ib[j + 2]];

			Triangle tri;
			tri.points[0].x = v0.x;
			tri.points[0].y = v0.y;
			tri.points[0].z = v0.z;

			tri.points[1].x = v1.x;
			tri.points[1].y = v1.y;
			tri.points[1].z = v1.z;

			tri.points[2].x = v2.x;
			tri.points[2].y = v2.y;
			tri.points[2].z = v2.z;

			mesh->m_triangles.push_back(tri);
		}
	}

	// Clear the temporary buffers.
	vertexBuffers.clear();
	indexBuffers.clear();

	// Read the skinning vertex buffers.
	UINT numSkinningVertexBuffers = reader->ReadUInt32();
	mesh->m_skinningVertexBuffers.resize(numSkinningVertexBuffers);
	for (UINT i = 0; i < numSkinningVertexBuffers; i++)
	{
		UINT vbCount = reader->ReadUInt32();
		if (vbCount > 0)
		{
			std::vector<SkinningVertexInput> verts(vbCount);

			// Read in the vertex data.
			for (SkinningVertexInput& vertex : verts)
			{
				for (size_t j = 0; j < NUM_BONE_INFLUENCES; j++)
				{
					// Convert indices to byte (to support D3D Feature Level 9).
					vertex.boneIndex[j] = (byte)reader->ReadUInt32();
					//string()
					//OutputDebugString(vertex.boneIndex[j]);
					//if (int(vertex.boneIndex[j]) > 94)
					//{
					//	OutputDebugString(L"index overflow");
					//}
				}
				for (size_t j = 0; j < NUM_BONE_INFLUENCES; j++)
				{
					vertex.boneWeight[j] = reader->ReadSingle();
				}
			}

			// Create a vertex buffer for this data.
			D3D11_BUFFER_DESC bd = { 0 };
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(SkinningVertexInput)* vbCount;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = &verts[0];

			graphics.m_deviceResources->GetD3DDevice()->CreateBuffer(&bd, &initData, &mesh->m_skinningVertexBuffers[i]);
		}
	}

	// Read the mesh extents.
	ReadStruct(reader, &mesh->m_meshExtents);

	mesh->m_meshExtents.CenterX *= _scale;
	mesh->m_meshExtents.CenterY *= _scale;
	mesh->m_meshExtents.CenterZ *= _scale;
	mesh->m_meshExtents.MaxX *= _scale;
	mesh->m_meshExtents.MaxY *= _scale;
	mesh->m_meshExtents.MaxZ *= _scale;
	mesh->m_meshExtents.MinX *= _scale;
	mesh->m_meshExtents.MinY *= _scale;
	mesh->m_meshExtents.MinZ *= _scale;

	mesh->m_meshExtents.Radius *= _scale;

	// Recalculate max points
	mesh->m_meshExtents.MaxX = 0.0f;
	mesh->m_meshExtents.MaxY = 0.0f;
	mesh->m_meshExtents.MaxZ = 0.0f;

	for (int z = 0; z < mesh->m_points.size(); z++)
	{
		if (abs(mesh->m_points[z].point.x - mesh->m_meshExtents.CenterX) > mesh->m_meshExtents.MaxX)
			mesh->m_meshExtents.MaxX = abs(mesh->m_points[z].point.x - mesh->m_meshExtents.CenterX);

		if (abs(mesh->m_points[z].point.y - mesh->m_meshExtents.CenterY) > mesh->m_meshExtents.MaxY)
			mesh->m_meshExtents.MaxY = abs(mesh->m_points[z].point.y - mesh->m_meshExtents.CenterY);

		if (abs(mesh->m_points[z].point.z - mesh->m_meshExtents.CenterZ) > mesh->m_meshExtents.MaxZ)
			mesh->m_meshExtents.MaxZ = abs(mesh->m_points[z].point.z - mesh->m_meshExtents.CenterZ);
	}

	//float dist1 = mesh->m_meshExtents.MaxX +
	//	mesh->m_meshExtents.MaxY +
	//	mesh->m_meshExtents.MaxZ;

	//XMStoreFloat(&dist1, XMVectorSqrt(XMLoadFloat(&dist1)));
	//
	//if (dist1 > OverallRadius)
	//	OverallRadius = dist1;
	//mesh->m_meshExtents.Radius = dist1;// *_scale;;

	// Read bones and animation information if needed.
	if (isSkeletalDataPresent)
	{
		// Read bone information.
		UINT boneCount = reader->ReadUInt32();

		mesh->m_boneInfo.resize(boneCount);

		for (BoneInfo& bone : mesh->m_boneInfo)
		{
			// Read the bone name.
			ReadString(reader, &bone.Name);

			// Read the transforms.
			bone.ParentIndex = reader->ReadInt32();
			ReadStruct(reader, &bone.InvBindPos);
			ReadStruct(reader, &bone.BindPose);
			ReadStruct(reader, &bone.BoneLocalTransform);

			/*
			bone.BoneLocalTransform._14 += 0.01f;
			bone.BoneLocalTransform._24 += 0.01f;
			bone.BoneLocalTransform._34 += 0.01f;
			bone.InvBindPos._14 += 0.01f;
			bone.InvBindPos._24 += 0.01f;
			bone.InvBindPos._34 += 0.01f;
			*/
			//bone.
		}

		// Read animation clips.
		UINT clipCount = reader->ReadUInt32();

		for (UINT j = 0; j < clipCount; j++)
		{
			// Read clip name.
			std::wstring clipName;
			ReadString(reader, &clipName);

			mesh->m_animationClips[clipName].StartTime = reader->ReadSingle();
			mesh->m_animationClips[clipName].EndTime = reader->ReadSingle();

			KeyframeArray& keyframes = mesh->m_animationClips[clipName].Keyframes;

			// Read keyframe count.
			UINT kfCount = reader->ReadUInt32();

			// Preallocate the memory.
			keyframes.reserve(kfCount);

			// Read each keyframe.
			for (UINT k = 0; k < kfCount; k++)
			{
				Keyframe kf;

				// Read the bone.
				kf.BoneIndex = reader->ReadUInt32();

				// Read the time.
				kf.Time = reader->ReadSingle();

				// Read the transform.
				ReadStruct(reader, &kf.Transform);

				// Add to the keyframe collection.
				keyframes.push_back(kf);
			}
		}
	}

	return when_all(innerTasks.begin(), innerTasks.end()).then([mesh]()
	{
		return mesh;
	});
}

void Mesh::ComputeConvexHull()
{
	std::vector<float> coords;

	convex_hull = new btConvexHullComputer;

	float point = 0.0f;

	for (int i = 0; i < GetPoints().size(); i++)
	{
		coords.push_back(GetPoints()[i].point.x - Extents().CenterX);
		coords.push_back(GetPoints()[i].point.y - Extents().CenterY);
		coords.push_back(GetPoints()[i].point.z - Extents().CenterZ);
	}

	convex_hull->compute((float *)coords.data(), sizeof(float) * 3, coords.size() / 3, 0.0f, 0.0f);
}

btRigidBody* Mesh::MakePhysicsBoxFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = Extents().MaxX*_scale;
	extent2 = Extents().MaxY*_scale;
	extent3 = Extents().MaxZ*_scale;

	btVector3 vec_cent = btVector3(Extents().CenterX, Extents().CenterY, Extents().CenterZ);
	vec_cent = vec_cent.rotate(btVector3(0.0f, 1.0f, 0.0f), ob_info->dir.x);

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, 0.0f, 0.0f), btVector3(ob_info->pos.x + vec_cent.getX(), ob_info->pos.y + vec_cent.getY(), ob_info->pos.z + vec_cent.getZ())));

	auto fallShape = new btBoxShape(btBoxShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody*  Mesh::MakePhysicsCylinderFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = Extents().MaxX*_scale;
	extent2 = Extents().MaxY*_scale;
	extent3 = Extents().MaxZ*_scale;

	btVector3 vec_cent = btVector3(Extents().CenterX, Extents().CenterY, Extents().CenterZ);
	vec_cent = vec_cent.rotate(btVector3(0.0f, 1.0f, 0.0f), ob_info->dir.x);

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, 0.0f, 0.0f), btVector3(ob_info->pos.x + vec_cent.getX(), ob_info->pos.y + vec_cent.getY(), ob_info->pos.z + vec_cent.getZ())));

	auto fallShape = new btCylinderShape(btCylinderShape(btVector3(extent1, extent2, extent3)));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody* Mesh::MakePhysicsSphereFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = Extents().MaxX*_scale;
	extent2 = Extents().MaxY*_scale;
	extent3 = Extents().MaxZ*_scale;

	if (extent2 > extent1)
		extent1 = extent2;

	if (extent3 > extent1)
		extent1 = extent3;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z)));

	auto fallShape = new btSphereShape(btSphereShape(extent1));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody* Mesh::MakePhysicsEllipseoidFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	extent1 = Extents().MaxX*_scale;
	extent2 = Extents().MaxY*_scale;
	extent3 = Extents().MaxZ*_scale;

	float uniform_value = 1.0f;

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, ob_info->dir.y, ob_info->dir.z), btVector3(ob_info->pos.x, ob_info->pos.y, ob_info->pos.z)));

	auto fallShape = new btMultiSphereShape(&btVector3(0.0f, 0.0f, 0.0f), &uniform_value, 1);

	fallShape->setLocalScaling(btVector3(extent1, extent2, extent3));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	btRigidBody* m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	//m_rigidbody->setWorldTransform(m_initialTransform);
	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}

btRigidBody* Mesh::MakePhysicsConvexHullFromMesh(ObjInfo* ob_info, float _scale, Physics* p_Phys)
{
	float extent1 = 0.0f;
	float extent2 = 0.0f;
	float extent3 = 0.0f;

	auto fallShape = new btConvexHullShape();

	btVector3 vec;
	for (int i = 0; i < convex_hull->vertices.size(); i++)
	{
		vec.setX(convex_hull->vertices[i].getX());
		vec.setY(convex_hull->vertices[i].getY());
		vec.setZ(convex_hull->vertices[i].getZ());

		fallShape->addPoint(vec);
	}

	btVector3 vec_cent = btVector3(Extents().CenterX, Extents().CenterY, Extents().CenterZ);
	vec_cent = vec_cent.rotate(btVector3(0.0f, 1.0f, 0.0f), ob_info->dir.x);

	m_motion = new btMyMotionState(btTransform(btQuaternion(ob_info->dir.x, 0.0f, 0.0f), btVector3(ob_info->pos.x + vec_cent.getX(), ob_info->pos.y + vec_cent.getY(), ob_info->pos.z + vec_cent.getZ())));

	btMotionState* fallMotionState = (btMotionState*)m_motion;
	btVector3 fallInertia(0, 0, 0);

	fallShape->calculateLocalInertia(ob_info->mrf.x, fallInertia);
	m_rigidbody = p_Phys->AddPhysicalObject(fallShape, fallMotionState, fallInertia, ob_info);

	m_rigidbody->setActivationState(WANTS_DEACTIVATION);

	return m_rigidbody;
}