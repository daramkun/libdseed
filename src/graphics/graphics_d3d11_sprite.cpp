#define USE_D3D11_NATIVE_OBJECT
#include <dseed.h>

#if PLATFORM_MICROSOFT

#	include "common_d3d11.hxx"
#	include "d3d11_sprite.hxx"

#	include <queue>

#	define SPRITE_INSTANCE_COUNT							682//(64 * 1024) / sizeof (SPRITE_INSTANCE_DATA)
#	define SPRITE_INSTANCE_TOTAL_SIZE						SPRITE_INSTANCE_COUNT * sizeof (SPRITE_INSTANCE_DATA)

constexpr char vertexShaderCode[] = CODE(
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
	SPRITE_INSTANCE_DATA_RECORD records[682];
};

VERTEXSHADER_OUT main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
	VERTEXSHADER_OUT output;

	switch (vertexId)
	{
	case 0: // Left-Top
		output.position = float4(-0.5f, -0.5f, 0, 1);
		output.uv = float2(records[instanceId].atlas_area.x, records[instanceId].atlas_area.y);
		break;

	case 1: // Left-Bottom
		output.position = float4(-0.5f, +0.5f, 0, 1);
		output.uv = float2(records[instanceId].atlas_area.x, records[instanceId].atlas_area.w);
		break;

	case 2: // Right-Top
		output.position = float4(+0.5f, -0.5f, 0, 1);
		output.uv = float2(records[instanceId].atlas_area.z, records[instanceId].atlas_area.y);
		break;

	case 3: // Right-Bottom
		output.position = float4(+0.5f, +0.5f, 0, 1);
		output.uv = float2(records[instanceId].atlas_area.z, records[instanceId].atlas_area.w);
		break;
	}

	output.position = mul(mul(mul(output.position, records[instanceId].transform), worldTransform), projectionTransform);
	output.color = records[instanceId].color;

	return output;
}
);

constexpr char pixelShaderCode[] = CODE(
	struct PIXELSHADER_IN
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD0;
	float4 color : COLOR;
};

SamplerState samplerState;
Texture2D inputTexture : register (t0);

float4 main(PIXELSHADER_IN input) : SV_Target
{
	return inputTexture.Sample(samplerState, input.uv) * input.color;
}
);

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Contant Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////
__d3d11_sprite_vgabuffer::__d3d11_sprite_vgabuffer(ID3D11Buffer* buffer) : _refCount(1), buffer(buffer) { }

int32_t __d3d11_sprite_vgabuffer::retain() { return ++_refCount; }
int32_t __d3d11_sprite_vgabuffer::release()
{
	auto ret = --_refCount;
	if (ret == 0)
		delete this;
	return ret;
}

dseed::error_t __d3d11_sprite_vgabuffer::native_object(void** nativeObject)
{
	if (nativeObject == nullptr)
		return dseed::error_invalid_args;

	auto no = reinterpret_cast<dseed::graphics::d3d11_sprite_vgabuffer_nativeobject*>(nativeObject);
	buffer.As(&no->buffer);

	return dseed::error_good;
}

size_t __d3d11_sprite_vgabuffer::stride() noexcept
{
	D3D11_BUFFER_DESC bufferDesc;
	buffer.Get()->GetDesc(&bufferDesc);
	return bufferDesc.ByteWidth;
}
size_t __d3d11_sprite_vgabuffer::length() noexcept { return 1; }
dseed::graphics::vgabuffertype __d3d11_sprite_vgabuffer::type() noexcept { return dseed::graphics::vgabuffertype::constant; }

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Pipeline
//
////////////////////////////////////////////////////////////////////////////////////////////
__d3d11_sprite_pipeline::__d3d11_sprite_pipeline(ID3D11PixelShader* pixelShader, ID3D11BlendState* blendState, ID3D11SamplerState* samplerState)
	: _refCount(1), pixelShader(pixelShader), blendState(blendState), samplerState(samplerState)
{

}

int32_t __d3d11_sprite_pipeline::retain() { return ++_refCount; }
int32_t __d3d11_sprite_pipeline::release()
{
	auto ret = --_refCount;
	if (ret == 0)
		delete this;
	return ret;
}

