#define USE_D3D11_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT

#	include <wrl.h>
#	include "common_dxgi.hxx"

#	include <d3d11.h>
#	if NTDDI_VERSION >= NTDDI_WIN10
#		include <d3d11_4.h>
#	endif
#	include <d3dcompiler.h>

constexpr D3D11_BLEND_OP DSEEDBLENDOP_TO_D3D11BLENDOP (dseed::graphics::blendop blendop)
{
	switch (blendop)
	{
		case dseed::graphics::blendop::add: return D3D11_BLEND_OP_ADD; break;
		case dseed::graphics::blendop::subtract: return D3D11_BLEND_OP_SUBTRACT; break;
		case dseed::graphics::blendop::reversed_subtract: return D3D11_BLEND_OP_REV_SUBTRACT; break;
		case dseed::graphics::blendop::min: return D3D11_BLEND_OP_MIN; break;
		case dseed::graphics::blendop::max: return D3D11_BLEND_OP_MAX; break;
		default: return (D3D11_BLEND_OP) -1;
	}
}

constexpr D3D11_BLEND DSEEDBLENDVALUE_TO_D3D11_BLENDVALUE (dseed::graphics::blendvalue bv)
{
	switch (bv)
	{
		case dseed::graphics::blendvalue::zero: return D3D11_BLEND_ZERO; break;
		case dseed::graphics::blendvalue::one: return D3D11_BLEND_ONE; break;
		case dseed::graphics::blendvalue::src_color: return D3D11_BLEND_SRC_COLOR; break;
		case dseed::graphics::blendvalue::src_alpha: return D3D11_BLEND_SRC_ALPHA; break;
		case dseed::graphics::blendvalue::dest_color: return D3D11_BLEND_DEST_COLOR; break;
		case dseed::graphics::blendvalue::dest_alpha: return D3D11_BLEND_DEST_ALPHA; break;
		case dseed::graphics::blendvalue::inverted_src_color: return D3D11_BLEND_INV_SRC_COLOR; break;
		case dseed::graphics::blendvalue::inverted_src_alpha: return D3D11_BLEND_INV_SRC_ALPHA; break;
		case dseed::graphics::blendvalue::inverted_dest_color: return D3D11_BLEND_INV_DEST_COLOR; break;
		case dseed::graphics::blendvalue::inverted_dest_alpha: return D3D11_BLEND_INV_DEST_ALPHA; break;
		default: (D3D11_BLEND) -1;
	}
}

struct SPRITE_TRANSFORM
{
	dseed::f32x4x4 worldTransform;
	dseed::f32x4x4 projectionTransform;
};

struct SPRITE_INSTANCE_DATA_RECORD
{
	dseed::f32x4x4 transform;
	dseed::f32x4 area;
	dseed::f32x4 color;
	uint32_t textureIndex;
	uint32_t width, height;
	uint32_t junk1;
};

struct SPRITE_INSTANCE_DATA
{
	SPRITE_INSTANCE_DATA_RECORD datas[512];
};

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

		auto no = reinterpret_cast<ID3D11Buffer**>(nativeObject);
		buffer.Get ()->QueryInterface (no);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t stride () noexcept
	{
		D3D11_BUFFER_DESC bufferDesc;
		buffer.Get ()->GetDesc (&bufferDesc);
		return bufferDesc.ByteWidth;
	}
	virtual dseed::error_t length () noexcept { return 1; }

public:
	virtual dseed::graphics::vgabuffertype type () noexcept { return dseed::graphics::vgabuffertype::constant; }

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
	__d3d11_sprite_pipeline (ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState)
		: _refCount (1), pixelShader (pixelShader), blendState (blendState)
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
	dseed::error_t apply_pipeline (ID3D11DeviceContext* deviceContext)
	{
		if (deviceContext == nullptr)
			return dseed::error_invalid_args;

		float blendFactor[] = {0, 0, 0, 0};
		deviceContext->OMSetBlendState (blendState.Get (), blendFactor, 0);
		deviceContext->PSSetShader (pixelShader.Get (), nullptr, 0);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Atlas
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_sprite_atlas : public dseed::graphics::sprite_atlas
{
public:
	__d3d11_sprite_atlas (ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv, const dseed::rect2i* atlases, size_t atlas_count)
		: _refCount (1), texture2d (texture), srv (srv)
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
	virtual dseed::size2i size () noexcept
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2d->GetDesc (&desc);
		return dseed::size2i (desc.Width, desc.Height);
	}
	virtual dseed::color::pixelformat format () noexcept
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2d->GetDesc (&desc);
		return DXGIPF_TO_DSEEDPF (desc.Format);
	}

public:
	virtual dseed::rect2i atlas_element (size_t index) noexcept
	{
		if (index >= atlases.size ())
			dseed::rect2i (-1, -1, -1, -1);

		return atlases.at (index);
	}
	virtual size_t atlas_count () noexcept { return atlases.size (); }

