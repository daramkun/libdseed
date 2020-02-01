#ifndef __DSEED_GRAPHICS_D3D11_SPRITE_HXX__
#define __DSEED_GRAPHICS_D3D11_SPRITE_HXX__

#include <dseed.h>

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Contant Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_vgabuffer : public dseed::graphics::vgabuffer
{
public:
	__d3d11_sprite_vgabuffer (ID3D11Buffer* buffer);

public:
	virtual int32_t retain () override;
	virtual int32_t release () override;

public:
	virtual dseed::error_t native_object (void** nativeObject) override;

public:
	virtual dseed::error_t stride () noexcept override;
	virtual dseed::error_t length () noexcept override;

public:
	virtual dseed::graphics::vgabuffertype type () noexcept override;

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Pipeline
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_pipeline : public dseed::graphics::pipeline
{
public:
	__d3d11_sprite_pipeline (ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState, ID3D11SamplerState* samplerState);

public:
	virtual int32_t retain () override;
	virtual int32_t release () override;

public:
	virtual dseed::error_t native_object (void** nativeObject) override;

private:
	std::atomic<int32_t> _refCount;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Atlas
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_atlas : public dseed::graphics::sprite_atlas
{
public:
	__d3d11_sprite_atlas (ID3D11Resource* texture, ID3D11ShaderResourceView* srv, const dseed::rect2i* atlases, size_t atlas_count);

public:
	virtual int32_t retain () override;
	virtual int32_t release () override;

public:
	virtual dseed::error_t native_object (void** nativeObject) override;

public:
	virtual dseed::size2i size () noexcept override;
	virtual dseed::color::pixelformat format () noexcept override;

public:
	virtual dseed::rect2i atlas_element (size_t index) noexcept override;
	virtual size_t atlas_count () noexcept override;

private:
	std::atomic<int32_t> _refCount;

	std::vector<dseed::rect2i> atlases;

	Microsoft::WRL::ComPtr<ID3D11Resource> texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Render Target
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_rendertarget : public dseed::graphics::sprite_rendertarget
{
public:
	__d3d11_sprite_rendertarget (dseed::graphics::sprite_atlas* atlas, ID3D11RenderTargetView* rtv);

public:
	virtual int32_t retain () override;
	virtual int32_t release () override;

public:
	virtual dseed::error_t native_object (void** nativeObject) override;

public:
	virtual dseed::size2i size () noexcept override;
	virtual dseed::color::pixelformat format () noexcept override;

public:
	virtual dseed::error_t atlas (dseed::graphics::sprite_atlas** atlas) noexcept override;

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::graphics::sprite_atlas> spriteAtlas;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Render Command, Instance, Transform
//
////////////////////////////////////////////////////////////////////////////////////////////
struct SPRITE_INSTANCE_DATA
{
	dseed::f32x4x4 transform;
	dseed::f32x4 area;
	dseed::f32x4 color;
};

struct SPRITE_RENDER_COMMAND
{
	std::vector<dseed::autoref<dseed::graphics::sprite_rendertarget>> renderTargets;
	dseed::autoref<dseed::graphics::pipeline> pipeline;
	std::vector<dseed::autoref<dseed::graphics::sprite_atlas>> atlases;
	std::vector<dseed::autoref<dseed::graphics::vgabuffer>> constbufs;
	std::vector<SPRITE_INSTANCE_DATA> instances;

	SPRITE_RENDER_COMMAND ()
	{
		renderTargets.reserve (8);
		atlases.reserve (16);
		constbufs.reserve (16);
		instances.reserve (1024);
	}
};

struct SPRITE_TRANSFORM
{
	dseed::f32x4x4 worldTransform;
	dseed::f32x4x4 projectionTransform;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Renderer
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_render : public dseed::graphics::sprite_render
{
public:
	__d3d11_sprite_render (dseed::graphics::vgadevice* device);
	dseed::error_t initialize ();

public:
	virtual int32_t retain () override;
	virtual int32_t release () override;

public:
	virtual dseed::error_t native_object (void** nativeObject) override;

public:
	virtual dseed::error_t create_pipeline (dseed::graphics::shaderpack* pixelshader, const dseed::graphics::blendparams* blendparams,
		dseed::graphics::sprite_texfilter texfilter, dseed::graphics::pipeline** pipeline) noexcept override;
	virtual dseed::error_t create_atlas (dseed::bitmaps::bitmap* bitmap, const dseed::rect2i* atlases, size_t atlas_count, dseed::graphics::sprite_atlas** atlas) noexcept override;
	virtual dseed::error_t create_rendertarget (const dseed::size2i& size, dseed::color::pixelformat format, dseed::graphics::sprite_rendertarget** target) noexcept override;
	virtual dseed::error_t create_constant (size_t size, dseed::graphics::vgabuffer** constbuf, const void* dat = nullptr) noexcept override;

public:
	virtual dseed::error_t set_pipeline (dseed::graphics::pipeline* pipeline) noexcept override;
	virtual dseed::error_t set_rendertarget (dseed::graphics::sprite_rendertarget** rendertargets, size_t size) noexcept override;
	virtual dseed::error_t set_atlas (dseed::graphics::sprite_atlas** atlas, size_t size) noexcept override;
	virtual dseed::error_t set_constant (dseed::graphics::vgabuffer** constbuf, size_t size) noexcept override;

public:
	virtual dseed::error_t clear_rendertarget (dseed::graphics::sprite_rendertarget* rendertarget, const dseed::f32x4& color) noexcept override;
	virtual dseed::error_t update_constant (dseed::graphics::vgabuffer* constbuf, const void* buf, size_t offset, size_t length) noexcept override;
	virtual dseed::error_t update_atlas (dseed::graphics::sprite_atlas* atlas, dseed::bitmaps::bitmap* data) noexcept override;

public:
	virtual dseed::error_t begin (dseed::graphics::rendermethod method, const dseed::f32x4x4& transform) noexcept override;
	virtual dseed::error_t end () noexcept override;

public:
	virtual dseed::error_t draw (size_t atlas_index, const dseed::f32x4x4& transform, const dseed::f32x4& color) noexcept;

public:
	dseed::error_t update_backbuffer ()
	{
		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED (nativeObject.dxgiSwapChain->GetBuffer (0, __uuidof(ID3D11Texture2D), &backBuffer)))
			return dseed::error_fail;

		D3D11_TEXTURE2D_DESC textureDesc;
		backBuffer->GetDesc (&textureDesc);

		renderTargetSize = dseed::size2i (textureDesc.Width, textureDesc.Height);

		renderTargetView.ReleaseAndGetAddressOf ();
		if (FAILED (d3dDevice->CreateRenderTargetView (backBuffer.Get (), nullptr, &renderTargetView)))
			return dseed::error_fail;

		return dseed::error_good;
	}

private:
	void render_ready_common (ID3D11DeviceContext* deviceContext);
	void render_ready_command (ID3D11DeviceContext* deviceContext, const SPRITE_RENDER_COMMAND* command);

private:
	std::atomic<int32_t> _refCount;

	dseed::graphics::vgadevice* vgadevice;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> instanceConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState [3];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	dseed::size2i renderTargetSize;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> defaultPixelShader;

	SPRITE_TRANSFORM transformBuffer;

	bool session;
	dseed::graphics::rendermethod renderMethod;

	std::queue<SPRITE_RENDER_COMMAND*> commands;
	dseed::memorypool<SPRITE_RENDER_COMMAND> commandPool;
};

#endif