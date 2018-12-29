#include "pch.h"
#include "Bert.h"

using namespace Game;

Bert::Bert(AllResources* p_Resources, Level* pp_Level) : AnimChar(p_Resources, pp_Level)
{
}

void Bert::Initialize(int char_num)
{
	switch (char_num)
	{
		/*
	case 0:
	{
		cvalues.model_fname = L"dark_templar_knightB.cmo";
		cvalues.texture_fname= L"knight_d.dds";
		cvalues.normal_fname= L"knight_n.dds";

		cvalues.material_name = L"chainmail_col";

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.124f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.
	}
	break;
	*/
	case 0:
	{
		cvalues.model_fname = L"sorceress_anims_split.cmo";
		//cvalues.texture_fname = L"mickey_mouse_d.dds";
		//cvalues.normal_fname = L"mickey_mouse_n.dds";

		//cvalues.material_name = L"MickeyMouse__Mickey_Mouse_D_tga";

		cvalues.model_idle = L"U3DRoot|U3DRoot|idle|Base Layer";
		cvalues.model_run = L"U3DRoot|U3DRoot|walk|Base Layer";
		cvalues.model_jump = L"U3DRoot|U3DRoot|walk|Base Layer";
		cvalues.model_attack = L"U3DRoot|U3DRoot|attack1|Base Layer";

		char_tex temp_tex;

		temp_tex.material_name = L"gay";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\gay.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"gay";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\gay_N.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.003";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.003";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.004";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.004";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.001";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.001";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"toc";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\toc.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.001";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.002";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"sorceress.002";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\sorceress_n.dds";

		cvalues.textures.push_back(temp_tex);

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI * 0.5f;
		cvalues.rot_y = M_PI * 0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.00224f;
		cvalues.mesh_scale = 0.0014f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.
	}
	break;

	case 1:
	{
		cvalues.model_fname = L"mikey.cmo";
		//cvalues.texture_fname = L"mickey_mouse_d.dds";
		//cvalues.normal_fname = L"mickey_mouse_n.dds";

		//cvalues.material_name = L"MickeyMouse__Mickey_Mouse_D_tga";

		cvalues.model_idle = L"my_avatar|idle";
		cvalues.model_run = L"my_avatar|run";
		cvalues.model_jump = L"my_avatar|walk";
		cvalues.model_attack = L"my_avatar|attack";

		char_tex temp_tex;

		temp_tex.material_name = L"MickeyMouse__Mickey_Mouse_D_tga";
		temp_tex.texture_type = L"t";
		temp_tex.texture_fname = L"Assets\\Compiled\\mickey_mouse_d.dds";

		cvalues.textures.push_back(temp_tex);

		temp_tex.material_name = L"MickeyMouse__Mickey_Mouse_D_tga";
		temp_tex.texture_type = L"n";
		temp_tex.texture_fname = L"Assets\\Compiled\\mickey_mouse_n.dds";

		cvalues.textures.push_back(temp_tex);

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI * 0.5f;
		cvalues.rot_y = M_PI * 0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.
	}
	break;

	/*
	case 2:
	{
		cvalues.model_fname = L"animsanta.cmo";
		strcpy_s(cvalues.texture_fname, "mickey_mouse_d");
		strcpy_s(cvalues.normal_fname, "mickey_mouse_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.
	}
	break;
	case 3:
	{
		cvalues.model_fname = L"mario.cmo";
		strcpy_s(cvalues.texture_fname, "mickey_mouse_d");
		strcpy_s(cvalues.normal_fname, "mickey_mouse_n");

		cvalues.off_x = 0.0f;
		cvalues.off_y = 0.0f;
		cvalues.off_z = 0.0f;
		cvalues.rot_x = M_PI*0.5f;
		cvalues.rot_y = M_PI*0.5f;
		cvalues.rot_z = 0.0f;

		cvalues.char_scale = 0.224f;
		cvalues.mesh_scale = 0.14f;
		//cvalues.model_material.specularPow = 20.0f;
		//cvalues.model_material.specularLvl = 0.5f;
		//cvalues.model_material.diffuseLvl = 1.0f;
		//cvalues.model_material.ambientLvl = 1.0f;
		//cvalues.model_material.specularCol = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);

		cvalues.max_walk_speed = 8.0f;
		//cvalues.
	}
	break;
	*/
	}

	//AnimChar::Initialize();
}