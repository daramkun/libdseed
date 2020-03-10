#ifndef __DSEED_GRAPHICS_COMMON_D3D11_HXX__
#define __DSEED_GRAPHICS_COMMON_D3D11_HXX__

#	include <dseed.h>
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
		default: return (D3D11_BLEND_OP)-1;
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
		default: return (D3D11_BLEND)-1;
	}
}

inline HRESULT CreateBlendStateFromBlendParams (ID3D11Device* d3dDevice, const dseed::graphics::blendparams& params, ID3D11BlendState** blendState)
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = params.blend_enable;
	blendDesc.RenderTarget[0].BlendOp = DSEEDBLENDOP_TO_D3D11BLENDOP (params.blend_op);
	blendDesc.RenderTarget[0].SrcBlend = DSEEDBLENDVALUE_TO_D3D11_BLENDVALUE (params.src);
	blendDesc.RenderTarget[0].DestBlend = DSEEDBLENDVALUE_TO_D3D11_BLENDVALUE (params.dest);
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	return d3dDevice->CreateBlendState (&blendDesc, blendState);
}

inline HRESULT CompileHLSL (const char* profile, const char* code, ID3DBlob** blob)
{
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorMsgBlob;
	if (FAILED (hr = D3DCompile2 (code, strlen (code), nullptr, nullptr, nullptr,
		"main", profile, D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, 0, nullptr, 0, blob, &errorMsgBlob)))
	{
		dseed::diagnostics::logger::shared_logger ()->write (dseed::diagnostics::loglevel::error, "HLSL Compile",
			(const char*)errorMsgBlob.Get ()->GetBufferPointer ());
	}
	return hr;
}

inline HRESULT CreateTexture2D (ID3D11Device* d3dDevice, const dseed::size2i& size, dseed::color::pixelformat format, ID3D11Texture2D** texture,
	ID3D11ShaderResourceView** srv, ID3D11DepthStencilView** dsv, ID3D11RenderTargetView** rtv)
{
	if (d3dDevice == nullptr || texture == nullptr)
		return E_FAIL;

	DWORD flags = 0;
	if (srv != nullptr) flags |= D3D11_BIND_SHADER_RESOURCE;
	if (dsv != nullptr) flags |= D3D11_BIND_DEPTH_STENCIL;
	if (rtv != nullptr) flags |= D3D11_BIND_RENDER_TARGET;

	DXGI_FORMAT dxgiFormat = DSEEDPF_TO_DXGIPF (format);

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = size.width;
	textureDesc.Height = size.height;
	textureDesc.ArraySize = 1;
	if (dxgiFormat == DXGI_FORMAT_D16_UNORM) textureDesc.Format = DXGI_FORMAT_R16_TYPELESS;
	else if (dxgiFormat == DXGI_FORMAT_D24_UNORM_S8_UINT) textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	else if (dxgiFormat == DXGI_FORMAT_D32_FLOAT) textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	else textureDesc.Format = dxgiFormat;
	textureDesc.MipLevels = 1;
	textureDesc.BindFlags = flags;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT hr;

	hr = d3dDevice->CreateTexture2D (&textureDesc, nullptr, texture);
	if (FAILED (hr)) return hr;

	if (srv != nullptr)
	{
		hr = d3dDevice->CreateShaderResourceView (*texture, nullptr, srv);
		if (FAILED (hr)) return hr;
	}
	if (dsv != nullptr)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = dxgiFormat;
		dsvDesc.Texture2D.MipSlice = 1;

		hr = d3dDevice->CreateDepthStencilView (*texture, nullptr, dsv);
		if (FAILED (hr)) return hr;
	}
	if (rtv != nullptr)
	{
		hr = d3dDevice->CreateRenderTargetView (*texture, nullptr, rtv);
		if (FAILED (hr)) return hr;
	}

	return hr;
}

