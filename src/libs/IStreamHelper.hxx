#ifndef __DSEED_ISTREAM_HELPER_HXX__
#define __DSEED_ISTREAM_HELPER_HXX__

class ImpledIStream : public IStream
{
public:
	static HRESULT Create(dseed::io::stream* stream, IStream** ret)
	{
		if (stream == nullptr || ret == nullptr) return E_FAIL;
		*ret = new ImpledIStream(stream);
		if (*ret == nullptr)
			return E_OUTOFMEMORY;
		return S_OK;
	}

public:
	ImpledIStream(dseed::io::stream* stream)
		: _refCount(1), _stream(stream)
	{ }

public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
	{
		if (riid == __uuidof (IStream) || riid == __uuidof (IUnknown)
			|| riid == __uuidof(ISequentialStream))
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	virtual ULONG __stdcall AddRef(void) override { return InterlockedIncrement(&_refCount); }
	virtual ULONG __stdcall Release(void) override
	{
		auto ret = InterlockedDecrement(&_refCount);
		if (ret == 0)
			delete this;
		return ret;
	}

public:
	virtual HRESULT __stdcall Read(void* pv, ULONG cb, ULONG* pcbRead) override
	{
		if (!_stream->readable()) return E_FAIL;
		auto read = _stream->read(pv, cb);
		if (pcbRead)
			*pcbRead = (ULONG)read;
		return S_OK;
	}
	virtual HRESULT __stdcall Write(const void* pv, ULONG cb, ULONG* pcbWritten) override
	{
		if (!_stream->writable()) return E_FAIL;
		auto written = _stream->write(pv, cb);
		if (pcbWritten)
			*pcbWritten = (ULONG)written;
		return S_OK;
	}
	virtual HRESULT __stdcall Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
	{
		if (!_stream->seekable()) return E_FAIL;
		if (!_stream->seek((dseed::io::seekorigin)dwOrigin, dlibMove.QuadPart))
			return E_FAIL;

		if (plibNewPosition)
			plibNewPosition->QuadPart = _stream->position();

		return S_OK;
	}
	virtual HRESULT __stdcall SetSize(ULARGE_INTEGER libNewSize) override
	{
		_stream->set_length(libNewSize.QuadPart);
		return S_OK;
	}
	virtual HRESULT __stdcall Stat(STATSTG* pstatstg, DWORD grfStatFlag) override
	{
		memset(pstatstg, 0, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize.QuadPart = _stream->length();
		pstatstg->grfMode = (_stream->readable() ? STGM_READ : 0) | (_stream->writable() ? STGM_WRITE : 0);
		return S_OK;
	}
	virtual HRESULT __stdcall Commit(DWORD grfCommitFlags) override { _stream->flush(); return S_OK; }
	virtual HRESULT __stdcall CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override { return E_NOTIMPL; }
	virtual HRESULT __stdcall Revert(void) override { return E_NOTIMPL; }
	virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override { return E_NOTIMPL; }
	virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override { return E_NOTIMPL; }
	virtual HRESULT __stdcall Clone(IStream** ppstm) override { return E_NOTIMPL; }

private:
	ULONG _refCount;
	dseed::autoref<dseed::io::stream> _stream;
};

#endif