public:
	dseed::error_t apply_atlas (ID3D11DeviceContext* deviceContext, size_t i)
	{
		if (deviceContext == nullptr)
			return dseed::error_invalid_args;
		if (i >= 16)
			return dseed::error_out_of_range;

		ID3D11ShaderResourceView* tempSrv = srv.Get ();
		deviceContext->PSSetShaderResources ((UINT)i, 1, &tempSrv);

		return dseed::error_good;
	}
	dseed::error_t get_srv (ID3D11ShaderResourceView** srv)
	{
		(*srv = this->srv.Get ())->AddRef ();
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	std::vector<dseed::rect2i> atlases;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
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
	__d3d11_sprite_rendertarget (dseed::graphics::sprite_atlas* atlas, ID3D11RenderTargetView* rtv, dseed::graphics::sprite_atlas* depthStencilBuffer, ID3D11DepthStencilView* dsv)
		: _refCount (1), spriteAtlas (atlas), rtv (rtv), depthStencilBuffer (depthStencilBuffer), dsv (dsv)
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
	virtual dseed::size2i size () noexcept { auto s = spriteAtlas->size (); return dseed::size2i (s.width, s.height); }
	virtual dseed::color::pixelformat format () noexcept { return spriteAtlas->format (); }

public:
	virtual dseed::error_t atlas (dseed::graphics::sprite_atlas** atlas) noexcept override
	{
		if (atlas == nullptr) return dseed::error_invalid_args;
		(*atlas = spriteAtlas)->retain ();
		return dseed::error_good;
	}
	virtual dseed::error_t depth_atlas (dseed::graphics::sprite_atlas** atlas) noexcept override
	{
		if (atlas == nullptr) return dseed::error_invalid_args;
		(*atlas = depthStencilBuffer)->retain ();
		return dseed::error_good;
	}

public:
	dseed::error_t apply_rendertarget (ID3D11DeviceContext* deviceContext)
	{
		if (deviceContext == nullptr)
			return dseed::error_invalid_args;

		ID3D11RenderTargetView* tempRtv = rtv.Get ();
		deviceContext->OMSetRenderTargets (1, &tempRtv, dsv.Get ());

		dseed::size2i size = spriteAtlas->size ();

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = viewport.TopLeftY = 0;
		viewport.Width = (float)size.width;
		viewport.Height = (float)size.height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		deviceContext->RSSetViewports (1, &viewport);

		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;

	dseed::autoref<dseed::graphics::sprite_atlas> spriteAtlas;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;

	dseed::autoref<dseed::graphics::sprite_atlas> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
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

		const char vertexShaderCode [] = CODE (
			struct VERTEXSHADER_OUT
			{
				float4 position : SV_Position;
				float2 uv : TEXCOORD0;
				float4 color : COLOR;
				uint textureIndex : TEXCOORD1;
				uint instanceId : TEXCOORD2;
			};

			cbuffer TRANSFORM
			{
				float4x4 worldTransform;
				float4x4 projectionTransform;
			};

			struct SPRITE_INSTANCE_DATA_RECORD
			{
				float4x4 transform;
				float4 atlas_area;
				float4 color;
				uint4 textureIndexAndSize;
			};

			cbuffer SPRITE_INSTANCE_DATA
			{
				SPRITE_INSTANCE_DATA_RECORD records[512];
			};

			VERTEXSHADER_OUT main (uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
			{
				VERTEXSHADER_OUT output;

				switch (vertexId)
				{
					case 0:
						output.position = float4 (0, 0, 0, 1);
						output.uv = float2 (records[instanceId].atlas_area.x, records[instanceId].atlas_area.y);
						break;

					case 1:
						output.position = float4 (1, 0, 0, 1);
						output.uv = float2 (records[instanceId].atlas_area.z, records[instanceId].atlas_area.y);
						break;

					case 2:
						output.position = float4 (0, 1, 0, 1);
						output.uv = float2 (records[instanceId].atlas_area.x, records[instanceId].atlas_area.w);
						break;

					case 3:
						output.position = float4 (1, 1, 0, 1);
						output.uv = float2 (records[instanceId].atlas_area.z, records[instanceId].atlas_area.w);
						break;
				}
				
				output.position = mul (mul (mul (output.position
					* float4 ((float)records[instanceId].textureIndexAndSize.y, (float)records[instanceId].textureIndexAndSize.z, 0, 1), worldTransform)
					, records[instanceId].transform), projectionTransform);
				output.color = records[instanceId].color;
				output.textureIndex = records[instanceId].textureIndexAndSize.x;
				output.instanceId = instanceId;

				return output;
			}
		);

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob, errorMsgBlob;

		if (FAILED (D3DCompile2 (vertexShaderCode, _countof (vertexShaderCode), nullptr, nullptr, nullptr,
			"main", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, 0, nullptr, 0, &vertexShaderBlob, &errorMsgBlob)))
		{
			dseed::diagnostics::logger::shared_logger ()->write (dseed::diagnostics::loglevel::error, "Sprite Render Initialize",
				(const char*)errorMsgBlob.Get ()->GetBufferPointer ());

			return dseed::error_fail;
		}

		if (FAILED (nativeObject.d3dDevice->CreateVertexShader (
			vertexShaderBlob.Get ()->GetBufferPointer (), vertexShaderBlob.Get ()->GetBufferSize (),
			nullptr, &vertexShader)))
			return dseed::error_fail;

		const char pixelShaderCode [] = CODE (
			struct PIXELSHADER_IN
			{
				float4 position : SV_Position;
				float2 uv : TEXCOORD0;
				float4 color : COLOR;
				uint textureIndex : TEXCOORD1;
				uint instanceId : TEXCOORD1;
			};

			SamplerState samplerState;
			Texture2DArray textures;

			float4 main (PIXELSHADER_IN input) : SV_Target
			{
				return textures.Sample (samplerState, float3(input.uv, input.textureIndex)) * input.color;
			}
		);

		if (FAILED (D3DCompile2 (pixelShaderCode, _countof (pixelShaderCode), nullptr, nullptr, nullptr,
			"main", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, 0, nullptr, 0, &pixelShaderBlob, &errorMsgBlob)))
		{
			dseed::diagnostics::logger::shared_logger ()->write (dseed::diagnostics::loglevel::error, "Sprite Render Initialize",
				(const char*)errorMsgBlob.Get ()->GetBufferPointer ());

			return dseed::error_fail;
		}

		if (FAILED (nativeObject.d3dDevice->CreatePixelShader (
			pixelShaderBlob.Get ()->GetBufferPointer (), pixelShaderBlob.Get ()->GetBufferSize (),
			nullptr, &defaultPixelShader)))
			return dseed::error_fail;

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.StencilEnable = false;
		
		if (FAILED (nativeObject.d3dDevice->CreateDepthStencilState (&depthStencilDesc, &depthStencilState)))
			return dseed::error_fail;

		D3D11_BUFFER_DESC transformConstantBufferDesc = {};
		transformConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		transformConstantBufferDesc.StructureByteStride = 0;//sizeof (SPRITE_TRANSFORM);
		transformConstantBufferDesc.ByteWidth = sizeof (SPRITE_TRANSFORM);
		transformConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED (nativeObject.d3dDevice->CreateBuffer (&transformConstantBufferDesc, nullptr, &transformConstantBuffer)))
			return dseed::error_fail;

		D3D11_BUFFER_DESC instanceConstantBufferDesc = {};
		instanceConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		instanceConstantBufferDesc.StructureByteStride = 0;//sizeof (SPRITE_TRANSFORM);
		instanceConstantBufferDesc.ByteWidth = sizeof (SPRITE_INSTANCE_DATA);
		instanceConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED (nativeObject.d3dDevice->CreateBuffer (&instanceConstantBufferDesc, nullptr, &instanceConstantBuffer)))
			return dseed::error_fail;

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = 1;
		
		if (FAILED (nativeObject.d3dDevice->CreateSamplerState (&samplerDesc, &samplerState)))
			return dseed::error_fail;

		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		
		if (FAILED (nativeObject.d3dDevice->CreateBlendState (&blendDesc, &defaultBlendState)))
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
	virtual dseed::error_t begin (dseed::graphics::rendermethod method, const dseed::f32x4x4& transform, dseed::graphics::sprite_rendertarget* rendertarget,
		dseed::graphics::pipeline* pipeline, dseed::graphics::vgabuffer** constbufs, size_t constbufcount) noexcept
	{
		if (session) return dseed::error_invalid_op;
		if (pipeline == nullptr)
			return dseed::error_invalid_args;

		session = true;
		renderMethod = method;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
		nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

		immediateContext->ClearState ();

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			this->pipeline = pipeline;
			renderTarget = rendertarget;
			for (size_t i = 0; i < constbufcount; ++i)
				constantBuffers.push_back (constbufs[i]);
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			reinterpret_cast<__d3d11_sprite_rendertarget*>(rendertarget)->apply_rendertarget (immediateContext.Get ());
			reinterpret_cast<__d3d11_sprite_pipeline*>(pipeline)->apply_pipeline (immediateContext.Get ());

			immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			immediateContext->VSSetShader (vertexShader.Get (), nullptr, 0);
			ID3D11Buffer* vsConstantBuffers[] = { transformConstantBuffer.Get (), instanceConstantBuffer.Get () };
			immediateContext->VSSetConstantBuffers (0, _countof (vsConstantBuffers), vsConstantBuffers);
			immediateContext->OMSetDepthStencilState (depthStencilState.Get (), 0);
			ID3D11SamplerState* sampler = samplerState.Get ();
			immediateContext->PSSetSamplers (0, 1, &sampler);

			std::vector<ID3D11Buffer*> constantBuffers;
			for (auto& cb : this->constantBuffers)
			{
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11Buffer;
				cb->native_object (&d3d11Buffer);
				constantBuffers.push_back (d3d11Buffer.Get ());
			}
			immediateContext->PSSetConstantBuffers (0, (UINT)constantBuffers.size (), constantBuffers.data ());
		}

		dseed::size2i size;
		if (rendertarget != nullptr)
			size = rendertarget->size ();
		else
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			nativeObject.dxgiSwapChain->GetDesc (&swapChainDesc);
			size = dseed::size2i (swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height);
		}

		transformBuffer.worldTransform = transform;
		transformBuffer.projectionTransform = dseed::float4x4::orthographic_offcenter (0, (float)size.width, (float)size.height, 0, 0.00001f, 32767.0f);

		immediateContext->UpdateSubresource (transformConstantBuffer.Get (), 0, nullptr, &transformBuffer, sizeof (SPRITE_TRANSFORM), 0);

		return dseed::error_good;
	}
	virtual dseed::error_t end () noexcept
	{
		if (!session)
			return dseed::error_invalid_op;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
		nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			reinterpret_cast<__d3d11_sprite_rendertarget*>(renderTarget.get ())->apply_rendertarget (immediateContext.Get ());
			reinterpret_cast<__d3d11_sprite_pipeline*>(pipeline.get ())->apply_pipeline (immediateContext.Get ());

			std::vector<ID3D11Buffer*> constantBuffers;
			for (auto& cb : this->constantBuffers)
			{
				Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11Buffer;
				cb->native_object (&d3d11Buffer);
				constantBuffers.push_back (d3d11Buffer.Get ());
			}
			immediateContext->PSSetConstantBuffers (0, (UINT)constantBuffers.size (), constantBuffers.data ());

			immediateContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			immediateContext->VSSetShader (vertexShader.Get (), nullptr, 0);
			ID3D11Buffer* vsConstantBuffers[] = { transformConstantBuffer.Get (), instanceConstantBuffer.Get () };
			immediateContext->VSSetConstantBuffers (0, _countof (vsConstantBuffers), vsConstantBuffers);
			immediateContext->OMSetDepthStencilState (depthStencilState.Get (), 0);
			ID3D11SamplerState* sampler = samplerState.Get ();
			immediateContext->PSSetSamplers (0, 1, &sampler);
		}

		sessionAtlases.clear ();
		currentAtlas = 0;

		records.clear ();
		pipeline.release ();
		renderTarget.release ();
		constantBuffers.clear ();

		session = false;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t create_pipeline (dseed::graphics::shaderpack* pixelshader, const dseed::graphics::blendparams* blendparams, dseed::graphics::pipeline** pipeline) noexcept
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

				Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob, errorMsgBlob;

				if (FAILED (D3DCompile2 (pixelShaderCodeBlob->ptr (), pixelShaderCodeBlob->size (), nullptr, nullptr, nullptr,
					"main", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, 0, nullptr, 0, &pixelShaderBlob, &errorMsgBlob)))
				{
					dseed::diagnostics::logger::shared_logger ()->write (dseed::diagnostics::loglevel::error, "Sprite Render Initialize",
						(const char*)errorMsgBlob.Get ()->GetBufferPointer ());

					return dseed::error_fail;
				}

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
			D3D11_BLEND_DESC blendDesc = {};
			blendDesc.RenderTarget[0].BlendEnable = blendparams->blend_enable;
			blendDesc.RenderTarget[0].BlendOp = DSEEDBLENDOP_TO_D3D11BLENDOP (blendparams->blend_op);
			blendDesc.RenderTarget[0].SrcBlend = DSEEDBLENDVALUE_TO_D3D11_BLENDVALUE (blendparams->src);
			blendDesc.RenderTarget[0].DestBlend = DSEEDBLENDVALUE_TO_D3D11_BLENDVALUE (blendparams->dest);
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			if (FAILED (nativeObject.d3dDevice->CreateBlendState (&blendDesc, &blendState)))
				return dseed::error_invalid_args;
		}
		else
			blendState = defaultBlendState;

		*pipeline = new __d3d11_sprite_pipeline (pixelShader.Get (), blendState.Get ());
		if (*pipeline == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_atlas (dseed::bitmaps::bitmap* bitmap, const dseed::rect2i* atlases, size_t atlas_count, dseed::graphics::sprite_atlas** atlas) noexcept
	{
		if (bitmap == nullptr || atlas == nullptr)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		D3D11_TEXTURE2D_DESC textureDesc = { };
		textureDesc.Width = bitmap->size ().width;
		textureDesc.Height = bitmap->size ().height;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DSEEDPF_TO_DXGIPF (bitmap->format ());
		textureDesc.MipLevels = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;

		void* ptr;
		bitmap->lock (&ptr);

		D3D11_SUBRESOURCE_DATA subResourceData = { };
		subResourceData.pSysMem = ptr;
		subResourceData.SysMemPitch = (UINT)dseed::color::calc_bitmap_stride (bitmap->format (), textureDesc.Width);
		subResourceData.SysMemSlicePitch = subResourceData.SysMemPitch * textureDesc.Height;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		if (FAILED (nativeObject.d3dDevice->CreateTexture2D (&textureDesc, &subResourceData, &texture)))
		{
			bitmap->unlock ();
			return dseed::error_fail;
		}

		bitmap->unlock ();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
		if (FAILED (nativeObject.d3dDevice->CreateShaderResourceView (texture.Get (), nullptr, &srv)))
			return dseed::error_fail;

		dseed::rect2i defaultAtlas (0, 0, textureDesc.Width, textureDesc.Height);
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
	virtual dseed::error_t create_rendertarget (const dseed::size2i& size, dseed::color::pixelformat format, dseed::graphics::sprite_rendertarget** target) noexcept
	{
		if (target == nullptr)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		D3D11_TEXTURE2D_DESC textureDesc = { };
		textureDesc.Width = size.width;
		textureDesc.Height = size.height;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DSEEDPF_TO_DXGIPF (format);
		textureDesc.MipLevels = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED (nativeObject.d3dDevice->CreateTexture2D (&textureDesc, nullptr, &backBuffer)))
			return dseed::error_fail;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> backBufferSRV;
		if (FAILED (nativeObject.d3dDevice->CreateShaderResourceView (backBuffer.Get (), nullptr, &backBufferSRV)))
			return dseed::error_fail;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
		if (FAILED (nativeObject.d3dDevice->CreateRenderTargetView (backBuffer.Get (), nullptr, &rtv)))
			return dseed::error_fail;

		D3D11_TEXTURE2D_DESC dsDesc = { };
		dsDesc.Width = size.width;
		dsDesc.Height = size.height;
		dsDesc.ArraySize = 1;
		dsDesc.Format = DXGI_FORMAT_R16_TYPELESS;
		dsDesc.MipLevels = 1;
		dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.Usage = D3D11_USAGE_DEFAULT;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
		if (FAILED (nativeObject.d3dDevice->CreateTexture2D (&dsDesc, nullptr, &depthStencilBuffer)))
			return dseed::error_fail;

		D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilBufferSRVDesc = {};
		depthStencilBufferSRVDesc.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilBufferSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilBufferSRV;
		if (FAILED (nativeObject.d3dDevice->CreateShaderResourceView (depthStencilBuffer.Get (), nullptr, &depthStencilBufferSRV)))
			return dseed::error_fail;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D16_UNORM;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
		if (FAILED (nativeObject.d3dDevice->CreateDepthStencilView (depthStencilBuffer.Get (), &dsvDesc, &dsv)))
			return dseed::error_fail;

		dseed::rect2i atlasSize (0, 0, size.width, size.height);

		dseed::autoref<dseed::graphics::sprite_atlas> atlas, dsAtlas;
		*&atlas = new __d3d11_sprite_atlas (backBuffer.Get (), backBufferSRV.Get (), &atlasSize, 1);
		*&dsAtlas = new __d3d11_sprite_atlas (depthStencilBuffer.Get (), depthStencilBufferSRV.Get (), &atlasSize, 1);

		*target = new __d3d11_sprite_rendertarget (atlas, rtv.Get (), dsAtlas, dsv.Get ());
		if (*target == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}
	virtual dseed::error_t create_constant (size_t size, dseed::graphics::vgabuffer** constbuf, const void* dat = nullptr) noexcept
	{
		if (constbuf == nullptr || size == 0)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.StructureByteStride = 0;//sizeof (SPRITE_TRANSFORM);
		bufferDesc.ByteWidth = (UINT)size;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pSysMem = dat;
		subResourceData.SysMemPitch = (UINT)size;

		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
		if (FAILED (nativeObject.d3dDevice->CreateBuffer (&bufferDesc, dat != nullptr ? &subResourceData : nullptr, &constantBuffer)))
			return dseed::error_fail;

		*constbuf = new __d3d11_sprite_vgabuffer (constantBuffer.Get ());
		if (*constbuf == nullptr)
			return dseed::error_out_of_memory;

		return dseed::error_good;
	}

public:
	virtual dseed::error_t set_atlas (dseed::graphics::sprite_atlas* atlas) noexcept
	{
		if (!session)
			return dseed::error_invalid_args;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			if (atlas == nullptr)
				return dseed::error_invalid_args;

			for (size_t i = 0; i < sessionAtlases.size (); ++i)
			{
				if (sessionAtlases[i] == atlas)
				{
					currentAtlas = i;
					return dseed::error_good;
				}
			}

			if (sessionAtlases.size () > 128)
				return dseed::error_invalid_op;

			sessionAtlases.push_back (atlas);
			currentAtlas = sessionAtlases.size () - 1;
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
			vgadevice->native_object ((void**)&nativeObject);

			Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
			nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
			reinterpret_cast<__d3d11_sprite_atlas*>(atlas)->get_srv (&srv);
			immediateContext->PSSetShaderResources (0, 1, &srv);

			if (sessionAtlases.size () == 0)
				sessionAtlases.push_back (nullptr);
			sessionAtlases[0] = atlas;
		}

		return dseed::error_good;
	}
	virtual dseed::error_t set_constant (dseed::graphics::vgabuffer* constbuf, size_t i) noexcept
	{
		if (!session || renderMethod != dseed::graphics::rendermethod::forward)
			return  dseed::error_invalid_op;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
		nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

		ID3D11Buffer* cb;
		constbuf->native_object ((void**)&cb);

		immediateContext->PSSetConstantBuffers ((UINT)i, 1, &cb);

		cb->Release ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t update_constant (dseed::graphics::vgabuffer* constbuf, const void* buf, size_t offset, size_t length) noexcept
	{
		if (session && renderMethod == dseed::graphics::rendermethod::deferred)
			return dseed::error_invalid_op;
		if (constbuf == nullptr || buf == nullptr || length == 0)
			return dseed::error_invalid_args;

		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		constbuf->native_object (&buffer);

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
		nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

		D3D11_BOX destBox = {};
		destBox.left = (UINT)offset;
		destBox.right = (UINT)(offset + length);
		immediateContext->UpdateSubresource (buffer.Get (), 0, &destBox, buf, (UINT)length, 0);

		return dseed::error_good;
	}

public:
	virtual dseed::error_t draw (size_t atlas_index, const dseed::f32x4x4& transform, const dseed::f32x4& color) noexcept
	{
		if (!session)
			return dseed::error_invalid_args;

		if (atlas_index <= sessionAtlases[currentAtlas]->atlas_count ())
			return dseed::error_out_of_range;
		const dseed::rect2i atlas = sessionAtlases[currentAtlas]->atlas_element (atlas_index);
		auto size = sessionAtlases[currentAtlas]->size ();

		SPRITE_INSTANCE_DATA_RECORD record;
		record.area = dseed::f32x4 (
			atlas.x / (float)size.width,
			atlas.y / (float)size.height,
			atlas.width / (float)size.width,
			atlas.height / (float)size.width);
		record.transform = transform;
		record.color = color;
		record.textureIndex = (uint32_t)currentAtlas;
		record.width = atlas.width;
		record.height = atlas.height;

		if (renderMethod == dseed::graphics::rendermethod::deferred)
		{
			records.push_back (record);
		}
		else if (renderMethod == dseed::graphics::rendermethod::forward)
		{
			dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
			vgadevice->native_object ((void**)&nativeObject);

			Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
			nativeObject.d3dDevice->GetImmediateContext (&immediateContext);

			immediateContext->UpdateSubresource (instanceConstantBuffer.Get (), 0, nullptr, &record, sizeof (SPRITE_INSTANCE_DATA_RECORD), 0);
			immediateContext->DrawInstanced (4, 1, 0, 0);
		}

		return dseed::error_good;
	}

public:
	dseed::error_t update_backbuffer ()
	{
		dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
		vgadevice->native_object ((void**)&nativeObject);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		nativeObject.dxgiSwapChain->GetDesc (&swapChainDesc);
		dseed::size2i size = dseed::size2i (swapChainDesc.BufferDesc.Width, swapChainDesc.BufferDesc.Height);


	}

private:
	std::atomic<int32_t> _refCount;
	dseed::graphics::vgadevice* vgadevice;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> transformConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> instanceConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> defaultPixelShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState> defaultBlendState;

	SPRITE_TRANSFORM transformBuffer;

	bool session;
	dseed::graphics::rendermethod renderMethod;

	std::vector<dseed::autoref<dseed::graphics::sprite_atlas>> sessionAtlases;
	size_t currentAtlas;

	std::vector<SPRITE_INSTANCE_DATA_RECORD> records;
	dseed::autoref<dseed::graphics::pipeline> pipeline;
	dseed::autoref<dseed::graphics::sprite_rendertarget> renderTarget;
	std::vector<dseed::autoref<dseed::graphics::vgabuffer>> constantBuffers;
};

////////////////////////////////////////////////////////////////////////////////////////////
//
// VGA Logical Device
//
////////////////////////////////////////////////////////////////////////////////////////////
class __d3d11_vgadevice : public dseed::graphics::vgadevice
{
public:
	__d3d11_vgadevice (dseed::graphics::vgaadapter* adapter, ID3D11Device* d3dDevice, ID3D11DeviceContext* immediateContext, IDXGISwapChain* dxgiSwapChain)
		: _refCount (1), vgaadapter (adapter), d3dDevice (d3dDevice), immediateContext (immediateContext), dxgiSwapChain (dxgiSwapChain), vsync_enable (false)
	{

	}

	dseed::error_t initialize ()
	{
		*&spriterender = new __d3d11_sprite_render (this);
		if (dseed::failed (spriterender->initialize ()))
		{
			spriterender.release ();
			return dseed::error_fail;
		}

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
		if (nativeObject == nullptr)
			return dseed::error_invalid_args;

		auto& no = *reinterpret_cast<dseed::graphics::d3d11_vgadevice_nativeobject*>(nativeObject);
		no.d3dDevice = d3dDevice.Get ();
		no.dxgiSwapChain = dxgiSwapChain.Get ();

		return dseed::error_good;
	}

public:
	virtual dseed::error_t adapter (dseed::graphics::vgaadapter** adapter) noexcept
	{
		if (adapter == nullptr)
			return dseed::error_invalid_args;
		(*adapter = vgaadapter)->retain ();
		return dseed::error_good;
	}

public:
	virtual bool is_support_format (dseed::color::pixelformat pf) noexcept
	{
		UINT support;
		d3dDevice->CheckFormatSupport (DSEEDPF_TO_DXGIPF (pf), &support);
		return (support & D3D11_FORMAT_SUPPORT_TEXTURE2D) ? true : false;
	}

public:
	virtual dseed::error_t sprite_render (dseed::graphics::vgarender** render) noexcept
	{
		if (render == nullptr)
			return dseed::error_invalid_args;
		(*render = spriterender)->retain ();
		return dseed::error_good;
	}

public:
	virtual bool vsync () noexcept { return vsync_enable; }
	virtual dseed::error_t set_vsync (bool vsync) noexcept { vsync_enable = vsync; return dseed::error_good; }

public:
	virtual void displaymode (dseed::graphics::displaymode* dm, bool* fullscreen) noexcept
	{
		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;
		if (SUCCEEDED (dxgiSwapChain.As<IDXGISwapChain1> (&dxgiSwapChain1)))
		{
			DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullscreenDesc;

			dxgiSwapChain1->GetDesc1 (&dxgiSwapChainDesc);
			dxgiSwapChain1->GetFullscreenDesc (&dxgiSwapChainFullscreenDesc);

			if (dm != nullptr)
			{
				dm->resolution = dseed::size2i (dxgiSwapChainDesc.Width, dxgiSwapChainDesc.Height);
				dm->format = DXGIPF_TO_DSEEDPF (dxgiSwapChainDesc.Format);
				dm->refresh_rate = dseed::fraction (
					dxgiSwapChainFullscreenDesc.RefreshRate.Numerator,
					dxgiSwapChainFullscreenDesc.RefreshRate.Denominator);
			}
			if (fullscreen != nullptr)
				*fullscreen = !dxgiSwapChainFullscreenDesc.Windowed;
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
			dxgiSwapChain->GetDesc (&dxgiSwapChainDesc);

			if (dm != nullptr)
			{
				dm->resolution = dseed::size2i (dxgiSwapChainDesc.BufferDesc.Width, dxgiSwapChainDesc.BufferDesc.Height);
				dm->format = DXGIPF_TO_DSEEDPF (dxgiSwapChainDesc.BufferDesc.Format);
				dm->refresh_rate = dseed::fraction (
					dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator,
					dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator);
			}
			if (fullscreen != nullptr)
				*fullscreen = !dxgiSwapChainDesc.Windowed;
		}
	}
	virtual bool set_displaymode (const dseed::graphics::displaymode* dm, bool fullscreen) noexcept
	{
		if (dm == nullptr) return false;
		if (fullscreen)
		{
#if PLATFORM_UWP
			return false;
#endif
		}

		DXGI_MODE_DESC dxgiModeDesc = {};
		dxgiModeDesc.Width = dm->resolution.width;
		dxgiModeDesc.Height = dm->resolution.height;
		dxgiModeDesc.Format = DSEEDPF_TO_DXGIPF (dm->format);
		dxgiModeDesc.RefreshRate.Denominator = dm->refresh_rate.denominator;
		dxgiModeDesc.RefreshRate.Numerator = dm->refresh_rate.numerator;

		if (fullscreen)
		{
			dxgiModeDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
			dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
		}

		if (FAILED (dxgiSwapChain->ResizeTarget (&dxgiModeDesc)) ||
			FAILED (dxgiSwapChain->SetFullscreenState (fullscreen, nullptr)))
			return false;

		return true;
	}

public:
	virtual dseed::error_t present () noexcept
	{
		HRESULT hr = dxgiSwapChain->Present ((int)vsync_enable, 0);

		if (hr == DXGI_ERROR_DEVICE_RESET)
			return dseed::error_device_reset;
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
			return dseed::error_device_disconn;
		return dseed::error_good;
	}

private:
	std::atomic<int32_t> _refCount;
	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> dxgiSwapChain;

	dseed::autoref<dseed::graphics::vgaadapter> vgaadapter;

	dseed::autoref<__d3d11_sprite_render> spriterender;

	bool vsync_enable;
};

#endif

////////////////////////////////////////////////////////////////////////////////////////////
//
// Create VGA Logical Device
//
////////////////////////////////////////////////////////////////////////////////////////////
dseed::error_t dseed::graphics::create_d3d11_vgadevice (platform::application* app, vgaadapter* adapter, vgadevice** device) noexcept
{
#if PLATFORM_MICROSOFT
	if (app == nullptr || device == nullptr)
		return dseed::error_invalid_args;

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	D3D_DRIVER_TYPE d3dDriverType;
	if (adapter != nullptr)
	{
		adapter->native_object (&dxgiAdapter);
		d3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
	}
	else
	{
		dxgiAdapter = nullptr;
		d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	}

	DWORD d3dDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3dDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	if (FAILED (D3D11CreateDevice (dxgiAdapter.Get (), d3dDriverType, nullptr, d3dDeviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &d3dDevice, nullptr, &immediateContext)))
		return dseed::error_fail;

	Microsoft::WRL::ComPtr<IDXGISwapChain> dxgiSwapChain;
#if PLATFORM_WINDOWS
	HWND hWnd;
	app->native_object ((void**)&hWnd);
#elif PLATFORM_UWP
	CoreWindow^ coreWindow;
	app->native_object ((void**)&coreWindow);
#endif
	dseed::size2i clientSize;
	app->client_size (&clientSize);

#if NTDDI_VERSION >= NTDDI_WIN8
	if (IsWindows8OrGreater ())
	{
		Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
		if (FAILED (CreateDXGIFactory2 (0, __uuidof(IDXGIFactory2), (void**)&dxgiFactory)))
			return dseed::error_fail;

		DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc = {};
#	if NTDDI_VERSION >= NTDDI_WIN10
		if (IsWindows10OrGreater ())
		{
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			dxgiSwapChainDesc.BufferCount = 2;
		}
		else
#	endif
		{
#	if PLATFORM_WINDOWS
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			dxgiSwapChainDesc.BufferCount = 1;
#	elif PLATFORM_UWP
			dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			dxgiSwapChainDesc.BufferCount = 2;
#	endif
		}
		dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.Width = clientSize.width;
		dxgiSwapChainDesc.Height = clientSize.height;
		dxgiSwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		dxgiSwapChainDesc.SampleDesc.Count = 1;
		dxgiSwapChainDesc.Stereo = FALSE;
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#	if NTDDI_VERSION >= NTDDI_WIN8
		dxgiSwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
#		if NTDDI_VERSION >= NTDDI_WIN10
		Microsoft::WRL::ComPtr<IDXGIFactory5> dxgiFactory5;
		dxgiFactory.As<IDXGIFactory5> (&dxgiFactory5);

		BOOL checkAllowTearing;
		if (SUCCEEDED (dxgiFactory5->CheckFeatureSupport (DXGI_FEATURE_PRESENT_ALLOW_TEARING, &checkAllowTearing, sizeof (checkAllowTearing))) && checkAllowTearing)
			dxgiSwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
#		endif
#	endif

		Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain1;
#	if PLATFORM_WINDOWS
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullscreenDesc = {};
		dxgiSwapChainFullscreenDesc.Windowed = TRUE;

		if (FAILED (dxgiFactory->CreateSwapChainForHwnd (d3dDevice.Get (), hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullscreenDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	elif PLATFORM_UWP
		if (FAILED (dxgiFactory->CreateSwapChainForCoreWindow (d3dDevice.Get (), coreWindow, &dxgiSwapChainDesc, nullptr, &dxgiSwapChain1)))
			return dseed::error_fail;
#	endif

		dxgiSwapChain1.As<IDXGISwapChain> (&dxgiSwapChain);
	}
#	if PLATFORM_WINDOWS
	else
#	endif
#endif
#if PLATFORM_WINDOWS
	{
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
		if (FAILED (CreateDXGIFactory (__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
			return dseed::error_fail;

		DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc = {};
		dxgiSwapChainDesc.BufferDesc.Width = clientSize.width;
		dxgiSwapChainDesc.BufferDesc.Height = clientSize.height;
		dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc.BufferCount = 1;
		dxgiSwapChainDesc.OutputWindow = hWnd;
		dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		dxgiSwapChainDesc.SampleDesc.Count = 1;
		dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		dxgiSwapChainDesc.Windowed = true;

		if (FAILED (dxgiFactory->CreateSwapChain (d3dDevice.Get (), &dxgiSwapChainDesc, &dxgiSwapChain)))
			return dseed::error_fail;
	}
#endif

	*device = new __d3d11_vgadevice (adapter, d3dDevice.Get (), immediateContext.Get (), dxgiSwapChain.Get ());
	if (*device == nullptr)
		return  dseed::error_out_of_memory;
	
	if (dseed::failed (reinterpret_cast<__d3d11_vgadevice*>(*device)->initialize ()))
	{
		delete* device;
		*device = nullptr;
		return dseed::error_fail;
	}

#else
	return dseed::error_not_support;
#endif
	return dseed::error_good;
}