inline HRESULT CreateTextureFromBitmap (ID3D11Device* d3dDevice, dseed::bitmaps::bitmap* bitmap, ID3D11Resource** texture,
	ID3D11ShaderResourceView** srv, ID3D11RenderTargetView** rtv)
{
	if (d3dDevice == nullptr || bitmap == nullptr || texture == nullptr)
		return E_FAIL;

	DWORD flags = 0;
	if (srv != nullptr) flags |= D3D11_BIND_SHADER_RESOURCE;
	if (rtv != nullptr) flags |= D3D11_BIND_RENDER_TARGET;

	DXGI_FORMAT dxgiFormat = DSEEDPF_TO_DXGIPF (bitmap->format ());

	HRESULT hr;

	switch (bitmap->type ())
	{
		case dseed::bitmaps::bitmaptype::bitmap2d:
		case dseed::bitmaps::bitmaptype::bitmap2dcube:
			{
				Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
				D3D11_TEXTURE2D_DESC textureDesc = { };
				textureDesc.MipLevels = 1;
				textureDesc.ArraySize = bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2d ? 1 : 6;
				textureDesc.Width = bitmap->size ().width;
				textureDesc.Height = bitmap->size ().height;
				textureDesc.Format = dxgiFormat;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = flags;
				textureDesc.MiscFlags = bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2dcube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
				textureDesc.SampleDesc.Count = 1;

				void* ptr;
				bitmap->lock (&ptr);

				D3D11_SUBRESOURCE_DATA subResourceData = { };
				subResourceData.pSysMem = ptr;
				subResourceData.SysMemPitch = (UINT)dseed::color::calc_bitmap_stride (bitmap->format (), textureDesc.Width);
				subResourceData.SysMemSlicePitch = (UINT)dseed::color::calc_bitmap_total_size (bitmap->format (), bitmap->size ());

				hr = d3dDevice->CreateTexture2D (&textureDesc, &subResourceData, &texture2d);
				bitmap->unlock ();

				if (FAILED (hr))
					return hr;

				texture2d->QueryInterface<ID3D11Resource> (texture);
			}
			break;

		case dseed::bitmaps::bitmaptype::bitmap3d:
			{
				Microsoft::WRL::ComPtr<ID3D11Texture3D> texture3d;
				D3D11_TEXTURE3D_DESC textureDesc = { };
				textureDesc.MipLevels = 1;
				textureDesc.Width = bitmap->size ().width;
				textureDesc.Height = bitmap->size ().height;
				textureDesc.Depth = bitmap->size ().depth;
				textureDesc.Format = dxgiFormat;
				textureDesc.Usage = D3D11_USAGE_DEFAULT;
				textureDesc.BindFlags = flags;

				void* ptr;
				bitmap->lock (&ptr);

				D3D11_SUBRESOURCE_DATA subResourceData = { };
				subResourceData.pSysMem = ptr;
				subResourceData.SysMemPitch = (UINT)dseed::color::calc_bitmap_stride (bitmap->format (), textureDesc.Width);
				subResourceData.SysMemSlicePitch = (UINT)dseed::color::calc_bitmap_total_size (bitmap->format (), bitmap->size ());

				hr = d3dDevice->CreateTexture3D (&textureDesc, &subResourceData, &texture3d);
				bitmap->unlock ();

				if (FAILED (hr))
					return hr;

				texture3d->QueryInterface<ID3D11Resource> (texture);
			}
			break;

		default: return E_INVALIDARG;
	}

	if (srv)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
		srvDesc.Format = dxgiFormat;
		if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2d)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
		}
		else if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2dcube)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.ArraySize = 6;
		}
		else if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap3d)
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = 1;
		}

		hr = d3dDevice->CreateShaderResourceView (*texture, &srvDesc, srv);
		if (FAILED (hr))
			return hr;
	}

	if (rtv)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
		rtvDesc.Format = dxgiFormat;
		if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2d)
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 1;
		}
		else if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap2dcube)
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.MipSlice = 1;
			rtvDesc.Texture2DArray.ArraySize = 6;
		}
		else if (bitmap->type () == dseed::bitmaps::bitmaptype::bitmap3d)
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
			rtvDesc.Texture3D.MipSlice = 1;
		}

		hr = d3dDevice->CreateRenderTargetView (*texture, &rtvDesc, rtv);
		if (FAILED (hr))
			return hr;
	}

	return S_OK;
}

inline HRESULT CreateConstantBuffer (ID3D11Device* d3dDevice, UINT size, ID3D11Buffer** buffer, const void* buf, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
{
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.StructureByteStride = 0;
	constantBufferDesc.ByteWidth = size;
	constantBufferDesc.Usage = usage;
	if (usage == D3D11_USAGE_DYNAMIC || usage == D3D11_USAGE_STAGING)
		constantBufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
	if (usage == D3D11_USAGE_STAGING)
		constantBufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;

	D3D11_SUBRESOURCE_DATA subResourceData = { };
	subResourceData.pSysMem = buf;
	subResourceData.SysMemPitch = size;

	return d3dDevice->CreateBuffer (&constantBufferDesc, buf != nullptr ? &subResourceData : nullptr, buffer);
}

#endif