dseed::error_t __d3d11_sprite_pipeline::native_object(void** nativeObject)
{
	if (nativeObject == nullptr)
		return dseed::error_invalid_args;

	auto* no = reinterpret_cast<dseed::graphics::d3d11_sprite_pipeline_nativeobject*>(nativeObject);
	pixelShader.As(&no->pixelShader);
	blendState.As(&no->blendState);
	samplerState.As(&no->samplerState);

	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Atlas
//
////////////////////////////////////////////////////////////////////////////////////////////
__d3d11_sprite_atlas::__d3d11_sprite_atlas(ID3D11Resource* texture, ID3D11ShaderResourceView* srv, const dseed::rect2i* atlases, size_t atlas_count)
	: _refCount(1), texture(texture), srv(srv)
{
	for (size_t i = 0; i < atlas_count; ++i)
	{
		const auto& atlas = atlases[i];
		this->atlases.emplace_back(atlas);
	}
}

int32_t __d3d11_sprite_atlas::retain() { return ++_refCount; }
int32_t __d3d11_sprite_atlas::release()
{
	const auto ret = --_refCount;
	if (ret == 0)
		delete this;
	return ret;
}

dseed::error_t __d3d11_sprite_atlas::native_object(void** nativeObject)
{
	if (nativeObject == nullptr)
		return dseed::error_invalid_args;

	auto* no = reinterpret_cast<dseed::graphics::d3d11_sprite_atlas_nativeobject*>(nativeObject);
	texture.As(&no->texture);
	srv.As(&no->shaderResourceView);

	return dseed::error_good;
}

dseed::size2i __d3d11_sprite_atlas::size() noexcept
{
	Microsoft::WRL::ComPtr<ID3D11Texture1D> texture1d;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
	if (SUCCEEDED(texture.As(&texture2d)))
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2d->GetDesc(&desc);
		return dseed::size2i(desc.Width, desc.Height);
	}
	else if (SUCCEEDED(texture.As(&texture3d)))
	{
		D3D11_TEXTURE3D_DESC desc;
		texture3d->GetDesc(&desc);
		return dseed::size2i(desc.Width, desc.Height);
	}
	else if (SUCCEEDED(texture.As(&texture2d)))
	{
		D3D11_TEXTURE1D_DESC desc;
		texture1d->GetDesc(&desc);
		return dseed::size2i(desc.Width, desc.ArraySize);
	}
	return dseed::size2i(-1, -1);
}
dseed::color::pixelformat __d3d11_sprite_atlas::format() noexcept
{
	Microsoft::WRL::ComPtr<ID3D11Texture1D> texture1d;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
	if (SUCCEEDED(texture.As(&texture2d)))
	{
		D3D11_TEXTURE2D_DESC desc;
		texture2d->GetDesc(&desc);
		return DXGIPF_TO_DSEEDPF(desc.Format);
	}
	else if (SUCCEEDED(texture.As(&texture3d)))
	{
		D3D11_TEXTURE3D_DESC desc;
		texture3d->GetDesc(&desc);
		return DXGIPF_TO_DSEEDPF(desc.Format);
	}
	else if (SUCCEEDED(texture.As(&texture2d)))
	{
		D3D11_TEXTURE1D_DESC desc;
		texture1d->GetDesc(&desc);
		return DXGIPF_TO_DSEEDPF(desc.Format);
	}
	return dseed::color::pixelformat::unknown;
}

dseed::rect2i __d3d11_sprite_atlas::atlas_element(size_t index) noexcept
{
	if (index >= atlases.size())
		dseed::rect2i(-1, -1, -1, -1);

	return atlases.at(index);
}
size_t __d3d11_sprite_atlas::atlas_count() noexcept { return atlases.size(); }

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Render Target
//
////////////////////////////////////////////////////////////////////////////////////////////
__d3d11_sprite_rendertarget::__d3d11_sprite_rendertarget(dseed::graphics::sprite_atlas* atlas, ID3D11RenderTargetView* rtv)
	: _refCount(1), spriteAtlas(atlas), rtv(rtv)
{

}

int32_t __d3d11_sprite_rendertarget::retain() { return ++_refCount; }
int32_t __d3d11_sprite_rendertarget::release()
{
	const auto ret = --_refCount;
	if (ret == 0)
		delete this;
	return ret;
}

dseed::error_t __d3d11_sprite_rendertarget::native_object(void** nativeObject)
{
	if (nativeObject == nullptr)
		return dseed::error_invalid_args;

	auto* no = reinterpret_cast<dseed::graphics::d3d11_sprite_rendertarget_nativeobject*>(nativeObject);
	rtv.As(&no->renderTargetView);

	return dseed::error_good;
}

dseed::size2i __d3d11_sprite_rendertarget::size() noexcept { auto s = spriteAtlas->size(); return dseed::size2i(s.width, s.height); }
dseed::color::pixelformat __d3d11_sprite_rendertarget::format() noexcept { return spriteAtlas->format(); }

