#include "duck.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;
using namespace directx;
using namespace utils;

Duck::Duck(HINSTANCE hInst): DuckBase(hInst)
{
	//Shader Variables
	m_variables.AddSemanticVariable("modelMtx", VariableSemantic::MatM);
	m_variables.AddSemanticVariable("modelInvTMtx", VariableSemantic::MatMInvT);
	m_variables.AddSemanticVariable("viewProjMtx", VariableSemantic::MatVP);
	m_variables.AddSemanticVariable("camPos", VariableSemantic::Vec4CamPos);
	m_variables.AddSemanticVariable("mvpMtx", VariableSemantic::MatMVP);

	XMFLOAT4 lightPos = { -1.f, 0.0f, -3.5f, 1.f };
	XMFLOAT3 lightColor = { 12.f, 9.f, 10.f };
	m_variables.AddGuiVariable("lightPos", lightPos, -10, 10);
	m_variables.AddGuiVariable("lightColor", lightColor, 0, 100, 1);
	m_variables.AddGuiColorVariable("surfaceColor", XMFLOAT3{ 0.5f, 1.0f, 0.8f });
	m_variables.AddGuiVariable("ks", 0.8f);
	m_variables.AddGuiVariable("kd", 0.5f);
	m_variables.AddGuiVariable("ka", 0.2f);
	m_variables.AddGuiVariable("m", 1.f, 0.1f, 200.f);

	m_variables.AddGuiVariable("waterLevel", -0.05f, -1, 1, 0.001f);

	//Models
	XMFLOAT4X4 modelMtx;
	auto quad = addModelFromString("pp 4\n1 0 1 0 1 0\n1 0 -1 0 1 0\n""-1 0 -1 0 1 0\n-1 0 1 0 1 0\n");
	auto envModel = addModelFromString("hex 0 0 0 1.73205");
	XMStoreFloat4x4(&modelMtx, XMMatrixScaling(20, 20, 20));
	model(quad).applyTransform(modelMtx);
	model(envModel).applyTransform(modelMtx);


	//Textures
	m_variables.AddSampler(m_device, "samp");
	m_variables.AddTexture(m_device, "envMap", L"textures/cubeMap.dds");

	//Render Passes
	auto passEnv = addPass(L"envVS.cso", L"envPS.cso");
	addModelToPass(passEnv, envModel);
	addRasterizerState(passEnv, rasterizer_info(true));

	auto passWater = addPass(L"waterVS.cso", L"waterPS.cso");
	addModelToPass(passWater, quad);
	rasterizer_info rs;
	rs.CullMode = D3D11_CULL_NONE;
	addRasterizerState(passWater, rs);

}
