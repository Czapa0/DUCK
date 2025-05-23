#include "dxDevice.h"
#include "exceptions.h"
#include <cassert>
#include "WICTextureLoader.h"
#include <fstream>
#include "window.h"
#include "DDSTextureLoader.h"
#include <array>

using namespace std;
using namespace mini;
using namespace utils;
using namespace directx;
using namespace DirectX;

DxDevice::DxDevice(const windows::window& window)
{
	const swap_chain_info desc{ window.handle(), window.client_size() };
	ID3D11Device *d = nullptr;
	ID3D11DeviceContext *dc = nullptr;
	IDXGISwapChain *sc = nullptr;
	unsigned int creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	const std::array featureLevels{ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	auto const hr =
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
			featureLevels.data(), static_cast<UINT>(featureLevels.size()),
			D3D11_SDK_VERSION, &desc, &sc, &d, nullptr, &dc);
	m_device.reset(d);
	m_immediateContext.reset(dc);
	m_swapChain = DxSwapChain{ dx_ptr<IDXGISwapChain>{ sc } };
	if (FAILED(hr))
		throw winapi_error{ hr };
}


dx_ptr<ID3D11Texture2D> DxDevice::CreateTexture(const tex2d_info& desc, const subresource_data* data) const
{
	assert(m_device);
	ID3D11Texture2D* t = nullptr;
	auto hr = m_device->CreateTexture2D(&desc, data, &t);
	dx_ptr<ID3D11Texture2D> texture(t);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return texture;
}

dx_ptr<ID3D11Texture3D> DxDevice::CreateTexture(const tex3d_info& desc, const subresource_data* data) const
{
	assert(m_device);
	ID3D11Texture3D* t = nullptr;
	auto hr = m_device->CreateTexture3D(&desc, data, &t);
	dx_ptr<ID3D11Texture3D> texture(t);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return texture;
}

dx_ptr<ID3D11RenderTargetView> DxDevice::CreateRenderTargetView(const dx_ptr<ID3D11Texture2D>& backTexture) const
{
	assert(m_device);
	ID3D11RenderTargetView* bb = nullptr;
	auto hr = m_device->CreateRenderTargetView(backTexture.get(), nullptr, &bb);
	dx_ptr<ID3D11RenderTargetView> backBuffer(bb);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return backBuffer;
}

dx_ptr<ID3D11DepthStencilView> DxDevice::CreateDepthStencilView(unsigned int width, unsigned int height) const
{
	auto desc = tex2d_info::depth_stencil(width, height);
	auto depthTexture = CreateTexture(desc);
	return CreateDepthStencilView(depthTexture);
}


dx_ptr<ID3D11DepthStencilView> DxDevice::CreateDepthStencilView(const dx_ptr<ID3D11Texture2D>& depthTexture, const depth_stencil_view_info* desc) const
{
	assert(m_device);
	ID3D11DepthStencilView* dsv = nullptr;
	auto hr = m_device->CreateDepthStencilView(depthTexture.get(), desc, &dsv);
	dx_ptr<ID3D11DepthStencilView> depthStencilView(dsv);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return depthStencilView;
}
//
//dx_ptr<ID3D11Buffer> DxDevice::_CreateBuffer(const void * data, unsigned int byteLenght, D3D11_BIND_FLAG flag,
//			D3D11_USAGE usage) const
//{
//	buffer_info desc(byteLenght, flag, usage);
//	if ((usage & D3D11_USAGE_DYNAMIC) != 0 || (usage & D3D11_USAGE_STAGING) != 0)
//		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	return CreateBuffer(desc, data);
//}

dx_ptr<ID3D11Buffer> DxDevice::CreateBuffer(const buffer_info& desc, const void* pData) const
{
	assert(m_device);
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = pData;
	ID3D11Buffer* b = nullptr;
	auto hr = m_device->CreateBuffer(&desc, pData ? &data : nullptr, &b);
	dx_ptr<ID3D11Buffer> buffer(b);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return buffer;
}

dx_ptr<ID3D11BlendState> DxDevice::CreateBlendState(const blend_info& desc) const
{
	assert(m_device);
	ID3D11BlendState* s = nullptr;
	auto hr = m_device->CreateBlendState(&desc, &s);
	dx_ptr<ID3D11BlendState> state(s);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return state;
}

dx_ptr<ID3D11DepthStencilState> DxDevice::CreateDepthStencilState(const depth_stencil_info& desc) const
{
	assert(m_device);
	ID3D11DepthStencilState* s = nullptr;
	auto hr = m_device->CreateDepthStencilState(&desc, &s);
	dx_ptr<ID3D11DepthStencilState> state(s);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return state;
}

dx_ptr<ID3D11InputLayout> DxDevice::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
	unsigned int layoutElements, const BYTE* vsByteCode, size_t vsByteCodeSize) const
{
	assert(m_device);
	ID3D11InputLayout* il = nullptr;
	auto hr = m_device->CreateInputLayout(layout, layoutElements, vsByteCode, vsByteCodeSize, &il);
	dx_ptr<ID3D11InputLayout> inputLayout(il);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return inputLayout;
}

dx_ptr<ID3D11VertexShader> DxDevice::CreateVertexShader(const BYTE* byteCode, size_t byteCodeSize) const
{
	assert(m_device);
	ID3D11VertexShader* vs = nullptr;
	auto hr = m_device->CreateVertexShader(byteCode, byteCodeSize, nullptr, &vs);
	dx_ptr<ID3D11VertexShader> vertexShader(vs);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return vertexShader;
}