dseed::error_t __d3d11_sprite_rendertarget::atlas(dseed::graphics::sprite_atlas** atlas) noexcept
{
	if (atlas == nullptr) return dseed::error_invalid_args;
	(*atlas = spriteAtlas)->retain();
	return dseed::error_good;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Sprite Renderer
//
////////////////////////////////////////////////////////////////////////////////////////////
__d3d11_sprite_render::__d3d11_sprite_render(dseed::graphics::vgadevice* device)
	: _refCount(1), vgadevice(device), renderTargetSize(0, 0)
	, transformBuffer({ dseed::float4x4::identity(), dseed::float4x4::identity() })
	, session(false), renderMethod(dseed::graphics::rendermethod::deferred)
{

}
dseed::error_t __d3d11_sprite_render::initialize()
{
	dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
	vgadevice->native_object(reinterpret_cast<void**>(&nativeObject));
	d3dDevice = nativeObject.d3dDevice.Get();
	d3dDevice->GetImmediateContext(&immediateContext);

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, pixelShaderBlob;
	if (FAILED(CompileHLSL("vs_5_0", vertexShaderCode, &vertexShaderBlob)))
		return dseed::error_fail;

	if (FAILED(d3dDevice->CreateVertexShader(
		vertexShaderBlob.Get()->GetBufferPointer(), vertexShaderBlob.Get()->GetBufferSize(),
		nullptr, &vertexShader)))
		return dseed::error_fail;

	if (FAILED(CompileHLSL("ps_5_0", pixelShaderCode, &pixelShaderBlob)))
		return dseed::error_fail;

	if (FAILED(d3dDevice->CreatePixelShader(
		pixelShaderBlob.Get()->GetBufferPointer(), pixelShaderBlob.Get()->GetBufferSize(),
		nullptr, &defaultPixelShader)))
		return dseed::error_fail;

	if (FAILED(CreateConstantBuffer(d3dDevice.Get(), sizeof(SPRITE_TRANSFORM), &transformConstantBuffer, nullptr)))
		return dseed::error_fail;

	if (FAILED(CreateConstantBuffer(d3dDevice.Get(), sizeof(SPRITE_INSTANCE_DATA) * SPRITE_INSTANCE_COUNT, &instanceConstantBuffer, nullptr)))
		return dseed::error_fail;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MinLOD = -FLT_MIN;
	samplerDesc.MaxLOD = FLT_MAX;

	if (FAILED(d3dDevice->CreateSamplerState(&samplerDesc, &samplerState[0])))
		return dseed::error_fail;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	if (FAILED(d3dDevice->CreateSamplerState(&samplerDesc, &samplerState[1])))
		return dseed::error_fail;

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	if (FAILED(d3dDevice->CreateSamplerState(&samplerDesc, &samplerState[2])))
		return dseed::error_fail;

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;

	if (FAILED(d3dDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState)))
		return dseed::error_fail;

	if (dseed::failed(update_backbuffer()))
		return dseed::error_fail;

	return dseed::error_good;
}

int32_t __d3d11_sprite_render::retain() { return ++_refCount; }
int32_t __d3d11_sprite_render::release()
{
	const auto ret = --_refCount;
	if (ret == 0)
		delete this;
	return ret;
}

dseed::error_t __d3d11_sprite_render::native_object(void** nativeObject) { return dseed::error_not_impl; }

