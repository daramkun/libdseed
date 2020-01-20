#ifndef __DSEED_GRAPHICS_D3D11_SPRITE_HXX__
#define __DSEED_GRAPHICS_D3D11_SPRITE_HXX__

#include <dseed.h>

#include <queue>

struct SPRITE_TRANSFORM
{
	dseed::f32x4x4 worldTransform;
	dseed::f32x4x4 projectionTransform;
};

struct SPRITE_INSTANCE_DATA
{
	dseed::f32x4x4 transform;
	dseed::f32x4 area;
	dseed::f32x4 color;
};

constexpr char vertexShaderCode[] = CODE (
	struct VERTEXSHADER_OUT
	{
		float4 position : SV_Position;
		float2 uv : TEXCOORD0;
		float4 color : COLOR;
	};

	cbuffer TRANSFORM : register (b0)
	{
		float4x4 worldTransform;
		float4x4 projectionTransform;
	};

	struct SPRITE_INSTANCE_DATA_RECORD
	{
		float4x4 transform;
		float4 atlas_area;
		float4 color;
	};

	cbuffer SPRITE_INSTANCE_DATA : register (b1)
	{
		SPRITE_INSTANCE_DATA_RECORD records[512];
	};

	VERTEXSHADER_OUT main (uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
	{
		VERTEXSHADER_OUT output;

		switch (vertexId)
		{
			case 0: // Left-Top
				output.position = float4 (-0.5f, -0.5f, 0, 1);
				output.uv = float2 (records[instanceId].atlas_area.x, records[instanceId].atlas_area.y);
				break;

			case 1: // Left-Bottom
				output.position = float4 (-0.5f, +0.5f, 0, 1);
				output.uv = float2 (records[instanceId].atlas_area.x, records[instanceId].atlas_area.w);
				break;

			case 2: // Right-Top
				output.position = float4 (+0.5f, -0.5f, 0, 1);
				output.uv = float2 (records[instanceId].atlas_area.z, records[instanceId].atlas_area.y);
				break;

			case 3: // Right-Bottom
				output.position = float4 (+0.5f, +0.5f, 0, 1);
				output.uv = float2 (records[instanceId].atlas_area.z, records[instanceId].atlas_area.w);
				break;
		}

		output.position = mul (mul (mul (output.position, records[instanceId].transform), worldTransform), projectionTransform);
		output.color = records[instanceId].color;

		return output;
	}
);

constexpr char pixelShaderCode[] = CODE (
	struct PIXELSHADER_IN
	{
		float4 position : SV_Position;
		float2 uv : TEXCOORD0;
		float4 color : COLOR;
	};

	SamplerState samplerState;
	Texture2D inputTexture : register (t0);

	float4 main (PIXELSHADER_IN input) : SV_Target
	{
		return inputTexture.Sample (samplerState, input.uv) * input.color;
	}
);

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Contant Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_vgabuffer : public dseed::graphics::vgabuffer
{
public:
	__d3d11_sprite_vgabuffer (ID3D11Buffer* buffer) : _refCount (1), buffer (buffer) { }

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto no = reinterpret_cast<dseed::graphics::d3d11_sprite_vgabuffer_nativeobject*>(nativeObject);
		buffer.As (&no->buffer);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t stride () noexcept override
	{
		D3D11_BUFFER_DESC bufferDesc;
		buffer.Get ()->GetDesc (&bufferDesc);
		return bufferDesc.ByteWidth;
	}
	virtual dseed::error_t length () noexcept override { return 1; }

public:
	virtual dseed::graphics::vgabuffertype type () noexcept override { return dseed::graphics::vgabuffertype::constant; }

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
	__d3d11_sprite_pipeline (ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState, ID3D11SamplerState* samplerState)
		: _refCount (1), pixelShader (pixelShader), blendState (blendState), samplerState (samplerState)
	{

	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto no = reinterpret_cast<dseed::graphics::d3d11_sprite_pipeline_nativeobject*>(nativeObject);
		pixelShader.As (&no->pixelShader);
		blendState.As (&no->blendState);
		samplerState.As (&no->samplerState);

		return dseed::error_good;
	}

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
	__d3d11_sprite_atlas (ID3D11Resource* texture, ID3D11ShaderResourceView* srv, const dseed::rect2i* atlases, size_t atlas_count)
		: _refCount (1), texture (texture), srv (srv)
	{
		for (size_t i = 0; i < atlas_count; ++i)
		{
			auto& atlas = atlases[i];
			this->atlases.push_back (atlas);
		}
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto no = reinterpret_cast<dseed::graphics::d3d11_sprite_atlas_nativeobject*>(nativeObject);
		texture.As (&no->texture);
		srv.As (&no->shaderResourceView);

		return dseed::error_good;
	}

public:
	virtual dseed::size2i size () noexcept override
	{
		Microsoft::WRL::ComPtr<ID3D11Texture1D> texture1d;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
		if (SUCCEEDED (texture.As (&texture2d)))
		{
			D3D11_TEXTURE2D_DESC desc;
			texture2d->GetDesc (&desc);
			return dseed::size2i (desc.Width, desc.Height);
		}
		else if (SUCCEEDED (texture.As (&texture3d)))
		{
			D3D11_TEXTURE3D_DESC desc;
			texture3d->GetDesc (&desc);
			return dseed::size2i (desc.Width, desc.Height);
		}
		else if (SUCCEEDED (texture.As (&texture2d)))
		{
			D3D11_TEXTURE1D_DESC desc;
			texture1d->GetDesc (&desc);
			return dseed::size2i (desc.Width, desc.ArraySize);
		}
		return dseed::size2i (-1, -1);
	}
	virtual dseed::color::pixelformat format () noexcept override
	{
		Microsoft::WRL::ComPtr<ID3D11Texture1D> texture1d;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
		if (SUCCEEDED (texture.As (&texture2d)))
		{
			D3D11_TEXTURE2D_DESC desc;
			texture2d->GetDesc (&desc);
			return DXGIPF_TO_DSEEDPF (desc.Format);
		}
		else if (SUCCEEDED (texture.As (&texture3d)))
		{
			D3D11_TEXTURE3D_DESC desc;
			texture3d->GetDesc (&desc);
			return DXGIPF_TO_DSEEDPF (desc.Format);
		}
		else if (SUCCEEDED (texture.As (&texture2d)))
		{
			D3D11_TEXTURE1D_DESC desc;
			texture1d->GetDesc (&desc);
			return DXGIPF_TO_DSEEDPF (desc.Format);
		}
		return dseed::color::pixelformat::unknown;
	}

public:
	virtual dseed::rect2i atlas_element (size_t index) noexcept override
	{
		if (index >= atlases.size ())
			dseed::rect2i (-1, -1, -1, -1);

		return atlases.at (index);
	}
	virtual size_t atlas_count () noexcept override { return atlases.size (); }

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
	__d3d11_sprite_rendertarget (dseed::graphics::sprite_atlas* atlas, ID3D11RenderTargetView* rtv)
		: _refCount (1), spriteAtlas (atlas), rtv (rtv)
	{

	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto no = reinterpret_cast<dseed::graphics::d3d11_sprite_rendertarget_nativeobject*>(nativeObject);
		rtv.As (&no->renderTargetView);

		return dseed::error_good;
	}

public:
	virtual dseed::size2i size () noexcept override { auto s = spriteAtlas->size (); return dseed::size2i (s.width, s.height); }
	virtual dseed::color::pixelformat format () noexcept override { return spriteAtlas->format (); }

public:
	virtual dseed::error_t atlas (dseed::graphics::sprite_atlas** atlas) noexcept override
	{
		if (atlas == nullptr) return dseed::error_invalid_args;
		(*atlas = spriteAtlas)->retain ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::graphics::sprite_atlas> spriteAtlas;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Render Command
//
////////////////////////////////////////////////////////////////////////////////////////////
struct SPRITE_RENDER_COMMAND
{
	std::vector<dseed::autoref<dseed::graphics::sprite_rendertarget>> renderTargets;
	dseed::autoref<dseed::graphics::pipeline> pipeline;
	std::vector<dseed::autoref<dseed::graphics::sprite_atlas>> atlases;
	std::vector<dseed::autoref<dseed::graphics::vgabuffer>> constbufs;
	std::vector<SPRITE_INSTANCE_DATA> instances;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Renderer
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_render : public dseed::graphics::sprite_render
{
public:
	__d3d11_sprite_render (dseed::graphics::vgadevice* device)
		: _refCount (1), vgadevice (device), session (false)
	{

	}

	dseed::error_t initialize ()
	{
		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);
		d3dDevice = nativeObject.d3dDevice.Get ();
		d3dDevice->GetImmediateContext (&immediateContext);

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob;
		if (FAILED (CompileHLSL ("vs_5_0", vertexShaderCode, &vertexShaderBlob)))
			return dseed::error_fail;

		if (FAILED (d3dDevice->CreateVertexShader (
			vertexShaderBlob.Get ()->GetBufferPointer (), vertexShaderBlob.Get ()->GetBufferSize (),
			nullptr, &vertexShader)))
			return dseed::error_fail;

		if (FAILED (CompileHLSL ("ps_5_0", pixelShaderCode, &pixelShaderBlob)))
			return dseed::error_fail;

		if (FAILED (d3dDevice->CreatePixelShader (
			pixelShaderBlob.Get ()->GetBufferPointer (), pixelShaderBlob.Get ()->GetBufferSize (),
			nullptr, &defaultPixelShader)))
			return dseed::error_fail;

		if (FAILED (CreateConstantBuffer (d3dDevice.Get (), sizeof (SPRITE_TRANSFORM), &transformConstantBuffer, nullptr)))
			return dseed::error_fail;

		if (FAILED (CreateConstantBuffer (d3dDevice.Get (), sizeof (SPRITE_INSTANCE_DATA) * 512, &instanceConstantBuffer, nullptr)))
			return dseed::error_fail;

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MinLOD = -FLT_MIN;
		samplerDesc.MaxLOD = FLT_MAX;

		if (FAILED (d3dDevice->CreateSamplerState (&samplerDesc, &samplerState[0])))
			return dseed::error_fail;

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		if (FAILED (d3dDevice->CreateSamplerState (&samplerDesc, &samplerState[1])))
			return dseed::error_fail;

		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = TRUE;

		if (FAILED (d3dDevice->CreateRasterizerState (&rasterizerDesc, &rasterizerState)))
			return dseed::error_fail;

		if (dseed::failed (update_backbuffer ()))
			return dseed::error_fail;

		return dseed::error_good;
	}

public:
	virtual int32_t retain () override { return ++_refCount; }
	virtual int32_t release () override
	{
		auto ret = --_refCount;
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual dseed::error_t native_object (void** nativeObject) override
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t create_pipeline (dseed::graphics::shaderpack* pixelshader, const dseed::graphics::blendparams* blendparams,
		dseed::graphics::sprite_texfilter texfilter, dseed::graphics::pipeline** pipeline) noexcept override
	{
		if (pipeline == nullptr) return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

		if (pixelshader != nullptr)
		{
			dseed::autoref<dseed::blob> pixelShaderCodeBlob;
			if (dseed::failed (pixelshader->at (dseed::graphics::shaderlang_dxil, &pixelShaderCodeBlob)))
			{
				if (dseed::failed (pixelshader->at (dseed::graphics::shaderlang_hlsl, &pixelShaderCodeBlob)))
					return dseed::error_invalid_args;

				Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
				if (FAILED (CompileHLSL ("ps_5_0", (const char*)pixelShaderCodeBlob->ptr (), &pixelShaderBlob)))
					return dseed::error_fail;

				if (FAILED (nativeObject.d3dDevice->CreatePixelShader (
					pixelShaderBlob.Get ()->GetBufferPointer (), pixelShaderBlob.Get ()->GetBufferSize (),
					nullptr, &defaultPixelShader)))
					return dseed::error_fail;
			}
			else
			{
				if (FAILED (nativeObject.d3dDevice->CreatePixelShader (
					pixelShaderCodeBlob->ptr (), pixelShaderCodeBlob->size (),
					nullptr, &defaultPixelShader)))
					return dseed::error_fail;
			}
		}
		else
			pixelShader = defaultPixelShader;

		if (blendparams != nullptr)
		{
			if (FAILED (CreateBlendStateFromBlendParams (nativeObject.d3dDevice.Get (), *blendparams, &blendState)))
				return dseed::error_invalid_args;
		}
		else
		{
			dseed::graphics::blendparams alphablend = dseed::graphics::blendparams::alphablend ();
			if (FAILED (CreateBlendStateFromBlendParams (nativeObject.d3dDevice.Get (), alphablend, &blendState)))
				return dseed::error_invalid_args;
		}

		*pipeline = new __d3d11_sprite_pipeline (pixelShader.Get (), blendState.Get (), samplerState[(int)texfilter].Get ());
		if (*pipeline == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_atlas (dseed::bitmaps::bitmap* bitmap, const dseed::rect2i* atlases, size_t atlas_count, dseed::graphics::sprite_atlas** atlas) noexcept override
	{
		if (bitmap == nullptr || atlas == nullptr)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		dseed::autoref<dseed::bitmaps::bitmap> convBitmap;
		if (vgadevice->is_support_format (bitmap->format ()))
			convBitmap = bitmap;
		else
		{
			if (dseed::failed (dseed::bitmaps::reformat_bitmap (bitmap, dseed::color::pixelformat::rgba8, &convBitmap)))
				return dseed::error_fail;
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		if (FAILED (CreateTextureFromBitmap (nativeObject.d3dDevice.Get (), convBitmap, (ID3D11Resource**)texture.GetAddressOf (), &srv, nullptr)))
			return dseed::error_fail;

		dseed::rect2i defaultAtlas (0, 0, bitmap->size ().width, bitmap->size ().height);
		if (atlas_count == 0)
		{
			atlases = &defaultAtlas;
			atlas_count = 1;
		}

		*atlas = new __d3d11_sprite_atlas (texture.Get (), srv.Get (), atlases, atlas_count);
		if (*atlas == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_rendertarget (const dseed::size2i& size, dseed::color::pixelformat format, dseed::graphics::sprite_rendertarget** target) noexcept override
	{
		if (target == nullptr || DSEEDPF_TO_DXGIPF (format) == DXGI_FORMAT_UNKNOWN)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> backBufferSRV;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		if (FAILED (CreateTexture2D (nativeObject.d3dDevice.Get (), size, format, &backBuffer, &backBufferSRV, nullptr, &rtv)))
			return dseed::error_fail;

		dseed::rect2i atlasSize (0, 0, size.width, size.height);

		dseed::autoref<dseed::graphics::sprite_atlas> atlas;
		*&atlas = new __d3d11_sprite_atlas (backBuffer.Get (), backBufferSRV.Get (), &atlasSize, 1);
		if (atlas == nullptr)
			return dseed::error_out_of_memory;

		*target = new __d3d11_sprite_rendertarget (atlas, rtv.Get ());
		if (*target == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_constant (size_t size, dseed::graphics::vgabuffer** constbuf, const void* dat = nullptr) noexcept override
	{
		if (constbuf == nullptr || size == 0)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		if (FAILED (CreateConstantBuffer (nativeObject.d3dDevice.Get (), size, &constantBuffer, dat)))
			return dseed::error_fail;

		*constbuf = new __d3d11_sprite_vgabuffer (constantBuffer.Get ());
		if (*constbuf == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_pipeline (dseed::graphics::pipeline* pipeline) noexcept override
	{
		if (!session)
			return dseed::error_invalid_op;
		if (pipeline == nullptr)
			return dseed::error_invalid_args;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			// Insert first command
			if (commands.empty ())
			{
				SPRITE_RENDER_COMMAND command;
				command.pipeline = pipeline;
				commands.push (command);
			}
			// Command no have instance: Change Atlases
			else if (commands.back ().instances.size () == 0)
			{
				commands.back ().pipeline = pipeline;
			}
			// Insert command based last command
			else
			{
				SPRITE_RENDER_COMMAND command = commands.back ();
				command.pipeline = pipeline;
				commands.push (command);
			}
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			commands.front ().pipeline = pipeline;

			dseed::graphics::d3d11_sprite_pipeline_nativeobject no;
			pipeline->native_object ((void**)&no);

			immediateContext->PSSetShader (no.pixelShader.Get (), nullptr, 0);
			immediateContext->OMSetBlendState (no.blendState.Get (), nullptr, 0xffffffff);
			ID3D11SamplerState* samplerState = no.samplerState.Get ();
			immediateContext->PSSetSamplers (0, 1, &samplerState);
		}

		return dseed::error_good;
	}
	virtual dseed::error_t set_rendertarget (dseed::graphics::sprite_rendertarget** rendertargets, size_t size) noexcept override
	{
		if (!session)
			return dseed::error_invalid_op;
		if (rendertargets == nullptr || size == 0)
			return dseed::error_invalid_args;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			// Insert first command
			if (commands.empty ())
			{
				SPRITE_RENDER_COMMAND command;
				for (size_t i = 0; i < size; ++i)
					command.renderTargets.push_back (rendertargets[i]);
				commands.push (command);
			}
			// Command no have instance: Change Atlases
			else if (commands.back ().instances.size () == 0)
			{
				commands.back ().renderTargets.clear ();
				for (size_t i = 0; i < size; ++i)
					commands.back ().renderTargets.push_back (rendertargets[i]);
			}
			// Insert command based last command
			else
			{
				SPRITE_RENDER_COMMAND command = commands.back ();
				command.renderTargets.clear ();
				for (size_t i = 0; i < size; ++i)
					command.renderTargets.push_back (rendertargets[i]);
				commands.push (command);
			}
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			commands.front ().renderTargets.clear ();

			std::vector<ID3D11RenderTargetView*> rtv;
			std::vector<D3D11_VIEWPORT> viewports;
			for (size_t i = 0; i < size; ++i)
			{
				commands.front ().renderTargets.push_back (rendertargets[i]);

				if (rendertargets[i] == nullptr)
				{
					rtv.push_back (renderTargetView.Get ());

					D3D11_VIEWPORT viewport = {};
					viewport.Width = renderTargetSize.width;
					viewport.Height = renderTargetSize.height;
					viewport.MaxDepth = 1.0f;
					viewports.push_back (viewport);
				}
				else
				{
					dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
					rendertargets[i]->native_object ((void**)&nativeObject);
					rtv.push_back (nativeObject.renderTargetView.Get ());

					dseed::autoref<dseed::graphics::sprite_atlas> atlas;
					rendertargets[i]->atlas (&atlas);

					D3D11_VIEWPORT viewport = {};
					viewport.Width = atlas->size ().width;
					viewport.Height = atlas->size ().height;
					viewport.MaxDepth = 1.0f;
					viewports.push_back (viewport);
				}
			}
			immediateContext->OMSetRenderTargets (rtv.size (), rtv.data (), nullptr);
			immediateContext->RSSetViewports (viewports.size (), viewports.data ());

			transformBuffer.projectionTransform = dseed::float4x4::orthographic_offcenter (0, (float)viewports[0].Width, (float)viewports[0].Height, 0, 0.00001f, 32767.0f);
			immediateContext->UpdateSubresource (transformConstantBuffer.Get (), 0, nullptr, &transformBuffer, sizeof (SPRITE_TRANSFORM), 0);
		}

		return dseed::error_good;
	}
	virtual dseed::error_t set_atlas (dseed::graphics::sprite_atlas** atlas, size_t size) noexcept override
	{
		if (!session)
			return dseed::error_invalid_op;
		if (atlas == nullptr || size == 0)
			return dseed::error_invalid_args;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			// Insert first command
			if (commands.empty ())
			{
				SPRITE_RENDER_COMMAND command;
				for (size_t i = 0; i < size; ++i)
					command.atlases.push_back (atlas[i]);
				commands.push (command);
			}
			// Command no have instance: Change Atlases
			else if (commands.back ().instances.size () == 0)
			{
				commands.back ().atlases.clear ();
				for (size_t i = 0; i < size; ++i)
					commands.back ().atlases.push_back (atlas[i]);
			}
			// Insert command based last command
			else
			{
				SPRITE_RENDER_COMMAND command = commands.back ();
				command.atlases.clear ();
				for (size_t i = 0; i < size; ++i)
					command.atlases.push_back (atlas[i]);
				commands.push (command);
			}
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			commands.front ().atlases.clear ();

			std::vector<ID3D11ShaderResourceView*> srv;
			for (size_t i = 0; i < size; ++i)
			{
				commands.front ().atlases.push_back (atlas[i]);

				dseed::graphics::d3d11_sprite_atlas_nativeobject nativeObject;
				atlas[i]->native_object ((void**)&nativeObject);
				srv.push_back (nativeObject.shaderResourceView.Get ());
			}
			immediateContext->PSSetShaderResources (0, srv.size (), srv.data ());
		}

		return dseed::error_good;
	}
	virtual dseed::error_t set_constant (dseed::graphics::vgabuffer** constbuf, size_t size) noexcept override
	{
		if (!session)
			return dseed::error_invalid_op;
		if (constbuf == nullptr || size == 0)
			return dseed::error_invalid_args;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			// Insert first command
			if (commands.empty ())
			{
				SPRITE_RENDER_COMMAND command;
				for (size_t i = 0; i < size; ++i)
					command.constbufs.push_back (constbuf[i]);
				commands.push (command);
			}
			// Command no have instance: Change Atlases
			else if (commands.back ().instances.size () == 0)
			{
				commands.back ().constbufs.clear ();
				for (size_t i = 0; i < size; ++i)
					commands.back ().constbufs.push_back (constbuf[i]);
			}
			// Insert command based last command
			else
			{
				SPRITE_RENDER_COMMAND command = commands.back ();
				command.atlases.clear ();
				for (size_t i = 0; i < size; ++i)
					command.constbufs.push_back (constbuf[i]);
				commands.push (command);
			}
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			commands.front ().constbufs.clear ();

			std::vector<ID3D11Buffer*> buf;
			for (size_t i = 0; i < size; ++i)
			{
				commands.front ().constbufs.push_back (constbuf[i]);
				dseed::graphics::d3d11_sprite_vgabuffer_nativeobject nativeObject;
				constbuf[i]->native_object ((void**)&nativeObject);
				buf.push_back (nativeObject.buffer.Get ());
			}
			immediateContext->PSSetConstantBuffers (0, buf.size (), buf.data ());
		}

		return dseed::error_good;
	}

public:
	virtual dseed::error_t clear_rendertarget (dseed::graphics::sprite_rendertarget* rendertarget, const dseed::f32x4& color) noexcept
	{
		if (session)
			return dseed::error_invalid_op;

		if (rendertarget != nullptr)
		{
			dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
			rendertarget->native_object ((void**)&nativeObject);
			immediateContext->ClearRenderTargetView (nativeObject.renderTargetView.Get (), (const float*)&color);
		}
		else
			immediateContext->ClearRenderTargetView (renderTargetView.Get (), (const float*)&color);

		return dseed::error_good;
	}
	virtual dseed::error_t update_constant (dseed::graphics::vgabuffer* constbuf, const void* buf, size_t offset, size_t length) noexcept
	{
		if (session && renderMethod == dseed::graphics::rendermethod::deferred)
			return dseed::error_invalid_op;
		if (constbuf == nullptr || buf == nullptr || length == 0)
			return dseed::error_invalid_args;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		constbuf->native_object (&buffer);

		D3D11_BOX destBox = {};
		destBox.left = (UINT)offset;
		destBox.right = (UINT)(offset + length);
		immediateContext->UpdateSubresource (buffer.Get (), 0, &destBox, buf, (UINT)length, 0);

		return dseed::error_good;
	}
	virtual dseed::error_t update_atlas (dseed::graphics::sprite_atlas* atlas, dseed::bitmaps::bitmap* data) noexcept
	{
		return dseed::error_not_impl;
	}

public:
	virtual dseed::error_t begin (dseed::graphics::rendermethod method, const dseed::f32x4x4& transform) noexcept
	{
		if (session) return dseed::error_invalid_op;
		if (!(method == dseed::graphics::rendermethod::deferred || method == dseed::graphics::rendermethod::forward))
			return dseed::error_invalid_args;

		session = true;
		renderMethod = method;

		immediateContext->ClearState ();

		transformBuffer.worldTransform = transform;

		if (method == dseed::graphics::rendermethod::forward)
		{
			render_ready_common (immediateContext.Get ());
			commands.push (SPRITE_RENDER_COMMAND ());
		}

		return dseed::error_good;
	}
	virtual dseed::error_t end () noexcept
	{
		if (!session)
			return dseed::error_invalid_op;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			render_ready_common (immediateContext.Get ());

			while (!commands.empty ())
			{
				SPRITE_RENDER_COMMAND& command = commands.front ();

				render_ready_command (immediateContext.Get (), command);

				auto instanceCount = command.instances.size ();
				auto loopCount = (size_t)ceil (instanceCount / 512.0f);
				command.instances.resize (loopCount * 512);
				for (size_t i = 0; i < instanceCount / 512; ++i)
				{
					size_t drawCount = dseed::minimum<size_t> (512, instanceCount - (i * 512));
					immediateContext->UpdateSubresource (instanceConstantBuffer.Get (), 0, nullptr,
						command.instances.data () + (i * 512), sizeof (SPRITE_INSTANCE_DATA) * drawCount, 0);
					immediateContext->DrawInstanced (4, drawCount, 0, 0);
				}

				commands.pop ();
			}
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			commands.pop ();
		}

		session = false;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t draw (size_t atlas_index, const dseed::f32x4x4& transform, const dseed::f32x4& color) noexcept
	{
		if (!session)
			return dseed::error_invalid_args;

		if (commands.size () == 0)
			return dseed::error_good;

		SPRITE_RENDER_COMMAND& command = commands.back ();
		if (command.renderTargets.size () == 0 || command.atlases.size () == 0 || command.pipeline == nullptr)
			return dseed::error_good;

		const dseed::rect2i atlas = command.atlases[0]->atlas_element (atlas_index);
		auto size = command.atlases[0]->size ();

		SPRITE_INSTANCE_DATA record = {};
		record.area = dseed::f32x4 (
			atlas.x / (float)size.width,
			atlas.y / (float)size.height,
			atlas.width / (float)size.width,
			atlas.height / (float)size.height);
		record.transform = dseed::float4x4::scale (size.width, size.height, 1) * transform;
		record.color = color;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			command.instances.push_back (record);
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			static SPRITE_INSTANCE_DATA records[512];

			records[0] = record;

			immediateContext->UpdateSubresource (instanceConstantBuffer.Get (), 0, nullptr, records, sizeof (SPRITE_INSTANCE_DATA) * 512, 0);
			immediateContext->DrawInstanced (4, 1, 0, 0);
		}

		return dseed::error_good;
	}

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
	void render_ready_common (ID3D11DeviceContext* deviceContext)
	{	
		// Set Vertex Shader, Rasterizer State, Sampler State, ... to Device Context
		{
			deviceContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			deviceContext->VSSetShader (vertexShader.Get (), nullptr, 0);
			ID3D11Buffer* vsConstantBuffers[] = { transformConstantBuffer.Get (), instanceConstantBuffer.Get () };
			deviceContext->VSSetConstantBuffers (0, _countof (vsConstantBuffers), vsConstantBuffers);
			deviceContext->RSSetState (rasterizerState.Get ());
		}
	}

	void render_ready_command (ID3D11DeviceContext* deviceContext, const SPRITE_RENDER_COMMAND& command)
	{
		// Set Render Target to Device Context
		{
			std::vector<ID3D11RenderTargetView*> rtv;
			std::vector<D3D11_VIEWPORT> viewports;
			for (size_t i = 0; i < command.renderTargets.size (); ++i)
			{
				if (command.renderTargets[i] == nullptr)
				{
					rtv.push_back (renderTargetView.Get ());

					D3D11_VIEWPORT viewport = {};
					viewport.Width = renderTargetSize.width;
					viewport.Height = renderTargetSize.height;
					viewport.MaxDepth = 1.0f;
					viewports.push_back (viewport);
				}
				else
				{
					dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
					command.renderTargets[i]->native_object ((void**)&nativeObject);
					rtv.push_back (nativeObject.renderTargetView.Get ());

					dseed::autoref<dseed::graphics::sprite_atlas> atlas;
					command.renderTargets[i]->atlas (&atlas);

					D3D11_VIEWPORT viewport = {};
					viewport.Width = atlas->size ().width;
					viewport.Height = atlas->size ().height;
					viewport.MaxDepth = 1.0f;
					viewports.push_back (viewport);
				}
			}
			immediateContext->OMSetRenderTargets (rtv.size (), rtv.data (), nullptr);
			immediateContext->RSSetViewports (viewports.size (), viewports.data ());

			transformBuffer.projectionTransform = dseed::float4x4::orthographic_offcenter (0, (float)viewports[0].Width, (float)viewports[0].Height, 0, 0.00001f, 32767.0f);
			immediateContext->UpdateSubresource (transformConstantBuffer.Get (), 0, nullptr, &transformBuffer, sizeof (SPRITE_TRANSFORM), 0);
		}

		// Set Pipeline to Device Context
		{
			dseed::graphics::d3d11_sprite_pipeline_nativeobject no;
			command.pipeline->native_object ((void**)&no);

			deviceContext->PSSetShader (no.pixelShader.Get (), nullptr, 0);
			deviceContext->OMSetBlendState (no.blendState.Get (), nullptr, 0xffffffff);
			ID3D11SamplerState* samplerState = no.samplerState.Get ();
			deviceContext->PSSetSamplers (0, 1, &samplerState);
		}

		// Set Constant Buffers to Pixel Shader for Device Context
		if (command.constbufs.size () > 0)
		{
			std::vector<ID3D11Buffer*> constantBuffers;
			for (auto& cb : command.constbufs)
			{
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11Buffer;
				cb->native_object (&d3d11Buffer);
				constantBuffers.push_back (d3d11Buffer.Get ());
			}
			deviceContext->PSSetConstantBuffers (0, (UINT)constantBuffers.size (), constantBuffers.data ());
		}
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::graphics::vgadevice* vgadevice;

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> instanceConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState [2];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	dseed::size2i renderTargetSize;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> defaultPixelShader;

	SPRITE_TRANSFORM transformBuffer;

	bool session;
	dseed::graphics::rendermethod renderMethod;

	std::queue<SPRITE_RENDER_COMMAND> commands;
};

#endif