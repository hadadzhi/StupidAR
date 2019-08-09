#include "StupidAudioRenderer.h"

StupidAudioRenderer::StupidAudioRenderer(LPUNKNOWN pUnknown, HRESULT* pResult)
	: CBaseRenderer(kMyFilterGuid, kMyFilterName, pUnknown, pResult) {
}

StupidAudioRenderer::~StupidAudioRenderer() {
}

HRESULT StupidAudioRenderer::CheckMediaType(const CMediaType*) {
	// Accept anything -- actual checking is done in SetMediaType()
	return S_OK;
}

HRESULT StupidAudioRenderer::SetMediaType(const CMediaType* pMediaType) {
	return S_OK;
}

HRESULT StupidAudioRenderer::DoRenderSample(IMediaSample* pSample) {
	return S_OK;
}
