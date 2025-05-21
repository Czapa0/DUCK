#include "duck.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;
using namespace directx;
using namespace utils;

Duck::Duck(HINSTANCE hInst): DuckBase(hInst), z1(), z2(), d()
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
	tex2d_info bump(N, N);
	bump.Usage = D3D11_USAGE_DYNAMIC;
	bump.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bump.MipLevels = 1;
	m_variables.AddTexture(m_device, "bumpMap", bump);

	//Render Passes
	auto passEnv = addPass(L"envVS.cso", L"envPS.cso");
	addModelToPass(passEnv, envModel);
	addRasterizerState(passEnv, rasterizer_info(true));

	auto passWater = addPass(L"waterVS.cso", L"waterPS.cso");
	addModelToPass(passWater, quad);
	rasterizer_info rs;
	rs.CullMode = D3D11_CULL_NONE;
	addRasterizerState(passWater, rs);

	//Other
	calculateDamping();
}

void Duck::update(utils::clock const& clock)
{
	DuckBase::update(clock);
	updateBumps();
	updateWater();
}

void Duck::updateWater()
{
	//Load bump map
	auto& bumpMap = m_variables.GetTexture("bumpMap");
	ID3D11Resource* resource;
	bumpMap.get()->GetResource(&resource);

	//Map resource
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = m_device.context()->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };

	//Update map
	uint8_t* pData = reinterpret_cast<uint8_t*>(mappedResource.pData);
	for (UINT y = 0; y < N; y++)
	{
		uint8_t* pRow = pData + y * mappedResource.RowPitch;
		for (UINT x = 0; x < N; x++)
		{
			uint8_t* pPixel = pRow + x * 4;

			//Calculate normal
			XMVECTOR dx = { 2.f * h, z1[x + 2][y + 1] - z1[x][y + 1], 0.f };
			XMVECTOR dz = { 0.f, z1[x + 1][y] - z1[x + 1][y + 2], 2.f * h };
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(dx, dz));

			//Encode normal
			pPixel[0] = static_cast<uint8_t>(std::clamp((XMVectorGetX(normal) + 1.f) / 2.f * 255.f, 0.f, 255.f));
			pPixel[1] = static_cast<uint8_t>(std::clamp((XMVectorGetY(normal) + 1.f) / 2.f * 255.f, 0.f, 255.f));
			pPixel[2] = static_cast<uint8_t>(std::clamp((XMVectorGetZ(normal) + 1.f) / 2.f * 255.f, 0.f, 255.f));
			pPixel[3] = 0;
		}
	}

	//Unmap the map
	m_device.context()->Unmap(resource, 0);
}

void Duck::calculateDamping()
{
	for (UINT i = 0; i < N; i++) {
		for (UINT j = 0; j < N; j++) {
			float l = min({ i,N - i,j,N - j }) * h;
			d[i][j] = 0.95f * min(1.f, l / 0.2f);
		}
	}
}

void Duck::updateBumps()
{
	for (UINT i = 1; i <= N; i++) {
		for (UINT j = 1; j <= N; j++) {
			z2[i][j] = d[i - 1][j - 1] * (A * (z1[i + 1][j] + z1[i - 1][j] + z1[i][j - 1] + z1[i][j + 1]) + B * z1[i][j] - z2[i][j]);
		}
	}

	std::swap(z1, z2);
}