dseed::error_t __d3d11_sprite_render::create_pipeline(dseed::graphics::shaderpack* pixelshader, const dseed::graphics::blendparams* blendparams,
	dseed::graphics::sprite_texfilter texfilter, dseed::graphics::pipeline** pipeline) noexcept
{
	if (pipeline == nullptr) return dseed::error_invalid_args;

	dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
	vgadevice->native_object(reinterpret_cast<void**>(&nativeObject));

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	if (pixelshader != nullptr)
	{
		dseed::autoref<dseed::blob> pixelShaderCodeBlob;
		if (dseed::failed(pixelshader->at(dseed::graphics::shaderlang_dxil, &pixelShaderCodeBlob)))
		{
			if (dseed::failed(pixelshader->at(dseed::graphics::shaderlang_hlsl, &pixelShaderCodeBlob)))
				return dseed::error_invalid_args;

			Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
			if (FAILED(CompileHLSL("ps_5_0", static_cast<const char*>(pixelShaderCodeBlob->ptr()), &pixelShaderBlob)))
				return dseed::error_fail;

			if (FAILED(nativeObject.d3dDevice->CreatePixelShader(
				pixelShaderBlob.Get()->GetBufferPointer(), pixelShaderBlob.Get()->GetBufferSize(),
				nullptr, &defaultPixelShader)))
				return dseed::error_fail;
		}
		else
		{
			if (FAILED(nativeObject.d3dDevice->CreatePixelShader(
				pixelShaderCodeBlob->ptr(), pixelShaderCodeBlob->size(),
				nullptr, &defaultPixelShader)))
				return dseed::error_fail;
		}
	}
	else
		pixelShader = defaultPixelShader;

	if (blendparams != nullptr)
	{
		if (FAILED(CreateBlendStateFromBlendParams(nativeObject.d3dDevice.Get(), *blendparams, &blendState)))
			return dseed::error_invalid_args;
	}
	else
	{
		const auto alphablend = dseed::graphics::blendparams::alphablend();
		if (FAILED(CreateBlendStateFromBlendParams(nativeObject.d3dDevice.Get(), alphablend, &blendState)))
			return dseed::error_invalid_args;
	}

	*pipeline = new __d3d11_sprite_pipeline(pixelShader.Get(), blendState.Get(), samplerState[(int)texfilter].Get());
	if (*pipeline == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::create_atlas(dseed::bitmaps::bitmap* bitmap, const dseed::rect2i* atlases, size_t atlas_count,
	dseed::graphics::sprite_atlas** atlas) noexcept
{
	if (bitmap == nullptr || atlas == nullptr)
		return dseed::error_invalid_args;

	dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
	vgadevice->native_object(reinterpret_cast<void**>(&nativeObject));

	dseed::autoref<dseed::bitmaps::bitmap> convBitmap;
	if (vgadevice->is_support_format(bitmap->format()))
		convBitmap = bitmap;
	else
	{
		if (dseed::failed(dseed::bitmaps::reformat_bitmap(bitmap, dseed::color::pixelformat::rgba8, &convBitmap)))
			return dseed::error_fail;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	if (FAILED(CreateTextureFromBitmap(nativeObject.d3dDevice.Get(), convBitmap, reinterpret_cast<ID3D11Resource**>(texture.GetAddressOf()), &srv, nullptr)))
		return dseed::error_fail;

	dseed::rect2i defaultAtlas(0, 0, bitmap->size().width, bitmap->size().height);
	if (atlas_count == 0)
	{
		atlases = &defaultAtlas;
		atlas_count = 1;
	}

	*atlas = new __d3d11_sprite_atlas(texture.Get(), srv.Get(), atlases, atlas_count);
	if (*atlas == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::create_rendertarget(const dseed::size2i& size, dseed::color::pixelformat format,
	dseed::graphics::sprite_rendertarget** target) noexcept
{
	if (target == nullptr || DSEEDPF_TO_DXGIPF(format) == DXGI_FORMAT_UNKNOWN)
		return dseed::error_invalid_args;

	dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
	vgadevice->native_object(reinterpret_cast<void**>(&nativeObject));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> backBufferSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	if (FAILED(CreateTexture2D(nativeObject.d3dDevice.Get(), size, format, &backBuffer, &backBufferSRV, nullptr, &rtv)))
		return dseed::error_fail;

	dseed::rect2i atlasSize(0, 0, size.width, size.height);

	dseed::autoref<dseed::graphics::sprite_atlas> atlas;
	*&atlas = new __d3d11_sprite_atlas(backBuffer.Get(), backBufferSRV.Get(), &atlasSize, 1);
	if (atlas == nullptr)
		return dseed::error_out_of_memory;

	*target = new __d3d11_sprite_rendertarget(atlas, rtv.Get());
	if (*target == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::create_constant(size_t size, dseed::graphics::vgabuffer** constbuf, const void* dat) noexcept
{
	if (constbuf == nullptr || size == 0)
		return dseed::error_invalid_args;

	dseed::graphics::d3d11_vgadevice_nativeobject nativeObject;
	vgadevice->native_object(reinterpret_cast<void**>(&nativeObject));

	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	if (FAILED(CreateConstantBuffer(nativeObject.d3dDevice.Get(), (UINT)size, &constantBuffer, dat)))
		return dseed::error_fail;

	*constbuf = new __d3d11_sprite_vgabuffer(constantBuffer.Get());
	if (*constbuf == nullptr)
		return dseed::error_out_of_memory;

	return dseed::error_good;
}

inline SPRITE_RENDER_COMMAND* add_command(std::queue<SPRITE_RENDER_COMMAND*>& commands, dseed::memorypool<SPRITE_RENDER_COMMAND>& commandPool)
{
	SPRITE_RENDER_COMMAND* command;
	if (commands.empty())
	{
		command = commandPool.get_memory();
		commands.push(command);
	}
	else if (commands.back()->instances.empty())
	{
		command = commands.back();
	}
	else
	{
		auto* baseCommand = commands.back();
		command = commandPool.get_memory();
		for (auto& rt : baseCommand->renderTargets)
			command->renderTargets.emplace_back(rt);
		command->pipeline = baseCommand->pipeline;
		for (auto& a : baseCommand->atlases)
			command->atlases.emplace_back(a);
		for (auto& cb : baseCommand->constbufs)
			command->constbufs.emplace_back(cb);
		commands.push(command);
	}

	return command;
}

dseed::error_t __d3d11_sprite_render::set_pipeline(dseed::graphics::pipeline* pipeline) noexcept
{
	if (!session)
		return dseed::error_invalid_op;
	if (pipeline == nullptr)
		return dseed::error_invalid_args;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		auto* command = add_command(commands, commandPool);
		command->pipeline = pipeline;
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		commands.front()->pipeline = pipeline;

		dseed::graphics::d3d11_sprite_pipeline_nativeobject no;
		pipeline->native_object(reinterpret_cast<void**>(&no));

		immediateContext->PSSetShader(no.pixelShader.Get(), nullptr, 0);
		immediateContext->OMSetBlendState(no.blendState.Get(), nullptr, 0xffffffff);
		auto* samplerState = no.samplerState.Get();
		immediateContext->PSSetSamplers(0, 1, &samplerState);
	}

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::set_rendertarget(dseed::graphics::sprite_rendertarget** rendertargets, size_t size) noexcept
{
	if (!session)
		return dseed::error_invalid_op;
	if (rendertargets == nullptr || size == 0)
		return dseed::error_invalid_args;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		auto* command = add_command(commands, commandPool);
		command->renderTargets.clear();
		for (size_t i = 0; i < size; ++i)
			command->renderTargets.emplace_back(rendertargets[i]);
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		commands.front()->renderTargets.clear();

		std::vector<ID3D11RenderTargetView*> rtv;
		std::vector<D3D11_VIEWPORT> viewports;
		for (size_t i = 0; i < size; ++i)
		{
			commands.front()->renderTargets.emplace_back(rendertargets[i]);

			if (rendertargets[i] == nullptr)
			{
				rtv.emplace_back(renderTargetView.Get());

				D3D11_VIEWPORT viewport = {};
				viewport.Width = (float)renderTargetSize.width;
				viewport.Height = (float)renderTargetSize.height;
				viewport.MaxDepth = 1.0f;
				viewports.emplace_back(viewport);
			}
			else
			{
				dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
				rendertargets[i]->native_object((void**)&nativeObject);
				rtv.emplace_back(nativeObject.renderTargetView.Get());

				dseed::autoref<dseed::graphics::sprite_atlas> atlas;
				rendertargets[i]->atlas(&atlas);

				D3D11_VIEWPORT viewport = {};
				viewport.Width = (float)atlas->size().width;
				viewport.Height = (float)atlas->size().height;
				viewport.MaxDepth = 1.0f;
				viewports.emplace_back(viewport);
			}
		}
		immediateContext->OMSetRenderTargets((UINT)rtv.size(), rtv.data(), nullptr);
		immediateContext->RSSetViewports((UINT)viewports.size(), viewports.data());

		transformBuffer.projectionTransform =
			dseed::float4x4::orthographic_offcenter(0, static_cast<float>(viewports[0].Width), static_cast<float>(viewports[0].Height), 0, 0.00001f, 32767.0f);
		immediateContext->UpdateSubresource(transformConstantBuffer.Get(), 0, nullptr, &transformBuffer, sizeof(SPRITE_TRANSFORM), 0);
	}

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::set_atlas(dseed::graphics::sprite_atlas** atlas, size_t size) noexcept
{
	if (!session)
		return dseed::error_invalid_op;
	if (atlas == nullptr || size == 0)
		return dseed::error_invalid_args;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		SPRITE_RENDER_COMMAND* command = add_command(commands, commandPool);
		command->atlases.clear();
		for (size_t i = 0; i < size; ++i)
			command->atlases.emplace_back(atlas[i]);
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		commands.front()->atlases.clear();

		std::vector<ID3D11ShaderResourceView*> srv;
		for (size_t i = 0; i < size; ++i)
		{
			commands.front()->atlases.emplace_back(atlas[i]);

			dseed::graphics::d3d11_sprite_atlas_nativeobject nativeObject;
			atlas[i]->native_object((void**)&nativeObject);
			srv.emplace_back(nativeObject.shaderResourceView.Get());
		}
		immediateContext->PSSetShaderResources(0, (UINT)srv.size(), srv.data());
	}

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::set_constant(dseed::graphics::vgabuffer** constbuf, size_t size) noexcept
{
	if (!session)
		return dseed::error_invalid_op;
	if (constbuf == nullptr || size == 0)
		return dseed::error_invalid_args;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		SPRITE_RENDER_COMMAND* command = add_command(commands, commandPool);
		command->atlases.clear();
		for (size_t i = 0; i < size; ++i)
			command->constbufs.emplace_back(constbuf[i]);
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		commands.front()->constbufs.clear();

		std::vector<ID3D11Buffer*> buf;
		for (size_t i = 0; i < size; ++i)
		{
			commands.front()->constbufs.emplace_back(constbuf[i]);
			dseed::graphics::d3d11_sprite_vgabuffer_nativeobject nativeObject;
			constbuf[i]->native_object(reinterpret_cast<void**>(&nativeObject));
			buf.emplace_back(nativeObject.buffer.Get());
		}
		immediateContext->PSSetConstantBuffers(0, (UINT)buf.size(), buf.data());
	}

	return dseed::error_good;
}

dseed::error_t __d3d11_sprite_render::clear_rendertarget(dseed::graphics::sprite_rendertarget* rendertarget, const dseed::f32x4_t& color) noexcept
{
	if (session)
		return dseed::error_invalid_op;

	if (rendertarget != nullptr)
	{
		dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
		rendertarget->native_object(reinterpret_cast<void**>(&nativeObject));
		immediateContext->ClearRenderTargetView(nativeObject.renderTargetView.Get(), reinterpret_cast<const float*>(&color));
	}
	else
		immediateContext->ClearRenderTargetView(renderTargetView.Get(), reinterpret_cast<const float*>(&color));

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::update_constant(dseed::graphics::vgabuffer* constbuf, const void* buf, size_t offset, size_t length) noexcept
{
	if (session && renderMethod == dseed::graphics::rendermethod::deferred)
		return dseed::error_invalid_op;
	if (constbuf == nullptr || buf == nullptr || length == 0)
		return dseed::error_invalid_args;

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	constbuf->native_object(&buffer);

	D3D11_BOX destBox = {};
	destBox.left = static_cast<UINT>(offset);
	destBox.right = static_cast<UINT>(offset + length);
	immediateContext->UpdateSubresource(buffer.Get(), 0, &destBox, buf, static_cast<UINT>(length), 0);

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::update_atlas(dseed::graphics::sprite_atlas* atlas, dseed::bitmaps::bitmap* data) noexcept
{
	if (session && renderMethod == dseed::graphics::rendermethod::deferred)
		return dseed::error_invalid_op;
	if (atlas == nullptr || data == nullptr)
		return dseed::error_invalid_args;

	dseed::graphics::d3d11_sprite_atlas_nativeobject no;
	atlas->native_object(reinterpret_cast<void**>(&no));

	if (data->type() == dseed::bitmaps::bitmaptype::bitmap2d || data->type() == dseed::bitmaps::bitmaptype::bitmap2dcube)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2d;
		if (FAILED(no.texture.As<ID3D11Texture2D>(&tex2d)))
			return dseed::error_fail;

		D3D11_TEXTURE2D_DESC texDesc;
		tex2d->GetDesc(&texDesc);
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initialData;
		data->lock(const_cast<void**>(&initialData.pSysMem));
		initialData.SysMemPitch = (UINT)dseed::color::calc_bitmap_stride(data->format(), data->size().width);
		initialData.SysMemSlicePitch = (UINT)dseed::color::calc_bitmap_total_size(data->format(), data->size());

		Microsoft::WRL::ComPtr<ID3D11Texture2D> cpuTex;
		if (FAILED(d3dDevice->CreateTexture2D(&texDesc, &initialData, &cpuTex)))
			return dseed::error_fail;

		data->unlock();

		immediateContext->CopyResource(tex2d.Get(), cpuTex.Get());
	}
	else if (data->type() == dseed::bitmaps::bitmaptype::bitmap3d)
	{
		Microsoft::WRL::ComPtr<ID3D11Texture3D> tex3d;
		if (FAILED(no.texture.As<ID3D11Texture3D>(&tex3d)))
			return dseed::error_fail;

		D3D11_TEXTURE3D_DESC texDesc;
		tex3d->GetDesc(&texDesc);
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initialData;
		data->lock(const_cast<void**>(&initialData.pSysMem));
		initialData.SysMemPitch = (UINT)dseed::color::calc_bitmap_stride(data->format(), data->size().width);
		initialData.SysMemSlicePitch = (UINT)dseed::color::calc_bitmap_total_size(data->format(), data->size());

		Microsoft::WRL::ComPtr<ID3D11Texture3D> cpuTex;
		if (FAILED(d3dDevice->CreateTexture3D(&texDesc, &initialData, &cpuTex)))
			return dseed::error_fail;

		data->unlock();

		immediateContext->CopyResource(tex3d.Get(), cpuTex.Get());
	}

	return dseed::error_not_impl;
}

dseed::error_t __d3d11_sprite_render::begin(dseed::graphics::rendermethod method, const dseed::f32x4x4_t& transform) noexcept
{
	if (session) return dseed::error_invalid_op;
	if (!(method == dseed::graphics::rendermethod::deferred || method == dseed::graphics::rendermethod::forward))
		return dseed::error_invalid_args;

	session = true;
	renderMethod = method;

	immediateContext->ClearState();

	transformBuffer.worldTransform = transform;

	if (method == dseed::graphics::rendermethod::forward)
	{
		render_ready_common(immediateContext.Get());
		commands.push(commandPool.get_memory());
	}

	return dseed::error_good;
}
dseed::error_t __d3d11_sprite_render::end() noexcept
{
	if (!session)
		return dseed::error_invalid_op;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		render_ready_common(immediateContext.Get());

		while (!commands.empty())
		{
			SPRITE_RENDER_COMMAND* command = commands.front();

			render_ready_command(immediateContext.Get(), command);

			const auto instanceCount = command->instances.size();
			const auto loopCount = static_cast<size_t>(ceil(instanceCount / static_cast<double>(SPRITE_INSTANCE_COUNT)));
			command->instances.resize(loopCount * SPRITE_INSTANCE_COUNT);

			const auto* currentInstances = command->instances.data();
			for (size_t i = 0; i < loopCount; ++i)
			{
				const auto drawCount = dseed::minimum<size_t>(SPRITE_INSTANCE_COUNT, instanceCount - (i * SPRITE_INSTANCE_COUNT));
				immediateContext->UpdateSubresource(instanceConstantBuffer.Get(), 0, nullptr,
					currentInstances + (i * SPRITE_INSTANCE_COUNT), SPRITE_INSTANCE_TOTAL_SIZE, 0);
				immediateContext->DrawInstanced(4, (UINT)drawCount, 0, 0);
			}

			command->pipeline = nullptr;
			command->atlases.clear();
			command->constbufs.clear();
			command->instances.clear();
			command->renderTargets.clear();

			commandPool.return_memory(command);
			commands.pop();
		}
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		SPRITE_RENDER_COMMAND* command = commands.back();
		command->pipeline = nullptr;
		command->atlases.clear();
		command->constbufs.clear();
		command->instances.clear();
		command->renderTargets.clear();

		commandPool.return_memory(command);
		commands.pop();
	}

	session = false;

	return dseed::error_good;
}

dseed::error_t __d3d11_sprite_render::draw(size_t atlas_index, const dseed::f32x4x4_t& transform, const dseed::f32x4_t& color) noexcept
{
	if (!session)
		return dseed::error_invalid_args;

	if (commands.empty())
		return dseed::error_good;

	auto* command = commands.back();
	if (command->renderTargets.empty() || command->atlases.empty() || command->pipeline == nullptr)
		return dseed::error_good;

	const auto atlas = command->atlases[0]->atlas_element(atlas_index);
	const auto size = command->atlases[0]->size();
	const auto rcp = dseed::rcp(dseed::f32x4_t((float)size.width, (float)size.height, 0, 0));

	SPRITE_INSTANCE_DATA record = {};
	record.area = dseed::f32x4_t(
		atlas.x * rcp.x(),
		atlas.y * rcp.y(),
		atlas.width * rcp.x(),
		atlas.height * rcp.y());
	record.transform = dseed::float4x4::scale((float)size.width, (float)size.height, 1) * transform;
	record.color = color;

	if (renderMethod == dseed::graphics::rendermethod::deferred)
	{
		command->instances.emplace_back(record);
	}
	else if (renderMethod == dseed::graphics::rendermethod::forward)
	{
		static SPRITE_INSTANCE_DATA records[SPRITE_INSTANCE_COUNT];

		records[0] = record;

		immediateContext->UpdateSubresource(instanceConstantBuffer.Get(), 0, nullptr, records, sizeof(SPRITE_INSTANCE_DATA) * SPRITE_INSTANCE_COUNT, 0);
		immediateContext->DrawInstanced(4, 1, 0, 0);
	}

	return dseed::error_good;
}


void __d3d11_sprite_render::render_ready_common(ID3D11DeviceContext* deviceContext) const
{
	// Set Vertex Shader, Rasterizer State, Sampler State, ... to Device Context
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		ID3D11Buffer* vsConstantBuffers[] = { transformConstantBuffer.Get(), instanceConstantBuffer.Get() };
		deviceContext->VSSetConstantBuffers(0, _countof(vsConstantBuffers), vsConstantBuffers);
		deviceContext->RSSetState(rasterizerState.Get());
	}
}
void __d3d11_sprite_render::render_ready_command(ID3D11DeviceContext* deviceContext, const SPRITE_RENDER_COMMAND* command)
{
	// Set Render Target to Device Context
	{
		std::vector<ID3D11RenderTargetView*> rtv;
		std::vector<D3D11_VIEWPORT> viewports;
		rtv.reserve(command->renderTargets.size());
		viewports.reserve(command->renderTargets.size());
		for (size_t i = 0; i < command->renderTargets.size(); ++i)
		{
			if (command->renderTargets[i] == nullptr)
			{
				rtv.emplace_back(renderTargetView.Get());

				D3D11_VIEWPORT viewport = {};
				viewport.Width = (float)renderTargetSize.width;
				viewport.Height = (float)renderTargetSize.height;
				viewport.MaxDepth = 1.0f;
				viewports.emplace_back(viewport);
			}
			else
			{
				dseed::graphics::d3d11_sprite_rendertarget_nativeobject nativeObject;
				command->renderTargets[i]->native_object((void**)&nativeObject);
				rtv.emplace_back(nativeObject.renderTargetView.Get());

				dseed::autoref<dseed::graphics::sprite_atlas> atlas;
				command->renderTargets[i]->atlas(&atlas);

				D3D11_VIEWPORT viewport = {};
				viewport.Width = (float)atlas->size().width;
				viewport.Height = (float)atlas->size().height;
				viewport.MaxDepth = 1.0f;
				viewports.emplace_back(viewport);
			}
		}
		immediateContext->OMSetRenderTargets((UINT)rtv.size(), rtv.data(), nullptr);
		immediateContext->RSSetViewports((UINT)viewports.size(), viewports.data());

		transformBuffer.projectionTransform = dseed::float4x4::orthographic_offcenter(0, (float)viewports[0].Width, (float)viewports[0].Height, 0, 0.00001f, 32767.0f);
		immediateContext->UpdateSubresource(transformConstantBuffer.Get(), 0, nullptr, &transformBuffer, sizeof(SPRITE_TRANSFORM), 0);
	}

	// Set Pipeline to Device Context
	{
		dseed::graphics::d3d11_sprite_pipeline_nativeobject no;
		command->pipeline->native_object(reinterpret_cast<void**>(&no));

		deviceContext->PSSetShader(no.pixelShader.Get(), nullptr, 0);
		deviceContext->OMSetBlendState(no.blendState.Get(), nullptr, 0xffffffff);
		ID3D11SamplerState* samplerState = no.samplerState.Get();
		deviceContext->PSSetSamplers(0, 1, &samplerState);
	}

	// Set Constant Buffers to Pixel Shader for Device Context
	if (!command->constbufs.empty())
	{
		std::vector<ID3D11Buffer*> constantBuffers;
		constantBuffers.reserve(command->constbufs.size());
		for (const auto& cb : command->constbufs)
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> d3d11Buffer;
			cb->native_object(&d3d11Buffer);
			constantBuffers.emplace_back(d3d11Buffer.Get());
		}
		deviceContext->PSSetConstantBuffers(0, static_cast<UINT>(constantBuffers.size()), constantBuffers.data());
	}

	// Set Atlases
	{
		std::vector<ID3D11ShaderResourceView*> srv;
		for (size_t i = 0; i < command->atlases.size(); ++i)
		{
			dseed::graphics::d3d11_sprite_atlas_nativeobject nativeObject;
			command->atlases[i]->native_object(reinterpret_cast<void**>(&nativeObject));
			srv.emplace_back(nativeObject.shaderResourceView.Get());
		}
		immediateContext->PSSetShaderResources(0, (UINT)srv.size(), srv.data());
	}
}

#endif