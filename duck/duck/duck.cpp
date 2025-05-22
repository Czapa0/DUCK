#include "duck.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;
using namespace directx;
using namespace utils;

Duck::Duck(HINSTANCE hInst): DuckBase(hInst), m_z1(), m_z2(), m_d(), m_randomGen(random_device{}())
{
	//Shader Variables
	m_variables.AddSemanticVariable("modelMtx", VariableSemantic::MatM);
	m_variables.AddSemanticVariable("modelInvTMtx", VariableSemantic::MatMInvT);
	m_variables.AddSemanticVariable("viewProjMtx", VariableSemantic::MatVP);
	m_variables.AddSemanticVariable("camPos", VariableSemantic::Vec4CamPos);
	m_variables.AddSemanticVariable("mvpMtx", VariableSemantic::MatMVP);

	XMFLOAT4 lightPos = { 0.f, 5.0f, 0.f, 1.f };
	XMFLOAT3 lightColor = { 3.f, 3.f, 3.f };
	m_variables.AddGuiVariable("lightPos", lightPos, -10, 10);
	m_variables.AddGuiVariable("lightColor", lightColor, 0, 10, 1);
	m_variables.AddGuiVariable("ks", 0.1f);
	m_variables.AddGuiVariable("kd", 0.7f);
	m_variables.AddGuiVariable("ka", 0.2f);
	m_variables.AddGuiVariable("m", 10.f, 0.1f, 200.f);

	m_variables.AddGuiVariable("waterLevel", -0.05f, -1, 1, 0.001f);
	m_variables.AddGuiVariable("drops", 0.04f, 0.f, 1.f, 0.001f);

	//Models
	XMFLOAT4X4 modelMtx;
	auto quad = addModelFromString("pp 4\n1 0 1 0 1 0\n1 0 -1 0 1 0\n""-1 0 -1 0 1 0\n-1 0 1 0 1 0\n");
	auto envModel = addModelFromString("hex 0 0 0 1.73205");
	XMStoreFloat4x4(&modelMtx, XMMatrixScaling(20, 20, 20));
	model(quad).applyTransform(modelMtx);
	model(envModel).applyTransform(modelMtx);

	m_duck = addModelFromFile("models/duck.obj");
	XMStoreFloat4x4(&modelMtx, XMMatrixScaling(5e-2f, 5e-2f, 5e-2f));
	model(m_duck).applyTransform(modelMtx);

	//Textures
	m_variables.AddSampler(m_device, "samp");
	m_variables.AddTexture(m_device, "envMap", L"textures/cubeMap.dds");
	tex2d_info bump(N, N);
	bump.Usage = D3D11_USAGE_DYNAMIC;
	bump.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bump.MipLevels = 1;
	m_variables.AddTexture(m_device, "bumpMap", bump);
	m_variables.AddTexture(m_device, "duckTex", L"textures/ducktex.jpg");

	//Render Passes
	auto passEnv = addPass(L"envVS.cso", L"envPS.cso");
	addModelToPass(passEnv, envModel);
	addRasterizerState(passEnv, rasterizer_info(true));

	auto passWater = addPass(L"waterVS.cso", L"waterPS.cso");
	addModelToPass(passWater, quad);
	rasterizer_info rs;
	rs.CullMode = D3D11_CULL_NONE;
	addRasterizerState(passWater, rs);

	auto passDuck = addPass(L"duckVS.cso", L"duckPS.cso");
	addModelToPass(passDuck, m_duck);

	//Water init
	calculateDamping();

	//BSpline
	for (int i = 0; i < 4; i++)
	{
		addNewControlPoint();
	}
}

void Duck::update(utils::clock const& clock)
{
	DuckBase::update(clock);
	m_time += clock.frame_time();
	spawnDrop();
	updateBumps();
	updateWater();
	updateDuck();
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
			XMVECTOR dx = { 2.f * h, m_z1[x + 2][y + 1] - m_z1[x][y + 1], 0.f };
			XMVECTOR dz = { 0.f, m_z1[x + 1][y] - m_z1[x + 1][y + 2], 2.f * h };
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(dz, dx));

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

void Duck::updateDuck()
{
	//Update control points
	if (m_time > BSPLINE_SEGMENT_TIME)
	{
		//modf(m_time / BSPLINE_SEGMENT_TIME, &m_time);
		//m_time *= BSPLINE_SEGMENT_TIME;
		while (m_time > BSPLINE_SEGMENT_TIME)
			m_time -= BSPLINE_SEGMENT_TIME;
		m_controlPoints.pop_front();
		addNewControlPoint();
	}

	//Calculate new position
	float t = m_time / BSPLINE_SEGMENT_TIME; //normalized to [0,1]
	m_duckPos = evaluateCubicBSpline(t);
	
	//Update model
	auto& duck = model(m_duck);
	XMFLOAT4X4 modelMtx;
	XMStoreFloat4x4(&modelMtx, XMMatrixScaling(5e-2f, 5e-2f, 5e-2f) * XMMatrixTranslationFromVector(20.0f * m_duckPos));
	duck.setTransform(modelMtx);
}

void Duck::calculateDamping()
{
	for (UINT i = 0; i < N; i++) {
		for (UINT j = 0; j < N; j++) {
			float l = min({ i,N - i,j,N - j }) * h;
			m_d[i][j] = 0.95f * min(1.f, l / 0.2f);
		}
	}
}

void Duck::updateBumps()
{
	for (UINT i = 1; i <= N; i++) {
		for (UINT j = 1; j <= N; j++) {
			m_z2[i][j] = m_d[i - 1][j - 1] * (A * (m_z1[i + 1][j] + m_z1[i - 1][j] + m_z1[i][j - 1] + m_z1[i][j + 1]) + B * m_z1[i][j] - m_z2[i][j]);
		}
	}

	std::swap(m_z1, m_z2);
}

void Duck::spawnDrop()
{
	const auto p = m_variables.GetVariable("drops");
	float probability;
	p->copyTo(&probability, sizeof(probability));
	if (m_spawnProbability(m_randomGen) < probability) {
		UINT i = m_spawnPosition(m_randomGen);
		UINT j = m_spawnPosition(m_randomGen);
		float height = m_spawnHeight(m_randomGen);
		m_z1[i][j] = height;
	}
}

void Duck::addNewControlPoint()
{
	float x = m_controlPointPosition(m_randomGen);
	float z = m_controlPointPosition(m_randomGen);
	m_controlPoints.push_back({ x,0.f,z,0.f });
}

XMVECTOR Duck::evaluateCubicBSpline(float t)
{
	float scale = 1.f / 6.f;
	float w0 = (((-t + 3.f) * t - 3.f) * t + 1.f) * scale;
	float w1 = (((3.f * t - 6.f) * t + 0.f) * t + 4.f) * scale;
	float w2 = (((-3.f * t + 3.f) * t + 3.f) * t + 1.f) * scale;
	float w3 = t * t * t * scale;

	return w0 * m_controlPoints[0] + w1 * m_controlPoints[1] + w2 * m_controlPoints[2] + w3 * m_controlPoints[3];
}