dx_ptr<ID3D11HullShader> DxDevice::CreateHullShader(const BYTE* byteCode, size_t byteCodeSize) const
{
	assert(m_device);
	ID3D11HullShader* hs = nullptr;
	auto hr = m_device->CreateHullShader(byteCode, byteCodeSize, nullptr, &hs);
	dx_ptr<ID3D11HullShader> hullShader(hs);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return hullShader;
}

dx_ptr<ID3D11DomainShader> DxDevice::CreateDomainShader(const BYTE* byteCode, size_t byteCodeSize) const
{
	assert(m_device);
	ID3D11DomainShader* ds = nullptr;
	auto hr = m_device->CreateDomainShader(byteCode, byteCodeSize, nullptr, &ds);
	dx_ptr<ID3D11DomainShader> domainShader(ds);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return domainShader;
}

dx_ptr<ID3D11GeometryShader> DxDevice::CreateGeometryShader(const BYTE* byteCode, size_t byteCodeSize) const
{
	assert(m_device);
	ID3D11GeometryShader* gs = nullptr;
	auto hr = m_device->CreateGeometryShader(byteCode, byteCodeSize, nullptr, &gs);
	dx_ptr<ID3D11GeometryShader> geometryShader(gs);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return geometryShader;
}

dx_ptr<ID3D11GeometryShader> DxDevice::CreateGeometryShader(const BYTE* byteCode, size_t byteCodeSize,
	const D3D11_SO_DECLARATION_ENTRY *soEntries, size_t soEntriesCount)
{
	assert(m_device);
	ID3D11GeometryShader *gs = nullptr;
	auto hr = m_device->CreateGeometryShaderWithStreamOutput(byteCode, static_cast<UINT>(byteCodeSize), soEntries, static_cast<UINT>(soEntriesCount), nullptr, 0, 0, nullptr, &gs);
	dx_ptr<ID3D11GeometryShader> geometryShader(gs);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return geometryShader;
}

dx_ptr<ID3D11PixelShader> DxDevice::CreatePixelShader(const BYTE* byteCode, size_t byteCodeSize) const
{
	assert(m_device);
	ID3D11PixelShader* ps = nullptr;
	auto hr = m_device->CreatePixelShader(byteCode, byteCodeSize, nullptr, &ps);
	dx_ptr<ID3D11PixelShader> pixelShader(ps);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return pixelShader;
}

dx_ptr<ID3D11RasterizerState> DxDevice::CreateRasterizerState(const rasterizer_info& desc) const
{
	assert(m_device);
	ID3D11RasterizerState* s = nullptr;
	auto hr = m_device->CreateRasterizerState(&desc, &s);
	dx_ptr<ID3D11RasterizerState> state(s);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return state;
}

dx_ptr<ID3D11SamplerState> DxDevice::CreateSamplerState(const sampler_info& desc) const
{
	assert(m_device);
	ID3D11SamplerState* s = nullptr;
	auto hr = m_device->CreateSamplerState(&desc, &s);
	dx_ptr<ID3D11SamplerState> sampler(s);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return sampler;
}
dx_ptr<ID3D11ShaderResourceView> DxDevice::CreateShaderResourceView(const BYTE* imageFileContent, size_t imageFileSize)
{
	assert(m_device);
	ID3D11ShaderResourceView* rv;
	HRESULT hr = 0;
	hr = CreateDDSTextureFromMemory(m_device.get(), m_immediateContext.get(),
		imageFileContent, imageFileSize, nullptr, &rv);
	if (FAILED(hr))
		hr = CreateWICTextureFromMemory(m_device.get(), m_immediateContext.get(),
			imageFileContent, imageFileSize, nullptr, &rv);
	dx_ptr<ID3D11ShaderResourceView> resourceView{ rv };
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return resourceView;
}

dx_ptr<ID3D11ShaderResourceView> DxDevice::CreateShaderResourceView(const wstring& texPath) const
{
	assert(m_device);
	ID3D11ShaderResourceView* rv;
	HRESULT hr = 0;
	const wstring ext{ L".dds" };
	if (texPath.size() > ext.size() && texPath.compare(texPath.size() - ext.size(), ext.size(), ext) == 0)
		hr = CreateDDSTextureFromFile(m_device.get(), m_immediateContext.get(), texPath.c_str(), nullptr, &rv);
	else
		hr = CreateWICTextureFromFile(m_device.get(), m_immediateContext.get(), texPath.c_str(), nullptr, &rv);
	dx_ptr<ID3D11ShaderResourceView> resourceView(rv);
	if (FAILED(hr))
		//Make sure CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); is called before first use of this function!
		throw utils::winapi_error{ hr };
	return resourceView;
}

dx_ptr<ID3D11ShaderResourceView> DxDevice::CreateShaderResourceView(ID3D11Resource* texture,
	const shader_resource_view_info* desc) const
{
	assert(m_device);
	ID3D11ShaderResourceView *srv;
	auto hr = m_device->CreateShaderResourceView(texture, desc, &srv);
	dx_ptr<ID3D11ShaderResourceView> resourceView(srv);
	if (FAILED(hr))
		throw utils::winapi_error{ hr };
	return resourceView;
}


vector<BYTE> DxDevice::LoadByteCode(const wstring& fileName)
{
	ifstream sIn(fileName, ios::in | ios::binary);
	if (!sIn)
		throw custom_error{ L"Unable to open " + fileName };
	sIn.seekg(0, ios::end);
	auto byteCodeLength = sIn.tellg();
	sIn.seekg(0, ios::beg);
	vector<BYTE> byteCode(static_cast<unsigned int>(byteCodeLength));
	if (!sIn.read(reinterpret_cast<char*>(byteCode.data()), byteCodeLength))
		throw custom_error{ L"Error reading" + fileName };
	sIn.close();
	return byteCode;
}